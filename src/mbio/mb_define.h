/*--------------------------------------------------------------------
 *    The MB-system:	mb_io.h	4/21/96
 *    $Id: mb_define.h,v 5.32 2006-11-10 22:36:04 caress Exp $
 *
 *    Copyright (c) 1996, 2000, 2002, 2003 by
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
 * Revision 5.31  2006/10/05 18:58:28  caress
 * Changes for 5.1.0beta4
 *
 * Revision 5.30  2006/09/11 18:55:52  caress
 * Changes during Western Flyer and Thomas Thompson cruises, August-September
 * 2006.
 *
 * Revision 5.29  2006/01/06 18:27:19  caress
 * Working towards 5.0.8
 *
 * Revision 5.28  2005/11/05 00:48:04  caress
 * Programs changed to register arrays through mb_register_array() rather than allocating the memory directly with mb_realloc() or mb_malloc().
 *
 * Revision 5.27  2004/12/18 01:34:43  caress
 * Working towards release 5.0.6.
 *
 * Revision 5.26  2004/12/02 06:33:30  caress
 * Fixes while supporting Reson 7k data.
 *
 * Revision 5.25  2004/11/06 03:55:17  caress
 * Working to support the Reson 7k format.
 *
 * Revision 5.24  2004/09/16 19:02:33  caress
 * Changes to better support segy data.
 *
 * Revision 5.23  2004/04/27 01:46:12  caress
 * Various updates of April 26, 2004.
 *
 * Revision 5.22  2003/09/23 20:56:00  caress
 * Added declaration of type mb_path.
 *
 * Revision 5.21  2003/07/26 17:59:32  caress
 * Changed beamflag handling code.
 *
 * Revision 5.20  2003/05/20 18:05:32  caress
 * Added svp_source to data source parameters.
 *
 * Revision 5.19  2003/04/17 21:05:23  caress
 * Release 5.0.beta30
 *
 * Revision 5.18  2002/07/20 20:42:40  caress
 * Release 5.0.beta20
 *
 * Revision 5.17  2002/05/29 23:40:48  caress
 * Release 5.0.beta18
 *
 * Revision 5.16  2002/05/02 04:00:41  caress
 * Release 5.0.beta17
 *
 * Revision 5.15  2002/04/08 20:59:38  caress
 * Release 5.0.beta17
 *
 * Revision 5.14  2002/04/06 02:43:39  caress
 * Release 5.0.beta16
 *
 * Revision 5.13  2002/02/22 09:03:43  caress
 * Release 5.0.beta13
 *
 * Revision 5.12  2002/01/24 02:28:29  caress
 * Added DARWIN.
 *
 * Revision 5.11  2001/12/18 04:27:45  caress
 * Release 5.0.beta11.
 *
 * Revision 5.10  2001/11/15  22:36:43  caress
 * Added function mb_get_shortest_path()
 *
 * Revision 5.9  2001/10/19  00:54:37  caress
 * Now tries to use relative paths.
 *
 * Revision 5.8  2001/10/12  21:10:41  caress
 * Added interpolation of attitude data.
 *
 * Revision 5.7  2001/09/17 23:25:13  caress
 * Added format 84
 *
 * Revision 5.6  2001/07/20  00:32:54  caress
 * Release 5.0.beta03
 *
 * Revision 5.5  2001/06/08 21:44:01  caress
 * Version 5.0.beta01
 *
 * Revision 5.4  2001/06/01  00:14:06  caress
 * Redid support for current Simrad multibeam data.
 *
 * Revision 5.3  2001/04/30  05:14:10  caress
 * Changes to MB-System defaults.
 *
 * Revision 5.2  2001/03/22 20:50:02  caress
 * Trying to make version 5.0.beta0
 *
 * Revision 5.1  2001/01/22  07:43:34  caress
 * Version 5.0.beta01
 *
 * Revision 5.0  2000/12/01  22:48:41  caress
 * First cut at Version 5.0.
 *
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
#define MB_PATH_MAXLINE 1024

/* maximum comment length in characters */
#define MB_COMMENT_MAXLINE 1944

/* other string length defines */
#define MB_NAME_LENGTH		32
#define MB_DESCRIPTION_LENGTH	2048

/* maximum number of asynchronous data saved */
#define MB_ASYNCH_SAVE_MAX 500

/* maximum size of SVP profiles */
#define MB_SVP_MAX 1024

