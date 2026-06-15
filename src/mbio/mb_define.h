/*--------------------------------------------------------------------
 *    The MB-system:  mb_define.h  4/21/96
 *
 *    Copyright (c) 1996-2025 by
 *    David W. Caress (caress@mbari.org)
 *      Monterey Bay Aquarium Research Institute
 *      Moss Landing, California, USA
 *    Dale N. Chayes 
 *      Center for Coastal and Ocean Mapping
 *      University of New Hampshire
 *      Durham, New Hampshire, USA
 *    Christian dos Santos Ferreira
 *      MARUM
 *      University of Bremen
 *      Bremen Germany
 *     
 *    MB-System was created by Caress and Chayes in 1992 at the
 *      Lamont-Doherty Earth Observatory
 *      Columbia University
 *      Palisades, NY 10964
 *
 *    See README.md file for copying and redistribution conditions.
 *--------------------------------------------------------------------*/
/**
 * @file 
 * @brief Define macros, types and functions used by MB-System 
 * 
 * Author:  D. W. Caress
 * Date:  April 21, 1996
 */

#ifndef MB_DEFINE_H_
#define MB_DEFINE_H_

#include <stdbool.h>
#include <stdint.h>

/* On Windows/MSVC, <unistd.h> does not exist; pull in the substitute shim
   (clock_gettime, strcasecmp, sockets, POSIX stat-mode bits, getopt, etc.).
   Formerly the force-included include/mb_win_compat.h + include/unistd.h.
   mb_define.h is force-included on every TU on MSVC (top-level CMakeLists.txt
   /FI option), so this reaches all sources. */
#ifdef _WIN32
#include "unistd_w.h"
#else
#include <unistd.h>
#endif

/* Define version and date for this release */
#define MB_VERSION "5.8.3beta13"
#define MB_VERSION_DATE "29 April 2026"

/* CMake supports current OS's and so there is only one form of RPC and XDR and no mb_config.h file */
#ifdef CMAKE_BUILD_SYSTEM

/* Windows has no SunRPC/XDR. Provide minimal XDR shim so consumers
   (mb_read_init.c, mb_write_init.c, format readers) still parse. */
#  ifndef _WIN32
#    include <rpc/rpc.h>
#    include <rpc/types.h>
#    include <rpc/xdr.h>
#  else
#    ifndef _MB_WIN_XDR_DEFINED
#    define _MB_WIN_XDR_DEFINED
typedef enum { XDR_ENCODE=0, XDR_DECODE=1, XDR_FREE=2 } XdrOp;
typedef struct {
    XdrOp x_op;
    char *x_public;
    char *x_private;
    char *x_base;
    int   x_handy;
} XDR;
#    endif
#  endif

#else // Begin Autotools section supporting legacy OS's

#include <mb_config.h>

#ifdef _WIN32
  /* https://www.zachburlingame.com/2011/05/resolving-redefinition-errors-betwen-ws2def-h-and-winsock-h/ */
#  ifndef WIN32
#    define WIN32
#  endif
#  include <WinSock2.h>
#  include <Windows.h>
#endif

/* For XDR/RPC */
#ifndef _WIN32
# ifdef HAVE_RPC_RPC_H
#  include <rpc/rpc.h>
#  include <rpc/types.h>
#  include <rpc/xdr.h>
# else
#  ifdef HAVE_TIRPC_RPC_RPC_H
#   include <tirpc/rpc/rpc.h>
#   include <tirpc/rpc/types.h>
#   include <tirpc/rpc/xdr.h>
#  endif
# endif
#else
#	include "types_win32.h"
#endif

#endif // End Autotools section

