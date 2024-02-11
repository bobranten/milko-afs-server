# $Id: Makefile.am,v 1.5 2005/01/17 09:20:43 lha Exp $

include $(top_srcdir)/Makefile.am.common

AM_CPPFLAGS += -I$(top_srcdir)/include $(INC_roken)

noinst_LTLIBRARIES = librx.la
noinst_PROGRAMS = rxdebug rxperf test_rx_clock rx_trace

librx_la_SOURCES = \
	rx_clock.c				\
	rx_event.c				\
	rx_user.c				\
	rx.c					\
	rx_globs.c				\
	rx_null.c				\
	rx_misc.c				\
	rx_pkt.c				\
	rx_rdwr.c				\
	rx_trace.c				\
	rx_multi.c

librx_la_CPPFLAGS = $(DEFS) -DRXDEBUG -DSOFT_ACK

#rxincludedir = $(includedir)/rx
noinst_HEADERS =				\
	rx-new.h				\
	rx.h					\
	rx_clock.h				\
	rx_event.h				\
	rx_globs.h				\
	rx_kern.h				\
	rx_kmutex.h				\
	rx_locl.h				\
	rx_mach.h				\
	rx_misc.h				\
	rx_multi.h				\
	rx_null.h				\
	rx_pkt.h				\
	rx_queue.h				\
	rx_strm.h				\
	rx_trace.h				\
	rx_user.h				\
	rxgencon.h

man_MANS = rxdebug.1

test_rx_clock_SOURCES = test_rx_clock.c
test_rx_clock_LDADD = librx.la -L../lwp -llwp $(LIBS)

rxdebug_SOURCES = rxdebug.c
rxdebug_LDADD = librx.la -L../lwp -llwp $(PLWP_LIB_FLAGS) \
	../lib/ko/libko.la $(LIB_roken) $(LIBS)

rxperf_SOURCES = rxperf.c
rxperf_LDADD = librx.la $(LIBS) -L../lwp -llwp $(PLWP_LIB_FLAGS) \
	$(LIB_roken) $(LIBS)

rx_trace_SOURCES = rx_trace.c rx_dumptrace.c
rx_trace_LDADD = librx.la $(LIBS) -L../lwp -llwp $(PLWP_LIB_FLAGS) \
	$(LIB_roken) $(LIBS)
