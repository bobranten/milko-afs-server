# $Id: Makefile.am,v 1.3 2005/03/15 21:47:28 lha Exp $

include $(top_srcdir)/Makefile.am.common

AM_CPPFLAGS += \
	-I$(top_srcdir)/include \
	-I$(top_srcdir)/lib/arla \
	-I../../rxdef \
	-I$(srcdir)/../..

bin_PROGRAMS = vos

man_MANS = vos.8

vos_SOURCES = \
	vos.c \
	vos_common.c \
	vos_examine.c \
	vos_listvol.c \
	vos_partinfo.c \
	vos_status.c \
	vos_createentry.c \
	vos_listpart.c \
	vos_syncsite.c \
	vos_listvldb.c \
	vos_dump.c \
	vos_createvolume.c \
	vos_endtrans.c \
	vos_vldbexamine.c \
	vos_lock.c \
	vos_unlock.c \
	vos_backup.c \
	vos_zap.c \
	vos_listaddrs.c \
	vos_addsite.c \
	vos_remsite.c \
	vos_rename.c

LDADD = \
	../../lib/arla/libarlalib.la \
	../../rxdef/librxdefclient.la \
	../../rx/librx.la \
	-L../../lwp -llwp $(PLWP_LIB_FLAGS) \
	../../lib/ko/libko.la ../../lib/util/libarlautil.la \
	../../lib/vers/libvers.la \
	$(LIB_sl) \
	$(READLINE_lib) \
	$(LIB_roken) \
	$(RXKAD_LIBS) \
	$(KAFS_LIBS) \
	$(RXKAD_LIBS) $(LIBS)
