/*--------------------------------------------------------------------
 *    The MB-system:	mbanglecorrect.c	8/13/95
 *    $Id: mbanglecorrect.c,v 4.15 1999-12-29 00:35:11 caress Exp $
 *
 *    Copyright (c) 1995 by 
 *    D. W. Caress (caress@lamont.ldgo.columbia.edu)
 *    and D. N. Chayes (dale@lamont.ldgo.columbia.edu)
 *    Lamont-Doherty Earth Observatory
 *    Palisades, NY  10964
 *
 *    See README file for copying and redistribution conditions.
 *--------------------------------------------------------------------*/
/*
The program mbanglecorrect is used for processing sidescan data.  
This program corrects the sidescan data by dividing by a model of the
backscatter vs grazing angle function to produce a flat image which 
shows geology better than the raw data (each value is also multiplied 
by a scale factor set with the -A option. The backscatter vs grazing 
angle model is either read from a file (global correction) or obtained 
by averaging over the input sidescan data in some number of nearby 
pings (local correction) using the same algorithm as the program 
mbbackangle. If local correction is used, the user specifies the 
angular width of the swath considered and the number of angular bins 
in that swath which are used in calculating the local correction. When 
the correction model is defined using the entire dataset the output 
data will predominantly show large scale variability in seafloor 
reflectivity. When the model used to correct the data is locally 
defined, the output data will tend to show an enhanced view of local 
or fine scale structure, although odd effects may occur at boundaries 
between different types of seafloor. No assumption is made about the 
nature of the data or the sonar used to collect it. If bathymetry 
data is available, then the acrosstrack seafloor slope is taken into 
account when calculating grazing angles, unless the -G option is used 
to force the assumption of a flat seafloor (this pertains only to the 
application of the grazing angle correction; seafloor slopes are always 
used, if available, in calculating local grazing angle correction
functions). The bathymetry can be smoothed prior to the calculation
of slopes, if desired. If bathymetry is not available, the seafloor is 
assumed to be flat with a depth specified by the -Z option. The use 
of bathymetric slopes in the grazing angle calculations will tend to 
remove bathymetric signals from the data. 
The default input and output streams are stdin and stdout.\n";
 *
 * Author:	D. W. Caress
 * Date:	January 12, 1995
 *
 * $Log: not supported by cvs2svn $
 * Revision 4.14  1999/02/04  23:55:08  caress
 * MB-System version 4.6beta7
 *
 * Revision 4.13  1998/12/17  22:50:20  caress
 * MB-System version 4.6beta4
 *
 * Revision 4.12  1998/10/05  19:19:24  caress
 * MB-System version 4.6beta
 *
 * Revision 4.11  1997/07/25  14:28:10  caress
 * Version 4.5beta2
 *
 * Revision 4.10  1997/04/21  17:19:14  caress
 * MB-System 4.5 Beta Release.
 *
 * Revision 4.10  1997/04/17  15:14:38  caress
 * MB-System 4.5 Beta Release
 *
 * Revision 4.9  1996/08/26  17:35:08  caress
 * Release 4.4 revision.
 *
 * Revision 4.8  1996/04/22  13:23:05  caress
 * Now have DTR and MIN/MAX defines in mb_define.h
 *
 * Revision 4.7  1996/03/12  17:27:01  caress
 * Check-in after flail with format 63.
 *
 * Revision 4.6  1995/10/03  13:28:50  caress
 * Fixed major bugs relating to amplitude data.
 *
 * Revision 4.5  1995/08/17  15:04:52  caress
 * Revision for release 4.3.
 *
 * Revision 4.4  1995/05/12  17:12:32  caress
 * Made exit status values consistent with Unix convention.
 * 0: ok  nonzero: error
 *
 * Revision 4.3  1995/03/06  19:37:59  caress
 * Changed include strings.h to string.h for POSIX compliance.
 *
 * Revision 4.2  1995/03/02  13:49:21  caress
 * Fixed bug related to error messages.
 *
 * Revision 4.1  1995/02/22  21:53:14  caress
 * Fixed bug in handling of buffered io.
 *
 * Revision 4.0  1995/02/14  21:17:15  caress
 * Version 4.2
 *
 *
 */

/* standard include files */
#include <stdio.h>
#include <math.h>
#include <string.h>
#include <time.h>

/* mbio include files */
#include "../../include/mb_status.h"
#include "../../include/mb_format.h"
#include "../../include/mb_define.h"
#include "../../include/mb_io.h"

/* mode defines */
#define	MBANGLECORRECT_AMP		1
#define	MBANGLECORRECT_SS		2
#define MBANGLECORRECT_LENGTH_NUMBER	1
#define MBANGLECORRECT_LENGTH_DISTANCE	2

/* MBIO buffer size default */
#define	MBANGLECORRECT_BUFFER_DEFAULT	500

/* ping structure definition */
struct mbanglecorrect_ping_struct 
	{
	int	id;
	int	time_i[7];
	double	time_d;
	double	navlon;
	double	navlat;
	double	speed;
	double	heading;
	double	distance;
	int	beams_bath;
	int	beams_amp;
	int	pixels_ss;
	char	*beamflag;
	double	*bath;
	double	*bathacrosstrack;
	double	*bathalongtrack;
	double	*amp;
	double	*ss;
	double	*ssacrosstrack;
	double	*ssalongtrack;
	int	ndepths;
	double	*depths;
	double	*depthacrosstrack;
	int	nslopes;
	double	*slopes;
	double	*slopeacrosstrack;
	double	*dataprocess;
	};

/*--------------------------------------------------------------------*/

