#! /bin/sh

set -x
autopoint
aclocal -I m4
autoheader
automake --add-missing
autoconf
