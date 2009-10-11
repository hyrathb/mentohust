#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.63])
AC_INIT([mentohust], [0.2.2], [http://code.google.com/p/mentohust/issues/list])

AM_INIT_AUTOMAKE
AC_CONFIG_SRCDIR([src/myconfig.h])
AC_CONFIG_HEADERS([config.h])

# Checks for programs.
${CFLAGS="-O2 -Wall"}
AC_PROG_CC

# Checks for libraries.
# FIXME: Replace `main' with a function in `-lpcap':
AC_CHECK_LIB([pcap], [main])

# Checks for header files.
AC_CHECK_HEADERS([iconv.h arpa/inet.h stdlib.h string.h sys/ioctl.h sys/socket.h sys/time.h unistd.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_TYPE_SIZE_T

# Checks for library functions.
AC_FUNC_MALLOC
AC_CHECK_FUNCS([atexit memmove memset socket])

AC_CONFIG_FILES([Makefile src/Makefile ])
AC_OUTPUT
