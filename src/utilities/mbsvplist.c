/*--------------------------------------------------------------------
 *    The MB-system:	mbsvplist.c	1/3/2001
 *    $Id$
 *
 *    Copyright (c) 2001-2013 by
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
 * This program, mbsvplist, lists all water sound velocity
 * profiles (SVPs) within swath data files. Swath bathymetry is
 * calculated from raw angles and travel times by raytracing
 * through a model of the speed of sound in water. Many swath
 * data formats allow SVPs to be embedded in the data, and
 * often the SVPs used to calculate the data will be included.
 * By default, all unique SVPs encountered are listed to
 * stdout. The SVPs may instead be written to individual files
 * with names FILE_XXX.svp, where FILE is the swath data
 * filename and XXX is the SVP count within the file.  The -D
 * option causes duplicate SVPs to be output. The -P option
 * implies -O, and also causes the parameter file to be modified
 * so that the first svp output for each file becomes the
 * svp used for recalculating bathymetry for that swath file.
 *
 * Author:	D. W. Caress
 * Date:	January 3,  2001
 *
 * $Log: mbsvplist.c,v $
 * Revision 5.10  2008/09/20 00:57:41  caress
 * Release 5.1.1beta23
 *
 * Revision 5.9  2006/09/11 18:55:54  caress
 * Changes during Western Flyer and Thomas Thompson cruises, August-September
 * 2006.
 *
 * Revision 5.8  2006/01/18 15:17:00  caress
 * Added stdlib.h include.
 *
 * Revision 5.7  2005/11/05 01:07:54  caress
 * Programs changed to register arrays through mb_register_array() rather than allocating the memory directly with mb_realloc() or mb_malloc().
 *
 * Revision 5.6  2005/03/25 04:42:59  caress
 * Standardized the string lengths used for filenames and comment data.
 *
 * Revision 5.5  2004/10/06 19:10:53  caress
 * Release 5.0.5 update.
 *
 * Revision 5.4  2003/07/02 18:14:19  caress
 * Release 5.0.0
 *
 * Revision 5.3  2003/04/17 21:18:57  caress
 * Release 5.0.beta30
 *
 * Revision 5.2  2001/07/20 00:34:38  caress
 * Release 5.0.beta03
 *
 * Revision 5.1  2001/03/22 21:15:49  caress
 * Trying to make release 5.0.beta0.
 *
 * Revision 5.0  2001/01/04  21:43:50  caress
 * Initial revision.
 *
 *
 */


/* standard include files */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <math.h>
#include <string.h>
#include <time.h>

/* MBIO include files */
#include "mb_status.h"
#include "mb_format.h"
#include "mb_define.h"
#include "mb_process.h"

/* system function declarations */
char	*ctime();
char	*getenv();

/* local defines */
#define	MBSVPLIST_SVP_NUM_ALLOC		24
#define	MBSVPLIST_PRINTMODE_CHANGE	0
#define	MBSVPLIST_PRINTMODE_UNIQUE	1
#define	MBSVPLIST_PRINTMODE_ALL		2

struct mbsvplist_svp_struct {
	int	time_set;		/* time stamp known */
	int	position_set;		/* position known */
	int	repeat_in_file;		/* repeats a previous svp in the same file */
	int	match_last;		/* repeats the last svp in the same file or the previous file */
	int	depthzero_reset;	/* uppermost SVP value set to zero depth */
	double	time_d;
	double	longitude;
	double	latitude;
	double	depthzero;
	int	n;
	double	depth[MB_SVP_MAX];
	double	velocity[MB_SVP_MAX];
};

static char rcs_id[] = "$Id$";

/*--------------------------------------------------------------------*/

