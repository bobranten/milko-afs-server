# milko/lib/msecurity/Makefile.am

include $(top_srcdir)/Makefile.am.common

noinst_LTLIBRARIES = libmsecurity.la

libmsecurity_la_SOURCES = acl_files.c msecurity.c netinit.c

AM_CPPFLAGS += \
	-I$(srcdir)/ \
	-I$(top_srcdir)/include \
	-I$(top_srcdir)/rxdef \
	-I$(top_srcdir)/rxkad \
	-I$(top_srcdir)/ \
	-DMILKO_SYSCONFDIR=\"$(sysconfdir)\"
