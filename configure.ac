dnl--------------------------------------------------------------------
dnl    The MB-system:    configure.ac    12/7/2011
dnl    $Id$
dnl
dnl    Copyright (c) 2011-2019 by
dnl    D. W. Caress (caress@lamont.ldgo.columbia.edu)
dnl    and D. N. Chayes (dale@lamont.ldgo.columbia.edu)
dnl    Lamont-Doherty Earth Observatory
dnl    Palisades, NY  10964
dnl
dnl    See README file for copying and redistribution conditions.
dnl--------------------------------------------------------------------

dnl Require automake 2.69 or later
AC_PREREQ([2.69])

dnl Disable configure caching
define([AC_CACHE_LOAD], )
define([AC_CACHE_SAVE], )

dnl Initialize and set version and version date
AC_INIT([mbsystem],[5.7.6beta1],[http://listserver.mbari.org/sympa/arc/mbsystem],[mbsystem],[http://www.mbari.org/data/mbsystem/])

AC_CONFIG_MACRO_DIR(m4)
AC_LANG(C)
# AC_LANG([C++])
AC_DEFINE(VERSION_DATE, "6 July 2019", [Set VERSION_DATE define in mb_config.h])

dnl Check system arch
AC_CANONICAL_HOST
AC_DISABLE_OPTION_CHECKING

AS_ECHO([" "])
AS_ECHO(["------------------------------------------------------------------------------"])
AS_ECHO(["    The MB-system:    configure    12/7/2011"])
AS_ECHO(["    $Id$"])
AS_ECHO([" "])
AS_ECHO(["    Copyright (c) 2011-2019 by"])
AS_ECHO(["    D. W. Caress (caress@lamont.ldgo.columbia.edu)"])
AS_ECHO(["    and D. N. Chayes (dale@lamont.ldgo.columbia.edu)"])
AS_ECHO(["    Lamont-Doherty Earth Observatory"])
AS_ECHO(["    Palisades, NY  10964"])
AS_ECHO([" "])
AS_ECHO(["    See README file for copying and redistribution conditions."])
AS_ECHO(["------------------------------------------------------------------------------"])
AS_ECHO([" "])
AS_ECHO(["    The initial version of the MB-System autoconf-based build system"])
AS_ECHO(["    was contributed by Bob Covill on 7 December 2011. Further"])
AS_ECHO(["    refinement of the build system involved contributions from"])
AS_ECHO(["    Bob Covill, Kurt Schwehr, Hamish Bowman, David Caress, and"])
AS_ECHO(["    Christian Ferreira."])
AS_ECHO([" "])
AS_ECHO(["------------------------------------------------------------------------------"])
AS_ECHO(["    To modify this build system..."])
AS_ECHO(["------------------------------------------------------------------------------"])
AS_ECHO([" "])
AS_ECHO(["    Edit the file configure.ac in the top directory and the files"])
AS_ECHO(["    Makefile.am in each directory under src. Then run the following sequence"])
AS_ECHO(["    of commands:"])
AS_ECHO([" "])
AS_ECHO(["        # Build libtool files for LT_INIT"])
AS_ECHO(["        libtoolize --force --copy"])
AS_ECHO(["        aclocal"])
AS_ECHO([" "])
AS_ECHO(["        # Build custom header for configure"])
AS_ECHO(["        autoheader"])
AS_ECHO(["        automake --add-missing --include-deps"])
AS_ECHO(["        autoconf"])
AS_ECHO([" "])
AS_ECHO(["        # To update configure files use the following:"])
AS_ECHO(["        autoupdate"])
AS_ECHO(["        autoreconf --force --install --warnings=all"])
AS_ECHO([" "])
AS_ECHO(["    When you run ./configure, a number of configure options are saved to a"])
AS_ECHO(["    header file:"])
AS_ECHO(["        ./src/mbio/mb_config.h"])
AS_ECHO(["    This file has a template:"])
AS_ECHO(["        ./src/mbio/mb_config.h.in"])
AS_ECHO(["    This file is conditionally included by:"])
AS_ECHO(["        ./src/mbio/mb_define.h"])
AS_ECHO(["    which is in turn included by almost every MB-System C source file."])
AS_ECHO([" "])
AS_ECHO(["------------------------------------------------------------------------------"])
AS_ECHO(["    To use the build system..."])
AS_ECHO(["------------------------------------------------------------------------------"])
AS_ECHO(["    To generate the makefiles needed to build MB-System, run ./configure"])
AS_ECHO(["    with the options appropriate for your situation. These options are"])
AS_ECHO(["    listed here:"])
AS_ECHO(["        --prefix=install      - location for mbsystem (/usr/local/mbsystem)"])
AS_ECHO(["        --with-netcdf-config  - location of NetCDF config script nc-config"])
AS_ECHO(["        --with-gdal-config    - location of GDAL config script gdal-config"])
AS_ECHO(["        --with-gmt-config     - location of GMT config script gmt-config"])
AS_ECHO(["        --with-proj-lib       - location of PROJ libs"])
AS_ECHO(["        --with-proj-include   - location of PROJ headers"])
AS_ECHO(["        --with-fftw-lib       - location of FFTW3 libs (optional)"])
AS_ECHO(["        --with-fftw-include   - location of FFTW3 headers (optional)"])
AS_ECHO(["        --with-motif-lib      - location of Motif libs (optional)"])
AS_ECHO(["        --with-motif-include  - location of Motif headers (optional)"])
AS_ECHO(["        --with-opengl-lib     - location of OpenGL libs (optional)"])
AS_ECHO(["        --with-opengl-include - location of OpenGL headers (optional)"])
AS_ECHO(["        --with-otps-dir       - location of OTPS installation (optional)"])
AS_ECHO(["        --enable-mbtrn        - enable building terrain relative navigation (TRN) tools (optional)"])
AS_ECHO(["        --enable-hardening    - Enable compiler and linker options to frustrate memory corruption exploits  (e.g. -fPIE and -pie) (optional)"])
AS_ECHO(["        --enable-test         - Enable building unit tests in test/ and third-party/"])
AS_ECHO([" "])
AS_ECHO(["    After configure you can run the make utility in the usual fashion"])
AS_ECHO(["        make"])
AS_ECHO(["        make install"])
AS_ECHO([" "])
AS_ECHO(["    Some other useful make commands include: "])
AS_ECHO(["        make clean (to clean up compiled code)"])
AS_ECHO(["        make distclean (to clean up compiled code and configure output)"])
AS_ECHO(["        make uninstall (to remove a previously installed version)"])
AS_ECHO(["------------------------------------------------------------------------------"])
AS_ECHO([" "])
AS_ECHO(["Configure will now attempt to construct the Makefiles required to build MB-System"])
AS_ECHO(["MB-System Version $PACKAGE_VERSION"])
AS_ECHO(["HOST System Type: ${host}"])
AS_ECHO([" "])

AM_INIT_AUTOMAKE([no-dist subdir-objects])
AM_MAINTAINER_MODE
AC_CONFIG_HEADERS([src/mbio/mb_config.h])

dnl Check for programs
AC_PROG_CC
AM_PROG_CC_C_O
AC_PROG_CC_STDC
AC_PROG_CXX
AC_PROG_INSTALL
AM_PROG_AS
m4_ifdef([AM_PROG_AR], [AM_PROG_AR])
AC_PROG_LN_S
AC_PROG_MAKE_SET
LT_INIT
AC_SUBST([LIBTOOL_DEPS])
LT_LIB_M
AM_PATH_PYTHON([3.6],, [:])
AM_CONDITIONAL([HAVE_PYTHON], [test "$PYTHON" != :])

dnl Check Endian size
BYTESWAP=no
AC_C_BIGENDIAN([BYTESWAP=no], [BYTESWAP=yes],
    AC_MSG_ERROR(Unknown endian state),
    AC_MSG_ERROR(Universial endianness not supported)
)
if test BYTESWAP == "no" ; then
    AC_DEFINE(ENDIAN_BIG, 1, [Machine is bigendian, (Byteswapping off)])
else
    AC_DEFINE(BYTESWAPPED, 1, [Machine is littleendian, (Byteswapping on)])
fi

dnl Check for headers
AC_HEADER_STDC

dnl Check for math lib
AC_CHECK_LIB(m,exp,,,)

dnl check for signed char
AC_TYPE_INT8_T

dnl check for malloc header
AC_CHECK_HEADER(malloc.h, AC_DEFINE(HAVE_MALLOC_H, 1, [Have malloc.h]))

dnl Parse host_os from CANONICAL and deterimine
dnl if solaris flag required
case "${host_os}" in
    solaris*|sunos*) CFLAGS="${CFLAGS} -DSUN" ;;
    *) AS_ECHO(["Build does not require -DSUN CFLAG"]) ;;