/* file mode (read or write) */
#define	MB_FILEMODE_READ	0
#define	MB_FILEMODE_WRITE	1

/* types of  files used by swath sonar data formats */
#define	MB_FILETYPE_NORMAL	1
#define	MB_FILETYPE_XDR		2
#define	MB_FILETYPE_GSF		3
#define	MB_FILETYPE_NETCDF	4
#define MB_FILETYPE_SURF	5
#define MB_FILETYPE_SEGY	6

/* settings for recursive datalist reading functions */
#define MB_DATALIST_LOOK_UNSET		0
#define MB_DATALIST_LOOK_NO 		1
#define MB_DATALIST_LOOK_YES		2

/* settings of i/o array dimension types */
#define MB_MEM_TYPE_NONE		0
#define MB_MEM_TYPE_BATHYMETRY		1
#define MB_MEM_TYPE_AMPLITUDE		2
#define MB_MEM_TYPE_SIDESCAN		3

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
#ifdef DARWIN
typedef char	mb_s_char;
#endif
#ifdef OTHER
typedef signed char	mb_s_char;
#endif

/* type definitions of signed and unsigned long int (64 bit integer) */
typedef unsigned long long	mb_u_long;
typedef long long	mb_s_long;

/* typedef for path string */
typedef char mb_path[MB_PATH_MAXLINE];

/* declare PI if needed */
#ifndef M_PI
#define	M_PI	3.14159265358979323846
#endif

/* multiply this by degrees to get radians */
#define DTR	0.01745329251994329500

/* multiply this by radians to get degrees */
#define RTD	57.2957795130823230000

/* min max define */
#ifndef MIN
#define	MIN(A, B)	((A) < (B) ? (A) : (B))
#endif
#ifndef MAX
#define	MAX(A, B)	((A) > (B) ? (A) : (B))
#endif
	
/* MBIO core function prototypes */
int mb_defaults(int verbose, int *format, int *pings,
		int *lonflip, double bounds[4], 
		int *btime_i, int *etime_i,
		double *speedmin, double *timegap);
int mb_env(int verbose, char *psdisplay, char *imgdisplay, char *mbproject);
int mb_lonflip(int verbose, int *lonflip);
int mb_format_register(int verbose, int *format, void *mbio_ptr, 
		int *error);
int mb_format_info(int verbose, int *format, int *system, 
		int *beams_bath_max, int *beams_amp_max, int *pixels_ss_max, 
		char *format_name, char *system_name, char *format_description, 
		int *numfile, int *filetype, int *variable_beams, 
		int *traveltime, int *beam_flagging, 
		int *nav_source, int *heading_source, int *vru_source, int *svp_source, 
		double *beamwidth_xtrack, double *beamwidth_ltrack, 
		int *error);
int mb_format(int verbose, int *format, int *error);
int mb_format_system(int verbose, int *format, int *system, int *error);
int mb_format_description(int verbose, int *format, 
		char *description, int *error);
int mb_format_dimensions(int verbose, int *format, 
		int *beams_bath_max, int *beams_amp_max, int *pixels_ss_max, 
		int *error);
int mb_format_flags(int verbose, int *format, 
		int *variable_beams, int *traveltime, int *beam_flagging, 
		int *error);
int mb_format_source(int verbose, int *format, 
		int *nav_source, int *heading_source, 
		int *vru_source, int *svp_source, 
		int *error);
int mb_format_beamwidth(int verbose, int *format, 
		double *beamwidth_xtrack, double *beamwidth_ltrack,
		int *error);
int mb_datalist_open(int verbose,
		void **datalist,
		char *path, 
		int look_processed,
		int *error);
int mb_datalist_read(int verbose,
		void *datalist,
		char *path, int *format, double *weight,
		int *error);
int mb_datalist_close(int verbose,
		void **datalist, int *error);
int mb_get_relative_path(int verbose,
		char *path,
		char *pwd,
		int *error);
int mb_get_shortest_path(int verbose,
		char *path,
		int *error);
int mb_get_basename(int verbose,
		char *path,
		int *error);
int mb_check_info(int verbose, char *file, int lonflip, 
		    double bounds[4], int *file_in_bounds,
		    int *error);
int mb_make_info(int verbose, int force,
		    char *file, int format, int *error);