main (argc, argv)
int argc;
char **argv; 
{
	static char rcs_id[] = "$Id: mbanglecorrect.c,v 4.15 1999-12-29 00:35:11 caress Exp $";
	static char program_name[] = "MBANGLECORRECT";
	static char help_message[] =  
"mbanglecorrect is a tool for processing sidescan data.  This program\n\t\
corrects the sidescan data by dividing by a model of the\n\t\
backscatter vs grazing angle function to produce a flat image\n\t\
which shows geology better than the raw data. The backscatter \n\t\
vs grazing angle model is either read from a file or obtained \n\t\
by averaging over the input sidescan data in some number \n\t\
of nearby pings using the same algorithm as the program \n\t\
mbbackangle. When the model used to correct the data is\n\t\
locally defined, the output data will show local (fine scale) \n\t\
structure. When the correction model is defined using the entire\n\t\
dataset the output data will predominantly show large scale\n\t\
variability in seafloor reflectivity.\n\t\
The default input and output streams are stdin and stdout.\n";
	static char usage_message[] = "mbanglecorrect [\
-Akind/scale -Byr/mo/da/hr/mn/sc -C -Dmode/length -Eyr/mo/da/hr/mn/sc \
-Fformat -G -Iinfile -K -Mnsmooth -Nnangles/angle_max -Ooutfile -Rw/e/s/n \
-Scorrectionfile -Zdepth -V -H]";
	extern char *optarg;
	extern int optkind;
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
	int	format;
	int	format_num;
	int	pings;
	int	lonflip;
	double	bounds[4];
	int	btime_i[7];
	int	etime_i[7];
	double	btime_d;
	double	etime_d;
	double	speedmin;
	double	timegap;
	int	beams_bath;
	int	beams_amp;
	int	pixels_ss;
	char	ifile[128];
	char	*imbio_ptr = NULL;

	/* MBIO write control parameters */
	char	ofile[128];
	char	*ombio_ptr = NULL;

	/* mbio read and write values */
	char	*store_ptr;
	int	kind;
	int	nrecord = 0;
	int	nbathdata = 0;
	int	ndata = 0;
	char	comment[256];

	/* buffer handling parameters */
	char	*buff_ptr;
	int	n_buffer_max = MBANGLECORRECT_BUFFER_DEFAULT;
	int	nwant = MBANGLECORRECT_BUFFER_DEFAULT;
	int	nhold = 0;
	int	nhold_ping = 0;
	int	nbuff = 0;
	int	nload;
	int	ndump;
	int	ndumptot = 0;
	int	nexpect;
	struct mbanglecorrect_ping_struct ping[MB_BUFFER_MAX];
	int	first = MB_YES;
	int	start, done;
	int	first_distance;
	double	save_time_d = 0.0;
	int	save_id = 0;

	/* time, user, host variables */
	time_t	right_now;
	char	date[25], user[128], *user_ptr, host[128];

	/* angle function variables */
	int	ampkind = MBANGLECORRECT_SS;
	char	sfile[128];
	int	use_global_statics = MB_NO;
	int	symmetry = MB_YES;
	int	subtract_mode = MB_NO;
	int	nangles = 161;
	double	angle_min = -80.0;
	double	angle_max = 80.0;
	double	angle_start;
	double	dangle;
	int	*nmean = NULL;
	double	*mean = NULL;
	double	*angles = NULL;
	double	*sigma = NULL;
	double	depth_default = 0.0;
	int	use_depth_default = MB_YES;
	int	length_mode = MBANGLECORRECT_LENGTH_DISTANCE;
	double	length_max = 5.0;
	int	length_num = 5;
	double	scale = 1000.0;
	int	require_bath = MB_NO;

	/* slope calculation variables */
	int	ndepths;
	double	*depths;
	double	*depthsmooth;
	double	*depthacrosstrack;
	int	nslopes;
	double	*slopes;
	double	*slopeacrosstrack;
	int	use_slope = MB_YES;
	int	nsmooth = 0;

	FILE	*fp;
	char	buffer[128], tmp[128], *result;
	double	mtodeglon, mtodeglat;
	double	dlon, dlat;
	int	first_set;
	double	bathy;
	double	slope;
	double	angle;
	double	slopeangle;
	double	rawangle;
	double	correction;
	int	ja,  jb,  jbeg,  jend;
	int	i, j, k, ii, jj, kk;

	char	*ctime();
	char	*getenv();

	/* get current default values */
	status = mb_defaults(verbose,&format,&pings,&lonflip,bounds,
		btime_i,etime_i,&speedmin,&timegap);

	/* reset all defaults but the format and lonflip */
	pings = 1;
	bounds[0] = -360.;
	bounds[1] = 360.;
	bounds[2] = -90.;
	bounds[3] = 90.;
	btime_i[0] = 1962;
	btime_i[1] = 2;
	btime_i[2] = 21;
	btime_i[3] = 10;
	btime_i[4] = 30;
	btime_i[5] = 0;
	btime_i[6] = 0;
	etime_i[0] = 2062;
	etime_i[1] = 2;
	etime_i[2] = 21;
	etime_i[3] = 10;
	etime_i[4] = 30;
	etime_i[5] = 0;
	etime_i[6] = 0;
	speedmin = 0.0;
	timegap = 1000000000.0;

	/* set default input and output */
	strcpy (ifile, "stdin");
	strcpy (ofile, "stdout");
	strcpy (sfile, "\0");

	/* process argument list */
	while ((c = getopt(argc, argv, "A:a:B:b:CcD:d:E:e:F:f:GgHhI:i:KkL:l:M:m:N:n:O:o:R:r:S:s:VvXxZ:z:")) != -1)
	  switch (c) 
		{
		case 'A':
		case 'a':
			sscanf (optarg,"%d/%lf", &ampkind, &scale);
			flag++;
			break;
		case 'B':
		case 'b':
			sscanf (optarg,"%d/%d/%d/%d/%d/%d",
				&btime_i[0],&btime_i[1],&btime_i[2],
				&btime_i[3],&btime_i[4],&btime_i[5]);
			btime_i[6] = 0;
			flag++;
			break;
		case 'C':
		case 'c':
			symmetry = MB_NO;
			flag++;
			break;
		case 'D':
		case 'd':
			sscanf (optarg,"%d/%lf", &length_mode, &length_max);
			length_num = (int) length_max;
			flag++;
			break;
		case 'E':
		case 'e':
			sscanf (optarg,"%d/%d/%d/%d/%d/%d",
				&etime_i[0],&etime_i[1],&etime_i[2],
				&etime_i[3],&etime_i[4],&etime_i[5]);
			etime_i[6] = 0;
			flag++;
			break;
		case 'F':
		case 'f':
			sscanf (optarg,"%d", &format);
			flag++;
			break;
		case 'G':
		case 'g':
			use_slope = MB_NO;
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
		case 'K':
		case 'k':
			subtract_mode = MB_YES;
			flag++;
			break;
		case 'L':
		case 'l':
			sscanf (optarg,"%d", &n_buffer_max);
			flag++;
			break;
		case 'M':
		case 'm':
			sscanf (optarg,"%d", &nsmooth);
			flag++;
			break;
		case 'N':
		case 'n':
			sscanf (optarg,"%d/%lf", &nangles, &angle_max);
			angle_min = -angle_max;
			flag++;
			break;
		case 'O':
		case 'o':
			sscanf (optarg,"%s", ofile);
			flag++;
			break;
		case 'R':
		case 'r':
			sscanf (optarg,"%lf/%lf/%lf/%lf", 
				&bounds[0],&bounds[1],&bounds[2],&bounds[3]);
			flag++;
			break;
		case 'S':
		case 's':
			sscanf (optarg,"%s", sfile);
			use_global_statics = MB_YES;
			flag++;
			break;
		case 'V':
		case 'v':
			verbose++;
			flag++;
			break;
		case 'X':
		case 'x':
			require_bath = MB_YES;
			flag++;
			break;
		case 'Z':
		case 'z':
			sscanf (optarg,"%lf", &depth_default);
			flag++;
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
		
	/* check buffer size */
	if (n_buffer_max > MB_BUFFER_MAX)
		n_buffer_max = MBANGLECORRECT_BUFFER_DEFAULT;
	if (length_mode == MBANGLECORRECT_LENGTH_NUMBER
		&& n_buffer_max < 3 * length_num + 1)
		n_buffer_max = MIN(3 * length_num + 1, MB_BUFFER_MAX);

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
		fprintf(stderr,"dbg2       data format:    %d\n",format);
		fprintf(stderr,"dbg2       input file:     %s\n",ifile);
		fprintf(stderr,"dbg2       output file:    %s\n",ofile);
		fprintf(stderr,"dbg2       AGA file:       %s\n",sfile);
		fprintf(stderr,"dbg2       n_buffer_max:   %d\n",n_buffer_max);
		fprintf(stderr,"dbg2       ampkind:        %d\n",ampkind);
		fprintf(stderr,"dbg2       depth_def:      %f\n",depth_default);
		fprintf(stderr,"dbg2       scale:          %f\n",scale);
		fprintf(stderr,"dbg2       length_mode:    %d\n",length_mode);
		fprintf(stderr,"dbg2       length_max:     %f\n",length_max);
		fprintf(stderr,"dbg2       use_slope:      %d\n",use_slope);
		fprintf(stderr,"dbg2       nsmooth:        %d\n",nsmooth);
		}

	/* if help desired then print it and exit */
	if (help)
		{
		fprintf(stderr,"\n%s\n",help_message);
		fprintf(stderr,"\nusage: %s\n", usage_message);
		exit(error);
		}

	/* check for format with amplitude or sidescan data */
	status = mb_format(verbose,&format,&format_num,&error);
	if (ampkind == MBANGLECORRECT_SS 
		&& pixels_ss_table[format_num] <= 0)
		{
		fprintf(stderr,"\nProgram <%s> requires sidescan data.\n",program_name);
		fprintf(stderr,"Format %d is unacceptable because it does not inlude sidescan data.\n",format);
		fprintf(stderr,"\nProgram <%s> Terminated\n",
			program_name);
		exit(MB_ERROR_BAD_FORMAT);
		}
	if (ampkind == MBANGLECORRECT_AMP 
		&& beams_amp_table[format_num] <= 0)
		{
		fprintf(stderr,"\nProgram <%s> requires amplitude data.\n",program_name);
		fprintf(stderr,"Format %d is unacceptable because it does not inlude amplitude data.\n",format);
		fprintf(stderr,"\nProgram <%s> Terminated\n",
			program_name);
		error = MB_ERROR_BAD_FORMAT;
		exit(error);
		}

	/* initialize reading the input swath sonar file */
	if ((status = mb_read_init(
		verbose,ifile,format,pings,lonflip,bounds,
		btime_i,etime_i,speedmin,timegap,
		&imbio_ptr,&btime_d,&etime_d,
		&beams_bath,&beams_amp,&pixels_ss,&error)) != MB_SUCCESS)
		{
		mb_error(verbose,error,&message);
		fprintf(stderr,"\nMBIO Error returned from function <mb_read_init>:\n%s\n",message);
		fprintf(stderr,"\nMultibeam File <%s> not initialized for reading\n",ifile);
		fprintf(stderr,"\nProgram <%s> Terminated\n",
			program_name);
		exit(error);
		}

	/* initialize writing the output swath sonar file */
	if ((status = mb_write_init(
		verbose,ofile,format,&ombio_ptr,
		&beams_bath,&beams_amp,&pixels_ss,&error)) != MB_SUCCESS)
		{
		mb_error(verbose,error,&message);
		fprintf(stderr,"\nMBIO Error returned from function <mb_write_init>:\n%s\n",message);
		fprintf(stderr,"\nMultibeam File <%s> not initialized for writing\n",ofile);
		fprintf(stderr,"\nProgram <%s> Terminated\n",
			program_name);
		exit(error);
		}

	/* allocate memory for data arrays */
	for (i=0;i<n_buffer_max;i++)
		{
		ping[i].beamflag = NULL;
		ping[i].bath = NULL;
		ping[i].amp = NULL;
		ping[i].bathacrosstrack = NULL;
		ping[i].bathalongtrack = NULL;
		ping[i].ss = NULL;
		ping[i].ssacrosstrack = NULL;
		ping[i].ssalongtrack = NULL;
		ping[i].dataprocess = NULL;
		status = mb_malloc(verbose,beams_bath*sizeof(char),
			&ping[i].beamflag,&error);
		status = mb_malloc(verbose,beams_bath*sizeof(double),
			&ping[i].bath,&error);
		status = mb_malloc(verbose,beams_amp*sizeof(double),
			&ping[i].amp,&error);
		status = mb_malloc(verbose,beams_bath*sizeof(double),
			&ping[i].bathacrosstrack,&error);
		status = mb_malloc(verbose,beams_bath*sizeof(double),
			&ping[i].bathalongtrack,&error);
		status = mb_malloc(verbose,pixels_ss*sizeof(double),
			&ping[i].ss,&error);
		status = mb_malloc(verbose,pixels_ss*sizeof(double),
			&ping[i].ssacrosstrack,&error);
		status = mb_malloc(verbose,pixels_ss*sizeof(double),
			&ping[i].ssalongtrack,&error);
		status = mb_malloc(verbose,beams_bath*sizeof(double),
			&ping[i].depths,&error);
		status = mb_malloc(verbose,beams_bath*sizeof(double),
			&ping[i].depthacrosstrack,&error);
		status = mb_malloc(verbose,(beams_bath+1)*sizeof(double),
			&ping[i].slopes,&error);
		status = mb_malloc(verbose,(beams_bath+1)*sizeof(double),
			&ping[i].slopeacrosstrack,&error);
		if (ampkind == MBANGLECORRECT_SS)
			status = mb_malloc(verbose,pixels_ss*sizeof(double),
				&ping[i].dataprocess,&error);
		else
			status = mb_malloc(verbose,beams_amp*sizeof(double),
				&ping[i].dataprocess,&error);
		status = mb_malloc(verbose,beams_bath*sizeof(double),
			&depthsmooth,&error);
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

	/* if specified get static angle sidescan corrections */
	if (use_global_statics == MB_YES)
		{
		/* count static corrections */
		if ((fp = fopen(sfile, "r")) == NULL) 
			{
			error = MB_ERROR_OPEN_FAIL;
			fprintf(stderr,"\nUnable to Open Static Angle Sidescan Correction File <%s> for reading\n",sfile);
			fprintf(stderr,"\nProgram <%s> Terminated\n",
				program_name);
			exit(error);
			}
		nangles = 0;
		while ((result = fgets(buffer,128,fp)) == buffer)
			{
			if (buffer[0] != '#')
				{
				nangles++;
				}
			}
		fclose(fp);

		/* allocate memory */
		if (error == MB_ERROR_NO_ERROR)
			status = mb_malloc(verbose,nangles*sizeof(int),
				&nmean,&error);
		if (error == MB_ERROR_NO_ERROR)
			status = mb_malloc(verbose,nangles*sizeof(double),
				&mean,&error);
		if (error == MB_ERROR_NO_ERROR)
			status = mb_malloc(verbose,nangles*sizeof(double),
				&angles,&error);
		if (error == MB_ERROR_NO_ERROR)
			status = mb_malloc(verbose,nangles*sizeof(double),
				&sigma,&error);
		if (error != MB_ERROR_NO_ERROR)
			{
			mb_error(verbose,error,&message);
			fprintf(stderr,"\nMBIO Error allocating data arrays:\n%s\n",message);
			fprintf(stderr,"\nProgram <%s> Terminated\n",
				program_name);
			exit(error);
			}
			
		/* read in static corrections */
		if ((fp = fopen(sfile, "r")) == NULL) 
			{
			error = MB_ERROR_OPEN_FAIL;
			fprintf(stderr,"\nUnable to Open Static Angle Sidescan Correction File <%s> for reading\n",sfile);
			fprintf(stderr,"\nProgram <%s> Terminated\n",
				program_name);
			exit(error);
			}
		nangles = 0;
		while ((result = fgets(buffer,128,fp)) == buffer)
			{
			if (buffer[0] != '#')
				{
				sscanf(buffer,"%lf %lf",&angles[nangles],
					&mean[nangles]);
				nmean[nangles] = 1;
				sigma[nangles] = 0.0;
				nangles++;
				}
			}
		angle_min = angles[0];
		angle_max = angles[nangles-1];
		fclose(fp);

		/* print out the results */
/*		fprintf(stdout, "\nABA Function for ping %d:\n", j);
		for (i=0;i<nangles;i++)
			{
			if (nmean[i] > 0)
				{
				fprintf(stdout,"%f %f %d %f\n",
				angles[i],mean[i],nmean[i], sigma[i]);
				}
			}
*/
		}

	/* set up for local calculation of AGA function */
	if (use_global_statics == MB_NO)
		{
		/* allocate memory for angle arrays */
		if (error == MB_ERROR_NO_ERROR)
			status = mb_malloc(verbose,nangles*sizeof(int),
				&nmean,&error);
		if (error == MB_ERROR_NO_ERROR)
			status = mb_malloc(verbose,nangles*sizeof(double),
				&mean,&error);
		if (error == MB_ERROR_NO_ERROR)
			status = mb_malloc(verbose,nangles*sizeof(double),
				&angles,&error);
		if (error == MB_ERROR_NO_ERROR)
			status = mb_malloc(verbose,nangles*sizeof(double),
				&sigma,&error);

		/* if error initializing memory then quit */
		if (error != MB_ERROR_NO_ERROR)
			{
			mb_error(verbose,error,&message);
			fprintf(stderr,"\nMBIO Error allocating data arrays:\n%s\n",message);
			fprintf(stderr,"\nProgram <%s> Terminated\n",
				program_name);
			exit(error);
			}

		/* get size of bins */
		dangle = (angle_max - angle_min)/(nangles-1);
		angle_start = angle_min - 0.5*dangle;

		/* initialize binning angles */
		for (i=0;i<nangles;i++)
			{
			angles[i] = angle_min + i*dangle;
			}
		}

	/* output some information */
	if (verbose > 0)
		{
		fprintf(stderr, "\nInput file:            %s\n", ifile);
		fprintf(stderr, "Output file:           %s\n", ofile);
		if (use_global_statics == MB_YES)
			{
			fprintf(stderr, "AGA function file:     %s\n", sfile);
			fprintf(stderr, "Using global correction...\n");
			}
		else
			{
			fprintf(stderr, "Using local correction...\n");
			if (length_mode == MBANGLECORRECT_LENGTH_NUMBER)
				fprintf(stderr, "Correction uses %d pings fore and aft...\n", length_num);
			else
				fprintf(stderr, "Correction uses pings within %f km along track...\n", length_max);
			if (use_slope)
				fprintf(stderr, "Correction uses flat seafloor assumption...\n");
			}
		fprintf(stderr, "Number of angle bins:  %d\n", nangles);
		fprintf(stderr, "Minimum angle:         %f\n", angle_min);
		fprintf(stderr, "Maximum angle:         %f\n", angle_max);
		fprintf(stderr, "Default depth:         %f\n", depth_default);
		fprintf(stderr, "Scaling factor:        %f\n", scale);
		if (subtract_mode == MB_YES)
			fprintf(stderr, "Correcting by subtraction (appropriate for log or dB data)...\n");
		else
			fprintf(stderr, "Correcting by division (appropriate for linear data)...\n");
		if (ampkind == MBANGLECORRECT_AMP)
			fprintf(stderr, "Working on beam amplitude data...\n");
		else
			fprintf(stderr, "Working on sidescan data...\n");
		if (require_bath == MB_YES && ampkind == MBANGLECORRECT_SS)
			fprintf(stderr, "Using sidescan only where good bathymetry is available...\n");
		}

	/* write comments to beginning of output file */
	kind = MB_DATA_COMMENT;
	sprintf(comment,"Sidescan data altered by program %s",
		program_name);
	status = mb_put_comment(verbose,ombio_ptr,comment,&error);
	sprintf(comment,"Version %s",rcs_id);
	status = mb_put_comment(verbose,ombio_ptr,comment,&error);
	sprintf(comment,"MB-system Version %s",MB_VERSION);
	status = mb_put_comment(verbose,ombio_ptr,comment,&error);
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
	sprintf(comment,"Run by user <%s> on cpu <%s> at <%s>",
		user,host,date);
	status = mb_put_comment(verbose,ombio_ptr,comment,&error);
	if (ampkind == MBANGLECORRECT_AMP
		&& subtract_mode == MB_YES)
		{
		sprintf(comment,"Beam amplitude values corrected by subtracting");
		status = mb_put_comment(verbose,ombio_ptr,comment,&error);
		}
	else if (ampkind == MBANGLECORRECT_AMP
		&& subtract_mode == MB_NO)
		{
		sprintf(comment,"Beam amplitude values corrected by dividing by");
		status = mb_put_comment(verbose,ombio_ptr,comment,&error);
		}
	else if (ampkind == MBANGLECORRECT_SS
		&& subtract_mode == MB_YES)
		{
		sprintf(comment,"Sidescan values corrected by subtracting");
		status = mb_put_comment(verbose,ombio_ptr,comment,&error);
		}
	else
		{
		sprintf(comment,"Sidescan values corrected by dividing by");
		status = mb_put_comment(verbose,ombio_ptr,comment,&error);
		}
	if (use_global_statics == MB_NO)
		{
		sprintf(comment,"  a locally defined function of grazing angle.");
		status = mb_put_comment(verbose,ombio_ptr,comment,&error);
		}
	else
		{
		sprintf(comment,"  a user supplied function of grazing angle.");
		status = mb_put_comment(verbose,ombio_ptr,comment,&error);
		}
	sprintf(comment,"Control Parameters:");
	status = mb_put_comment(verbose,ombio_ptr,comment,&error);
	sprintf(comment,"  MBIO data format:    %d",format);
	status = mb_put_comment(verbose,ombio_ptr,comment,&error);
	sprintf(comment,"  Input file:          %s",ifile);
	status = mb_put_comment(verbose,ombio_ptr,comment,&error);
	sprintf(comment,"  Output file:         %s",ofile);
	status = mb_put_comment(verbose,ombio_ptr,comment,&error);
	sprintf(comment,"  Longitude flip:      %d",lonflip);
	status = mb_put_comment(verbose,ombio_ptr,comment,&error);
	sprintf(comment,"  Data kind:           %d",ampkind);
	status = mb_put_comment(verbose,ombio_ptr,comment,&error);
	sprintf(comment,"  Default depth:       %f",depth_default);
	status = mb_put_comment(verbose,ombio_ptr,comment,&error);
	sprintf(comment,"  Scaling factor:      %f",scale);
	status = mb_put_comment(verbose,ombio_ptr,comment,&error);
	sprintf(comment,"  Smoothing dimension: %d",nsmooth);
	status = mb_put_comment(verbose,ombio_ptr,comment,&error);
	sprintf(comment,"  Length mode:         %d",length_mode);
	status = mb_put_comment(verbose,ombio_ptr,comment,&error);
	sprintf(comment,"  Length max:          %f",length_max);
	status = mb_put_comment(verbose,ombio_ptr,comment,&error);
	if (use_global_statics == MB_YES)
		{
		sprintf(comment,"  Static angle correction file: %s",sfile);
		status = mb_put_comment(verbose,ombio_ptr,comment,&error);
		sprintf(comment,"  Static sidescan corrections:");
		status = mb_put_comment(verbose,ombio_ptr,comment,&error);
		for (i=0;i<nangles;i++)
			{
			sprintf(comment,"    %f  %f",angles[i],mean[i]);
			status = mb_put_comment(verbose,ombio_ptr,
						comment,&error);
			}
		}
	sprintf(comment," ");
	status = mb_put_comment(verbose,ombio_ptr,comment,&error);

	/* initialize the buffer */
	status = mb_buffer_init(verbose,&buff_ptr,&error);
	
	/* set handling of buffer */
	if (use_global_statics == MB_YES)
		nhold_ping = 0;
	else if (length_mode == MBANGLECORRECT_LENGTH_NUMBER)
		nhold_ping = length_num;
	else
		nhold_ping = 0;


	/* read and write */
	done = MB_NO;
	first = MB_YES;
	nwant = n_buffer_max;
	if (verbose == 1) fprintf(stderr,"\n");
	while (!done)
		{
		/* load some data into the buffer */
		error = MB_ERROR_NO_ERROR;
		nexpect = nwant - nbuff;
		status = mb_buffer_load(verbose,buff_ptr,imbio_ptr,nwant,
				&nload,&nbuff,&error);
		nrecord += nload;

		/* give the statistics */
		if (verbose > 1) fprintf(stderr,"\n");
		if (verbose >= 1)
			{
			fprintf(stderr,"%d records loaded into buffer\n",nload);
			}

		/* check for done */
		if (nload < nexpect)
			{
			done = MB_YES;
			}

		/* extract data into ping arrays */
		ndata = 0;
		start = 0;
		jbeg = 0;
		jend = 0;
		first_distance = MB_YES;
		status = MB_SUCCESS;
		while (status == MB_SUCCESS
			&& ndata < n_buffer_max)
			{
			status = mb_buffer_get_next_data(verbose,
				buff_ptr,imbio_ptr,start,&ping[ndata].id,
				ping[ndata].time_i,&ping[ndata].time_d,
				&ping[ndata].navlon,&ping[ndata].navlat,
				&ping[ndata].speed,&ping[ndata].heading,
				&ping[ndata].beams_bath,
				&ping[ndata].beams_amp,
				&ping[ndata].pixels_ss,
				ping[ndata].beamflag,
				ping[ndata].bath,
				ping[ndata].amp,
				ping[ndata].bathacrosstrack,
				ping[ndata].bathalongtrack,
				ping[ndata].ss,
				ping[ndata].ssacrosstrack,
				ping[ndata].ssalongtrack,
				&error);

			/* if desired zero amplitude/sidescan where the bathymetry is lacking */
			if (require_bath == MB_YES 
				&& ampkind == MBANGLECORRECT_SS
				&& status == MB_SUCCESS 
				&& ping[ndata].beams_bath > 0)
				{
				check_ss_for_bath(verbose-5, 
					ping[ndata].beams_bath,
					ping[ndata].beamflag,					
					ping[ndata].bath,
					ping[ndata].bathacrosstrack,
					ping[ndata].pixels_ss,
					ping[ndata].ss,
					ping[ndata].ssacrosstrack,
					&error);
				}

			/* get the seafloor slopes */
			if (status == MB_SUCCESS 
				&& ping[ndata].beams_bath > 0)
				{
				set_bathyslope(verbose, 
					nsmooth, 
					ping[ndata].beams_bath,
					ping[ndata].beamflag,
					ping[ndata].bath,
					ping[ndata].bathacrosstrack,
					&ping[ndata].ndepths,
					ping[ndata].depths,
					ping[ndata].depthacrosstrack,
					&ping[ndata].nslopes,
					ping[ndata].slopes,
					ping[ndata].slopeacrosstrack,
					depthsmooth,
					&error);
				}

			if (status == MB_SUCCESS && first_distance == MB_YES)
				{
				first_distance = MB_NO;
				ping[ndata].distance = 0.0;
				mb_coor_scale(verbose-5,ping[ndata].navlat,
					&mtodeglon,&mtodeglat);
				}
			else if (status == MB_SUCCESS)
				{
				dlon = (ping[ndata].navlon 
					- ping[ndata-1].navlon)/mtodeglon;
				dlat = (ping[ndata].navlat 
					- ping[ndata-1].navlat)/mtodeglat;
				ping[ndata].distance = ping[ndata-1].distance 
					+ 0.001*sqrt(dlon*dlon + dlat*dlat);
				}
			if (status == MB_SUCCESS && first != MB_YES)
				{
				if (save_id == ping[ndata].id + ndumptot)
				    jbeg = ndata + 1;
				}
			if (status == MB_SUCCESS)
				{
				start = ping[ndata].id + 1;
				ndata++;
				}
			}
		if (first == MB_YES)
			jbeg = 0;
		if (done == MB_YES)
			jend = ndata - 1;
		else if (ndata > nhold_ping + jbeg)
			{
			jend = ndata - 1 - nhold_ping;
			save_time_d = ping[jend].time_d;
			save_id = ping[jend].id + ndumptot;
			}
		else
			{
			jend = ndata - 1;
			save_time_d = ping[jend].time_d;
			save_id = ping[jend].id + ndumptot;
			}
		if (ndata > 0)
		    nbathdata += (jend - jbeg + 1);
		if (verbose >= 1)
			{
			fprintf(stderr,"%d survey records being processed\n",
				(jend - jbeg + 1));
			}
		if (first == MB_YES && nbathdata > 0)
			first = MB_NO;

		/* loop over all of the pings and beams */
		for (j=jbeg;j<=jend;j++)
		  {
		  /* set beginning and end of pings use for 
			AGA (amplitude vs grazing angle) function */
		  if (use_global_statics == MB_YES)
		    {
		    ja = j;
		    jb = j;
		    }
		  else if (length_mode == MBANGLECORRECT_LENGTH_NUMBER)
		    {
		    ja = j - length_num;
		    jb = j + length_num;
		    }
		  else
		    {
		    first_set = MB_NO;
		    ja = 0;
		    jb = ndata - 1;
		    for (jj=0;jj<ndata;jj++)
		      {
		      if ((ping[j].distance - ping[jj].distance) 
			<= length_max && first_set == MB_NO)
			{
			first_set = MB_YES;
			ja = jj;
			}
		      if ((ping[jj].distance - ping[j].distance) 
			<= length_max)
			{
			jb = jj;
			}
		      }
		    }
		  if (ja < 0) ja = 0;
		  if (jb >= ndata) jb = ndata - 1;

		  /* get local AGA function if not 
		     using global function */
		  if (use_global_statics == MB_NO)
		    {
		    /* initialize AGA function */
		    for (i=0;i<nangles;i++)
		      {
		      nmean[i] = 0;
		      mean[i] = 0.0;
		      sigma[i] = 0.0;
		      }

		    /* loop over the local pings to get AGA function */
		    for (jj=ja;jj<=jb;jj++)
		      {

		      /* do the amplitude */
		      if (ampkind == MBANGLECORRECT_AMP)
		      for (i=0;i<ping[jj].beams_amp;i++)
			{
			if (ping[jj].amp[i] > 0.0)
			    {
			    if (ping[jj].ndepths > 1)
				{
				status = get_bathyslope(verbose,
				    ping[jj].ndepths,
				    ping[jj].depths,
				    ping[jj].depthacrosstrack,
				    ping[jj].nslopes,
				    ping[jj].slopes,
				    ping[jj].slopeacrosstrack,
				    ping[jj].bathacrosstrack[i],
				    &bathy,&slope,&error);
				if (status != MB_SUCCESS)
				    {
				    bathy = depth_default;
				    slope = 0.0;
				    status = MB_SUCCESS;
				    error = MB_ERROR_NO_ERROR;
				    }
				}
			    else
				{
				bathy = depth_default;
				slope = 0.0;
				}
			    if (bathy > 0.0)
				{
				angle = RTD*
				    (atan(ping[jj].bathacrosstrack[i]
				    /bathy) + atan(slope));
				k = (angle - angle_start)/dangle;
				if (k >= 0 && k < nangles)
				    {
				    mean[k] += ping[jj].amp[i];
				    sigma[k] += ping[jj].amp[i]*
					    ping[jj].amp[i];
				    nmean[k]++;
				    }
				}
			    }
			}

		      /* do the sidescan */
		      if (ampkind == MBANGLECORRECT_SS)
		      for (i=0;i<ping[jj].pixels_ss;i++)
			{
			if (ping[jj].ss[i] > 0.0)
			    {
			    if (ping[jj].ndepths > 1)
				{
				status = get_bathyslope(verbose,
				    ping[jj].ndepths,
				    ping[jj].depths,
				    ping[jj].depthacrosstrack,
				    ping[jj].nslopes,
				    ping[jj].slopes,
				    ping[jj].slopeacrosstrack,
				    ping[jj].ssacrosstrack[i],
				    &bathy,&slope,&error);
				if (status != MB_SUCCESS)
				    {
				    bathy = depth_default;
				    slope = 0.0;
				    status = MB_SUCCESS;
				    error = MB_ERROR_NO_ERROR;
				    }
				}
			    else
				{
				bathy = depth_default;
				slope = 0.0;
				}
			    if (bathy > 0.0)
				{
				rawangle = RTD*
				    atan(ping[jj].ssacrosstrack[i]
				    /bathy);
				slopeangle = RTD*atan(slope);
				angle = rawangle + slopeangle;
				k = (angle - angle_start)/dangle;
				if (k >= 0 && k < nangles)
				    {
				    mean[k] += ping[jj].ss[i];
				    sigma[k] += ping[jj].ss[i]*
					    ping[jj].ss[i];
				    nmean[k]++;
				    }
				}
			    }
			}
		      }

		    /* process sums */
		    if (symmetry == MB_NO)
		      {
		      for (i=0;i<nangles;i++)
			{
			if (nmean[i] > 0)
			  {
			  mean[i] = mean[i]/nmean[i];
			  sigma[i] = sqrt(sigma[i]/nmean[i] 
				- mean[i]*mean[i]);
			  }
			else
			  {
			  mean[i] = 0.0;
			  sigma[i] = 0.0;
			  }
			}
		      }
		    else
		      {
		      k = nangles/2;
		      if (fmod((double)nangles, 2) > 0.0)
		      k++;
		      for (i=0;i<k;i++)
			{
			kk = nangles - i - 1;
			if (nmean[i] + nmean[kk] > 0)
			  {
			  mean[i] = mean[i] + mean[kk];
			  nmean[i] = nmean[i] + nmean[kk];
			  sigma[i] = sigma[i] + sigma[kk];
			  mean[i] = mean[i]/nmean[i];
			  sigma[i] = sqrt(sigma[i]/nmean[i] 
				- mean[i]*mean[i]);
			  mean[kk] = mean[i];
			  nmean[kk] = nmean[i];
			  sigma[kk] = sigma[i];
			  }
			else
			  {
			  mean[i] = 0.0;
			  sigma[i] = 0.0;
			  mean[kk] = mean[i];
			  sigma[kk] = sigma[i];
			  }
			}
		      }

		    /* print out the results */
/*		    fprintf(stdout, "\nABA Function for ping %d:\n", j);
		    for (i=0;i<nangles;i++)
		      {
		      if (nmean[i] > 0)
			{
			fprintf(stdout,"%f %f %d %f\n",
				angles[i],mean[i],nmean[i], sigma[i]);
			}
		      }
*/

		    } /* end getting local AGA function */

		  /* apply AGA function */

		  /* do the amplitude */
		  if (ampkind == MBANGLECORRECT_AMP)
		    for (i=0;i<ping[j].beams_amp;i++)
			{
			ping[j].dataprocess[i] = 0.0;
			if (ping[j].amp[i] > 0.0)
			    {
			    if (ping[j].ndepths > 0)
				{
				status = get_bathyslope(verbose,
				    ping[j].ndepths,
				    ping[j].depths,
				    ping[j].depthacrosstrack,
				    ping[j].nslopes,
				    ping[j].slopes,
				    ping[j].slopeacrosstrack,
				    ping[j].bathacrosstrack[i],
				    &bathy,&slope,&error);
				if (status != MB_SUCCESS)
				    {
				    bathy = depth_default;
				    slope = 0.0;
				    status = MB_SUCCESS;
				    error = MB_ERROR_NO_ERROR;
				    }
				}
			    else
				{
				bathy = depth_default;
				slope = 0.0;
				}
			    if (use_slope == MB_NO)
				slope = 0.0;
			    if (bathy > 0.0)
				{
				rawangle = RTD*
				    atan(ping[j].bathacrosstrack[i]
				    /bathy);
				slopeangle = RTD*atan(slope);
				angle = rawangle + slopeangle;
				status = get_anglecorr(verbose-5, 
					    nangles, angles, mean, 
					    angle, &correction, &error);
				if (subtract_mode == MB_YES)
				    ping[j].dataprocess[i] = 
					ping[j].amp[i] - correction + scale;
				else
				    ping[j].dataprocess[i] = 
					scale * ping[j].amp[i] / correction;
				if (ping[j].dataprocess[i] < 0.0)
				    ping[j].dataprocess[i] = 0.0;
				}
			    }
			}

		  /* do the sidescan */
		  if (ampkind == MBANGLECORRECT_SS)
		    for (i=0;i<ping[j].pixels_ss;i++)
			{
			ping[j].dataprocess[i] = 0.0;
			if (ping[j].ss[i] > 0.0)
			    {
			    if (ping[j].ndepths > 1)
				{
				status = get_bathyslope(verbose,
				    ping[j].ndepths,
				    ping[j].depths,
				    ping[j].depthacrosstrack,
				    ping[j].nslopes,
				    ping[j].slopes,
				    ping[j].slopeacrosstrack,
				    ping[j].ssacrosstrack[i],
				    &bathy,&slope,&error);
				if (status != MB_SUCCESS)
				    {
				    bathy = depth_default;
				    slope = 0.0;
				    status = MB_SUCCESS;
				    error = MB_ERROR_NO_ERROR;
				    }
				}
			    else
				{
				bathy = depth_default;
				slope = 0.0;
				}
			    if (use_slope == MB_NO)
				slope = 0.0;
				
			    if (bathy > 0.0)
				{
				rawangle = RTD*
				    atan(ping[j].ssacrosstrack[i]
				    /bathy);
				slopeangle = RTD*atan(slope);
				angle = rawangle + slopeangle;
				status = get_anglecorr(verbose-5, 
					nangles, angles, mean, 
					angle, &correction, &error);
				if (subtract_mode == MB_YES)
				    ping[j].dataprocess[i] = 
					ping[j].ss[i] - correction + scale;
				else
				    ping[j].dataprocess[i] = 
					scale * ping[j].ss[i] / correction;
/*fprintf(stderr, "ping:%d pixel:%d slope:%f angle:%f corr:%f ss: %f %f\n", 
j, i, slopeangle, rawangle, correction, ping[j].ss[i], ping[j].dataprocess[i]);*/
				if (ping[j].dataprocess[i] < 0.0)
				    ping[j].dataprocess[i] = 0.0;
				}
			    }
			}

		  /* print debug statements */
		  if (verbose >= 2)
		    {
		    fprintf(stderr,"\ndbg2  Data from ping %d processed\n", j);
		    if (beams_bath > 0)
		      {
		      fprintf(stderr,"dbg2  Bathymetry Data:\n");
		      for (i=0;i<ping[j].beams_bath;i++)
		        fprintf(stderr,"dbg2       %d %f %f %f\n",
			    i, ping[j].bath[i], 
			    ping[j].bathacrosstrack[i], 
			    ping[j].bathalongtrack[i]);
		      }
		    if (beams_amp > 0 && ampkind == MBANGLECORRECT_AMP)
		      {
		      fprintf(stderr,"dbg2  Beam Intensity Data:\n");
		      for (i=0;i<ping[j].beams_bath;i++)
		        fprintf(stderr,"dbg2       %d %f %f %f\n",
			    i, ping[j].dataprocess[i], 
			    ping[j].bathacrosstrack[i], 
			    ping[j].bathalongtrack[i]);
		      }
		    else if (beams_amp > 0)
		      {
		      fprintf(stderr,"dbg2  Beam Intensity Data:\n");
		      for (i=0;i<ping[j].beams_bath;i++)
		        fprintf(stderr,"dbg2       %d %f %f %f\n",
			    i, ping[j].amp[i], 
			    ping[j].bathacrosstrack[i], 
			    ping[j].bathalongtrack[i]);
		      }
		    if (pixels_ss > 0 && ampkind == MBANGLECORRECT_SS)
		      {
		      fprintf(stderr,"dbg2  Sidescan Data:\n");
		      for (i=0;i<ping[j].pixels_ss;i++)
		        fprintf(stderr,"dbg2       %d %f %f %f\n",
			    i, ping[j].dataprocess[i], 
			    ping[j].ssacrosstrack[i], 
			    ping[j].ssalongtrack[i]);
		      }
		    else if (pixels_ss > 0)
		      {
		      fprintf(stderr,"dbg2  Sidescan Data:\n");
		      for (i=0;i<ping[j].pixels_ss;i++)
		        fprintf(stderr,"dbg2       %d %f %f %f\n",
			    i, ping[j].ss[i], 
			    ping[j].ssacrosstrack[i], 
			    ping[j].ssalongtrack[i]);
		      }
		    }


		  }

		/* find number of pings to hold */
		if (done == MB_YES)
			nhold = 0;
		else if (nhold_ping == 0 || jend <= jbeg)
			nhold = 0;
		else if (nhold_ping < jend)
			nhold = nbuff - ping[jend+1-nhold_ping].id;
		else if (ndata > 0)
			{
			nhold = nbuff - ping[0].id + 1;
			if (nhold > nbuff / 2)
				nhold = nbuff / 2;
			}
		else
			nhold = 0;

		/* reset pings to be dumped */
		if (ndata > 0)
		for (j=0;j<ndata-nhold;j++)
		  {
		  if (ampkind == MBANGLECORRECT_SS)
			for (i=0;i<ping[j].pixels_ss;i++)
				{
				ping[j].ss[i] = ping[j].dataprocess[i];
				}
		  else if (ampkind == MBANGLECORRECT_AMP)
			for (i=0;i<ping[j].beams_amp;i++)
				ping[j].amp[i] = ping[j].dataprocess[i];
		  status = mb_buffer_insert(verbose,
				buff_ptr,imbio_ptr,ping[j].id,
				ping[j].time_i,ping[j].time_d,
				ping[j].navlon,ping[j].navlat,
				ping[j].speed,ping[j].heading,
				ping[j].beams_bath,
				ping[j].beams_amp,
				ping[j].pixels_ss,
				ping[j].beamflag,
				ping[j].bath,
				ping[j].amp,
				ping[j].bathacrosstrack,
				ping[j].bathalongtrack,
				ping[j].ss,
				ping[j].ssacrosstrack,
				ping[j].ssalongtrack,
				comment,&error);
		  }

		/* save processed data held in buffer */
		if (ndata > 0)
		for (j=0;j<=jend-(ndata-nhold);j++)
		  {
		  jj = ndata - nhold + j;
		  if (ampkind == MBANGLECORRECT_SS)
			for (i=0;i<ping[jj].pixels_ss;i++)
				{
				ping[j].dataprocess[i] = ping[jj].dataprocess[i];
				}
		  else if (ampkind == MBANGLECORRECT_AMP)
			for (i=0;i<ping[jj].beams_amp;i++)
				ping[j].dataprocess[i] = ping[jj].dataprocess[i];
		  }

		/* dump data from the buffer */
		ndump = 0;
		if (nbuff > 0)
			{
			status = mb_buffer_dump(verbose,buff_ptr,ombio_ptr,
				nhold,&ndump,&nbuff,&error);
			ndumptot += ndump;
			}

		/* give the statistics */
		if (verbose >= 1)
			{
			fprintf(stderr,"%d records dumped from buffer\n",ndump);
			}
		}

	/* close the files */
	status = mb_buffer_close(verbose,&buff_ptr,imbio_ptr,&error);
	status = mb_close(verbose,&imbio_ptr,&error);
	status = mb_close(verbose,&ombio_ptr,&error);

	/* free the memory */
	for (j=0;j<n_buffer_max;j++)
		{
		mb_free(verbose,&ping[j].beamflag,&error); 
		mb_free(verbose,&ping[j].bath,&error); 
		mb_free(verbose,&ping[j].bathacrosstrack,&error); 
		mb_free(verbose,&ping[j].bathalongtrack,&error); 
		mb_free(verbose,&ping[j].amp,&error); 
		mb_free(verbose,&ping[j].ss,&error); 
		mb_free(verbose,&ping[j].ssacrosstrack,&error); 
		mb_free(verbose,&ping[j].ssalongtrack,&error); 
		mb_free(verbose,&ping[j].depths,&error); 
		mb_free(verbose,&ping[j].depthacrosstrack,&error); 
		mb_free(verbose,&ping[j].slopes,&error); 
		mb_free(verbose,&ping[j].slopeacrosstrack,&error); 
		mb_free(verbose,&ping[j].dataprocess,&error); 
		}
	mb_free(verbose,&depthsmooth,&error); 
	mb_free(verbose,&nmean,&error); 
	mb_free(verbose,&mean,&error); 
	mb_free(verbose,&angles,&error); 
	mb_free(verbose,&nmean,&error); 
	mb_free(verbose,&sigma,&error); 

	/* check memory */
	if (verbose >= 4)
		status = mb_memory_list(verbose,&error);

	/* give the statistics */
	if (verbose >= 1)
		{
		fprintf(stderr,"\n%d data records read and written\n",nrecord);
		fprintf(stderr,"%d survey data records processed\n",nbathdata);
		}

	/* end it all */
	exit(error);
}
/*--------------------------------------------------------------------*/
int check_ss_for_bath(verbose,
	nbath,beamflag,bath,bathacrosstrack,
	nss,ss,ssacrosstrack, 
	error)
int	verbose;
int	nbath;
char	*beamflag;
double	*bath;
double	*bathacrosstrack;
int	nss;
double	*ss;
double	*ssacrosstrack;
int	*error;
{
	char	*function_name = "check_ss_for_bath";
	int	status = MB_SUCCESS;
	int	ifirst, ilast;
	int	iss, ibath;
	int	i, j;
	
	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBANGLECORRECT function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:         %d\n",verbose);
		fprintf(stderr,"dbg2       nbath:           %d\n",nbath);
		fprintf(stderr,"dbg2       bath:            %d\n",bath);
		fprintf(stderr,"dbg2       bathacrosstrack: %d\n",
			bathacrosstrack);
		fprintf(stderr,"dbg2       bath:\n");
		for (i=0;i<nbath;i++)
			fprintf(stderr,"dbg2         %d %f %f\n", 
				i, bath[i], bathacrosstrack[i]);
		}
		
	/* find limits of good bathy */
	ifirst = -1;
	ilast = -1;
	i = 0;
	for (i=0;i<nbath;i++)
	    {
	    if (mb_beam_ok(beamflag[i]))
		{
		if (ifirst < 0)
		    ifirst = i;
		ilast = i;
		}
	    }
		
	/* loop over sidescan looking for bathy on either side
	   - zero sidescan if bathy lacking */
	if (ifirst < ilast)
	    {
	    ibath = ifirst;
	    for (iss=0;iss<nss;iss++)
		{
		/* make sure ibath sets right interval for ss */
		while (ibath < ilast - 1
		    && (!mb_beam_ok(beamflag[ibath])
			|| !mb_beam_ok(beamflag[ibath+1])
			|| (mb_beam_ok(beamflag[ibath+1])
			    && ssacrosstrack[iss] > bathacrosstrack[ibath+1])))
		    ibath++;
/*fprintf(stderr,"iss:%d ibath:%d %f %f  %f %f  ss: %f %f\n",
iss,ibath,bath[ibath],bath[ibath+1],
bathacrosstrack[ibath],bathacrosstrack[ibath+1],
ss[iss],ssacrosstrack[iss]);*/
		
		/* now zero sidescan if not surrounded by good bathy */
		if (!mb_beam_ok(beamflag[ibath]) || !mb_beam_ok(beamflag[ibath+1]))
		    ss[iss] = 0.0;
		else if (ssacrosstrack[iss] < bathacrosstrack[ibath])
		    ss[iss] = 0.0;
		else if (ssacrosstrack[iss] > bathacrosstrack[ibath+1])
		    ss[iss] = 0.0;
		}
	    }
	
	/* else if no good bathy zero all sidescan */
	else
	    {
	    for (iss=0;iss<nss;iss++)
		{
		ss[iss] = 0.0;
		}		
	    }

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBANGLECORRECT function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       error:           %d\n",*error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:          %d\n",status);
		}

	/* return status */
	return(status);
}
/*--------------------------------------------------------------------*/
int set_bathyslope(verbose,
	nsmooth, 
	nbath,beamflag,bath,bathacrosstrack,
	ndepths,depths,depthacrosstrack, 
	nslopes,slopes,slopeacrosstrack, 
	depthsmooth, 
	error)
