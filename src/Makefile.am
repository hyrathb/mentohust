bin_PROGRAMS = mentohust

EXTRA_DIST = $(top_srcdir)/other/pcap.h $(top_srcdir)/other/pcap-bpf.h

sysconfdir = /etc

dist_sysconf_DATA = mentohust.conf

mentohust_SOURCES = myini.c md5.c mycheck.c dlfunc.c myfunc.c mystate.c myconfig.c mentohust.c

mentohust_SOURCES += types.h myini.h md5.h mycheck.h dlfunc.h myfunc.h mystate.h myconfig.h

mentohust_LDADD = $(LDADD)