esac

dnl check for win32 flag
case "${host_os}" in
    *mingw32*|cygwin*)
#    AC_DEFINE(WIN32, 1, [Turned on WIN32 define in config])
    AS_ECHO(["Identified WIN32 but with Cygwin - no WIN32 define added to mb_config.h"])
    ;;
    *)
    AS_ECHO(["Not WIN32 system"])
    ;;
esac

dnl Check for RPC and XDR headers needed by MBIO library
AC_CHECK_HEADERS(rpc/rpc.h, , [AC_CHECK_HEADERS(tirpc/rpc/rpc.h)], )
AC_CHECK_HEADERS(rpc/types.h, ,
    [AC_CHECK_HEADERS(tirpc/rpc/types.h, , [AC_MSG_ERROR([Required header <rpc/types.h> not found, check include path and installed development packages])],)],)

dnl Check for WIN32 libraries
# AC_CHECK_LIB(uuid, main)
# AC_CHECK_LIB(ole32, main)
# AC_CHECK_LIB(gw32c, main)
# AC_CHECK_LIB(wsock32, main)
# AC_CHECK_LIB(ws2_32, main)

dnl Look for library containing the XDR functions
AC_CHECK_FUNC(xdrmem_create, XDR_LIB=, [
  AC_CHECK_LIB(sun, xdrmem_create, XDR_LIB=-lsun, [
    AC_CHECK_LIB(nsl, xdrmem_create, XDR_LIB=-lnsl, [
      AC_CHECK_LIB(rpclib, xdrmem_create, XDR_LIB=-lrpclib, [
        AC_CHECK_LIB(xdr, xdrmem_create, XDR_LIB=-lxdr, [
          AC_CHECK_LIB(tirpc, xdrmem_create, XDR_LIB=-ltirpc, [
            AC_CHECK_LIB(portablexdr, xdrmem_create, XDR_LIB=-lportablexdr,
                [AC_MSG_ERROR([*** Unable to locate XDR functions.])], -lws2_32 )
          ])
        ])
      ])
    ])
  ])
])
AC_SUBST(XDR_LIB)

dnl--------------------------------------------------------------------
dnl Set up to check compiler support for specific flags.
dnl There is no way to make clang's "argument unused" warning fatal,
dnl so when using check_cc_cxx_flag and check_link_flag modify CC, CXX,
dnl and LD to invoke the compiler through a wrapper script called
dnl ./flag-check.sh that greps for this message (and then restore
dnl CC, CXX, and LD when done).
dnl--------------------------------------------------------------------
AC_DEFUN([check_cc_cxx_flag], [AC_LANG_PUSH(C) AX_CHECK_COMPILE_FLAG([$1],
  [AC_LANG_PUSH(C) AX_CHECK_COMPILE_FLAG([$1], [$2], [$3], [-Werror $4])
    AC_LANG_POP(C)], [$3], [-Werror $4]) AC_LANG_POP(C)])
AC_DEFUN([check_link_flag], [AX_CHECK_LINK_FLAG([$1], [$2], [$3], [-Werror $4])])

dnl--------------------------------------------------------------------
dnl Enable compiler and linker options to frustrate memory corruption exploits
dnl including generating Position Independent Executables (PIE)
dnl using --enable-hardening
dnl--------------------------------------------------------------------
dnl HARDENING START
dnl This is taken from https://github.com/kmcallister/autoharden
dnl With some very small modifications (to remove C++ checking for instance)
dnl made by https://git.insomnia247.nl/irc/unreal4/blob/unreal40/configure.ac
dnl
# Save the raw compiler names.
saved_CC="$CC"
saved_CXX="$CXX"
saved_LD="$LD"
flag_wrap="./flag-check.sh"
CC="$flag_wrap $CC"
CXX="$flag_wrap $CXX"
LD="$flag_wrap $LD"

AC_ARG_ENABLE([hardening],
  [AS_HELP_STRING([--enable-hardening],
    [Enable compiler and linker options to frustrate memory corruption exploits @<:@yes@:>@])],
  [hardening="$enableval"],
  [hardening="no"])

HARDEN_CFLAGS=""
HARDEN_LDFLAGS=""
AS_ECHO(["Checking for hardening enabled.... $hardening"])

AS_IF([test x"$hardening" != x"no"], [
  check_cc_cxx_flag([-fno-strict-overflow], [HARDEN_CFLAGS="$HARDEN_CFLAGS -fno-strict-overflow"])

  # This one will likely succeed, even on platforms where it does nothing.
  check_cc_cxx_flag([-D_FORTIFY_SOURCE=2], [HARDEN_CFLAGS="$HARDEN_CFLAGS -D_FORTIFY_SOURCE=2"])

  check_cc_cxx_flag([-fstack-protector-all],
   [check_link_flag([-fstack-protector-all],
     [HARDEN_CFLAGS="$HARDEN_CFLAGS -fstack-protector-all"
      check_cc_cxx_flag([-Wstack-protector], [HARDEN_CFLAGS="$HARDEN_CFLAGS -Wstack-protector"],
        [], [-fstack-protector-all])
      check_cc_cxx_flag([--param ssp-buffer-size=1], [HARDEN_CFLAGS="$HARDEN_CFLAGS --param ssp-buffer-size=1"],
        [], [-fstack-protector-all])])])

  # At the link step, we might want -pie (GCC) or -Wl,-pie (Clang on OS X)
  #
  # The linker checks also compile code, so we need to include -fPIE as well.
  check_cc_cxx_flag([-fPIE],
   [check_link_flag([-fPIE -pie],
     [HARDEN_BINCFLAGS="-fPIE"
      HARDEN_BINLDFLAGS="-pie"],
     [check_link_flag([-fPIE -Wl,-pie],
       [HARDEN_BINCFLAGS="-fPIE"
        HARDEN_BINLDFLAGS="-Wl,-pie"])])])

  check_link_flag([-Wl,-z,relro],
   [HARDEN_LDFLAGS="$HARDEN_LDFLAGS -Wl,-z,relro"
    check_link_flag([-Wl,-z,now], [HARDEN_LDFLAGS="$HARDEN_LDFLAGS -Wl,-z,now"])])
    ])
