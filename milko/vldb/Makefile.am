# milko/vldb/Makefile.am

include $(top_srcdir)/Makefile.am.common

libexec_PROGRAMS = vldbserver

vldbserver_SOURCES = \
	ubikprocs.c \
	vl_db.c \
	vldbserver.c \
	vl_disk.c

vldbserver_CPPFLAGS = \
	-I../lib/mdb \
	-I../lib/mlog \
	-I../lib/msecurity \
	-I$(top_srcdir)/include \
	-I$(top_srcdir)/rxdef \
	-I$(top_srcdir)/rxkad \
	-I$(top_srcdir)/ \
	$(KRB5_INC_FLAGS) \
	-DMILKO_LIBEXECDIR=\"$(libexecdir)\" \
	-DMILKO_SYSCONFDIR=\"$(sysconfdir)\"

vldbserver_LDADD = \
	-L../lib/mdb -lmdb \
	-L../lib/mlog -lmlog \
	-L../lib/msecurity -lmsecurity \
	-L$(top_srcdir)/rxdef -lrxvlserver \
	-L$(top_srcdir)/rx -lrx \
	-L$(top_srcdir)/lib/ko -lko -L$(top_srcdir)/lib/util -larlautil \
	-L$(top_srcdir)/lwp -llwp $(PLWP_LIB_FLAGS) \
	$(LIB_roken) \
	@DBLIB@ \
	@LIBS@
