#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.63])
AC_INIT([mentohust], [0.3.1], [http://code.google.com/p/mentohust/issues/list])
AC_CANONICAL_HOST
AM_INIT_AUTOMAKE
AC_CONFIG_SRCDIR([src/myconfig.h])
AC_CONFIG_HEADERS([config.h])

# Checks for programs.
AC_PROG_CC

AC_ARG_ENABLE([debug], [AC_HELP_STRING([--enable-debug], [build a debug version])],
              [CFLAGS="-g -O0 -DDEBUG -Wall"],
              [CFLAGS="-Os -Wall"])

# Checks for libraries.
AS_CASE([$host_os],
        [*apple* | *darwin*],
        [AC_DEFINE(MAC_OS, [], [Define MAC_OS when compiling for Mac OS])
         AC_CHECK_LIB([iconv], [iconv])])

AC_ARG_WITH([pcap], [AC_HELP_STRING([--with-pcap=TYPE], [How to link libpcap: dyload(Default)|dynamic|static|PATH])],
            [], [with_pcap="dyload"])
AS_IF([test "x$with_pcap" = "xyes" || test -z "$with_pcap"], [with_pcap="dyload"])
AS_IF([test "$with_pcap" = "dyload"], [AC_CHECK_LIB([dl], [dlsym])],
      [AC_DEFINE(NODLL, [], [Define NODLL when not load libpcap dynamicly])
       AS_CASE([$with_pcap],
               [dynamic], [AC_CHECK_LIB([pcap], [pcap_sendpacket])],
               [static], [AC_SUBST(LDADD, '$(top_builddir)'/other/libpcap.a)],
               [AC_SUBST(LDADD, "$with_pcap")])])

AC_ARG_ENABLE([notify], [AC_HELP_STRING([--disable-notify], [Don't show notification])],
              [], [enable_notify="yes"])
AS_IF([test "x$enable_notify" = "xno"],
      [AC_DEFINE(NONOTIFY, [], [Define NONOTIFY to disable notification])],
      [AC_CHECK_LIB([dl], [dlsym])])

# Checks for header files.
AC_CHECK_HEADERS([iconv.h arpa/inet.h fcntl.h netinet/in.h stdint.h stdlib.h string.h sys/ioctl.h sys/socket.h sys/time.h unistd.h])
AC_CHECK_HEADER([pcap.h], [], [AC_SUBST(AM_CFLAGS, -I'$(top_builddir)'/other)])

# Checks for typedefs, structures, and compiler characteristics.
AC_TYPE_SIZE_T
AC_TYPE_UINT32_T

# Checks for library functions.
AC_CHECK_FUNCS([atexit memmove memset socket strrchr])

AC_CONFIG_FILES([Makefile src/Makefile ])

AC_MSG_RESULT([config result: host_os($host_os) with_pcap($with_pcap) enable_notify($enable_notify)])
AC_OUTPUT