AC_SUBST([HARDEN_CFLAGS])
AC_SUBST([HARDEN_LDFLAGS])
AC_SUBST([HARDEN_BINCFLAGS])
AC_SUBST([HARDEN_BINLDFLAGS])

# End of flag tests.
CC="$saved_CC"
CXX="$saved_CXX"
LD="$saved_LD"
dnl HARDENING END

dnl--------------------------------------------------------------------
dnl Shared Data Section
dnl--------------------------------------------------------------------

dnl variables for shared data
mbsystemdatadir="$datadir/$PACKAGE"
AC_SUBST(mbsystemdatadir)
mbsystemhtmldir="$docdir/html"
AC_SUBST(mbsystemhtmldir)
mbsystempsdir="$docdir/ps"
AC_SUBST(mbsystempsdir)

dnl--------------------------------------------------------------------
dnl OTPS Section
dnl--------------------------------------------------------------------

dnl Use OTPS (OSU Tidal Prediction Software) location if specified with --with-otps-dir, use default otherwise
AS_ECHO(["Use OTPS (OSU Tidal Prediction Software) location if specified with --with-otps-dir, use default otherwise"])
OTPS_DIR="/usr/local/OTPS2"
AC_ARG_WITH(otps_dir, [ --with-otps-dir=DIR    Location of OSU Tidal Prediction Software], [OTPS_DIR=${withval}])
AC_SUBST(OTPS_DIR)

dnl--------------------------------------------------------------------
dnl PROJ Section
dnl--------------------------------------------------------------------

dnl Check for Proj.4 library libproj
AS_ECHO(["Check for Proj.4 library libproj"])
GOT_PROJ=no
PROJ_SETTING=no
AC_ARG_WITH(proj-lib, [  --with-proj-lib=DIR    Location of PROJ library],
    [proj_libdir="${withval}"] [GOT_PROJ=yes], [GOT_PROJ=no])
if test "$GOT_PROJ" = "yes" ; then
    dnl PROJ library location specified: $proj_libdir - check if libproj is there...
    AS_ECHO(["PROJ library location specified: $proj_libdir - check if libproj is there..."])
    save_LDFLAGS=$LDFLAGS
    LDFLAGS="$LDFLAGS -L$proj_libdir"
    AC_CHECK_LIB([proj], [pj_init_plus], [PROJ_SETTING=yes], [PROJ_SETTING=no], )
    LDFLAGS="$save_LDFLAGS"
    if test "$PROJ_SETTING" = "no"; then
        AC_MSG_ERROR([Did not find libproj according to --with-proj-lib - check the installation!])
    fi
    libproj_LIBS="-L$proj_libdir -R $proj_libdir -lproj"
    libproj_LDFLAGS="-L$proj_libdir"
else
    dnl Look for pkg-config PROJ package...
    AS_ECHO(["Look for pkg-config PROJ package..."])
    PKG_CHECK_MODULES([libproj], [proj], [GOT_PROJ=yes], [GOT_PROJ=no])
    libproj_LIBS="-lproj"
    libproj_LDFLAGS=""
fi
if test "$GOT_PROJ" = "no" ; then
    dnl Did not find PROJ library pkg-config package, looking in the usual places...
    AS_ECHO(["Did not find PROJ library pkg-config package, looking in the usual places..."])
    AC_CHECK_LIB([proj], [pj_init_plus], [PROJ_SETTING=yes], [PROJ_SETTING=no],)
    if test "$PROJ_SETTING" = "yes" ; then
        GOT_PROJ=yes
        AS_ECHO(["Using pre-installed libproj..."])
        libproj_LIBS="-lproj"
        libproj_LDFLAGS=""
    else
        AS_ECHO(["Did not find PROJ library anywhere, use --with-proj-lib to set the location"])
        AC_MSG_ERROR([Did not find PROJ library anywhere, use --with-proj-lib to set the location])
    fi
fi
dnl Done checking for Proj.4 library libproj

dnl Check if proj header location specified on the command line
AS_ECHO(["Check if proj header location specified on the command line"])
GOT_PROJ=no
PROJ_SETTING=no
AC_ARG_WITH(proj-include, [  --with-proj-include=DIR    Location of PROJ headers],
        [libproj_CPPFLAGS="-I${withval}"] [GOT_PROJ=yes], [GOT_PROJ=no])
if test "$GOT_PROJ" = "yes" ; then
    dnl PROJ header location specified: $libproj_CPPFLAGS - check if proj_api.h is there...
    AS_ECHO(["PROJ header location specified: $libproj_CPPFLAGS - check if proj_api.h is there..."])
    save_CPPFLAGS=$CPPFLAGS
    save_CFLAGS=$CFLAGS
    CPPFLAGS="$CPPFLAGS $libproj_CPPFLAGS"
    CFLAGS="$CFLAGS $libproj_CPPFLAGS -DACCEPT_USE_OF_DEPRECATED_PROJ_API_H"
    AC_CHECK_HEADER(proj_api.h, [PROJ_SETTING=yes], [PROJ_SETTING=no],)
    CPPFLAGS=$save_CPPFLAGS
    CFLAGS=$save_CFLAGS
    if test "$PROJ_SETTING" = "no"; then
       AC_MSG_ERROR([Did not find PROJ headers according to --with-proj-include - check the installation!])
    fi
else
    dnl Looking for PROJ headers in the usual places...
    AS_ECHO(["Looking for PROJ headers in the usual places..."])
    save_CPPFLAGS=$CPPFLAGS
    save_CFLAGS=$CFLAGS
    CPPFLAGS="$CPPFLAGS $libproj_CPPFLAGS"
    CFLAGS="$CFLAGS $libproj_CPPFLAGS -DACCEPT_USE_OF_DEPRECATED_PROJ_API_H"
    AC_CHECK_HEADER(proj_api.h, [PROJ_SETTING=yes], [PROJ_SETTING=no])
    CPPFLAGS=$save_CPPFLAGS
    CFLAGS=$save_CFLAGS
    if test "$PROJ_SETTING" = "yes" ; then
        GOT_PROJ=yes
        AS_ECHO(["Found PROJ headers..."])
    else
        AS_ECHO(["Did not find PROJ headers anywhere, use --with-proj-include to set the location"])
        AC_MSG_ERROR([Did not find PROJ headers anywhere, use --with-proj-include to set the location])
    fi
fi
dnl Done checking for PROJ headers
dnl Done checking for PROJ library libproj and PROJ headers

dnl Add -DACCEPT_USE_OF_DEPRECATED_PROJ_API_H to allow building with PROJ6
libproj_CPPFLAGS="$libproj_CPPFLAGS -DACCEPT_USE_OF_DEPRECATED_PROJ_API_H"

