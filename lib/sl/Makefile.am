# $Id: Makefile.am,v 1.5 2005/11/22 15:58:15 tol Exp $

include $(top_srcdir)/Makefile.am.common

if do_roken_rename
ES = strtok_r.c snprintf.c strdup.c strupr.c getprogname.c
endif

AM_CPPFLAGS += $(ROKEN_RENAME) $(COMERR_CPPFLAGS) $(INC_roken)

YFLAGS = -d

noinst_HEADERS = sl.h ss.h

noinst_LTLIBRARIES = libsl.la libss.la
#libsl_la_LDFLAGS = -version-info 1:2:1
#libss_la_LDFLAGS = -version-info 1:4:1

libsl_la_LIBADD = @LIB_readline@
libss_la_LIBADD = @LIB_readline@ @LIB_com_err@

libsl_la_SOURCES = sl_locl.h sl.c $(ES)
libss_la_SOURCES = $(libsl_la_SOURCES) ss.c ss.h

noinst_PROGRAMS = mk_cmds

mk_cmds_SOURCES = make_cmds.c make_cmds.h parse.y lex.l
mk_cmds_LDADD = libsl.la $(LDADD)

CLEANFILES = mk_cmds lex.c parse.c parse.h snprintf.c strtok_r.c strdup.c strupr.c getprogname.c

$(mk_cmds_OBJECTS): parse.h parse.c

LDADD =						\
	$(LIB_roken)				\
	$(LEXLIB)

strtok_r.c:
	$(LN_S) $(srcdir)/../roken/strtok_r.c .
snprintf.c:
	$(LN_S) $(srcdir)/../roken/snprintf.c .
strdup.c:
	$(LN_S) $(srcdir)/../roken/strdup.c .
strupr.c:
	$(LN_S) $(srcdir)/../roken/strupr.c .
getprogname.c:
	$(LN_S) $(srcdir)/../roken/getprogname.c .
