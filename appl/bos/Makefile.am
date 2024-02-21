# $Id: Makefile.am,v 1.3 2005/03/15 21:46:27 lha Exp $

include $(top_srcdir)/Makefile.am.common

bin_PROGRAMS = bos

man_MANS = bos.8

bos_SOURCES = \
	bos.c \
	bos_status.c \
	bos_adduser.c \
	bos_getrestart.c \
	bos_listhosts.c \
	bos_listusers.c \
	bos_stop.c \
	bos_start.c \
	bos_addhost.c \
	bos_removehost.c \
	bos_restart.c

bos_CPPFLAGS = \
	$(KAFS_CPPFLAGS) \
	$(KRB5_INC_FLAGS) \
	-I$(srcdir)/../.. \
	-I$(top_srcdir)/include \
	-I$(top_srcdir)/lib/arla \
	-I../../include \
	$(INC_sl) \
	-I../../rxdef \
	-I. \
	-I$(srcdir)/../../rxkad \
	$(INC_roken)

bos_LDADD = \
	../../lib/arla/libarlalib.la \
	../../rxdef/librxdefclient.la \
	../../rx/librx.la \
	-L../../lwp -llwp $(PLWP_LIB_FLAGS) \
	../../lib/vers/libvers.la \
	../../lib/ko/libko.la ../../lib/util/libarlautil.la \
	$(LIB_sl) \
	$(READLINE_lib) \
	$(LIB_roken) \
	$(RXKAD_LIBS) \
	$(KAFS_LIBS) \
	$(RXKAD_LIBS) @LIBS@