int	verbose;
int	nsmooth;
int	nbath;
char	*beamflag;
double	*bath;
double	*bathacrosstrack;
int	*ndepths;
double	*depths;
double	*depthacrosstrack;
int	*nslopes;
double	*slopes;
double	*slopeacrosstrack;
double	*depthsmooth;
int	*error;
{
	char	*function_name = "set_bathyslope";
	int	status = MB_SUCCESS;
	int	first, next, last;
	int	nbathgood;
	double	depthsum;
	double	dacrosstrack;
	int	j1, j2;
	int	i, j;
	
	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBANGLECORRECT function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:         %d\n",verbose);
		fprintf(stderr,"dbg2       nbath:           %d\n",nbath);
		fprintf(stderr,"dbg2       bath:            %d\n",bath);
		fprintf(stderr,"dbg2       bathacrosstrack: %d\n",
			bathacrosstrack);
		fprintf(stderr,"dbg2       bath:\n");
		for (i=0;i<nbath;i++)
			fprintf(stderr,"dbg2         %d  %d  %f %f\n", 
				i, beamflag[i], bath[i], bathacrosstrack[i]);
		}

	/* initialize depths */
	*ndepths = 0;
	for (i=0;i<nbath;i++)
		{
		depths[i] = 0.0;
		depthacrosstrack[i] = 0.0;
		}

	/* first fill in the existing depths */
	first = -1;
	last = -1;
	nbathgood = 0;
	for (i=0;i<nbath;i++)
		{
		if (mb_beam_ok(beamflag[i]))
			{
			if (first == -1)
				{
				first = i;
				}
			last = i;
			depths[i] = bath[i];
			depthacrosstrack[i] = bathacrosstrack[i];
			nbathgood++;
			}
		}

	/* now interpolate the depths */
	if (nbathgood > 0)
	for (i=first;i<last;i++)
		{
		if (mb_beam_ok(beamflag[i]))
			{
			next = i;
			j = i + 1;
			while (next == i && j < nbath)
				{
				if (mb_beam_ok(beamflag[j]))
					next = j;
				else
					j++;
				}
			if (next > i)
				{
				for (j=i+1;j<next;j++)
					{
					depths[j] = bath[i] + 
						(bath[next] - bath[i])
						*(j-i)/(next-1);
					depthacrosstrack[j] = bathacrosstrack[i] + 
						(bathacrosstrack[next] - bathacrosstrack[i])
						*(j-i)/(next-1);
					}
				}
			}
		}

	/* now smooth the depths */
	if (nbathgood > 0 && nsmooth > 0)
		{
		for (i=first;i<=last;i++)
			{
			j1 = i - nsmooth;
			j2 = i + nsmooth;
			if (j1 < first)
				j1 = first;
			if (j2 > last)
				j2 = last;
			depthsum = 0.0;
			for (j=j1;j<=j2;j++)
				{
				depthsum += depths[j];
				}
			if (depthsum > 0.0)
				depthsmooth[i] = depthsum/(j2-j1+1);
			else
				depthsmooth[i] = depths[i];
			}
		for (i=first;i<=last;i++)
			depths[i] = depthsmooth[i];
		}

	/* now extrapolate the depths at the ends of the swath */
	if (nbathgood > 0)
		{
		*ndepths = nbath;
		if (last - first > 0)
			dacrosstrack = 
				(depthacrosstrack[last] 
				- depthacrosstrack[first]) 
				/ (last - first);
		else 
			dacrosstrack = 1.0;
		for (i=0;i<first;i++)
			{
			depths[i] = depths[first];
			depthacrosstrack[i] = depthacrosstrack[first] 
				+ dacrosstrack * (i - first);
			}
		for (i=last+1;i<nbath;i++)
			{
			depths[i] = depths[last];
			depthacrosstrack[i] = depthacrosstrack[last] 
				+ dacrosstrack * (i - last);
			}
		}

	/* now calculate slopes */
	if (nbathgood > 0)
		{
		*nslopes = nbath + 1;
		for (i=0;i<nbath-1;i++)
			{
			slopes[i+1] = (depths[i+1] - depths[i])
				/(depthacrosstrack[i+1] - depthacrosstrack[i]);
			slopeacrosstrack[i+1] = 0.5*(depthacrosstrack[i+1] 
				+ depthacrosstrack[i]);
			}
		slopes[0] = 0.0;
		slopeacrosstrack[0] = depthacrosstrack[0];
		slopes[nbath] = 0.0;
		slopeacrosstrack[nbath] = depthacrosstrack[nbath-1];
		}

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBANGLECORRECT function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       ndepths:         %d\n",
			*ndepths);
		fprintf(stderr,"dbg2       depths:\n");
		for (i=0;i<nbath;i++)
			fprintf(stderr,"dbg2         %d %f %f\n", 
				i, depths[i], depthacrosstrack[i]);
		fprintf(stderr,"dbg2       nslopes:         %d\n",
			*nslopes);
		fprintf(stderr,"dbg2       slopes:\n");
		for (i=0;i<*nslopes;i++)
			fprintf(stderr,"dbg2         %d %f %f\n", 
				i, slopes[i], slopeacrosstrack[i]);
		fprintf(stderr,"dbg2       error:           %d\n",*error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:          %d\n",status);
		}

	/* return status */
	return(status);
}
/*--------------------------------------------------------------------*/
int get_bathyslope(verbose,
	ndepths,depths,depthacrosstrack,
	nslopes,slopes,slopeacrosstrack, 
	acrosstrack,depth,slope,error)
