#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.63])
AC_INIT([mentohust], [0.3.1], [http://code.google.com/p/mentohust/issues/list])
AC_CANONICAL_HOST
AM_INIT_AUTOMAKE
AC_CONFIG_SRCDIR([src/myconfig.h])
AC_CONFIG_HEADERS([config.h])

m4_ifdef([AM_SILENT_RULES], [AM_SILENT_RULES([yes])])

# Checks for programs.
AS_IF([test "x$prefix" = "xNONE"], [prefix="/usr"])
AC_ARG_ENABLE([debug], [AC_HELP_STRING([ --enable-debug], [Build a debug version])],
              [CFLAGS="$CFLAGS -g -O0 -DDEBUG -Wall"],
              [CFLAGS="$CFLAGS -Os -Wall"])

AC_PROG_CC
AC_C_BIGENDIAN

AC_ARG_ENABLE([encodepass], [AC_HELP_STRING([ --disable-encodepass], [Don't encode password])],
              [], [enable_encodepass="yes"])
AS_IF([test "x$enable_encodepass" = "xno"],
      [AC_DEFINE(NO_ENCODE_PASS, [], [Define NO_ENCODE_PASS to disable encoding password])])

AC_ARG_ENABLE([arp], [AC_HELP_STRING([ --disable-arp], [Don't care ARP info])],
              [], [enable_arp="yes"])
AS_IF([test "x$enable_arp" = "xno"],
      [AC_DEFINE(NO_ARP, [], [Define NO_ARP to disable careing ARP])])

# Checks for libraries.
AS_CASE([$host_os],
        [*darwin*],
        [AC_DEFINE(MAC_OS, [], [Define MAC_OS when compiling for Mac OS])
         AC_CHECK_LIB([iconv], [iconv])])

AC_ARG_WITH([pcap], [AC_HELP_STRING([ --with-pcap=TYPE], [How to link libpcap: dyload(Default) | dylib | stlib | PATH])],
            [], [with_pcap="dyload"])
AS_IF([test "x$with_pcap" = "xyes" || test -z "$with_pcap"], [with_pcap="dyload"])
AS_IF([test "$with_pcap" = "dyload"], [AC_CHECK_LIB([dl], [dlsym])],
      [AC_DEFINE(NO_DYLOAD, [], [Define NO_DYLOAD when not load libpcap dynamicly])
       AS_CASE([$with_pcap],
               [dylib], [AC_CHECK_LIB([pcap], [pcap_sendpacket])],
               [stlib], [AC_SUBST(LDADD, libpcap.a)],
               [AC_SUBST(LDADD, "$with_pcap")])])

AC_ARG_ENABLE([notify], [AC_HELP_STRING([ --disable-notify], [Don't show notification])],
              [], [enable_notify="yes"])
AS_IF([test "x$enable_notify" = "xno"],
      [AC_DEFINE(NO_NOTIFY, [], [Define NO_NOTIFY to disable notification])],
      [AC_CHECK_LIB([dl], [dlsym])])

AC_ARG_ENABLE([logout], [AC_HELP_STRING([ --disable-logout], [Don't logout when exiting])],
              [], [enable_logout="yes"])
AS_IF([test "x$enable_logout" = "xno"],
      [AC_DEFINE(NO_LOGOUT, [], [Define NO_LOGOUT to disable sending exiting packet])])

AS_IF([test -z "$enable_nls"], [enable_nls="yes"])

AC_ARG_ENABLE([getopt-long], [AC_HELP_STRING([ --disable-getopt-long], [Don't use long options (and getopt_log())])],
              [], [enable_getopt_long="yes"])
AS_IF([test "x$enable_getopt_long" = "xno"],
      [AC_DEFINE(NO_GETOPT_LONG, [], [Define NO_ETOPT_LONG to disable using of getopt_long() to parse options])])

GETTEXT_PACKAGE="$PACKAGE_NAME"
AC_SUBST(GETTEXT_PACKAGE)
AC_DEFINE_UNQUOTED(GETTEXT_PACKAGE, "$GETTEXT_PACKAGE", [Package name for gettext.])
AM_GNU_GETTEXT([external])
AM_GNU_GETTEXT_VERSION(0.16.1)

# Checks for header files.
AC_CHECK_HEADERS([iconv.h arpa/inet.h fcntl.h netinet/in.h stdint.h stdlib.h string.h sys/ioctl.h sys/socket.h sys/time.h unistd.h libintl.h locale.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_TYPE_SIZE_T

# Checks for library functions.
AC_CHECK_FUNCS([atexit memmove memset socket strrchr setlocale])

AC_CONFIG_FILES([Makefile src/Makefile po/Makefile.in mentohust.spec])

AC_MSG_RESULT([config result: host($host_os) pcap($with_pcap) notify($enable_notify) encode($enable_encodepass) arp($enable_arp) nls($enable_nls) getopt_long($enable_getopt_long)])
AC_OUTPUT
