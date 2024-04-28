# Copyright (c) 2024 Yifan Gu
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions
# are met:
# 1. Redistributions of source code must retain the above copyright
#    notice, this list of conditions and the following disclaimer.
# 2. Redistributions in binary form must reproduce the above copyright
#    notice, this list of conditions and the following disclaimer in the
#    documentation and/or other materials provided with the distribution.
#
# THIS SOFTWARE IS PROVIDED BY THE CONTRIBUTOR ``AS IS'' AND ANY EXPRESS OR
# IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
# OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
# IN NO EVENT SHALL THE CONTRIBUTOR BE LIABLE FOR ANY DIRECT, INDIRECT,
# INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
# NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
# DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
# THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
# (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
# OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

.PHONY: all clean

PREFIX ?= /usr/local
MANPREFIX ?= ${PREFIX}/share/man

all: dotenv

dotenv: dotenv.c
	${CC} -Os -Wall -Werror -o dotenv dotenv.c

clean:
	rm -f dotenv

install: all
	install -dm755 ${DESTDIR}${PREFIX}/bin
	install -dm755 ${DESTDIR}${MANPREFIX}/man1
	install -m755 dotenv ${DESTDIR}${PREFIX}/bin/dotenv
	install -m644 dotenv.1 ${DESTDIR}${MANPREFIX}/man1/dotenv.1

README.md: dotenv.1 README.install.md
	cat README.install.md > README.md
	echo "" >> README.md
	mandoc -T markdown -I os="Unknown OS" dotenv.1 >> README.md