#ifdef __cplusplus
extern "C" {
#endif

#ifndef CMAKE_BUILD_SYSTEM // Begin Autotools section supporting legacy OS's

/* for Windows */
#if defined(_WIN32) && (_MSC_VER < 1800)
#if !defined(copysign)
#define copysign(x, y) _copysign(x, y)
#endif
#if !defined(log2)
#define log2(x) (log(x) / log(2))
#endif
#if !defined(rint)
#define rint(x) (floor((x) + 0.5))
#endif
#endif

#ifdef _WIN32
#define sleep Sleep
#define popen _popen
#define pclose _pclose
#define ftello ftell
#define fseeko fseek
#if !defined(isnan) && (_MSC_VER < 1900)
#  define isnan(x) _isnan(x)
#endif
#if !defined(inline) && (_MSC_VER < 1900)
#  define inline __inline
#endif
#endif

#endif // End Autotools section

/* type definitions of signed and unsigned char */
typedef unsigned char mb_u_char;

/* From stdint.h if available */
#if defined INT8_MAX || defined int8_t
typedef int8_t mb_s_char;
#else
typedef signed char mb_s_char;
#endif

/* type definitions of signed and unsigned long int (64 bit integer) */
typedef long long unsigned mb_u_long;
typedef long long mb_s_long;

/** Type definitions for structures used in beam angle calculations */
typedef struct {
  double x;
  double y;
  double z;
} mb_3D_vector;

typedef struct {
  double roll;
  double pitch;
  double heading;
} mb_3D_orientation;

/** declare buffer maximum */
#define MB_BUFFER_MAX 5000

/* maximum path length in characters */
#define MB_PATH_MAXLINE 1024
#define MB_PATHPLUS_MAXLINE 1152
#define MB_PATHPLUSPLUS_MAXLINE 2304

/* maximum comment length in characters */
#define MB_COMMENT_MAXLINE 1944

/* other string length defines */
#define MB_NAME_LENGTH 32
#define MB_LONGNAME_LENGTH 128
#define MB_DESCRIPTION_LENGTH 2048
#define MB_COMMAND_LENGTH 8192 // Windows command line maximum

/* maximum UDP packet size */
#define MB_UDP_SIZE_MAX 65536

/* typedef for path string */
typedef char mb_path[MB_PATH_MAXLINE];
typedef char mb_pathplus[MB_PATHPLUS_MAXLINE];
typedef char mb_pathplusplus[MB_PATHPLUSPLUS_MAXLINE];
typedef char mb_name[MB_NAME_LENGTH];
typedef char mb_longname[MB_LONGNAME_LENGTH];
typedef char mb_command[MB_COMMAND_LENGTH];

/* maximum number of threads created by an MB-System program/function */
#define MB_THREAD_MAX 16

/* maximum number of asynchronous data saved */
#define MB_ASYNCH_SAVE_MAX 10000

/* maximum size of SVP profiles */
#define MB_SVP_MAX 1024

/* maximum number of CTD samples per record */
#define MB_CTD_MAX 256

/* maximum number of asynchronous nav samples per record */
#define MB_NAV_MAX 256

/* maximum number of record types that can specified to have contents dumped during
   reading for debugging */
#define MB_NUM_DEBUG_RECORD_MAX 10

/* file mode (read or write) */
typedef enum {
  MB_FILEMODE_READ = 0,
  MB_FILEMODE_WRITE = 1,
} mb_filemode_enum;

/* types of  files used by swath sonar data formats */
#define MB_FILETYPE_NORMAL 1
#define MB_FILETYPE_SINGLE 2
#define MB_FILETYPE_XDR 3
#define MB_FILETYPE_GSF 4
#define MB_FILETYPE_NETCDF 5
#define MB_FILETYPE_SURF 6
#define MB_FILETYPE_SEGY 7
#define MB_FILETYPE_INPUT 8

/* settings for recursive datalist reading functions */
#define MB_DATALIST_LOOK_UNSET 0
#define MB_DATALIST_LOOK_NO 1
#define MB_DATALIST_LOOK_YES 2

/* settings for recursive imagelist reading functions */
#define MB_IMAGELIST_LOOK_UNSET 0
#define MB_IMAGELIST_LOOK_NO 1
#define MB_IMAGELIST_LOOK_YES 2

/* settings of i/o array dimension types */
#define MB_MEM_TYPE_NONE 0
#define MB_MEM_TYPE_BATHYMETRY 1
#define MB_MEM_TYPE_AMPLITUDE 2
#define MB_MEM_TYPE_SIDESCAN 3

/* declare PI if needed */
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

/* the natural log of 2 is always useful */
#define MB_LN_2 0.69314718056

/* multiply this by degrees to get radians */
#define DTR 0.01745329251994329500

/* multiply this by radians to get degrees */
#define RTD 57.2957795130823230000

/* declare Golden Mean ratios */
#define GOLDEN_MEAN_SMALL 0.38197
#define GOLDEN_MEAN_LARGE 0.61803

/* time conversions */
#define MB_SECINYEAR 31536000.0
#define MB_SECINDAY 86400.0
#define MB_SECINHOUR 3600.0
#define MB_SECINMINUTE 60.0
#define MB_ISECINYEAR 31536000
#define MB_ISECINDAY 86400
#define MB_ISECINHOUR 3600
#define MB_ISECINMINUTE 60
#define MB_IMININHOUR 60
#define MB_SECONDS_01JAN2000 946684800.0

/* water sound speed calculation algorithms */
#define MB_SOUNDSPEEDALGORITHM_NONE     0
#define MB_SOUNDSPEEDALGORITHM_CHENMILLERO   1
#define MB_SOUNDSPEEDALGORITHM_WILSON     2
#define MB_SOUNDSPEEDALGORITHM_DELGROSSO   3

// MB-System graphics color and linestyle definitions
typedef enum {
  MB_COLOR_WHITE = 0,
  MB_COLOR_BLACK,
  MB_COLOR_RED,
  MB_COLOR_ORANGE,
  MB_COLOR_YELLOW,
  MB_COLOR_GREEN,
  MB_COLOR_BLUEGREEN,
  MB_COLOR_BLUE,
  MB_COLOR_PURPLE,
  MB_COLOR_CORAL,
  MB_COLOR_LIGHTGREY,
  MB_NDrawingColors
} mb_color_t;
typedef enum {
  MB_SOLID_LINE = 0,
  MB_DASH_LINE
} mb_linestyle_t;

/* min max round define */
#ifndef MIN
#define MIN(A, B) ((A) < (B) ? (A) : (B))
#endif
#ifndef MAX
#define MAX(A, B) ((A) > (B) ? (A) : (B))
#endif

/* NaN defines */
#ifdef NO_IEEE
#define MB_MAKE_FNAN(x) (x = FLT_MAX)
#define MB_MAKE_DNAN(x) (x = DBL_MAX)
#define MB_IS_FNAN(x) ((x) == FLT_MAX)
#define MB_IS_DNAN(x) ((x) == DBL_MAX)
#else
#define MB_MAKE_FNAN(x) (x = (float)NAN)
#define MB_MAKE_DNAN(x) (x = NAN)
#define MB_IS_FNAN isnan
#define MB_IS_DNAN isnan
#endif

/* printf format for binary bitmask */
/* from https://stackoverflow.com/questions/111928/is-there-a-printf-converter-to-print-in-binary-format */
#define MB_PRINTF_BINARY_PATTERN_INT8 "%c%c%c%c%c%c%c%c"
#define MB_PRINTF_BYTE_TO_BINARY_INT8(i)    \
    (((i) & 0x80ll) ? '1' : '0'), \
    (((i) & 0x40ll) ? '1' : '0'), \
    (((i) & 0x20ll) ? '1' : '0'), \
    (((i) & 0x10ll) ? '1' : '0'), \
    (((i) & 0x08ll) ? '1' : '0'), \
    (((i) & 0x04ll) ? '1' : '0'), \
    (((i) & 0x02ll) ? '1' : '0'), \
    (((i) & 0x01ll) ? '1' : '0')

#define MB_PRINTF_BINARY_PATTERN_INT16 \
    MB_PRINTF_BINARY_PATTERN_INT8              MB_PRINTF_BINARY_PATTERN_INT8
#define MB_PRINTF_BYTE_TO_BINARY_INT16(i) \
    MB_PRINTF_BYTE_TO_BINARY_INT8((i) >> 8),   MB_PRINTF_BYTE_TO_BINARY_INT8(i)
#define MB_PRINTF_BINARY_PATTERN_INT32 \
    MB_PRINTF_BINARY_PATTERN_INT16             MB_PRINTF_BINARY_PATTERN_INT16
#define MB_PRINTF_BYTE_TO_BINARY_INT32(i) \
    MB_PRINTF_BYTE_TO_BINARY_INT16((i) >> 16), MB_PRINTF_BYTE_TO_BINARY_INT16(i)
#define MB_PRINTF_BINARY_PATTERN_INT64    \
    MB_PRINTF_BINARY_PATTERN_INT32             MB_PRINTF_BINARY_PATTERN_INT32
#define MB_PRINTF_BYTE_TO_BINARY_INT64(i) \
    MB_PRINTF_BYTE_TO_BINARY_INT32((i) >> 32), MB_PRINTF_BYTE_TO_BINARY_INT32(i)

/* default grid no data value define */
#define MB_DEFAULT_GRID_NODATA -9999999.9

/* safe square root define - sets argument to zero if negative */
#define SAFESQRT(X) sqrt(MAX(0.0, X))

/* position projection flag (0 = longitude latitude, 1 = projected eastings northings) */
#define MB_PROJECTION_GEOGRAPHIC 0
#define MB_PROJECTION_PROJECTED 1

/* MBIO core function prototypes */
int mb_version(int verbose, char *version_string, int *version_id, int *version_major, int *version_minor, int *version_archive,
               int *error);
int mb_user_host_date(int verbose, char user[256], char host[256], char date[32], int *error);
int mb_default_defaults(int verbose, int *format, int *pings, int *lonflip, double bounds[4], int *btime_i, int *etime_i,
                double *speedmin, double *timegap);
int mb_defaults(int verbose, int *format, int *pings, int *lonflip, double bounds[4], int *btime_i, int *etime_i,
                double *speedmin, double *timegap);
int mb_env(int verbose, char *psdisplay, char *imgdisplay, char *mbproject);
int mb_lonflip(int verbose, int *lonflip);
int mb_mbview_defaults(int verbose, int *primary_colortable, int *primary_colortable_mode, int *primary_shade_mode,
                       int *slope_colortable, int *slope_colortable_mode, int *secondary_colortable,
                       int *secondary_colortable_mode, double *illuminate_magnitude, double *illuminate_elevation,
                       double *illuminate_azimuth, double *slope_magnitude);
int mb_fbtversion(int verbose, int *fbtversion);
int mb_uselockfiles(int verbose, bool *uselockfiles);
int mb_fileiobuffer(int verbose, int *fileiobuffer);
int mb_format_register(int verbose, int *format, void *mbio_ptr, int *error);
int mb_format_info(int verbose, int *format, int *system, int *beams_bath_max, int *beams_amp_max, int *pixels_ss_max,
                   char *format_name, char *system_name, char *format_description, int *numfile, int *filetype,
                   bool *variable_beams, bool *traveltime, bool *beam_flagging, int *platform_source, int *nav_source,
                   int *sensordepth_source, int *heading_source, int *attitude_source, int *svp_source, double *beamwidth_xtrack,
                   double *beamwidth_ltrack, int *error);
int mb_format(int verbose, int *format, int *error);
int mb_format_system(int verbose, int *format, int *system, int *error);
int mb_format_description(int verbose, int *format, char *description, int *error);
int mb_format_dimensions(int verbose, int *format, int *beams_bath_max, int *beams_amp_max, int *pixels_ss_max, int *error);
int mb_format_flags(int verbose, int *format, bool *variable_beams, bool *traveltime, bool *beam_flagging, int *error);
int mb_format_source(int verbose, int *format, int *platform_source, int *nav_source, int *sensordepth_source, int *heading_source,
                     int *attitude_source, int *svp_source, int *error);
int mb_format_beamwidth(int verbose, int *format, double *beamwidth_xtrack, double *beamwidth_ltrack, int *error);

/** Get swath file format code
    @param verbose verbose debug output, True or False
    @param filename swath data file
    @param fileroot ???
    @param format swath code
    @param error error code, if returns MB_FAILURE
    @return MB_SUCCESS or MB_FAILURE
 */
 int mb_get_format(int verbose, char *filename, char *fileroot, int *format, int *error);
  
int mb_datalist_open(int verbose, void **datalist_ptr, char *path, int look_processed, int *error);
int mb_datalist_read(int verbose, void *datalist_ptr, char *path, char *dpath, int *format, double *weight, int *error);
int mb_datalist_read2(int verbose, void *datalist_ptr, int *pstatus, char *path, char *ppath, char *dpath, int *format,
                      double *weight, int *error);
int mb_datalist_read3(int verbose, void *datalist_ptr, int *pstatus, char *path, char *ppath, int *astatus, char *apath, 
                      char *dpath, int *format, double *weight, int *error);
int mb_datalist_readorg(int verbose, void *datalist_ptr, char *path, int *format, double *weight, int *error);
int mb_datalist_recursion(int verbose, void *datalist_ptr, bool print, int *recursion, int *error);
int mb_datalist_close(int verbose, void **datalist_ptr, int *error);
int mb_imagelist_open(int verbose, void **imagelist_ptr, char *path, int *error);
int mb_imagelist_read(int verbose, void *imagelist_ptr, int *imagestatus, bool *rectified, 
                      char *path0, char *path1, char *dpath,
                      double *time_d0, double *time_d1,
                      double *gain0, double *gain1,
                      double *exposure0, double *exposure1, int *error);
int mb_imagelist_recursion(int verbose, void *imagelist_ptr, bool print, int *recursion, int *error);
int mb_imagelist_close(int verbose, void **imagelist_ptr, int *error);
int mb_get_relative_path(int verbose, char *path, char *pwd, int *error);
int mb_get_absolute_path(int verbose, char *path, char *pwd, int *error);
int mb_get_shortest_path(int verbose, char *path, int *error);
int mb_get_basename(int verbose, char *path, int *error);
int mb_check_info(int verbose, char *file, int lonflip, double bounds[4], bool *file_in_bounds, int *error);
bool mb_should_make_fbt(int verbose, int format);
bool mb_should_make_fnv(int verbose, int format);
int mb_make_info(int verbose, bool force, char *file, int format, int *error);
int mb_get_fbt(int verbose, char *file, int *format, int *error);
int mb_get_fnv(int verbose, char *file, int *format, int *error);
int mb_get_ffa(int verbose, char *file, int *format, int *error);
int mb_get_ffs(int verbose, char *file, int *format, int *error);
int mb_swathbounds(int verbose, int checkgood, int nbath, int nss,
                  char *beamflag, double *bathacrosstrack,
                  double *ss, double *ssacrosstrack,
                  int *ibeamport, int *ibeamcntr, int *ibeamstbd,
                  int *ipixelport, int *ipixelcntr, int *ipixelstbd, int *error);
int mb_read_init(int verbose, char *file, int format, int pings, int lonflip, double bounds[4], int btime_i[7], int etime_i[7],
                  double speedmin, double timegap, void **mbio_ptr, double *btime_d, double *etime_d, int *beams_bath,
                  int *beams_amp, int *pixels_ss, int *error);
int mb_read_init_altnav(int verbose, char *file, int format, int pings, 
                  int lonflip, double bounds[4], int btime_i[7], int etime_i[7],
                  double speedmin, double timegap, int astatus, char *apath, 
                  void **mbio_ptr, double *btime_d, double *etime_d, 
                  int *beams_bath, int *beams_amp, int *pixels_ss, int *error);
int mb_input_init(int verbose, char *socket_definition, int format, int pings,
                  int lonflip, double bounds[4], int btime_i[7], int etime_i[7],
                  double speedmin, double timegap, void **mbio_ptr,
                  double *btime_d, double *etime_d, int *beams_bath,
                  int *beams_amp, int *pixels_ss,
                  int (*input_open)(int verbose, void *mbio_ptr, char *definition, int *error),
                  int (*input_read)(int verbose, void *mbio_ptr, size_t *size, char *buffer, int *error),
                  int (*input_close)(int verbose, void *mbio_ptr, int *error),
                  int *error);
int mb_set_debug_records(int verbose, void *mbio_ptr, 
									bool enable_debug_record_type_listing, 
									int num_debug_record_identifiers, 
									mb_name *debug_record_identifiers,
									int *error);
int mb_write_init(int verbose, char *file, int format, void **mbio_ptr, int *beams_bath, int *beams_amp, int *pixels_ss,
                  int *error);
int mb_close(int verbose, void **mbio_ptr, int *error);
int mb_read_ping(int verbose, void *mbio_ptr, void *store_ptr, int *kind, int *error);
int mb_get_all(int verbose, void *mbio_ptr, void **store_ptr, int *kind, int time_i[7], double *time_d, double *navlon,
                  double *navlat, double *speed, double *heading, double *distance, double *altitude, double *sensordepth, int *nbath,
                  int *namp, int *nss, char *beamflag, double *bath, double *amp, double *bathacrosstrack, double *bathalongtrack,
                  double *ss, double *ssacrosstrack, double *ssalongtrack, char *comment, int *error);
int mb_get(int verbose, void *mbio_ptr, int *kind, int *pings, int time_i[7], double *time_d, double *navlon, double *navlat,
                  double *speed, double *heading, double *distance, double *altitude, double *sensordepth, int *nbath, int *namp,
                  int *nss, char *beamflag, double *bath, double *amp, double *bathacrosstrack, double *bathalongtrack, double *ss,
                  double *ssacrosstrack, double *ssalongtrack, char *comment, int *error);
int mb_read(int verbose, void *mbio_ptr, int *kind, int *pings, int time_i[7], double *time_d, double *navlon, double *navlat,
                  double *speed, double *heading, double *distance, double *altitude, double *sensordepth, int *nbath, int *namp,
                  int *nss, char *beamflag, double *bath, double *amp, double *bathlon, double *bathlat, double *ss, double *sslon,
                  double *sslat, char *comment, int *error);
int mb_write_ping(int verbose, void *mbio_ptr, void *store_ptr, int *error);
int mb_put_all(int verbose, void *mbio_ptr, void *store_ptr, int usevalues, int kind, int time_i[7], double time_d, double navlon,
                  double navlat, double speed, double heading, int nbath, int namp, int nss, char *beamflag, double *bath,
                  double *amp, double *bathacrosstrack, double *bathalongtrack, double *ss, double *ssacrosstrack,
                  double *ssalongtrack, char *comment, int *error);
int mb_put_comment(int verbose, void *mbio_ptr, char *comment, int *error);
int mb_fileio_open(int verbose, void *mbio_ptr, int *error);
int mb_fileio_close(int verbose, void *mbio_ptr, int *error);
int mb_fileio_get(int verbose, void *mbio_ptr, char *buffer, size_t *size, int *error);
int mb_fileio_put(int verbose, void *mbio_ptr, char *buffer, size_t *size, int *error);
int mb_copyfile(int verbose, const char *src, const char *dst, int *error);
int mb_catfiles(int verbose, const char *src1, const char *src2, const char *dst, int *error);
int mb_alloc(int verbose, void *mbio_ptr, void **store_ptr, int *error);
int mb_deall(int verbose, void *mbio_ptr, void **store_ptr, int *error);
int mb_get_store(int verbose, void *mbio_ptr, void **store_ptr, int *error);
int mb_dimensions(int verbose, void *mbio_ptr, void *store_ptr, int *kind, int *nbath, int *namp, int *nss, int *error);
int mb_pingnumber(int verbose, void *mbio_ptr, unsigned int *pingnumber, int *error);
int mb_segynumber(int verbose, void *mbio_ptr, unsigned int *line, unsigned int *shot, unsigned int *cdp, int *error);
int mb_beamwidths(int verbose, void *mbio_ptr, double *beamwidth_xtrack, double *beamwidth_ltrack, int *error);
int mb_sonartype(int verbose, void *mbio_ptr, void *store_ptr, int *sonartype, int *error);
int mb_sidescantype(int verbose, void *mbio_ptr, void *store_ptr, int *ss_type, int *error);
int mb_preprocess(int verbose, void *mbio_ptr, void *store_ptr, void *platform_ptr, void *preprocess_pars_ptr, int *error);
int mb_extract_platform(int verbose, void *mbio_ptr, void *store_ptr, int *kind, void **platform_ptr, int *error);
int mb_sensorhead(int verbose, void *mbio_ptr, void *store_ptr, int *sensorhead, int *error);
int mb_extract(int verbose, void *mbio_ptr, void *store_ptr, int *kind, int time_i[7], double *time_d, double *navlon,
                double *navlat, double *speed, double *heading, int *nbath, int *namp, int *nss, char *beamflag, double *bath,
                double *amp, double *bathacrosstrack, double *bathalongtrack, double *ss, double *ssacrosstrack,
                double *ssalongtrack, char *comment, int *error);
int mb_insert(int verbose, void *mbio_ptr, void *store_ptr, int kind, int time_i[7], double time_d, double navlon, double navlat,
                double speed, double heading, int nbath, int namp, int nss, char *beamflag, double *bath, double *amp,
                double *bathacrosstrack, double *bathalongtrack, double *ss, double *ssacrosstrack, double *ssalongtrack,
                char *comment, int *error);
int mb_extract_lonlat(int verbose, void *mbio_ptr, void *store_ptr, int *kind, int time_i[7], double *time_d, double *navlon,
               double *navlat, double *speed, double *heading, int *nbath, int *namp, int *nss, char *beamflag, double *bath,
               double *amp, double *bathlon, double *bathlat, double *ss, double *sslon,
               double *sslat, char *comment, int *error);
int mb_extract_nav(int verbose, void *mbio_ptr, void *store_ptr, int *kind, int time_i[7], double *time_d, double *navlon,
                double *navlat, double *speed, double *heading, double *draft, double *roll, double *pitch, double *heave,
                int *error);
int mb_extract_nnav(int verbose, void *mbio_ptr, void *store_ptr, int nmax, int *kind, int *n, int *time_i, double *time_d,
                double *navlon, double *navlat, double *speed, double *heading, double *draft, double *roll, double *pitch,
                double *heave, int *error);
int mb_insert_nav(int verbose, void *mbio_ptr, void *store_ptr, int time_i[7], double time_d, double navlon, double navlat,
                double speed, double heading, double draft, double roll, double pitch, double heave, int *error);
int mb_extract_altitude(int verbose, void *mbio_ptr, void *store_ptr, int *kind, double *transducer_depth, double *altitude,
                int *error);
int mb_insert_altitude(int verbose, void *mbio_ptr, void *store_ptr, double transducer_depth, double altitude, int *error);
int mb_extract_svp(int verbose, void *mbio_ptr, void *store_ptr, int *kind, int *nsvp, double *depth, double *velocity,
                int *error);
int mb_insert_svp(int verbose, void *mbio_ptr, void *store_ptr, int nsvp, double *depth, double *velocity, int *error);
int mb_ttimes(int verbose, void *mbio_ptr, void *store_ptr, int *kind, int *nbeams, double *ttimes, double *angles,
              double *angles_forward, double *angles_null, double *heave, double *alongtrack_offset, double *draft, double *ssv,
              int *error);
int mb_detects(int verbose, void *mbio_ptr, void *store_ptr, int *kind, int *nbeams, int *detects, int *error);
int mb_pulses(int verbose, void *mbio_ptr, void *store_ptr, int *kind, int *nbeams, int *pulses, int *error);
int mb_gains(int verbose, void *mbio_ptr, void *store_ptr, int *kind, double *transmit_gain, double *pulse_length,
             double *receive_gain, int *error);
int mb_makess(int verbose, void *mbio_ptr, void *store_ptr, int pixel_size_set, double *pixel_size,
                         int swath_width_set, double *swath_width, int pixel_int, int *error);
int mb_extract_rawssdimensions(int verbose, void *mbio_ptr, void *store_ptr, int *kind, double *sample_interval,
                               int *num_samples_port, int *num_samples_stbd, int *error);
int mb_extract_rawss(int verbose, void *mbio_ptr, void *store_ptr, int *kind, int *sidescan_type, double *sample_interval,
                     double *beamwidth_xtrack, double *beamwidth_ltrack, int *num_samples_port, double *rawss_port,
                     int *num_samples_stbd, double *rawss_stbd, int *error);
int mb_insert_rawss(int verbose, void *mbio_ptr, void *store_ptr, int kind, int sidescan_type, double sample_interval,
                    double beamwidth_xtrack, double beamwidth_ltrack, int num_samples_port, double *rawss_port,
                    int num_samples_stbd, double *rawss_stbd, int *error);
int mb_extract_segytraceheader(int verbose, void *mbio_ptr, void *store_ptr, int *kind, void *segytraceheader_ptr, int *error);
int mb_extract_segy(int verbose, void *mbio_ptr, void *store_ptr, int *sampleformat, int *kind, void *segyheader_ptr,
                    float *segydata, int *error);
int mb_insert_segy(int verbose, void *mbio_ptr, void *store_ptr, int kind, void *segyheader_ptr, float *segydata, int *error);
int mb_ctd(int verbose, void *mbio_ptr, void *store_ptr, int *kind, int *nctd, double *time_d, double *conductivity,
           double *temperature, double *depth, double *salinity, double *soundspeed, int *error);
int mb_ancilliarysensor(int verbose, void *mbio_ptr, void *store_ptr, int *kind, int *nsensor, double *time_d, double *sensor1,
                        double *sensor2, double *sensor3, double *sensor4, double *sensor5, double *sensor6, double *sensor7,
                        double *sensor8, int *error);
int mb_copyrecord(int verbose, void *mbio_ptr, void *store_ptr, void *copy_ptr, int *error);
int mb_indextable(int verbose, void *mbio_ptr, int *num_indextable, void **indextable_ptr, int *error);
int mb_indextablefix(int verbose, void *mbio_ptr, int num_indextable, void *indextable_ptr, int *error);
int mb_indextableapply(int verbose, void *mbio_ptr, int num_indextable, void *indextable_ptr, int n_file, int *error);

int mb_platform_init(int verbose, void **platform_ptr, int *error);
int mb_platform_setinfo(int verbose, void *platform_ptr, int type, char *name, char *organization, char *documentation_url,
                        double start_time_d, double end_time_d, int *error);
int mb_platform_add_sensor(int verbose, void *platform_ptr, int type, mb_longname model, mb_longname manufacturer,
                           mb_longname serialnumber, int capability1, int capability2, int num_offsets, int num_time_latency,
                           int *error);
int mb_platform_set_sensor_offset(int verbose, void *platform_ptr, int isensor, int ioffset, int position_offset_mode,
                                  double position_offset_x, double position_offset_y, double position_offset_z,
                                  int attitude_offset_mode, double attitude_offset_azimuth, double attitude_offset_roll,
                                  double attitude_offset_pitch, int *error);
int mb_platform_set_sensor_timelatency(int verbose, void *platform_ptr, int isensor, int time_latency_mode,
                                       double time_latency_static, int num_time_latency, double *time_latency_time_d,
                                       double *time_latency_value, int *error);
int mb_platform_set_source_sensor(int verbose, void *platform_ptr, int source_type, int sensor, int *error);
int mb_platform_deall(int verbose, void **platform_ptr, int *error);
int mb_platform_read(int verbose, char *platform_file, void **platform_ptr, int *error);
int mb_platform_write(int verbose, char *platform_file, void *platform_ptr, int *error);
int mb_platform_lever(int verbose, void *platform_ptr, int targetsensor, int targetsensoroffset, double heading, double roll,
                      double pitch, double *lever_x, double *lever_y, double *lever_z, int *error);
int mb_platform_position(int verbose, void *platform_ptr, int targetsensor, int targetsensoroffset, double navlon, double navlat,
                         double sensordepth, double heading, double roll, double pitch, double *targetlon, double *targetlat,
                         double *targetz, int *error);
int mb_platform_position_offset(int verbose, void *platform_ptr, int targetsensor, int targetsensoroffset,
                                   double *target_x_offset, double *target_y_offset, double *target_z_offset,
                                   int *error);
int mb_platform_orientation(int verbose, void *platform_ptr, double heading, double roll, double pitch, double *platform_heading,
                            double *platform_roll, double *platform_pitch, int *error);
int mb_platform_orientation_offset(int verbose, void *platform_ptr, int targetsensor, int targetsensoroffset,
                                   double *target_hdg_offset, double *target_roll_offset, double *target_pitch_offset,
                                   int *error);
int mb_platform_orientation_target(int verbose, void *platform_ptr, int targetsensor, int targetsensoroffset, double heading,
                                   double roll, double pitch, double *target_heading, double *target_roll, double *target_pitch,
                                   int *error);
int mb_platform_print(int verbose, void *platform_ptr, int *error);

void mb_platform_math_matrix_times_vector_3x1(double *A, double *b, double *Ab);
void mb_platform_math_matrix_times_matrix_3x3(double *A, double *B, double *AB);
void mb_platform_math_matrix_transpose_3x3(double *R, double *R_T);
void mb_platform_math_rph2rot(double *rph, double *R);
void mb_platform_math_rot2rph(double *R, double *rph);
int mb_platform_math_attitude_offset(int verbose, double target_offset_roll, double target_offset_pitch,
                                     double target_offset_heading, double source_offset_roll, double source_offset_pitch,
                                     double source_offset_heading, double *target2source_offset_roll,
                                     double *target2source_offset_pitch, double *target2source_offset_heading, int *error);
int mb_platform_math_attitude_platform(int verbose, double nav_attitude_roll, double nav_attitude_pitch,
                                       double nav_attitude_heading, double attitude_offset_roll, double attitude_offset_pitch,
                                       double attitude_offset_heading, double *platform_roll, double *platform_pitch,
                                       double *platform_heading, int *error);
int mb_platform_math_attitude_target(int verbose, double source_attitude_roll, double source_attitude_pitch,
                                     double source_attitude_heading, double target_offset_to_source_roll,
                                     double target_offset_to_source_pitch, double target_offset_to_source_heading,
                                     double *target_roll, double *target_pitch, double *target_heading, int *error);
int mb_platform_math_attitude_offset_corrected_by_nav(int verbose, double prev_attitude_roll, double prev_attitude_pitch,
                                                      double prev_attitude_heading, double target_offset_to_source_roll,
                                                      double target_offset_to_source_pitch,
                                                      double target_offset_to_source_heading, double new_attitude_roll,
                                                      double new_attitude_pitch, double new_attitude_heading,
                                                      double *corrected_offset_roll, double *corrected_offset_pitch,
                                                      double *corrected_offset_heading, int *error);
int mb_platform_math_attitude_rotate_beam(int verbose, double beam_acrosstrack, double beam_alongtrack, double beam_bath,
                                          double attitude_roll, double attitude_pitch, double attitude_heading,
                                          double *newbeam_easting, double *newbeam_northing, double *newbeam_bath, int *error);

int mb_buffer_init(int verbose, void **buff_ptr, int *error);
int mb_buffer_close(int verbose, void **buff_ptr, void *mbio_ptr, int *error);
int mb_buffer_load(int verbose, void *buff_ptr, void *mbio_ptr, int nwant, int *nload, int *nbuff, int *error);
int mb_buffer_dump(int verbose, void *buff_ptr, void *mbio_ptr, void *ombio_ptr, int nhold, int *ndump, int *nbuff, int *error);
int mb_buffer_clear(int verbose, void *buff_ptr, void *mbio_ptr, int nhold, int *ndump, int *nbuff, int *error);
int mb_buffer_info(int verbose, void *buff_ptr, void *mbio_ptr, int id, int *system, int *kind, int *error);
int mb_buffer_get_next_data(int verbose, void *buff_ptr, void *mbio_ptr, int start, int *id, int time_i[7], double *time_d,
                            double *navlon, double *navlat, double *speed, double *heading, int *nbath, int *namp, int *nss,
                            char *beamflag, double *bath, double *amp, double *bathacrosstrack, double *bathalongtrack,
                            double *ss, double *ssacrosstrack, double *ssalongtrack, int *error);
int mb_buffer_get_next_nav(int verbose, void *buff_ptr, void *mbio_ptr, int start, int *id, int time_i[7], double *time_d,
                           double *navlon, double *navlat, double *speed, double *heading, double *draft, double *roll,
                           double *pitch, double *heave, int *error);
int mb_buffer_extract(int verbose, void *buff_ptr, void *mbio_ptr, int id, int *kind, int time_i[7], double *time_d,
                      double *navlon, double *navlat, double *speed, double *heading, int *nbath, int *namp, int *nss,
                      char *beamflag, double *bath, double *amp, double *bathacrosstrack, double *bathalongtrack, double *ss,
                      double *ssacrosstrack, double *ssalongtrack, char *comment, int *error);
int mb_buffer_extract_nav(int verbose, void *buff_ptr, void *mbio_ptr, int id, int *kind, int time_i[7], double *time_d,
                          double *navlon, double *navlat, double *speed, double *heading, double *draft, double *roll,
                          double *pitch, double *heave, int *error);
int mb_buffer_insert(int verbose, void *buff_ptr, void *mbio_ptr, int id, int time_i[7], double time_d, double navlon,
                     double navlat, double speed, double heading, int nbath, int namp, int nss, char *beamflag, double *bath,
                     double *amp, double *bathacrosstrack, double *bathalongtrack, double *ss, double *ssacrosstrack,
                     double *ssalongtrack, char *comment, int *error);
int mb_buffer_insert_nav(int verbose, void *buff_ptr, void *mbio_ptr, int id, int time_i[7], double time_d, double navlon,
                         double navlat, double speed, double heading, double draft, double roll, double pitch, double heave,
                         int *error);
int mb_buffer_get_kind(int verbose, void *buff_ptr, void *mbio_ptr, int id, int *kind, int *error);
int mb_buffer_get_ptr(int verbose, void *buff_ptr, void *mbio_ptr, int id, void **store_ptr, int *error);

int mb_coor_scale(int verbose, double latitude, double *mtodeglon, double *mtodeglat);
int mb_alvinxy_scale(int verbose, double latitude, double *mtodeglon, double *mtodeglat);
int mb_apply_lonflip(int verbose, int lonflip, double *longitude);

int mb_error(int, int, char **);
int mb_notice_log_datatype(int verbose, void *mbio_ptr, int data_id);
int mb_notice_log_error(int verbose, void *mbio_ptr, int error_id);
int mb_notice_log_problem(int verbose, void *mbio_ptr, int problem_id);
int mb_notice_get_list(int verbose, void *mbio_ptr, int *notice_list);
int mb_notice_message(int verbose, int notice, char **message);
int mb_navint_add(int verbose, void *mbio_ptr, double time_d, double lon_easting, double lat_northing, int *error);
int mb_navint_interp(int verbose, void *mbio_ptr, double time_d, double heading, double rawspeed, double *lon, double *lat,
                     double *speed, int *error);
int mb_navint_prjinterp(int verbose, void *mbio_ptr, double time_d, double heading, double rawspeed, double *easting,
                        double *northing, double *speed, int *error);
int mb_attint_add(int verbose, void *mbio_ptr, double time_d, double heave, double roll, double pitch, int *error);
int mb_attint_nadd(int verbose, void *mbio_ptr, int nsamples, double *time_d, double *heave, double *roll, double *pitch,
                   int *error);
int mb_attint_interp(int verbose, void *mbio_ptr, double time_d, double *heave, double *roll, double *pitch, int *error);
int mb_hedint_add(int verbose, void *mbio_ptr, double time_d, double heading, int *error);
int mb_hedint_nadd(int verbose, void *mbio_ptr, int nsamples, double *time_d, double *heading, int *error);
int mb_hedint_interp(int verbose, void *mbio_ptr, double time_d, double *heading, int *error);
int mb_depint_add(int verbose, void *mbio_ptr, double time_d, double sensordepth, int *error);
int mb_depint_interp(int verbose, void *mbio_ptr, double time_d, double *sensordepth, int *error);
int mb_altint_add(int verbose, void *mbio_ptr, double time_d, double altitude, int *error);
int mb_altint_interp(int verbose, void *mbio_ptr, double time_d, double *altitude, int *error);
int mb_loadnavdata(int verbose, char *merge_nav_file, int merge_nav_format, int merge_nav_lonflip, int *merge_nav_num,
                   int *merge_nav_alloc, double **merge_nav_time_d, double **merge_nav_lon, double **merge_nav_lat,
                   double **merge_nav_speed, int *error);
int mb_loadsensordepthdata(int verbose, char *merge_sensordepth_file, int merge_sensordepth_format, int *merge_sensordepth_num,
                           int *merge_sensordepth_alloc, double **merge_sensordepth_time_d,
                           double **merge_sensordepth_sensordepth, int *error);
int mb_loadaltitudedata(int verbose, char *merge_altitude_file, int merge_altitude_format, int *merge_altitude_num,
                        int *merge_altitude_alloc, double **merge_altitude_time_d, double **merge_altitude_altitude, int *error);
int mb_loadheadingdata(int verbose, char *merge_heading_file, int merge_heading_format, int *merge_heading_num,
                       int *merge_heading_alloc, double **merge_heading_time_d, double **merge_heading_heading, int *error);
int mb_loadattitudedata(int verbose, char *merge_attitude_file, int merge_attitude_format, int *merge_attitude_num,
                        int *merge_attitude_alloc, double **merge_attitude_time_d, double **merge_attitude_roll,
                        double **merge_attitude_pitch, double **merge_attitude_heave, int *error);
int mb_loadsoundspeeddata(int verbose, char *merge_soundspeed_file, int merge_soundspeed_format, int *merge_soundspeed_num,
                          int *merge_soundspeed_alloc, double **merge_soundspeed_time_d, double **merge_soundspeed_soundspeed,
                          int *error);
int mb_loadtimeshiftdata(int verbose, char *merge_timeshift_file, int merge_timeshift_format, int *merge_timeshift_num,
                         int *merge_timeshift_alloc, double **merge_timeshift_time_d, double **merge_timeshift_timeshift,
                         int *error);
int mb_apply_time_latency(int verbose, int data_num, double *data_time_d, int time_latency_mode, double time_latency_static,
                          int time_latency_num, double *time_latency_time_d, double *time_latency_value, int *error);
int mb_apply_time_filter(int verbose, int data_num, double *data_time_d, double *data_value, double filter_length, int *error);

int mb_swap_check(void);
int mb_get_double(double *, char *, int);
int mb_get_int(int *, char *, int);

int mb_get_binary_short(bool swapped, void *buffer, const void *ptr);
int mb_get_binary_int(bool swapped, void *buffer, const void *ptr);
int mb_get_binary_float(bool swapped, void *buffer, const void *ptr);
int mb_get_binary_double(bool swapped, void *buffer, const void *ptr);
int mb_get_binary_long(bool swapped, void *buffer, const void *ptr);
int mb_put_binary_short(bool swapped, short value, void *buffer);
int mb_put_binary_int(bool swapped, int value, void *buffer);
int mb_put_binary_float(bool swapped, float value, void *buffer);
int mb_put_binary_double(bool swapped, double value, void *buffer);
int mb_put_binary_long(bool swapped, mb_s_long value, void *buffer);

int mb_get_bounds(char *text, double *bounds);
double mb_ddmmss_to_degree(const char *text);
int mb_takeoff_to_rollpitch(int verbose, double theta, double phi, double *pitch, double *roll, int *error);
int mb_rollpitch_to_takeoff(int verbose, double pitch, double roll, double *theta, double *phi, int *error);
int mb_xyz_to_takeoff(int verbose, double x, double y, double z, double *theta, double *phi, int *error);
int mb_lever(int verbose, double sonar_offset_x, double sonar_offset_y, double sonar_offset_z, double nav_offset_x,
             double nav_offset_y, double nav_offset_z, double vru_offset_x, double vru_offset_y, double vru_offset_z,
             double vru_pitch, double vru_roll, double *lever_x, double *lever_y, double *lever_z, int *error);
//int mb_mergesort(void *base, size_t nmemb, register size_t size, int (*cmp)(const void *, const void *));
int mb_mergesort(void *base, size_t nmemb, size_t size, int (*cmp)(const void *, const void *));
int mb_double_compare(const void *a, const void *b);
int mb_int_compare(const void *a, const void *b);
int mb_edit_compare(const void *a, const void *b);
int mb_edit_compare_coarse(const void *a, const void *b);
void hilbert(int n, double delta[], double kappa[]);
void hilbert2(int n, double data[]);

int mb_absorption(int verbose, double frequency, double temperature, double salinity, double depth, double ph, double soundspeed,
                  double *absorption, int *error);
int mb_potential_temperature(int verbose, double temperature, double salinity, double pressure, double *potential_temperature,
                             int *error);
int mb_seabird_density(int verbose, double salinity, double temperature,
             double pressure, double *density, int *error);
int mb_seabird_depth(int verbose, double pressure, double latitude, double *depth, int *error);
int mb_seabird_salinity(int verbose, double conductivity, double temperature,
            double pressure, double *salinity, int *error);
int mb_seabird_soundspeed(int verbose, int algorithm, double salinity,
              double temperature, double pressure,
              double *soundspeed, int *error);

int mb_mem_list_enable(int verbose, int *error);
int mb_mem_list_disable(int verbose, int *error);
int mb_mem_debug_on(int verbose, int *error);
int mb_mem_debug_off(int verbose, int *error);
int mb_malloc(int verbose, size_t size, void **ptr, int *error);
int mb_realloc(int verbose, size_t size, void **ptr, int *error);
int mb_free(int verbose, void **ptr, int *error);
int mb_mallocd(int verbose, const char *sourcefile, int sourceline, size_t size, void **ptr, int *error);
int mb_reallocd(int verbose, const char *sourcefile, int sourceline, size_t size, void **ptr, int *error);
int mb_freed(int verbose, const char *sourcefile, int sourceline, void **ptr, int *error);
int mb_memory_clear(int verbose, int *error);
int mb_memory_status(int verbose, int *nalloc, int *nallocmax, int *overflow, size_t *allocsize, int *error);
int mb_memory_list(int verbose, int *error);
int mb_register_array(int verbose, void *mbio_ptr, int type, size_t size, void **handle, int *error);
int mb_update_arrays(int verbose, void *mbio_ptr, int nbath, int namp, int nss, int *error);
int mb_update_arrayptr(int verbose, void *mbio_ptr, void **handle, int *error);
int mb_list_arrays(int verbose, void *mbio_ptr, int *error);
int mb_deall_ioarrays(int verbose, void *mbio_ptr, int *error);

int mb_get_time(int verbose, int time_i[7], double *time_d);
int mb_get_date(int verbose, double time_d, int time_i[7]);
int mb_get_date_string(int verbose, double time_d, char *string);
int mb_get_jtime(int verbose, int time_i[7], int time_j[5]);
int mb_get_itime(int verbose, int time_j[5], int time_i[7]);
char *mb_day_name(int verbose, int day);
char *mb_month_name(int verbose, int month);
int mb_fix_y2k(int verbose, int year_short, int *year_long);
int mb_unfix_y2k(int verbose, int year_long, int *year_short);

int mb_proj_init(int verbose, char *projection, void **pjptr, int *error);
int mb_proj_free(int verbose, void **pjptr, int *error);
int mb_proj_forward(int verbose, void *pjptr, double lon, double lat, double *easting, double *northing, int *error);
int mb_proj_inverse(int verbose, void *pjptr, double easting, double northing, double *lon, double *lat, int *error);
int mb_geod_init(int verbose, double radius_equatorial, double flattening, void **g_ptr, int *error);
int mb_geod_free(int verbose, void **g_ptr, int *error);
int mb_geod_inverse(int verbose, void *g_ptr,
                    double lat1, double lon1, double lat2, double lon2,
                    double *distance, double *azimuth1, double *azimuth2, int *error);

/* mb_spline function prototypes */
int mb_spline_init(int verbose, const double *x, const double *y, int n, double yp1, double ypn, double *y2, int *error);
int mb_spline_interp(int verbose, const double *xa, const double *ya, double *y2a, int n, double x, double *y, int *i, int *error);
int mb_linear_interp(int verbose, const double *xa, const double *ya, int n, double x, double *y, int *i, int *error);
int mb_linear_interp_longitude(int verbose, const double *xa, const double *ya, int n, double x, double *y, int *i, int *error);
int mb_linear_interp_latitude(int verbose, const double *xa, const double *ya, int n, double x, double *y, int *i, int *error);
int mb_linear_interp_heading(int verbose, const double *xa, const double *ya, int n, double x, double *y, int *i, int *error);

/* byte swap function prototypes */
int mb_swap_check();
int mb_swap_float(float *a);
int mb_swap_double(double *a);
int mb_swap_long(mb_s_long *a);

/* beam angle calculation function prototypes */
int mb_beaudoin(int verbose, mb_3D_orientation tx_align, mb_3D_orientation tx_orientation, double tx_steer,
                mb_3D_orientation rx_align, mb_3D_orientation rx_orientation, double rx_steer, double reference_heading,
                double *beamAzimuth, double *beamDepression, int *error);
int mb_beaudoin_unrotate(int verbose, mb_3D_vector orig, mb_3D_orientation rotate, mb_3D_vector *final, int *error);

/* mb_rt function prototypes */
int mb_rt_init(int verbose, int number_node, double *depth, double *velocity, void **modelptr, int *error);
int mb_rt_deall(int verbose, void **modelptr, int *error);
int mb_rt(int verbose, void *modelptr, double source_depth, double source_angle, double end_time, int ssv_mode,
          double surface_vel, double null_angle, int nplot_max,
          int *nplot, double *xplot, double *zplot, double *tplot,
          double *x, double *z, double *travel_time, int *ray_stat, int *error);
          
/* mb_bitpack C API function prototypes */
void *mb_bitpack_new();
void mb_bitpack_delete(void **mbbpptr);
void mb_bitpack_clear(void *mbbpptr);
void mb_bitpack_setbitsize(void *mbbpptr, unsigned int nbits);
bool mb_bitpack_resize(void *mbbpptr, unsigned int arraySize, char **buffer, unsigned int* buffer_size);
int mb_bitpack_getbytestoread(void *mbbpptr);
int mb_bitpack_getbytestowrite(void *mbbpptr);
bool mb_bitpack_readvalue(void *mbbpptr, unsigned int* value);
bool mb_bitpack_writevalue(void *mbbpptr, unsigned int value); 

#ifdef __cplusplus
}  /* extern "C" */
#endif

