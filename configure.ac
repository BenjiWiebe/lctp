#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.69])
AC_INIT([lctp], [dev], [benjiwiebe14@gmail.com])
AM_INIT_AUTOMAKE([-Wall -Werror foreign])
AM_SILENT_RULES([yes])
AC_CONFIG_MACRO_DIR([m4])
AC_CONFIG_SRCDIR([lctp.c])
AC_CONFIG_HEADERS([config.h])

# Checks for programs.
PKG_PROG_PKG_CONFIG
m4_ifdef([PKG_INSTALLDIR], [PKG_INSTALLDIR], AC_SUBST([pkgconfigdir], ${libdir}/pkgconfig))
AC_PROG_CC
AC_PROG_CC_STDC
gl_EARLY
AM_PROG_AR
LT_INIT([disable-shared])
gl_INIT

# Checks for libraries.

# Checks for header files.
AC_STDC_HEADERS

# Checks for typedefs, structures, and compiler characteristics.
AC_TYPE_SIZE_T

# Checks for library functions.
AC_FUNC_MALLOC
AC_FUNC_MKTIME
AC_CHECK_FUNCS([memset strtol])

AC_CONFIG_FILES([Makefile lib/Makefile tests/Makefile lctp.pc])
AC_OUTPUT
