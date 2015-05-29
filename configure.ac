dnl--------------------------------------------------------------------
dnl    The MB-system:	configure.ac	12/7/2011
dnl    $Id$
dnl
dnl    Copyright (c) 2011-2015 by
dnl    D. W. Caress (caress@lamont.ldgo.columbia.edu)
dnl    and D. N. Chayes (dale@lamont.ldgo.columbia.edu)
dnl    Lamont-Doherty Earth Observatory
dnl    Palisades, NY  10964
dnl
dnl    See README file for copying and redistribution conditions.
dnl--------------------------------------------------------------------

dnl Require automake 2.65 or later
AC_PREREQ([2.65])

dnl Disable configure caching
define([AC_CACHE_LOAD], )
define([AC_CACHE_SAVE], )

dnl Initialize and set version and version date
AC_INIT([mbsystem],[5.5.2247],[http://listserver.mbari.org/sympa/arc/mbsystem],[mbsystem],[http://www.mbari.org/data/mbsystem/])

AC_CONFIG_MACRO_DIR(m4)
AC_LANG(C)
AC_DEFINE(VERSION_DATE, "29 May 2015", [Set VERSION_DATE define in mb_config.h])

dnl Check system arch
AC_CANONICAL_HOST

AS_ECHO([" "])
AS_ECHO(["------------------------------------------------------------------------------"])
AS_ECHO(["    The MB-system:	configure	12/7/2011"])
AS_ECHO(["    $Id$"])
AS_ECHO([" "])
AS_ECHO(["    Copyright (c) 2011-2015 by"])
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
AS_ECHO(["        # Reset the autotools version to 2.65 to accomodate some Linux distributions"])
AS_ECHO(["        sed -i.bak s/2\.69/2\.65/ configure.ac"])
AS_ECHO([" "])
AS_ECHO(["    When you run ./configure, a number of configure options are saved to a"])
AS_ECHO(["    header file:"])
AS_ECHO(["        ./src/mbio/mb_config.h"])
AS_ECHO(["    This file has a template:"])
AS_ECHO(["        ./src/mbio/mb_config.h.in"])
AS_ECHO(["    This file is conditionally included by:"])
AS_ECHO(["        ./src/mbio/mb_define.h"])
AS_ECHO(["    which is in turn included by essentially every MB-System C source file."])
AS_ECHO([" "])
AS_ECHO(["------------------------------------------------------------------------------"])
AS_ECHO(["    To use the build system..."])
AS_ECHO(["------------------------------------------------------------------------------"])
AS_ECHO(["    To generate the makefiles needed to build MB-System, run ./configure"])
AS_ECHO(["    with the options appropriate for your situation. These options are"])
AS_ECHO(["    listed here:"])
AS_ECHO(["        --prefix=install      - location for mbsystem (/usr/local/mbsystem)"])
AS_ECHO(["        --with-netcdf-lib     - location of NetCDF libs"])
AS_ECHO(["        --with-netcdf-include - location of NetCDF headers"])
AS_ECHO(["        --with-proj-lib       - location of PROJ libs"])
AS_ECHO(["        --with-proj-include   - location of PROJ headers"])
AS_ECHO(["        --with-gmt-lib        - location of GMT libs"])
AS_ECHO(["        --with-gmt-include    - location of GMT headers"])
AS_ECHO(["        --with-fftw-lib       - location of FFTW3 libs (optional)"])
AS_ECHO(["        --with-fftw-include   - location of FFTW3 headers (optional)"])
AS_ECHO(["        --with-motif-lib      - location of Motif libs (optional)"])
AS_ECHO(["        --with-motif-include  - location of Motif headers (optional)"])
AS_ECHO(["        --with-opengl-lib     - location of OpenGL libs (optional)"])
AS_ECHO(["        --with-opengl-include - location of OpenGL headers (optional)"])
AS_ECHO(["        --with-otps-dir       - location of OTPS installation (optional)"])
AS_ECHO(["        --enable-bundledproj  - build using bundled proj package - default is to link with libproj"])
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

AM_INIT_AUTOMAKE
AM_MAINTAINER_MODE
AC_CONFIG_HEADERS([src/mbio/mb_config.h])

dnl Check for programs
AC_PROG_CC
AM_PROG_CC_C_O
AC_PROG_INSTALL
AM_PROG_AS
m4_ifdef([AM_PROG_AR], [AM_PROG_AR])
AC_PROG_LN_S
AC_PROG_MAKE_SET
LT_INIT
AC_SUBST([LIBTOOL_DEPS])
LT_LIB_M

dnl Check Endian size
BYTESWAP=no
AC_C_BIGENDIAN([BYTESWAP=no], [BYTESWAP=yes],
	AC_MSG_ERROR(Unknown endian state),
	AC_MSG_ERROR(Universial endianess not supported)
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
	AC_DEFINE(WIN32, 1, [Turned on WIN32 define in config])
	AS_ECHO(["Added WIN32 define to mb_config.h"])
	;;
	*)
	AS_ECHO(["Not WIN32 system"])
	;;
esac

dnl check for RPC XDR headers needed by mbio lib
AC_CHECK_HEADERS(rpc/rpc.h)
AC_CHECK_HEADERS(rpc/types.h, ,
                 [AC_MSG_ERROR([Required header <rpc/types.h> not found, check include path and intsalled devel packages])],)

dnl for win32
dnl AC_CHECK_LIB(uuid, main)
dnl AC_CHECK_LIB(ole32, main)
dnl AC_CHECK_LIB(gw32c, main)
dnl AC_CHECK_LIB(wsock32, main)
AC_CHECK_LIB(ws2_32, main)

AC_CHECK_FUNC(xdrmem_create, XDR_LIB=, [
 AC_CHECK_LIB(sun, xdrmem_create, XDR_LIB=-lsun, [
  AC_CHECK_LIB(nsl, xdrmem_create, XDR_LIB=-lnsl, [
   AC_CHECK_LIB(rpclib, xdrmem_create, XDR_LIB=-lrpclib, [
    AC_CHECK_LIB(xdr, xdrmem_create, XDR_LIB=-lxdr, [
     AC_CHECK_LIB(portablexdr, xdrmem_create, XDR_LIB=-lportablexdr,
   [ AC_MSG_ERROR([*** Unable to locate XDR functions.]) ], -lws2_32 )
], -lws2_32 )])])])])
AC_SUBST(XDR_LIB)

dnl--------------------------------------------------------------------
dnl OTPS Section
dnl--------------------------------------------------------------------

dnl Use OTPS (OSU Tidal Prediction Software) location if specified with --with-otps_dir, use default otherwise
AS_ECHO(["Use OTPS (OSU Tidal Prediction Software) location if specified with --with-otps_dir, use default otherwise"])
OTPS_DIR="/usr/local/OTPSnc"
AC_ARG_WITH(otps_dir, [ --with-otps_dir=DIR    Location of OSU Tidal Prediction Software], [OTPS_DIR=${withval}])
AC_SUBST(OTPS_DIR)

dnl--------------------------------------------------------------------
dnl NetCDF Section
dnl--------------------------------------------------------------------

dnl Check for netCDF library
AS_ECHO(["Check for netCDF library libnetcdf"])
GOT_CDF=no
CDF_SETTING=no
dnl First check if netcdf location specified on the command line
AC_ARG_WITH(netcdf-lib, [  --with-netcdf-lib=DIR	Location of NetCDF library],
	[netcdf_libdir="${withval}"] [GOT_CDF=yes], [GOT_CDF=no])
if test "$GOT_CDF" = "yes" ; then
    dnl NetCDF library location specified: $netcdf_libdir - check if libnetcdf is there...
    AS_ECHO(["NetCDF library location specified: $netcdf_libdir - check if libnetcdf is there..."])
    save_LDFLAGS=$LDFLAGS
    LDFLAGS="$LDFLAGS -L$netcdf_libdir"
    AC_CHECK_LIB([netcdf], [nc_open], [CDF_SETTING=yes], [CDF_SETTING=no], )
    LDFLAGS="$save_LDFLAGS"
    if test "$CDF_SETTING" = "no"; then
	AC_MSG_ERROR([Did not find libnetcdf according to --with-netcdf-lib - check the installation!])
    fi
    libnetcdf_LIBS="-L$netcdf_libdir -R $netcdf_libdir -lnetcdf"
    libnetcdf_LDFLAGS="-L$netcdf_libdir"
else
    dnl Look for pkg-config netCDF package...
    AS_ECHO(["Look for pkg-config netCDF package..."])
    PKG_CHECK_MODULES([libnetcdf], [netcdf], [GOT_CDF=yes], [GOT_CDF=no])
    libnetcdf_LIBS="-lnetcdf"
    libnetcdf_LDFLAGS=""
fi
if test "$GOT_CDF" = "no" ; then
    dnl Did not find netCDF library pkg-config package, looking in the usual places...
    AS_ECHO(["Did not find netCDF library pkg-config package, looking in the usual places..."])
    AC_CHECK_LIB([netcdf], [nc_open], [CDF_SETTING=yes], [CDF_SETTING=no],)
    if test "$CDF_SETTING" = "yes" ; then
        GOT_CDF=yes
        AS_ECHO(["Using pre-installed libnetcdf..."])
        libnetcdf_LIBS="-lnetcdf"
        libnetcdf_LDFLAGS=""
    else
        AS_ECHO(["Did not find netCDF library anywhere, use --with-netcdf-lib to set the location"])
        AC_MSG_ERROR([Did not find netCDF library anywhere, use --with-netcdf-lib to set the location])
    fi
fi
dnl Done checking for netCDF library libnetcdf

dnl Check for netCDF headers
AS_ECHO(["Check for netCDF headers"])
GOT_CDF=no
CDF_SETTING=no
dnl First check if netcdf location specified on the command line
AC_ARG_WITH(netcdf-include, [  --with-netcdf-include=DIR	Location of NetCDF headers],
	[libnetcdf_CPPFLAGS="-I${withval}"] [GOT_CDF=yes], [GOT_CDF=no])
if test "$GOT_CDF" = "yes" ; then
    dnl NetCDF header location specified: $libnetcdf_CPPFLAGS - check if netcdf.h is there...
    AS_ECHO(["NetCDF header location specified: $libnetcdf_CPPFLAGS - check if netcdf.h is there..."])
    save_CPPFLAGS=$CPPFLAGS
    save_CFLAGS=$CFLAGS
    CPPFLAGS="$CPPFLAGS $libnetcdf_CPPFLAGS"
    CFLAGS="$CFLAGS $libnetcdf_CPPFLAGS"
    AC_CHECK_HEADER(netcdf.h, [CDF_SETTING=yes], [CDF_SETTING=no],)
    CPPFLAGS=$save_CPPFLAGS
    CFLAGS=$save_CFLAGS
    if test "$CDF_SETTING" = "no"; then
	AC_MSG_ERROR([Did not find netcdf.h according to --with-netcdf-include - check the installation!])
    fi
else
    dnl Looking for netCDF headers in the usual places...
    AS_ECHO(["Looking for netCDF headers in the usual places..."])
    AC_CHECK_HEADER(netcdf.h, [CDF_SETTING=yes], [CDF_SETTING=no])
    if test "$CDF_SETTING" = "yes" ; then
        GOT_CDF=yes
        AS_ECHO(["Using pre-installed netCDF headers..."])
    else
        AS_ECHO(["Did not find netCDF headers anywhere, use --with-netcdf-include to set the location"])
        AC_MSG_ERROR([Did not find netCDF headers anywhere, use --with-netcdf-include to set the location])
    fi
fi
dnl Done checking for netCDF headers

dnl Substitute netCDF library and header arguments
AC_SUBST([libnetcdf_LIBS], [$libnetcdf_LIBS])
AC_SUBST([libnetcdf_CPPFLAGS], [$libnetcdf_CPPFLAGS])

dnl--------------------------------------------------------------------
dnl PROJ Section
dnl--------------------------------------------------------------------

dnl Check whether to use bundled proj package or link with external libproj
AS_ECHO(["Check whether to use bundled proj package or link with external libproj"])
AC_ARG_ENABLE(bundledproj, [AS_HELP_STRING([--enable-bundledproj], [build and use the bundled proj package])],
        [bundledproj=yes], [bundledproj=no])
AM_CONDITIONAL([BUILD_PROJ], [test x"$bundledproj" = xyes])
if test "$bundledproj" = "yes" ; then
    dnl Using bundled PROJ library
    AS_ECHO(["Using bundled PROJ library"])
    libproj_LIBS="-lproj"
    libproj_LDFLAGS=
else
    dnl Check if proj library location specified on the command line
    AS_ECHO(["Check if proj library location specified on the command line"])
    GOT_PROJ=no
    PROJ_SETTING=no
    AC_ARG_WITH(proj-lib, [  --with-proj-lib=DIR	Location of PROJ library],
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
    
    dnl Check if proj header location specified on the command line
    AS_ECHO(["Check if proj header location specified on the command line"])
    GOT_PROJ=no
    PROJ_SETTING=no
    AC_ARG_WITH(proj-include, [  --with-proj-include=DIR	Location of PROJ headers],
            [libproj_CPPFLAGS="-I${withval}"] [GOT_PROJ=yes], [GOT_PROJ=no])
    if test "$GOT_PROJ" = "yes" ; then
        dnl PROJ header location specified: $libproj_CPPFLAGS - check if proj_api.h is there...
        AS_ECHO(["PROJ header location specified: $libproj_CPPFLAGS - check if proj_api.h is there..."])
        save_CPPFLAGS=$CPPFLAGS
        save_CFLAGS=$CFLAGS
        CPPFLAGS="$CPPFLAGS $libproj_CPPFLAGS"
        CFLAGS="$CFLAGS $libproj_CPPFLAGS"
        AC_CHECK_HEADER(proj_api.h, [PROJ_SETTING=yes], [PROJ_SETTING=no],)
        CPPFLAGS=$save_CPPFLAGS
        CFLAGS=$save_CFLAGS
        if test "$PROJ_SETTING" = "no"; then
           AC_MSG_ERROR([Did not find PROJ headers according to --with-proj-include - check the installation!])
        fi
    else
        dnl Looking for PROJ headers in the usual places...
        AS_ECHO(["Looking for PROJ headers in the usual places..."])
        AC_CHECK_HEADER(proj_api.h, [PROJ_SETTING=yes], [PROJ_SETTING=no])
        if test "$PROJ_SETTING" = "yes" ; then
            GOT_PROJ=yes
            AS_ECHO(["Using pre-installed PROJ headers..."])
        else
            AS_ECHO(["Did not find PROJ headers anywhere, use --with-proj-include to set the location"])
            AC_MSG_ERROR([Did not find PROJ headers anywhere, use --with-proj-include to set the location])
        fi
    fi
     dnl Done checking for PROJ headers
fi
dnl Done checking for PROJ library libproj and PROJ headers

dnl Substitute PROJ library and header arguments
AC_SUBST([libproj_LIBS], [$libproj_LIBS])
AC_SUBST([libproj_CPPFLAGS], [$libproj_CPPFLAGS])

dnl--------------------------------------------------------------------
dnl GMT Section
dnl--------------------------------------------------------------------

dnl Check for GMT libraries libgmt libpsl
AS_ECHO(["Check for GMT libraries libgmt libpsl"])
GOT_GMT=no
GMT_SETTING=no
dnl First check if GMT library location specified on the command line
AC_ARG_WITH(gmt-lib, [  --with-gmt-lib=DIR	Location of GMT libraries libgmt libpsl],
	[gmt_libdir="${withval}"] [GOT_GMT=yes], [GOT_GMT=no])
if test "$GOT_GMT" = "yes" ; then
    dnl GMT library location specified: $gmt_libdir - check if libgmt libpsl are there...
    AS_ECHO(["GMT library location specified: $gmt_libdir - check if libgmt libpsl are there..."])
    save_LDFLAGS=$LDFLAGS
    LDFLAGS="$LDFLAGS -L$gmt_libdir"
    AC_CHECK_LIB([gmt], [GMT_begin], [GMT_SETTING=yes], [GMT_SETTING=no], )
    LDFLAGS="$save_LDFLAGS"
    if test "$GMT_SETTING" = "no"; then
	AC_MSG_ERROR([Did not find libgmt according to --with-gmt-lib - check the installation!])
    fi
    libgmt_LIBS="-L$gmt_libdir -R $gmt_libdir -lgmt -lpsl"
    libgmt_LDFLAGS="-L$gmt_libdir"
else
    dnl Look for pkg-config GMT package...
    AS_ECHO(["Look for pkg-config GMT package..."])
    PKG_CHECK_MODULES([libgmt], [gmt], [GOT_LIBGMT=yes], [GOT_LIBGMT=no])
    PKG_CHECK_MODULES([libpsl], [gmt], [GOT_LIBPSL=yes], [GOT_LIBPSL=no])
    if test "$GOT_LIBGMT" = "yes" ; then
            GOT_GMT=yes
    fi
    if test "$GOT_PSL" = "no" ; then
            GOT_GMT=no
    fi
    libgmt_LIBS="-lgmt -lpsl"
    libgmt_LDFLAGS=""
fi
if test "$GOT_GMT" = "no" ; then
    dnl Did not find GMT library pkg-config package, looking in the usual places...
    AS_ECHO(["Did not find GMT library pkg-config package, looking in the usual places..."])
    AC_CHECK_LIB([gmt], [GMT_begin], [GMT_SETTING=yes], [GMT_SETTING=no],)
    if test "$GMT_SETTING" = "yes" ; then
        GOT_GMT=yes
        AS_ECHO(["Using pre-installed libgmt..."])
        libgmt_LIBS="-lgmt -lpsl"
        libgmt_LDFLAGS=""
    else
        AS_ECHO(["Did not find GMT libraries anywhere, use --with-gmt-lib to set the location"])
        AC_MSG_ERROR([Did not find GMT libraries anywhere, use --with-gmt-lib to set the location])
    fi
fi
dnl Done checking for GMT libraries libgmt libpsl

dnl Check for GMT headers
AS_ECHO(["Check for GMT headers"])
GOT_GMT=no
GMT_SETTING=no
dnl First check if GMT header location specified on the command line
AC_ARG_WITH(gmt-include, [  --with-gmt-include=DIR	Location of GMT headers],
	[libgmt_CPPFLAGS="-I${withval}"] [GOT_GMT=yes], [GOT_GMT=no])
if test "$GOT_GMT" = "yes" ; then
    dnl GMT header location specified: $libgmt_CPPFLAGS - check if gmt.h is there...
    AS_ECHO(["GMT header location specified: $libgmt_CPPFLAGS - check if gmt.h is there..."])
    save_CPPFLAGS=$CPPFLAGS
    save_CFLAGS=$CFLAGS
    CPPFLAGS="$CPPFLAGS $libgmt_CPPFLAGS"
    CFLAGS="$CFLAGS $libgmt_CPPFLAGS"
    AC_CHECK_HEADER(gmt.h, [GMT_SETTING=yes], [GMT_SETTING=no],)
    CPPFLAGS=$save_CPPFLAGS
    CFLAGS=$save_CFLAGS
    if test "$GMT_SETTING" = "no"; then
	AC_MSG_ERROR([Did not find gmt.h according to --with-gmt-include - check the installation!])
    fi
else
    dnl Looking for GMT headers in the usual places...
    AS_ECHO(["Looking for GMT headers in the usual places..."])
    AC_CHECK_HEADER(gmt.h, [GMT_SETTING=yes], [GMT_SETTING=no])
    if test "$GMT_SETTING" = "yes" ; then
        GOT_GMT=yes
        AS_ECHO(["Using pre-installed GMT headers..."])
    else
        AS_ECHO(["Did not find GMT headers anywhere, use --with-gmt-include to set the location"])
        AC_MSG_ERROR([Did not find GMT headers anywhere, use --with-gmt-include to set the location])
    fi
fi
dnl Done checking for GMT headers

dnl Substitute PROJ library and header arguments
AC_SUBST([libgmt_LIBS], [$libgmt_LIBS])
AC_SUBST([libgmt_CPPFLAGS], [$libgmt_CPPFLAGS])

dnl--------------------------------------------------------------------
dnl FFTW Section
dnl--------------------------------------------------------------------

dnl Check for FFTW library libfftw3
AS_ECHO(["Check for FFTW library libfftw3"])
GOT_FFTW=no
FFTW_SETTING=no
dnl First check if fftw location specified on the command line
AC_ARG_WITH(fftw-lib, [  --with-fftw-lib=DIR	Location of FFTW library],
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
AC_ARG_WITH(fftw-include, [  --with-fftw-include=DIR	Location of FFTW headers],
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
    AC_ARG_WITH(motif-lib, [  --with-motif-lib=DIR	Location of MOTIF library],
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
        AC_ARG_WITH(motif-include, [  --with-motif-include=DIR	Location of Motif headers],
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
    AC_ARG_WITH(opengl-lib, [  --with-opengl-lib=DIR	Location of OpenGL library],
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
        AC_ARG_WITH(opengl-include, [  --with-opengl-include=DIR	Location of OpenGL headers],
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
dnl Generate Makefiles
dnl--------------------------------------------------------------------

AS_ECHO([" "])
AS_ECHO(["About to create Makefiles..."])
AC_CONFIG_FILES([Makefile src/Makefile src/mbio/Makefile \
	  src/surf/Makefile \
	  src/bsio/Makefile \
          src/gsf/Makefile \
          src/mbaux/Makefile \
	  src/utilities/Makefile src/gmt/Makefile \
	  src/macros/Makefile src/mbedit/Makefile src/mbnavedit/Makefile \
	  src/mbvelocitytool/Makefile src/mbnavadjust/Makefile \
	  src/otps/Makefile \
	  src/mbview/Makefile  src/mbgrdviz/Makefile  src/mbeditviz/Makefile  \
	  src/man/Makefile src/man/man1/Makefile src/man/man3/Makefile \
	  src/html/Makefile src/ps/Makefile src/share/Makefile])
if test "$bundledproj" = "yes" ; then
AC_CONFIG_FILES([src/proj/Makefile])
fi
AC_OUTPUT

dnl Summary Report
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
if test $BYTESWAP = "yes" ; then
	AS_ECHO(["Byteswapping: Enabled"])
else
	AS_ECHO(["Byteswapping: Disabled"])
fi
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

if test "$bundledproj" = "yes" ; then
	AS_ECHO(["Building using bundled proj package"])
else
	AS_ECHO(["Building using system libproj library, not bundled proj package"])
fi

AS_ECHO(["------------------------------------------------------------------------------"])
AS_ECHO(["Key Variables:"])
AS_ECHO(["    prefix:                   ${prefix}"])
AS_ECHO(["    exec_prefix:              ${exec_prefix}"])
AS_ECHO(["    datarootdir:              ${datarootdir}"])
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
