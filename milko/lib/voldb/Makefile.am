# milko/lib/voldb/Makefile.am

include $(top_srcdir)/Makefile.am.common

noinst_LTLIBRARIES = libvoldb.la

libvoldb_la_SOURCES = vdb_flat.c vol.c voldb.c

AM_CPPFLAGS += \
	-I../dpart \
	-I../vstatus \
	-I../../fs \
	-I$(srcdir)/ \
	-I$(top_srcdir)/include \
	-I$(top_srcdir)/rxdef \
	-I$(top_srcdir)/
