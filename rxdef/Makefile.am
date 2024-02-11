# $Id: Makefile.am,v 1.2 2007/05/12 01:00:33 lha Exp $

include $(top_srcdir)/Makefile.am.common

noinst_LTLIBRARIES = \
	librxfsserver.la \
	librxcbserver.la \
	librxcbclient.la \
	librxvlserver.la \
	librxvolserver.la \
	libptserver.la \
	libptclient.la \
	libbosserver.la \
	libbosclient.la \
	libkaclient.la \
	libkaserver.la \
	librxdefclient.la \
	libtoken.la

librxdefclient_la_SOURCES = \
	vldb.cs.c \
	fs.cs.c \
	volumeserver.cs.c \
	pts.cs.c \
	ubik.cs.c \
	bos.cs.c \
	vldb.cs.h vldb.h \
	fs.cs.h fs.h \
	volumeserver.cs.h volumeserver.h \
	pts.cs.h pts.h \
	ubik.cs.h ubik.h \
	bos.cs.h bos.h

librxcbserver_la_SOURCES = \
	cb.ss.c cb.ydr.c cb.h cb.ss.h

librxcbclient_la_SOURCES = \
	cb.cs.c cb.ydr.c cb.h cb.cs.h

librxfsserver_la_SOURCES = \
	fs.ss.c fs.ydr.c cb.cs.c pts.cs.c ubik.cs.c \
	fs.ss.h cb.cs.h pts.cs.h ubik.cs.h \
	fs.h cb.h pts.h ubik.h

librxvlserver_la_SOURCES = \
	vldb.ss.c vldb.ydr.c ubik.ss.c ubik.ydr.c \
	vldb.ss.h vldb.ydr.h ubik.ss.h vldb.h ubik.h

librxvolserver_la_SOURCES = \
	volumeserver.ss.c volumeserver.ydr.c \
	volumeserver.ss.h volumeserver.ydr.h volumeserver.h

libptserver_la_SOURCES = \
	pts.ydr.c pts.ss.c \
	pts.ydr.h pts.ss.h pts.h

libptclient_la_SOURCES = \
	pts.ydr.c pts.cs.c \
	pts.ydr.h pts.cs.h pts.h

libbosserver_la_SOURCES = \
	bos.ydr.c bos.ss.c \
	bos.ydr.h bos.ss.h bos.h

libbosclient_la_SOURCES = \
	bos.ydr.c bos.cs.c \
	bos.ydr.h bos.cs.h bos.h

libkaserver_la_SOURCES = \
	ka.ydr.c ka.ss.c \
	ka.ydr.h ka.ss.h ka.h

libkaclient_la_SOURCES = \
	ka.ydr.c ka.cs.c \
	ka.ydr.h ka.cs.h ka.h

libtoken_la_SOURCES = \
	token.ydr.c token.ydr.h token.h

YDR		= ../ydr/ydr$(EXEEXT)
YDRFLAGS	= -I$(srcdir)

AM_CPPFLAGS	+= -I$(top_srcdir)/include \
		  @INC_roken@ \
		  -I$(srcdir)/..

REALCFLAGS	= $(INCLUDES) $(DEFINES) $(CFLAGS)

XGFILES		= $(srcdir)/vldb.xg \
		  $(srcdir)/fs.xg \
		  $(srcdir)/cb.xg \
		  $(srcdir)/pts.xg \
		  $(srcdir)/bos.xg \
		  $(srcdir)/volumeserver.xg 

ydr:		$(XGFILES)
		../ydr/ydr $< 

vldb.cs.c vldb.cs.h vldb.ss.c vldb.ss.h vldb.ydr.c vldb.h: vldb.xg fs.h common.h
		$(YDR) $(YDRFLAGS) $(srcdir)/vldb.xg

fs.cs.c fs.cs.h fs.ss.c fs.ss.h fs.ydr.c fs.h: fs.xg common.h
		$(YDR) $(YDRFLAGS) $(srcdir)/fs.xg

cb.cs.c cb.cs.h cb.ss.c cb.ss.h cb.ydr.c cb.h: cb.xg fs.h common.h
		$(YDR) $(YDRFLAGS) $(srcdir)/cb.xg

pts.cs.c pts.cs.h pts.ss.c pts.ss.h pts.ydr.c pts.h: pts.xg
		$(YDR) $(YDRFLAGS) $(srcdir)/pts.xg

bos.cs.c bos.cs.h bos.ss.c bos.ss.h bos.ydr.c bos.h: bos.xg
		$(YDR) $(YDRFLAGS) $(srcdir)/bos.xg

ka.cs.c ka.cs.h ka.ss.c ka.ss.h ka.ydr.c ka.h: ka.xg
		$(YDR) $(YDRFLAGS) $(srcdir)/ka.xg

volumeserver.cs.c volumeserver.cs.h volumeserver.ss.c volumeserver.ss.h volumeserver.ydr.c volumeserver.h: volumeserver.xg
		$(YDR) $(YDRFLAGS) $(srcdir)/volumeserver.xg

ubik.cs.c ubik.cs.h ubik.ss.c ubik.ss.h ubik.ydr.c ubik.h: ubik.xg
		$(YDR) $(YDRFLAGS) $(srcdir)/ubik.xg

token.cs.c token.cs.h token.ss.c token.ss.h token.ydr.c token.h: token.xg
		$(YDR) $(YDRFLAGS) $(srcdir)/token.xg

CLEANFILES = \
	$(librxdefclient_la_SOURCES) \
	$(librxcbserver_la_SOURCES) \
	$(librxcbclient_la_SOURCES) \
	$(librxfsserver_la_SOURCES) \
	$(librxvlserver_la_SOURCES) \
	$(librxvolserver_la_SOURCES) \
	$(libptserver_la_SOURCES) \
	$(libptclient_la_SOURCES) \
	$(libbosserver_la_SOURCES) \
	$(libbosclient_la_SOURCES) \
	$(libkaserver_la_SOURCES) \
	$(libkaclient_la_SOURCES) \
	$(libtoken_la_SOURCES) \
	token.cs.c token.cs.h token.ss.c token.ss.h