dnl Substitute PROJ library and header arguments
AC_SUBST([libproj_LIBS], [$libproj_LIBS])
AC_SUBST([libproj_CPPFLAGS], [$libproj_CPPFLAGS])

dnl--------------------------------------------------------------------
dnl NetCDF Section
dnl Use specified nc-config or look for nc-config in the path
dnl--------------------------------------------------------------------
dnl

if test "X$with_netcdf_config" != "X" ; then
    NC_CONF="$with_netcdf_config/nc-config"
    AC_MSG_CHECKING(for netCDF nc-config using --with-netcdf-config=$with_netcdf_config )
    AC_MSG_RESULT($NC_CONF)
else
    AC_PATH_PROG([NC_CONF], [nc-config] )
fi

if test -x "$NC_CONF" ; then
    AC_MSG_CHECKING(for netCDF includes)
    libnetcdf_CPPFLAGS=-I`$NC_CONF --includedir | sed -e 's/^-I//'`
    AC_MSG_RESULT($libnetcdf_CPPFLAGS)
    AC_MSG_CHECKING(for netCDF libs)
    libnetcdf_LIBS=`$NC_CONF --libs`
    AC_MSG_RESULT($libnetcdf_LIBS)
    NETCDF_LIB=`echo $libnetcdf_LIBS | sed 's/^-L//;s/\ .*//'`
    libnetcdf_LDFLAGS="-L$NETCDF_LIB"
else
    if test "X$enable_netcdf" = "X" || test "X$enable_netcdf" = "Xyes" ; then
        AC_PATH_PROG([NETCDF], [ncdump], [/usr/local/netcdf/bin/ncdump])
        NETCDF=`dirname $NETCDF`
        NETCDF=`dirname $NETCDF`
    else
        NETCDF=$enable_netcdf
    fi
    AC_MSG_CHECKING(for netCDF include directory)
    if test "X$NETCDF_INC" = "X"; then
        NETCDF_INC=$NETCDF/include
    fi
    AC_MSG_RESULT($NETCDF_INC)
    if test ! -f $NETCDF_INC/netcdf.h; then
        AC_MSG_WARN([[netcdf.h can not be found in $NETCDF_INC]])
    fi
    AC_MSG_CHECKING(for netCDF library directory)
    if test "X$NETCDF_LIB" = "X"; then
        NETCDF_LIB=$NETCDF/lib
    fi
    AC_MSG_RESULT($NETCDF_LIB)
    if test ! -d $NETCDF_LIB; then
        AC_MSG_WARN([[No such directory: $NETCDF_LIB]])
    fi
    libnetcdf_LIBS="-L$NETCDF_LIB -R $NETCDF_LIB -lnetcdf"
    libnetcdf_LDFLAGS="-L$NETCDF_LIB"
    libnetcdf_CPPFLAGS="-I$NETCDF_INC"
fi
if test ! -x "$NC_CONF" && test ! -d $NETCDF_LIB; then
    AC_MSG_ERROR(["Did not find nc-config program or netCDF installation, use --with-nc-config to set the location"])
fi
dnl
dnl Done checking for netCDF headers and libraries

dnl Substitute netCDF library and header arguments
AC_SUBST([libnetcdf_LIBS], [$libnetcdf_LIBS])
AC_SUBST([libnetcdf_CPPFLAGS], [$libnetcdf_CPPFLAGS])

dnl--------------------------------------------------------------------
dnl GDAL Section
dnl We use gdal-config to set GDAL paths and settings
dnl--------------------------------------------------------------------

if test "X$with_gdal_config" != "X" ; then
    GDAL_CONF="$with_gdal_config/gdal-config"
    AC_MSG_CHECKING(for GDAL gdal-config using --with-gdal-config=$with_gdal_config )
    AC_MSG_RESULT($GDAL_CONF)
else
    AC_PATH_PROG([GDAL_CONF], [gdal-config])
fi

if test -x "$GDAL_CONF" ; then
    AC_MSG_CHECKING(for GDAL includes)
    GDAL_INC=`$GDAL_CONF --cflags`
    AC_MSG_RESULT($GDAL_INC)
    AC_MSG_CHECKING(for GDAL libs)
    GDAL_LIB=`$GDAL_CONF --libs`
    DEP_LIBS=`$GDAL_CONF --dep-libs`
    AC_MSG_RESULT($GDAL_LIB)
    GDAL_LIB_PATH=`echo $GDAL_LIB | sed 's/^-L//;s/\ .*//'`

    libgdal_CPPFLAGS="$GDAL_INC"
    libgdal_LIBS="-L$GDAL_LIB_PATH -R $GDAL_LIB_PATH -lgdal"
    libgdal_LDFLAGS="-L$GDAL_LIB_PATH"
else
    AS_ECHO(["Did not find gdal-config program, use --with-gdal-config to set the location"])
    AC_MSG_ERROR(["Did not find gdal-config program, use --with-gdal-config to set the location"])
fi

dnl Substitute GDAL library and header arguments
AC_SUBST([libgdal_LIBS], [$libgdal_LIBS])
AC_SUBST([libgdal_CPPFLAGS], [$libgdal_CPPFLAGS])


dnl--------------------------------------------------------------------
dnl GMT Section
dnl We use gmt-config to set GMT5 paths and settings
dnl--------------------------------------------------------------------

if test "X$with_gmt_config" != "X" ; then
    GMT_CONF="$with_gmt_config/gmt-config"
    AC_MSG_CHECKING(for GMT gmt-config using --with-gmt-config=$with_gmt_config )
    AC_MSG_RESULT($GMT_CONF)
else
    AC_PATH_PROG([GMT_CONF], [gmt-config])
fi
if test -x "$GMT_CONF" ; then
    AC_MSG_CHECKING(for GMT includes)
    GMT_INC=`$GMT_CONF --cflags 2>/dev/null`
    AC_MSG_RESULT($GMT_INC)
    AC_MSG_CHECKING(for GMT libs)
    GMT_LIB=`$GMT_CONF --libs 2>/dev/null`
    DEP_LIBS=`$GMT_CONF --dep-libs 2>/dev/null`
    AC_MSG_RESULT($GMT_LIB)
    GMT_LIB_PATH=`echo $GMT_LIB | sed 's/^-L//;s/\ .*//'`
    rpath="$GMT_LIB_PATH"
    AC_MSG_CHECKING(for 32/64-bit GMT5 installation)
    bits=`$GMT_CONF --bits 2>/dev/null`
    GCC_64="-m${bits}"
    LDFLAGS="$LDFLAGS -m${bits}"
    AC_MSG_RESULT(found ${bits}-bit)
    AC_MSG_CHECKING(for GMT plugin location)
    GMT_PLUGINDIR=`$GMT_CONF --plugindir 2>/dev/null`
    AC_MSG_RESULT($GMT_PLUGINDIR)

    libgmt_CPPFLAGS="$GMT_INC"
    libgmt_LIBS="-L$GMT_LIB_PATH -R $GMT_LIB_PATH -lgmt -lpostscriptlight"
    libgmt_LDFLAGS="-L$GMT_LIB_PATH"
