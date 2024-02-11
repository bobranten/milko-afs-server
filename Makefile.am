# Makefile.am

include $(top_srcdir)/Makefile.am.common

if RXKAD
rxkad = rxkad
endif

SUBDIRS		= \
		include \
		lib \
		lwp \
		ydr \
		rxdef \
		lib/ko \
		rx \
		$(rxkad) \
		lib/arla \
		milko \
		appl \
		conf

ACLOCAL_AMFLAGS = -I cf

EXTRA_DIST = Makefile.am.common

DISTCLEANFILES = configure~
