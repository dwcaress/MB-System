dnl--------------------------------------------------------------------
dnl    The MB-system:	configure.ac	12/7/2011
dnl    $Id$
dnl
dnl    Copyright (c) 2011-2014 by
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
AC_INIT([mbsystem],[5.4.2212],[http://listserver.mbari.org/sympa/arc/mbsystem],[mbsystem],[http://www.mbari.org/data/mbsystem/])

AC_CONFIG_MACRO_DIR(m4)
AC_LANG(C)
AC_DEFINE(VERSION_DATE, "12 November 2014", [Set VERSION_DATE define in mb_config.h])

dnl Check system arch
AC_CANONICAL_HOST

AS_ECHO([" "])
AS_ECHO(["------------------------------------------------------------------------------"])
AS_ECHO(["    The MB-system:	configure	12/7/2011"])
AS_ECHO(["    $Id$"])
AS_ECHO([" "])
AS_ECHO(["    Copyright (c) 2011-2014 by"])
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
AS_ECHO(["    When you run ./configure, a number of configure options are saved  to a"])
AS_ECHO(["    header file:"])
AS_ECHO(["        ./src/mbio/mb_config.h"])
AS_ECHO(["    This file has a template:"])
AS_ECHO(["        ./src/mbio/mb_config.h.in"])
AS_ECHO(["    This file is conditionally included by:"])
AS_ECHO(["        ./src/mbio/mb_define.h"])
AS_ECHO(["    which is in turn included by essentially every MB-System C source file."])
AS_ECHO(["    The condition under which mb_config.h is used is simply the use of the"])
AS_ECHO(["    configure script to generate the makefiles. If one uses the alternate"])
AS_ECHO(["    install_makefiles build system, then an alternate header file named"])
AS_ECHO(["        ./src/mbio/mb_config2.h"])
AS_ECHO(["    is used instead."])
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
AS_ECHO(["        --without-gsf         - build without including or supporting GSF"])
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

dnl check for OTPS arg
dnl TODO add actual check and disable if not found
AS_ECHO(["Check for OSU Tidal Prediction Software"])
dnl initialize with defaut
OTPS_DIR="/usr/local/OTPSnc"
dnl check for command line option and reset
AC_ARG_WITH(otps_dir, [  --with-otps_dir=DIR        Location of OSU Tidal Prediction Software], [OTPS_DIR=${withval}])
AC_SUBST(OTPS_DIR)

AC_MSG_CHECKING([whether to build with Generic Sensor Format (GSF)])
AC_ARG_WITH([gsf], AS_HELP_STRING([--without-gsf], [Disable unlicensed SAIC Generic Sensor Format (GSF)]), [ ], [with_gsf=yes])
AC_MSG_RESULT([$with_gsf])

AS_IF([test x"$with_gsf" = xyes], [AC_DEFINE(WITH_GSF, 1, [Build with GSF])])
AM_CONDITIONAL([BUILD_GSF], [test x"$with_gsf" = xyes])

dnl Check for NetCDF lib
AS_ECHO(["Check for NetCDF"])
GOT_CDF=no
CDF_SETTING=no
dnl first check for package
PKG_CHECK_MODULES([libnetcdf], [netcdf], [GOT_CDF=yes], [GOT_CDF=no])

dnl Package check failed so check manually
if test "$GOT_CDF" = "no" ; then

libnetcdf_LIBS="-lnetcdf"
libnetcdf_LDFLAGS=

dnl arg for command line
AC_ARG_WITH(netcdf-lib, [  --with-netcdf-lib=DIR	Location of NetCDF library],
	[netcdf_libdir="${withval}"] [GOT_CDF=yes], [GOT_CDF=no])
dnl First check if lib installed
if test "$GOT_CDF" = "no" ; then
 AC_CHECK_LIB([netcdf], [nc_open], [CDF_SETTING=yes], [CDF_SETTING=no],)
fi
 if test "$CDF_SETTING" = "yes" ; then
    GOT_CDF=yes
    AS_ECHO(["Using pre-installed libcdf."])
 else
    if test "$GOT_CDF" = "no" ; then
	AC_MSG_ERROR([Did not find libnetcdf, use --with-netcdf-lib])
    else
	AS_ECHO(["Looking for libnetcdf in $netcdf_libdir ..."])
	save_LDFLAGS=$LDFLAGS
	LDFLAGS="$LDFLAGS -L$netcdf_libdir"
	AC_CHECK_LIB([netcdf], [nc_open], [CDF_SETTING=yes], [CDF_SETTING=no], )
	LDFLAGS="$save_LDFLAGS"
	if test "$CDF_SETTING" = "no"; then
	   AC_MSG_ERROR([Did not find libnetcdf, use --with-netcdf-lib or check installation])
	fi
	libnetcdf_LIBS="-L$netcdf_libdir -R $netcdf_libdir $libnetcdf_LIBS"
#
#       Save the netcdf library path for later gmt tests
#
	libnetcdf_LDFLAGS="-L$netcdf_libdir"
    fi
 fi
 dnl Use ac_subst to set vars that would be set by pkg_check
 AC_SUBST([libnetcdf_LIBS], ["$libnetcdf_LIBS"])
fi
dnl done with CDF lib

dnl Now check CDF headers
GOT_CDF=no
CDF_SETTING=no
dnl add command line option
AC_ARG_WITH(netcdf-include, [  --with-netcdf-include=DIR	Location of NetCDF headers],
	[libnetcdf_CPPFLAGS="-I${withval}"] [GOT_CDF=yes], [GOT_CDF=no])
dnl First check if cdf headers installed
if test "$GOT_CDF" = "no" ; then
 AC_CHECK_HEADER(netcdf.h, [CDF_SETTING=yes], [CDF_SETTING=no])
fi
 if test "$CDF_SETTING" = "yes" ; then
    GOT_CDF=yes
    AS_ECHO(["Using pre-installed NetCDF headers."])
 else
    if test "$GOT_CDF" = "no" ; then
	AC_MSG_ERROR([Did not find required NetCDF headers, use --with-netcdf-include])
    else
	AS_ECHO(["Checking netcdf.h in $libnetcdf_CPPFLAGS..."])
	save_CPPFLAGS=$CPPFLAGS
	save_CFLAGS=$CFLAGS
	CPPFLAGS="$CPPFLAGS $libnetcdf_CPPFLAGS"
	CFLAGS="$CFLAGS $libnetcdf_CPPFLAGS"
	AC_CHECK_HEADER(netcdf.h, [CDF_SETTING=yes], [CDF_SETTING=no],)
	CPPFLAGS=$save_CPPFLAGS
	CFLAGS=$save_CFLAGS
	if test "$CDF_SETTING" = "no"; then
	   AC_MSG_ERROR([Did not find required NetCDF headers, use --with-netcdf-include])
	fi
    fi
 fi
AC_SUBST([libnetcdf_CPPFLAGS], [$libnetcdf_CPPFLAGS])
dnl done with CDF headers


dnl check whether to use bundled proj package or link with external libproj
AC_ARG_ENABLE(bundledproj, [AS_HELP_STRING([--enable-bundledproj], [build and use the bundled proj package])],
        [bundledproj=yes], [bundledproj=no])
AM_CONDITIONAL([BUILD_PROJ], [test x"$bundledproj" = xyes])
        
dnl Using bundled PROJ
if test "$bundledproj" = "yes" ; then
    AS_ECHO(["Using bundled PROJ lib"])
    libproj_LIBS="-lproj"
    libproj_LDFLAGS=
    AC_SUBST([libproj_LIBS], ["$libproj_LIBS"])

dnl Linking with external PROJ
else

    dnl Check for PROJ lib
    AS_ECHO(["Check for PROJ"])
    GOT_PROJ=no
    PROJ_SETTING=no
    dnl first check for package
    PKG_CHECK_MODULES([libproj], [proj], [GOT_PROJ=yes], [GOT_PROJ=no])
    
    dnl Package check failed so check manually
    if test "$GOT_PROJ" = "no" ; then
    
    libproj_LIBS="-lproj"
    libproj_LDFLAGS=
    
    dnl arg for command line
    AC_ARG_WITH(proj-lib, [  --with-proj-lib=DIR	Location of PROJ library],
            [proj_libdir="${withval}"] [GOT_PROJ=yes], [GOT_PROJ=no])
    dnl First check if lib installed
    if test "$GOT_PROJ" = "no" ; then
     AC_CHECK_LIB([proj], [pj_init_plus], [PROJ_SETTING=yes], [PROJ_SETTING=no],)
    fi
     if test "$PROJ_SETTING" = "yes" ; then
        GOT_PROJ=yes
        AS_ECHO(["Using pre-installed libproj."])
     else
        if test "$GOT_PROJ" = "no" ; then
            AC_MSG_ERROR([Did not find libproj, use --with-proj-lib])
        else
            AS_ECHO(["Looking for libproj in $proj_libdir ..."])
            save_LDFLAGS=$LDFLAGS
            LDFLAGS="$LDFLAGS -L$proj_libdir"
            AC_CHECK_LIB([proj], [pj_init_plus], [PROJ_SETTING=yes], [PROJ_SETTING=no], )
            LDFLAGS="$save_LDFLAGS"
            if test "$PROJ_SETTING" = "no"; then
               AC_MSG_ERROR([Did not find libproj, use --with-proj-lib or check installation])
            fi
            libproj_LIBS="-L$proj_libdir -R $proj_libdir $libproj_LIBS"
    #
    #       Save the proj library path for later gmt tests
    #
            libproj_LDFLAGS="-L$proj_libdir"
        fi
     fi
     dnl Use ac_subst to set vars that would be set by pkg_check
     AC_SUBST([libproj_LIBS], ["$libproj_LIBS"])
    fi
    dnl done with PROJ lib

    dnl Now check PROJ headers
    GOT_PROJ=no
    PROJ_SETTING=no
    dnl add command line option
    AC_ARG_WITH(proj-include, [  --with-proj-include=DIR	Location of PROJ headers],
            [libproj_CPPFLAGS="-I${withval}"] [GOT_PROJ=yes], [GOT_PROJ=no])
    dnl First check if PROJ headers installed
    if test "$GOT_PROJ" = "no" ; then
        AC_CHECK_HEADER(proj_api.h, [PROJ_SETTING=yes], [PROJ_SETTING=no])
    fi
    if test "$PROJ_SETTING" = "yes" ; then
        GOT_PROJ=yes
        AS_ECHO(["Using pre-installed PROJ headers."])
    else
        if test "$GOT_PROJ" = "no" ; then
            AC_MSG_ERROR([Did not find required PROJ headers, use --with-proj-include])
        else
            AS_ECHO(["Checking proj_api.h in $libproj_CPPFLAGS..."])
            save_CPPFLAGS=$CPPFLAGS
            save_CFLAGS=$CFLAGS
            CPPFLAGS="$CPPFLAGS $libproj_CPPFLAGS"
            CFLAGS="$CFLAGS $libproj_CPPFLAGS"
            AC_CHECK_HEADER(proj_api.h, [PROJ_SETTING=yes], [PROJ_SETTING=no],)
            CPPFLAGS=$save_CPPFLAGS
            CFLAGS=$save_CFLAGS
            if test "$PROJ_SETTING" = "no"; then
               AC_MSG_ERROR([Did not find required PROJ headers, use --with-proj-include])
            fi
        fi
    fi
    AC_SUBST([libproj_CPPFLAGS], [$libproj_CPPFLAGS])
    dnl done with PROJ headers
    
fi
dnl Done linking with external PROJ

dnl Check for Generic Mapping Tools (GMT) libs
dnl - these are libgmt, libpsl, and libgmtps
dnl Would certainly be nice if GMT had pkg-config file (gmt.pc)...
AS_ECHO(["Check for Generic Mapping Tools (GMT)"])
GOT_GMTPKG=no
dnl first check for package
PKG_CHECK_MODULES([libgmt], [gmt], [GOT_LIBGMT=yes], [GOT_LIBGMT=no])
PKG_CHECK_MODULES([libgmtps], [gmt], [GOT_LIBGMTPS=yes], [GOT_LIBGMTPS=no])
PKG_CHECK_MODULES([libpsl], [gmt], [GOT_LIBPSL=yes], [GOT_LIBPSL=no])
if test "$GOT_LIBGMT" = "yes" ; then
	GOT_GMTPKG=yes
fi
if test "$GOT_GMTPS" = "no" ; then
	GOT_GMTPKG=no
fi
if test "$GOT_PSL" = "no" ; then
	GOT_GMTPKG=no
fi

dnl pkg-config check failed so try to find libraries and headers manually
if test "$GOT_GMTPKG" = "no" ; then

	GMT_SETTING=no
	GOT_GMTLIBS=no
	libgmt_LIBS="-lgmt -lgmtps -lpsl"
	dnl arg for command line
	AC_ARG_WITH(gmt-lib, [  --with-gmt-lib=DIR	Location of GMT libraries],
	       [gmt_libdir="${withval}"] [GOT_GMTLIBS=yes], [GOT_GMTLIBS=no])
	dnl First check if lib installed
	if test "$GOT_GMTLIBS" = "no" ; then
		AC_CHECK_LIB([gmt], [GMT_begin], [GMT_SETTING=yes], [GMT_SETTING=no],)
	fi
	if test "$GMT_SETTING" = "yes" ; then
		AS_ECHO(["Using pre-installed libgmt."])
	else
		if test "$GOT_GMTLIBS" = "no" ; then
			AC_MSG_ERROR([Did not find libgmt, use --with-gmt-lib])
		else
			AS_ECHO(["Looking for libgmt in $gmt_libdir ..."])
			save_LDFLAGS=$LDFLAGS
			save_LIBS=$LIBS
			LDFLAGS="$LDFLAGS -L$gmt_libdir $libnetcdf_LDFLAGS"
			LIBS="$LDFLAGS -lnetcdf $LIBM"
			AC_MSG_NOTICE(LDFLAGS $LDFLAGS)
			AC_MSG_NOTICE(LIBS $LIBS)
			AC_CHECK_LIB([gmt], [GMT_begin], [GMT_SETTING=yes], [GMT_SETTING=no], )
			LDFLAGS="$save_LDFLAGS"
			LIBS="$save_LIBS"
			if test "$GMT_SETTING" = "no"; then
				AC_MSG_ERROR([Did not find libgmt, use --with-gmt-lib or check installation])
			fi
			libgmt_LIBS="-L$gmt_libdir -R$gmt_libdir $libgmt_LIBS"
		fi
	fi

	dnl Now look for GMT headers
	GOT_GMTHEADERS=no
	GMT_SETTING=no
	libgmt_CPPFLAGS=
	dnl add command line option
	AC_ARG_WITH(gmt-include, [  --with-gmt-include=DIR	Location of GMT headers],
		[libgmt_CPPFLAGS="-I${withval}"] [GOT_GMTHEADERS=yes], [GOT_GMTHEADERS=no])

	dnl First check if gmt headers installed
	dnl Only if no arg given
	if test "$GOT_GMTHEADERS" = "no" ; then
		AC_CHECK_HEADER(gmt.h, [GMT_SETTING=yes], [GMT_SETTING=no])
	fi

	if test "$GMT_SETTING" = "yes" ; then
		GOT_GMTHEADERS=yes
		AS_ECHO(["Using pre-installed gmt headers."])
	else
		if test "$GOT_GMTHEADERS" = "no" ; then
			AC_MSG_ERROR([Did not find required GMT headers, use --with-gmt-include])
		else
			AS_ECHO(["Checking gmt.h in $libgmt_CPPFLAGS..."])
			save_CPPFLAGS=$CPPFLAGS
			save_CFLAGS=$CFLAGS
			CPPFLAGS="$CPPFLAGS $libgmt_CPPFLAGS $libnetcdf_CPPFLAGS"
			CFLAGS="$CFLAGS $libgmt_CPPFLAGS $libnetcdf_CPPFLAGS"
			AC_CHECK_HEADER(gmt.h, [GMT_SETTING=yes], [GMT_SETTING=no])
			CPPFLAGS=$save_CPPFLAGS
			CFLAGS=$save_CFLAGS
			if test "$GMT_SETTING" = "no"; then
				AC_MSG_ERROR([Did not find required GMT headers, use --with-gmt-include])
			fi
		fi
	fi
	AS_ECHO(["Setting libgmt_CPPFLAGS=$libgmt_CPPFLAGS"])
	AC_SUBST([libgmt_CPPFLAGS], [$libgmt_CPPFLAGS])
	dnl done with GMT headers
fi

AS_ECHO(["Setting libgmt_LIBS=$libgmt_LIBS"])
AC_SUBST([libgmt_LIBS], ["$libgmt_LIBS"])

dnl done with GMT


dnl Deal with FFTW for mbsegypsd
dnl seems to want fftw3 ... do not check for older fftw?
GOT_FFTW=no
FFTW_SETTING=no
USE_FFTW=
libfftw_CPPFLAGS=
dnl first check for package
PKG_CHECK_MODULES([libfftw], [fftw3], [GOT_FFTW=yes], [GOT_FFTW=no])

dnl Package check failed so check manually
if test "$GOT_FFTW" = "no" ; then

libfftw_LIBS="-lfftw3"
dnl arg for command line
AC_ARG_WITH(fftw-lib, [  --with-fftw-lib=DIR	Location of FFTW3 library],
	[fftw_libdir="${withval}"] [GOT_FFTW=yes], [GOT_FFTW=no])
dnl First check if lib installed
if test "$GOT_FFTW" = "no" ; then
 AC_CHECK_LIB([fftw3], [fftw_plan_dft_1d], [FFTW_SETTING=yes], [FFTW_SETTING=no])
fi
 if test "$FFTW_SETTING" = "yes" ; then
    GOT_FFTW=yes
    USE_FFTW=mbsegypsd
    AS_ECHO(["Using pre-installed FFTW3 lib."])
 else
    if test "$GOT_FFTW" = "no" ; then
	AS_ECHO(["Did not find libfftw3, use --with-fftw-lib, (FFTW disabled)"])
    else
	AS_ECHO(["Looking for libfftw3 in $libfftw_CFLAGS ..."])
	save_LDFLAGS=$LDFLAGS
	LDFLAGS="$LDFLAGS -L$fftw_libdir"
	AC_CHECK_LIB([fftw3], [fftw_plan_dft_1d], [FFTW_SETTING=yes], [FFTW_SETTING=no])
	LDFLAGS="$save_LDFLAGS"
	if test "$FFTW_SETTING" = "no"; then
	   	AS_ECHO(["libfftw not found, FFTW support is disabled"])
		GOT_FFTW=no
		USE_FFTW=
	else
		GOT_FFTW=yes
		USE_FFTW=mbsegypsd
		libfftw_LIBS="-L$fftw_libdir -R $fftw_libdir $libfftw_LIBS"
	fi
    fi
 fi
 if test "$GOT_FFTW" = "yes" ; then
	AC_SUBST([libfftw_LIBS], ["$libfftw_LIBS"])
 fi
fi
dnl done with FFTW lib


dnl Now check FFTW headers
dnl Only check for headers if lib check pass
if test "$GOT_FFTW" = "yes" ; then
GOT_FFTW=no
FFTW_SETTING=no
dnl add command line option
AC_ARG_WITH(fftw-include, [  --with-fftw-include=DIR	Location of FFTW headers],
	[libfftw_CPPFLAGS="-I${withval}"] [GOT_FFTW=yes], [GOT_FFTW=no])

dnl First check if fftw headers installed
if test "$GOT_FFTW" = "no" ; then
  AC_CHECK_HEADER(fftw3.h, [FFTW_SETTING=yes], [FFTW_SETTING=no],)
fi
 if test "$FFTW_SETTING" = "yes" ; then
    GOT_FFTW=yes
    AS_ECHO(["Using pre-installed fftw3 headers."])
 else
    if test "$GOT_FFTW" = "no" ; then
	AS_ECHO(["Did not find FFTW headers, use --with-fftw-inc, (FFTW Disabled)"])
    else
	AS_ECHO(["Checking fftw3.h in $libfftw_CPPFLAGS..."])
	save_CPPFLAGS=$CPPFLAGS
	save_CFLAGS=$CFLAGS
	CPPFLAGS="$CPPFLAGS $libfftw_CPPFLAGS"
	CFLAGS="$CFLAGS $libfftw_CPPFLAGS"
	AC_CHECK_HEADER(fftw3.h, [FFTW_SETTING=yes], [FFTW_SETTING=no],)
	CPPFLAGS=$save_CPPFLAGS
	CFLAGS=$save_CFLAGS
	if test "$FFTW_SETTING" = "no"; then
	   AS_ECHO(["Did not find FFTW headers, FFTW support disabled"])
	   GOT_FFTW=no
	   USE_FFTW=
	   FFTW_LIB=
	fi
    fi
 fi

if test "$GOT_FFTW" = "yes" ; then
	AC_SUBST([fftw_app], [$USE_FFTW])
	AC_SUBST([libfftw_CPPFLAGS], [$libfftw_CPPFLAGS])
fi
dnl done with FFTW headers
fi

AM_CONDITIONAL([BUILD_FFTW], [test "$GOT_FFTW" = yes])


dnl Deal with Motif for GUI
dnl Xm, Xt and X11
GOT_MOTIF=no
MOTIF_SETTING=no
motif_LIBS="-lXm -lXt -lX11"
motif_CPPFLAGS=
dnl arg for command line
AC_ARG_WITH(motif-lib, [  --with-motif-lib=DIR	Location of MOTIF library],
	[motif_libdir="${withval}"] [GOT_MOTIF=yes], [GOT_MOTIF=no])
dnl First check if lib installed
if test "$GOT_MOTIF" = "no" ; then
  AC_CHECK_LIB([Xm], [XmCreatePushButton], [MOTIF_SETTING=yes], [MOTIF_SETTING=no])
  if test "$MOTIF_SETTING" = "yes" ; then
	AC_CHECK_LIB([Xt], [XtAppMainLoop], [MOTIF_SETTING=yes], [MOTIF_SETTING=no])
	if test "$MOTIF_SETTING" = "yes" ; then
		AC_CHECK_LIB([X11], [XSendEvent], [MOTIF_SETTING=yes], [MOTIF_SETTING=no])
	fi
  fi
fi

dnl Now try from args
 if test "$MOTIF_SETTING" = "yes" ; then
    GOT_MOTIF=yes
    AS_ECHO(["Using pre-installed MOTIF libs."])
 else
    if test "$GOT_MOTIF" = "no" ; then
	AS_ECHO(["Did not find Motif libs (Xt, Xm, and X11), use --with-motif-lib, (Motif disabled)"])
    else
	AS_ECHO(["Looking for Motif Libs in $motif_libdir ..."])
	save_LDFLAGS=$LDFLAGS
	save_LIBS=$LIBS
	LDFLAGS="$LDFLAGS -L$motif_libdir"
	LIBS="$LIBS $motif_LIBS"
	AC_CHECK_LIB([Xm], [XmCreatePushButton], [MOTIF_SETTING=yes], [MOTIF_SETTING=no])
  	if test "$MOTIF_SETTING" = "yes" ; then
		AC_CHECK_LIB([Xt], [XtAppMainLoop], [MOTIF_SETTING=yes], [MOTIF_SETTING=no])
		if test "$MOTIF_SETTING" = "yes" ; then
			AC_CHECK_LIB([X11], [XSendEvent], [MOTIF_SETTING=yes], [MOTIF_SETTING=no])
		fi
  	fi
	LDFLAGS=$save_LDFLAGS
	LIBS=$save_LIBS
	if test "$MOTIF_SETTING" = "no"; then
	   	AS_ECHO(["Motif libs not found, Motif support is disabled"])
		GOT_MOTIF=no
	else
		GOT_MOTIF=yes
		motif_LIBS="-L$motif_libdir -R $motif_libdir $motif_LIBS"
	fi
    fi
 fi

if test "$GOT_MOTIF" = "yes" ; then
	AC_SUBST([motif_LIBS], ["$motif_LIBS"])
fi
dnl done with Motif libs

dnl Now check Motif headers only if lib check passed
if test "$GOT_MOTIF" = "yes" ; then
GOT_MOTIF=no
MOTIF_SETTING=no
dnl add command line option
AC_ARG_WITH(motif-include, [  --with-motif-include=DIR	Location of Motif headers],
	[motif_CPPFLAGS="-I${withval}"] [GOT_MOTIF=yes], [GOT_MOTIF=no])

dnl First check if Motif headers installed
if test "$GOT_MOTIF" = "no" ; then
  AC_CHECK_HEADER(X11/Intrinsic.h, [MOTIF_SETTING=yes], [MOTIF_SETTING=no],)
fi
 if test "$MOTIF_SETTING" = "yes" ; then
    GOT_MOTIF=yes
    AS_ECHO(["Using pre-installed Motif headers."])
 else
    if test "$GOT_MOTIF" = "no" ; then
	AS_ECHO(["Did not find Motif headers, use --with-motif-inc, (Motif Disabled)"])
    else
	AS_ECHO(["Checking X11/Intrinsic.h in $MOTIF_INC_DIR..."])
	save_CPPFLAGS=$CPPFLAGS
	save_CFLAGS=$CFLAGS
	CPPFLAGS="$CPPFLAGS $motif_CPPFLAGS"
	CFLAGS="$CFLAGS $motif_CPPFLAGS"
	AC_CHECK_HEADER(X11/Intrinsic.h, [MOTIF_SETTING=yes], [MOTIF_SETTING=no],)
	CPPFLAGS=$save_CPPFLAGS
	CFLAGS=$save_CFLAGS
	if test "$MOTIF_SETTING" = "no"; then
	   AS_ECHO(["Did not find Motif headers, Motif support disabled"])
	   GOT_MOTIF=no
	fi
    fi
 fi

if test "$GOT_MOTIF" = "yes" ; then
	AC_SUBST([motif_CPPFLAGS], [$motif_CPPFLAGS])
	AC_DEFINE(GOT_MOTIF, 1, [Turned on Motif define in config])
fi
dnl done with Motif headers
fi

AM_CONDITIONAL([BUILD_MOTIF], [test "$GOT_MOTIF" = yes])


dnl OpenGL Check
dnl Libs GL and GLU
GOT_GL=no

dnl Only check for GL if Motif Passed
if test "$GOT_MOTIF" = "yes" ; then

GL_SETTING=no
GL_LIBS="-lGL -lGLU"
GL_CPPFLAGS=
dnl arg for command line
AC_ARG_WITH(opengl-lib, [  --with-opengl-lib=DIR	Location of OpenGL library],
	[opengl_libdir="${withval}"] [GOT_GL=yes], [GOT_GL=no])
dnl First check if lib installed
if test "$GOT_GL" = "no" ; then
  AC_CHECK_LIB([GL], [main], [GL_SETTING=yes], [GL_SETTING=no])
  if test "$GL_SETTING" = "yes" ; then
	AC_CHECK_LIB([GLU], [main], [GL_SETTING=yes], [GL_SETTING=no], -lGL)
  fi
fi

dnl Now try from args
 if test "$GL_SETTING" = "yes" ; then
    GOT_GL=yes
    AS_ECHO(["Using pre-installed OpenGL libs."])
 else
    if test "$GOT_GL" = "no" ; then
	AS_ECHO(["Did not find OpenGL libs (GL and GLU), use --with-opengl-lib, (OpenGL disabled)"])
    else
	AS_ECHO(["Looking for OpenGL Libs in $opengl_libdir ..."])
	save_LDFLAGS=$LDFLAGS
	save_LIBS=$LIBS
	LDFLAGS="$LDFLAGS -L$opengl_libdir"
	LIBS="$LIBS $GL_LIBS"
	AC_CHECK_LIB([GL], [main], [GL_SETTING=yes], [GL_SETTING=no])
  	if test "$GL_SETTING" = "yes" ; then
		AC_CHECK_LIB([GLU], [main], [GL_SETTING=yes], [GL_SETTING=no])
  	fi
	LDFLAGS=$save_LDFLAGS
	LIBS=$save_LIBS
	if test "$GL_SETTING" = "no"; then
	   	AS_ECHO(["OpenGL libs not found, OpenGL support is disabled"])
		GOT_GL=no
	else
		GOT_GL=yes
		GL_LIBS="-L$opengl_libdir -R $opengl_libdir $GL_LIBS"
	fi
    fi
 fi

if test "$GOT_GL" = "yes" ; then
	AC_SUBST([GL_LIBS], ["$GL_LIBS"])
fi
dnl done with GL libs


dnl Now check OpenGL headers
dnl Only check for headers if lib passed
if test "$GOT_GL" = "yes" ; then
GOT_GL=no
GL_SETTING=no
dnl add command line option
AC_ARG_WITH(opengl-include, [  --with-opengl-include=DIR	Location of OpenGL headers],
	[GL_CPPFLAGS="-I${withval}"] [GOT_GL=yes], [GOT_GL=no])

dnl First check if OpenGL headers installed
if test "$GOT_GL" = "no" ; then
  AC_CHECK_HEADERS(GL/gl.h GL/glu.h GL/glx.h, [GL_SETTING=yes], [GL_SETTING=no])
fi
 if test "$GL_SETTING" = "yes" ; then
    GOT_GL=yes
    AS_ECHO(["Using pre-installed OpenGL headers."])
 else
    if test "$GOT_GL" = "no" ; then
	AS_ECHO(["Did not find OpenGL headers, use --with-opengl-inc, (OpenGL Disabled)"])
    else
	AS_ECHO(["Checking OpenGL Headers in $GL_CPPFLAGS..."])
	save_CPPFLAGS=$CPPFLAGS
	save_CFLAGS=$CFLAGS
	CPPFLAGS="$CPPFLAGS $GL_CPPFLAGS"
	CFLAGS="$CFLAGS $GL_CPPFLAGS"
	AC_CHECK_HEADERS(GL/gl.h GL/glu.h GL/glx.h, [GL_SETTING=yes], [GL_SETTING=no])
	CPPFLAGS=$save_CPPFLAGS
	CFLAGS=$save_CFLAGS
	if test "$GL_SETTING" = "no"; then
	   AS_ECHO(["Did not find OpenGL headers, OpenGL support disabled"])
	   GOT_GL=no
	fi
    fi
 fi

if test "$GOT_GL" = "yes" ; then
	AC_SUBST([GL_CPPFLAGS], [$GL_CPPFLAGS])
	AC_DEFINE(GOT_GL, 1, [Turned on OpenGL define in config])
fi
dnl done with OpenGL headers

fi
dnl done check for GL lib pass

fi
dnl Close Motif check for OpenGL

AM_CONDITIONAL([BUILD_GL], [test "$GOT_GL" = yes])

dnl variables for shared data
mbsystemdatadir="$datadir/$PACKAGE"
AC_SUBST(mbsystemdatadir)
mbsystemhtmldir="$docdir/html"
AC_SUBST(mbsystemhtmldir)
mbsystempsdir="$docdir/ps"
AC_SUBST(mbsystempsdir)

AS_ECHO([" "])
AS_ECHO(["About to create Makefiles..."])
AC_CONFIG_FILES([Makefile src/Makefile src/mbio/Makefile \
	  src/surf/Makefile \
	  src/bsio/Makefile \
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
if test "$with_gsf" = "yes" ; then
AC_CONFIG_FILES([src/gsf/Makefile])
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
if test "$GOT_GL" = "yes" ; then
	AS_ECHO(["OpenGL Support: Enabled"])
else
	AS_ECHO(["OpenGL Support: Disabled"])
fi
AS_ECHO(["Using $OTPS_DIR for OSU Tidal Prediction Software"])

AS_ECHO(["Build with Generic Sensor Format (GSF) Support: $with_gsf"])


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