else
    AS_ECHO(["Did not find gmt-config program, use --with-gmt-config to set the location"])
    AC_MSG_ERROR(["Did not find gmt-config program, use --with-gmt-config to set the location"])
fi

dnl Substitute GMT library and header arguments
AC_SUBST([libgmt_LIBS], [$libgmt_LIBS])
AC_SUBST([libgmt_CPPFLAGS], [$libgmt_CPPFLAGS])
AC_SUBST(GMT_PLUGINDIR)

dnl--------------------------------------------------------------------
dnl FFTW Section
dnl--------------------------------------------------------------------

dnl Check for FFTW library libfftw3
AS_ECHO(["Check for FFTW library libfftw3"])
GOT_FFTW=no
FFTW_SETTING=no
dnl First check if fftw location specified on the command line
AC_ARG_WITH(fftw-lib, [  --with-fftw-lib=DIR    Location of FFTW library],
    [fftw_libdir="${withval}"] [GOT_FFTW=yes], [GOT_FFTW=no])
if test "$GOT_FFTW" = "yes" ; then
    dnl FFTW library location specified: $fftw_libdir - check if libfftw3 is there...
    AS_ECHO(["FFTW library location specified: $fftw_libdir - check if libfftw3 is there..."])
    save_LDFLAGS=$LDFLAGS
    LDFLAGS="$LDFLAGS -L$fftw_libdir"
    AC_CHECK_LIB([fftw3], [fftw_plan_dft_1d], [FFTW_SETTING=yes], [FFTW_SETTING=no], )
    LDFLAGS="$save_LDFLAGS"
    if test "$FFTW_SETTING" = "no"; then
        AC_MSG_ERROR([Did not find libfftw3 according to --with-fftw-lib - check the installation!])
    fi
    libfftw_LIBS="-L$fftw_libdir -R $fftw_libdir -lfftw3"
    libfftw_LDFLAGS="-L$fftw_libdir"
else
    dnl Look for pkg-config FFTW package...
    AS_ECHO(["Look for pkg-config FFTW package..."])
    PKG_CHECK_MODULES([libfftw3], [fftw3], [GOT_FFTW=yes], [GOT_FFTW=no])
    libfftw_LIBS="-lfftw3"
    libfftw_LDFLAGS=""
fi
if test "$GOT_FFTW" = "no" ; then
    dnl Did not find FFTW library pkg-config package, looking in the usual places...
    AS_ECHO(["Did not find FFTW library pkg-config package, looking in the usual places..."])
    AC_CHECK_LIB([fftw3], [fftw_plan_dft_1d], [FFTW_SETTING=yes], [FFTW_SETTING=no],)
    if test "$FFTW_SETTING" = "yes" ; then
        GOT_FFTW=yes
        AS_ECHO(["Using pre-installed libfftw3..."])
        libfftw_LIBS="-lfftw3"
        libfftw_LDFLAGS=""
    else
        AS_ECHO(["Did not find FFTW library anywhere, use --with-fftw-lib to set the location"])
        AC_MSG_ERROR([Did not find FFTW library anywhere, use --with-fftw-lib to set the location])
    fi
fi
dnl Done checking for FFTW library libfftw3

dnl Check for FFTW headers
AS_ECHO(["Check for FFTW headers"])
GOT_FFTW=no
FFTW_SETTING=no
dnl First check if fftw3 location specified on the command line
AC_ARG_WITH(fftw-include, [  --with-fftw-include=DIR    Location of FFTW headers],
    [libfftw_CPPFLAGS="-I${withval}"] [GOT_FFTW=yes], [GOT_FFTW=no])
if test "$GOT_FFTW" = "yes" ; then
    dnl FFTW header location specified: $libfftw_CPPFLAGS - check if fftw3.h is there...
    AS_ECHO(["FFTW header location specified: $libfftw_CPPFLAGS - check if fftw3.h is there..."])
    save_CPPFLAGS=$CPPFLAGS
    save_CFLAGS=$CFLAGS
    CPPFLAGS="$CPPFLAGS $libfftw_CPPFLAGS"
    CFLAGS="$CFLAGS $libfftw_CPPFLAGS"
    AC_CHECK_HEADER(fftw3.h, [FFTW_SETTING=yes], [FFTW_SETTING=no],)
    CPPFLAGS=$save_CPPFLAGS
    CFLAGS=$save_CFLAGS
    if test "$FFTW_SETTING" = "no"; then
    AC_MSG_ERROR([Did not find fftw3.h according to --with-fftw-include - check the installation!])
    fi
else
    dnl Looking for FFTW headers in the usual places...
    AS_ECHO(["Looking for FFTW headers in the usual places..."])
    AC_CHECK_HEADER(fftw3.h, [FFTW_SETTING=yes], [FFTW_SETTING=no])
    if test "$FFTW_SETTING" = "yes" ; then
        GOT_FFTW=yes
        AS_ECHO(["Using pre-installed FFTW headers..."])
    else
        AS_ECHO(["Did not find FFTW headers anywhere, use --with-fftw-include to set the location"])
        AC_MSG_ERROR([Did not find FFTW headers anywhere, use --with-fftw-include to set the location])
    fi
fi
dnl Done checking for FFTW headers

dnl Substitute FFTW library and header arguments
if test "$GOT_FFTW" = "yes" ; then
    AC_SUBST([fftw_app], [$USE_FFTW])
    AC_SUBST([libfftw_LIBS], [$libfftw_LIBS])
    AC_SUBST([libfftw_CPPFLAGS], [$libfftw_CPPFLAGS])
fi
AM_CONDITIONAL([BUILD_FFTW], [test "$GOT_FFTW" = yes])

dnl--------------------------------------------------------------------
dnl X11/Motif & OpenGL Section
dnl--------------------------------------------------------------------

dnl Check for X11 libraries libX11 libXt and X11 header file X11/Intrinsic.h
AS_ECHO(["Check for X11 libraries libX11 libXt and X11 header file X11/Intrinsic.h"])
AC_CHECK_LIB([Xt], [XtAppMainLoop], [GOT_X11=yes], [GOT_X11=no])
if test "$GOT_X11" = "yes" ; then
    AC_CHECK_LIB([X11], [XSendEvent], [GOT_X11=yes], [GOT_X11=no])
fi
if test "$GOT_X11" = "yes" ; then
    AC_CHECK_HEADER(X11/Intrinsic.h, [GOT_X11=yes], [GOT_X11=no],)
    if test "$GOT_X11" = "yes" ; then
        AS_ECHO(["Found X11 libraries libX11 libXt and X11 header file X11/Intrinsic.h"])
    else
        AS_ECHO(["Found X11 libraries libX11 libXt but did not find X11 header file X11/Intrinsic.h - graphical tools cannot be built"])
    fi
else
    AS_ECHO(["Did not find X11 libraries libX11 libXt - graphical tools cannot be built"])
fi

