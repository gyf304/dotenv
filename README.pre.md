Prefix any command with `dotenv` to load environment variables from a
file named `.env` in the current directory.

```bash
dotenv sh -c 'echo $MY_VAR'
```

Alternatively put `dotenv` in the shebang line of a script to
automatically load environment variables.

```bash
#!/usr/bin/env -S dotenv bash
echo $MY_VAR
```

`dotenv` is a tiny, self-contained utility.

This README file contains the installation instructions and the manpage
for the `dotenv` utility.

# INSTALL

`dotenv` can be installed on a Unix-like system with a C compiler and
`make` installed by running the following commands:

```bash
make
sudo make install
```

This will install the `dotenv` binary to `/usr/local/bin`, and the
manpage to `/usr/local/share/man/man1`.
