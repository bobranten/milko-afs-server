# $Id: Makefile.am,v 1.3 2005/03/15 21:47:39 lha Exp $

include $(top_srcdir)/Makefile.am.common

bin_PROGRAMS = afsfsperf

afsfsperf_SOURCES = afsfsperf.c

afsfsperf_CPPFLAGS = \
	-I$(top_srcdir)/include \
	-I$(top_srcdir)/appl/arlalib \
	-I$(srcdir)/../.. \
	-I../../include \
	$(INC_sl) \
	-I../../rxdef \
	$(KAFS_CPPFLAGS) \
	$(INC_roken) \
	-I.

afsfsperf_LDADD = \
	$(LIB_sl) \
	$(LIB_readline) \
	../../rx/librx.la \
	-L../../lwp -llwp $(PLWP_LIB_FLAGS) \
	-L../../appl/arlalib -larlalib \
	../../rxdef/librxdefclient.la \
	../../rxdef/librxcbserver.la \
	../../lib/ko/libko.la \
	../../lib/vers/libvers.la \
	../../lib/util/libarlautil.la \
	$(LIB_roken) \
	$(KAFS_LIBS) \
	$(RXKAD_LIBS) \
	$(LIBS)