GOT_MOTIF=no
MOTIF_SETTING=no
if test "$GOT_X11" = "yes" ; then

    dnl Check for Motif library libXm
    AS_ECHO(["Check for Motif library libXm"])
    AC_ARG_WITH(motif-lib, [  --with-motif-lib=DIR    Location of MOTIF library],
            [motif_libdir="${withval}"] [GOT_MOTIF=yes], [GOT_MOTIF=no])
    if test "$GOT_MOTIF" = "yes" ; then
        dnl Motif library location specified: $motif_libdir - check if libXm is there...
        AS_ECHO(["Motif library location specified: $motif_libdir - check if libXm is there..."])
        save_LDFLAGS=$LDFLAGS
        LDFLAGS="$LDFLAGS -L$motif_libdir"
        AC_CHECK_LIB([Xm], [XmCreatePushButton], [MOTIF_SETTING=yes], [MOTIF_SETTING=no], )
        LDFLAGS="$save_LDFLAGS"
        if test "$MOTIF_SETTING" = "no"; then
            AC_MSG_ERROR([Did not find libXm according to --with-motif-lib - check the installation!])
        fi
        libmotif_LIBS="-L$motif_libdir -R $motif_libdir -lXm -lXt -lX11"
        libmotif_LDFLAGS="-L$motif_libdir"
    else
        dnl Look for pkg-config X11 package...
        AS_ECHO(["Look for pkg-config X11 package..."])
        PKG_CHECK_MODULES([libXm], [openmotif], [GOT_MOTIF=yes], [GOT_MOTIF=no])
        libmotif_LIBS="-lXm -lXt -lX11"
        libmotif_LDFLAGS=""
    fi
    if test "$GOT_MOTIF" = "no" ; then
        dnl Did not find Motif library pkg-config package, looking in the usual places...
        AS_ECHO(["Did not find Motif library pkg-config package, looking in the usual places..."])
        AC_CHECK_LIB([Xm], [XmCreatePushButton], [MOTIF_SETTING=yes], [MOTIF_SETTING=no],)
        if test "$MOTIF_SETTING" = "yes" ; then
            GOT_MOTIF=yes
            AS_ECHO(["Using pre-installed libXm..."])
            libmotif_LIBS="-lXm -lXt -lX11"
            libmotif_LDFLAGS=""
        else
            AS_ECHO(["Did not find Motif library anywhere, use --with-motif-lib to set the location"])
            AC_MSG_ERROR([Did not find Motif library anywhere, use --with-motif-lib to set the location])
        fi
    fi
    dnl Done checking for Motif library libmotif

    if test "$GOT_MOTIF" = "yes" ; then
        dnl Check for Motif headers
        AS_ECHO(["Check for Motif headers"])
        GOT_MOTIF=no
        MOTIF_SETTING=no
        dnl First check if Motif location specified on the command line
        AC_ARG_WITH(motif-include, [  --with-motif-include=DIR    Location of Motif headers],
                [libmotif_CPPFLAGS="-I${withval}"] [GOT_MOTIF=yes], [GOT_MOTIF=no])
        if test "$GOT_MOTIF" = "yes" ; then
            dnl Motif header location specified: $libmotif_CPPFLAGS - check if Xm/Xm.h is there...
            AS_ECHO(["Motif header location specified: $libmotif_CPPFLAGS - check if Xm/Xm.h is there..."])
            save_CPPFLAGS=$CPPFLAGS
            save_CFLAGS=$CFLAGS
            CPPFLAGS="$CPPFLAGS $libmotif_CPPFLAGS"
            CFLAGS="$CFLAGS $libmotif_CPPFLAGS"
            AC_CHECK_HEADER(Xm/Xm.h, [MOTIF_SETTING=yes], [MOTIF_SETTING=no],)
            CPPFLAGS=$save_CPPFLAGS
            CFLAGS=$save_CFLAGS
            if test "$MOTIF_SETTING" = "no"; then
                AC_MSG_ERROR([Did not find Xm/Xm.h according to --with-motif-include - check the installation!])
            fi
        else
            dnl Looking for Motif headers in the usual places...
            AS_ECHO(["Looking for Motif headers in the usual places..."])
            AC_CHECK_HEADER(Xm/Xm.h, [MOTIF_SETTING=yes], [MOTIF_SETTING=no])
            if test "$MOTIF_SETTING" = "yes" ; then
                GOT_MOTIF=yes
                AS_ECHO(["Using pre-installed Motif headers..."])
            else
                AS_ECHO(["Did not find Motif headers anywhere, use --with-motif-include to set the location"])
                AC_MSG_ERROR([Did not find Motif headers anywhere, use --with-motif-include to set the location])
            fi
        fi
        dnl Done checking for Motif headers
    fi

    dnl Substitute Motif library and header arguments
    AC_SUBST([libmotif_LIBS], [$libmotif_LIBS])
    AC_SUBST([libmotif_CPPFLAGS], [$libmotif_CPPFLAGS])

else
    dnl Failed to find X11 libraries and headers - unable to build graphical tools
    AS_ECHO(["Failed to find X11 libraries and headers - unable to build graphical tools"])
fi

AM_CONDITIONAL([BUILD_MOTIF], [test "$GOT_MOTIF" = yes])
dnl End check for Motif library and headers