int	verbose;
int	ndepths;
double	*depths;
double	*depthacrosstrack;
int	nslopes;
double	*slopes;
double	*slopeacrosstrack;
double	acrosstrack;
double	*depth;
double	*slope;
int	*error;
{
	char	*function_name = "get_bathyslope";
	int	status = MB_SUCCESS;
	int	found_depth, found_slope;
	int	idepth, islope;
	int	i;
	
	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBANGLECORRECT function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:         %d\n",verbose);
		fprintf(stderr,"dbg2       ndepths:         %d\n",
			ndepths);
		fprintf(stderr,"dbg2       depths:\n");
		for (i=0;i<ndepths;i++)
			fprintf(stderr,"dbg2         %d %f %f\n", 
				i, depths[i], depthacrosstrack[i]);
		fprintf(stderr,"dbg2       nslopes:         %d\n",
			nslopes);
		fprintf(stderr,"dbg2       slopes:\n");
		for (i=0;i<nslopes;i++)
			fprintf(stderr,"dbg2         %d %f %f\n", 
				i, slopes[i], slopeacrosstrack[i]);
		fprintf(stderr,"dbg2       acrosstrack:     %f\n",acrosstrack);
		}

	/* check if acrosstrack is in defined interval */
	found_depth = MB_NO;
	found_slope = MB_NO;
	if (ndepths > 1)
	    {
	    
	    if (acrosstrack < depthacrosstrack[0])
		{
		*depth = depths[0];
		*slope = 0.0;
		found_depth = MB_YES;
		found_slope = MB_YES;
		}

	    else if (acrosstrack > depthacrosstrack[ndepths-1])
		{
		*depth = depths[ndepths-1];
		*slope = 0.0;
		found_depth = MB_YES;
		found_slope = MB_YES;
		}
    
	    else if (acrosstrack >= depthacrosstrack[0]
		    && acrosstrack <= depthacrosstrack[ndepths-1])
		{
    
		/* look for depth */
		idepth = -1;
		while (found_depth == MB_NO && idepth < ndepths - 2)
		    {
		    idepth++;
		    if (acrosstrack >= depthacrosstrack[idepth]
			&& acrosstrack <= depthacrosstrack[idepth+1])
			{
			*depth = depths[idepth] 
				+ (acrosstrack - depthacrosstrack[idepth])
				/(depthacrosstrack[idepth+1] 
				- depthacrosstrack[idepth])
				*(depths[idepth+1] - depths[idepth]);
			found_depth = MB_YES;
			*error = MB_ERROR_NO_ERROR;
			}
		    }
    
		/* look for slope */
		islope = -1;
		while (found_slope == MB_NO && islope < nslopes - 2)
		    {
		    islope++;
		    if (acrosstrack >= slopeacrosstrack[islope]
			&& acrosstrack <= slopeacrosstrack[islope+1])
			{
			*slope = slopes[islope] 
				+ (acrosstrack - slopeacrosstrack[islope])
				/(slopeacrosstrack[islope+1] 
				- slopeacrosstrack[islope])
				*(slopes[islope+1] - slopes[islope]);
			found_slope = MB_YES;
			*error = MB_ERROR_NO_ERROR;
			}
		    }
		}
	    }

	/* check for failure */
	if (found_depth != MB_YES || found_slope != MB_YES)
		{
		status = MB_FAILURE;
		*error = MB_ERROR_OTHER;
		*depth = 0.0;
		*slope = 0.0;
		}

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBANGLECORRECT function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       depth:           %f\n",*depth);
		fprintf(stderr,"dbg2       slope:           %f\n",*slope);
		fprintf(stderr,"dbg2       error:           %d\n",*error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:          %d\n",status);
		}

	/* return status */
	return(status);
}
/*--------------------------------------------------------------------*/
int get_anglecorr(verbose,nangle,angles,corrs,angle,
		corr,error)
