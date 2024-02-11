#
# $Id: Makefile.am,v 1.9 2007/05/14 20:21:21 lha Exp $
#

include $(top_srcdir)/Makefile.am.common

# For now, we cheat
COMPILE_ET = compile_et

noinst_LTLIBRARIES = librxgkclient.la librxgkserver.la librxgkcm.la

noinst_PROGRAMS = test_client test_server test_ticket

libexec_PROGRAMS = rxgk-service
#bin_PROGRAMS = rxgk-settoken

nodist_include_HEADERS = rxgk_err.h

ERR_FILES = rxgk_err.c

EXTRA_DIST = rxgk_err.et

CPPFLAGS = -Wno-int-conversion -Wno-misleading-indentation -DAFS_HCRYPTO

AM_CPPFLAGS += -I. 			\
	-I$(srcdir)			\
	-I../include			\
	-I$(srcdir)/../include		\
	-I../rxdef			\
	-I.. -I$(srcdir)/..		\
	$(INC_roken) 			\
	$(KRB5_INC_FLAGS)

librxgkcm_la_SOURCES =			\
	rxgk_locl.h			\
	rxgk_proto.h			\
	rxgk_clnt.c			\
	rxgk_info.c			\
	rxgk_hcrypto.c			\
	rxgk_proto.ydr.c		\
	rxgk_proto.cs.c

librxgkclient_la_SOURCES =		\
	rxgk_locl.h			\
	rxgk_proto.h			\
	rxgk_clnt.c			\
	rxgk_info.c			\
	rxgk_crpc.c			\
	rxgk_hcrypto.c			\
	rxgk_common.c			\
	rxgk_proto.ydr.c		\
	rxgk_proto.cs.c

librxgkserver_la_SOURCES =		\
	rxgk_locl.h			\
	rxgk_proto.h			\
	rxgk_serv.c			\
	rxgk_info.c			\
	rxgk_srpc.c			\
	rxgk_hcrypto.c			\
	rxgk_common.c			\
	rxgk_proto.ydr.c		\
	rxgk_ticket.c			\
	rxgk_proto.ss.c

LIBYDR = 				\
	rxgk_proto.cs.c 		\
	rxgk_proto.ss.c 		\
	rxgk_proto.ydr.c		\
	rxgk_proto.h			\
	rxgk_proto.cs.h			\
	rxgk_proto.ss.h

LIBTYDR = 				\
	test.cs.h			\
	test.ss.h			\
	test.ss.c			\
	test.cs.c			\
	test.ydr.c			\
	test.h

common_LDADD = \
	../rx/librx.la \
	-L../lwp -llwp $(PLWP_LIB_FLAGS) \
	-lgssapi_krb5 $(KRB5_LIB_FLAGS)

test_client_SOURCES = test_client.c test.cs.c
test_server_SOURCES = test_server.c test.ss.c
test_ticket_SOURCES = test_ticket.c
rxgk_service_SOURCES = rxgk-service.c
#rxgk_settoken_SOURCES = rxgk-settoken.c

test_client_LDADD = librxgkclient.la $(common_LDADD) ../lib/roken/libroken.la
test_server_LDADD = librxgkserver.la $(common_LDADD) ../lib/roken/libroken.la
test_ticket_LDADD = librxgkserver.la $(common_LDADD) ../lib/roken/libroken.la
rxgk_service_LDADD = librxgkserver.la $(common_LDADD) ../lib/roken/libroken.la
#rxgk_settoken_LDADD = librxgkclient.la $(common_LDADD) \
#	../lib/arla/libarlalib.la \
#	../lib/util/libarlautil.la \
#	../lib/vers/libvers.la \
#	../lib/roken/libroken.la \
#	../rxdef/libtoken.la $(KAFS_LIBS)

$(librxgkclient_la_OBJECTS) $(librxgkserver_la_OBJECTS): $(LIBYDR) rxgk_err.h

$(test_server_OBJECTS) $(test_client_OBJECTS) $(srcdir)/rxgk.h: $(LIBTYDR)

$(LIBYDR): rxgk_proto.xg
	../ydr/ydr $(srcdir)/rxgk_proto.xg

$(LIBTYDR): test.xg
	../ydr/ydr $(srcdir)/test.xg

LDADD = $(KRB5_LIB_FLAGS)

# for evil make
rxgk_err.h: rxgk_err.et

#
# tests
#

noinst_DATA = krb5.conf

check_SCRIPTS = $(SCRIPT_TESTS) 

SCRIPT_TESTS = test_gss
TESTS = test_ticket $(SCRIPT_TESTS) 

do_subst = sed -e 's,[@]srcdir[@],$(srcdir),g' \
	-e 's,[@]objdir[@],$(top_builddir)/rxgk,g'

test_gss: test_gss.in Makefile
	$(do_subst) < $(srcdir)/test_gss.in > test_gss.tmp
	chmod +x test_gss.tmp
	mv test_gss.tmp test_gss

krb5.conf: krb5.conf.in Makefile
	$(do_subst) < $(srcdir)/krb5.conf.in > krb5.conf.tmp
	mv krb5.conf.tmp krb5.conf

ERRFILES = rxgk_err.c rxgk_err.h

CLEANFILES = \
	$(LIBYDR) \
	$(LIBTYDR) \
	$(nodist_include_HEADERS) \
	$(ERRFILES) \
	test_gss \
	krb5.conf \
	client-log \
	server-log \
	cache.krb5