dnl Check for OpenGL libraries libGL libGLU and header files GL/gl.h GL/glu.h GL/glx.h
AS_ECHO(["Check for OpenGL libraries libGL libGLU and header files GL/gl.h GL/glu.h GL/glx.h"])
GOT_OPENGL=no
OPENGL_SETTING=no
if test "$GOT_MOTIF" = "yes" ; then

    dnl Check for OpenGL libraries libGLU libGL
    AS_ECHO(["Check for OpenGL libraries libGLU libGL"])
    AC_ARG_WITH(opengl-lib, [  --with-opengl-lib=DIR    Location of OpenGL library],
            [opengl_libdir="${withval}"] [GOT_OPENGL=yes], [GOT_OPENGL=no])
    if test "$GOT_OPENGL" = "yes" ; then
        dnl OpenGL library location specified: $opengl_libdir - check if libGLU libGL is there...
        AS_ECHO(["OpenGL library location specified: $opengl_libdir - check if libGL libGLU are there..."])
        save_LDFLAGS=$LDFLAGS
        LDFLAGS="$LDFLAGS -L$opengl_libdir"
        AC_CHECK_LIB([GLU], [main], [OPENGL_SETTING=yes], [OPENGL_SETTING=no], )
        if test "$OPENGL_SETTING" = "yes"; then
            AC_CHECK_LIB([GL], [main], [OPENGL_SETTING=yes], [OPENGL_SETTING=no], )
        fi
        LDFLAGS="$save_LDFLAGS"
        if test "$OPENGL_SETTING" = "no"; then
            AC_MSG_ERROR([Did not find libGLU libGL according to --with-opengl-lib - check the installation!])
        fi
        libopengl_LIBS="-L$opengl_libdir -R $opengl_libdir -lGL -lGLU"
        libopengl_LDFLAGS="-L$opengl_libdir"
    else
        dnl Look for pkg-config X11 package...
        AS_ECHO(["Look for pkg-config OpenGL package..."])
        PKG_CHECK_MODULES([libGLU], [opengl], [GOT_OPENGL=yes], [GOT_OPENGL=no])
        libopengl_LIBS="-lGL -lGLU"
        libopengl_LDFLAGS=""
    fi
    if test "$GOT_OPENGL" = "no" ; then
        dnl Did not find OpenGL library pkg-config package, looking in the usual places...
        AS_ECHO(["Did not find OpenGL library pkg-config package, looking in the usual places..."])
        AC_CHECK_LIB([GLU], [main], [OPENGL_SETTING=yes], [OPENGL_SETTING=no],)
        if test "$OPENGL_SETTING" = "yes" ; then
            AC_CHECK_LIB([GL], [main], [OPENGL_SETTING=yes], [OPENGL_SETTING=no],)
        fi
        if test "$OPENGL_SETTING" = "yes" ; then
            GOT_OPENGL=yes
            AS_ECHO(["Using pre-installed libGLU libGL..."])
            libopengl_LIBS="-lGL -lGLU"
            libopengl_LDFLAGS=""
        else
            AS_ECHO(["Did not find OpenGL library anywhere, use --with-opengl-lib to set the location"])
            AC_MSG_ERROR([Did not find OpenGL library anywhere, use --with-opengl-lib to set the location])
        fi
    fi
    dnl Done checking for OpenGL libraries libGLU libGL

    if test "$GOT_OPENGL" = "yes" ; then
        dnl Check for OpenGL headers
        AS_ECHO(["Check for OpenGL headers"])
        GOT_OPENGL=no
        OPENGL_SETTING=no
        dnl First check if OpenGL location specified on the command line
        AC_ARG_WITH(opengl-include, [  --with-opengl-include=DIR    Location of OpenGL headers],
                [libopengl_CPPFLAGS="-I${withval}"] [GOT_OPENGL=yes], [GOT_OPENGL=no])
        if test "$GOT_OPENGL" = "yes" ; then
            dnl OpenGL header location specified: $libopengl_CPPFLAGS - check if Xm/Xm.h is there...
            AS_ECHO(["OpenGL header location specified: $libopengl_CPPFLAGS - check if Xm/Xm.h is there..."])
            save_CPPFLAGS=$CPPFLAGS
            save_CFLAGS=$CFLAGS
            CPPFLAGS="$CPPFLAGS $libopengl_CPPFLAGS"
            CFLAGS="$CFLAGS $libopengl_CPPFLAGS"
            AC_CHECK_HEADER(Xm/Xm.h, [OPENGL_SETTING=yes], [OPENGL_SETTING=no],)
            CPPFLAGS=$save_CPPFLAGS
            CFLAGS=$save_CFLAGS
            if test "$OPENGL_SETTING" = "no"; then
                AC_MSG_ERROR([Did not find Xm/Xm.h according to --with-opengl-include - check the installation!])
            fi
        else
            dnl Looking for OpenGL headers in the usual places...
            AS_ECHO(["Looking for OpenGL headers in the usual places..."])
            AC_CHECK_HEADER(Xm/Xm.h, [OPENGL_SETTING=yes], [OPENGL_SETTING=no])
            if test "$OPENGL_SETTING" = "yes" ; then
                GOT_OPENGL=yes
                AS_ECHO(["Using pre-installed OpenGL headers..."])
            else
                AS_ECHO(["Did not find OpenGL headers anywhere, use --with-opengl-include to set the location"])
                AC_MSG_ERROR([Did not find OpenGL headers anywhere, use --with-opengl-include to set the location])
            fi
        fi
        dnl Done checking for OpenGL headers
    fi

    dnl Substitute OpenGL library and header arguments
    AC_SUBST([libopengl_LIBS], [$libopengl_LIBS])
    AC_SUBST([libopengl_CPPFLAGS], [$libopengl_CPPFLAGS])
    if test "$GOT_OPENGL" = "yes" ; then
        AC_DEFINE(GOT_GL, 1, [Turned on OpenGL define in config])
    fi
    AM_CONDITIONAL([BUILD_GL], [test "$GOT_OPENGL" = yes])

else
    dnl Failed to find Motif libraries and headers - did not look for OpenGL libraries and headers - unable to build visualization tools
    AS_ECHO(["Failed to find Motif libraries and headers - did not look for OpenGL libraries and headers - unable to build visualization tools"])
fi

dnl--------------------------------------------------------------------
dnl Add some variables to be exported to the src/mbio/mb_config.h file
dnl--------------------------------------------------------------------

AC_DEFINE_UNQUOTED(MBSYSTEM_CONFIG_DEFINED, 1, [Set MBSYSTEM_CONFIG_DEFINED define in mb_config.h])
AC_DEFINE_UNQUOTED(MBSYSTEM_INSTALL_PREFIX, "$prefix", [Set MBSYSTEM_INSTALL_PREFIX define in mb_config.h])
AC_DEFINE_UNQUOTED(MBSYSTEM_OTPS_LOCATION, "$OTPS_DIR", [Set MBSYSTEM_OTPS_LOCATION define in mb_config.h])


dnl--------------------------------------------------------------------
dnl MBTRN Section - optionally build terrain relative navigation (TRN) tools
dnl--------------------------------------------------------------------

dnl Check if the mbtrn tools should be built
AS_ECHO(["Check if the mbtrn tools should be built"])
AC_ARG_ENABLE(mbtrn, [AS_HELP_STRING([--enable-mbtrn], [build mbtrn tools])],
        [build_mbtrn=yes], [build_mbtrn=no])
AM_CONDITIONAL([BUILD_MBTRN], [test x$build_mbtrn = xyes])
if test "$build_mbtrn" = "yes" ; then
    dnl Building mbtrn tools
    AS_ECHO(["Building mbtrn tools"])
    AC_DEFINE(MBTRN_ENABLED, 1, [Build libmbtrn and mbtrnpreprocess])
else
    dnl Not building mbtrn tools
    AS_ECHO(["Not building mbtrn tools"])
fi

dnl--------------------------------------------------------------------
dnl Point Cloud Library Section - optionally build ICP based feature matching
dnl in mbnavadjust - uses ICP algorithm in Point Cloud Library (PCL)
dnl--------------------------------------------------------------------

dnl Check if programs using PCL should be built
AS_ECHO(["Check if programs using PCL should be built"])
AC_ARG_ENABLE(pcltools, [AS_HELP_STRING([--enable-pcltools],
        [build tools using PCL])], [build_pcltools=yes], [build_mbpcltools=no])
AM_CONDITIONAL([BUILD_PCLTOOLS], [test x$build_pcltools = xyes])
if test "$build_pcltools" = "yes" ; then
    dnl Build tools using PCL
    AS_ECHO(["Build tools using PCL"])
    AC_DEFINE(PCLTOOLS_ENABLED, 1, [Build tools using PCL])
else
    dnl Not building tools using PCL
    AS_ECHO(["Not building tools using PCL"])
fi

dnl If tools using PCL are being built then look for PCL libraries and headers
if test "$build_pcltools" = "yes" ; then
    AC_CHECK_LIB([libpcl_common], [XSendEvent], [GOT_libpcl_common=yes], [GOT_libpcl_common=no])
fi

dnl--------------------------------------------------------------------
dnl Test Section - optionally build unit tests in mbsystem/test and mbsystem/third-party
dnl--------------------------------------------------------------------

