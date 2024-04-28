# INSTALL

```bash
make
sudo make install
```

DOTENV(1) - General Commands Manual

# NAME

**dotenv** - load environment variables from a file

# SYNOPSIS

**dotenv**
*program&nbsp;...*

# DESCRIPTION

The
**dotenv**
utility
reads a file (by default
*.env*
in the current directory) and loads the environment variables from it.
It then executes the specified program with the loaded environment
variables.
If no dotenv file is found, it will execute the program with
the current environment.

# ENVIRONMENT VARIABLES

`DOTENV_PATH`

> The path to the dotenv file.
> If not set, the default path is
> *.env*
> in the current directory.

`DOTENV_CONFIG_PATH`

> Same as
> `DOTENV_PATH`
> for compatibility with the original nodejs
> **dotenv**
> implementation.

# EXAMPLES

Run the
env(1)
command with the environment variables loaded from the .env file in the
current directory:

	$ dotenv env

Same as above but with a custom dotenv file path:

	$ DOTENV_PATH=.env2 dotenv env

Use dotenv in the shebang line of a script:

	#!/usr/bin/env -S dotenv bash
	echo $MY_VAR

# FILE FORMAT

The file format is a list of key-value pairs, one per line.
The key and value are separated by an equal sign.
Whitespaces around keys and values are ignored.
A line is terminated by a newline character or a carriage return.
Comments can be added using a hash sign (#) either at the start of a line
or inline. For values that contain a hash sign, the value must be quoted.
Values can be optionally quoted with single or double quotes.
If quoted, the following escape sequences are supported:

&#92;n

> A newline.

&#92;r

> A carriage return.

&#92;t

> A tab.

&#92;'

> A single quote.

&#92;"

> A double quote.

&#92;&#92;

> A backslash.

# SEE ALSO

env(1)

# AUTHORS

Yifan Gu &lt;[me@yifangu.com](mailto:me@yifangu.com)&gt;

Unknown OS - April 28, 2024
