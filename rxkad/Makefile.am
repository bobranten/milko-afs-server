#
# $Id: Makefile.am,v 1.6 2005/01/17 10:02:38 lha Exp $
#

include $(top_srcdir)/Makefile.am.common

noinst_LTLIBRARIES = librxkad.la

AM_CPPFLAGS += -I. 			\
	-I$(srcdir)			\
	-I../include			\
	-I$(srcdir)/../include		\
	-I.. -I$(srcdir)/..		\
	$(INC_roken) 			\
	$(KRB5_INC_FLAGS)		\
	-Wno-shift-overflow		\
	-Wno-deprecated-declarations

KRB4_COMPAT_SOURCES = 			\
	decomp_ticket.c			\
	lifetime.c			\
	rw.c

librxkad_la_SOURCES =			\
	rxk_locl.c			\
	rxk_clnt.c			\
	rxk_crpt.c			\
	osi_alloc.c			\
	rxk_info.c			\
	rxk_serv.c			\
	$(KRB4_COMPAT_SOURCES)

LDADD = \
	$(KRB5_LIB_FLAGS)

#rxincludedir = $(includedir)/rxkad
noinst_HEADERS = rxkad.h
