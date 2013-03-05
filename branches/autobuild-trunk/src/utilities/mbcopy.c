/* Added HAVE_CONFIG_H for autogen files */
#ifdef HAVE_CONFIG_H
#  include <mbsystem_config.h>
#endif


/*--------------------------------------------------------------------
 *    The MB-system:	mbcopy.c	2/4/93
 *    $Id: mbcopy.c 1934 2012-02-22 07:51:16Z caress $
 *
 *    Copyright (c) 1993-2012 by
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
 * MBcopy copies an input swath sonar data file to an output
 * swath sonar data file with the specified conversions.  Options include
 * windowing in time and space and ping averaging.  The input and
 * output data formats may differ, though not all possible combinations
 * make sense.  The default input and output streams are stdin 
 * and stdout.
 *
 * Author:	D. W. Caress
 * Date:	February 4, 1993
 *
 * $Log: mbcopy.c,v $
 * Revision 5.28  2009/03/02 18:54:40  caress
 * Fixed pixel size problems with mbmosaic, resurrected program mbfilter, and also updated copyright dates in several source files.
 *
 * Revision 5.27  2008/12/05 17:32:52  caress
 * Check-in mods 5 December 2008 including contributions from Gordon Keith.
 *
 * Revision 5.26  2008/07/10 18:16:33  caress
 * Proceeding towards 5.1.1beta20.
 *
 * Revision 5.24  2007/11/16 17:53:02  caress
 * Fixes applied.
 *
 * Revision 5.23  2007/10/31 18:41:42  caress
 * Fixed handling of null sidescan values.
 *
 * Revision 5.22  2007/10/17 20:32:26  caress
 * Release 5.1.1beta11
 * Added Gordon Keith mods to merge bathy from third file.
 *
 * Revision 5.21  2007/05/21 16:15:11  caress
 * Fixed translation to and from XSE format.
 *
 * Revision 5.20  2006/09/11 18:55:53  caress
 * Changes during Western Flyer and Thomas Thompson cruises, August-September
 * 2006.
 *
 * Revision 5.19  2006/03/06 21:47:02  caress
 * Implemented changes suggested by Bob Courtney of the Geological Survey of Canada to support translating Reson data to GSF.
 *
 * Revision 5.18  2006/01/18 15:17:00  caress
 * Added stdlib.h include.
 *
 * Revision 5.17  2005/11/05 01:07:54  caress
 * Programs changed to register arrays through mb_register_array() rather than allocating the memory directly with mb_realloc() or mb_malloc().
 *
 * Revision 5.16  2005/03/25 04:42:59  caress
 * Standardized the string lengths used for filenames and comment data.
 *
 * Revision 5.15  2004/09/16 01:00:01  caress
 * Fixed copyright.
 *
 * Revision 5.14  2003/12/10 02:18:04  caress
 * Fixed bug in which pings could not be found inbounds when a timegap error occurred.
 *
 * Revision 5.13  2003/11/24 22:56:20  caress
 * Added inbounds check so that ancillary data records are only output when the last survey record was within the specified or default time and space bounds. This should allow time and space windowing for data formats containing large numbers of ancillary records.
 *
 * Revision 5.12  2003/04/17 21:17:10  caress
 * Release 5.0.beta30
 *
 * Revision 5.11  2002/07/20 20:56:55  caress
 * Release 5.0.beta20
 *
 * Revision 5.10  2002/05/29 23:43:09  caress
 * Release 5.0.beta18
 *
 * Revision 5.9  2002/05/02 04:01:37  caress
 * Release 5.0.beta17
 *
 * Revision 5.8  2002/04/06 02:53:45  caress
 * Release 5.0.beta16
 *
 * Revision 5.7  2001/08/25 00:58:08  caress
 * Fixed problem with simrad to simrad2 function.
 *
 * Revision 5.6  2001/07/20  00:34:38  caress
 * Release 5.0.beta03
 *
 * Revision 5.5  2001/06/30  17:42:04  caress
 * Release 5.0.beta02
 *
 * Revision 5.4  2001/06/29  22:50:23  caress
 * Atlas Hydrosweep DS2 raw data and SURF data formats.
 *
 * Revision 5.3  2001/06/08  21:45:46  caress
 * Version 5.0.beta01
 *
 * Revision 5.2  2001/06/03  07:07:34  caress
 * Release 5.0.beta01.
 *
 * Revision 5.1  2001/03/22 21:14:16  caress
 * Trying to make release 5.0.beta0.
 *
 * Revision 5.0  2000/12/01  22:57:08  caress
 * First cut at Version 5.0.
 *
 * Revision 4.15  2000/10/11  01:06:15  caress
 * Convert to ANSI C
 *
 * Revision 4.14  2000/09/30  07:06:28  caress
 * Snapshot for Dale.
 *
 * Revision 4.13  1999/08/16  23:13:42  caress
 * Fixed pointer casting bug in elac data copying.
 *
 * Revision 4.12  1999/08/08  04:17:40  caress
 * Added ability to copy between old and new Elac formats.
 *
 * Revision 4.11  1999/04/21 05:44:42  caress
 * Fixed error printing.
 *
 * Revision 4.10  1998/10/05  19:19:24  caress
 * MB-System version 4.6beta
 *
 * Revision 4.9  1997/04/21  17:19:14  caress
 * MB-System 4.5 Beta Release.
 *
 * Revision 4.9  1997/04/17  15:14:38  caress
 * MB-System 4.5 Beta Release
 *
 * Revision 4.8  1996/04/22  13:23:05  caress
 * Now have DTR and MIN/MAX defines in mb_define.h
 *
 * Revision 4.7  1996/01/26  21:25:58  caress
 * Version 4.3 distribution
 *
 * Revision 4.6  1995/05/12  17:12:32  caress
 * Made exit status values consistent with Unix convention.
 * 0: ok  nonzero: error
 *
 * Revision 4.5  1995/03/06  19:37:59  caress
 * Changed include strings.h to string.h for POSIX compliance.
 *
 * Revision 4.4  1995/03/02  13:49:21  caress
 * Fixed bug related to error messages.
 *
 * Revision 4.3  1994/10/21  13:02:31  caress
 * Release V4.0
 *
 * Revision 4.2  1994/07/29  19:02:56  caress
 * Changes associated with supporting byte swapped Lynx OS and
 * using unix second time base.
 *
 * Revision 4.1  1994/03/12  01:44:37  caress
 * Added declarations of ctime and/or getenv for compatability
 * with SGI compilers.
 *
 * Revision 4.1  1994/03/12  01:44:37  caress
 * Added declarations of ctime and/or getenv for compatability
 * with SGI compilers.
 *
 * Revision 4.0  1994/03/06  00:13:22  caress
 * First cut at version 4.0
 *
 * Revision 4.1  1994/03/05  22:49:18  caress
 * Fixed significant bug - output arrays were allocated to
 * size of input arrays.  Also added zeroing of beam/pixel
 * values not set in copying from one system to another.
 *
 * Revision 4.0  1994/03/01  18:59:27  caress
 * First cut at new version. Any changes are associated with
 * support of three data types (beam bathymetry, beam amplitude,
 * and sidescan) instead of two (bathymetry and backscatter).
 *
 * Revision 3.1  1993/06/14  17:53:29  caress
 * Fixed stripcomments option so it does what the man page says.
 *
 * Revision 3.0  1993/05/04  22:25:09  dale
 * Initial version.
 *
 */

/* standard include files */
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

/* mbio include files */
#include "mb_status.h"
#include "mb_format.h"
#include "mb_define.h"
#include "mb_io.h"
#include "mbsys_xse.h"
#include "mbsys_elacmk2.h"
#include "mbsys_simrad.h"
#include "mbsys_simrad2.h"
#include "mbsys_ldeoih.h"
#include "mbsys_gsf.h"
#include "mbsys_hsds.h"
#include "mbsys_reson8k.h"

/* defines for special copying routines */
#define	MBCOPY_PARTIAL			0
#define	MBCOPY_FULL			1
#define	MBCOPY_ELACMK2_TO_XSE		2
#define	MBCOPY_XSE_TO_ELACMK2		3
#define	MBCOPY_SIMRAD_TO_SIMRAD2	4
#define	MBCOPY_ANY_TO_MBLDEOIH		5
#define	MBCOPY_RESON8K_TO_GSF		6

/* function prototypes */
int setup_transfer_rules(int verbose, int ibeams, int obeams,
		int *istart, int *iend, int *offset, int *error);
int mbcopy_elacmk2_to_xse(int verbose, 
		struct mbsys_elacmk2_struct *istore, 
		struct mbsys_xse_struct *ostore, 
		int *error);
int mbcopy_xse_to_elacmk2(int verbose, 
		struct mbsys_xse_struct *istore, 
		struct mbsys_elacmk2_struct *ostore, 
		int *error);
int mbcopy_simrad_to_simrad2(int verbose, 
		struct mbsys_simrad_struct *istore, 
		struct mbsys_simrad2_struct *ostore, 
		int *error);
int mbcopy_simrad_time_convert(int verbose, 
		int year, int month, 
		int day, int hour, 
		int minute, int second, 
		int centisecond, 
		int *date, int *msec, 
		int *error);
int mbcopy_any_to_mbldeoih(int verbose, 
		int kind, int *time_i, double time_d, 
		double navlon, double navlat, double speed, double heading, 
		double draft, double altitude, 
		double roll, double pitch, double heave, 
		double	beamwidth_xtrack, double beamwidth_ltrack, 
		int nbath, int namp, int nss,
		char *beamflag, double *bath, double *amp, 
		double *bathacrosstrack, double *bathalongtrack,
		double *ss, double *ssacrosstrack, double *ssalongtrack,
		char *comment, 
		void *ombio_ptr, void *ostore_ptr, 
		int *error);
int mbcopy_reson8k_to_gsf(int verbose, 
		void *imbio_ptr, 
		void *ombio_ptr,
		int *error);

static char rcs_id[] = "$Id: mbcopy.c 1934 2012-02-22 07:51:16Z caress $";

/*--------------------------------------------------------------------*/