int mb_swathbounds(int verbose, int checkgood,
		double navlon, double navlat, double heading, 
		int nbath, int nss,
		char *beamflag, double *bath, 
		double *bathacrosstrack, double *bathalongtrack,
		double *ss, double *ssacrosstrack, double *ssalongtrack,
		int *ibeamport,
		int *ibeamcntr,
		int *ibeamstbd,
		int *ipixelport,
		int *ipixelcntr,
		int *ipixelstbd,
		int *error);
int mb_read_init(int verbose, char *file, 
		int format, int pings, int lonflip, double bounds[4],
		int btime_i[7], int etime_i[7], 
		double speedmin, double timegap,
		void **mbio_ptr, double *btime_d, double *etime_d,
		int *beams_bath, int *beams_amp, int *pixels_ss, 
		int *error);
int mb_write_init(int verbose, 
		char *file, int format, void **mbio_ptr, 
		int *beams_bath, int *beams_amp, int *pixels_ss,
		int *error);
int mb_close(int verbose, void **mbio_ptr, int *error);
int mb_register_array(int verbose, void *mbio_ptr, 
		int type, int size, void **handle, int *error);
int mb_update_arrays(int verbose, void *mbio_ptr, 
		int nbath, int namp, int nss, int *error);
int mb_update_arrayptr(int verbose, void *mbio_ptr, 
		void **handle, int *error);
int mb_read_ping(int verbose, void *mbio_ptr, void *store_ptr, 
		int *kind, int *error);
int mb_get_all(int verbose, void *mbio_ptr, void **store_ptr, int *kind,
		int time_i[7], double *time_d,
		double *navlon, double *navlat, 
		double *speed, double *heading, 
		double *distance, double *altitude, double *sonardepth, 
		int *nbath, int *namp, int *nss,
		char *beamflag, double *bath, double *amp,
		double *bathacrosstrack, double *bathalongtrack,
		double *ss, double *ssacrosstrack, double *ssalongtrack,
		char *comment, int *error);
int mb_get(int verbose, void *mbio_ptr, int *kind, int *pings, 
		int time_i[7], double *time_d,
		double *navlon, double *navlat, 
		double *speed, double *heading, 
		double *distance, double *altitude, double *sonardepth, 
		int *nbath, int *namp, int *nss,
		char *beamflag, double *bath, double *amp,
		double *bathacrosstrack, double *bathalongtrack,
		double *ss, double *ssacrosstrack, double *ssalongtrack,
		char *comment, int *error);
int mb_read(int verbose, void *mbio_ptr,
		int *kind, int *pings, 
		int time_i[7], double *time_d,
		double *navlon, double *navlat, 
		double *speed, double *heading, 
		double *distance, double *altitude, double *sonardepth, 
		int *nbath, int *namp, int *nss,
		char *beamflag, double *bath, double *amp, 
		double *bathlon, double *bathlat,
		double *ss, double *sslon, double *sslat,
		char *comment, int *error);
int mb_write_ping(int verbose, void *mbio_ptr, void *store_ptr, 
		int *error);
int mb_put_all(int verbose, void *mbio_ptr, void *store_ptr,
		int usevalues, int kind, 
		int time_i[7], double time_d,
		double navlon, double navlat, 
		double speed, double heading,
		int nbath, int namp, int nss,
		char *beamflag, double *bath, double *amp, 
		double *bathacrosstrack, double *bathalongtrack,
		double *ss, double *ssacrosstrack, double *ssalongtrack,
		char *comment, int *error);
int mb_put_comment(int verbose, void *mbio_ptr, char *comment, 
		int *error);
int mb_alloc(int verbose, void *mbio_ptr,
		void **store_ptr, int *error);
int mb_deall(int verbose, void *mbio_ptr,
		void **store_ptr, int *error);
int mb_dimensions(int verbose, void *mbio_ptr, void *store_ptr, 
		int *kind, int *nbath, int *namp, int *nss, int *error);
int mb_extract(int verbose, void *mbio_ptr, void *store_ptr, 
		int *kind, int time_i[7], double *time_d,
		double *navlon, double *navlat,
		double *speed, double *heading,
		int *nbath, int *namp, int *nss,
		char *beamflag, double *bath, double *amp, 
		double *bathacrosstrack, double *bathalongtrack,
		double *ss, double *ssacrosstrack, double *ssalongtrack,
		char *comment, int *error);
