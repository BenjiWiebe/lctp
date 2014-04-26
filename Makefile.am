ACLOCAL_AMFLAGS = -I m4
AM_CPPFLAGS = -I$(top_builddir)/lib -I$(top_srcdir)/lib
LDADD = lib/libgnu.a
SUBDIRS = lib
bin_PROGRAMS = lctp lctp_helper
lctp_SOURCES = lctp.c datetime.c datetime.h chomp.c chomp.h
lctp_helper_SOURCES = lctp_helper.c
EXTRA_DIST = m4/gnulib-cache.m4