int	verbose;
int	nangle;
double	*angles;
double	*corrs;
double	angle;
double	*corr;
int	*error;
{
	char	*function_name = "get_anglecorr";
	int	status = MB_SUCCESS;
	int	iroll, found;
	int	i, j;
	

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBANGLECORRECT function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       nangle:      %d\n",nangle);
		fprintf(stderr,"dbg2       angles:      %d\n",angles);
		fprintf(stderr,"dbg2       corrs:       %d\n",corrs);
		fprintf(stderr,"dbg2       angle:       %f\n",angle);
		}

	/* search for the specified angle */
	found = MB_NO;
	for (i=0;i<nangle-1;i++)
		if (angle >= angles[i] && angle <= angles[i+1])
			{
			found = MB_YES;
			iroll = i;
			}

	/* set the correction */
	if (found == MB_YES)
		{
		*corr = corrs[iroll] 
			+ (corrs[iroll+1] - corrs[iroll])
			*(angle - angles[iroll])
			/(angles[iroll+1] - angles[iroll]);
		}
	else if (angle < angles[0])
		*corr = corrs[0];
	else if (angle > angles[nangle-1])
		*corr = corrs[nangle-1];
	else
		*corr = 0.0;

	/* assume success */
	*error = MB_ERROR_NO_ERROR;
	status = MB_SUCCESS;

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBANGLECORRECT function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       corr:            %f\n",*corr);
		fprintf(stderr,"dbg2       error:           %d\n",*error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:          %d\n",status);
		}

	/* return status */
	return(status);
}
/*--------------------------------------------------------------------*/