int main (int argc, char **argv)
{
	/* id variables */
	char program_name[] = "MBcopy";
	char help_message[] =  "MBcopy copies an input swath sonar data file to an output \nswath sonar data file with the specified conversions.  Options include \nwindowing in time and space and ping averaging.  The input and \noutput data formats may differ, though not all possible combinations \nmake sense.  The default input and output streams are stdin and stdout.";
	char usage_message[] = "mbcopy [-Byr/mo/da/hr/mn/sc -Ccommentfile -D -Eyr/mo/da/hr/mn/sc \n\t-Fiformat/oformat/mformat -H  -Iinfile -Llonflip -Mmergefile -N -Ooutfile \n\t-Ppings -Qsleep_factor -Rw/e/s/n -Sspeed -V]";

	/* parsing variables */
	extern char *optarg;
	int	errflg = 0;
	int	c;
	int	help = 0;
	int	flag = 0;

	/* MBIO status variables */
	int	status;
	int	verbose = 0;
	int	error = MB_ERROR_NO_ERROR;
	char	*message;

	/* MBIO read control parameters */
	int	iformat = 0;
	int	pings;
	int	lonflip;
	double	bounds[4];
	int	btime_i[7];
	int	etime_i[7];
	double	btime_d;
	double	etime_d;
	double	speedmin;
	double	timegap;
	int	fbtversion;
	char	ifile[MB_PATH_MAXLINE];
	int	ibeams_bath;
	int	ibeams_amp;
	int	ipixels_ss;
	void	*imbio_ptr = NULL;

	/* MBIO write control parameters */
	int	oformat = 0;
	char	ofile[MB_PATH_MAXLINE];
	int	obeams_bath;
	int	obeams_amp;
	int	opixels_ss;
	void	*ombio_ptr = NULL;

	/* MBIO merge control parameters */
	int	merge = MB_NO;
	int	mformat = 0;
	char	mfile[MB_PATH_MAXLINE];
	int	mbeams_bath;
	int	mbeams_amp;
	int	mpixels_ss;
	void	*mmbio_ptr = NULL;

	/* MBIO read and write values */
	struct mb_io_struct *omb_io_ptr;
	struct mb_io_struct *imb_io_ptr;
	struct mb_io_struct *mmb_io_ptr;
	void	*istore_ptr;
	void	*ostore_ptr;
	int	kind;
	int	time_i[7];
	double	time_d;
	double	navlon;
	double	navlat;
	double	speed;
	double	heading;
	double	distance;
	double	altitude;
	double	sonardepth;
	char	*ibeamflag = NULL;
	double	*ibath = NULL;
	double	*ibathacrosstrack = NULL;
	double	*ibathalongtrack = NULL;
	double	*iamp = NULL;
	double	*iss = NULL;
	double	*issacrosstrack = NULL;
	double	*issalongtrack = NULL;
	char	*obeamflag = NULL;
	double	*obath = NULL;
	double	*obathacrosstrack = NULL;
	double	*obathalongtrack = NULL;
	double	*oamp = NULL;
	double	*oss = NULL;
	double	*ossacrosstrack = NULL;
	double	*ossalongtrack = NULL;
	double	draft;
	double	roll;
	double	pitch;
	double	heave;
	int	mstatus;
	int	merror = MB_ERROR_NO_ERROR;
	int	mkind;
	int	mpings;
	int	mtime_i[7];
	double	mtime_d;
	double	mnavlon;
	double	mnavlat;
	double	mspeed;
	double	mheading;
	double	mdistance;
	double	maltitude;
	double	msonardepth;

	char	mcomment[MB_COMMENT_MAXLINE];
	int	mnbath, mnamp, mnss;
	char	*mbeamflag = NULL;
	double	*mbath = NULL;
	double	*mbathacrosstrack = NULL;
	double	*mbathalongtrack = NULL;
	double	*mamp = NULL;
	double	*mss = NULL;
	double	*mssacrosstrack = NULL;
	double	*mssalongtrack = NULL;
	int	idata = 0;
	int	icomment = 0;
	int	odata = 0;
	int	ocomment = 0;
	int	nbath, namp, nss;
	int	istart_bath, iend_bath, offset_bath;
	int	istart_amp, iend_amp, offset_amp;
	int	istart_ss, iend_ss, offset_ss;
	char	comment[MB_COMMENT_MAXLINE];
	int	insertcomments = MB_NO;
	int	bathonly = MB_NO;
	char	commentfile[MB_PATH_MAXLINE];
	int	stripcomments = MB_NO;
	int	copymode = MBCOPY_PARTIAL;
	int	use_sleep = MB_NO;
	int	inbounds = MB_YES;
	
	/* sleep variable */
	double	sleep_factor = 1.0;
	double	time_d_last;
	unsigned int	sleep_time;

	/* time, user, host variables */
	time_t	right_now;
	char	date[25], user[128], *user_ptr, host[128];

	FILE	*fp;
	char	*result;
	int	format;
	double	seconds;
	int	i, j;

	char	*ctime();
	char	*getenv();

	/* get current default values */
	status = mb_defaults(verbose,&format,&pings,&lonflip,bounds,
		btime_i,etime_i,&speedmin,&timegap);
	status = mb_fbtversion(verbose, &fbtversion);

	/* set default input and output */
	iformat = 0;
	oformat = 0;
	mformat = 0;
	strcpy (commentfile, "\0");
	strcpy (ifile, "stdin");
	strcpy (ofile, "stdout");

	/* process argument list */
	while ((c = getopt(argc, argv, "B:b:C:c:DdE:e:F:f:HhI:i:L:l:M:m:NnO:o:P:p:Q:q:R:r:S:s:T:t:Vv")) != -1)
	  switch (c) 
		{
		case 'B':
		case 'b':
			sscanf (optarg,"%d/%d/%d/%d/%d/%lf",
				&btime_i[0],&btime_i[1],&btime_i[2],
				&btime_i[3],&btime_i[4],&seconds);				
			btime_i[5] = (int)floor(seconds);
			btime_i[6] = 1000000 * (seconds - btime_i[5]);
			flag++;
			break;
		case 'C':
		case 'c':
			sscanf (optarg,"%s", commentfile);
			insertcomments = MB_YES;
			flag++;
			break;
		case 'D':
		case 'd':
			bathonly = MB_YES;
			flag++;
			break;
		case 'E':
		case 'e':
			sscanf (optarg,"%d/%d/%d/%d/%d/%lf",
				&etime_i[0],&etime_i[1],&etime_i[2],
				&etime_i[3],&etime_i[4],&seconds);
			etime_i[5] = (int)floor(seconds);
			etime_i[6] = 1000000 * (seconds - btime_i[5]);
			flag++;
			break;
		case 'F':
		case 'f':
			i = sscanf (optarg,"%d/%d/%d", &iformat,&oformat,&mformat);
			if (i == 1)
				oformat = iformat;
			flag++;
			break;
		case 'H':
		case 'h':
			help++;
			break;
		case 'I':
		case 'i':
			sscanf (optarg,"%s", ifile);
			flag++;
			break;
		case 'L':
		case 'l':
			sscanf (optarg,"%d", &lonflip);
			flag++;
			break;
		case 'M':
		case 'm':
			i = sscanf (optarg,"%s", mfile);
			if (i == 1)
				merge = MB_YES;
			flag++;
			break;
		case 'N':
		case 'n':
			stripcomments = MB_YES;
			break;
		case 'O':
		case 'o':
			sscanf (optarg,"%s", ofile);
			flag++;
			break;
		case 'P':
		case 'p':
			sscanf (optarg,"%d", &pings);
			flag++;
			break;
		case 'Q':
		case 'q':
			sscanf (optarg,"%lf", &sleep_factor);
			use_sleep = MB_YES;
			flag++;
			break;
		case 'R':
		case 'r':
			mb_get_bounds(optarg, bounds);
			flag++;
			break;
		case 'S':
		case 's':
			sscanf (optarg,"%lf", &speedmin);
			flag++;
			break;
		case 'T':
		case 't':
			sscanf (optarg,"%lf", &timegap);
			flag++;
			break;
		case 'V':
		case 'v':
			verbose++;
			break;
		case '?':
			errflg++;
		}

	/* if error flagged then print it and exit */
	if (errflg)
		{
		fprintf(stderr,"usage: %s\n", usage_message);
		fprintf(stderr,"\nProgram <%s> Terminated\n",
			program_name);
		error = MB_ERROR_BAD_USAGE;
		exit(error);
		}

	/* print starting message */
	if (verbose == 1 || help)
		{
		fprintf(stderr,"\nProgram %s\n",program_name);
		fprintf(stderr,"Version %s\n",rcs_id);
		fprintf(stderr,"MB-system Version %s\n",MB_VERSION);
		}

	/* print starting debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  Program <%s>\n",program_name);
		fprintf(stderr,"dbg2  Version %s\n",rcs_id);
		fprintf(stderr,"dbg2  MB-system Version %s\n",MB_VERSION);
		fprintf(stderr,"dbg2  Control Parameters:\n");
		fprintf(stderr,"dbg2       verbose:        %d\n",verbose);
		fprintf(stderr,"dbg2       help:           %d\n",help);
		fprintf(stderr,"dbg2       pings:          %d\n",pings);
		fprintf(stderr,"dbg2       lonflip:        %d\n",lonflip);
		fprintf(stderr,"dbg2       bounds[0]:      %f\n",bounds[0]);
		fprintf(stderr,"dbg2       bounds[1]:      %f\n",bounds[1]);
		fprintf(stderr,"dbg2       bounds[2]:      %f\n",bounds[2]);
		fprintf(stderr,"dbg2       bounds[3]:      %f\n",bounds[3]);
		fprintf(stderr,"dbg2       btime_i[0]:     %d\n",btime_i[0]);
		fprintf(stderr,"dbg2       btime_i[1]:     %d\n",btime_i[1]);
		fprintf(stderr,"dbg2       btime_i[2]:     %d\n",btime_i[2]);
		fprintf(stderr,"dbg2       btime_i[3]:     %d\n",btime_i[3]);
		fprintf(stderr,"dbg2       btime_i[4]:     %d\n",btime_i[4]);
		fprintf(stderr,"dbg2       btime_i[5]:     %d\n",btime_i[5]);
		fprintf(stderr,"dbg2       btime_i[6]:     %d\n",btime_i[6]);
		fprintf(stderr,"dbg2       etime_i[0]:     %d\n",etime_i[0]);
		fprintf(stderr,"dbg2       etime_i[1]:     %d\n",etime_i[1]);
		fprintf(stderr,"dbg2       etime_i[2]:     %d\n",etime_i[2]);
		fprintf(stderr,"dbg2       etime_i[3]:     %d\n",etime_i[3]);
		fprintf(stderr,"dbg2       etime_i[4]:     %d\n",etime_i[4]);
		fprintf(stderr,"dbg2       etime_i[5]:     %d\n",etime_i[5]);
		fprintf(stderr,"dbg2       etime_i[6]:     %d\n",etime_i[6]);
		fprintf(stderr,"dbg2       speedmin:       %f\n",speedmin);
		fprintf(stderr,"dbg2       timegap:        %f\n",timegap);
		fprintf(stderr,"dbg2       input format:   %d\n",iformat);
		fprintf(stderr,"dbg2       output format:  %d\n",oformat);
		fprintf(stderr,"dbg2       merge format:   %d\n",mformat);
		fprintf(stderr,"dbg2       input file:     %s\n",ifile);
		fprintf(stderr,"dbg2       output file:    %s\n",ofile);
		fprintf(stderr,"dbg2       merge file:     %s\n",mfile);
		fprintf(stderr,"dbg2       insert comments:%d\n",insertcomments);
		fprintf(stderr,"dbg2       comment file:   %s\n",commentfile);
		fprintf(stderr,"dbg2       strip comments: %d\n",stripcomments);
		fprintf(stderr,"dbg2       bath only:      %d\n",bathonly);
		fprintf(stderr,"dbg2       use sleep:      %d\n",use_sleep);
		fprintf(stderr,"dbg2       sleep factor:   %f\n",sleep_factor);
		fprintf(stderr,"dbg2       fbtversion:     %d\n",fbtversion);
		}

	/* if help desired then print it and exit */
	if (help)
		{
		fprintf(stderr,"\n%s\n",help_message);
		fprintf(stderr,"\nusage: %s\n", usage_message);
		exit(error);
		}

	/* get format if required */
	if (format == 0)
		mb_get_format(verbose,ifile,NULL,&format,&error);

	/* settle the input/output formats */
	if (iformat <= 0 && oformat <= 0)
		{
		iformat = format;
		oformat = format;
		}
	else if (iformat > 0 && oformat <= 0)
		oformat = iformat;

	if (merge == MB_YES && mformat <=0)
		mb_get_format(verbose,mfile,NULL,&mformat,&error);
		

	/* obtain format array locations - format ids will 
		be aliased to current ids if old format ids given */
	if ((status = mb_format(verbose,&iformat,&error)) 
		!= MB_SUCCESS)
		{
		mb_error(verbose,error,&message);
		fprintf(stderr,"\nMBIO Error returned from function <mb_format> regarding input format %d:\n%s\n",iformat,message);
		fprintf(stderr,"\nProgram <%s> Terminated\n",
			program_name);
		exit(error);
		}
	if ((status = mb_format(verbose,&oformat,&error)) 
		!= MB_SUCCESS)
		{
		mb_error(verbose,error,&message);
		fprintf(stderr,"\nMBIO Error returned from function <mb_format> regarding output format %d:\n%s\n",oformat,message);
		fprintf(stderr,"\nProgram <%s> Terminated\n",
			program_name);
		exit(error);
		}
	if (merge == MB_YES && (status = mb_format(verbose,&mformat,&error)) 
		!= MB_SUCCESS)
		{
		mb_error(verbose,error,&message);
		fprintf(stderr,"\nMBIO Error returned from function <mb_format> regarding merge format %d:\n%s\n",mformat,message);
		fprintf(stderr,"\nProgram <%s> Terminated\n",
			program_name);
		exit(error);
		}

	/* initialize reading the input swath sonar file */
	if ((status = mb_read_init(
		verbose,ifile,iformat,pings,lonflip,bounds,
		btime_i,etime_i,speedmin,timegap,
		&imbio_ptr,&btime_d,&etime_d,
		&ibeams_bath,&ibeams_amp,&ipixels_ss,&error)) != MB_SUCCESS)
		{
		mb_error(verbose,error,&message);
		fprintf(stderr,"\nMBIO Error returned from function <mb_read_init>:\n%s\n",message);
		fprintf(stderr,"\nMultibeam File <%s> not initialized for reading\n",ifile);
		fprintf(stderr,"\nProgram <%s> Terminated\n",
			program_name);
		exit(error);
		}
	imb_io_ptr = (struct mb_io_struct *) imbio_ptr; 

	/* initialize reading the merge swath sonar file */
	if (merge == MB_YES && (status = mb_read_init(
		verbose,mfile,mformat,pings,lonflip,bounds,
		btime_i,etime_i,speedmin,timegap,
		&mmbio_ptr,&btime_d,&etime_d,
		&mbeams_bath,&mbeams_amp,&mpixels_ss,&error)) != MB_SUCCESS)
		{
		mb_error(verbose,error,&message);
		fprintf(stderr,"\nMBIO Error returned from function <mb_read_init>:\n%s\n",message);
		fprintf(stderr,"\nMultibeam File <%s> not initialized for reading\n",mfile);
		fprintf(stderr,"\nProgram <%s> Terminated\n",
			program_name);
		exit(error);
		}
	mmb_io_ptr = (struct mb_io_struct *) mmbio_ptr; 

	/* initialize writing the output swath sonar file */
	if ((status = mb_write_init(
		verbose,ofile,oformat,&ombio_ptr,
		&obeams_bath,&obeams_amp,&opixels_ss,&error)) != MB_SUCCESS)
		{
		mb_error(verbose,error,&message);
		fprintf(stderr,"\nMBIO Error returned from function <mb_write_init>:\n%s\n",message);
		fprintf(stderr,"\nMultibeam File <%s> not initialized for writing\n",ofile);
		fprintf(stderr,"\nProgram <%s> Terminated\n",
			program_name);
		exit(error);
		}
	omb_io_ptr = (struct mb_io_struct *) ombio_ptr;
	
	/* bathonly mode works only if output format is mbldeoih */
	if (bathonly == MB_YES && oformat != MBF_MBLDEOIH)
		{
		bathonly = MB_NO;
		if (verbose > 0)
		    {
		    fprintf(stderr,"\nThe -D option (strip amplitude and sidescan) is only valid for output format %d\n",MBF_MBLDEOIH);
		    fprintf(stderr,"Program %s is ignoring the -D argument\n",program_name);
		    }
		}
		
	/* if bathonly mode for mbldeoih format, assume we are making an fbt file
		- set the format to use - this allows user to set use of old format
		in .mbio_defaults file - purpose is to keep compatibility with 
		Fledermaus */
	if (bathonly == MB_YES && oformat == MBF_MBLDEOIH)
		{
		omb_io_ptr->save1 = fbtversion;
		}
		
	/* determine if full or partial copies will be made */
	if (pings == 1 
		&& imb_io_ptr->system != MB_SYS_NONE 
		&& imb_io_ptr->system == omb_io_ptr->system)
		copymode = MBCOPY_FULL;
	else if (pings == 1 
		&& imb_io_ptr->system == MB_SYS_ELACMK2 
		&& omb_io_ptr->system == MB_SYS_XSE)
		copymode = MBCOPY_ELACMK2_TO_XSE;
	else if (pings == 1 
		&& imb_io_ptr->system == MB_SYS_XSE 
		&& omb_io_ptr->system == MB_SYS_ELACMK2)
		copymode = MBCOPY_XSE_TO_ELACMK2;
	else if (pings == 1 
		&& imb_io_ptr->system == MB_SYS_SIMRAD 
		&& omb_io_ptr->format == MBF_EM300MBA)
		copymode = MBCOPY_SIMRAD_TO_SIMRAD2;
	else if (pings == 1 
		&& omb_io_ptr->format == MBF_MBLDEOIH)
		copymode = MBCOPY_ANY_TO_MBLDEOIH;
	else if (pings == 1 
		&& imb_io_ptr->format == MBF_XTFR8101 
		&& omb_io_ptr->format ==  MBF_GSFGENMB )
		copymode = MBCOPY_RESON8K_TO_GSF;		
	else
		copymode = MBCOPY_PARTIAL;
		
	/* quit if an unsupported copy to GSF is requested */
	if (omb_io_ptr->format == MBF_GSFGENMB && copymode == MBCOPY_PARTIAL)
		{
		fprintf(stderr,"Requested copy from format %d to GSF format %d is unsupported\n",
			imb_io_ptr->format, omb_io_ptr->format);
		fprintf(stderr,"Please consider writing the necessary translation code for mbcopy.c \n");
		fprintf(stderr,"\tand contributing it to the MB-System community\n");
		exit(error);
		}

	/* print debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  Copy mode set in program <%s>\n",
			program_name);
		fprintf(stderr,"dbg2       pings:         %d\n",pings);
		fprintf(stderr,"dbg2       iformat:       %d\n",iformat);
		fprintf(stderr,"dbg2       oformat:       %d\n",oformat);
		fprintf(stderr,"dbg2       isystem:       %d\n",
			imb_io_ptr->system);
		fprintf(stderr,"dbg2       osystem:       %d\n",
			omb_io_ptr->system);
		fprintf(stderr,"dbg2       copymode:      %d\n",copymode);
		}

	/* allocate memory for data arrays */
	if (error == MB_ERROR_NO_ERROR)
		status = mb_register_array(verbose, imbio_ptr, MB_MEM_TYPE_BATHYMETRY,
						sizeof(char), (void **)&ibeamflag, &error);
	if (error == MB_ERROR_NO_ERROR)
		status = mb_register_array(verbose, imbio_ptr, MB_MEM_TYPE_BATHYMETRY,
						sizeof(double), (void **)&ibath, &error);
	if (error == MB_ERROR_NO_ERROR)
		status = mb_register_array(verbose, imbio_ptr, MB_MEM_TYPE_AMPLITUDE,
						sizeof(double), (void **)&iamp, &error);
	if (error == MB_ERROR_NO_ERROR)
		status = mb_register_array(verbose, imbio_ptr, MB_MEM_TYPE_BATHYMETRY,
						sizeof(double), (void **)&ibathacrosstrack, &error);
	if (error == MB_ERROR_NO_ERROR)
		status = mb_register_array(verbose, imbio_ptr, MB_MEM_TYPE_BATHYMETRY,
						sizeof(double), (void **)&ibathalongtrack, &error);
	if (error == MB_ERROR_NO_ERROR)
		status = mb_register_array(verbose, imbio_ptr, MB_MEM_TYPE_SIDESCAN, 
						sizeof(double), (void **)&iss, &error);
	if (error == MB_ERROR_NO_ERROR)
		status = mb_register_array(verbose, imbio_ptr, MB_MEM_TYPE_SIDESCAN, 
						sizeof(double), (void **)&issacrosstrack, &error);
	if (error == MB_ERROR_NO_ERROR)
		status = mb_register_array(verbose, imbio_ptr, MB_MEM_TYPE_SIDESCAN, 
						sizeof(double), (void **)&issalongtrack, &error);
	if (error == MB_ERROR_NO_ERROR)
		status = mb_register_array(verbose, ombio_ptr, MB_MEM_TYPE_BATHYMETRY,
						sizeof(char), (void **)&obeamflag, &error);
	if (error == MB_ERROR_NO_ERROR)
		status = mb_register_array(verbose, ombio_ptr, MB_MEM_TYPE_BATHYMETRY,
						sizeof(double), (void **)&obath, &error);
	if (error == MB_ERROR_NO_ERROR)
		status = mb_register_array(verbose, ombio_ptr, MB_MEM_TYPE_AMPLITUDE,
						sizeof(double), (void **)&oamp, &error);
	if (error == MB_ERROR_NO_ERROR)
		status = mb_register_array(verbose, ombio_ptr, MB_MEM_TYPE_BATHYMETRY,
						sizeof(double), (void **)&obathacrosstrack, &error);
	if (error == MB_ERROR_NO_ERROR)
		status = mb_register_array(verbose, ombio_ptr, MB_MEM_TYPE_BATHYMETRY,
						sizeof(double), (void **)&obathalongtrack, &error);
	if (error == MB_ERROR_NO_ERROR)
		status = mb_register_array(verbose, ombio_ptr, MB_MEM_TYPE_SIDESCAN, 
						sizeof(double), (void **)&oss, &error);
	if (error == MB_ERROR_NO_ERROR)
		status = mb_register_array(verbose, ombio_ptr, MB_MEM_TYPE_SIDESCAN, 
						sizeof(double), (void **)&ossacrosstrack, &error);
	if (error == MB_ERROR_NO_ERROR)
		status = mb_register_array(verbose, ombio_ptr, MB_MEM_TYPE_SIDESCAN, 
						sizeof(double), (void **)&ossalongtrack, &error);

	if (MB_YES == merge) {
	if (error == MB_ERROR_NO_ERROR)
		status = mb_register_array(verbose, mmbio_ptr, MB_MEM_TYPE_BATHYMETRY,
						sizeof(char), (void **)&mbeamflag, &error);
	if (error == MB_ERROR_NO_ERROR)
		status = mb_register_array(verbose, mmbio_ptr, MB_MEM_TYPE_BATHYMETRY,
						sizeof(double), (void **)&mbath, &error);
	if (error == MB_ERROR_NO_ERROR)
		status = mb_register_array(verbose, mmbio_ptr, MB_MEM_TYPE_AMPLITUDE,
						sizeof(double), (void **)&mamp, &error);
	if (error == MB_ERROR_NO_ERROR)
		status = mb_register_array(verbose, mmbio_ptr, MB_MEM_TYPE_BATHYMETRY,
						sizeof(double), (void **)&mbathacrosstrack, &error);
	if (error == MB_ERROR_NO_ERROR)
		status = mb_register_array(verbose, mmbio_ptr, MB_MEM_TYPE_BATHYMETRY,
						sizeof(double), (void **)&mbathalongtrack, &error);
	if (error == MB_ERROR_NO_ERROR)
		status = mb_register_array(verbose, mmbio_ptr, MB_MEM_TYPE_SIDESCAN, 
						sizeof(double), (void **)&mss, &error);
	if (error == MB_ERROR_NO_ERROR)
		status = mb_register_array(verbose, mmbio_ptr, MB_MEM_TYPE_SIDESCAN, 
						sizeof(double), (void **)&mssacrosstrack, &error);
	if (error == MB_ERROR_NO_ERROR)
		status = mb_register_array(verbose, mmbio_ptr, MB_MEM_TYPE_SIDESCAN, 
						sizeof(double), (void **)&mssalongtrack, &error);
	}

	/* if error initializing memory then quit */
	if (error != MB_ERROR_NO_ERROR)
		{
		mb_error(verbose,error,&message);
		fprintf(stderr,"\nMBIO Error allocating data arrays:\n%s\n",message);
		fprintf(stderr,"\nProgram <%s> Terminated\n",
			program_name);
		exit(error);
		}

	/* set up transfer rules */
	if (omb_io_ptr->variable_beams == MB_YES
		&& obeams_bath != ibeams_bath)
		obeams_bath = ibeams_bath;
	if (omb_io_ptr->variable_beams == MB_YES
		&& obeams_amp != ibeams_amp)
		obeams_amp = ibeams_amp;
	if (omb_io_ptr->variable_beams == MB_YES
		&& opixels_ss != ipixels_ss)
		opixels_ss = ipixels_ss;
	setup_transfer_rules(verbose,ibeams_bath,obeams_bath,
		&istart_bath,&iend_bath,&offset_bath,&error);
	setup_transfer_rules(verbose,ibeams_amp,obeams_amp,
		&istart_amp,&iend_amp,&offset_amp,&error);
	setup_transfer_rules(verbose,ipixels_ss,opixels_ss,
		&istart_ss,&iend_ss,&offset_ss,&error);

	/* insert comments from file into output */
	if (insertcomments == MB_YES)
		{
		/* open file */
		if ((fp = fopen(commentfile, "r")) == NULL) 
			{
			error = MB_ERROR_OPEN_FAIL;
			fprintf(stderr,"\nUnable to Open Comment File <%s> for reading\n",commentfile);
			fprintf(stderr,"\nProgram <%s> Terminated\n",
				program_name);
			exit(error);
			}

		/* read and output comment lines */
		strncpy(comment,"\0",256);
		while ((result = fgets(comment,256,fp)) == comment)
			{
			kind = MB_DATA_COMMENT;
			comment[(int)strlen(comment)-1] = '\0';
			status = mb_put_comment(verbose,ombio_ptr,
				comment,&error);
			if (error == MB_ERROR_NO_ERROR) ocomment++;
			}

		/* close the file */
		fclose(fp);
		}

	/* write comments to beginning of output file */
	if (stripcomments == MB_NO)
		{
		kind = MB_DATA_COMMENT;
		strncpy(comment,"\0",256);
		sprintf(comment,"These data copied by program %s version %s",
			program_name,rcs_id);
		status = mb_put_comment(verbose,ombio_ptr,
				comment,&error);
		if (error == MB_ERROR_NO_ERROR) ocomment++;
		strncpy(comment,"\0",256);
		sprintf(comment,"MB-system Version %s",MB_VERSION);
		status = mb_put_comment(verbose,ombio_ptr,
				comment,&error);
		if (error == MB_ERROR_NO_ERROR) ocomment++;
		strncpy(date,"\0",25);
		right_now = time((time_t *)0);
		strncpy(date,ctime(&right_now),24);
		if ((user_ptr = getenv("USER")) == NULL)
			user_ptr = getenv("LOGNAME");
		if (user_ptr != NULL)
			strcpy(user,user_ptr);
		else
			strcpy(user, "unknown");
		gethostname(host,128);
		strncpy(comment,"\0",256);
		sprintf(comment,"Run by user <%s> on cpu <%s> at <%s>",
			user,host,date);
		status = mb_put_comment(verbose,ombio_ptr,
				comment,&error);
		if (error == MB_ERROR_NO_ERROR) ocomment++;
		strncpy(comment,"\0",256);
		sprintf(comment,"Control Parameters:");
		status = mb_put_comment(verbose,ombio_ptr,
				comment,&error);
		if (error == MB_ERROR_NO_ERROR) ocomment++;
		strncpy(comment,"\0",256);
		sprintf(comment,"  Input file:         %s",ifile);
		status = mb_put_comment(verbose,ombio_ptr,
				comment,&error);
		if (error == MB_ERROR_NO_ERROR) ocomment++;
		strncpy(comment,"\0",256);
		sprintf(comment,"  Input MBIO format:  %d",iformat);
		status = mb_put_comment(verbose,ombio_ptr,
				comment,&error);
		if (error == MB_ERROR_NO_ERROR) ocomment++;
		if (merge == MB_YES)
			{
			strncpy(comment,"\0",256);
			sprintf(comment,"  Merge file:         %s",mfile);
			status = mb_put_comment(verbose,ombio_ptr,
					comment,&error);
			if (error == MB_ERROR_NO_ERROR) ocomment++;
			strncpy(comment,"\0",256);
			sprintf(comment,"  Merge MBIO format:  %d",mformat);
			status = mb_put_comment(verbose,ombio_ptr,
					comment,&error);
			if (error == MB_ERROR_NO_ERROR) ocomment++;
			}
		strncpy(comment,"\0",256);
		sprintf(comment,"  Output file:        %s",ofile);
		status = mb_put_comment(verbose,ombio_ptr,
				comment,&error);
		if (error == MB_ERROR_NO_ERROR) ocomment++;
		strncpy(comment,"\0",256);
		sprintf(comment,"  Output MBIO format: %d",oformat);
		status = mb_put_comment(verbose,ombio_ptr,
				comment,&error);
		if (error == MB_ERROR_NO_ERROR) ocomment++;
		strncpy(comment,"\0",256);
		sprintf(comment,"  Ping averaging:     %d",pings);
		status = mb_put_comment(verbose,ombio_ptr,
				comment,&error);
		if (error == MB_ERROR_NO_ERROR) ocomment++;
		strncpy(comment,"\0",256);
		sprintf(comment,"  Longitude flip:     %d",lonflip);
		status = mb_put_comment(verbose,ombio_ptr,
				comment,&error);
		if (error == MB_ERROR_NO_ERROR) ocomment++;
		strncpy(comment,"\0",256);
		sprintf(comment,"  Longitude bounds:   %f %f",
			bounds[0],bounds[1]);
		status = mb_put_comment(verbose,ombio_ptr,
				comment,&error);
		if (error == MB_ERROR_NO_ERROR) ocomment++;
		strncpy(comment,"\0",256);
		sprintf(comment,"  Latitude bounds:    %f %f",
			bounds[2],bounds[3]);
		status = mb_put_comment(verbose,ombio_ptr,
				comment,&error);
		if (error == MB_ERROR_NO_ERROR) ocomment++;
		strncpy(comment,"\0",256);
		sprintf(comment,"  Begin time:         %d %d %d %d %d %d %d",
			btime_i[0],btime_i[1],btime_i[2],
			btime_i[3],btime_i[4],btime_i[5],btime_i[6]);
		status = mb_put_comment(verbose,ombio_ptr,
				comment,&error);
		if (error == MB_ERROR_NO_ERROR) ocomment++;
		strncpy(comment,"\0",256);
		sprintf(comment,"  End time:           %d %d %d %d %d %d %d",
			etime_i[0],etime_i[1],etime_i[2],
			etime_i[3],etime_i[4],etime_i[5],etime_i[6]);
		status = mb_put_comment(verbose,ombio_ptr,
				comment,&error);
		if (error == MB_ERROR_NO_ERROR) ocomment++;
		strncpy(comment,"\0",256);
		sprintf(comment,"  Minimum speed:      %f",speedmin);
		status = mb_put_comment(verbose,ombio_ptr,
				comment,&error);
		if (error == MB_ERROR_NO_ERROR) ocomment++;
		strncpy(comment,"\0",256);
		sprintf(comment,"  Time gap:           %f",timegap);
		status = mb_put_comment(verbose,ombio_ptr,
				comment,&error);
		if (error == MB_ERROR_NO_ERROR) ocomment++;
		strncpy(comment,"\0",256);
		sprintf(comment," ");
		status = mb_put_comment(verbose,ombio_ptr,
				comment,&error);
		if (error == MB_ERROR_NO_ERROR) ocomment++;
		}

	/* start expecting data to be in time and space bounds */
	inbounds = MB_YES;

	/* read and write */
	while (error <= MB_ERROR_NO_ERROR)
		{
		
		/* read some data */
		error = MB_ERROR_NO_ERROR;
		status = MB_SUCCESS;
		if (copymode != MBCOPY_PARTIAL)
			{
			status = mb_get_all(verbose,imbio_ptr,&istore_ptr,&kind,
				time_i,&time_d,
				&navlon,&navlat,
				&speed,&heading,
				&distance,&altitude,&sonardepth,
				&nbath,&namp,&nss,
				ibeamflag,ibath,iamp,
				ibathacrosstrack,ibathalongtrack,
				iss,issacrosstrack,issalongtrack,
				comment,&error);
			}
		else
			{
			status = mb_get(verbose,imbio_ptr,&kind,&pings,
				time_i,&time_d,
				&navlon,&navlat,
				&speed,&heading,
				&distance,&altitude,&sonardepth,
				&nbath,&namp,&nss,
				ibeamflag,ibath,iamp,
				ibathacrosstrack,ibathalongtrack,
				iss,issacrosstrack,issalongtrack,
				comment,&error);
			}

		/* increment counter */
		if (error <= MB_ERROR_NO_ERROR 
			&& kind == MB_DATA_DATA)
			idata = idata + pings;
		else if (error <= MB_ERROR_NO_ERROR 
			&& kind == MB_DATA_COMMENT)
			icomment++;
			
		/* time gaps do not matter to mbcopy */
		if (error == MB_ERROR_TIME_GAP)
			{
			status = MB_SUCCESS;
			error = MB_ERROR_NO_ERROR;
			}
			
		/* check for survey data in or out of bounds */
		if (kind == MB_DATA_DATA)
			{
			if (error == MB_ERROR_NO_ERROR)
				inbounds = MB_YES;
			else if (error == MB_ERROR_OUT_BOUNDS
				||error == MB_ERROR_OUT_TIME)
				inbounds = MB_NO;
			}


		if(merge == MB_YES
			&& kind == MB_DATA_DATA
			&& error == MB_ERROR_NO_ERROR
			&& inbounds == MB_YES)
			{
			  while (merror <= MB_ERROR_NO_ERROR
				 && (mkind != MB_DATA_DATA
				 	|| time_d - .001 > mtime_d))
			    {
			      /* find merge record */
			      
			      mstatus = mb_get(verbose,mmbio_ptr,&mkind,&mpings,
					       mtime_i,&mtime_d,
					       &mnavlon,&mnavlat,
					       &mspeed,&mheading,
					       &mdistance,&maltitude,&msonardepth,
					       &mnbath,&mnamp,&mnss,
					       mbeamflag,mbath,mamp,
					       mbathacrosstrack,mbathalongtrack,
					       mss,mssacrosstrack,mssalongtrack,
					       mcomment,&merror);
			      
			    }
			  
			  if (time_d + .001 < mtime_d ||
			      merror > 0)
			    {
			      inbounds = MB_NO;
			    }
			}

		/* check numbers of input and output beams */
		if (copymode == MBCOPY_PARTIAL
			&& kind == MB_DATA_DATA
			&& error == MB_ERROR_NO_ERROR
			&& nbath != ibeams_bath)
			{
			ibeams_bath = nbath;
			if (omb_io_ptr->variable_beams == MB_YES)
				obeams_bath = ibeams_bath;
			setup_transfer_rules(verbose,ibeams_bath,obeams_bath,
				&istart_bath,&iend_bath,&offset_bath,&error);
			}
		if (copymode == MBCOPY_PARTIAL
			&& kind == MB_DATA_DATA
			&& error == MB_ERROR_NO_ERROR
			&& namp != ibeams_amp)
			{
			ibeams_amp = namp;
			if (omb_io_ptr->variable_beams == MB_YES)
				obeams_amp = ibeams_amp;
			setup_transfer_rules(verbose,ibeams_amp,obeams_amp,
				&istart_amp,&iend_amp,&offset_amp,&error);
			}
		if (copymode == MBCOPY_PARTIAL
			&& kind == MB_DATA_DATA
			&& error == MB_ERROR_NO_ERROR
			&& nss != ipixels_ss)
			{
			ipixels_ss = nss;
			if (omb_io_ptr->variable_beams == MB_YES)
				opixels_ss = ipixels_ss;
			setup_transfer_rules(verbose,ipixels_ss,opixels_ss,
				&istart_ss,&iend_ss,&offset_ss,&error);
			}

		/* output error messages */
		if (verbose >= 1 && error == MB_ERROR_COMMENT)
			{
			if (icomment == 1)
				fprintf(stderr,"\nComments:\n");
			fprintf(stderr,"%s\n",comment);
			}
		else if (verbose >= 1 && error < MB_ERROR_NO_ERROR
			&& error >= MB_ERROR_OTHER)
			{
			mb_error(verbose,error,&message);
			fprintf(stderr,"\nNonfatal MBIO Error:\n%s\n",message);
			fprintf(stderr,"Input Record: %d\n",idata);
			fprintf(stderr,"Time: %d %d %d %d %d %d %d\n",
				time_i[0],time_i[1],time_i[2],
				time_i[3],time_i[4],time_i[5],time_i[6]);
			}
		else if (verbose >= 1 && error < MB_ERROR_NO_ERROR)
			{
			mb_error(verbose,error,&message);
			fprintf(stderr,"\nNonfatal MBIO Error:\n%s\n",message);
			fprintf(stderr,"Number of good records so far: %d\n",idata);
			}
		else if (verbose >= 1 && error != MB_ERROR_NO_ERROR 
			&& error != MB_ERROR_EOF)
			{
			mb_error(verbose,error,&message);
			fprintf(stderr,"\nFatal MBIO Error:\n%s\n",message);
			fprintf(stderr,"Last Good Time: %d %d %d %d %d %d %d\n",
				time_i[0],time_i[1],time_i[2],
				time_i[3],time_i[4],time_i[5],time_i[6]);
			}
		
		/* do sleep if required */
		if (use_sleep == MB_YES
			&& kind == MB_DATA_DATA
			&& error <= MB_ERROR_NO_ERROR
			&& idata == 1)
			{
			time_d_last = time_d;
			}
		else if (use_sleep == MB_YES
			&& kind == MB_DATA_DATA
			&& error <= MB_ERROR_NO_ERROR
			&& idata > 1)
			{
			sleep_time = (unsigned int) 
				(sleep_factor * (time_d - time_d_last));
			sleep(sleep_time);
			time_d_last = time_d;
			}

		/* process some data */
		if (copymode == MBCOPY_PARTIAL
			&& kind == MB_DATA_DATA
			&& error == MB_ERROR_NO_ERROR)
			{
			/* zero bathymetry */
			for (j=0;j<offset_bath;j++)
				{
				obeamflag[j] = MB_FLAG_NULL;
				obath[j] = 0.0;
				obathacrosstrack[j] = 0.0;
				obathalongtrack[j] = 0.0;
				}

			/* do bathymetry */
			if (merge == MB_YES)
				{
				/* merge data */
				for (i=istart_bath;i<iend_bath;i++)
					{
					j = i + offset_bath;
					obeamflag[j] = mbeamflag[i];
					obath[j] = mbath[i];
					obathacrosstrack[j] = mbathacrosstrack[i];
					obathalongtrack[j] = mbathalongtrack[i];
					}
				}
			else
				{
				for (i=istart_bath;i<iend_bath;i++)
					{
					j = i + offset_bath;
					obeamflag[j] = ibeamflag[i];
					obath[j] = ibath[i];
					obathacrosstrack[j] = ibathacrosstrack[i];
					obathalongtrack[j] = ibathalongtrack[i];
					}
				}
			for (j=iend_bath+offset_bath;j<obeams_bath;j++)
				{
				obeamflag[j] = MB_FLAG_NULL;
				obath[j] = 0.0;
				obathacrosstrack[j] = 0.0;
				obathalongtrack[j] = 0.0;
				}

			/* do amplitudes */
			for (j=0;j<offset_amp;j++)
				{
				oamp[j] = 0.0;
				}
			for (i=istart_amp;i<iend_amp;i++)
				{
				j = i + offset_amp;
				oamp[j] = iamp[i];
				}
			for (j=iend_amp+offset_amp;j<obeams_amp;j++)
				{
				oamp[j] = 0.0;
				}

			/* do sidescan */
			for (j=0;j<offset_ss;j++)
				{
				oss[j] = 0.0;
				ossacrosstrack[j] = 0.0;
				ossalongtrack[j] = 0.0;
				}
			for (i=istart_ss;i<iend_ss;i++)
				{
				j = i + offset_ss;
				oss[j] = iss[i];
				ossacrosstrack[j] = issacrosstrack[i];
				ossalongtrack[j] = issalongtrack[i];
				}
			for (j=iend_ss+offset_ss;j<opixels_ss;j++)
				{
				oss[j] = 0.0;
				ossacrosstrack[j] = 0.0;
				ossalongtrack[j] = 0.0;
				}
			}
			
		/* handle special full translation cases */
		if (copymode == MBCOPY_FULL
			&& error == MB_ERROR_NO_ERROR)
			{
			ostore_ptr = istore_ptr;
			}
		else if (copymode == MBCOPY_ELACMK2_TO_XSE
			&& error == MB_ERROR_NO_ERROR)
			{
			ostore_ptr = omb_io_ptr->store_data;
			status = mbcopy_elacmk2_to_xse(verbose, 
				    istore_ptr, ostore_ptr, &error);
			}
		else if (copymode == MBCOPY_XSE_TO_ELACMK2
			&& error == MB_ERROR_NO_ERROR)
			{
			ostore_ptr = omb_io_ptr->store_data;
			status = mbcopy_xse_to_elacmk2(verbose, 
				    istore_ptr, ostore_ptr, &error);
			}
		else if (copymode == MBCOPY_SIMRAD_TO_SIMRAD2
			&& error == MB_ERROR_NO_ERROR)
			{
			ostore_ptr = omb_io_ptr->store_data;
			status = mbcopy_simrad_to_simrad2(verbose, 
				    istore_ptr, ostore_ptr, &error);
			}
		else if (copymode == MBCOPY_RESON8K_TO_GSF
			&& error == MB_ERROR_NO_ERROR)
			{

			ostore_ptr = omb_io_ptr->store_data;
			status = mbcopy_reson8k_to_gsf(verbose, 
				    imbio_ptr, ombio_ptr, &error);
			}
		else if (copymode == MBCOPY_ANY_TO_MBLDEOIH
			&& error == MB_ERROR_NO_ERROR)
			{
			if (kind == MB_DATA_DATA)
				mb_extract_nav(verbose, imbio_ptr, istore_ptr, 
					&kind, time_i, &time_d, 
					&navlon, &navlat, &speed, &heading, &draft, 
					&roll, &pitch, &heave, 
					&error);
			ostore_ptr = omb_io_ptr->store_data;
			if (kind == MB_DATA_DATA
				|| kind == MB_DATA_COMMENT)
				{
				/* strip amplitude and sidescan if requested */
				if (bathonly == MB_YES)
				    {
				    namp = 0;
				    nss = 0;
				    }
				    
				/* copy the data to mbldeoih */
				if (merge == MB_YES)
					{
					status = mbcopy_any_to_mbldeoih(verbose, 
						kind, time_i, time_d, 
						navlon, navlat, speed, heading, 
						draft, altitude, roll, pitch, heave, 
						imb_io_ptr->beamwidth_xtrack, 
						imb_io_ptr->beamwidth_ltrack, 
						nbath,namp,nss,
						mbeamflag,mbath,iamp,mbathacrosstrack,
						mbathalongtrack,
						iss,issacrosstrack,issalongtrack,
						comment, 
						ombio_ptr, ostore_ptr, &error);
					}
				else
					{
					  status = mbcopy_any_to_mbldeoih(verbose, 
						kind, time_i, time_d, 
						navlon, navlat, speed, heading, 
						draft, altitude, roll, pitch, heave, 
						imb_io_ptr->beamwidth_xtrack, 
						imb_io_ptr->beamwidth_ltrack, 
						nbath,namp,nss,
						ibeamflag,ibath,iamp,ibathacrosstrack,
						ibathalongtrack,
						iss,issacrosstrack,issalongtrack,
						comment, 
						ombio_ptr, ostore_ptr, &error);
					}
				}
			else 
				error = MB_ERROR_OTHER;
			}
		else if (copymode == MBCOPY_PARTIAL
			&& error == MB_ERROR_NO_ERROR)
			{
			istore_ptr = imb_io_ptr->store_data;
			ostore_ptr = omb_io_ptr->store_data;
			if (pings == 1 && kind == MB_DATA_DATA)
				{
				mb_extract_nav(verbose, imbio_ptr, istore_ptr, 
						&kind, time_i, &time_d, 
						&navlon, &navlat, &speed, &heading, &draft, 
						&roll, &pitch, &heave, 
						&error);
				mb_insert_nav(verbose, ombio_ptr, ostore_ptr, 
						time_i, time_d, 
						navlon, navlat, speed, heading, draft, 
						roll, pitch, heave, 
						&error);
				}
			status = mb_insert(verbose, ombio_ptr, ostore_ptr,
						kind, time_i, time_d, 
						navlon, navlat, speed, heading, 
						obeams_bath,obeams_amp,opixels_ss,
						obeamflag,obath,oamp,obathacrosstrack,
						obathalongtrack,
						oss,ossacrosstrack,ossalongtrack,
						comment, &error);
			}

		if (merge == MB_YES
		    && kind == MB_DATA_DATA
		    && error == MB_ERROR_NO_ERROR)
		  {
		    switch(copymode) {
		    case MBCOPY_PARTIAL:
		    case MBCOPY_ANY_TO_MBLDEOIH:
		      /* Already looked after */
		      break;
		    case MBCOPY_FULL :
		    case MBCOPY_SIMRAD_TO_SIMRAD2:
		    case MBCOPY_ELACMK2_TO_XSE:
		    case MBCOPY_XSE_TO_ELACMK2:
		    case MBCOPY_RESON8K_TO_GSF:
		      status = mb_insert(verbose, ombio_ptr, ostore_ptr,
						kind, time_i, time_d, 
						navlon, navlat, speed, heading, 
						mbeams_bath,ibeams_amp,ipixels_ss,
						mbeamflag,mbath,iamp,mbathacrosstrack,
						mbathalongtrack,
						iss,issacrosstrack,issalongtrack,
						comment, &error);
		      break;
		    }
		  }

		/* write some data */
		if ((error == MB_ERROR_NO_ERROR 
				&& kind != MB_DATA_COMMENT
				&& inbounds == MB_YES)
			|| (kind == MB_DATA_COMMENT && stripcomments == MB_NO))
			{
			status = mb_put_all(verbose,ombio_ptr,
					ostore_ptr,MB_NO,kind,
					time_i,time_d,
					navlon,navlat,speed,heading,
					obeams_bath,obeams_amp,opixels_ss,
					obeamflag,obath,oamp,obathacrosstrack,
					obathalongtrack,
					oss,ossacrosstrack,ossalongtrack,
					comment,&error);
			if (status == MB_SUCCESS)
				{
				if (kind == MB_DATA_DATA)
					odata++;
				else if (kind == MB_DATA_COMMENT)
					ocomment++;
				}
			else
				{
				mb_error(verbose,error,&message);
				if (copymode != MBCOPY_PARTIAL)
				    fprintf(stderr,"\nMBIO Error returned from function <mb_put_all>:\n%s\n",message);
				else
				    fprintf(stderr,"\nMBIO Error returned from function <mb_put>:\n%s\n",message);
				fprintf(stderr,"\nMultibeam Data Not Written To File <%s>\n",ofile);
				fprintf(stderr,"Output Record: %d\n",odata+1);
				fprintf(stderr,"Time: %d %d %d %d %d %d %d\n",
					time_i[0],time_i[1],time_i[2],
					time_i[3],time_i[4],time_i[5],
					time_i[6]);
				fprintf(stderr,"\nProgram <%s> Terminated\n",
					program_name);
				exit(error);
				}
			}
		}

	/* close the files */
	status = mb_close(verbose,&imbio_ptr,&error);
	status = mb_close(verbose,&ombio_ptr,&error);

	/* check memory */
	if (verbose >= 4)
		status = mb_memory_list(verbose,&error);

	/* give the statistics */
	if (verbose >= 1)
		{
		fprintf(stderr,"\n%d input data records\n",idata);
		fprintf(stderr,"%d input comment records\n",icomment);
		fprintf(stderr,"%d output data records\n",odata);
		fprintf(stderr,"%d output comment records\n",ocomment);
		}

	/* end it all */
	exit(error);
}
/*--------------------------------------------------------------------*/
int setup_transfer_rules(int verbose, int ibeams, int obeams,
		int *istart, int *iend, int *offset, int *error)
{
	char	*function_name = "setup_transfer_rules";
	int	status = MB_SUCCESS;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBcopy function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       ibeams:     %d\n",ibeams);
		fprintf(stderr,"dbg2       obeams:     %d\n",obeams);
		}

	/* set up transfer rules */
	if (ibeams == obeams)
		{
		*istart = 0;
		*iend = ibeams;
		*offset = 0;
		}
	else if (ibeams < obeams)
		{
		*istart = 0;
		*iend = ibeams;
		*offset = obeams/2 - ibeams/2;
		}
	else if (ibeams > obeams)
		{
		*istart = ibeams/2 - obeams/2;
		*iend = *istart + obeams;
		*offset = -*istart;
		}

	/* assume success */
	*error = MB_ERROR_NO_ERROR;
	status = MB_SUCCESS;

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBcopy function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       error:      %d\n",*error);
		fprintf(stderr,"dbg2       istart:     %d\n",*istart);
		fprintf(stderr,"dbg2       iend:       %d\n",*iend);
		fprintf(stderr,"dbg2       offset:     %d\n",*offset);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:     %d\n",status);
		}

	/* return status */
	return(status);
}
/*--------------------------------------------------------------------*/
int mbcopy_elacmk2_to_xse(int verbose, 
	struct mbsys_elacmk2_struct *istore, 
	struct mbsys_xse_struct *ostore, 
	int *error)
{
	char	*function_name = "mbcopy_elacmk2_to_xse";
	int	status = MB_SUCCESS;
	double	time_d;
	int	time_i[7];
	int	i, j;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBcopy function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       istore:     %lu\n",(size_t)istore);
		fprintf(stderr,"dbg2       ostore:     %lu\n",(size_t)ostore);
		}

	/* copy the data  */
	if (istore != NULL && ostore != NULL 
		&& (void *) istore != (void *) ostore)
		{
		/* type of data record */
		ostore->kind = istore->kind;  /* Survey, nav, Comment */
		
		/* parameter (ship frames) */
		ostore->par_source = 0;		/* sensor id */
		mb_fix_y2k(verbose, istore->par_year, &time_i[0]);
		time_i[1] = istore->par_month;
		time_i[2] = istore->par_day;
		time_i[3] = istore->par_hour;
		time_i[4] = istore->par_minute;
		time_i[5] = istore->par_second;
		time_i[6] = 10000*istore->par_hundredth_sec 
			+ 100*istore->par_thousandth_sec;
		mb_get_time(verbose,time_i,&time_d);
		ostore->par_sec = ((unsigned int) time_d) + MBSYS_XSE_TIME_OFFSET;		/* sec since 1/1/1901 00:00 */
		ostore->par_usec = (time_d - ((int) time_d)) * 1000000;		/* microseconds */
		ostore->par_roll_bias = DTR * 0.01 * istore->roll_offset;		/* radians */
		ostore->par_pitch_bias = DTR * 0.01 * istore->pitch_offset;		/* radians */
		ostore->par_heading_bias = DTR * 0.01 * istore->heading_offset;	/* radians */
		ostore->par_time_delay = 0.01 * istore->time_delay;		/* nav time lag, seconds */
		ostore->par_trans_x_port = 0.01 * istore->transducer_port_x;	/* port transducer x position, meters */
		ostore->par_trans_y_port = 0.01 * istore->transducer_port_y;	/* port transducer y position, meters */
		ostore->par_trans_z_port = 0.01 * istore->transducer_port_depth;	/* port transducer z position, meters */
		ostore->par_trans_x_stbd = 0.01 * istore->transducer_starboard_x;	/* starboard transducer x position, meters */
		ostore->par_trans_y_stbd = 0.01 * istore->transducer_starboard_y;	/* starboard transducer y position, meters */
		ostore->par_trans_z_stbd = 0.01 * istore->transducer_starboard_depth;	/* starboard transducer z position, meters */
		ostore->par_trans_err_port = 0.01 * istore->transducer_port_error;	/* port transducer rotation in roll direction, radians */
		ostore->par_trans_err_stbd = 0.01 * istore->transducer_starboard_error;	/* starboard transducer rotation in roll direction, radians */
		ostore->par_nav_x = 0.01 * istore->antenna_x;		/* navigation antenna x position, meters */
		ostore->par_nav_y = 0.01 * istore->antenna_y;		/* navigation antenna y position, meters */
		ostore->par_nav_z = 0.01 * istore->antenna_height;		/* navigation antenna z position, meters */
		ostore->par_hrp_x = 0.01 * istore->vru_x;		/* motion sensor x position, meters */
		ostore->par_hrp_y = 0.01 * istore->vru_y;		/* motion sensor y position, meters */
		ostore->par_hrp_z = 0.01 * istore->vru_height;		/* motion sensor z position, meters */
	
		/* svp (sound velocity frames) */
		ostore->svp_source = 0;		/* sensor id */
		mb_fix_y2k(verbose, istore->svp_year, &time_i[0]);
		time_i[1] = istore->svp_month;
		time_i[2] = istore->svp_day;
		time_i[3] = istore->svp_hour;
		time_i[4] = istore->svp_minute;
		time_i[5] = istore->svp_second;
		time_i[6] = 10000*istore->svp_hundredth_sec 
			+ 100*istore->svp_thousandth_sec;
		mb_get_time(verbose,time_i,&time_d);
		ostore->svp_sec = ((unsigned int) time_d) + MBSYS_XSE_TIME_OFFSET;		/* sec since 1/1/1901 00:00 */
		ostore->svp_usec = (time_d - ((int) time_d)) * 1000000;		/* microseconds */
		ostore->svp_nsvp = istore->svp_num;		/* number of depth values */
		ostore->svp_nctd = 0;		/* number of ctd values */
		ostore->svp_ssv = istore->sound_vel;				/* m/s */
		for (i=0;i<ostore->svp_nsvp;i++)
		    {
		    ostore->svp_depth[i] = 0.1 * istore->svp_depth[i];		/* m */
		    ostore->svp_velocity[i] = 0.1 * istore->svp_vel[i];	/* m/s */
		    ostore->svp_conductivity[i] = 0.0;	/* mmho/cm */
		    ostore->svp_salinity[i] = 0.0;	/* o/oo */
		    ostore->svp_temperature[i] = 0.0;	/* degree celcius */
		    ostore->svp_pressure[i] = 0.0;	/* bar */
		    }
	
		/* position (navigation frames) */
		ostore->nav_source = 0;		/* sensor id */
		mb_fix_y2k(verbose, istore->pos_year, &time_i[0]);
		time_i[1] = istore->pos_month;
		time_i[2] = istore->pos_day;
		time_i[3] = istore->pos_hour;
		time_i[4] = istore->pos_minute;
		time_i[5] = istore->pos_second;
		time_i[6] = 10000*istore->pos_hundredth_sec 
			+ 100*istore->pos_thousandth_sec;
		mb_get_time(verbose,time_i,&time_d);
		ostore->nav_sec = ((unsigned int) time_d) + MBSYS_XSE_TIME_OFFSET;		/* sec since 1/1/1901 00:00 */
		ostore->nav_usec = (time_d - ((int) time_d)) * 1000000;		/* microseconds */
		ostore->nav_quality = 0;
		ostore->nav_status = 0;
		ostore->nav_description_len = 0;
		for (i=0;i<MBSYS_XSE_DESCRIPTION_LENGTH;i++)
		    ostore->nav_description[i] = 0;
		ostore->nav_x = DTR * 0.00000009 * istore->pos_longitude;			/* eastings (m) or 
						    longitude (radians) */
		ostore->nav_y = DTR * 0.00000009 * istore->pos_latitude;			/* northings (m) or 
						    latitude (radians) */
		ostore->nav_z = 0.0;			/* height (m) or 
						    ellipsoidal height (m) */
		ostore->nav_speed_ground = 0.0;	/* m/s */
		ostore->nav_course_ground = DTR * 0.01 * istore->heading;	/* radians */
		ostore->nav_speed_water = 0.0;	/* m/s */
		ostore->nav_course_water = 0.0;	/* radians */
		
		/* survey depth (multibeam frames) */
		if (ostore->kind == MB_DATA_DATA)
		    {
		    ostore->mul_frame = MB_YES;	/* boolean flag - multibeam frame read */
		    ostore->mul_group_beam = MB_NO;	/* boolean flag - beam group read */
		    ostore->mul_group_tt = MB_YES;	/* boolean flag - tt group read */
		    ostore->mul_group_quality = MB_YES;/* boolean flag - quality group read */
		    ostore->mul_group_amp = MB_YES;	/* boolean flag - amp group read */
		    ostore->mul_group_delay = MB_YES;	/* boolean flag - delay group read */
		    ostore->mul_group_lateral = MB_YES;/* boolean flag - lateral group read */
		    ostore->mul_group_along = MB_YES;	/* boolean flag - along group read */
		    ostore->mul_group_depth = MB_YES;	/* boolean flag - depth group read */
		    ostore->mul_group_angle = MB_YES;	/* boolean flag - angle group read */
		    ostore->mul_group_heave = MB_YES;	/* boolean flag - heave group read */
		    ostore->mul_group_roll = MB_YES;	/* boolean flag - roll group read */
		    ostore->mul_group_pitch = MB_YES;	/* boolean flag - pitch group read */
		    ostore->mul_group_gates = MB_NO;	/* boolean flag - gates group read */
		    ostore->mul_group_noise = MB_NO;	/* boolean flag - noise group read */
		    ostore->mul_group_length = MB_NO;	/* boolean flag - length group read */
		    ostore->mul_group_hits = MB_NO;		/* boolean flag - hits group read */
		    ostore->mul_group_heavereceive = MB_NO;	/* boolean flag - heavereceive group read */
		    ostore->mul_group_azimuth = MB_NO;	/* boolean flag - azimuth group read */
		    ostore->mul_group_mbsystemnav = MB_YES;	/* boolean flag - mbsystemnav group read */
		    }
		else
		    {
		    ostore->mul_frame = MB_NO;	/* boolean flag - multibeam frame read */
		    ostore->mul_group_beam = MB_NO;	/* boolean flag - beam group read */
		    ostore->mul_group_tt = MB_NO;	/* boolean flag - tt group read */
		    ostore->mul_group_quality = MB_NO;/* boolean flag - quality group read */
		    ostore->mul_group_amp = MB_NO;	/* boolean flag - amp group read */
		    ostore->mul_group_delay = MB_NO;	/* boolean flag - delay group read */
		    ostore->mul_group_lateral = MB_NO;/* boolean flag - lateral group read */
		    ostore->mul_group_along = MB_NO;	/* boolean flag - along group read */
		    ostore->mul_group_depth = MB_NO;	/* boolean flag - depth group read */
		    ostore->mul_group_angle = MB_NO;	/* boolean flag - angle group read */
		    ostore->mul_group_heave = MB_NO;	/* boolean flag - heave group read */
		    ostore->mul_group_roll = MB_NO;	/* boolean flag - roll group read */
		    ostore->mul_group_pitch = MB_NO;	/* boolean flag - pitch group read */
		    ostore->mul_group_gates = MB_NO;	/* boolean flag - gates group read */
		    ostore->mul_group_noise = MB_NO;	/* boolean flag - noise group read */
		    ostore->mul_group_length = MB_NO;	/* boolean flag - length group read */
		    ostore->mul_group_hits = MB_NO;		/* boolean flag - hits group read */
		    ostore->mul_group_heavereceive = MB_NO;	/* boolean flag - heavereceive group read */
		    ostore->mul_group_azimuth = MB_NO;	/* boolean flag - azimuth group read */
		    ostore->mul_group_mbsystemnav = MB_NO;	/* boolean flag - mbsystemnav group read */
		    }
		ostore->mul_source = 0;		/* sensor id */
		mb_fix_y2k(verbose, istore->pos_year, &time_i[0]);
		time_i[1] = istore->month;
		time_i[2] = istore->day;
		time_i[3] = istore->hour;
		time_i[4] = istore->minute;
		time_i[5] = istore->second;
		time_i[6] = 10000*istore->hundredth_sec 
			+ 100*istore->thousandth_sec;
		mb_get_time(verbose,time_i,&time_d);
		ostore->mul_sec = ((unsigned int) time_d) + MBSYS_XSE_TIME_OFFSET;		/* sec since 1/1/1901 00:00 */
		ostore->mul_usec = (time_d - ((int) time_d)) * 1000000;		/* microseconds */
		ostore->mul_lon = DTR * istore->longitude;		/* longitude (radians) */
		ostore->mul_lat = DTR * istore->latitude;		/* latitude (radians) */
		ostore->mul_heading = DTR * 0.01 * istore->heading;		/* heading (radians) */
		ostore->mul_speed = 0.0;		/* speed (m/s) */
		ostore->mul_ping = istore->ping_num;		/* ping number */
		ostore->mul_frequency = 0.0;	/* transducer frequency (Hz) */
		ostore->mul_pulse = istore->pulse_length;		/* transmit pulse length (sec) */
		ostore->mul_power = istore->source_power;		/* transmit power (dB) */
		ostore->mul_bandwidth = 0.0;	/* receive bandwidth (Hz) */
		ostore->mul_sample = 0.0;		/* receive sample interval (sec) */
		ostore->mul_swath = 0.0;		/* swath width (radians) */
		ostore->mul_num_beams = istore->beams_bath;	/* number of beams */
		for (i=0;i<ostore->mul_num_beams;i++)
		    {
		    j = istore->beams_bath - i - 1;
		    ostore->beams[i].tt = 0.0001 * istore->beams[j].tt;
		    ostore->beams[i].delay = 0.0005 * istore->beams[j].time_offset;
		    ostore->beams[i].lateral = 0.01 * istore->beams[j].bath_acrosstrack;
		    ostore->beams[i].along = 0.01 * istore->beams[j].bath_alongtrack;
		    ostore->beams[i].depth = 0.01 * istore->beams[j].bath;
		    ostore->beams[i].angle = DTR * 0.005 * istore->beams[j].angle;
		    ostore->beams[i].heave = 0.001 * istore->beams[j].heave;
		    ostore->beams[i].roll = DTR * 0.005 * istore->beams[j].roll;
		    ostore->beams[i].pitch = DTR * 0.005 * istore->beams[j].pitch;
		    ostore->beams[i].beam = i + 1;
		    ostore->beams[i].quality = istore->beams[j].quality;
		    ostore->beams[i].amplitude = istore->beams[j].amplitude;		    
		    }
		
		/* survey sidescan (sidescan frames) */
		ostore->sid_frame = MB_NO;		/* boolean flag - sidescan frame read */
		ostore->sid_group_avt = MB_NO;		/* boolean flag - amp vs time group read */
		ostore->sid_group_pvt = MB_NO;		/* boolean flag - phase vs time group read */
		ostore->sid_group_avl = MB_NO;		/* boolean flag - amp vs lateral group read */
		ostore->sid_group_pvl = MB_NO;		/* boolean flag - phase vs lateral group read */
		ostore->sid_group_signal = MB_NO;	/* boolean flag - phase vs lateral group read */
		ostore->sid_group_ping = MB_NO;		/* boolean flag - phase vs lateral group read */
		ostore->sid_group_complex = MB_NO;	/* boolean flag - phase vs lateral group read */
		ostore->sid_group_weighting = MB_NO;	/* boolean flag - phase vs lateral group read */
		ostore->sid_source = 0;		/* sensor id */
		ostore->sid_sec = 0;	/* sec since 1/1/1901 00:00 */
		ostore->sid_usec = 0;	/* microseconds */
		ostore->sid_ping = 0;		/* ping number */
		ostore->sid_frequency = 0.0;		/* transducer frequency (Hz) */
		ostore->sid_pulse = 0.0;		/* transmit pulse length (sec) */
		ostore->sid_power = 0.0;		/* transmit power (dB) */
		ostore->sid_bandwidth = 0.0;		/* receive bandwidth (Hz) */
		ostore->sid_sample = 0.0;		/* receive sample interval (sec) */
		ostore->sid_avt_sampleus = 0;	/* sample interval (usec) */
		ostore->sid_avt_offset = 0;		/* time offset (usec) */
		ostore->sid_avt_num_samples = 0;	/* number of samples */
		for (i=0;i<MBSYS_XSE_MAXPIXELS;i++)
			ostore->sid_avt_amp[i] = 0; /* sidescan amplitude (dB) */
		ostore->sid_pvt_sampleus = 0;	/* sample interval (usec) */
		ostore->sid_pvt_offset = 0;		/* time offset (usec) */
		ostore->sid_pvt_num_samples = 0;	/* number of samples */
		for (i=0;i<MBSYS_XSE_MAXPIXELS;i++)
			ostore->sid_pvt_phase[i] = 0; /* sidescan phase (radians) */
		ostore->sid_avl_binsize = 0;	/* bin size (mm) */
		ostore->sid_avl_offset = 0;		/* lateral offset (mm) */
		ostore->sid_avl_num_samples = 0;	/* number of samples */
		for (i=0;i<MBSYS_XSE_MAXPIXELS;i++)
			ostore->sid_avl_amp[i] = 0; /* sidescan amplitude (dB) */
		ostore->sid_pvl_binsize = 0;	/* bin size (mm) */
		ostore->sid_pvl_offset = 0;		/* lateral offset (mm) */
		ostore->sid_pvl_num_samples = 0;	/* number of samples */
		for (i=0;i<MBSYS_XSE_MAXPIXELS;i++)
			ostore->sid_pvl_phase[i] = 0; /* sidescan phase (radians) */
		ostore->sid_sig_ping = 0;		/* ping number */
		ostore->sid_sig_channel = 0;	/* channel number */
		ostore->sid_sig_offset = 0.0;		/* start offset */
		ostore->sid_sig_sample = 0.0;		/* bin size / sample interval */
		ostore->sid_sig_num_samples = 0;	/* number of samples */
		for (i=0;i<MBSYS_XSE_MAXPIXELS;i++)
			ostore->sid_sig_phase[i] = 0; /* sidescan phase in radians */
		ostore->sid_png_pulse = 0;	/* pulse type (0=constant, 1=linear sweep) */
		ostore->sid_png_startfrequency = 0.0;	/* start frequency (Hz) */
		ostore->sid_png_endfrequency = 0.0;	/* end frequency (Hz) */
		ostore->sid_png_duration = 0.0;	/* pulse duration (msec) */
		ostore->sid_png_mancode = 0;	/* manufacturer code (1=Edgetech, 2=Elac) */
		ostore->sid_png_pulseid = 0;/* pulse identifier */
		for (i=0;i<MBSYS_XSE_DESCRIPTION_LENGTH;i++)
			ostore->sid_png_pulsename[i] = 0;	/* pulse name */
		ostore->sid_cmp_ping = 0;		/* ping number */
		ostore->sid_cmp_channel = 0;	/* channel number */
		ostore->sid_cmp_offset = 0.0;		/* start offset (usec) */
		ostore->sid_cmp_sample = 0.0;		/* bin size / sample interval (usec) */
		ostore->sid_cmp_num_samples = 0;	/* number of samples */
		for (i=0;i<MBSYS_XSE_MAXPIXELS;i++)
			ostore->sid_cmp_real[i] = 0; /* real sidescan signal */
		for (i=0;i<MBSYS_XSE_MAXPIXELS;i++)
			ostore->sid_cmp_imaginary[i] = 0; /* imaginary sidescan signal */
		ostore->sid_wgt_factorleft = 0;		/* weighting factor for block floating 
						point expansion  -- 
						defined as 2^(-N) volts for lsb */
		ostore->sid_wgt_samplesleft = 0;	/* number of left samples */
		ostore->sid_wgt_factorright = 0;		/* weighting factor for block floating 
						point expansion  -- 
						defined as 2^(-N) volts for lsb */
		ostore->sid_wgt_samplesright = 0;	/* number of right samples */
	
		/* comment */
		for (i=0;i<MIN(MBSYS_ELACMK2_COMMENT_LENGTH, MBSYS_XSE_COMMENT_LENGTH);i++)
			ostore->comment[i] = istore->comment[i];
	
		/* unsupported frame */
		ostore->rawsize = 0;
		for (i=0;i<MBSYS_XSE_BUFFER_SIZE;i++)
		    ostore->raw[i] = 0;
		}

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBcopy function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       error:      %d\n",*error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:     %d\n",status);
		}

	/* return status */
	return(status);
}
/*--------------------------------------------------------------------*/
int mbcopy_xse_to_elacmk2(int verbose, 
		struct mbsys_xse_struct *istore, 
		struct mbsys_elacmk2_struct *ostore, 
		int *error)
{
	char	*function_name = "mbcopy_xse_to_elacmk2";
	int	status = MB_SUCCESS;
	double	time_d;
	int	time_i[7];
	int	i, j;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBcopy function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       istore:     %lu\n",(size_t)istore);
		fprintf(stderr,"dbg2       ostore:     %lu\n",(size_t)ostore);
		fprintf(stderr,"dbg2       kind:       %d\n",istore->kind);
		}

	/* copy the data  */
	if (istore != NULL && ostore != NULL 
		&& (void *) istore != (void *) ostore)
		{
		/* type of data record */
		ostore->kind = istore->kind;
		ostore->sonar = MBSYS_ELACMK2_UNKNOWN;
	
		/* parameter telegram */
		time_d = istore->par_sec 
			    - MBSYS_XSE_TIME_OFFSET
			    + 0.000001 * istore->par_usec;
		mb_get_date(verbose,time_d,time_i);
		mb_unfix_y2k(verbose, time_i[0], &ostore->par_year);
		ostore->par_month = time_i[1];
		ostore->par_day = time_i[2];
		ostore->par_hour = time_i[3];
		ostore->par_minute = time_i[4];
		ostore->par_second = time_i[5];
		ostore->par_hundredth_sec = time_i[6]/10000;
		ostore->par_thousandth_sec 
			= (time_i[6] 
			- 10000 * ostore->par_hundredth_sec)/100;
		ostore->roll_offset = RTD * 100 * istore->par_roll_bias;		/* roll offset (degrees) */
		ostore->pitch_offset = RTD * 100 * istore->par_pitch_bias;	/* pitch offset (degrees) */
		ostore->heading_offset = RTD * 100 * istore->par_heading_bias;	/* heading offset (degrees) */
		ostore->time_delay = 100 * istore->par_time_delay;		/* positioning system delay (sec) */
		ostore->transducer_port_height = 0;
		ostore->transducer_starboard_height = 0;
		ostore->transducer_port_depth = 200 * istore->par_trans_z_port;
		ostore->transducer_starboard_depth = 200 * istore->par_trans_z_stbd;
		ostore->transducer_port_x = 200 * istore->par_trans_x_port;
		ostore->transducer_starboard_x = 200 * istore->par_trans_x_port;
		ostore->transducer_port_y = 200 * istore->par_trans_x_port;
		ostore->transducer_starboard_y = 200 * istore->par_trans_x_port;
		ostore->transducer_port_error = 200 * RTD * istore->par_trans_err_port;
		ostore->transducer_starboard_error = 200 * RTD * istore->par_trans_err_stbd;
		ostore->antenna_height = 200 * istore->par_nav_z;
		ostore->antenna_x = 200 * istore->par_nav_x;
		ostore->antenna_y = 200 * istore->par_nav_y;
		ostore->vru_height = 200 * istore->par_hrp_z;
		ostore->vru_x = 200 * istore->par_hrp_x;
		ostore->vru_y =200 * istore->par_hrp_y;
		ostore->line_number = 0;
		ostore->start_or_stop = 0;
		ostore->transducer_serial_number = 0;
		for (i=0;i<MIN(MBSYS_ELACMK2_COMMENT_LENGTH, MBSYS_XSE_COMMENT_LENGTH);i++)
			ostore->comment[i] = istore->comment[i];
	
		/* position (position telegrams) */
		time_d = istore->nav_sec 
			    - MBSYS_XSE_TIME_OFFSET
			    + 0.000001 * istore->nav_usec;
		mb_get_date(verbose,time_d,time_i);
		mb_unfix_y2k(verbose, time_i[0], &ostore->pos_year);
		ostore->pos_month = time_i[1];
		ostore->pos_day = time_i[2];
		ostore->pos_hour = time_i[3];
		ostore->pos_minute = time_i[4];
		ostore->pos_second = time_i[5];
		ostore->pos_hundredth_sec = time_i[6]/10000;
		ostore->pos_thousandth_sec 
			= (time_i[6] 
			- 10000 * ostore->pos_hundredth_sec)/100;
		ostore->pos_latitude = RTD * istore->nav_y / 0.00000009;
		ostore->pos_longitude = RTD * istore->nav_x / 0.00000009;
		ostore->utm_northing = 0;
		ostore->utm_easting = 0;
		ostore->utm_zone_lon = 0;
		ostore->utm_zone = 0;
		ostore->hemisphere = 0;
		ostore->ellipsoid = 0;
		ostore->pos_spare = 0;
		ostore->semi_major_axis = 0;
		ostore->other_quality = 0;
	
		/* sound velocity profile */
		time_d = istore->svp_sec 
			    - MBSYS_XSE_TIME_OFFSET
			    + 0.000001 * istore->svp_usec;
		mb_get_date(verbose,time_d,time_i);
		mb_unfix_y2k(verbose, time_i[0], &ostore->svp_year);
		ostore->svp_month = time_i[1];
		ostore->svp_day = time_i[2];
		ostore->svp_hour = time_i[3];
		ostore->svp_minute = time_i[4];
		ostore->svp_second = time_i[5];
		ostore->svp_hundredth_sec = time_i[6]/10000;
		ostore->svp_thousandth_sec 
			= (time_i[6] 
			- 10000 * ostore->svp_hundredth_sec)/100;
		ostore->svp_num = istore->svp_nsvp;
		for (i=0;i<500;i++)
			{
			ostore->svp_depth[i] = 10 * istore->svp_depth[i]; /* 0.1 meters */
			ostore->svp_vel[i] = 10 * istore->svp_velocity[i];	/* 0.1 meters/sec */
			}
	
		/* depth telegram */
		time_d = istore->mul_sec 
			    - MBSYS_XSE_TIME_OFFSET
			    + 0.000001 * istore->mul_usec;
		mb_get_date(verbose,time_d,time_i);
		mb_unfix_y2k(verbose, time_i[0], &ostore->year);
		ostore->month = time_i[1];
		ostore->day = time_i[2];
		ostore->hour = time_i[3];
		ostore->minute = time_i[4];
		ostore->second = time_i[5];
		ostore->hundredth_sec = time_i[6]/10000;
		ostore->thousandth_sec 
			= (time_i[6] 
			- 10000 * ostore->hundredth_sec)/100;
		ostore->longitude = RTD * istore->mul_lon;
		ostore->latitude = RTD * istore->mul_lat;
		ostore->ping_num = istore->mul_ping;
		ostore->sound_vel = 10 * istore->svp_ssv;
		ostore->heading = 100 * RTD * istore->nav_course_ground;
		ostore->pulse_length = istore->mul_pulse;
		ostore->mode = 0;
		ostore->source_power = istore->mul_power;
		ostore->receiver_gain_stbd = 0;
		ostore->receiver_gain_port = 0;
		ostore->reserved = 0;
		ostore->beams_bath = 0;
		for (i=0;i<MBSYS_ELACMK2_MAXBEAMS;i++)
			{
			ostore->beams[i].bath = 0;
			ostore->beams[i].bath_acrosstrack = 0;
			ostore->beams[i].bath_alongtrack = 0;
			ostore->beams[i].tt = 0;
			ostore->beams[i].quality = 0;
			ostore->beams[i].amplitude = 0;
			ostore->beams[i].time_offset = 0;
			ostore->beams[i].heave = 0;
			ostore->beams[i].roll = 0;
			ostore->beams[i].pitch = 0;
			ostore->beams[i].angle = 0;
			}
		ostore->beams_bath = istore->beams[istore->mul_num_beams-1].beam;
		for (i=0;i<istore->mul_num_beams;i++)
			{
			j = ostore->beams_bath - istore->beams[i].beam;
			ostore->beams[j].bath = 100 * istore->beams[i].depth;
			ostore->beams[j].bath_acrosstrack = -100 * istore->beams[i].lateral;
			ostore->beams[j].bath_alongtrack = 100 * istore->beams[i].along;
			ostore->beams[j].tt = 10000 * istore->beams[i].tt;
			ostore->beams[j].quality = istore->beams[i].quality;
			ostore->beams[j].amplitude = istore->beams[i].amplitude;
			ostore->beams[j].time_offset = 10000 * istore->beams[i].delay;
			ostore->beams[j].heave = 1000 * istore->beams[i].heave;
			ostore->beams[j].roll = 200 * RTD * istore->beams[i].roll;
			ostore->beams[j].pitch = 200 * RTD * istore->beams[i].pitch;
			ostore->beams[j].angle = 200 * istore->beams[i].angle;
			}
		}

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBcopy function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       error:      %d\n",*error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:     %d\n",status);
		}

	/* return status */
	return(status);
}
/*--------------------------------------------------------------------*/
int mbcopy_simrad_to_simrad2(int verbose, 
		struct mbsys_simrad_struct *istore, 
		struct mbsys_simrad2_struct *ostore, 
		int *error)
{
	char	*function_name = "mbcopy_simrad_to_simrad2";
	int	status = MB_SUCCESS;
	struct mbsys_simrad_survey_struct *iping;
	struct mbsys_simrad2_ping_struct *oping;
	double	*angles_simrad;
	double	bath_offset;
	double	alpha, beta, theta, phi;
	int	istep = 0;
	int	interleave = 0;
	int	i;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBcopy function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       istore:     %lu\n",(size_t)istore);
		fprintf(stderr,"dbg2       ostore:     %lu\n",(size_t)ostore);
		fprintf(stderr,"dbg2       kind:       %d\n",istore->kind);
		}

	/* copy the data  */
	if (istore != NULL && ostore != NULL 
		&& (void *) istore != (void *) ostore)
		{
		/* type of data record */
		ostore->kind = istore->kind;
		ostore->type = EM2_NONE;
  		if (istore->kind == MB_DATA_DATA)
		    ostore->type = EM2_BATH;
		else if (istore->kind == MB_DATA_COMMENT)
		    ostore->type = EM2_START;
		else if (istore->kind == MB_DATA_START)
		    ostore->type = EM2_START;
		else if (istore->kind == MB_DATA_STOP)
		    ostore->type = EM2_STOP2;
		else if (istore->kind == MB_DATA_NAV)
		    ostore->type = EM2_POS;
		else if (istore->kind == MB_DATA_VELOCITY_PROFILE)
		    ostore->type = EM2_SVP;
		if (istore->sonar == MBSYS_SIMRAD_EM12S)
		    ostore->sonar = MBSYS_SIMRAD2_EM12S;
		else if (istore->sonar == MBSYS_SIMRAD_EM12D)
		    ostore->sonar = MBSYS_SIMRAD2_EM12D;
		else if (istore->sonar == MBSYS_SIMRAD_EM1000)
		    ostore->sonar = MBSYS_SIMRAD2_EM1000;
		else if (istore->sonar == MBSYS_SIMRAD_EM121)
		    ostore->sonar = MBSYS_SIMRAD2_EM121;

		/* time stamp */
		mbcopy_simrad_time_convert(verbose, 
		    istore->year, istore->month, 
		    istore->day, istore->hour, 
		    istore->minute, istore->second, 
		    istore->centisecond, 
		    &ostore->date, &ostore->msec, 
		    error);
	
		/* installation parameter values */
		ostore->par_date = 0;	/* installation parameter date = year*10000 + month*100 + day
					    Feb 26, 1995 = 19950226 */
		ostore->par_msec = 0;	/* installation parameter time since midnight in msec
					    08:12:51.234 = 29570234 */
		mbcopy_simrad_time_convert(verbose, 
		    istore->par_year, istore->par_month, 
		    istore->par_day, istore->par_hour, 
		    istore->par_minute, istore->par_second, 
		    istore->par_centisecond, 
		    &ostore->par_date, &ostore->par_msec, 
		    error);
		ostore->par_line_num = istore->survey_line; /* survey line number */
		ostore->par_serial_1 = 0;/* system 1 serial number */
		ostore->par_serial_2 = 0;/* system 2 serial number */
		ostore->par_wlz = 0.0;	/* water line vertical location (m) */
		ostore->par_smh = 0;	/* system main head serial number */
		if (istore->sonar == MBSYS_SIMRAD_EM100)
		    {
		    ostore->par_s1z = istore->em100_td;	/* transducer 1 vertical location (m) */
		    ostore->par_s1x = istore->em100_tx;	/* transducer 1 along location (m) */
		    ostore->par_s1y = istore->em100_ty;	/* transducer 1 athwart location (m) */
		    }
		else if (istore->sonar == MBSYS_SIMRAD_EM1000)
		    {
		    ostore->par_s1z = istore->em1000_td;	/* transducer 1 vertical location (m) */
		    ostore->par_s1x = istore->em1000_tx;	/* transducer 1 along location (m) */
		    ostore->par_s1y = istore->em1000_ty;	/* transducer 1 athwart location (m) */
		    }
		else
		    {
		    ostore->par_s1z = istore->em12_td;	/* transducer 1 vertical location (m) */
		    ostore->par_s1x = istore->em12_tx;	/* transducer 1 along location (m) */
		    ostore->par_s1y = istore->em12_ty;	/* transducer 1 athwart location (m) */
		    }
		ostore->par_s1h = istore->heading_offset;	/* transducer 1 heading (deg) */
		ostore->par_s1r = istore->roll_offset;	/* transducer 1 roll (m) */
		ostore->par_s1p = istore->pitch_offset;	/* transducer 1 pitch (m) */
		ostore->par_s1n = 0;	/* transducer 1 number of modules */
		ostore->par_s2z = 0.0;	/* transducer 2 vertical location (m) */
		ostore->par_s2x = 0.0;	/* transducer 2 along location (m) */
		ostore->par_s2y = 0.0;	/* transducer 2 athwart location (m) */
		ostore->par_s2h = 0.0;	/* transducer 2 heading (deg) */
		ostore->par_s2r = 0.0;	/* transducer 2 roll (m) */
		ostore->par_s2p = 0.0;	/* transducer 2 pitch (m) */
		ostore->par_s2n = 0;	/* transducer 2 number of modules */
		ostore->par_go1 = 0.0;	/* system (sonar head 1) gain offset */
		ostore->par_go2 = 0.0;	/* sonar head 2 gain offset */
		for (i=0;i<16;i++)
		    {
		    ostore->par_tsv[i] = '\0';	/* transmitter (sonar head 1) software version */
		    ostore->par_rsv[i] = '\0';	/* receiver (sonar head 2) software version */
		    ostore->par_bsv[i] = '\0';	/* beamformer software version */
		    ostore->par_psv[i] = '\0';	/* processing unit software version */
		    ostore->par_osv[i] = '\0';	/* operator station software version */
		    }
		ostore->par_dsd = 0.0;	/* depth sensor time delay (msec) */
		ostore->par_dso = 0.0;	/* depth sensor offset */
		ostore->par_dsf = 0.0;	/* depth sensor scale factor */
		ostore->par_dsh[0] = 'I';	/* depth sensor heave (IN or NI) */
		ostore->par_dsh[1] = 'N';	/* depth sensor heave (IN or NI) */
		ostore->par_aps = 0;	/* active position system number */
		ostore->par_p1m = 0;	/* position system 1 motion compensation (boolean) */
		ostore->par_p1t = 0;	/* position system 1 time stamp used 
					    (0=system time, 1=position input time) */
		ostore->par_p1z = 0.0;	/* position system 1 vertical location (m) */
		ostore->par_p1x = 0.0;	/* position system 1 along location (m) */
		ostore->par_p1y = 0.0;	/* position system 1 athwart location (m) */
		ostore->par_p1d = istore->pos_delay;	/* position system 1 time delay (sec) */
		for (i=0;i<16;i++)
		    {
		    ostore->par_p1g[i] = '\0';	/* position system 1 geodetic datum */
		    }
		ostore->par_p2m = 0;	/* position system 2 motion compensation (boolean) */
		ostore->par_p2t = 0;	/* position system 2 time stamp used 
					    (0=system time, 1=position input time) */
		ostore->par_p2z = 0.0;	/* position system 2 vertical location (m) */
		ostore->par_p2x = 0.0;	/* position system 2 along location (m) */
		ostore->par_p2y = 0.0;	/* position system 2 athwart location (m) */
		ostore->par_p2d = 0.0;	/* position system 2 time delay (sec) */
		for (i=0;i<16;i++)
		    {
		    ostore->par_p2g[i] = '\0';	/* position system 2 geodetic datum */
		    }
		ostore->par_p3m = 0;	/* position system 3 motion compensation (boolean) */
		ostore->par_p3t = 0;	/* position system 3 time stamp used 
					    (0=system time, 1=position input time) */
		ostore->par_p3z = 0.0;	/* position system 3 vertical location (m) */
		ostore->par_p3x = 0.0;	/* position system 3 along location (m) */
		ostore->par_p3y = 0.0;	/* position system 3 athwart location (m) */
		ostore->par_p3d = 0.0;	/* position system 3 time delay (sec) */
		for (i=0;i<16;i++)
		    {
		    ostore->par_p3g[i] = '\0';	/* position system 3 geodetic datum */
		    }
		ostore->par_msz = 0.0;	/* motion sensor vertical location (m) */
		ostore->par_msx = 0.0;	/* motion sensor along location (m) */
		ostore->par_msy = 0.0;	/* motion sensor athwart location (m) */
		ostore->par_mrp[0] = 'H';	/* motion sensor roll reference plane (HO or RP) */
		ostore->par_mrp[1] = 'O';	/* motion sensor roll reference plane (HO or RP) */
		ostore->par_msd = 0.0;	/* motion sensor time delay (sec) */
		ostore->par_msr = 0.0;	/* motion sensor roll offset (deg) */
		ostore->par_msp = 0.0;	/* motion sensor pitch offset (deg) */
		ostore->par_msg = 0.0;	/* motion sensor heading offset (deg) */
		ostore->par_gcg = 0.0;	/* gyro compass heading offset (deg) */
		for (i=0;i<4;i++)
		    {
		    ostore->par_cpr[i] = '\0';	/* cartographic projection */
		    }
		for (i=0;i<MBSYS_SIMRAD2_COMMENT_LENGTH;i++)
		    {
		    ostore->par_rop[i] = '\0';	/* responsible operator */
		    ostore->par_sid[i] = '\0';	/* survey identifier */
		    ostore->par_pll[i] = '\0';	/* survey line identifier (planned line number) */
		    ostore->par_com[i] = '\0';	/* comment */
		    }
	
		/* runtime parameter values */
		ostore->run_date = 0;		/* runtime parameter date = year*10000 + month*100 + day
					    Feb 26, 1995 = 19950226 */
		ostore->run_msec = 0;		/* runtime parameter time since midnight in msec
					    08:12:51.234 = 29570234 */
		ostore->run_ping_count = 0;	/* ping counter */
		ostore->run_serial = 0;		/* system 1 or 2 serial number */
		ostore->run_status = 0;		/* system status */
		ostore->run_mode = 0;		/* system mode:
					    0 : nearfield (EM3000) or very shallow (EM300)
					    1 :	normal (EM3000) or shallow (EM300)
					    2 : medium (EM300)
					    3 : deep (EM300)
					    4 : very deep (EM300) */
		ostore->run_filter_id = 0;	/* filter identifier - the two lowest bits
					    indicate spike filter strength:
						00 : off
						01 : weak
						10 : medium
						11 : strong 
					    bit 2 is set if the slope filter is on
					    bit 3 is set if the sidelobe filter is on
					    bit 4 is set if the range windows are expanded
					    bit 5 is set if the smoothing filter is on
					    bit	6 is set if the interference filter is on */
		ostore->run_min_depth = 0;	/* minimum depth (m) */
		ostore->run_max_depth = 0;	/* maximum depth (m) */
		ostore->run_absorption = 0;	/* absorption coefficient (0.01 dB/km) */
	
		ostore->run_tran_pulse = 0;	/* transmit pulse length (usec) */
		if (istore->sonar == MBSYS_SIMRAD_EM12S
		    || istore->sonar == MBSYS_SIMRAD_EM12D)
		    ostore->run_tran_beam = 17;	/* transmit beamwidth (0.1 deg) */
		else if (istore->sonar == MBSYS_SIMRAD_EM1000)
		    ostore->run_tran_beam = 33;	/* transmit beamwidth (0.1 deg) */
		else if (istore->sonar == MBSYS_SIMRAD_EM121)
		    ostore->run_tran_beam = 10;	/* transmit beamwidth (0.1 deg) */
		ostore->run_tran_pow = 0;	/* transmit power reduction (dB) */
		if (istore->sonar == MBSYS_SIMRAD_EM12S
		    || istore->sonar == MBSYS_SIMRAD_EM12D)
		    ostore->run_rec_beam = 35;	/* receiver beamwidth (0.1 deg) */
		else if (istore->sonar == MBSYS_SIMRAD_EM1000)
		    ostore->run_rec_beam = 33;	/* receiver beamwidth (0.1 deg) */
		else if (istore->sonar == MBSYS_SIMRAD_EM121)
		    ostore->run_rec_beam = 10;	/* transmit beamwidth (0.1 deg) */
		ostore->run_rec_band = 0;	/* receiver bandwidth (50 hz) */
		ostore->run_rec_gain = 0;	/* receiver fixed gain (dB) */
		ostore->run_tvg_cross = 0;	/* TVG law crossover angle (deg) */
		ostore->run_ssv_source = 0;	/* source of sound speed at transducer:
					    0 : from sensor
					    1 : manual
					    2 : from profile */
		ostore->run_max_swath = 0;	/* maximum swath width (m) */
		ostore->run_beam_space = 0;	/* beam spacing:
					    0 : determined by beamwidth (EM3000)
					    1 : equidistant
					    2 : equiangle */
		ostore->run_swath_angle = 0;	/* coverage sector of swath (deg) */
		ostore->run_stab_mode = 0;	/* yaw and pitch stabilization mode:
					    The upper bit (bit 7) is set if pitch
					    stabilization is on.
					    The two lower bits are used to show yaw
					    stabilization mode as follows:
						00 : none
						01 : to survey line heading
						10 : to mean vessel heading
						11 : to manually entered heading */
		for (i=0;i<4;i++)
		    {
		    ostore->run_spare[i] = '\0';
		    }
	
		/* sound velocity profile */
		ostore->svp_use_date = 0;	/* date at start of use
					    date = year*10000 + month*100 + day
					    Feb 26, 1995 = 19950226 */
		ostore->svp_use_msec = 0;	/* time at start of use since midnight in msec
					    08:12:51.234 = 29570234 */
		mbcopy_simrad_time_convert(verbose, 
		    istore->svp_year, istore->svp_month, 
		    istore->svp_day, istore->svp_hour, 
		    istore->svp_minute, istore->svp_second, 
		    istore->svp_centisecond, 
		    &ostore->svp_use_date, &ostore->svp_use_msec, 
		    error);
		ostore->svp_count = 0;		/* sequential counter or input identifier */
		ostore->svp_serial = 0;		/* system 1 serial number */
		ostore->svp_origin_date = 0;	/* date at svp origin
					    date = year*10000 + month*100 + day
					    Feb 26, 1995 = 19950226 */
		ostore->svp_origin_msec = 0;	/* time at svp origin since midnight in msec
					    08:12:51.234 = 29570234 */
		ostore->svp_num = istore->svp_num;		/* number of svp entries */
		ostore->svp_depth_res = 100;	/* depth resolution (cm) */
		for (i=0;i<MBSYS_SIMRAD2_MAXSVP;i++)
		    {
		    ostore->svp_depth[i] = istore->svp_depth[i];	/* depth of svp entries (according to svp_depth_res) */
		    ostore->svp_vel[i] = istore->svp_vel[i];	/* sound speed of svp entries (0.1 m/sec) */
		    }
		    
		/* position */
		ostore->pos_date = 0;		/* position date = year*10000 + month*100 + day
					    Feb 26, 1995 = 19950226 */
		ostore->pos_msec = 0;		/* position time since midnight in msec
					    08:12:51.234 = 29570234 */
		mbcopy_simrad_time_convert(verbose, 
		    istore->pos_year, istore->pos_month, 
		    istore->pos_day, istore->pos_hour, 
		    istore->pos_minute, istore->pos_second, 
		    istore->pos_centisecond, 
		    &ostore->pos_date, &ostore->pos_msec, 
		    error);
		ostore->pos_count = 0;		/* sequential counter */
		ostore->pos_serial = 0;		/* system 1 serial number */
		ostore->pos_latitude = 20000000 * istore->pos_latitude;
					/* latitude in decimal degrees * 20000000
					    (negative in southern hemisphere) 
					    if valid, invalid = 0x7FFFFFFF */
		ostore->pos_longitude = 10000000 * istore->pos_longitude;
					/* longitude in decimal degrees * 10000000
					    (negative in western hemisphere) 
					    if valid, invalid = 0x7FFFFFFF */
		ostore->pos_quality = 0;	/* measure of position fix quality (cm) */
		ostore->pos_speed = (int)(istore->speed / 0.036);
					/* speed over ground (cm/sec) if valid,
					    invalid = 0xFFFF */
		ostore->pos_course = 0xFFFF;	/* course over ground (0.01 deg) if valid,
					    invalid = 0xFFFF */
		ostore->pos_heading = (int) (istore->line_heading * 100);;
						/* heading (0.01 deg) if valid,
					    invalid = 0xFFFF */
		ostore->pos_system = istore->pos_type;
					/* position system number, type, and realtime use
					    - position system number given by two lowest bits
					    - fifth bit set means position must be derived
						from input Simrad 90 datagram
					    - sixth bit set means valid time is that of
						input datagram */
		ostore->pos_input_size = 0;	/* number of bytes in input position datagram */
		for (i=0;i<256;i++)
		    {
		    ostore->pos_input[i] = 0;	/* position input datagram as received, minus
					    header and tail (such as NMEA 0183 $ and CRLF) */
		    }
		    
		/* height */
		ostore->hgt_date = 0;		/* height date = year*10000 + month*100 + day
					    Feb 26, 1995 = 19950226 */
		ostore->hgt_msec = 0;		/* height time since midnight in msec
					    08:12:51.234 = 29570234 */
		ostore->hgt_count = 0;		/* sequential counter */
		ostore->hgt_serial = 0;		/* system 1 serial number */
		ostore->hgt_height = 0;		/* height (0.01 m) */
		ostore->hgt_type = 0;		/* height type as given in input datagram or if
					    zero the height is derived from the GGK datagram
					    and is the height of the water level re the
					    vertical datum */
		
		/* tide */
		ostore->tid_date = 0;		/* tide date = year*10000 + month*100 + day
					    Feb 26, 1995 = 19950226 */
		ostore->tid_msec = 0;		/* tide time since midnight in msec
					    08:12:51.234 = 29570234 */
		ostore->tid_count = 0;		/* sequential counter */
		ostore->tid_serial = 0;		/* system 1 serial number */
		ostore->tid_origin_date = 0;	/* tide input date = year*10000 + month*100 + day
					    Feb 26, 1995 = 19950226 */
		ostore->tid_origin_msec = 0;	/* tide input time since midnight in msec
					    08:12:51.234 = 29570234 */
		ostore->tid_tide = 0;		/* tide offset (0.01 m) */	
		
		/* clock */
		ostore->clk_date = 0;		/* system date = year*10000 + month*100 + day
					    Feb 26, 1995 = 19950226 */
		ostore->clk_msec = 0;		/* system time since midnight in msec
					    08:12:51.234 = 29570234 */
		ostore->clk_count = 0;		/* sequential counter */
		ostore->clk_serial = 0;		/* system 1 serial number */
		ostore->clk_origin_date	= 0;	/* external clock date = year*10000 + month*100 + day
					    Feb 26, 1995 = 19950226 */
		ostore->clk_origin_msec = 0;	/* external clock time since midnight in msec
					    08:12:51.234 = 29570234 */
		ostore->clk_1_pps_use = 0;	/* if 1 then the internal clock is synchronized
					    to an external 1 PPS signal, if 0 then not */
	
		/* allocate memory for data structure if needed */
		if (istore->kind == MB_DATA_DATA
			&& ostore->ping == NULL)
			status = mb_mallocd(verbose, __FILE__, __LINE__,
				sizeof(struct mbsys_simrad2_ping_struct),
				(void **)&(ostore->ping),error);
				
		if (istore->kind == MB_DATA_DATA
			&& istore->ping != NULL
			&& ostore->ping != NULL)
			{
			/* get data structure pointer */
			iping = (struct mbsys_simrad_survey_struct *) istore->ping;
			oping = (struct mbsys_simrad2_ping_struct *) ostore->ping;

			/* set beam widths for EM121 */
			if (istore->sonar == MBSYS_SIMRAD_EM121)
			    {
			    if (iping->bath_mode == 3)
				{
				ostore->run_tran_beam = 40;	/* transmit beamwidth (0.1 deg) */
				ostore->run_rec_beam = 40;	/* transmit beamwidth (0.1 deg) */
				}
			    else if (iping->bath_mode == 2)
				{
				ostore->run_tran_beam = 20;	/* transmit beamwidth (0.1 deg) */
				ostore->run_rec_beam = 20;	/* transmit beamwidth (0.1 deg) */
				}
			    else
				{
				ostore->run_tran_beam = 10;	/* transmit beamwidth (0.1 deg) */
				ostore->run_rec_beam = 10;	/* transmit beamwidth (0.1 deg) */
				}
			    }

			/* initialize everything */
			oping->png_date = ostore->date;	
					/* date = year*10000 + month*100 + day
					    Feb 26, 1995 = 19950226 */
			oping->png_msec = ostore->msec;	
					/* time since midnight in msec
					    08:12:51.234 = 29570234 */
			oping->png_count = iping->ping_number;	
					/* sequential counter or input identifier */
			oping->png_serial = iping->swath_id;	
					/* system 1 or system 2 serial number */
			oping->png_latitude = 20000000 * iping->latitude;
					/* latitude in decimal degrees * 20000000
					    (negative in southern hemisphere) 
					    if valid, invalid = 0x7FFFFFFF */
			oping->png_longitude = 10000000 * iping->longitude;
					/* longitude in decimal degrees * 10000000
					    (negative in western hemisphere) 
					    if valid, invalid = 0x7FFFFFFF */
			oping->png_speed = 0xFFFF;
					/* speed over ground (cm/sec) if valid,
					    invalid = 0xFFFF */
			if (ostore->sonar == MBSYS_SIMRAD2_EM121)
			    oping->png_heading = iping->heading; /* heading (0.01 deg) */	
			else
			    oping->png_heading = 10 * iping->heading; /* heading (0.01 deg) */
			oping->png_ssv = iping->sound_vel;	
					/* sound speed at transducer (0.1 m/sec) */
			oping->png_xducer_depth = iping->ping_heave + (int) (100 * ostore->par_s1z);
			bath_offset = 0.01 * oping->png_xducer_depth;
					/* transmit transducer depth (0.01 m) 
					    - The transmit transducer depth plus the
						depth offset multiplier times 65536 cm
						should be added to the beam depths to 
						derive the depths re the water line.
						The depth offset multiplier will usually
						be zero, except when the EM3000 sonar
						head is on an underwater vehicle at a
						depth greater than about 650 m. Note that
						the offset multiplier will be negative
						(-1) if the actual heave is large enough
						to bring the transmit transducer above 
						the water line. This may represent a valid
						situation,  but may also be due to an 
						erroneously set installation depth of 
						the either transducer or the water line. */
			if (oping->png_xducer_depth > 0)
			    oping->png_offset_multiplier = 0;	
					/* transmit transducer depth offset multiplier */ 
			else
			    {
			    oping->png_offset_multiplier = -1;	
			    oping->png_xducer_depth = oping->png_xducer_depth + 65536;   
					/* transmit transducer depth offset multiplier */ 
			    }
					   
			/* beam data */
			oping->png_nbeams_max = iping->beams_bath;
					/* maximum number of beams possible */
			oping->png_nbeams = iping->beams_bath;	
					/* number of valid beams */
			if ((ostore->sonar == MBSYS_SIMRAD2_EM12S
				|| ostore->sonar == MBSYS_SIMRAD2_EM12D)
			    && iping->bath_res == 1)
			    {
			    oping->png_depth_res = 10;	
					/* depth resolution (0.1 m) */
			    oping->png_distance_res = 20;	
					/* x and y resolution (0.2 m) */
			    oping->png_sample_rate = 5000;	
					/* sampling rate (Hz) */
			    }
			else if ((ostore->sonar == MBSYS_SIMRAD2_EM12S
				|| ostore->sonar == MBSYS_SIMRAD2_EM12D)
			    && iping->bath_res == 2)
			    {
			    oping->png_depth_res = 20;	
					/* depth resolution (0.2 m) */
			    oping->png_distance_res = 50;	
					/* x and y resolution (0.5 m) */
			    oping->png_sample_rate = 1250;	
					/* sampling rate (Hz) */
			    }
			else if (ostore->sonar == MBSYS_SIMRAD2_EM1000)
			    {
			    oping->png_depth_res = 2;	
					/* depth resolution (0.02 m) */
			    oping->png_distance_res = 10;	
					/* x and y resolution (0.1 m) */
			    oping->png_sample_rate = 20000;	
					/* sampling rate (Hz) */
			    }
			else if (ostore->sonar == MBSYS_SIMRAD2_EM121)
			    {
			    oping->png_depth_res = iping->depth_res;	
					/* depth resolution (0.01 m) */
			    oping->png_distance_res = iping->across_res;	
					/* x and y resolution (0.01 m) */
			    oping->png_sample_rate = (int)(1.0 / (0.0001 * iping->range_res));	
					/* sampling rate (Hz) */
			    }

			/* get angles */
			interleave = MB_NO;
			if (istore->sonar == MBSYS_SIMRAD_EM1000)
				{
				if (iping->bath_mode == 1)
				    {
				    angles_simrad = angles_EM1000_ISO_ANG_60_2_MS_48_FAIS;
				    interleave = MB_NO;
				    }
				else if (iping->bath_mode == 2)
				    {
				    angles_simrad = angles_EM1000_ISO_ANG_120_07_MS_48_FAIS;
				    interleave = MB_YES;
				    }
				else if (iping->bath_mode == 3)
				    {
				    angles_simrad = angles_EM1000_ISO_ANG_150_02_MS_60_FAIS;
				    interleave = MB_YES;
				    }
				else if (iping->bath_mode == 4)
				    {
				    angles_simrad = angles_EM1000_CHANNEL_02_MS_60_FAIS;
				    interleave = MB_YES;
				    }
				else if (iping->bath_mode == 5)
				    {
				    angles_simrad = angles_EM1000_150_02_MS_60_FAIS;
				    interleave = MB_YES;
				    }
				else if (iping->bath_mode == 6)
				    {
				    angles_simrad = angles_EM1000_140_02_MS_60_FAIS;
				    interleave = MB_YES;
				    }
				else if (iping->bath_mode == 7)
				    {
				    angles_simrad = angles_EM1000_128_02_MS_60_FAIS;
				    interleave = MB_YES;
				    }
				else if (iping->bath_mode == 8)
				    {
				    angles_simrad = angles_EM1000_120_07_MS_48_FAIS;
				    interleave = MB_YES;
				    }
				else if (iping->bath_mode == 9)
				    {
				    angles_simrad = angles_EM1000_104_07_MS_48_FAIS;
				    interleave = MB_YES;
				    }
				else if (iping->bath_mode == 10)
				    {
				    angles_simrad = angles_EM1000_88_07_MS_48_FAIS;
				    interleave = MB_YES;
				    }
				else if (iping->bath_mode == 11)
				    {
				    angles_simrad = angles_EM1000_70_2_MS_48_FAIS;
				    interleave = MB_NO;
				    }
				else if (iping->bath_mode == 12)
				    {
				    angles_simrad = angles_EM1000_BERGE_02_MS_60_FAIS;
				    interleave = MB_YES;
				    }
				else if (iping->bath_mode == 13)
				    {
				    angles_simrad = angles_EM1000_BERGE_02_MS_60_FAIS;
				    interleave = MB_YES;
				    }
				}
			else if (istore->sonar == MBSYS_SIMRAD_EM12S)
				{
				if (iping->bath_mode == 1)
				    angles_simrad = angles_EM12S_ISO_ANG_SHALLOW;
				else if (iping->bath_mode == 2)
				    angles_simrad = angles_EM12S_ISO_ANG_DEEP;
				else if (iping->bath_mode == 3)
				    angles_simrad = angles_EM12S_SHALLOW;
				else if (iping->bath_mode == 4)
				    angles_simrad = angles_EM12S_120;
				else if (iping->bath_mode == 5)
				    angles_simrad = angles_EM12S_105;
				else if (iping->bath_mode == 6)
				    angles_simrad = angles_EM12S_90;			
				}
			else if (istore->sonar == MBSYS_SIMRAD_EM12D
				&& iping->swath_id == EM_SWATH_PORT)
				{
				if (iping->bath_mode == 1)
				    angles_simrad = angles_EM12DP_ISO_ANG_SHALLOW;
				else if (iping->bath_mode == 2)
				    angles_simrad = angles_EM12DP_ISO_ANG_DEEP;
				else if (iping->bath_mode == 3)
				    angles_simrad = angles_EM12DP_SHALLOW;
				else if (iping->bath_mode == 4)
				    angles_simrad = angles_EM12DP_150;
				else if (iping->bath_mode == 5)
				    angles_simrad = angles_EM12DP_140;
				else if (iping->bath_mode == 6)
				    angles_simrad = angles_EM12DP_128;			
				else if (iping->bath_mode == 7)
				    angles_simrad = angles_EM12DP_114;			
				else if (iping->bath_mode == 8)
				    angles_simrad = angles_EM12DP_98;			
				}
			else if (istore->sonar == MBSYS_SIMRAD_EM12D
				&& iping->swath_id == EM_SWATH_STARBOARD)
				{
				if (iping->bath_mode == 1)
				    angles_simrad = angles_EM12DS_ISO_ANG_SHALLOW;
				else if (iping->bath_mode == 2)
				    angles_simrad = angles_EM12DS_ISO_ANG_DEEP;
				else if (iping->bath_mode == 3)
				    angles_simrad = angles_EM12DS_SHALLOW;
				else if (iping->bath_mode == 4)
				    angles_simrad = angles_EM12DS_150;
				else if (iping->bath_mode == 5)
				    angles_simrad = angles_EM12DS_140;
				else if (iping->bath_mode == 6)
				    angles_simrad = angles_EM12DS_128;			
				else if (iping->bath_mode == 7)
				    angles_simrad = angles_EM12DS_114;			
				else if (iping->bath_mode == 8)
				    angles_simrad = angles_EM12DS_98;			
				}
				
			/* if interleaved get center beam */
			if (interleave == MB_YES)
				{
				if (iping->bath_mode == 12
				    && fabs(iping->bath_acrosstrack[28])
					< fabs(iping->bath_acrosstrack[29]))
				    istep = 1;
				else if (iping->bath_mode == 13
				    && fabs(iping->bath_acrosstrack[31])
					< fabs(iping->bath_acrosstrack[30]))
				    istep = 1;
				else if (fabs(iping->bath_acrosstrack[oping->png_nbeams/2-1])
				    < fabs(iping->bath_acrosstrack[oping->png_nbeams/2]))
				    istep = 1;
				else
				    istep = 0;
				}
				
			/* set beam values */
			for (i=0;i<oping->png_nbeams;i++)
			    {
			    oping->png_depth[i] = (int)((unsigned short)iping->bath[i]);	
					/* depths in depth resolution units */
			    if (oping->png_depth[i] != 0)
				oping->png_depth[i] -= (int)(bath_offset / (0.01 * oping->png_depth_res));
			    oping->png_acrosstrack[i] = iping->bath_acrosstrack[i];
					/* acrosstrack distances in distance resolution units */
			    oping->png_alongtrack[i] = iping->bath_alongtrack[i];
					/* alongtrack distances in distance resolution units */

			    alpha = 0.01 * iping->pitch;
			    if (istore->sonar == MBSYS_SIMRAD_EM1000
				&& iping->bath_mode == 13)
				{
				beta = 90.0 - angles_simrad[oping->png_nbeams-1-(2*i+istep)];
				}
			    else if (istore->sonar == MBSYS_SIMRAD_EM1000
				&& interleave == MB_YES)
				{
				beta = 90.0 + angles_simrad[2*i+istep];
				}
			    else if (istore->sonar == MBSYS_SIMRAD_EM1000)
				{
				beta = 90.0 + angles_simrad[i];
				}
			    else
				{
				beta = 90.0 + angles_simrad[i];
				}
			    mb_rollpitch_to_takeoff(verbose, 
				    alpha, beta, &theta, &phi, error);
			    oping->png_depression[i] = (int) (100 * (90.0 - theta));
					/* Beam depression angles
						in 0.01 degree. These are the takeoff angles used
						in raytracing calculations. */
			    oping->png_azimuth[i] = (int) (100 * (90.0 - phi));
			    if (oping->png_azimuth[i] < 0) oping->png_azimuth[i] += 36000;
					/* Beam azimuth angles
						in 0.01 degree. These values used to rotate sounding
						position relative to the sonar after raytracing. */
			    oping->png_range[i] = iping->tt[i];
					/* Ranges as one way 
						travel times in time units defined as half 
						the inverse sampling rate. */
			    oping->png_quality[i] = iping->quality[i];	
					/* 0-254 */
			    oping->png_window[i] = 0;		
					/* samples/4 */
			    oping->png_amp[i] = iping->amp[i];		
					/* 0.5 dB */
			    oping->png_beam_num[i] = i + 1;	
					/* beam 128 is first beam on 
					    second head of EM3000D */
			    if (iping->bath[i] > 0)
				oping->png_beamflag[i] = MB_FLAG_NONE;
			    else
				oping->png_beamflag[i] = MB_FLAG_NULL;
			    }
					   
			/* raw travel time and angle data */
			oping->png_raw1_read = MB_NO;	/* flag indicating actual reading of rawbeam1 record */
			oping->png_raw2_read = MB_NO;	/* flag indicating actual reading of rawbeam2 record */
			oping->png_raw_nbeams = 0;	/* number of raw travel times and angles
				    - nonzero only if raw beam record read */
					/* number of valid beams */
	
			/* get raw pixel size to be stored in oping->png_max_range */
			if (iping->pixels_ssraw > 0)
			    oping->png_ss_read = MB_YES;	/* flag indicating actual reading of sidescan record */
			else
			    oping->png_ss_read = MB_NO;	/* flag indicating actual reading of sidescan record */
			oping->png_ss_date = oping->png_date;	/* date = year*10000 + month*100 + day
				    Feb 26, 1995 = 19950226 */
			oping->png_ss_msec = oping->png_msec;	/* time since midnight in msec
				    08:12:51.234 = 29570234 */
			if (istore->sonar == MBSYS_SIMRAD_EM12D
			    || istore->sonar == MBSYS_SIMRAD_EM12S
			    || istore->sonar == MBSYS_SIMRAD_EM121)
			    {
			    if (iping->ss_mode == 1)
				oping->png_max_range = 60;
			    else if (iping->ss_mode == 2)
				oping->png_max_range = 240;
			    else if (iping->bath_mode == 1
					|| iping->bath_mode == 3)
				oping->png_max_range = 60;
			    else
				oping->png_max_range = 240;
			    }
			else if (istore->sonar == MBSYS_SIMRAD_EM1000)
			    {
			    if (iping->ss_mode == 3)
				oping->png_max_range = 30;
			    else if (iping->ss_mode == 4)
				oping->png_max_range = 30;
			    else if (iping->ss_mode == 5)
				oping->png_max_range = 15;
			    else
				oping->png_max_range = 15;
			    }
		
			/* sidescan */
			oping->png_r_zero = 0;	
					/* range to normal incidence used in TVG
					    (R0 predicted) in samples */
			oping->png_r_zero_corr = 0;
					/* range to normal incidence used to correct
					    sample amplitudes in number of samples */
			oping->png_tvg_start = 0;	
					/* start sample of TVG ramp if not enough 
					    dynamic range (0 otherwise) */
			oping->png_tvg_stop = 0;	\
					/* stop sample of TVG ramp if not enough 
					    dynamic range (0 otherwise) */
			oping->png_bsn = 0;	
					/* normal incidence backscatter (BSN) in dB */
			oping->png_bso = 0;	
					/* oblique incidence backscatter (BSO) in dB */
			if (ostore->sonar == MBSYS_SIMRAD2_EM121)
			    oping->png_tx = 10 * iping->beam_width;	
			else if (ostore->sonar == MBSYS_SIMRAD2_EM12S
				    || ostore->sonar == MBSYS_SIMRAD2_EM12D)
			    oping->png_tx = 17;	
			else if (ostore->sonar == MBSYS_SIMRAD2_EM1000)
			    oping->png_tx = 33;	
					/* Tx beamwidth in 0.1 degree */
			oping->png_tvg_crossover = 0;	
					/* TVG law crossover angle in degrees */
			oping->png_nbeams_ss = oping->png_nbeams;	
					/* number of beams with sidescan */
			oping->png_npixels = iping->pixels_ssraw;
			for (i=0;i<oping->png_nbeams_ss;i++)
			    {
			    oping->png_beam_index[i] = i;	
					/* beam index number */
			    oping->png_sort_direction[i] = 0;	
					/* sorting direction - first sample in beam has lowest
					    range if 1, highest if -1. */
			    oping->png_beam_samples[i] = iping->beam_samples[i];	
					/* number of sidescan samples derived from
						each beam */
			    oping->png_start_sample[i] = iping->beam_start_sample[i];	
					/* start sample number */
			    oping->png_center_sample[i] = iping->beam_center_sample[i];	
					/* center sample number */
			    }
			for (i=0;i<oping->png_npixels;i++)
			    {
			    oping->png_ssraw[i] = iping->ssraw[i];
					/* the raw sidescan ordered port to starboard */
			    }
			oping->png_pixel_size = iping->pixel_size;
			oping->png_pixels_ss = iping->pixels_ss;
			for (i=0;i<oping->png_pixels_ss;i++)
			    {
			    if (iping->ss[i] != 0)
			    	{
			    	oping->png_ss[i] = iping->ss[i];
					/* the processed sidescan ordered port to starboard */
			    	oping->png_ssalongtrack[i] = iping->ssalongtrack[i];
					/* the processed sidescan alongtrack distances 
						in distance resolution units */
				}
			    else
			    	{
			    	oping->png_ss[i] = EM2_INVALID_AMP;
					/* the processed sidescan ordered port to starboard */
			    	oping->png_ssalongtrack[i] = EM2_INVALID_AMP;
					/* the processed sidescan alongtrack distances 
						in distance resolution units */
				}
			    }
			}
		}

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBcopy function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       error:      %d\n",*error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:     %d\n",status);
		}

	/* return status */
	return(status);
}
/*--------------------------------------------------------------------*/
int mbcopy_simrad_time_convert(int verbose, 
		    int year, int month, 
		    int day, int hour, 
		    int minute, int second, 
		    int centisecond, 
		    int *date, int *msec, 
		    int *error)
{
	char	*function_name = "mbcopy_simrad_time_convert";
	int	status = MB_SUCCESS;
	int	time_i[7];

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBcopy function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       year:       %d\n",year);
		fprintf(stderr,"dbg2       month:      %d\n",month);
		fprintf(stderr,"dbg2       day:        %d\n",day);
		fprintf(stderr,"dbg2       hour:       %d\n",hour);
		fprintf(stderr,"dbg2       minute:     %d\n",minute);
		fprintf(stderr,"dbg2       second:     %d\n",second);
		fprintf(stderr,"dbg2       centisecond:%d\n",centisecond);
		}
		
	/* get time */
	mb_fix_y2k(verbose, year, &time_i[0]);
	time_i[1] = month;
	time_i[2] = day;
	time_i[3] = hour;
	time_i[4] = minute;
	time_i[5] = second;
	time_i[6] = 10000 * centisecond;
	*date = 10000 * time_i[0]
			+ 100 * time_i[1]
			+ time_i[2];
	*msec = 3600000 * time_i[3]
			+ 60000 * time_i[4]
			+ 1000 * time_i[5]
			+ 0.001 * time_i[6];

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBcopy function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       date:       %d\n",*date);
		fprintf(stderr,"dbg2       msec:       %d\n",*msec);
		fprintf(stderr,"dbg2       error:      %d\n",*error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:     %d\n",status);
		}

	/* return status */
	return(status);
}
/*--------------------------------------------------------------------*/
int mbcopy_any_to_mbldeoih(int verbose, 
		int kind, int *time_i, double time_d, 
		double navlon, double navlat, double speed, double heading, 
		double draft, double altitude, 
		double roll, double pitch, double heave, 
		double	beamwidth_xtrack, double beamwidth_ltrack, 
		int nbath, int namp, int nss,
		char *beamflag, double *bath, double *amp, 
		double *bathacrosstrack, double *bathalongtrack,
		double *ss, double *ssacrosstrack, double *ssalongtrack,
		char *comment, 
		void *ombio_ptr, void *ostore_ptr, 
		int *error)
{
	char	*function_name = "mbcopy_any_to_mbldeoih";
	int	status = MB_SUCCESS;
	struct mbsys_ldeoih_struct *ostore;
	int	i;

	/* get data structure pointer */
	ostore = (struct mbsys_ldeoih_struct *) ostore_ptr;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBcopy function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       ombio_ptr:  %lu\n",(size_t)ombio_ptr);
		fprintf(stderr,"dbg2       ostore_ptr: %lu\n",(size_t)ostore_ptr);
		fprintf(stderr,"dbg2       kind:       %d\n",kind);
		}
	if (verbose >= 2 && (kind == MB_DATA_DATA || kind == MB_DATA_NAV))
		{
		fprintf(stderr,"dbg2       time_i[0]:  %d\n",time_i[0]);
		fprintf(stderr,"dbg2       time_i[1]:  %d\n",time_i[1]);
		fprintf(stderr,"dbg2       time_i[2]:  %d\n",time_i[2]);
		fprintf(stderr,"dbg2       time_i[3]:  %d\n",time_i[3]);
		fprintf(stderr,"dbg2       time_i[4]:  %d\n",time_i[4]);
		fprintf(stderr,"dbg2       time_i[5]:  %d\n",time_i[5]);
		fprintf(stderr,"dbg2       time_i[6]:  %d\n",time_i[6]);
		fprintf(stderr,"dbg2       time_d:     %f\n",time_d);
		fprintf(stderr,"dbg2       navlon:     %f\n",navlon);
		fprintf(stderr,"dbg2       navlat:     %f\n",navlat);
		fprintf(stderr,"dbg2       speed:      %f\n",speed);
		fprintf(stderr,"dbg2       heading:    %f\n",heading);
		fprintf(stderr,"dbg2       draft:      %f\n",draft);
		fprintf(stderr,"dbg2       altitude:   %f\n",altitude);
		fprintf(stderr,"dbg2       roll:       %f\n",roll);
		fprintf(stderr,"dbg2       pitch:      %f\n",pitch);
		fprintf(stderr,"dbg2       heave:      %f\n",heave);
		fprintf(stderr,"dbg2       beamwidth_xtrack: %f\n",beamwidth_xtrack);
		fprintf(stderr,"dbg2       beamwidth_ltrack: %f\n",beamwidth_ltrack);
		}
	if (verbose >= 2 && kind == MB_DATA_DATA)
		{
		fprintf(stderr,"dbg2       nbath:      %d\n",
			nbath);
		if (verbose >= 3) 
		 for (i=0;i<nbath;i++)
		  fprintf(stderr,"dbg3       beam:%d  flag:%3d  bath:%f  acrosstrack:%f  alongtrack:%f\n",
			i,beamflag[i],bath[i],
			bathacrosstrack[i],bathalongtrack[i]);
		fprintf(stderr,"dbg2       namp:       %d\n",namp);
		if (verbose >= 3) 
		 for (i=0;i<namp;i++)
		  fprintf(stderr,"dbg3        beam:%d   amp:%f  acrosstrack:%f  alongtrack:%f\n",
			i,amp[i],bathacrosstrack[i],bathalongtrack[i]);
		fprintf(stderr,"dbg2        nss:       %d\n",nss);
		if (verbose >= 3) 
		 for (i=0;i<nss;i++)
		  fprintf(stderr,"dbg3        pixel:%d   ss:%f  acrosstrack:%f  alongtrack:%f\n",
			i,ss[i],ssacrosstrack[i],ssalongtrack[i]);
		}
	if (verbose >= 2 && kind == MB_DATA_COMMENT)
		{
		fprintf(stderr,"dbg2       comment:     \ndbg2       %s\n",
			comment);
		}

	/* copy the data  */
	if (ostore != NULL)
		{		
		/* set beam widths */
		ostore->beam_xwidth = beamwidth_xtrack;
		ostore->beam_lwidth = beamwidth_ltrack;
		ostore->kind = kind;

		/* insert data */
		if (kind == MB_DATA_DATA)
		        {
			mb_insert_nav(verbose, ombio_ptr, (void *)ostore, 
					time_i, time_d, 
					navlon, navlat, speed, heading, draft, 
					roll, pitch, heave, 
					error);
			mb_insert_altitude(verbose, ombio_ptr, (void *)ostore, 
					draft, altitude, 
					error);
			}
		status = mb_insert(verbose, ombio_ptr, (void *)ostore,
				kind, time_i, time_d, 
				navlon, navlat, speed, heading, 
				nbath,namp,nss,
				beamflag,bath,amp,bathacrosstrack,
				bathalongtrack,
				ss,ssacrosstrack,ssalongtrack,
				comment, error);
		}

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBcopy function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       error:      %d\n",*error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:     %d\n",status);
		}

	/* return status */
	return(status);
}
/*--------------------------------------------------------------------*/
int mbcopy_reson8k_to_gsf(int verbose, 
		void *imbio_ptr, 
		void *ombio_ptr,
		int *error)
{
	char	*function_name = "mbcopy_reson8k_to_gsf";
	int	status = MB_SUCCESS;
	struct mb_io_struct *imb_io_ptr;
	struct mb_io_struct *omb_io_ptr;
	struct mbsys_reson8k_struct *istore;
	struct mbsys_gsf_struct *ostore;			
	gsfDataID	    *dataID; 		/* pointers withinin gsf data */
	gsfRecords	    *records;
	gsfSwathBathyPing   *mb_ping;
	gsfMBParams params;
	double  multiplier, offset;
	double gain_correction;
	double	angscale;
	double	alpha;
	double	beta;
	double	theta;
	double	phi;
	int	icenter;
	int	i, ret;

	imb_io_ptr  = (struct mb_io_struct *) imbio_ptr;
	omb_io_ptr = (struct mb_io_struct *) ombio_ptr;
	
	/* get reson data structure pointer */
	istore = imb_io_ptr->store_data;
	
	/* get gsf data structure pointer */
	ostore = omb_io_ptr->store_data;
		
	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBcopy function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       imbio_ptr:  %lu\n",(size_t)imbio_ptr);
		fprintf(stderr,"dbg2       ombio_ptr:  %lu\n",(size_t)ombio_ptr);
		fprintf(stderr,"dbg2       istore:     %lu\n",(size_t)istore);
		fprintf(stderr,"dbg2       ostore:     %lu\n",(size_t)ostore);
		fprintf(stderr,"dbg2       kind:       %lu\n",(size_t)istore->kind);
		}
	
	/* copy the data  */
	if (istore != NULL && ostore != NULL)
		{
		/* output gsf data structure  */
		records = &(ostore->records);
		dataID  = &(ostore->dataID);
		mb_ping = &(records->mb_ping);

		/* set data kind */
		ostore->kind = istore->kind;

		/* insert data in structure */
		if (istore->kind == MB_DATA_DATA)
			{		
			/* on the first ping set the processing parameters up  */
			if (omb_io_ptr->ping_count == 0) 
				{
				/* ostore->kind = MB_DATA_PROCESSING_PARAMETERS;
				dataID->recordID = GSF_RECORD_PROCESSING_PARAMETERS; */
				memset((void *)&params, 0, sizeof(gsfMBParams));

				params.roll_compensated = GSF_COMPENSATED; 
				params.pitch_compensated = GSF_COMPENSATED;
				params.heave_compensated = GSF_COMPENSATED;
				params.tide_compensated = 0; 
				params.ray_tracing = 0      ;
				params.depth_calculation =GSF_DEPTHS_RE_1500_MS; 
				params.to_apply.draft[0] = 0;	    
				params.to_apply.roll_bias[0] = 0;;	    
				params.to_apply.pitch_bias[0] = 0;;	    
				params.to_apply.gyro_bias[0] = 0;;
				/* note it appears the x and y axis are switched between 
					reson and gsf reference systems	*/
				params.to_apply.position_x_offset = istore->NavOffsetY;  	    
				params.to_apply.position_y_offset = istore->NavOffsetX;  	    
				params.to_apply.position_z_offset = istore->NavOffsetZ;  	    
				params.to_apply.transducer_x_offset[0] = istore->MBOffsetY;
				params.to_apply.transducer_y_offset[0] = istore->MBOffsetX;
				params.to_apply.transducer_z_offset[0] = istore->MBOffsetZ;
				params.to_apply.mru_roll_bias = istore->MRUOffsetRoll;			    
				params.to_apply.mru_pitch_bias = istore->MRUOffsetPitch; 		    
				params.to_apply.mru_heading_bias = 0;		    
				params.to_apply.mru_x_offset = istore->MRUOffsetY;			    
				params.to_apply.mru_y_offset = istore->MRUOffsetX;			    
				params.to_apply.mru_z_offset = istore->MRUOffsetZ;			    
				params.to_apply.center_of_rotation_x_offset = 0;	    
				params.to_apply.center_of_rotation_y_offset = 0;	    
				params.to_apply.center_of_rotation_z_offset = 0;
				ret = gsfPutMBParams(&params, records, omb_io_ptr->gsfid, 1); 
				}

			/* set data id */
			dataID->recordID = GSF_RECORD_SWATH_BATHYMETRY_PING;
			mb_ping = &(records->mb_ping);

			/* get time */
			mb_ping->ping_time.tv_sec = (int) istore->png_time_d;
			mb_ping->ping_time.tv_nsec 
				= (int) (1000000000 
				    * (istore->png_time_d 
					    - mb_ping->ping_time.tv_sec));

			/* get navigation, applying inverse projection if defined */
			mb_ping->longitude = istore->png_longitude;
			mb_ping->latitude = istore->png_latitude;
			if (imb_io_ptr->projection_initialized == MB_YES)
				{
				mb_proj_inverse(verbose, imb_io_ptr->pjptr,
								mb_ping->longitude, mb_ping->latitude,
								&(mb_ping->longitude), &(mb_ping->latitude),
								error);
				}

			/* get heading */
			mb_ping->heading = istore->png_heading;

			/* get speed */
			mb_ping->speed = istore->png_speed/ 1.852;
			
			/* set sonar depth */
			mb_ping->depth_corrector = istore->MBOffsetZ;

                	/* do roll pitch heave */
			mb_ping->roll =  istore->png_roll;
			mb_ping->pitch = istore->png_pitch;
			mb_ping->heave = istore->png_heave;		

			/* get numbers of beams */
			mb_ping->number_beams = istore->beams_bath;

			/* allocate memory in arrays if required */
			if (istore->beams_bath > 0)
			      {
			      mb_ping->beam_flags 
				  = (unsigned char *) 
				      realloc(mb_ping->beam_flags,
						  istore->beams_bath * sizeof(char));
			      mb_ping->depth 
				  = (double *) 
				      realloc(mb_ping->depth,
						  istore->beams_bath * sizeof(double));
			      mb_ping->across_track 
				  = (double *) 
				      realloc(mb_ping->across_track,
						  istore->beams_bath * sizeof(double));
			      mb_ping->along_track 
				  = (double *) 
				      realloc(mb_ping->along_track,
						  istore->beams_bath * sizeof(double));
			      mb_ping->travel_time 
				  = (double *) 
				      realloc(mb_ping->travel_time,
						  istore->beams_bath * sizeof(double));
			      mb_ping->beam_angle 
				  = (double *) 
				      realloc(mb_ping->beam_angle,
						  istore->beams_bath * sizeof(double));
			      mb_ping->beam_angle_forward
				  = (double *) 
				      realloc(mb_ping->beam_angle_forward,
						  istore->beams_bath * sizeof(double));

			      if (mb_ping->beam_flags == NULL
				  || mb_ping->depth == NULL
				  || mb_ping->across_track == NULL
				  || mb_ping->along_track == NULL
				  || mb_ping->travel_time == NULL
				  || mb_ping->beam_angle_forward == NULL 
				  || mb_ping->beam_angle == NULL )
				  {
				  status = MB_FAILURE;
				  *error = MB_ERROR_MEMORY_FAIL;
				  }
			      }
			  if (istore->beams_amp > 0)
			      {
			      mb_ping->mr_amplitude 
				  = (double *) 
				      realloc(mb_ping->mr_amplitude,
						  istore->beams_amp * sizeof(double));
			      if (mb_ping->mr_amplitude == NULL)
				  {
				  status = MB_FAILURE;
				  *error = MB_ERROR_MEMORY_FAIL;
				  }
			      }

			  /* if ping flag set check for any unset
			      beam flags - unset ping flag if any
			      good beams found */
			  if (mb_ping->ping_flags != 0)
			      {
			      for (i=0;i<istore->beams_bath;i++)
				  {
				  if (mb_beam_ok(istore->beamflag[i]))
				      mb_ping->ping_flags = 0;
				  }
			      }

			  /* read depth and beam location values into storage arrays */
			  icenter = istore->beams_bath / 2;
			  angscale = ((double)istore->beam_width_num) 
					/ ((double)istore->beam_width_denom);
			  for (i=0;i<istore->beams_bath;i++)
				  {
				  mb_ping->beam_flags[i] = istore->beamflag[i];
				  if (istore->beamflag[i] != MB_FLAG_NULL)
				      {
				      mb_ping->depth[i] = istore->bath[i];
				      mb_ping->across_track[i] = istore->bath_acrosstrack[i];
				      mb_ping->along_track[i] = istore->bath_alongtrack[i];
				      mb_ping->travel_time[i] = 0.25 * (double)istore->range[i]/(double)istore->sample_rate;
				      alpha = istore->png_pitch;
				      beta = 90.0 + (icenter - i) * angscale + istore->png_roll; 
				      mb_rollpitch_to_takeoff(
					      verbose, 
					      alpha, beta, 
					      &theta, &phi, 
					      error);
				      mb_ping->beam_angle[i] = theta;
				      if (phi < 0.0)
				      	phi += 360.0;
				      if (phi > 360.0)
				      	phi -= 360.0;
				      mb_ping->beam_angle_forward[i] = phi;
/*fprintf(stderr,"MBCOPY: i:%d angles: %f %f\n",i,mb_ping->beam_angle[i],mb_ping->beam_angle_forward[i]);*/
				      }
				  else
				      {
				      mb_ping->depth[i] = 0.0;
				      mb_ping->across_track[i] = 0.0;
				      mb_ping->along_track[i] = 0.0;
				      mb_ping->travel_time[i] = 0.0;
				      mb_ping->beam_angle[i] = 0.0;
				      mb_ping->beam_angle_forward[i] = 0;
				      }
				  }
			  for (i=0;i<istore->beams_amp;i++)
				  {
				  mb_ping->mr_amplitude[i] = istore->amp[i];
				  }

			  /* set scale factor for bathymetry */
			  mbsys_gsf_getscale(verbose, istore->bath, istore->beamflag, istore->beams_bath, 
					16, MB_NO, 
					&mb_ping->scaleFactors.scaleTable[0].multiplier, 
					&mb_ping->scaleFactors.scaleTable[0].offset,
					error);
			  mbsys_gsf_getscale(verbose, istore->bath_acrosstrack, istore->beamflag, istore->beams_bath, 
					16, MB_YES, 
					&mb_ping->scaleFactors.scaleTable[1].multiplier, 
					&mb_ping->scaleFactors.scaleTable[1].offset,
					error);
			  mbsys_gsf_getscale(verbose, istore->bath_alongtrack, istore->beamflag, istore->beams_bath, 
					16, MB_YES, 
					&mb_ping->scaleFactors.scaleTable[2].multiplier, 
					&mb_ping->scaleFactors.scaleTable[2].offset,
					error);

			  /* set travel time scale assume two full byte for a 
			  	two way 120 m travel path */
			  mb_ping->scaleFactors.scaleTable[3].multiplier = 65535/0.160;
			  mb_ping->scaleFactors.scaleTable[3].offset = 0;

			  /* assume angle degrees are stored in hundreds of a degree */
			  mb_ping->scaleFactors.scaleTable[4].multiplier  = 100.; 
			  mb_ping->scaleFactors.scaleTable[4].offset = 0;
			  mb_ping->scaleFactors.scaleTable[17].multiplier = 50.; 
			  mb_ping->scaleFactors.scaleTable[17].offset = 0;

			  mb_ping->scaleFactors.numArraySubrecords = 6;	

			  /* choose gain factor -it's a guess based on dataset 
			  	regression analysis!! rcc */
			  gain_correction = 2.2*(istore->gain & 63) + 6*istore->power;

			  /* read amplitude values into storage arrays */
			  if (mb_ping->mc_amplitude != NULL)
			  	{
				for (i=0;i<istore->beams_amp;i++)
				  	{
				 	/* note - we are storing 1/2 db increments */
					mb_ping->mc_amplitude[i] = 40*log10(istore->intensity[i]);
				  	}

				/* set scale factor for mc_amplitude */
				mbsys_gsf_getscale(verbose, mb_ping->mc_amplitude, istore->beamflag, istore->beams_amp, 
						8, MB_YES, &multiplier, &offset, error);
		    		mb_ping->scaleFactors.scaleTable[GSF_SWATH_BATHY_SUBRECORD_MEAN_CAL_AMPLITUDE_ARRAY-1].multiplier
						= multiplier;
				mb_ping->scaleFactors.scaleTable[GSF_SWATH_BATHY_SUBRECORD_MEAN_CAL_AMPLITUDE_ARRAY-1].offset
						= offset;
				mb_ping->scaleFactors.numArraySubrecords ++ ;			
			  	}
			  else if (mb_ping->mr_amplitude != NULL)
			  	{
				for (i=0;i<istore->beams_amp;i++)
					{
					mb_ping->mr_amplitude[i] = 40*log10(istore->intensity[i])- gain_correction;
					}

			       /* Set scale factor for mr _amplitude */
				mbsys_gsf_getscale(verbose, mb_ping->mr_amplitude, istore->beamflag, istore->beams_amp, 
						8, MB_NO, &multiplier, &offset, error);
		    		mb_ping->scaleFactors.scaleTable[GSF_SWATH_BATHY_SUBRECORD_MEAN_REL_AMPLITUDE_ARRAY-1].multiplier
						= multiplier;
				mb_ping->scaleFactors.scaleTable[GSF_SWATH_BATHY_SUBRECORD_MEAN_REL_AMPLITUDE_ARRAY-1].offset
						= offset;	
				mb_ping->scaleFactors.numArraySubrecords  ++ ;		
			  	}

			/* generate imagery from sidescan trace 
				code here would have to be modified for snippets

	        		mb_ping->brb_inten = (gsfBRBIntensity *) realloc(mb_ping->brb_inten,sizeof(gsfBRBIntensity));

				gsfBRBIntensity *brb;
				brb = mb_ping->brb_inten;					  
				brb->bits_per_sample = (unsigned char) 16;
				brb->applied_corrections = 0;
				gsfTimeSeriesIntensity *brb_ts;
				*brb_ts = brb->time_series;
				*brb_ts = (gsfTimeSeriesIntensity*)realloc(*brb_ts,istore->beams_bath*sizeof(gsfTimeSeriesIntensity));	
				*/	 

			/* now fill in the reson 8100 specific fields */
			mb_ping->sensor_id = GSF_SWATH_BATHY_SUBRECORD_RESON_8101_SPECIFIC;
			mb_ping->sensor_data.gsfReson8100Specific.latency = istore->latency ;
			mb_ping->sensor_data.gsfReson8100Specific.ping_number = istore->ping_number ; 
			mb_ping->sensor_data.gsfReson8100Specific.sonar_id = istore->sonar_id ;   
			mb_ping->sensor_data.gsfReson8100Specific.sonar_model = istore->sonar_model ; 
			mb_ping->sensor_data.gsfReson8100Specific.frequency = istore->frequency ;   
			mb_ping->sensor_data.gsfReson8100Specific.surface_velocity = istore->velocity ;
			mb_ping->sensor_data.gsfReson8100Specific.sample_rate = istore->sample_rate ; 
			mb_ping->sensor_data.gsfReson8100Specific.ping_rate = istore->ping_rate ;   
			mb_ping->sensor_data.gsfReson8100Specific.mode = GSF_8100_AMPLITUDE ;
			mb_ping->sensor_data.gsfReson8100Specific.range = istore->range_set ;         
			mb_ping->sensor_data.gsfReson8100Specific.power = istore->power ;         
			mb_ping->sensor_data.gsfReson8100Specific.gain = istore->gain ;	       
			mb_ping->sensor_data.gsfReson8100Specific.pulse_width = istore->pulse_width ;   
			mb_ping->sensor_data.gsfReson8100Specific.tvg_spreading = istore->tvg_spread ; 
			mb_ping->sensor_data.gsfReson8100Specific.tvg_absorption = istore->tvg_absorp ;
			mb_ping->sensor_data.gsfReson8100Specific.fore_aft_bw = istore->projector_beam_width/10.;   
			mb_ping->sensor_data.gsfReson8100Specific.athwart_bw = (double)istore->beam_width_num/(double)istore->beam_width_denom;    
			mb_ping->sensor_data.gsfReson8100Specific.projector_type = istore->projector_type ;
			mb_ping->sensor_data.gsfReson8100Specific.projector_angle = istore->projector_angle ;
			mb_ping->sensor_data.gsfReson8100Specific.range_filt_min = istore->min_range ;
			mb_ping->sensor_data.gsfReson8100Specific.range_filt_max = istore->max_range ;
			mb_ping->sensor_data.gsfReson8100Specific.depth_filt_min = istore->min_depth ;
			mb_ping->sensor_data.gsfReson8100Specific.depth_filt_max = istore-> max_depth;
			mb_ping->sensor_data.gsfReson8100Specific.filters_active = istore->filters_active ;
			mb_ping->sensor_data.gsfReson8100Specific.temperature = istore->temperature ;   
			mb_ping->sensor_data.gsfReson8100Specific.beam_spacing = (double)istore->beam_width_num/(double)istore->beam_width_denom;  
			}

		/* insert comment in structure */
		else if (istore->kind == MB_DATA_COMMENT)
			{
			dataID->recordID = GSF_RECORD_COMMENT;
			if (records->comment.comment_length < strlen(istore->comment) + 1)
			    {
			    if ((records->comment.comment 
					= (char *) 
					    realloc(records->comment.comment,
						strlen(istore->comment)+1))
						    == NULL) 
				{
				status = MB_FAILURE;
				*error = MB_ERROR_MEMORY_FAIL;
				records->comment.comment_length = 0;
				}
			    }
			if ((status = MB_SUCCESS) && (records->comment.comment != NULL))
			    {
			    strcpy(records->comment.comment, istore->comment);
			    records->comment.comment_length = strlen(istore->comment)+1;
			    records->comment.comment_time.tv_sec = (int) istore->png_time_d;
			    records->comment.comment_time.tv_nsec 
				    = (int) (1000000000 
					* (istore->png_time_d 
						- records->comment.comment_time.tv_sec));
			    }
			}
		}

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return value:\n");
		fprintf(stderr,"dbg2       error:      %d\n",*error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:  %d\n",status);
		}

	/* return status */
	return(status);
}
/*--------------------------------------------------------------------*/
