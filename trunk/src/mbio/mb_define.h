/*--------------------------------------------------------------------
 *    The MB-system:	mb_io.h	4/21/96
 *    $Id: mb_define.h,v 5.0 2000-12-01 22:48:41 caress Exp $
 *
 *    Copyright (c) 1996, 2000 by
 *    David W. Caress (caress@mbari.org)
 *      Monterey Bay Aquarium Research Institute
 *      Moss Landing, CA 95039
 *    and Dale N. Chayes (dale@ldeo.columbia.edu)
 *      Lamont-Doherty Earth Observatory
 *      Palisades, NY 10964
 *
 *    See README file for copying and redistribution conditions.
 *--------------------------------------------------------------------*/
/*
 * mb_define.h defines macros used by MB-System programs and functions
 * for degree/radian conversions and min/max calculations.
 *
 * Author:	D. W. Caress
 * Date:	April 21, 1996
 *
 * $Log: not supported by cvs2svn $
 * Revision 4.4  2000/09/30  06:29:44  caress
 * Snapshot for Dale.
 *
 * Revision 4.3  1998/12/17  23:01:15  caress
 * MB-System version 4.6beta4
 *
 * Revision 4.2  1998/10/05 17:46:15  caress
 * MB-System version 4.6beta
 *
 * Revision 4.1  1997/09/15  19:06:40  caress
 * Real Version 4.5
 *
 * Revision 4.0  1996/08/05  15:24:55  caress
 * Initial revision.
 *
 *
 */
 
/* include this code only once */
#ifndef MB_DEFINE_DEF
#define MB_DEFINE_DEF
 
/* declare buffer maximum */
#define	MB_BUFFER_MAX	5000

/* maximum path length in characters */
#define MB_PATH_MAXLINE 256

/* maximum comment length in characters */
#define MB_COMMENT_MAXLINE 1944

/* other string length defines */
#define MB_NAME_LENGTH		32
#define MB_DESCRIPTION_LENGTH	1024

/* maximum number of navigation points saved */
#define MB_NAV_SAVE_MAX 20

/* types of  files used by swath sonar data formats */
#define	MB_FILETYPE_NORMAL	1
#define	MB_FILETYPE_XDR		2
#define	MB_FILETYPE_GSF		3
 
/* type definitions of signed and unsigned char */
typedef unsigned char	mb_u_char;
#ifdef IRIX
typedef signed char	mb_s_char;
#endif
#ifdef IRIX64
typedef signed char	mb_s_char;
#endif
#ifdef SOLARIS
typedef signed char	mb_s_char;
#endif
#ifdef LINUX
typedef signed char	mb_s_char;
#endif
#ifdef LYNX
typedef signed char	mb_s_char;
#endif
#ifdef SUN
typedef char	mb_s_char;
#endif
#ifdef HPUX
typedef signed char	mb_s_char;
#endif
#ifdef OTHER
typedef signed char	mb_s_char;
#endif

/* declare PI if needed */
#ifndef M_PI
#define	M_PI	3.14159265358979323846
#endif

/* multiply this by degrees to get radians */
#define DTR	0.01745329251994329500

/* multiply this by radians to get degrees */
#define RTD	57.2957795130823230000

/* min max define */
#define	MIN(A, B)	((A) < (B) ? (A) : (B))
#define	MAX(A, B)	((A) > (B) ? (A) : (B))
	
/* MBIO core function prototypes */
int mb_format_info(int verbose, int *format, int *system, 
		int *beams_bath_max, int *beams_amp_max, int *pixels_ss_max, 
		char *format_name, char *system_name, char *format_description, 
		int *numfile, int *filetype, int *variable_beams, 
		int *traveltime, int *beam_flagging, 
		int *nav_source, int *heading_source, int *vru_source, 
		double *beamwidth_xtrack, double *beamwidth_ltrack, 
		int (**format_alloc)(), int (**format_free)(), 
		int (**store_alloc)(),  int (**store_free)(), 
		int (**read_ping)(), int (**write_ping)(), 
		int (**extract)(),  int (**insert)(), 
		int (**extract_nav)(), int (**insert_nav)(), 
		int (**altitude)(),  int (**insert_altitude)(), 
		int (**ttimes)(), int (**copyrecord)(), 
		int *error);
int mb_format(int verbose, int *format, int *error);
int mb_format_system(int verbose, int *format, int *system, int *error);
int mb_format_description(int verbose, int *format, 
		char **description, int *error);
int mb_format_dimensions(int verbose, int *format, 
		int *beams_bath_max, int *beams_amp_max, int *pixels_ss_max, 
		int *error);
int mb_format_flags(int verbose, int *format, 
		int *variable_beams, int *traveltime, int *beam_flagging, 
		int *error);
