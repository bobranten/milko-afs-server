# milko/appl/sked/Makefile.am

include $(top_srcdir)/Makefile.am.common

bin_PROGRAMS = sked

sked_SOURCES = sked.c

sked_CPPFLAGS = \
	-I../../fs \
	-I../../lib/dpart \
	-I../../lib/mlog \
	-I../../lib/msecurity \
	-I../../lib/ropa \
	-I../../lib/vld \
	-I../../lib/voldb \
	-I../../lib/vstatus \
	-I$(top_srcdir)/include \
	-I$(top_srcdir)/rxdef \
	-I$(top_srcdir)/

sked_LDADD = \
	-L../../lib/dpart -ldpart \
	-L../../lib/mlog -lmlog \
	-L../../lib/msecurity -lmsecurity \
	-L../../lib/vld -lvld \
	-L../../lib/voldb -lvoldb \
	-L../../lib/vstatus -lvstatus \
	-L$(top_srcdir)/lib/bufdir -lbufdir \
	-L$(top_srcdir)/rxdef -lrxvolserver \
	-L$(top_srcdir)/rx -lrx \
	-L$(top_srcdir)/lib/editline -leditline \
	-L$(top_srcdir)/lib/sl -lsl \
	-L$(top_srcdir)/lib/ko -lko -L$(top_srcdir)/lib/util -larlautil \
	-L$(top_srcdir)/lwp -llwp $(PLWP_LIB_FLAGS) \
	$(LIB_roken) \
	@LIBS@
