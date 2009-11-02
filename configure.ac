#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.63])
AC_INIT([mentohust], [0.2.4], [http://code.google.com/p/mentohust/issues/list])

AM_INIT_AUTOMAKE
AC_CONFIG_SRCDIR([src/myconfig.h])
AC_CONFIG_HEADERS([config.h])

# Checks for programs.
AC_PROG_CC

AM_PROG_CC_C_O

AC_PROG_LN_S
AC_PROG_MAKE_SET

AC_SUBST_FILE(INCLUDES)

AC_ARG_ENABLE(debug,
                [  --enable-debug Enable DEBUG output. This is a debugging feature
        which should not be only enabled by develpoers
                ],
                [ CFLAGS="$CFLAGS -g2 -O0 -DDEBUG -Wall"],
                [ CFLAGS="$CFLAGS -Os -Wall"]
              )

# Checks for libraries.
# FIXME: Replace `main' with a function in `-lpcap':
AC_CHECK_LIB([pcap], [main])

# Checks for header files.
AC_CHECK_HEADERS([iconv.h arpa/inet.h fcntl.h netinet/in.h stdint.h stdlib.h string.h sys/ioctl.h sys/socket.h sys/time.h unistd.h])

AC_CHECK_HEADER([pcapd.h],[],[AC_SUBST(INCLUDES,-I'$(top_builddir)'/other)])


# Checks for typedefs, structures, and compiler characteristics.
AC_TYPE_SIZE_T
AC_TYPE_UINT32_T

# Checks for library functions.
AC_FUNC_MALLOC
AC_CHECK_FUNCS([atexit memmove memset socket])

AC_CONFIG_FILES([Makefile src/Makefile ])
AC_OUTPUT
