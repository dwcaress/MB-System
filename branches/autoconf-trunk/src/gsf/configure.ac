#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.
# 
# This version of gsf is 2.08


AC_PREREQ([2.63])
AC_INIT([mb-gsf], [2.08], [dale@ldeo.columbia.edu])
AC_CONFIG_SRCDIR([dump_gsf.c])
AC_CONFIG_HEADERS([config.h])
AM_INIT_AUTOMAKE([foreign])

# Checks for programs.
AC_PROG_CC
AC_PROG_RANLIB

# Checks for libraries.

# Checks for header files.
AC_CHECK_HEADERS([float.h limits.h netinet/in.h stdlib.h string.h unistd.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_TYPE_SIZE_T

# Checks for library functions.
AC_FUNC_MKTIME
AC_FUNC_REALLOC
AC_CHECK_FUNCS([memset modf putenv strstr tzset])

AC_CONFIG_FILES([Makefile])
AC_OUTPUT
