#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.63])
AC_INIT([mentohust], [0.3.0], [http://code.google.com/p/mentohust/issues/list])

AM_INIT_AUTOMAKE
AC_CONFIG_SRCDIR([src/myconfig.h])
AC_CONFIG_HEADERS([config.h])

# Checks for programs.
AC_ARG_ENABLE(debug,
                [ AC_HELP_STRING([--enable-debug], [Enable DEBUG output. This is a debugging feature which should not be only enabled by develpoers]) ],
                [ CFLAGS="$CFLAGS -g2 -O0 -DDEBUG -Wall" ],
                [ CFLAGS="$CFLAGS -Os -Wall" ]
              )

AC_PROG_CC

AM_PROG_CC_C_O

AC_PROG_LN_S
AC_PROG_MAKE_SET

# Checks for libraries.
AC_CHECK_LIB([iconv], [iconv])
AC_ARG_ENABLE(nodypcap,
                [ AC_HELP_STRING([--enable-nodypcap], [Don't load libpcap.so dynamicly]) ],
                [
                  AC_CHECK_LIB([pcap], [pcap_sendpacket])
                  CFLAGS="$CFLAGS -DNODLL"
                ],
                [
	AC_ARG_WITH(stpcap,
					[ AC_HELP_STRING([--with-stpcap], [Link with libpcap.a]) ],
					[
					  AC_SUBST(LDADD,'$(top_builddir)'/other/libpcap.a)
					  CFLAGS="$CFLAGS -DNODLL"
					],
					[ AC_CHECK_LIB([dl], [dlsym]) ]
				  )
				]
              )
AC_ARG_ENABLE(nonotify,
                [ AC_HELP_STRING([--enable-nonotify], [Don't show notification]) ],
                [ CFLAGS="$CFLAGS -DNONOTIFY" ],
                [ AC_CHECK_LIB([dl], [dlsym]) ]
              )

# Checks for header files.
AC_CHECK_HEADERS([iconv.h arpa/inet.h fcntl.h netinet/in.h stdint.h stdlib.h string.h sys/ioctl.h sys/socket.h sys/time.h unistd.h])

AC_CHECK_HEADER([pcap.h],[],[AC_SUBST(AM_CFLAGS,-I'$(top_builddir)'/other)])


# Checks for typedefs, structures, and compiler characteristics.
AC_TYPE_SIZE_T
AC_TYPE_UINT32_T

# Checks for library functions.
AC_FUNC_MALLOC
AC_CHECK_FUNCS([atexit memmove memset socket])

AC_CONFIG_FILES([Makefile src/Makefile ])
AC_OUTPUT
