# $Id: Makefile.am,v 1.1 2003/03/06 16:01:42 lha Exp $

include $(top_srcdir)/Makefile.am.common

bin_SCRIPTS		= udebug

udebug: udebug.in
	sed -e "s!%bindir%!$(bindir)!" $(srcdir)/udebug.in > $@
	chmod +x $@

man_MANS = udebug.1

CLEANFILES = udebug

EXTRA_DIST = udebug.in $(man_MANS)
