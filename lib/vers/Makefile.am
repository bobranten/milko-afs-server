# $Id: Makefile.am,v 1.1 2003/01/17 02:43:09 lha Exp $

include $(top_srcdir)/Makefile.am.common

CLEANFILES		= make-print-version print_version.h

noinst_LTLIBRARIES	= libvers.la

build_HEADERZ		= vers.h

noinst_PROGRAMS		= make-print-version

libvers_la_SOURCES	= print_version.c

print_version.lo: print_version.h

print_version.h: make-print-version$(EXEEXT)
	./make-print-version$(EXEEXT) print_version.h

make-print-version.o: $(top_builddir)/include/version.h
