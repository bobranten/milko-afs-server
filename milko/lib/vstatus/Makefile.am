# milko/lib/vstatus/Makefile.am

include $(top_srcdir)/Makefile.am.common

noinst_LTLIBRARIES = libvstatus.la

libvstatus_la_SOURCES = vstatus.c vstat.ydr.c

AM_CPPFLAGS += \
	-I$(srcdir)/ \
	-I$(top_srcdir)/include \
	-I$(top_srcdir)/

BUILT_SOURCES = vstat.h vstat.ydr.c

YDR		= $(top_srcdir)/ydr/ydr$(EXEEXT)
YDRFLAGS	= -I$(srcdir)

vstat.h vstat.ydr.c: vstat.xg
	$(YDR) $(YDRFLAGS) $(srcdir)/vstat.xg

CLEANFILES = vstat.cs.c vstat.cs.h vstat.ss.c vstat.ss.h vstat.ydr.c vstat.h