int mb_insert(int verbose, void *mbio_ptr, void *store_ptr, 
		int kind, int time_i[7], double time_d,
		double navlon, double navlat,
		double speed, double heading,
		int nbath, int namp, int nss,
		char *beamflag, double *bath, double *amp, 
		double *bathacrosstrack, double *bathalongtrack,
		double *ss, double *ssacrosstrack, double *ssalongtrack,
		char *comment, int *error);
int mb_extract_nav(int verbose, void *mbio_ptr, void *store_ptr, int *kind,
		int time_i[7], double *time_d, 
		double *navlon, double *navlat,
		double *speed, double *heading, double *draft, 
		double *roll, double *pitch, double *heave, 
		int *error);
int mb_extract_nnav(int verbose, void *mbio_ptr, void *store_ptr, 
		int nmax, int *kind, int *n,
		int *time_i, double *time_d, 
		double *navlon, double *navlat,
		double *speed, double *heading, double *draft, 
		double *roll, double *pitch, double *heave, 
		int *error);
int mb_insert_nav(int verbose, void *mbio_ptr, void *store_ptr,
		int time_i[7], double time_d, 
		double navlon, double navlat,
		double speed, double heading, double draft, 
		double roll, double pitch, double heave, 
		int *error);
int mb_extract_altitude(int verbose, void *mbio_ptr, void *store_ptr,
		int *kind,
		double *transducer_depth, double *altitude,
		int *error);
int mb_insert_altitude(int verbose, void *mbio_ptr, void *store_ptr,
		double transducer_depth, double altitude,
		int *error);
int mb_extract_svp(int verbose, void *mbio_ptr, void *store_ptr,
		int *kind,
		int *nsvp,
		double *depth, double *velocity,
		int *error);
int mb_insert_svp(int verbose, void *mbio_ptr, void *store_ptr,
		int nsvp,
		double *depth, double *velocity,
		int *error);
int mb_ttimes(int verbose, void *mbio_ptr, void *store_ptr,
		int *kind, int *nbeams,
		double *ttimes, double	*angles, 
		double *angles_forward, double *angles_null,
		double *heave, double *alongtrack_offset, 
		double *draft, double *ssv, int *error);
int mb_extract_rawss(int verbose, void *mbio_ptr, void *store_ptr,
		int *kind,
		int *nrawss,
		double *rawss, 
		double *rawssacrosstrack, 
		double *rawssalongtrack, 
		int *error);
int mb_insert_rawss(int verbose, void *mbio_ptr, void *store_ptr,
		int nrawss,
		double *rawss, 
		double *rawssacrosstrack, 
		double *rawssalongtrack, 
		int *error);
int mb_extract_segyheader(int verbose, void *mbio_ptr, void *store_ptr,
		int *kind,
		void *segyheader_ptr, 
		int *error);
int mb_extract_segy(int verbose, void *mbio_ptr, void *store_ptr,
		int *sampleformat,
		int *kind,
		void *segyheader_ptr, 
		float *segydata, 
		int *error);
int mb_insert_segy(int verbose, void *mbio_ptr, void *store_ptr,
		int kind,
		void *segyheader_ptr, 
		float *segydata, 
		int *error);
int mb_copyrecord(int verbose, void *mbio_ptr,
		void *store_ptr, void *copy_ptr, int *error);

int mb_buffer_init(int verbose, void **buff_ptr, int *error);
int mb_buffer_close(int verbose, void **buff_ptr, void *mbio_ptr, 
		int *error);
int mb_buffer_load(int verbose, void *buff_ptr,void *mbio_ptr,
		int nwant, int *nload, int *nbuff, int *error);
int mb_buffer_dump(int verbose, void *buff_ptr, void *mbio_ptr,
		int nhold, int *ndump, int *nbuff, int *error);
int mb_buffer_clear(int verbose, void *buff_ptr, void *mbio_ptr,
		int nhold, int *ndump, int *nbuff, int *error);
int mb_buffer_info(int verbose, void *buff_ptr, void *mbio_ptr,
		int id, int *system, int *kind, int *error);
int mb_buffer_get_next_data(int verbose, void *buff_ptr, void *mbio_ptr,
		int start, int *id,
		int time_i[7], double *time_d,
		double *navlon, double *navlat, 
		double *speed, double *heading,
		int *nbath, int *namp, int *nss,
		char *beamflag, double *bath, double *amp,
		double *bathacrosstrack, double *bathalongtrack,
		double *ss, double *ssacrosstrack, double *ssalongtrack,
		int *error);