/*--------------------------------------------------------------------
 * Windows / MSVC POSIX compatibility shim (formerly include/mb_win_compat.h).
 * Folded into mb_define.h per MB-System dev request (no root include/ dir).
 * mb_define.h is force-included on every TU on MSVC (top-level CMakeLists.txt
 * /FI option), so this shim reaches all sources. No-op off Windows.
 *------------------------------------------------------------------*/
#ifdef _WIN32
#if defined(_MSC_VER)

/* Expose M_PI, M_E etc from <math.h> — MSVC gates them behind this. */
#ifndef _USE_MATH_DEFINES
#define _USE_MATH_DEFINES
#endif

/* Suppress Windows.h's min/max macros that clobber std::numeric_limits<>::max()
   and similar; trim rarely-used Win32 sub-APIs to speed parsing.
   NOGDI suppresses wingdi.h's ERROR macro (collides with cproj.h ERROR define). */
#ifndef NOMINMAX
#define NOMINMAX
#endif
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOGDI
#define NOGDI
#endif

#include <time.h>     /* struct timespec (VS2015+) */
#include <sys/types.h>/* mode_t (POSIX), via _CRT_NONSTDC_NO_DEPRECATE */
#include <sys/stat.h> /* _stat / S_IFDIR / S_IFREG */
#include <direct.h>   /* _mkdir */
#include <BaseTsd.h>  /* SSIZE_T */
#include <stdbool.h>  /* bool / true / false for C TUs that forget to include it */
#include <stdint.h>   /* uint32_t / int64_t / etc */
#include <inttypes.h> /* PRIu32 / PRId64 / PRIx32 etc — POSIX format macros */
#include <stdio.h>    /* stderr / stdout / FILE */
#include <malloc.h>   /* _alloca */
#include <io.h>       /* _access, R_OK/W_OK constants live in unistd_w.h */
#include <winsock2.h> /* struct timeval, sockets — MUST precede windows.h on MSVC */
#include <ws2tcpip.h>
#include <windows.h>