dnl Check if the testing tools should be built
AS_ECHO(["Check if the unit tests should be built"])
AC_ARG_ENABLE(test, [AS_HELP_STRING([--enable-test], [build test tools])],
        [build_test=yes], [build_test=no])
AM_CONDITIONAL([BUILD_TEST], [test x$build_test = xyes])
if test "$build_test" = "yes" ; then
    dnl Building unit tests
    AS_ECHO(["Building unit tests"])
    AC_DEFINE(TEST_ENABLED, 1, [Build unit tests])
else
    dnl Not building unit tests
    AS_ECHO(["Not building unit tests"])
fi

dnl--------------------------------------------------------------------
dnl Configure pthreads.
dnl Force to yes.
dnl--------------------------------------------------------------------

AM_CONDITIONAL([HAVE_PTHREADS],[test "xyes" = "xyes"])
AC_SUBST(PTHREAD_CFLAGS)
AC_SUBST(PTHREAD_LIBS)

dnl--------------------------------------------------------------------
dnl Generate Makefiles
dnl--------------------------------------------------------------------

AS_ECHO([" "])
AS_ECHO(["About to create Makefiles..."])
AC_CONFIG_FILES([\
      Makefile \
      src/Makefile \
      src/mbio/Makefile \
      src/surf/Makefile \
      src/bsio/Makefile \
      src/gsf/Makefile \
      src/mbaux/Makefile \
      src/utilities/Makefile \
      src/gmt/Makefile \
      src/macros/Makefile \
      src/mbedit/Makefile \
      src/mbnavedit/Makefile \
      src/mbvelocitytool/Makefile \
      src/mbnavadjust/Makefile \
      src/otps/Makefile \
      src/mbtrn/Makefile \
      src/mbview/Makefile \
      src/mbgrdviz/Makefile \
      src/mbeditviz/Makefile  \
      src/man/Makefile \
      src/man/man1/Makefile \
      src/man/man3/Makefile \
      src/html/Makefile \
      src/ps/Makefile \
      src/share/Makefile \
      third_party/Makefile \
      third_party/googletest/Makefile \
      third_party/googlemock/Makefile \
      test/Makefile \
      test/mbio/Makefile \
      test/utilities/Makefile \
      ])
AC_OUTPUT

dnl--------------------------------------------------------------------
dnl Summary Report
dnl--------------------------------------------------------------------
AS_ECHO([""])
AS_ECHO(["------------------------------------------------------------------------------"])
AS_ECHO(["MB-System Configure Summary"])
AS_ECHO(["------------------------------------------------------------------------------"])
AS_ECHO(["MB-System will be installed in: ${prefix}"])
AS_ECHO(["Installation host:              ${host}"])
if test "x$enable_static" = xyes ; then
    AS_ECHO(["Building Static Libs: Yes"])
else
    AS_ECHO(["Building Static Libs: No"])
fi
if test "x$enable_shared" = xyes ; then
    AS_ECHO(["Building Shared Libs: Yes"])
else
    AS_ECHO(["Building Shared Libs: No"])
fi
if test x"$hardening" != x"no" ; then
    AS_ECHO(["Hardening Enabled: Yes"])
    AS_ECHO(["HARDEN_CFLAGS:     $HARDEN_CFLAGS"])
    AS_ECHO(["HARDEN_LDFLAGS:    $HARDEN_LDFLAGS"])
    AS_ECHO(["HARDEN_BINCFLAGS:  $HARDEN_BINCFLAGS"])
    AS_ECHO(["HARDEN_BINLDFLAGS: $HARDEN_BINLDFLAGS"])
else
    AS_ECHO(["Hardening Enabled: No"])
fi
if test $BYTESWAP = "yes" ; then
    AS_ECHO(["Byteswapping: Enabled"])
else
    AS_ECHO(["Byteswapping: Disabled"])
fi
AS_ECHO(["libgmt_CPPFLAGS:  $libgmt_CPPFLAGS"])
AS_ECHO(["libgmt_LIBS:      $libgmt_LIBS"])
AS_ECHO(["libgmt_LDFLAGS:   $libgmt_LDFLAGS"])
AS_ECHO(["GMT PLUGIN DIR:   $GMT_PLUGINDIR"])
AS_ECHO(["libnetcdf_CPPFLAGS:  $libnetcdf_CPPFLAGS"])
AS_ECHO(["libnetcdf_LIBS:      $libnetcdf_LIBS"])
AS_ECHO(["libnetcdf_LDFLAGS:   $libnetcdf_LDFLAGS"])
AS_ECHO(["libproj_CPPFLAGS:    $libproj_CPPFLAGS"])
AS_ECHO(["libproj_LIBS:        $libproj_LIBS"])
AS_ECHO(["libproj_LDFLAGS:     $libproj_LDFLAGS"])

if test "$GOT_FFTW" = "yes" ; then
    AS_ECHO(["FFTW Support: Enabled"])
else
    AS_ECHO(["FFTW Support: Disabled"])
fi
if test "$GOT_MOTIF" = "yes" ; then
    AS_ECHO(["Motif Support: Enabled"])
else
    AS_ECHO(["Motif Support: Disabled"])
fi
if test "$GOT_OPENGL" = "yes" ; then
    AS_ECHO(["OpenGL Support: Enabled"])
else
    AS_ECHO(["OpenGL Support: Disabled"])
fi
AS_ECHO(["Using $OTPS_DIR for OSU Tidal Prediction Software"])
if test "$build_mbtrn" = "yes" ; then
    AS_ECHO(["Build mbtrn tools: Enabled"])
else
    AS_ECHO(["Build mbtrn tools: Disabled"])
fi
if test "$build_pcltools" = "yes" ; then
    AS_ECHO(["Build PCL tools: Enabled"])
else
    AS_ECHO(["Build PCL tools: Disabled"])
fi
if test "$build_test" = "yes" ; then
    AS_ECHO(["Build unit tests: Enabled"])
else
    AS_ECHO(["Build unit tests: Disabled"])
fi

AS_ECHO(["------------------------------------------------------------------------------"])
AS_ECHO(["Key Variables:"])
AS_ECHO(["    prefix:                   ${prefix}"])
AS_ECHO(["    exec_prefix:              ${exec_prefix}          $exec_prefix"])
AS_ECHO(["    datarootdir:              ${datarootdir}          $datarootdir"])
AS_ECHO(["    PACKAGE_TARNAME:          ${PACKAGE_TARNAME}"])
AS_ECHO(["------------------------------------------------------------------------------"])
AS_ECHO(["Installation Locations:"])
AS_ECHO(["    executables:              ${bindir}"])
AS_ECHO(["    libraries:                ${libdir}"])
AS_ECHO(["    header files:             ${includedir}"])
AS_ECHO(["    data files:               ${mbsystemdatadir}"])
AS_ECHO(["    man pages:                ${mandir}"])
AS_ECHO(["    Html documentation:       ${mbsystemhtmldir}"])
AS_ECHO(["    Postscript documentation: ${mbsystempsdir}"])
AS_ECHO(["------------------------------------------------------------------------------"])
AS_ECHO(["See config.log for details of configure results"])
AS_ECHO(["See INSTALL for further build instructions and hints"])
AS_ECHO(["------------------------------------------------------------------------------"])