int mb_format_source(int verbose, int *format, 
		int *nav_source, int *heading_source, int *vru_source, 
		int *error);
int mb_format_beamwidth(int verbose, int *format, 
		double *beamwidth_xtrack, double *beamwidth_ltrack,
		int *error);
int mb_datalist_open(int verbose,
		char **datalist,
		char *path, int *error);
int mb_datalist_read(int verbose,
		char  *datalist,
		char *path, int *format, double *weight,
		int *error);
int mb_datalist_close(int verbose,
		char **datalist, int *error);
int mb_read_init(int verbose, char *file, 
		int format, int pings, int lonflip, double bounds[4],
		int btime_i[7], int etime_i[7], 
		double speedmin, double timegap,
		char **mbio_ptr, double *btime_d, double *etime_d,
		int *beams_bath, int *beams_amp, int *pixels_ss, 
		int *error);
int mb_write_init(int verbose, 
		char *file, int format, char **mbio_ptr, 
		int *beams_bath, int *beams_amp, int *pixels_ss,
		int *error);
int mb_read_ping(int verbose, char *mbio_ptr, char *store_ptr, 
		int *kind, int *error);
int mb_get_all(int verbose, char *mbio_ptr, char **store_ptr, int *kind,
		int time_i[7], double *time_d,
		double *navlon, double *navlat, double *speed, 
		double *heading, double *distance,
		int *nbath, int *namp, int *nss,
		char *beamflag, double *bath, double *amp,
		double *bathacrosstrack, double *bathalongtrack,
		double *ss, double *ssacrosstrack, double *ssalongtrack,
		char *comment, int *error);
int mb_get(int verbose, char *mbio_ptr, int *kind, int *pings, 
		int time_i[7], double *time_d,
		double *navlon, double *navlat, double *speed, 
		double *heading, double *distance,
		int *nbath, int *namp, int *nss,
		char *beamflag, double *bath, double *amp,
		double *bathacrosstrack, double *bathalongtrack,
		double *ss, double *ssacrosstrack, double *ssalongtrack,
		char *comment, int *error);
int mb_read(int verbose, char *mbio_ptr,
		int *kind, int *pings, 
		int time_i[7], double *time_d,
		double *navlon, double *navlat, 
		double *speed, double *heading, double *distance,
		int *nbath, int *namp, int *nss,
		char *beamflag, double *bath, double *amp, 
		double *bathlon, double *bathlat,
		double *ss, double *sslon, double *sslat,
		char *comment, int *error);
int mb_write_ping(int verbose, char *mbio_ptr, char *store_ptr, 
		int *error);
int mb_put_all(int verbose, char *mbio_ptr, char *store_ptr,
		int usevalues, int kind, 
		int time_i[7], double time_d,
		double navlon, double navlat, 
		double speed, double heading,
		int nbath, int namp, int nss,
		char *beamflag, double *bath, double *amp, 
		double *bathacrosstrack, double *bathalongtrack,
		double *ss, double *ssacrosstrack, double *ssalongtrack,
		char *comment, int *error);
int mb_put_comment(int verbose, char *mbio_ptr, char *comment, 
		int *error);
int mb_alloc(int verbose, char *mbio_ptr,
		char **store_ptr, int *error);
int mb_deall(int verbose, char *mbio_ptr,
		char **store_ptr, int *error);
int mb_insert(int verbose, char *mbio_ptr, char *store_ptr, 
		int kind, int time_i[7], double time_d,
		double navlon, double navlat,
		double speed, double heading,
		int nbath, int namp, int nss,
		char *beamflag, double *bath, double *amp, 
		double *bathacrosstrack, double *bathalongtrack,
		double *ss, double *ssacrosstrack, double *ssalongtrack,
		char *comment, int *error);
int mb_extract(int verbose, char *mbio_ptr, char *store_ptr, 
		int *kind, int time_i[7], double *time_d,
		double *navlon, double *navlat,
		double *speed, double *heading,
		int *nbath, int *namp, int *nss,
		char *beamflag, double *bath, double *amp, 
		double *bathacrosstrack, double *bathalongtrack,
		double *ss, double *ssacrosstrack, double *ssalongtrack,
		char *comment, int *error);
int mb_extract_nav(int verbose, char *mbio_ptr, char *store_ptr, int *kind,
		int time_i[7], double *time_d, 
		double *navlon, double *navlat,
		double *speed, double *heading, double *draft, 
		double *roll, double *pitch, double *heave, 
		int *error);
int mb_insert_nav(int verbose, char *mbio_ptr, char *store_ptr,
		int time_i[7], double time_d, 
		double navlon, double navlat,
		double speed, double heading, double draft, 
		double roll, double pitch, double heave, 
		int *error);
