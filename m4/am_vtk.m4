dnl ======================================================================================
dnl Author: Francesco Montorsi
dnl RCS-ID: $Id: vtk.m4,v 1.1 2005/11/20 14:47:40 frm Exp $
dnl
dnl Implements the AM_OPTIONS_VTK, to add the --with-vtk=path option, and the
dnl AM_PATH_VTK macro used to detect VTK presence, location and version.
dnl ======================================================================================



dnl
dnl  AM_OPTIONS_VTK
dnl  ------------------------------------------------------------------------
dnl  Adds the --with-vtk=path option to the configure options
dnl
AC_DEFUN([AM_OPTIONS_VTK],
[
  AC_ARG_WITH([vtk],
              [AC_HELP_STRING([--with-vtk],
              [The prefix where VTK is installed (default is /usr)])],
              [with_vtk=$withval],
              [with_vtk="/usr"])

  AC_ARG_WITH([vtk-version],
    [AC_HELP_STRING([--with-vtk-version],
    [VTK's include directory name is vtk-suffix, e.g. vtk-5.0/. What's the suffix? (Default -5.0)])],
    [vtk_suffix=$withval],
    [vtk_suffix="-5.0"])

])# AM_OPTIONS_VTK



dnl
dnl  AM_PATH_VTK([minimum-version], [action-if-found], [action-if-not-found])
dnl  ------------------------------------------------------------------------
dnl
dnl  NOTE: [minimum-version] must be in the form [X.Y.Z]
dnl
AC_DEFUN([AM_PATH_VTK],
[
  dnl do we want to check for VTK ?
  if test "$with_vtk" = yes; then
    dnl in case user wrote --with-vtk=yes
    with_vtk="/usr/local"
  fi

  if test "$with_vtk" != no; then
    dnl
    dnl A path was provided in $with_vtk...try hard to find the VTK library {{{
    VTK_PREFIX="$with_vtk"

    AC_CHECK_FILE([$VTK_PREFIX/include/vtk$vtk_suffix/vtkCommonInstantiator.h], [vtkFound="OK"])
    AC_MSG_CHECKING([if VTK is installed in $VTK_PREFIX])

    if test -z "$vtkFound"; then
      dnl
      dnl VTK was not found! ...execute $3 unconditionally {{{
      AC_MSG_RESULT([no])
      $3
      dnl }}}
      dnl
    else
      dnl
      dnl VTK was found! ...execute $2 if version matches {{{
      AC_MSG_RESULT([yes])

      dnl these are the VTK libraries of a default build
      VTK_LIBS="-lvtkCommon -lvtkDICOMParser -lvtkexpat -lvtkFiltering -lvtkfreetype -lvtkftgl -lvtkGraphics -lvtkHybrid -lvtkImaging -lvtkIO -lvtkjpeg -lvtkpng -lvtkRendering -lvtktiff -lvtkzlib"

      dnl set VTK c,cpp,ld flags
      VTK_CFLAGS="-I$VTK_PREFIX/include/vtk$vtk_suffix"
      VTK_CXXFLAGS="$VTK_CFLAGS"
      VTK_LDFLAGS="-L$VTK_PREFIX/lib/vtk$vtk_suffix $VTK_LIBS"

      dnl now, eventually check version {{{
      if test -n "$1"; then
        dnl
        dnl A version was specified... parse the version string in $1 {{{

        dnl The version of VTK that we need: {{{
        maj=`echo $1 | sed 's/\([[0-9]]*\).\([[0-9]]*\).\([[0-9]]*\)/\1/'`
        min=`echo $1 | sed 's/\([[0-9]]*\).\([[0-9]]*\).\([[0-9]]*\)/\2/'`
        rel=`echo $1 | sed 's/\([[0-9]]*\).\([[0-9]]*\).\([[0-9]]*\)/\3/'`
        AC_MSG_CHECKING([if VTK version is at least $maj.$min.$rel])
        dnl }}}

        dnl Compare required version of VTK against installed version: {{{
        dnl
        dnl Note that in order to be able to compile the following test program,
        dnl we need to add to the current flags, the VTK settings...
        OLD_CFLAGS=$CFLAGS
        OLD_CXXFLAGS=$CXXFLAGS
        OLD_LDFLAGS=$LDFLAGS
        CFLAGS="$VTK_CFLAGS $CFLAGS"
        CXXFLAGS="$VTK_CXXFLAGS $CXXFLAGS"
        LDFLAGS="$VTK_LDFLAGS $LDFLAGS"
        dnl
        dnl check if the installed VTK is greater or not
        AC_COMPILE_IFELSE([AC_LANG_PROGRAM(
              [
                 #include <vtkConfigure.h>
                 #include <stdio.h>
              ],
              [
                printf("VTK version is: %d.%d.%d", VTK_MAJOR_VERSION, VTK_MINOR_VERSION, VTK_BUILD_VERSION);
                #if VTK_MAJOR_VERSION < $maj
                #error Installed VTK is too old !
                #endif
                #if VTK_MINOR_VERSION < $min
                #error Installed VTK is too old !
                #endif
                #if VTK_BUILD_VERSION < $rel
                #error Installed VTK is too old !
                #endif
              ])
        ], [vtkVersion="OK"])
        dnl
        dnl restore all flags without VTK values
        CFLAGS=$OLD_CFLAGS
        CXXFLAGS=$OLD_CXXFLAGS
        LDFLAGS=$OLD_LDFLAGS
        dnl }}}

        dnl Execute $2 if version is ok, otherwise execute $3 {{{
        if test "$vtkVersion" = "OK"; then
          AC_MSG_RESULT([yes])
          $2
        else
          AC_MSG_RESULT([no])
          $3
        fi
        dnl }}}

        dnl }}}
        dnl
      else
        dnl
        dnl A target version number was not provided... execute $2 unconditionally {{{

        dnl if we don't have to check for minimum version (because the user did not set that option),
        dnl then we can execute here the block action-if-found
        #CFLAGS="$VTK_CFLAGS $CFLAGS"
        #CXXFLAGS="$VTK_CXXFLAGS $CXXFLAGS"
        #LDFLAGS="$VTK_LDFLAGS $LDFLAGS"
        $2
        dnl }}}
        dnl
      fi
      dnl }}}

      dnl }}}
      dnl
    fi
    dnl }}}
    dnl
  fi
])# AM_PATH_VTK
dnl
dnl vim: foldmethod=marker foldlevel=1 ts=2 sw=2
