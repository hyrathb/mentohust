
bin_PROGRAMS = MentoHUST

 
MentoHUST_SOURCES = mentohust.c myconfig.c mystate.c 
MentoHUST_SOURCES +=  myconfig.h mystate.h

MentoHUST_LDADD = -lconfig -lmd5 -L$(top_builddir)/other -L$(top_builddir)/lib

MentoHUST_CFLAGS = -I$(top_srcdir)/other -I$(top_srcdir)/lib