int mb_altitude(int verbose, char *mbio_ptr, char *store_ptr,
		int *kind,
		double *transducer_depth, double *altitude,
		int *error);
int mb_insert_altitude(int verbose, char *mbio_ptr, char *store_ptr,
		double transducer_depth, double altitude,
		int *error);
int mb_ttimes(int verbose, char *mbio_ptr, char *store_ptr,
		int *kind, int *nbeams,
		double *ttimes, double	*angles, 
		double *angles_forward, double *angles_null,
		double *heave, double *alongtrack_offset, 
		double *draft, double *ssv, int *error);
int mb_copyrecord(int verbose, char *mbio_ptr,
		char *store_ptr, char *copy_ptr, int *error);

int mb_buffer_init(int verbose, char **buff_ptr, int *error);
int mb_buffer_close(int verbose, char **buff_ptr, char *mbio_ptr, 
		int *error);
int mb_buffer_load(int verbose, char *buff_ptr,char *mbio_ptr,
		int nwant, int *nload, int *nbuff, int *error);
int mb_buffer_dump(int verbose, char *buff_ptr, char *mbio_ptr,
		int nhold, int *ndump, int *nbuff, int *error);
int mb_buffer_clear(int verbose, char *buff_ptr, char *mbio_ptr,
		int nhold, int *ndump, int *nbuff, int *error);
int mb_buffer_info(int verbose, char *buff_ptr, char *mbio_ptr,
		int id, int *system, int *kind, int *error);
int mb_buffer_get_next_data(int verbose, char *buff_ptr, char *mbio_ptr,
		int start, int *id,
		int time_i[7], double *time_d,
		double *navlon, double *navlat, 
		double *speed, double *heading,
		int *nbath, int *namp, int *nss,
		char *beamflag, double *bath, double *amp,
		double *bathacrosstrack, double *bathalongtrack,
		double *ss, double *ssacrosstrack, double *ssalongtrack,
		int *error);
int mb_buffer_get_next_nav(int verbose, char *buff_ptr, char *mbio_ptr,
		int start, int *id,
		int time_i[7], double *time_d,
		double *navlon, double *navlat, 
		double *speed, double *heading, double *draft, 
		double *roll, double *pitch, double *heave,
		int *error);
int mb_buffer_extract(int verbose, char *buff_ptr, char *mbio_ptr,
		int id, int *kind, 
		int time_i[7], double *time_d,
		double *navlon, double *navlat, 
		double *speed, double *heading,
		int *nbath, int *namp, int *nss,
		char *beamflag, double *bath, double *amp,
		double *bathacrosstrack, double *bathalongtrack,
		double *ss, double *ssacrosstrack, double *ssalongtrack,
		char *comment, 
		int *error);
int mb_buffer_extract_nav(int verbose, char *buff_ptr, char *mbio_ptr,
		int id, int *kind, 
		int time_i[7], double *time_d,
		double *navlon, double *navlat, 
		double *speed, double *heading, double *draft,
		double *roll, double *pitch, double *heave,
		int *error);
int mb_buffer_insert(int verbose, char *buff_ptr, char *mbio_ptr,
		int id, int time_i[7], double time_d,
		double navlon, double navlat, 
		double speed, double heading,
		int nbath, int namp, int nss,
		char *beamflag, double *bath, double *amp,
		double *bathacrosstrack, double *bathalongtrack,
		double *ss, double *ssacrosstrack, double *ssalongtrack,
		char *comment, 
		int *error);
int mb_buffer_insert_nav(int verbose, char *buff_ptr, char *mbio_ptr,
		int id, int time_i[7], double time_d,
		double navlon, double navlat, 
		double speed, double heading, double draft,
		double roll, double pitch, double heave,
		int *error);
int mb_buffer_get_ptr(int verbose, char *buff_ptr, char *mbio_ptr,
		int id, char **store_ptr, 
		int *error);

int mb_error(int, int, char **);
int mb_get_double(double *, char *, int);
int mb_get_int(int *, char *, int);
int mb_get_binary_short(int, void *, short *);
int mb_get_binary_int(int, void *, int *);
int mb_get_binary_float(int, void *, float *);
int mb_get_binary_double(int, void *, double *);
int mb_put_binary_short(int, short, void *);
int mb_put_binary_int(int, int, void *);
int mb_put_binary_float(int, float, void *);
int mb_put_binary_double(int, double, void *);
int mb_takeoff_to_rollpitch(int, double, double, double *, double *, int *);
int mb_rollpitch_to_takeoff(int, double, double, double *, double *, int *);
int mb_double_compare(double *a, double *b);
int mb_int_compare(int *a, int *b);

/* end conditional include */
#endif
