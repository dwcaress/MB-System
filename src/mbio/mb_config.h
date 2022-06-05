/* src/mbio/mb_config.h.  Generated from mb_config.h.in by configure.  */
/* src/mbio/mb_config.h.in.  Generated from configure.ac by autoheader.  */

/* Machine is littleendian, (Byteswapping on) */
#define BYTESWAPPED 1

/* Build with GSF */
#define ENABLE_GSF 1

/* Machine is bigendian, (Byteswapping off) */
/* #undef ENDIAN_BIG */

/* define if the compiler supports basic C++11 syntax */
#define HAVE_CXX11 1

/* Define to 1 if you have the <dlfcn.h> header file. */
#define HAVE_DLFCN_H 1

/* Define to 1 if you have the `gsincos' function. */
/* #undef HAVE_GSINCOS */

/* Define to 1 if you have the <inttypes.h> header file. */
#define HAVE_INTTYPES_H 1

/* Define to 1 if you have the `m' library (-lm). */
#define HAVE_LIBM 1

/* Define to 1 if you have the `pthread' library (-lpthread). */
#define HAVE_LIBPTHREAD 1

/* Have malloc.h */
/* #undef HAVE_MALLOC_H */

/* Have rpc/rpc.h */
#define HAVE_RPC_RPC_H 1

/* Define to 1 if you have the `sincos' function. */
/* #undef HAVE_SINCOS */

/* Define to 1 if you have the <stdint.h> header file. */
#define HAVE_STDINT_H 1

/* Define to 1 if you have the <stdio.h> header file. */
#define HAVE_STDIO_H 1

/* Define to 1 if you have the <stdlib.h> header file. */
#define HAVE_STDLIB_H 1

/* Define to 1 if you have the <strings.h> header file. */
#define HAVE_STRINGS_H 1

/* Define to 1 if you have the <string.h> header file. */
#define HAVE_STRING_H 1

/* Define to 1 if you have the <sys/stat.h> header file. */
#define HAVE_SYS_STAT_H 1

/* Define to 1 if you have the <sys/types.h> header file. */
#define HAVE_SYS_TYPES_H 1

/* Define to 1 if you have the <unistd.h> header file. */
#define HAVE_UNISTD_H 1

/* Define to the sub-directory where libtool stores uninstalled libraries. */
#define LT_OBJDIR ".libs/"

/* Set MBSYSTEM_CONFIG_DEFINED define in mb_config.h */
#define MBSYSTEM_CONFIG_DEFINED 1

/* Set MBSYSTEM_INSTALL_PREFIX define in mb_config.h */
#define MBSYSTEM_INSTALL_PREFIX "/usr/local"

/* Set MBSYSTEM_OTPS_LOCATION define in mb_config.h */
#define MBSYSTEM_OTPS_LOCATION "/usr/local/OTPS2"

/* Build libmbtnav and embed TRN instance in mbtrnpp */
#define MBTNAV_ENABLED 1

/* Build libmbtrn and mbtrnpp */
#define MBTRN_ENABLED 1

/* Build graphical tools */
/* #undef MB_GRAPHICAL_ENABLED */

/* Build tools using OpenCV */
/* #undef OPENCVTOOLS_ENABLED */

/* Name of package */
#define PACKAGE "mbsystem"

/* Define to the address where bug reports for this package should be sent. */
#define PACKAGE_BUGREPORT "http://listserver.mbari.org/sympa/arc/mbsystem"

/* Define to the full name of this package. */
#define PACKAGE_NAME "mbsystem"

/* Define to the full name and version of this package. */
#define PACKAGE_STRING "mbsystem 5.7.9beta29"

/* Define to the one symbol short name of this package. */
#define PACKAGE_TARNAME "mbsystem"

/* Define to the home page for this package. */
#define PACKAGE_URL "http://www.mbari.org/data/mbsystem/"

/* Define to the version of this package. */
#define PACKAGE_VERSION "5.7.9beta29"

/* Build tools using PCL */
/* #undef PCLTOOLS_ENABLED */

/* Build tools using Qt5 */
/* #undef QTTOOLS_ENABLED */

/* Define to 1 if all of the C90 standard headers exist (not just the ones
   required in a freestanding environment). This macro is provided for
   backward compatibility; new code need not use it. */
#define STDC_HEADERS 1

/* Building unit tests */
/* #undef TEST_ENABLED */

/* Version number of package */
#define VERSION "5.7.9beta29"

/* Set VERSION_DATE define in mb_config.h */
#define VERSION_DATE "13 May 2022"

/* 0 */
/* #undef WITH_DEBUG */

/* Define WORDS_BIGENDIAN to 1 if your processor stores words with the most
   significant byte first (like Motorola and SPARC, unlike Intel). */
#if defined AC_APPLE_UNIVERSAL_BUILD
# if defined __BIG_ENDIAN__
#  define WORDS_BIGENDIAN 1
# endif
#else
# ifndef WORDS_BIGENDIAN
/* #  undef WORDS_BIGENDIAN */
# endif
#endif

/* Define to 1 if the X Window System is missing or not being used. */
/* #undef X_DISPLAY_MISSING */

/* Define to the type of a signed integer type of width exactly 8 bits if such
   a type exists and the standard includes do not define it. */
/* #undef int8_t */
