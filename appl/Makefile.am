# appl/Makefile.am

include $(top_srcdir)/Makefile.am.common

# This is the commands used to administer an AFS server
SUBDIRS=bos pts vos

# The other programs is to be used from a client and they are
# left from the Arla project so we don't build them as default
# but leave them here if anyone wants them. Also note that when
# you install an AFS client it will include its own versions of
# the standard commands like "fs".
#SUBDIRS+=arlalib afstool kalog perf rxgklog udebug

if X11
#SUBDIRS+=amon
endif

if RXKAD
#SUBDIRS+=fs afsutils
endif
