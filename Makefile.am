ACLOCAL_AMFLAGS = -I m4
AM_CPPFLAGS = -I$(top_builddir)/lib -I$(top_srcdir)/lib -I$(top_srcdir)
LDADD = lib/libgnu.a
SUBDIRS = lib tests
bin_PROGRAMS = lctp lctp_helper
lib_LIBRARIES = liblctp.a
include_HEADERS = lctp.h
pkgconfig_DATA = lctp.pc
lctp_SOURCES = lctp.c
lctp_LDADD = liblctp.a lib/libgnu.a
lctp_helper_SOURCES = lctp_helper.c
lctp_helper_LDADD = liblctp.a lib/libgnu.a
liblctp_a_SOURCES = liblctp.c datetime.c datetime.h chomp.c chomp.h lctp.h lctp_atol.h
EXTRA_DIST = m4/gnulib-cache.m4 lctp.pc.in
