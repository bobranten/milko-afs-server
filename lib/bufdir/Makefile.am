#
# $Id: Makefile.am,v 1.2 2005/08/09 15:53:10 lha Exp $
#

include $(top_srcdir)/Makefile.am.common

noinst_HEADERS = fdir.h afs_dir.h

AM_CPPFLAGS += \
	-I../../include -I$(srcdir) \
	-I../../rxdef \
	@INC_roken@ \
	-I$(srcdir)/../.. 

noinst_LTLIBRARIES = libbufdir.la
libbufdir_la_LDFLAGS = 

libbufdir_la_SOURCES = fbuf.c fdir.c

check_PROGRAMS = test_createutf

LDADD =	\
	libbufdir.la \
	../ko/libko.la \
	../../rx/librx.la \
	-L../../lwp -llwp \
	$(LIB_roken)
