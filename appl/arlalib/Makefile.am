# $Id: Makefile.am,v 1.7 2005/03/15 21:49:56 lha Exp $

include $(top_srcdir)/Makefile.am.common

noinst_LTLIBRARIES = libarlalib.la

AM_CPPFLAGS += \
	-I.					\
	-I$(srcdir)				\
	-I../../include				\
	-I$(srcdir)/../../include		\
	-I../.. -I$(srcdir)/../..		\
	-I../../rxdef				\
	$(INC_roken)

libarlalib_la_SOURCES = arlalib.c fs_lib.c ka-procs.c tokens.c
libarlalib_la_CPPFLAGS = -I../.. -I../../rxdef -DSYSCONFDIR=\"$(sysconfdir)\"

libarlalib_la_LIBADD = \
	../../rxdef/librxdefclient.la \
	../../rxdef/libkaclient.la \
	../../lib/ko/libko.la \
	../../lib/util/libarlautil.la \
	../../rx/librx.la \
	../../lib/vers/libvers.la \
	$(RXKAD_LIBS) \
	$(KAFS_LIBS) \
	$(LIB_sl) \
	$(LIB_roken)
