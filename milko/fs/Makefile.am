# milko/fs/Makefile.am

include $(top_srcdir)/Makefile.am.common

libexec_PROGRAMS = fileserver

fileserver_SOURCES = \
	connsec.c \
	dump.c \
	fileserver.c \
	fsprocs.c \
	volprocs.c

fileserver_CPPFLAGS = \
	-I../lib/dpart \
	-I../lib/mlog \
	-I../lib/msecurity \
	-I../lib/ropa \
	-I../lib/vld \
	-I../lib/voldb \
	-I../lib/vstatus \
	-I../../lib/arla \
	-I$(top_srcdir)/include \
	-I$(top_srcdir)/rxdef \
	-I$(top_srcdir)/ \
	$(KRB5_INC_FLAGS) \
	-DMILKO_LIBEXECDIR=\"$(libexecdir)\" \
	-DMILKO_SYSCONFDIR=\"$(sysconfdir)\"

fileserver_LDADD = \
	-L../lib/dpart -ldpart \
	-L../lib/mlog -lmlog \
	-L../lib/msecurity -lmsecurity \
	-L../lib/ropa -lropa \
	-L../lib/vld -lvld \
	-L../lib/voldb -lvoldb \
	-L../lib/vstatus -lvstatus \
	-L$(top_srcdir)/lib/arla -larlalib \
	-L$(top_srcdir)/rxkad -lrxkad \
	-L$(top_srcdir)/rxdef -lrxvolserver -lrxfsserver \
	-L$(top_srcdir)/rx -lrx \
	-L$(top_srcdir)/lib/bufdir -lbufdir \
	-L$(top_srcdir)/lib/vers -lvers \
	-L$(top_srcdir)/lib/ko -lko -L$(top_srcdir)/lib/util -larlautil \
	-L$(top_srcdir)/lwp -llwp $(PLWP_LIB_FLAGS) \
	$(LIB_roken) \
	@DBLIB@ \
	@LIBS@
