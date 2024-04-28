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
		fprintf(stderr, "Error: Unable to open file\n");
		return 1;
	}

	rc = fseek(file, 0L, SEEK_END);
	if (rc != 0) {
		fprintf(stderr, "Error: Unable to seek file\n");
		goto end;
	}

	*file_size = (unsigned long) ftell(file);

	if (*file_size > MAX_FILE_SIZE) {
		rc = 1;
		fprintf(stderr, "Error: File too large\n");
		goto end;
	}

	rc = fseek(file, 0L, SEEK_SET);
	if (rc != 0) {
		fprintf(stderr, "Error: Unable to seek file\n");
		goto end;
	}

	*buffer = malloc(*file_size + 1);
	if (buffer == NULL) {
		rc = 1;
		fprintf(stderr, "Error: Unable to allocate buffer\n");
		goto end;
	}
	(*buffer)[*file_size] = '\0';

	if (fread(*buffer, 1, *file_size, file) != *file_size) {
		rc = 1;
		fprintf(stderr, "Error: Unable to read file\n");
		goto end;
	}

end:
	if (file != NULL) {
		fclose(file);
	}

	return rc;
}

static inline void skip_charset(char **cur, const char *end, const char *charset) {
	while (*cur < end && strchr(charset, **cur) != NULL) {
		(*cur)++;
	}
}

static inline void skip_until_charset(char **cur, const char *end, const char *charset) {
	while (*cur < end && strchr(charset, **cur) == NULL) {
		(*cur)++;
	}
}

int main(int argc, const char *argv[]) {
	int rc;
	unsigned long file_size;
	char *buffer, *end, *cur;

	if (argc < 1) {
		return 1;
	}

	if (argc < 2) {
		fprintf(stderr, "Error: Missing command. Usage: %s <command> [args...]\n", argv[0]);
		return 1;
	}

	buffer = NULL;

	rc = read_file(".env", &buffer, &file_size);
	if (rc != 0) {
		return rc;
	}

	cur = buffer;
	end = buffer + file_size;

	while (cur < end) {
		char *key, *key_end, *value, *value_end;
		// for each line
		skip_charset(&cur, end, " \t\r\n");

		if (cur >= end) {
			break;
		}

		if (*cur == '#') {
			// skip line comment
			skip_until_charset(&cur, end, "\n");
			continue;
		}

		key = cur;
		skip_until_charset(&cur, end, " \t=");
		if (cur >= end) {
			fprintf(stderr, "Error: unexpected end of file after key \"%s\"\n", key);
			rc = 1;
			goto end;
		}
		key_end = cur;
		skip_charset(&cur, end, " \t");

		if (*cur != '=' && *cur != '\0') {
			fprintf(stderr, "Error: Missing equal after key \"%s\"\n", key);
			rc = 1;
			goto end;
		}
		if (key_end == key) {
			fprintf(stderr, "Error: Empty key\n");
			rc = 1;
			goto end;
		}

		*key_end = '\0';

		cur++;
		skip_charset(&cur, end, " \t");

		if (*cur == '"' || *cur == '\'') {
			// quoted value
			char quote;
			char *inner_cur;

			quote = *cur;
			cur++;
			value = cur;

			/* this cursor is used to write the unescaped value */
			inner_cur = cur;

			while (cur < end) {
				if (*cur == '\\') {
					cur++; // skip escape character
					if (cur >= end) {
						fprintf(stderr, "Error: unexpected end of file after escape character at key \"%s\"\n", key);
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
							fprintf(stderr, "Error: Invalid escape character at key \"%s\"\n", key);
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
				fprintf(stderr, "Error: Missing closing quote\n");
				rc = 1;
				goto end;
			}

			key_end = inner_cur;
			cur++;
		} else {
			value = cur;
			// unquoted value
			skip_until_charset(&cur, end, "\r\n");
			key_end = cur;
		}
		*key_end = '\0';

		rc = setenv(key, value, 1);
		if (rc != 0) {
			perror("Error: Unable to set environment variable");
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
		perror("Error: Unable to execute command");
		return rc;
	}

	return 0;
}
