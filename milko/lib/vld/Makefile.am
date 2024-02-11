# milko/lib/vld/Makefile.am

include $(top_srcdir)/Makefile.am.common

noinst_LTLIBRARIES = libvld.la

libvld_la_SOURCES = common.c fvol.c glue.c mdir.c \
	mnode.c salvage.c svol.c vld.c

AM_CPPFLAGS += \
	-I../dpart \
	-I../mlog \
	-I../voldb \
	-I../vstatus \
	-I../../fs \
	-I$(srcdir)/ \
	-I$(top_srcdir)/include \
	-I$(top_srcdir)/rxdef \
	-I$(top_srcdir)/