int main (int argc, char **argv)
{
	char program_name[] = "mbsvplist";
	char help_message[] =  "mbsvplist lists all water sound velocity\nprofiles (SVPs) within swath data files. Swath bathymetry is\ncalculated from raw angles and travel times by raytracing\nthrough a model of the speed of sound in water. Many swath\ndata formats allow SVPs to be embedded in the data, and\noften the SVPs used to calculate the data will be included.\nBy default, all unique SVPs encountered are listed to\nstdout. The SVPs may instead be written to individual files\nwith names FILE_XXX.svp, where FILE is the swath data\nfilename and XXX is the SVP count within the file. The -D\noption causes duplicate SVPs to be output.\nThe -T option will output a CSV table of svp#, time, longitude, latitude and number of points for SVPs.\nWhen the -Nmin_num_pairs option is used, only svps that have at least min_num_pairs svp values will be output.(This is particularly useful for .xse data where the svp is entered as a single values svp.)";
	char usage_message[] = "mbsvplist [-C -D -Fformat -H -Ifile -Mmode -O -Nmin_num_pairs -P -T -V -Z]";
	extern char *optarg;
	int	errflg = 0;
	int	c;
	int	help = 0;
	int	flag = 0;

	/* MBIO status variables */
	int	status = MB_SUCCESS;
	int	verbose = 0;
	int	error = MB_ERROR_NO_ERROR;
	char	*message;

	/* MBIO read control parameters */
	int	read_datalist = MB_NO;
	char	read_file[MB_PATH_MAXLINE];
	void	*datalist;
	int	look_processed = MB_DATALIST_LOOK_UNSET;
	double	file_weight;
	int	format;
	int	pings;
	int	lonflip;
	double	bounds[4];
	int	btime_i[7];
	int	etime_i[7];
	double	btime_d;
	double	etime_d;
	double	speedmin;
	double	timegap;
	char	file[MB_PATH_MAXLINE];
	int	beams_bath;
	int	beams_amp;
	int	pixels_ss;

	/* MBIO read values */
	void	*mbio_ptr = NULL;
	void	*store_ptr;
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
	char	*beamflag = NULL;
	double	*bath = NULL;
	double	*bathacrosstrack = NULL;
	double	*bathalongtrack = NULL;
	double	*amp = NULL;
	double	*ss = NULL;
	double	*ssacrosstrack = NULL;
	double	*ssalongtrack = NULL;
	char	comment[MB_COMMENT_MAXLINE];
	
	/* save time stamp and position of last survey data */
	double	last_time_d = 0.0;
	double	last_navlon = 0.0;
	double	last_navlat = 0.0;

	/* data record source types */
	int	nav_source;
	int	heading_source;
	int	vru_source;
	int	svp_source;

	/* output mode settings */
	int	svp_printmode;
	int	svp_force_zero;
	int	svp_file_output;
	int output_as_table = MB_NO;

	/* SVP values */
	int	svp_match_last = MB_NO;
	int	svp_loaded = MB_NO;
	int	svp_setprocess;
	int	svp_save_count;
	struct mbsvplist_svp_struct	svp;
	struct mbsvplist_svp_struct	svp_last;
	int				svp_save_alloc = 0;
	struct mbsvplist_svp_struct	*svp_save = NULL;
	char	svp_file[MB_PATH_MAXLINE];
	FILE	*svp_fp;
	int	svp_read;
	int	svp_read_tot;
	int	svp_written;
	int	svp_written_tot;
	int	svp_repeat_in_file;
	int	svp_unique;
	int	svp_unique_tot;
	int	output_counts = MB_NO;
	int	out_cnt = 0;
	int	min_num_pairs = 0;
	int	svp_time_i[7];
	
	/* ttimes values */
	int	ssv_output = MB_NO;
	int	nbeams;
	double	*ttimes = NULL;
	double	*angles = NULL;
	double	*angles_forward = NULL;
	double	*angles_null = NULL;
	double	*heave = NULL;
	double	*alongtrack_offset = NULL;
	double	ssv;

	time_t	right_now;
	char	date[25], user[MB_PATH_MAXLINE], *user_ptr, host[MB_PATH_MAXLINE];
	int	read_data;
	int	i, j, isvp;

	/* get current default values */
	status = mb_defaults(verbose,&format,&pings,&lonflip,bounds,
		btime_i,etime_i,&speedmin,&timegap);
	pings = 1;
	bounds[0] = -360.0;
	bounds[1] = 360.0;
	bounds[2] = -90.0;
	bounds[3] = 90.0;
	svp_printmode = MBSVPLIST_PRINTMODE_CHANGE;
	svp_file_output = MB_NO;
	svp_setprocess = MB_NO;
	svp_force_zero = MB_NO;
	ssv_output = MB_NO;
	svp_read_tot = 0;
	svp_written_tot = 0;
	svp_unique_tot = 0;
	svp_last.n = 0;

	/* set default input to datalist.mb-1 */
	strcpy (read_file, "datalist.mb-1");

	/* process argument list */
	while ((c = getopt(argc, argv, "CcDdF:f:I:i:M:m:N:n:OoPpSsTtZzVvHh")) != -1)
	  switch (c)
		{
		case 'H':
		case 'h':
			help++;
			break;
		case 'V':
		case 'v':
			verbose++;
			break;
		case 'D':
		case 'd':
			svp_printmode = MBSVPLIST_PRINTMODE_ALL;
			break;
		case 'C':
		case 'c':
			output_counts = MB_YES;
			ssv_output = MB_NO;
			break;
		case 'F':
		case 'f':
			sscanf (optarg,"%d", &format);
			flag++;
			break;
		case 'I':
		case 'i':
			sscanf (optarg,"%s", read_file);
			flag++;
			break;
		case 'M':
		case 'm':
			sscanf (optarg,"%d", &svp_printmode);
			flag++;
			break;
		case 'N':
		case 'n':
			sscanf (optarg,"%d", &min_num_pairs);
			break;
		case 'O':
		case 'o':
			svp_file_output = MB_YES;
			ssv_output = MB_NO;
			break;
		case 'P':
		case 'p':
			svp_file_output = MB_YES;
			svp_setprocess = MB_YES;
			ssv_output = MB_NO;
			break;
		case 'S':
		case 's':
			ssv_output = MB_YES;
			svp_file_output = MB_NO;
			svp_setprocess = MB_NO;
			break;
		case 'T':
		case 't':
			output_as_table = MB_YES;
			ssv_output = MB_NO;
			break;
		case 'Z':
		case 'z':
			svp_force_zero = MB_YES;
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
		fprintf(stderr,"dbg2       format:         %d\n",format);
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
		fprintf(stderr,"dbg2       file:           %s\n",file);
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
		mb_get_format(verbose,read_file,NULL,&format,&error);

	/* determine whether to read one file or a list of files */
	if (format < 0)
		read_datalist = MB_YES;

	/* open file list */
	if (read_datalist == MB_YES)
	    {
	    if ((status = mb_datalist_open(verbose,&datalist,
					    read_file,look_processed,&error)) != MB_SUCCESS)
		{
		error = MB_ERROR_OPEN_FAIL;
		fprintf(stderr,"\nUnable to open data list file: %s\n",
			read_file);
		fprintf(stderr,"\nProgram <%s> Terminated\n",
			program_name);
		exit(error);
		}
	    if ((status = mb_datalist_read(verbose,datalist,
			    file,&format,&file_weight,&error))
			    == MB_SUCCESS)
		read_data = MB_YES;
	    else
		read_data = MB_NO;
	    }
	/* else copy single filename to be read */
	else
	    {
	    strcpy(file, read_file);
	    read_data = MB_YES;
	    }

	/* loop over all files to be read */
	while (read_data == MB_YES)
	{
	/* check format and get data sources */
	if ((status = mb_format_source(verbose, &format,
			&nav_source, &heading_source,
			&vru_source, &svp_source,
			&error)) == MB_FAILURE)
		{
		mb_error(verbose,error,&message);
		fprintf(stderr,"\nMBIO Error returned from function <mb_format_source>:\n%s\n",message);
		fprintf(stderr,"\nProgram <%s> Terminated\n",
			program_name);
		exit(error);
		}

	/* initialize reading the swath file */
	if ((status = mb_read_init(
		verbose,file,format,pings,lonflip,bounds,
		btime_i,etime_i,speedmin,timegap,
		&mbio_ptr,&btime_d,&etime_d,
		&beams_bath,&beams_amp,&pixels_ss,&error)) != MB_SUCCESS)
		{
		mb_error(verbose,error,&message);
		fprintf(stderr,"\nMBIO Error returned from function <mb_read_init>:\n%s\n",message);
		fprintf(stderr,"\nMultibeam File <%s> not initialized for reading\n",file);
		fprintf(stderr,"\nProgram <%s> Terminated\n",
			program_name);
		exit(error);
		}

	/* allocate memory for data arrays */
	if (error == MB_ERROR_NO_ERROR)
		status = mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_BATHYMETRY,
						sizeof(char), (void **)&beamflag, &error);
	if (error == MB_ERROR_NO_ERROR)
		status = mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_BATHYMETRY,
						sizeof(double), (void **)&bath, &error);
	if (error == MB_ERROR_NO_ERROR)
		status = mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_AMPLITUDE,
						sizeof(double), (void **)&amp, &error);
	if (error == MB_ERROR_NO_ERROR)
		status = mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_BATHYMETRY,
						sizeof(double), (void **)&bathacrosstrack, &error);
	if (error == MB_ERROR_NO_ERROR)
		status = mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_BATHYMETRY,
						sizeof(double), (void **)&bathalongtrack, &error);
	if (error == MB_ERROR_NO_ERROR)
		status = mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_SIDESCAN,
						sizeof(double), (void **)&ss, &error);
	if (error == MB_ERROR_NO_ERROR)
		status = mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_SIDESCAN,
						sizeof(double), (void **)&ssacrosstrack, &error);
	if (error == MB_ERROR_NO_ERROR)
		status = mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_SIDESCAN,
						sizeof(double), (void **)&ssalongtrack, &error);
	if (error == MB_ERROR_NO_ERROR)
		status = mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_BATHYMETRY,
						sizeof(double), (void **)&ttimes, &error);
	if (error == MB_ERROR_NO_ERROR)
		status = mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_BATHYMETRY,
						sizeof(double), (void **)&angles, &error);
	if (error == MB_ERROR_NO_ERROR)
		status = mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_BATHYMETRY,
						sizeof(double), (void **)&angles_forward, &error);
	if (error == MB_ERROR_NO_ERROR)
		status = mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_BATHYMETRY,
						sizeof(double), (void **)&angles_null, &error);
	if (error == MB_ERROR_NO_ERROR)
		status = mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_BATHYMETRY,
						sizeof(double), (void **)&heave, &error);
	if (error == MB_ERROR_NO_ERROR)
		status = mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_BATHYMETRY,
						sizeof(double), (void **)&alongtrack_offset, &error);

	/* if error initializing memory then quit */
	if (error != MB_ERROR_NO_ERROR)
		{
		mb_error(verbose,error,&message);
		fprintf(stderr,"\nMBIO Error allocating data arrays:\n%s\n",
			message);
		fprintf(stderr,"\nProgram <%s> Terminated\n",
			program_name);
		exit(error);
		}

	/* output info */
	if (verbose >= 1)
		{
		if (ssv_output == MB_YES)
			fprintf(stderr, "\nSearching %s for SSV records\n", file);
		else
			fprintf(stderr, "\nSearching %s for SVP records\n", file);
		}

	/* read and print data */
	svp_loaded = MB_NO;
	svp.n = 0;
	svp_save_count = 0;
	svp_read = 0;
	svp_written = 0;
	svp_unique = 0;
	while (error <= MB_ERROR_NO_ERROR)
		{
		/* read a data record */
		status = mb_get_all(verbose,mbio_ptr,&store_ptr,&kind,
			time_i,&time_d,&navlon,&navlat,
			&speed,&heading,
			&distance,&altitude,&sonardepth,
			&beams_bath,&beams_amp,&pixels_ss,
			beamflag,bath,amp,bathacrosstrack,bathalongtrack,
			ss,ssacrosstrack,ssalongtrack,
			comment,&error);

		/* print debug statements */
		if (verbose >= 2)
			{
			fprintf(stderr,"\ndbg2  Ping read in program <%s>\n",
				program_name);
			fprintf(stderr,"dbg2       kind:           %d\n",kind);
			fprintf(stderr,"dbg2       error:          %d\n",error);
			fprintf(stderr,"dbg2       status:         %d\n",status);
			}

		/* if svp then extract data */
		if (error <= MB_ERROR_NO_ERROR
			&& kind == svp_source
			&& svp_source != MB_DATA_NONE)
			{
			/* extract svp */
			status = mb_extract_svp(verbose, mbio_ptr, store_ptr,
						&kind, &svp.n,
						svp.depth, svp.velocity,
						&error);
			if (status == MB_SUCCESS)
				{
				svp_read++;
				svp_loaded = MB_YES;
				svp.match_last = MB_NO;
				svp.repeat_in_file = MB_NO;
				if (last_time_d != 0.0)
					{
					svp.time_set = MB_YES;
					svp.time_d = last_time_d;
					}
				else
					{
					svp.time_set = MB_NO;
					svp.time_d = 0.0;
					}
				if (navlon != 0.0 || navlat != 0.0)
					{
					svp.position_set = MB_YES;
					svp.longitude = navlon;
					svp.latitude = navlat;
					}
				else if (last_navlon != 0.0 || last_navlat != 0.0)
					{
					svp.position_set = MB_YES;
					svp.longitude = last_navlon;
					svp.latitude = last_navlat;
					}
				else
					{
					svp.position_set = MB_NO;
					svp.longitude = 0.0;
					svp.latitude = 0.0;
					}
				svp.depthzero_reset = MB_NO;
				svp.depthzero = 0.0;
				}
			else
				{
				svp_loaded = MB_NO;
				}

			/* force zero depth if requested */
			if (svp_loaded == MB_YES
				&& svp.n > 0
				&& svp_force_zero == MB_YES
				&& svp.depth[0] != 0.0)
				{
				svp.depthzero = svp.depth[0];
				svp.depth[0] = 0.0;
				svp.depthzero_reset = MB_YES;
				}

			/* check if the svp is a duplicate to a previous svp
				in the same file */
			if (svp_loaded == MB_YES)
				{
				svp_match_last = MB_NO;
				for (j=0; j<svp_save_count && svp_match_last == MB_YES; j++)
					{
					if (svp.n == svp_save[j].n
						&& memcmp(svp.depth, svp_save[j].depth, svp.n) ==0
						&& memcmp(svp.velocity, svp_save[j].velocity, svp.n)==0)
						{
						svp_match_last = MB_YES;
						}
					}
				svp.match_last = svp_match_last;
				}

			/* check if the svp is a duplicate to the previous svp
				whether from the same file or a previous file */
			if (svp_loaded == MB_YES)
				{
				/* check if svp is the same as the previous */
				if (svp.n == svp_last.n
					&& memcmp(svp.depth, svp_last.depth, svp.n) ==0
					&& memcmp(svp.velocity, svp_last.velocity, svp.n)==0)
					{
					svp_repeat_in_file = MB_YES;
					}
				else
					{
					svp_repeat_in_file = MB_NO;
					}
				svp.repeat_in_file = svp_repeat_in_file;

				/* save the svp */
				svp_last.time_set = MB_NO;
				svp_last.position_set = MB_NO;
				svp_last.n = svp.n;
				for (i=0;i<svp.n;i++)
					{
					svp_last.depth[i] = svp.depth[i];
					svp_last.velocity[i] = svp.velocity[i];
					}
				}

			/* if the svp is unique so far, save it in memory */
			if (svp_loaded == MB_YES
				&& svp_match_last == MB_NO
				&& svp.n >= min_num_pairs)
				{
				/* allocate memory as needed */
				if (svp_save_count >= svp_save_alloc)
					{
					svp_save_alloc += MBSVPLIST_SVP_NUM_ALLOC;
					status = mb_reallocd(verbose,__FILE__,__LINE__,
								svp_save_alloc * sizeof(struct mbsvplist_svp_struct),
								(void **)&svp_save, &error);
					}

				/* save the svp */
				svp_save[svp_save_count].time_set = svp.time_set;
				svp_save[svp_save_count].position_set = svp.position_set;
				svp_save[svp_save_count].repeat_in_file = svp.repeat_in_file;
				svp_save[svp_save_count].match_last = svp.match_last;
				svp_save[svp_save_count].time_d = svp.time_d;
				svp_save[svp_save_count].longitude = svp.longitude;
				svp_save[svp_save_count].latitude = svp.latitude;
				svp_save[svp_save_count].n = svp.n;
				for (i=0;i<svp.n;i++)
					{
					svp_save[svp_save_count].depth[i] = svp.depth[i];
					svp_save[svp_save_count].velocity[i] = svp.velocity[i];
					}
				svp_save_count++;
				svp_unique++;
				}
			}

		/* else if survey data save most recent ping time
			and if ssv output desired call mb_ttimes() and output ssv */
		else if (error <= MB_ERROR_NO_ERROR
			&& kind == MB_DATA_DATA)
			{
			/* save most recent survey time stamp and position */
			last_time_d = time_d;
			last_navlon = navlon;
			last_navlat = navlat;
			
			/* check if any saved svps need time tags and position */
			if (time_d != 0.0 && (navlon != 0.0 || navlat != 0.0))
				{
				for (isvp=0;isvp<svp_save_count;isvp++)
					{
					if (svp_save[isvp].time_set == MB_NO)
						{
						svp_save[isvp].time_set = MB_YES;
						svp_save[isvp].time_d = time_d;
						}
					if (svp_save[isvp].position_set == MB_NO)
						{
						svp_save[isvp].position_set = MB_YES;
						svp_save[isvp].longitude = navlon;
						svp_save[isvp].latitude = navlat;
						}
					}
				}

			/* if desired output ssv_output */
			if (ssv_output == MB_YES)
				{
				/* extract ttimes */
				status = mb_ttimes(verbose, mbio_ptr, store_ptr,
						&kind, &nbeams,
						ttimes, angles,
						angles_forward, angles_null,
						heave, alongtrack_offset,
						&sonardepth, &ssv, &error);

				/* output ssv */
				if (status == MB_SUCCESS)
					fprintf(stdout, "%f %f\n", sonardepth, ssv);
				}
			}
		}

	/* close the swath file */
	status = mb_close(verbose,&mbio_ptr,&error);
	
	/* output svps from this file if there are any and ssv_output and output_counts are MB_NO */
	if (svp_save_count > 0 && ssv_output == MB_NO && output_counts == MB_NO)
		{
		for (isvp = 0; isvp < svp_save_count; isvp++)
			{
			if (svp_save[isvp].n >= min_num_pairs
				&& ((svp_printmode == MBSVPLIST_PRINTMODE_CHANGE
						&& (svp_written == 0 || svp_save[isvp].repeat_in_file == MB_NO))
					|| (svp_printmode == MBSVPLIST_PRINTMODE_UNIQUE
						&& (svp_save[isvp].match_last == MB_NO))
					|| (svp_printmode == MBSVPLIST_PRINTMODE_ALL)))
				{
				/* set the output */
				if (svp_file_output == MB_YES)
					{
					/* set file name */
					sprintf(svp_file, "%s_%3.3d.svp", file, isvp);

					/* open the file */
					svp_fp = fopen(svp_file, "w");
					}
				else
					svp_fp = stdout;
					
				/* get time as date */
				mb_get_date(verbose, svp_save[isvp].time_d, svp_time_i);

				/* print out the svp */
				if (output_as_table == MB_YES) /* output csv table to stdout */
					{
					if (out_cnt == 0) /* output header records */
						{
						printf("#mbsvplist CSV table output\n#navigation information is approximate\n#SVP_cnt,date_time,longitude,latitude,num_data_points\n");
						}
					out_cnt++;
					printf( "%d,%4.4d/%2.2d/%2.2d %2.2d:%2.2d:%2.2d.%6.6d,%.6f,%.6f,%d\n",out_cnt,
							svp_time_i[0], svp_time_i[1],
							svp_time_i[2], svp_time_i[3],
							svp_time_i[4], svp_time_i[5],
							svp_time_i[6],
							svp_save[isvp].longitude,svp_save[isvp].latitude,
							svp_save[isvp].n);
					}
				else if (svp_fp != NULL)
					{
					/* output info */
					if (verbose >= 1)
					    {
					    fprintf(stderr, "Outputting SVP to file: %s (# svp pairs=%d)\n", svp_file,svp_save[isvp].n);
					    }

					/* write it out */
					fprintf(svp_fp, "## MB-SVP %4.4d/%2.2d/%2.2d %2.2d:%2.2d:%2.2d.%6.6d %.9f %.9f\n",
							    svp_time_i[0], svp_time_i[1],
							    svp_time_i[2], svp_time_i[3],
							    svp_time_i[4], svp_time_i[5],
							    svp_time_i[6],
							    svp_save[isvp].longitude,
							    svp_save[isvp].latitude);
					fprintf(svp_fp, "## Water Sound Velocity Profile (SVP)\n");
					fprintf(svp_fp, "## Output by Program %s\n",program_name);
					fprintf(svp_fp, "## Program Version %s\n",rcs_id);
					fprintf(svp_fp, "## MB-System Version %s\n",MB_VERSION);
					strncpy(date,"\0",25);
					right_now = time((time_t *)0);
					strncpy(date,ctime(&right_now),24);
					if ((user_ptr = getenv("USER")) == NULL)
						user_ptr = getenv("LOGNAME");
					if (user_ptr != NULL)
						strcpy(user,user_ptr);
					else
						strcpy(user, "unknown");
					gethostname(host,MB_PATH_MAXLINE);
					fprintf(svp_fp, "## Run by user <%s> on cpu <%s> at <%s>\n",
						user,host,date);
					fprintf(svp_fp, "## Swath File: %s\n",file);
					fprintf(svp_fp, "## Start Time: %4.4d/%2.2d/%2.2d %2.2d:%2.2d:%2.2d.%6.6d\n",
							    svp_time_i[0], svp_time_i[1],
							    svp_time_i[2], svp_time_i[3],
							    svp_time_i[4], svp_time_i[5],
							    svp_time_i[6]);
					fprintf(svp_fp, "## SVP Longitude: %f\n", svp_save[isvp].longitude);
					fprintf(svp_fp, "## SVP Latitude:  %f\n", svp_save[isvp].latitude);
					fprintf(svp_fp, "## SVP Count: %d\n", svp_save_count);
					if (svp_save[isvp].depthzero_reset == MB_YES)
						{
						fprintf(svp_fp, "## Initial depth reset from %f to 0.0 meters\n", svp_save[isvp].depthzero);
						}
					if (verbose >= 1 && svp_save[isvp].depthzero_reset == MB_YES)
					    {
					    fprintf(stderr, "Initial depth reset from %f to 0.0 meters\n", svp_save[isvp].depthzero);
					    }
					fprintf(svp_fp, "## Number of SVP Points: %d\n",svp_save[isvp].n);
					for (i=0;i<svp_save[isvp].n;i++)
						fprintf(svp_fp, "%8.2f\t%7.2f\n",
							svp_save[isvp].depth[i], svp_save[isvp].velocity[i]);
					if (svp_file_output == MB_NO)
						{
						fprintf(svp_fp, "## \n");
						fprintf(svp_fp, "## \n");
						}
					svp_written++;
					}

				/* close the svp file */
				if (svp_file_output == MB_YES
					&& svp_fp != NULL)
					{
					fclose(svp_fp);

					/* if desired, set first svp output to be used for recalculating
						bathymetry */
					if (svp_setprocess == MB_YES
						&& svp_save_count == 1)
						{
						status = mb_pr_update_svp(verbose, file,
								MB_YES, svp_file, MBP_ANGLES_OK, MB_YES, &error);
						}
					}
				}
			}
		}

	/* update total counts */
	svp_read_tot += svp_read;
	svp_unique_tot += svp_unique;
	svp_written_tot += svp_written;

	/* output info */
	if (verbose >= 1)
		{
		fprintf(stderr, "%d SVP records read\n", svp_read);
		fprintf(stderr, "%d SVP unique records read\n", svp_unique);
		fprintf(stderr, "%d SVP records written\n", svp_written);
		}

	/* figure out whether and what to read next */
        if (read_datalist == MB_YES)
                {
		if ((status = mb_datalist_read(verbose,datalist,
				    file,&format,&file_weight,&error))
				    == MB_SUCCESS)
                        read_data = MB_YES;
                else
                        read_data = MB_NO;
                }
        else
                {
                read_data = MB_NO;
                }

	/* end loop over files in list */
	}
	if (read_datalist == MB_YES)
		mb_datalist_close(verbose,&datalist,&error);

	/* output info */
	if (verbose >= 1)
		{
		fprintf(stderr, "\nTotal %d SVP records read\n", svp_read_tot);
		fprintf(stderr, "Total %d SVP unique records found\n", svp_unique_tot);
		fprintf(stderr, "Total %d SVP records written\n", svp_written_tot);
		}
	if (output_counts == MB_YES)
		fprintf(stdout, "%d\n", svp_unique_tot);

	/* deallocate memory */
	status = mb_freed(verbose,__FILE__,__LINE__,(void **)&svp_save,&error);

	/* check memory */
	if (verbose >= 4)
		status = mb_memory_list(verbose,&error);

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  Program <%s> completed\n",
			program_name);
		fprintf(stderr,"dbg2  Ending status:\n");
		fprintf(stderr,"dbg2       status:  %d\n",status);
		}

	/* end it all */
	exit(error);
}
/*--------------------------------------------------------------------*/
