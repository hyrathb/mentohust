#! /bin/sh

set -x

aclocal
autoheader
automake --add-missing
autoconf
