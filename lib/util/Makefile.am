# $Id: Makefile.am,v 1.3 2006/11/13 17:17:31 tol Exp $

include $(top_srcdir)/Makefile.am.common

AM_CPPFLAGS += -I$(top_srcdir)/include

noinst_LTLIBRARIES = libarlautil.la

#libarlautil_la_LDFLAGS = -version-info 1:0:0

noinst_PROGRAMS = util-tester heaptest

#man_MANS = log_log.3

libarlautil_la_SOURCES = \
	hash.c list.c log.c heap.c \
	timeval.c localtime_r.c arlamath.c sha.c

util_tester_SOURCES = util-tester.c

heaptest_SOURCES = heaptest.c

util_tester_LDADD = libarlautil.la $(LIB_roken)

heaptest_LDADD = libarlautil.la $(LIB_roken)

noinst_HEADERS	= arlamath.h bool.h hash.h heap.h list.h log.h sha.h
