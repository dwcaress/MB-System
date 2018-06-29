/* src/mbio/mb_config.h.  Generated from mb_config.h.in by configure.  */
/* src/mbio/mb_config.h.in.  Generated from configure.ac by autoheader.  */

/* Machine is littleendian, (Byteswapping on) */
#define BYTESWAPPED 1

/* Machine is bigendian, (Byteswapping off) */
/* #undef ENDIAN_BIG */

/* Turned on OpenGL define in config */
#define GOT_GL 1

/* Define to 1 if you have the <dlfcn.h> header file. */
#define HAVE_DLFCN_H 1

/* Define to 1 if you have the <inttypes.h> header file. */
#define HAVE_INTTYPES_H 1

/* Define to 1 if you have the `m' library (-lm). */
#define HAVE_LIBM 1

/* Have malloc.h */
/* #undef HAVE_MALLOC_H */

/* Define to 1 if you have the <memory.h> header file. */
#define HAVE_MEMORY_H 1

/* Define to 1 if you have the <rpc/rpc.h> header file. */
#define HAVE_RPC_RPC_H 1

/* Define to 1 if you have the <rpc/types.h> header file. */
#define HAVE_RPC_TYPES_H 1

/* Define to 1 if you have the <stdint.h> header file. */
#define HAVE_STDINT_H 1

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

/* Define to 1 if you have the <tirpc/rpc/rpc.h> header file. */
/* #undef HAVE_TIRPC_RPC_RPC_H */

/* Define to 1 if you have the <tirpc/rpc/types.h> header file. */
/* #undef HAVE_TIRPC_RPC_TYPES_H */

/* Define to 1 if you have the <unistd.h> header file. */
#define HAVE_UNISTD_H 1

/* Define to the sub-directory where libtool stores uninstalled libraries. */
#define LT_OBJDIR ".libs/"

/* Set MBSYSTEM_CONFIG_DEFINED define in mb_config.h */
#define MBSYSTEM_CONFIG_DEFINED 1

/* Set MBSYSTEM_INSTALL_PREFIX define in mb_config.h */
#define MBSYSTEM_INSTALL_PREFIX "/usr/local"

/* Set MBSYSTEM_OTPS_LOCATION define in mb_config.h */
#define MBSYSTEM_OTPS_LOCATION "/usr/local/opt/otps"

/* Build libmbtrn and mbtrnpreprocess */
#define MBTRN_ENABLED 1

/* Name of package */
#define PACKAGE "mbsystem"

/* Define to the address where bug reports for this package should be sent. */
#define PACKAGE_BUGREPORT "http://listserver.mbari.org/sympa/arc/mbsystem"

/* Define to the full name of this package. */
#define PACKAGE_NAME "mbsystem"

/* Define to the full name and version of this package. */
#define PACKAGE_STRING "mbsystem 5.5.2341"

/* Define to the one symbol short name of this package. */
#define PACKAGE_TARNAME "mbsystem"

/* Define to the home page for this package. */
#define PACKAGE_URL "http://www.mbari.org/data/mbsystem/"

/* Define to the version of this package. */
#define PACKAGE_VERSION "5.5.2341"

/* Define to 1 if you have the ANSI C header files. */
#define STDC_HEADERS 1

/* Version number of package */
#define VERSION "5.5.2341"

/* Set VERSION_DATE define in mb_config.h */
#define VERSION_DATE "28 June 2018"

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

/* Define to the type of a signed integer type of width exactly 8 bits if such
   a type exists and the standard includes do not define it. */
/* #undef int8_t */
