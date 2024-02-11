# $Id: Makefile.am,v 1.3 2006/11/13 17:17:31 tol Exp $

include $(top_srcdir)/Makefile.am.common

AM_CPPFLAGS += -I$(top_srcdir)/include

noinst_LTLIBRARIES = libeditline.la

noinst_PROGRAMS = testit

#man_MANS = editline.3

libeditline_la_SOURCES = complete.c editline.c sysunix.c

testit_SOURCES = testit.c

testit_LDADD = libeditline.la -lncurses
