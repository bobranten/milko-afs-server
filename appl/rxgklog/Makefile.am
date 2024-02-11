
include $(top_srcdir)/Makefile.am.common

bin_PROGRAMS = rxgklog

rxgklog_CPPFLAGS = \
	-I$(top_srcdir)/appl/arlalib \
	-I$(top_srcdir)/include \
	-I$(top_srcdir) \
	-I$(top_builddir)

LDADD = \
	$(top_builddir)/rxgk/librxgkclient.la \
	$(top_builddir)/lib/ko/libko.la \
	$(top_builddir)/lib/util/libarlautil.la \
	$(top_builddir)/lib/vers/libvers.la \
	$(LIB_roken) \
	$(KAFS_LIBS) \
	$(LIBS) \
	$(top_builddir)/rx/librx.la \
	-L$(top_builddir)/lwp -llwp \
	$(PLWP_LIB_FLAGS) \
	-lgssapi_krb5
