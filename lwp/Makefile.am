# lwp/Makefile.am

include $(top_srcdir)/Makefile.am.common

noinst_LTLIBRARIES = liblwp.la

noinst_PROGRAMS = testlwp testrw

liblwp_la_SOURCES = fasttime.c iomgr.c lock.c plwp.c preempt.c q.c timer.c

AM_CPPFLAGS += \
	-I$(top_srcdir)/lib/roken \
	-I$(srcdir)/ \
	-I$(top_srcdir)/include \
	-I$(top_srcdir)/ \
	-DDEBUG -DFD_SPEED_HACK -DPTHREADS_LWP

testlwp_LDADD = liblwp.la $(LIB_roken)
testrw_LDADD = liblwp.la $(LIB_roken)
