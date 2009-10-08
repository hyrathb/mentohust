
bin_PROGRAMS = MentoHUST
 
MentoHUST_SOURCES = mentohust.c myconfig.c mystate.c mydata.c function.c 
MentoHUST_SOURCES +=  inireader.h myconfig.h

#MentoHUST_DEPENDENCIES = $(top_builddir)/lib/libconfig.a

MentoHUST_LDADD = -lcommon
MentoHUST_LDFLAGS =  -L$(top_builddir)/lib

MentoHUST_CFLAGS = -I$(top_srcdir)/lib
