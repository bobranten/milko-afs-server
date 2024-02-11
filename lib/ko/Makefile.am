#
# $Id: Makefile.am,v 1.10 2007/01/11 13:11:53 tol Exp $
#

include $(top_srcdir)/Makefile.am.common

AM_CPPFLAGS += -I. 			\
	-I$(srcdir)			\
	-I$(srcdir)/../../include	\
	-I../.. -I$(srcdir)/../..	\
	-I../../rxdef			\
	$(INC_roken)

libko_la_CFLAGS = \
	-DARLACACHEDIR=$(ARLACACHEDIR) \
	-DARLACONFFILE=$(ARLACONFFILE) \
	-DSYSCONFDIR=\"$(sysconfdir)\" \
	-DARLAOS=\"$(target_os)\" \
	-DARLAVENDOR=\"$(target_vendor)\" \
	-DARLACPU=\"$(target_cpu)\"

noinst_LTLIBRARIES = libko.la

noinst_PROGRAMS = sysnametest kotest restest

libko_la_SOURCES = \
	afs_uuid.c \
	afsconf.c \
	agetarg.c \
	misc.c \
	koerror.c \
	kocell.c \
	ports.c \
	part.c \
	slcompat.c \
	gensysname.c \
	resolve.c \
	vlmisc.c \
	uae.c

noinst_HEADERS =	 			\
	afs_uuid.h				\
	agetarg.h				\
	cellconfig.h				\
	ko.h					\
	part.h					\
	ports.h					\
	uae.h

kotest_SOURCES = kotest.c

sysnametest_SOURCES = sysnametest.c

restest_SOURCES = restest.c

LDADD = \
	../../lib/util/libarlautil.la 	\
	$(LIB_roken)			\
	$(KRB5_LIB_FLAGS)		\
	$(LIBS)

kotest_LDADD = libko.la $(LDADD)

sysnametest_LDADD = libko.la ../vers/libvers.la $(LDADD)

restest_LDADD = libko.la $(LDADD)