/* POSIX alloca() — MSVC has _alloca (in malloc.h). Used as a portable
   stand-in for C99 variable-length arrays, which MSVC does not support. */
#ifndef alloca
#define alloca(n) _alloca(n)
#endif

/* POSIX ssize_t — MSVC has SSIZE_T (BaseTsd.h) but not the lowercase POSIX name. */
#ifndef _SSIZE_T_DEFINED
typedef SSIZE_T ssize_t;
#define _SSIZE_T_DEFINED
#endif

/* POSIX mode_t — UCRT's <sys/types.h> exposes mode_t inconsistently (depends
   on NO_OLDNAMES / _CRT_DECLARE_NONSTDC_NAMES). Provide a guarded typedef
   matching iowrap-posix.h's _MODE_T_DEFINED guard so the two don't collide. */
#ifndef _MODE_T_DEFINED
typedef int mode_t;
#define _MODE_T_DEFINED
#endif
/* pthreads-win32 semaphore.h gates its own mode_t typedef on HAVE_MODE_T;
   announce ours so it doesn't redefine. */
#ifndef HAVE_MODE_T
#define HAVE_MODE_T
#endif

#ifndef CLOCK_REALTIME
#define CLOCK_REALTIME  0
#endif
#ifndef CLOCK_MONOTONIC
#define CLOCK_MONOTONIC 1
#endif

