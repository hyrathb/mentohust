
bin_PROGRAMS = mentohust

EXTRA_DIST = mentohust.conf $(top_srcdir)/other/pcap.h $(top_srcdir)/other/pcap-bpf.h
 
mentohust_SOURCES = mentohust.c myconfig.c mystate.c myfunc.c dlfunc.c md5.c myini.c
mentohust_SOURCES +=  myconfig.h mystate.h myfunc.h dlfunc.h md5.h myini.h

mentohust_LDADD = $(LDADD)

