#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.63])
AC_INIT([mentohust], [2.4], [http://code.google.com/p/mentohust/issues/list])

AM_INIT_AUTOMAKE

AC_PROG_RANLIB

AC_CONFIG_SRCDIR([src/myconfig.h])
AC_CONFIG_HEADERS([config.h])

# Checks for programs.
AC_PROG_CC

AM_PROG_CC_C_O


AC_PROG_LN_S
AC_PROG_MAKE_SET

# Checks for libraries.
# FIXME: Replace `main' with a function in `-lpcap':
AC_CHECK_LIB([pcap], [main])

#AC_CHECK_LIB([pcap], [main])

AC_CHECK_HEADERS([iconv.h])

# Checks for header files.
AC_CHECK_HEADERS([arpa/inet.h stdlib.h string.h sys/ioctl.h sys/socket.h sys/time.h unistd.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_TYPE_SIZE_T

# Checks for library functions.
AC_FUNC_MALLOC
AC_CHECK_FUNCS([memset socket])

AC_CONFIG_FILES([Makefile src/Makefile ])
AC_OUTPUT
