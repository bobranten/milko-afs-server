#
# $Id: Makefile.am,v 1.5 2004/02/04 12:06:31 lha Exp $
#

include $(top_srcdir)/Makefile.am.common

YFLAGS = -d

AM_CPPFLAGS += \
	-I$(srcdir) \
	 -I$(srcdir)/../lib/util \
	-I$(srcdir)/../include \
	$(INC_roken)

noinst_PROGRAMS = ydr

ydr_CFLAGS = -Wno-enum-compare -Wno-unused-function -DCPP="\"$(CPP) $(CPPFLAGS) $(YDR_CPPFLAGS)\""

ydr_SOURCES = \
	parse.y \
	lex.l \
	lex.h \
	main.c \
	symbol.c \
	types.c \
	types.h \
	output.c \
	output.h \
	sym.h

LDADD = \
	../lib/util/libarlautil.la 	\
	$(LIB_roken)			\
	$(LIBS)

CLEANFILES = lex.c parse.c parse.h
