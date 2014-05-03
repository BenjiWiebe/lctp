AM_TESTS_ENVIRONMENT = \
	BUILDDIR=$(top_builddir);
	SRCDIR=$(top_srcdir);
	export BUILDDIR;
	export SRCDIR;
	touch /tmp/FOO;
AM_CPPFLAGS = -I$(top_srcdir)
check_PROGRAMS = datetime1 datetime2 datetime3
datetime1_SOURCES = datetime1.c  $(top_srcdir)/datetime.c
datetime2_SOURCES = datetime2.c $(top_srcdir)/datetime.c
datetime3_SOURCES = datetime3.c $(top_srcdir)/datetime.c
TESTS = $(check_PROGRAMS) onehour.sh

onehour.sh: $(top_builddir)/tests/onehour.sh
	cp $(top_srcdir)/tests/onehour.sh $(top_builddir)/tests/onehour.sh
