ACLOCAL_AMFLAGS = -I m4
AM_CPPFLAGS = -I$(top_builddir)/lib -I$(top_srcdir)/lib
LDADD = lib/libgnu.a
SUBDIRS = lib
bin_PROGRAMS = read helper
read_SOURCES = read.c datetime.c datetime.h
helper_SOURCES = helper.c
EXTRA_DIST = m4/gnulib-cache.m4
