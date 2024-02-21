# milko/bos/Makefile.am

include $(top_srcdir)/Makefile.am.common

libexec_PROGRAMS = bosserver

bosserver_SOURCES = \
	bosprocs.c \
	bosserver.c \
	kconf.c

bosserver_CPPFLAGS = \
	-I../lib/msecurity \
	-I$(top_srcdir)/include \
	-I$(top_srcdir)/rxdef \
	-I$(top_srcdir)/ \
	$(KRB5_INC_FLAGS) \
	-DMILKO_LIBEXECDIR=\"$(libexecdir)\" \
	-DMILKO_SYSCONFDIR=\"$(sysconfdir)\"

bosserver_LDADD = \
	-L../lib/msecurity -lmsecurity \
	-L$(top_srcdir)/rxdef -lbosserver -lrxvlserver \
	-L$(top_srcdir)/rxdef -lrxvolserver -lrxfsserver \
	-L$(top_srcdir)/rx -lrx \
	-L$(top_srcdir)/lib/ko -lko -L$(top_srcdir)/lib/util -larlautil \
	-L$(top_srcdir)/lwp -llwp $(PLWP_LIB_FLAGS) \
	$(LIB_roken) \
	@LIBS@
