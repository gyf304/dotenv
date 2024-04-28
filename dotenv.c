/*
Copyright (c) 2024 Yifan Gu

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions
are met:
1. Redistributions of source code must retain the above copyright
   notice, this list of conditions and the following disclaimer.
2. Redistributions in binary form must reproduce the above copyright
   notice, this list of conditions and the following disclaimer in the
   documentation and/or other materials provided with the distribution.

THIS SOFTWARE IS PROVIDED BY THE CONTRIBUTOR ``AS IS'' AND ANY EXPRESS OR
IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
IN NO EVENT SHALL THE CONTRIBUTOR BE LIABLE FOR ANY DIRECT, INDIRECT,
INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define MAX_FILE_SIZE (1024 * 1024)

static inline int read_file(const char *filename, char **buffer, unsigned long *file_size) {
	int rc;
	FILE *file;

	file = fopen(filename, "rb");
	if (file == NULL) {
		*buffer = NULL;
		*file_size = 0;
		return 0;
	}

	rc = fseek(file, 0L, SEEK_END);
	if (rc != 0) {
		fprintf(stderr, "Unable to seek file\n");
		goto end;
	}

	*file_size = (unsigned long) ftell(file);

	if (*file_size > MAX_FILE_SIZE) {
		rc = 1;
		fprintf(stderr, "File too large\n");
		goto end;
	}

	rc = fseek(file, 0L, SEEK_SET);
	if (rc != 0) {
		fprintf(stderr, "Unable to seek file\n");
		goto end;
	}

	*buffer = malloc(*file_size + 1);
	if (buffer == NULL) {
		rc = 1;
		fprintf(stderr, "Unable to allocate buffer\n");
		goto end;
	}
	(*buffer)[*file_size] = '\0';

	if (fread(*buffer, 1, *file_size, file) != *file_size) {
		rc = 1;
		fprintf(stderr, "Unable to read file\n");
		goto end;
	}

end:
	if (file != NULL) {
		fclose(file);
	}

	return rc;
}

static inline int skip_charset(char **cur, const char *end, const char *charset) {
	int count = 0;
	while (*cur < end && strchr(charset, **cur) != NULL) {
		(*cur)++;
		count++;
	}
	return count;
}

static inline int skip_until_charset(char **cur, const char *end, const char *charset) {
	int count = 0;
	while (*cur < end && strchr(charset, **cur) == NULL) {
		(*cur)++;
		count++;
	}
	return count;
}

int main(int argc, const char *argv[]) {
	int rc;
	unsigned long file_size;
	char *buffer, *end, *cur;
	const char *dotenv_path;

	dotenv_path = getenv("DOTENV_PATH");
	if (dotenv_path == NULL || *dotenv_path == '\0') {
		dotenv_path = getenv("DOTENV_CONFIG_PATH");
	}
	if (dotenv_path == NULL || *dotenv_path == '\0') {
		dotenv_path = ".env";
	}

	if (argc < 1) {
		return 1;
	}

	if (argc < 2) {
		fprintf(stderr, "Missing command. Usage: %s <command> [args...]\n", argv[0]);
		return 1;
	}

	buffer = NULL;
	file_size = 0;

	rc = read_file(dotenv_path, &buffer, &file_size);
	if (rc != 0 || buffer == NULL) {
		rc = 1;
		goto end;
	}

	cur = buffer;
	end = buffer + file_size;

	while (cur < end) {
		char *key, *key_end, *value, *value_end;
		/* for each line */
		skip_charset(&cur, end, " \t\r\n");

		if (cur >= end) {
			break;
		}

		if (*cur == '#') {
			/* skip line comment */
			skip_until_charset(&cur, end, "\r\n");
			continue;
		}

		key = cur;
		skip_until_charset(&cur, end, " \t=\r\n");
		if (cur >= end) {
			fprintf(stderr, "Unexpected end of file after key \"%s\"\n", key);
			rc = 1;
			goto end;
		}
		key_end = cur;
		skip_charset(&cur, end, " \t");

		if (*cur != '=' && *cur != '\0') {
			*cur = '\0';
			fprintf(stderr, "Missing equal after key \"%s\"\n", key);
			rc = 1;
			goto end;
		}
		if (key_end == key) {
			fprintf(stderr, "Empty key\n");
			rc = 1;
			goto end;
		}

		*key_end = '\0';

		cur++;
		skip_charset(&cur, end, " \t");

		if (*cur == '"' || *cur == '\'') {
			/* parse quoted value */
			char quote;
			char *inner_cur;

			quote = *cur;
			cur++;
			value = cur;

			/* this cursor is used to write the unescaped value */
			inner_cur = cur;

			while (cur < end) {
				if (*cur == '\\') {
					cur++;
					if (cur >= end) {
						fprintf(stderr, "Unexpected end of file after escape character\n");
						rc = 1;
						goto end;
					}
					switch (*cur) {
						case 'n':
							*inner_cur = '\n';
							break;
						case 'r':
							*inner_cur = '\r';
							break;
						case 't':
							*inner_cur = '\t';
							break;
						case '\\':
							*inner_cur = '\\';
							break;
						case '\'':
							*inner_cur = '\'';
							break;
						case '"':
							*inner_cur = '"';
							break;
						default:
							fprintf(stderr, "Invalid escape character at key \"%s\"\n", key);
							rc = 1;
							goto end;
					}
					inner_cur++;
					cur++;
					continue;
				}

				if (*cur == quote) {
					break;
				}
				*inner_cur = *cur;

				inner_cur++;
				cur++;
			}

			if (*cur != quote) {
				fprintf(stderr, "Missing closing quote\n");
				rc = 1;
				goto end;
			}

			value_end = inner_cur;
			cur++;
			skip_charset(&cur, end, " \t");
			if (skip_until_charset(&cur, end, "#\r\n") > 0) {
				fprintf(stderr, "Unexpected characters after closing quote\n");
				rc = 1;
				goto end;
			}
			skip_until_charset(&cur, end, "\r\n");
		} else {
			/* parse unquoted value */
			value_end = NULL;
			value = cur;
			while (skip_until_charset(&cur, end, "# \t\r\n") && strchr("#\r\n", *cur) == NULL) {
				value_end = cur;
				cur++;
			}
			if (value_end == NULL) {
				value_end = cur;
			}
			skip_until_charset(&cur, end, "\r\n");
		}
		*value_end = '\0';

		rc = setenv(key, value, 1);
		if (rc != 0) {
			perror("Unable to set environment variable");
			goto end;
		}
	}

end:
	if (buffer != NULL) {
		free(buffer);
	}

	if (rc != 0) {
		return rc;
	}

	rc = execvp(argv[1], (char *const *) &argv[1]);
	if (rc != 0) {
		fprintf(stderr, "Unable to execute command %s: ", argv[1]);
		perror("");
		return rc;
	}

	return 0;
}