#ifdef __cplusplus
extern "C" {
#endif

/* clock_gettime() emulation using Win32 high-precision time. */
static __inline int mb_win_clock_gettime(int clk_id, struct timespec *ts)
{
    (void)clk_id;
    FILETIME ft;
    ULARGE_INTEGER ull;
    /* 100-ns intervals between 1601-01-01 and 1970-01-01 Unix epoch. */
    const unsigned long long EPOCH_DIFF_100NS = 116444736000000000ULL;
    GetSystemTimePreciseAsFileTime(&ft);
    ull.LowPart  = ft.dwLowDateTime;
    ull.HighPart = ft.dwHighDateTime;
    unsigned long long t = ull.QuadPart - EPOCH_DIFF_100NS;
    ts->tv_sec  = (time_t)(t / 10000000ULL);
    ts->tv_nsec = (long)((t % 10000000ULL) * 100);
    return 0;
}

#define clock_gettime(clk, ts) mb_win_clock_gettime((clk), (ts))

/* POSIX stat-mode predicates (MSVC has only _S_IFDIR etc., no S_IS* macros). */
#ifndef S_ISDIR
#define S_ISDIR(m)  (((m) & _S_IFMT) == _S_IFDIR)
#endif
#ifndef S_ISREG
#define S_ISREG(m)  (((m) & _S_IFMT) == _S_IFREG)
#endif
/* Windows has no symlink concept at stat-mode level; always false. */
#ifndef S_ISLNK
#define S_ISLNK(m)  (0)
#endif

/* POSIX getopt — prebuilt lib supplied via ConfigUser.cmake (GETOPT_LIBRARY).
   Force-included so utilities don't need explicit #include <getopt.h>. */
#include <getopt.h>

/* POSIX realpath(path, resolved) — MSVC has _fullpath with arg order
   (resolved, path, max_len). Wrap. NULL resolved => malloc'd buffer (PATH_MAX).*/
#ifndef realpath
#define realpath(path, resolved) _fullpath((resolved), (path), _MAX_PATH)
#endif

/* POSIX sleep(seconds) — Windows has Sleep(milliseconds) (note capital S). */
#ifndef sleep
#define sleep(s) Sleep((s) * 1000)
#endif
#ifndef usleep
#define usleep(us) Sleep((us) / 1000)
#endif

/* POSIX popen / pclose — MSVC has _popen / _pclose with same signatures. */
#ifndef popen
#define popen  _popen
#endif
#ifndef pclose
#define pclose _pclose
#endif

/* POSIX strcasecmp / strncasecmp — MSVC has _stricmp / _strnicmp. */
#include <string.h>
#ifndef strcasecmp
#define strcasecmp  _stricmp
#endif
#ifndef strncasecmp
#define strncasecmp _strnicmp
#endif

/* POSIX strtok_r — MSVC has strtok_s with same signature semantics. */
#ifndef strtok_r
#define strtok_r(str, delim, saveptr) strtok_s((str), (delim), (saveptr))
#endif

/* POSIX file-open flags missing from MSVC <fcntl.h>: NONBLOCK/SYNC/DSYNC.
   Windows file I/O is blocking + synchronous by default; treat as no-op. */
#include <fcntl.h>
#ifndef O_NONBLOCK
#define O_NONBLOCK 0
#endif
#ifndef O_SYNC
#define O_SYNC 0
#endif
#ifndef O_DSYNC
#define O_DSYNC 0
#endif

/* access() mode flags — also defined in unistd_w.h; define here too so TUs
   that pull mb_define.h via /FI but never include <unistd.h> still see them. */
#ifndef F_OK
#define F_OK 00
#define R_OK 04
#define W_OK 02
#define X_OK 01
#endif

/* fcntl() commands — only matter for socket non-blocking toggling; not used at
   build time, just need to parse. Stub as 0 so source compiles; runtime call
   would need ioctlsocket replacement, but most call paths are gated to POSIX. */
#ifndef F_GETFL
#define F_GETFL 0
#endif
#ifndef F_SETFL
#define F_SETFL 0
#endif

/* POSIX stdio recursive lock — Windows CRT has _lock_file / _unlock_file. */
#ifndef flockfile
#define flockfile(f)   _lock_file(f)
#endif
#ifndef funlockfile
#define funlockfile(f) _unlock_file(f)
#endif

/* MSG_NOSIGNAL — POSIX flag to send() that suppresses SIGPIPE. Windows has no
   SIGPIPE, so 0 is correct. */
#ifndef MSG_NOSIGNAL
#define MSG_NOSIGNAL 0
#endif

/* SO_REUSEPORT — Linux extension; map to SO_REUSEADDR on Windows. */
#ifndef SO_REUSEPORT
#define SO_REUSEPORT SO_REUSEADDR
#endif

/* MSG_DONTWAIT — POSIX flag for non-blocking I/O. Winsock has no equivalent;
   the right way is ioctlsocket(FIONBIO). Map to 0 so code compiles; callers
   that want non-blocking must set it via msock_set_blocking() instead. */
#ifndef MSG_DONTWAIT
#define MSG_DONTWAIT 0
#endif

/* O_NOCTTY — POSIX open flag (don't acquire controlling tty); no-op on Win. */
#ifndef O_NOCTTY
#define O_NOCTTY 0
#endif

/* SIGSTOP / SIGTSTP / SIGCONT — POSIX job-control signals; not on Win. */
#ifndef SIGSTOP
#define SIGSTOP 19
#endif
#ifndef SIGTSTP
#define SIGTSTP 20
#endif
#ifndef SIGCONT
#define SIGCONT 18
#endif

/* struct timezone — legacy POSIX (gettimeofday 2nd arg); MSVC has no definition. */
#ifndef _STRUCT_TIMEZONE_DEFINED
#define _STRUCT_TIMEZONE_DEFINED
struct timezone {
    int tz_minuteswest;
    int tz_dsttime;
};
#endif

/* gettimeofday — POSIX; not in MSVC runtime. Use GetSystemTimePreciseAsFileTime. */
static __inline int mb_win_gettimeofday(struct timeval *tv, struct timezone *tz)
{
    FILETIME ft;
    ULARGE_INTEGER ui;
    /* Windows epoch (1601-01-01) to Unix epoch (1970-01-01) = 11644473600 sec. */
    static const unsigned long long EPOCH_DIFF = 116444736000000000ULL;
    if (tv) {
        GetSystemTimePreciseAsFileTime(&ft);
        ui.LowPart = ft.dwLowDateTime;
        ui.HighPart = ft.dwHighDateTime;
        ui.QuadPart -= EPOCH_DIFF;
        tv->tv_sec  = (long)(ui.QuadPart / 10000000ULL);
        tv->tv_usec = (long)((ui.QuadPart / 10ULL) % 1000000ULL);
    }
    if (tz) {
        tz->tz_minuteswest = 0;
        tz->tz_dsttime = 0;
    }
    return 0;
}
#define gettimeofday(tv, tz) mb_win_gettimeofday((tv), (tz))

/* Winsock setsockopt/getsockopt take const char* for optval; POSIX takes
   const void*. Wrap with cast so POSIX-flavoured call sites compile. The
   wrapper bodies are defined BEFORE the macros so they call the real WS2 fn. */
static __inline int mb_win_setsockopt(SOCKET s, int level, int optname, const void *optval, int optlen)
{
    return setsockopt(s, level, optname, (const char *)optval, optlen);
}
static __inline int mb_win_getsockopt(SOCKET s, int level, int optname, void *optval, int *optlen)
{
    return getsockopt(s, level, optname, (char *)optval, optlen);
}
#define setsockopt(s, lev, opt, val, len)  mb_win_setsockopt((s), (lev), (opt), (val), (len))
#define getsockopt(s, lev, opt, val, len)  mb_win_getsockopt((s), (lev), (opt), (val), (len))

/* POSIX shutdown how-flags — winsock has SD_RECEIVE/SD_SEND/SD_BOTH. */
#ifndef SHUT_RD
#define SHUT_RD   SD_RECEIVE
#endif
#ifndef SHUT_WR
#define SHUT_WR   SD_SEND
#endif
#ifndef SHUT_RDWR
#define SHUT_RDWR SD_BOTH
#endif

/* POSIX signals — Windows <signal.h> has SIGINT/TERM/etc but no SIGHUP/SIGPIPE,
   and no struct sigaction. Provide minimal shim so signal-setup boilerplate
   in mbtrn utilities parses (sigaction() forwards to signal()). */
#include <signal.h>
#ifndef SIGHUP
#define SIGHUP  1
#endif
#ifndef SIGQUIT
#define SIGQUIT 3
#endif
#ifndef SIGKILL
#define SIGKILL 9
#endif
#ifndef SIGPIPE
#define SIGPIPE 13
#endif
#ifndef SIGALRM
#define SIGALRM 14
#endif
#ifndef SIGCHLD
#define SIGCHLD 17
#endif
#ifndef SIGUSR1
#define SIGUSR1 30
#endif
#ifndef SIGUSR2
#define SIGUSR2 31
#endif

#ifndef MB_WIN_SIGACTION_DEFINED
#define MB_WIN_SIGACTION_DEFINED
typedef int sigset_t;
struct sigaction {
    void (*sa_handler)(int);
    sigset_t sa_mask;
    int sa_flags;
};
static __inline int sigemptyset(sigset_t *s)             { if (s) *s = 0; return 0; }
static __inline int sigfillset(sigset_t *s)              { if (s) *s = ~0; return 0; }
static __inline int sigaddset(sigset_t *s, int n)        { (void)n; if (s) *s |= 1; return 0; }
static __inline int sigdelset(sigset_t *s, int n)        { (void)n; if (s) *s &= ~1; return 0; }
static __inline int sigismember(const sigset_t *s, int n){ (void)n; return s ? (*s & 1) : 0; }
static __inline int sigaction(int sig, const struct sigaction *act, struct sigaction *old)
{
    (void)old;
    if (act && act->sa_handler) signal(sig, act->sa_handler);
    return 0;
}
#endif

/* POSIX setenv — MSVC has _putenv_s with (name, value); ignore overwrite. */
#include <stdlib.h>
static __inline int mb_win_setenv(const char *name, const char *value, int overwrite)
{
    (void)overwrite;
    if (!name || !value) return -1;
    return _putenv_s(name, value);
}
#define setenv(n, v, o) mb_win_setenv((n), (v), (o))
#define unsetenv(n)     _putenv_s((n), "")

/* POSIX readlink — no Windows symlink semantics worth emulating; stub to -1. */
static __inline long mb_win_readlink_stub(const char *path, char *buf, unsigned long bufsiz)
{
    (void)path; (void)buf; (void)bufsiz;
    return -1;
}
#define readlink(p, b, n) mb_win_readlink_stub((p), (b), (n))

/* POSIX mode-bit constants. MSVC defines only _S_IREAD/_S_IWRITE and a few
   _S_I* aliases; the POSIX names (and group/other variants) are missing.
   Map missing ones onto user bits — Windows ACLs ignore group/other anyway. */
#ifndef S_IRUSR
#define S_IRUSR _S_IREAD
#endif
#ifndef S_IWUSR
#define S_IWUSR _S_IWRITE
#endif
#ifndef S_IXUSR
#define S_IXUSR 0
#endif
#ifndef S_IRWXU
#define S_IRWXU (S_IRUSR | S_IWUSR | S_IXUSR)
#endif
#ifndef S_IRGRP
#define S_IRGRP 0
#endif
#ifndef S_IWGRP
#define S_IWGRP 0
#endif
#ifndef S_IXGRP
#define S_IXGRP 0
#endif
#ifndef S_IRWXG
#define S_IRWXG 0
#endif
#ifndef S_IROTH
#define S_IROTH 0
#endif
#ifndef S_IWOTH
#define S_IWOTH 0
#endif
#ifndef S_IXOTH
#define S_IXOTH 0
#endif
#ifndef S_IRWXO
#define S_IRWXO 0
#endif

/* localtime_r(time_t*, struct tm*) -> MSVC has localtime_s(struct tm*, time_t*) with swapped args. */
#define localtime_r(tp, tmp)  (localtime_s((tmp), (tp)) == 0 ? (tmp) : NULL)
#define gmtime_r(tp, tmp)     (gmtime_s((tmp), (tp))    == 0 ? (tmp) : NULL)

/* POSIX mkdir(path, mode); MSVC has _mkdir(path) only. Ignore mode on Windows. */
#define mkdir(path, mode)  _mkdir(path)

/* POSIX file ops missing on MSVC — implement via Win32 / CRT equivalents.
   Used by mbtrnframe (mfile.c, mtime.c, msocket.c) and a few utilities. */
#include <stdarg.h>
static __inline int mb_win_ftruncate(int fd, long long length)
{
    return _chsize_s(fd, length) == 0 ? 0 : -1;
}
#define ftruncate(fd, len) mb_win_ftruncate((fd), (long long)(len))

static __inline int mb_win_fsync(int fd) { return _commit(fd); }
#define fsync(fd) mb_win_fsync(fd)

static __inline int mb_win_vdprintf(int fd, const char *fmt, va_list ap)
{
    char buf[4096];
    int n = vsnprintf(buf, sizeof(buf), fmt, ap);
    if (n < 0) return -1;
    if (n > (int)sizeof(buf)) n = (int)sizeof(buf);
    return _write(fd, buf, (unsigned)n);
}
#define vdprintf(fd, fmt, ap) mb_win_vdprintf((fd), (fmt), (ap))

/* fcntl — only used for nonblocking sockets in mbtrn code; ioctlsocket would be
   the proper replacement but call sites tolerate failure. Stub to 0. */
static __inline int mb_win_fcntl_stub(int fd, int cmd, ...) { (void)fd; (void)cmd; return 0; }
#define fcntl mb_win_fcntl_stub

/* nanosleep — millisecond resolution via Win32 Sleep. */
static __inline int mb_win_nanosleep(const struct timespec *req, struct timespec *rem)
{
    (void)rem;
    if (!req) return -1;
    DWORD ms = (DWORD)(req->tv_sec * 1000ULL + req->tv_nsec / 1000000ULL);
    Sleep(ms);
    return 0;
}
#define nanosleep(req, rem) mb_win_nanosleep((req), (rem))

/* clock_getres / clock_setres — report 1 ns resolution; setres no-op. */
static __inline int mb_win_clock_getres(int clk, struct timespec *ts)
{
    (void)clk;
    if (ts) { ts->tv_sec = 0; ts->tv_nsec = 1; }
    return 0;
}
#define clock_getres(clk, ts) mb_win_clock_getres((clk), (ts))

static __inline int mb_win_clock_setres(int clk, struct timespec *ts)
{
    (void)clk; (void)ts; return 0;
}
#define clock_setres(clk, ts) mb_win_clock_setres((clk), (ts))

/* Minimal POSIX strptime — handles the format specifiers used by mbtrn
   (%Y %j %H %M %S %m %d). Returns pointer past last char consumed, or NULL. */
static __inline char *mb_win_strptime(const char *s, const char *fmt, struct tm *tm)
{
    while (*fmt && *s) {
        if (*fmt == '%' && fmt[1]) {
            int val = 0, n = 0;
            char c = fmt[1];
            if (c == 'Y') { if (sscanf(s, "%4d%n", &val, &n) != 1) return NULL; tm->tm_year = val - 1900; }
            else if (c == 'm') { if (sscanf(s, "%2d%n", &val, &n) != 1) return NULL; tm->tm_mon  = val - 1; }
            else if (c == 'd') { if (sscanf(s, "%2d%n", &val, &n) != 1) return NULL; tm->tm_mday = val; }
            else if (c == 'H') { if (sscanf(s, "%2d%n", &val, &n) != 1) return NULL; tm->tm_hour = val; }
            else if (c == 'M') { if (sscanf(s, "%2d%n", &val, &n) != 1) return NULL; tm->tm_min  = val; }
            else if (c == 'S') { if (sscanf(s, "%2d%n", &val, &n) != 1) return NULL; tm->tm_sec  = val; }
            else if (c == 'j') {
                if (sscanf(s, "%3d%n", &val, &n) != 1) return NULL;
                tm->tm_yday = val - 1;
                /* Convert day-of-year → month/day (needed by mktime). */
                static const int mdays[] = {31,28,31,30,31,30,31,31,30,31,30,31};
                int year = tm->tm_year + 1900;
                int leap = (year%4==0 && (year%100!=0 || year%400==0)) ? 1 : 0;
                int d = val, m = 0;
                while (m < 12) {
                    int dim = mdays[m] + ((m==1) ? leap : 0);
                    if (d <= dim) break;
                    d -= dim; m++;
                }
                tm->tm_mon = m; tm->tm_mday = d;
            }
            else return NULL;
            s += n; fmt += 2;
        } else if (*fmt == *s || *fmt == ' ') {
            while (*fmt == ' ') fmt++;
            while (*s == ' ') s++;
            if (*fmt == *s) { fmt++; s++; }
        } else return NULL;
    }
    return (char *)s;
}
#define strptime(s, fmt, tm) mb_win_strptime((s), (fmt), (tm))

/* strsignal — POSIX; not in MSVC runtime. Return signal number as static string. */
static __inline const char *mb_win_strsignal(int sig)
{
    static char buf[32];
    _snprintf_s(buf, sizeof(buf), _TRUNCATE, "signal %d", sig);
    return buf;
}
#define strsignal(sig) mb_win_strsignal(sig)

/* No symlinks on plain Windows builds. lstat == stat; symlink() not supported. */
#define lstat(path, sb)    stat((path), (sb))
static __inline int mb_win_symlink_unsupported(const char *target, const char *linkpath)
{
    (void)target; (void)linkpath;
    return -1;  /* errno left untouched; caller treats nonzero as failure */
}
#define symlink(t, l)  mb_win_symlink_unsupported((t), (l))

#ifdef __cplusplus
}
#endif

#endif /* _MSC_VER */
#endif /* _WIN32 */

#endif /* MB_DEFINE_H_ */