int mb_buffer_get_next_nav(int verbose, void *buff_ptr, void *mbio_ptr,
		int start, int *id,
		int time_i[7], double *time_d,
		double *navlon, double *navlat, 
		double *speed, double *heading, double *draft, 
		double *roll, double *pitch, double *heave,
		int *error);
int mb_buffer_extract(int verbose, void *buff_ptr, void *mbio_ptr,
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
int mb_buffer_extract_nav(int verbose, void *buff_ptr, void *mbio_ptr,
		int id, int *kind, 
		int time_i[7], double *time_d,
		double *navlon, double *navlat, 
		double *speed, double *heading, double *draft,
		double *roll, double *pitch, double *heave,
		int *error);
int mb_buffer_insert(int verbose, void *buff_ptr, void *mbio_ptr,
		int id, int time_i[7], double time_d,
		double navlon, double navlat, 
		double speed, double heading,
		int nbath, int namp, int nss,
		char *beamflag, double *bath, double *amp,
		double *bathacrosstrack, double *bathalongtrack,
		double *ss, double *ssacrosstrack, double *ssalongtrack,
		char *comment, 
		int *error);
int mb_buffer_insert_nav(int verbose, void *buff_ptr, void *mbio_ptr,
		int id, int time_i[7], double time_d,
		double navlon, double navlat, 
		double speed, double heading, double draft,
		double roll, double pitch, double heave,
		int *error);
int mb_buffer_get_ptr(int verbose, void *buff_ptr, void *mbio_ptr,
		int id, void **store_ptr, 
		int *error);

int mb_coor_scale(int verbose, double latitude, 
		double *mtodeglon, double *mtodeglat);
int mb_apply_lonflip(int verbose, int lonflip, double *longitude);

int mb_error(int, int, char **);
int mb_notice_log_datatype(int verbose, void *mbio_ptr, int data_id);
int mb_notice_log_error(int verbose, void *mbio_ptr, int error_id);
int mb_notice_log_problem(int verbose, void *mbio_ptr, int problem_id);
int mb_notice_get_list(int verbose, void *mbio_ptr, int *notice_list);
int mb_notice_message(int verbose, int notice, char **message);
int mb_navint_add(int verbose, void *mbio_ptr, 
		double time_d, double lon, double lat, int *error);
int mb_navint_interp(int verbose, void *mbio_ptr, 
		double time_d, double heading, double rawspeed, 
		double *lon, double *lat, double *speed, 
		int *error);
int mb_attint_add(int verbose, void *mbio_ptr, 
		double time_d, double heave, 
		double roll, double pitch, int *error);
int mb_attint_interp(int verbose, void *mbio_ptr, 
		double time_d, double *heave, 
		double *roll, double *pitch, 
		int *error);
int mb_hedint_add(int verbose, void *mbio_ptr, 
		double time_d, double heading, int *error);
int mb_hedint_interp(int verbose, void *mbio_ptr, 
		double time_d, double *heading, 
		int *error);
int mb_swap_check();
int mb_get_double(double *, char *, int);
int mb_get_int(int *, char *, int);
int mb_get_binary_short(int, void *, short *);
int mb_get_binary_int(int, void *, int *);
int mb_get_binary_float(int, void *, float *);
int mb_get_binary_double(int, void *, double *);
int mb_get_binary_long(int, void *, mb_s_long *);
int mb_put_binary_short(int, short, void *);
int mb_put_binary_int(int, int, void *);
int mb_put_binary_float(int, float, void *);
int mb_put_binary_double(int, double, void *);
int mb_put_binary_long(int, mb_s_long, void *);
int mb_get_bounds (char *text, double *bounds);
double mb_ddmmss_to_degree (char *text);
int mb_takeoff_to_rollpitch(int verbose,
		double theta, double phi,
		double *alpha, double *beta,
		int *error);
int mb_rollpitch_to_takeoff(int verbose,
		double alpha, double beta,
		double *theta, double *phi,
		int *error);
int mb_xyz_to_takeoff(int verbose,
		double x, double y, double z,
		double *theta, double *phi,
		int *error);
int mb_double_compare(void *a, void *b);
int mb_int_compare(void *a, void *b);
int mb_edit_compare(void *a, void *b);
void hilbert(int n, double delta[], double kappa[]);
void hilbert2(int n, double data[]);

/* end conditional include */
#endif
