/*--------------------------------------------------------------------
 *    The MB-system:	mbfilter.c	1/16/95
 *    $Id: mbfilter.c,v 4.1 1995-03-02 13:49:21 caress Exp $
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
 * mbfilter applies one or more simple filters to the specified
 * data (sidescan, beam amplitude, and/or bathymetry). The filters
 * include:
 *   a: boxcar mean filter for smoothing
 *   b: gaussian mean filter for smoothing
 *   c: boxcar median filter for smoothing
 *   d: inverse gradient filter for smoothing
 *   e: edge detection filter for contrast enhancement
 *   f: gradient subtraction filter for contrast enhancement
 * These filters are mostly intended for use with sidescan
 * data, and operate on 3x3 or 5x5 value windows with
 * no accommodation for differences in acrosstrack vs
 * alongtrack sampling.
 * The default input and output streams are stdin and stdout.
 *
 * Author:	D. W. Caress
 * Date:	January 16, 1995
 *
 * $Log: not supported by cvs2svn $
 * Revision 4.0  1995/02/14  21:17:15  caress
 * Version 4.2
 *
 *
 */

/* standard include files */
#include <stdio.h>
#include <math.h>
#include <strings.h>

/* mbio include files */
#include "../../include/mb_status.h"
#include "../../include/mb_format.h"

/* DTR define */
#ifndef M_PI
#define	M_PI	3.14159265358979323846
#endif
#define DTR (M_PI/180.)
#define RTD (180./M_PI)
#define max(A, B)   ((A) > (B) ? (A) : (B))
#define max(A, B)   ((A) > (B) ? (A) : (B))

/* mode defines */
#define	MBFILTER_BATH			0
#define	MBFILTER_AMP			1
#define	MBFILTER_SS			2
#define	MBFILTER_HIPASS_NONE		0
#define	MBFILTER_HIPASS_MEAN		1
#define	MBFILTER_HIPASS_GAUSSIAN	2
#define	MBFILTER_HIPASS_MEDIAN		3
#define	MBFILTER_SMOOTH_NONE		0
#define	MBFILTER_SMOOTH_MEAN		1
#define	MBFILTER_SMOOTH_GAUSSIAN	2
#define	MBFILTER_SMOOTH_MEDIAN		3
#define	MBFILTER_SMOOTH_GRADIENT	4
#define	MBFILTER_CONTRAST_NONE		0
#define	MBFILTER_CONTRAST_EDGE		1
#define	MBFILTER_CONTRAST_GRADIENT	2

/* MBIO buffer structure pointer */
#define	MBFILTER_BUFFER	500
#define	MBFILTER_HOLD	50

/* ping structure definition */
struct mbfilter_ping_struct 
	{
	int	id;
	int	time_i[7];
	double	time_d;
	double	navlon;
	double	navlat;
	double	speed;
	double	heading;
	double	*bath;
	double	*bathacrosstrack;
	double	*bathalongtrack;
	double	*amp;
	double	*ss;
	double	*ssacrosstrack;
	double	*ssalongtrack;
	double	*dataprocess;
	};

/*--------------------------------------------------------------------*/

main (argc, argv)
int argc;
char **argv; 
{
	static char rcs_id[] = "$Id: mbfilter.c,v 4.1 1995-03-02 13:49:21 caress Exp $";
	static char program_name[] = "MBFILTER";
	static char help_message[] =  
"mbfilter applies one or more simple filters to the specified\n\t\
data (sidescan, beam amplitude, and/or bathymetry). The filters\n\t\
include:\n\t\
  - boxcar mean for lo-pass filtering\n\t\
  - gaussian mean for lo-pass filtering\n\t\
  - boxcar median for lo-pass filtering\n\t\
  - inverse gradient for lo-pass filtering\n\t\
  - boxcar mean subtraction for hi-pass filtering\n\t\
  - gaussian mean subtraction for hi-pass filtering\n\t\
  - boxcar median subtraction for hi-pass filtering\n\t\
These filters are mostly intended for use with sidescan\n\t\
data. In particular, the lo-pass or smoothing filters\n\t\
can be used for first-order speckle reduction in sidescan\n\t\
data, and the hi-pass filters can be used to emphasize\n\t\
fine scale structure in the data.\n\t\
The default input and output streams are stdin and stdout.\n";

	static char usage_message[] = "mbfilter [\
-Akind -Byr/mo/da/hr/mn/sc -Dmode/xdim/ldim/iteration/offset \
-Eyr/mo/da/hr/mn/sc -Fformat -Iinfile -Ooutfile \
-Rwest/east/south/north -Smode/xdim/ldim/iteration -V -H]";
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
	int	nwant = MBFILTER_BUFFER;
	int	nhold = MBFILTER_HOLD;
	int	nbuff = 0;
	int	nload;
	int	ndump;
	int	nexpect;
	struct mbfilter_ping_struct ping[MBFILTER_BUFFER];
	int	nping = 0;
	int	nping_start;
	int	nping_end;
	int	first = MB_YES;
	int	start, done;
	int	first_distance;
	double	save_time_d;

	/* time, user, host variables */
	long int	right_now;
	char	date[25], user[128], host[128];

	/* processing control variables */
	int	datakind = MBFILTER_SS;
	int	hipass_mode = MBFILTER_HIPASS_NONE;
	int	hipass_xdim = 10;
	int	hipass_ldim = 3;
	int	hipass_iter = 1;
	double	hipass_offset = 10000.0;
	int	smooth_mode = MBFILTER_SMOOTH_NONE;
	int	smooth_xdim = 3;
	int	smooth_ldim = 3;
	int	smooth_iter = 1;
	int	contrast_mode = MBFILTER_CONTRAST_NONE;
	int	contrast_xdim = 3;
	int	contrast_ldim = 3;
	int	contrast_iter = 1;
	int	nweight;
	int	nweightmax;
	double	*weights;
	double	*values;
	double	*distances;
	int	hipass_ndx, hipass_ndl;
	int	smooth_ndx, smooth_ndl;
	int	contrast_ndx, contrast_ndl;
	int	iteration;

	double	*dataptr0, *dataptr1;
	double	ddis;
	int	ndatapts;
	int	ia,  ib;
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

	/* process argument list */
	while ((c = getopt(argc, argv, "A:a:B:b:C:c:D:d:E:e:F:f:HhI:i:O:o:R:r:S:s:Vv")) != -1)
	  switch (c) 
		{
		case 'A':
		case 'a':
			sscanf (optarg,"%d", &datakind);
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
			sscanf (optarg,"%d/%d/%d/%d",
				&contrast_mode, &contrast_xdim, 
				&contrast_ldim, &contrast_iter);
			flag++;
			break;
		case 'D':
		case 'd':
			sscanf (optarg,"%d/%d/%d/%d/%f",
				&hipass_mode, &hipass_xdim, 
				&hipass_ldim, &hipass_iter, 
				&hipass_offset);
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
		case 'H':
		case 'h':
			help++;
			break;
		case 'I':
		case 'i':
			sscanf (optarg,"%s", ifile);
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
			sscanf (optarg,"%d/%d/%d/%d",
				&smooth_mode, &smooth_xdim, 
				&smooth_ldim, &smooth_iter);
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
		exit(MB_FAILURE);
		}

	/* print starting message */
	if (verbose == 1)
		{
		fprintf(stderr,"\nProgram %s\n",program_name);
		fprintf(stderr,"Version %s\n",rcs_id);
		fprintf(stderr,"MB-system Version %s\n",MB_VERSION);
		}

	/* set mode if not set */
	if (hipass_mode == MBFILTER_HIPASS_NONE
		&& smooth_mode == MBFILTER_SMOOTH_NONE
		&& contrast_mode == MBFILTER_CONTRAST_NONE)
		{
		smooth_mode = MBFILTER_SMOOTH_GAUSSIAN;
		smooth_xdim = 5;
		smooth_ldim = 3;
		smooth_iter = 3;
		}

	/* set data type if not set properly */
	if (datakind != MBFILTER_BATH && datakind != MBFILTER_AMP)
		datakind = MBFILTER_SS;

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
		fprintf(stderr,"dbg2       datakind:       %d\n",datakind);
		fprintf(stderr,"dbg2       hipass_mode:    %d\n",hipass_mode);
		fprintf(stderr,"dbg2       hipass_xdim:    %d\n",hipass_xdim);
		fprintf(stderr,"dbg2       hipass_ldim:    %d\n",hipass_ldim);
		fprintf(stderr,"dbg2       hipass_iter:    %d\n",hipass_iter);
		fprintf(stderr,"dbg2       hipass_offset:  %f\n",hipass_offset);
		fprintf(stderr,"dbg2       smooth_mode:    %d\n",smooth_mode);
		fprintf(stderr,"dbg2       smooth_xdim:    %d\n",smooth_xdim);
		fprintf(stderr,"dbg2       smooth_ldim:    %d\n",smooth_ldim);
		fprintf(stderr,"dbg2       smooth_iter:    %d\n",smooth_iter);
		fprintf(stderr,"dbg2       contrast_mode:    %d\n",contrast_mode);
		fprintf(stderr,"dbg2       contrast_xdim:    %d\n",contrast_xdim);
		fprintf(stderr,"dbg2       contrast_ldim:    %d\n",contrast_ldim);
		fprintf(stderr,"dbg2       contrast_iter:    %d\n",contrast_iter);
		}

	/* if help desired then print it and exit */
	if (help)
		{
		fprintf(stderr,"\n%s\n",help_message);
		fprintf(stderr,"\nusage: %s\n", usage_message);
		exit(MB_ERROR_NO_ERROR);
		}

	/* check for format with amplitude or sidescan data */
	status = mb_format(verbose,&format,&format_num,&error);
	if (datakind == MBFILTER_BATH 
		&& beams_bath_table[format_num] <= 0)
		{
		fprintf(stderr,"\nProgram <%s> requires bathymetry data.\n",program_name);
		fprintf(stderr,"Format %d is unacceptable because it does not inlude sidescan data.\n",format);
		fprintf(stderr,"\nProgram <%s> Terminated\n",
			program_name);
		exit(MB_ERROR_BAD_FORMAT);
		}
	if (datakind == MBFILTER_SS 
		&& pixels_ss_table[format_num] <= 0)
		{
		fprintf(stderr,"\nProgram <%s> requires sidescan data.\n",program_name);
		fprintf(stderr,"Format %d is unacceptable because it does not inlude sidescan data.\n",format);
		fprintf(stderr,"\nProgram <%s> Terminated\n",
			program_name);
		exit(MB_ERROR_BAD_FORMAT);
		}
	if (datakind == MBFILTER_AMP 
		&& beams_amp_table[format_num] <= 0)
		{
		fprintf(stderr,"\nProgram <%s> requires amplitude data.\n",program_name);
		fprintf(stderr,"Format %d is unacceptable because it does not inlude amplitude data.\n",format);
		fprintf(stderr,"\nProgram <%s> Terminated\n",
			program_name);
		exit(MB_ERROR_BAD_FORMAT);
		}

	/* output some information */
	if (verbose > 0)
		{
		if (datakind == MBFILTER_BATH)
			fprintf(stderr, "\nprocessing bathymetry data...\n");
		else if (datakind == MBFILTER_AMP)
			fprintf(stderr, "\nprocessing beam amplitude data...\n");
		else if (datakind == MBFILTER_SS)
			fprintf(stderr, "\nprocessing sidescan data...\n");
		if (hipass_mode == MBFILTER_HIPASS_MEAN)
			fprintf(stderr, "applying mean subtraction filter for hipass\n");
		else if (hipass_mode == MBFILTER_HIPASS_GAUSSIAN)
			fprintf(stderr, "applying gaussian mean subtraction filter for hipass\n");
		else if (hipass_mode == MBFILTER_HIPASS_MEDIAN)
			fprintf(stderr, "applying median subtraction filter for hipass\n");
		if (hipass_mode != MBFILTER_HIPASS_NONE)
			{
			fprintf(stderr, "  filter acrosstrack dimension: %d\n", hipass_xdim);
			fprintf(stderr, "  filter alongtrack dimension:  %d\n", hipass_ldim);
			fprintf(stderr, "  filter iterations:            %d\n", hipass_iter);
			fprintf(stderr, "  filter offset:                %f\n", hipass_offset);
			}
		if (smooth_mode == MBFILTER_SMOOTH_MEAN)
			fprintf(stderr, "applying mean filter for smoothing\n");
		else if (smooth_mode == MBFILTER_SMOOTH_GAUSSIAN)
			fprintf(stderr, "applying gaussian mean filter for smoothing\n");
		else if (smooth_mode == MBFILTER_SMOOTH_MEDIAN)
			fprintf(stderr, "applying median filter for smoothing\n");
		else if (smooth_mode == MBFILTER_SMOOTH_GRADIENT)
			fprintf(stderr, "applying inverse gradient filter for smoothing\n");
		if (smooth_mode != MBFILTER_SMOOTH_NONE)
			{
			fprintf(stderr, "  filter acrosstrack dimension: %d\n", smooth_xdim);
			fprintf(stderr, "  filter alongtrack dimension:  %d\n", smooth_ldim);
			fprintf(stderr, "  filter iterations:            %d\n", smooth_iter);
			}
		if (contrast_mode == MBFILTER_CONTRAST_EDGE)
			fprintf(stderr, "applying edge detection filter for contrast enhancement\n");
		else if (contrast_mode == MBFILTER_CONTRAST_GRADIENT)
			fprintf(stderr, "applying gradient subtraction filter for contrast enhancement\n");
		if (contrast_mode != MBFILTER_CONTRAST_NONE)
			{
			fprintf(stderr, "  filter acrosstrack dimension: %d\n", contrast_xdim);
			fprintf(stderr, "  filter alongtrack dimension:  %d\n", contrast_ldim);
			fprintf(stderr, "  filter iterations:            %d\n", contrast_iter);
			}
		}

	/* initialize reading the input multibeam file */
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

	/* initialize writing the output multibeam file */
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
	for (i=0;i<MBFILTER_BUFFER;i++)
		{
		ping[i].bath = NULL;
		ping[i].amp = NULL;
		ping[i].bathacrosstrack = NULL;
		ping[i].bathalongtrack = NULL;
		ping[i].ss = NULL;
		ping[i].ssacrosstrack = NULL;
		ping[i].ssalongtrack = NULL;
		ping[i].dataprocess = NULL;
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
		if (datakind == MBFILTER_SS)
			status = mb_malloc(verbose,pixels_ss*sizeof(double),
				&ping[i].dataprocess,&error);
		else
			status = mb_malloc(verbose,beams_amp*sizeof(double),
				&ping[i].dataprocess,&error);
		}

	/* allocate memory for weights */
	nweightmax = 2*max(hipass_xdim*hipass_ldim, 
	    max(smooth_xdim*smooth_ldim, contrast_xdim*contrast_ldim) );
	hipass_ndx = hipass_xdim/2;
	hipass_ndl = hipass_ldim/2;
	smooth_ndx = smooth_xdim/2;
	smooth_ndl = smooth_ldim/2;
	contrast_ndx = contrast_xdim/2;
	contrast_ndl = contrast_ldim/2;
	if (error == MB_ERROR_NO_ERROR)
		status = mb_malloc(verbose,nweightmax*sizeof(double),
				&weights,&error);
	if (error == MB_ERROR_NO_ERROR)
		status = mb_malloc(verbose,nweightmax*sizeof(double),
				&values,&error);
	if (error == MB_ERROR_NO_ERROR)
		status = mb_malloc(verbose,nweightmax*sizeof(double),
				&distances,&error);

	/* if error initializing memory then quit */
	if (error != MB_ERROR_NO_ERROR)
		{
		mb_error(verbose,error,&message);
		fprintf(stderr,"\nMBIO Error allocating data arrays:\n%s\n",message);
		fprintf(stderr,"\nProgram <%s> Terminated\n",
				program_name);
		exit(error);
		}

	/* write comments to beginning of output file */
	kind = MB_DATA_COMMENT;
	strncpy(comment,"\0",256);
	sprintf(comment,"Sidescan data altered by program %s",
		program_name);
	status = mb_put(verbose,ombio_ptr,kind,
			ping[0].time_i,ping[0].time_d,
			ping[0].navlon,ping[0].navlat,
			ping[0].speed,ping[0].heading,
			beams_bath,beams_amp,pixels_ss,
			ping[0].bath,ping[0].amp,
			ping[0].bathacrosstrack,ping[0].bathalongtrack,
			ping[0].ss,ping[0].ssacrosstrack,
			ping[0].ssalongtrack,
			comment,&error);
	strncpy(comment,"\0",256);
	sprintf(comment,"Version %s",rcs_id);
	status = mb_put(verbose,ombio_ptr,kind,
			ping[0].time_i,ping[0].time_d,
			ping[0].navlon,ping[0].navlat,
			ping[0].speed,ping[0].heading,
			beams_bath,beams_amp,pixels_ss,
			ping[0].bath,ping[0].amp,
			ping[0].bathacrosstrack,ping[0].bathalongtrack,
			ping[0].ss,ping[0].ssacrosstrack,
			ping[0].ssalongtrack,
			comment,&error);
	strncpy(comment,"\0",256);
	sprintf(comment,"MB-system Version %s",MB_VERSION);
	status = mb_put(verbose,ombio_ptr,kind,
			ping[0].time_i,ping[0].time_d,
			ping[0].navlon,ping[0].navlat,
			ping[0].speed,ping[0].heading,
			beams_bath,beams_amp,pixels_ss,
			ping[0].bath,ping[0].amp,
			ping[0].bathacrosstrack,ping[0].bathalongtrack,
			ping[0].ss,ping[0].ssacrosstrack,
			ping[0].ssalongtrack,
			comment,&error);
	right_now = time((long *)0);
	strncpy(date,"\0",25);
	right_now = time((long *)0);
	strncpy(date,ctime(&right_now),24);
	strcpy(user,getenv("USER"));
	gethostname(host,128);
	strncpy(comment,"\0",256);
	sprintf(comment,"Run by user <%s> on cpu <%s> at <%s>",
		user,host,date);
	status = mb_put(verbose,ombio_ptr,kind,
			ping[0].time_i,ping[0].time_d,
			ping[0].navlon,ping[0].navlat,
			ping[0].speed,ping[0].heading,
			beams_bath,beams_amp,pixels_ss,
			ping[0].bath,ping[0].amp,
			ping[0].bathacrosstrack,ping[0].bathalongtrack,
			ping[0].ss,ping[0].ssacrosstrack,
			ping[0].ssalongtrack,
			comment,&error);
	if (datakind == MBFILTER_BATH)
		{
		strncpy(comment,"\0",256);
		sprintf(comment,"Bathymetry values filtered");
		status = mb_put(verbose,ombio_ptr,kind,
			ping[0].time_i,ping[0].time_d,
			ping[0].navlon,ping[0].navlat,
			ping[0].speed,ping[0].heading,
			beams_bath,beams_amp,pixels_ss,
			ping[0].bath,ping[0].amp,
			ping[0].bathacrosstrack,ping[0].bathalongtrack,
			ping[0].ss,ping[0].ssacrosstrack,
			ping[0].ssalongtrack,
			comment,&error);
		}
	else if (datakind == MBFILTER_AMP)
		{
		strncpy(comment,"\0",256);
		sprintf(comment,"Beam amplitude values filtered");
		status = mb_put(verbose,ombio_ptr,kind,
			ping[0].time_i,ping[0].time_d,
			ping[0].navlon,ping[0].navlat,
			ping[0].speed,ping[0].heading,
			beams_bath,beams_amp,pixels_ss,
			ping[0].bath,ping[0].amp,
			ping[0].bathacrosstrack,ping[0].bathalongtrack,
			ping[0].ss,ping[0].ssacrosstrack,
			ping[0].ssalongtrack,
			comment,&error);
		}
	else
		{
		strncpy(comment,"\0",256);
		sprintf(comment,"Sidescan values filtered");
		status = mb_put(verbose,ombio_ptr,kind,
			ping[0].time_i,ping[0].time_d,
			ping[0].navlon,ping[0].navlat,
			ping[0].speed,ping[0].heading,
			beams_bath,beams_amp,pixels_ss,
			ping[0].bath,ping[0].amp,
			ping[0].bathacrosstrack,ping[0].bathalongtrack,
			ping[0].ss,ping[0].ssacrosstrack,
			ping[0].ssalongtrack,
			comment,&error);
		}
	strncpy(comment,"\0",256);
	sprintf(comment,"  by a locally defined function of grazing angle.");
	status = mb_put(verbose,ombio_ptr,kind,
			ping[0].time_i,ping[0].time_d,
			ping[0].navlon,ping[0].navlat,
			ping[0].speed,ping[0].heading,
			beams_bath,beams_amp,pixels_ss,
			ping[0].bath,ping[0].amp,
			ping[0].bathacrosstrack,ping[0].bathalongtrack,
			ping[0].ss,ping[0].ssacrosstrack,
			ping[0].ssalongtrack,
			comment,&error);
	strncpy(comment,"\0",256);
	sprintf(comment,"Control Parameters:");
	status = mb_put(verbose,ombio_ptr,kind,
			ping[0].time_i,ping[0].time_d,
			ping[0].navlon,ping[0].navlat,
			ping[0].speed,ping[0].heading,
			beams_bath,beams_amp,pixels_ss,
			ping[0].bath,ping[0].amp,
			ping[0].bathacrosstrack,ping[0].bathalongtrack,
			ping[0].ss,ping[0].ssacrosstrack,
			ping[0].ssalongtrack,
			comment,&error);
	strncpy(comment,"\0",256);
	sprintf(comment,"  MBIO data format:   %d",format);
	status = mb_put(verbose,ombio_ptr,kind,
			ping[0].time_i,ping[0].time_d,
			ping[0].navlon,ping[0].navlat,
			ping[0].speed,ping[0].heading,
			beams_bath,beams_amp,pixels_ss,
			ping[0].bath,ping[0].amp,
			ping[0].bathacrosstrack,ping[0].bathalongtrack,
			ping[0].ss,ping[0].ssacrosstrack,
			ping[0].ssalongtrack,
			comment,&error);
	strncpy(comment,"\0",256);
	sprintf(comment,"  Input file:         %s",ifile);
	status = mb_put(verbose,ombio_ptr,kind,
			ping[0].time_i,ping[0].time_d,
			ping[0].navlon,ping[0].navlat,
			ping[0].speed,ping[0].heading,
			beams_bath,beams_amp,pixels_ss,
			ping[0].bath,ping[0].amp,
			ping[0].bathacrosstrack,ping[0].bathalongtrack,
			ping[0].ss,ping[0].ssacrosstrack,
			ping[0].ssalongtrack,
			comment,&error);
	strncpy(comment,"\0",256);
	sprintf(comment,"  Output file:        %s",ofile);
	status = mb_put(verbose,ombio_ptr,kind,
			ping[0].time_i,ping[0].time_d,
			ping[0].navlon,ping[0].navlat,
			ping[0].speed,ping[0].heading,
			beams_bath,beams_amp,pixels_ss,
			ping[0].bath,ping[0].amp,
			ping[0].bathacrosstrack,ping[0].bathalongtrack,
			ping[0].ss,ping[0].ssacrosstrack,
			ping[0].ssalongtrack,
			comment,&error);
	strncpy(comment,"\0",256);
	sprintf(comment,"  Longitude flip:     %d",lonflip);
	status = mb_put(verbose,ombio_ptr,kind,
			ping[0].time_i,ping[0].time_d,
			ping[0].navlon,ping[0].navlat,
			ping[0].speed,ping[0].heading,
			beams_bath,beams_amp,pixels_ss,
			ping[0].bath,ping[0].amp,
			ping[0].bathacrosstrack,ping[0].bathalongtrack,
			ping[0].ss,ping[0].ssacrosstrack,
			ping[0].ssalongtrack,
			comment,&error);
	strncpy(comment,"\0",256);
	sprintf(comment,"  Data kind:         %d",datakind);
	status = mb_put(verbose,ombio_ptr,kind,
			ping[0].time_i,ping[0].time_d,
			ping[0].navlon,ping[0].navlat,
			ping[0].speed,ping[0].heading,
			beams_bath,beams_amp,pixels_ss,
			ping[0].bath,ping[0].amp,
			ping[0].bathacrosstrack,ping[0].bathalongtrack,
			ping[0].ss,ping[0].ssacrosstrack,
			ping[0].ssalongtrack,
			comment,&error);
	strncpy(comment,"\0",256);
	sprintf(comment," ");
	status = mb_put(verbose,ombio_ptr,kind,
			ping[0].time_i,ping[0].time_d,
			ping[0].navlon,ping[0].navlat,
			ping[0].speed,ping[0].heading,
			beams_bath,beams_amp,pixels_ss,
			ping[0].bath,ping[0].amp,
			ping[0].bathacrosstrack,ping[0].bathalongtrack,
			ping[0].ss,ping[0].ssacrosstrack,
			ping[0].ssalongtrack,
			comment,&error);

	/* initialize the buffer */
	status = mb_buffer_init(verbose,&buff_ptr,&error);

	/* read and write */
	done = MB_NO;
	first = MB_YES;
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
		status = MB_SUCCESS;
		while (status == MB_SUCCESS)
			{
			status = mb_buffer_get_next_data(verbose,
				buff_ptr,imbio_ptr,start,&ping[ndata].id,
				ping[ndata].time_i,&ping[ndata].time_d,
				&ping[ndata].navlon,&ping[ndata].navlat,
				&ping[ndata].speed,&ping[ndata].heading,
				&beams_bath,&beams_amp,&pixels_ss,
				ping[ndata].bath,ping[ndata].amp,
				ping[ndata].bathacrosstrack,
				ping[ndata].bathalongtrack,
				ping[ndata].ss,
				ping[ndata].ssacrosstrack,
				ping[ndata].ssalongtrack,
				&error);
			if (status == MB_SUCCESS && first != MB_YES)
				{
				if (save_time_d == ping[ndata].time_d)
				    jbeg = ndata + 1;
				}
			if (status == MB_SUCCESS && done != MB_YES)
				{
				if (jend == 0 && ping[ndata].id >= 
				    nbuff - MBFILTER_HOLD/2)
				    {
				    jend = ndata;
				    save_time_d = ping[ndata].time_d;
				    }
				}
			if (status == MB_SUCCESS)
				{
				start = ping[ndata].id + 1;
				ndata++;
				}
			}
		status = MB_SUCCESS;
		error = MB_ERROR_NO_ERROR;
		if (first == MB_YES)
			{
			jbeg = 0;
			}
		if (done == MB_YES)
			jend = ndata - 1;
		nbathdata += (jend - jbeg + 1);
		if (first == MB_YES && nbathdata > 0)
			first = MB_NO;

		/* do hipass */
		iteration = 0;
		while (hipass_mode != MBFILTER_HIPASS_NONE
			&& iteration < hipass_iter)
		{
		/* increment iteration counter */
		iteration++;
		if (verbose > 0)
			fprintf(stderr, "doing hi-pass iteration %d...\n", iteration);

		for (j=jbeg;j<=jend;j++)
		  {

		  /* get beginning and end pings */
		  ja = j - hipass_ndl;
		  jb = j + hipass_ndl;
		  if (ja < 0) ja = 0;
		  if (jb >= ndata) jb = ndata - 1;

		  /* get data arrays and sizes to be used */
		  if (datakind == MBFILTER_BATH)
		    {
		    dataptr0 = ping[j].bath;
		    ndatapts = beams_bath;
		    }
		  else if (datakind == MBFILTER_AMP)
		    {
		    dataptr0 = ping[j].amp;
		    ndatapts = beams_amp;
		    }
		  else if (datakind == MBFILTER_SS)
		    {
		    dataptr0 = ping[j].ss;
		    ndatapts = pixels_ss;
		    }

		  /* loop over each value */
		  for (i=0;i<ndatapts;i++)
		    {
		    /* check if data is valid */
		    if (dataptr0[i] > 0.0)
		      {
		      /* get beginning and end values */
		      ia = i - hipass_ndx;
		      ib = i + hipass_ndx;
		      if (ia < 0) ia = 0;
		      if (ib >= ndatapts) ib = ndatapts - 1;

		      /* loop over surrounding pings and values */
		      nweight = 1;
		      values[0] = dataptr0[i];
		      distances[0] = 0.0;
		      for (jj=ja;jj<=jb;jj++)
		        for (ii=ia;ii<=ib;ii++)
		          {
			  if (datakind == MBFILTER_BATH)
			    dataptr1 = ping[jj].bath;
			  else if (datakind == MBFILTER_AMP)
			    dataptr1 = ping[jj].amp;
			  else if (datakind == MBFILTER_SS)
			    dataptr1 = ping[jj].ss;
			  if (jj != j || ii != i 
			    && dataptr1[ii] > 0.0)
			    {
			    values[nweight] = dataptr1[ii];
			    ddis = 0.0;
			    if (hipass_ndx > 0)
				ddis += (ii - i)*(ii - i)/(hipass_ndx*hipass_ndx);
			    if (hipass_ndl > 0)
				ddis += (jj - j)*(jj - j)/(hipass_ndl*hipass_ndl);
			    distances[nweight] = sqrt(ddis);
			    nweight++;
			    }
			  }

		      /* get hipassed value */
		      if (hipass_mode == MBFILTER_HIPASS_MEAN)
		        hipass_mean(verbose, nweight, values, 
				weights, hipass_offset, 
				&ping[j].dataprocess[i], &error);
		      else if (hipass_mode == MBFILTER_HIPASS_GAUSSIAN)
		        hipass_gaussian(verbose, nweight, values, 
				weights, distances, hipass_offset, 
				&ping[j].dataprocess[i], &error);
		      else if (hipass_mode == MBFILTER_HIPASS_MEDIAN)
		        hipass_median(verbose, nweight, values, 
				weights, hipass_offset, 
				&ping[j].dataprocess[i], &error);
		      }
		    }

		  /* print out progress */
		  if (verbose > 0)
		    fprintf(stderr, "done with ping %d of %d\n", 
			(j-jbeg+1), (jend-jbeg+1));
		  }

		  /* reset ping values */
		  for (j=jbeg;j<=jend;j++)
		    {
		    if (datakind == MBFILTER_SS)
		      for (i=0;i<pixels_ss;i++)
			ping[j].ss[i] = ping[j].dataprocess[i];
		    else if (datakind == MBFILTER_AMP)
		      for (i=0;i<beams_amp;i++)
			ping[j].amp[i] = ping[j].dataprocess[i];
		    else if (datakind == MBFILTER_BATH)
		      for (i=0;i<beams_bath;i++)
			ping[j].bath[i] = ping[j].dataprocess[i];
		    }
		}

		/* do smoothing */
		iteration = 0;
		while (smooth_mode != MBFILTER_SMOOTH_NONE
			&& iteration < smooth_iter)
		{
		/* increment iteration counter */
		iteration++;
		if (verbose > 0)
			fprintf(stderr, "doing lo-pass iteration %d...\n", iteration);

		for (j=jbeg;j<=jend;j++)
		  {

		  /* get beginning and end pings */
		  ja = j - smooth_ndl;
		  jb = j + smooth_ndl;
		  if (ja < 0) ja = 0;
		  if (jb >= ndata) jb = ndata - 1;

		  /* get data arrays and sizes to be used */
		  if (datakind == MBFILTER_BATH)
		    {
		    dataptr0 = ping[j].bath;
		    ndatapts = beams_bath;
		    }
		  else if (datakind == MBFILTER_AMP)
		    {
		    dataptr0 = ping[j].amp;
		    ndatapts = beams_amp;
		    }
		  else if (datakind == MBFILTER_SS)
		    {
		    dataptr0 = ping[j].ss;
		    ndatapts = pixels_ss;
		    }

		  /* loop over each value */
		  for (i=0;i<ndatapts;i++)
		    {
		    /* check if data is valid */
		    if (dataptr0[i] > 0.0)
		      {
		      /* get beginning and end values */
		      ia = i - smooth_ndx;
		      ib = i + smooth_ndx;
		      if (ia < 0) ia = 0;
		      if (ib >= ndatapts) ib = ndatapts - 1;

		      /* loop over surrounding pings and values */
		      nweight = 1;
		      values[0] = dataptr0[i];
		      distances[0] = 0.0;
		      for (jj=ja;jj<=jb;jj++)
		        for (ii=ia;ii<=ib;ii++)
		          {
			  if (datakind == MBFILTER_BATH)
			    dataptr1 = ping[jj].bath;
			  else if (datakind == MBFILTER_AMP)
			    dataptr1 = ping[jj].amp;
			  else if (datakind == MBFILTER_SS)
			    dataptr1 = ping[jj].ss;
			  if (jj != j || ii != i 
			    && dataptr1[ii] > 0.0)
			    {
			    values[nweight] = dataptr1[ii];
			    ddis = 0.0;
			    if (smooth_ndx > 0)
				ddis += (ii - i)*(ii - i)/(smooth_ndx*smooth_ndx);
			    if (smooth_ndl > 0)
				ddis += (jj - j)*(jj - j)/(smooth_ndl*smooth_ndl);
			    distances[nweight] = sqrt(ddis);
			    nweight++;
			    }
			  }

		      /* get smoothed value */
		      if (smooth_mode == MBFILTER_SMOOTH_MEAN)
		        smooth_mean(verbose, nweight, values, weights, 
				&ping[j].dataprocess[i], &error);
		      else if (smooth_mode == MBFILTER_SMOOTH_GAUSSIAN)
		        smooth_gaussian(verbose, nweight, values, 
				weights, distances, 
				&ping[j].dataprocess[i], &error);
		      else if (smooth_mode == MBFILTER_SMOOTH_MEDIAN)
		        smooth_median(verbose, nweight, values, weights, 
				&ping[j].dataprocess[i], &error);
		      else if (smooth_mode == MBFILTER_SMOOTH_GRADIENT)
		        smooth_gradient(verbose, nweight, values, weights, 
				&ping[j].dataprocess[i], &error);
		      }
		    }

		  /* print out progress */
		  if (verbose > 0)
		    fprintf(stderr, "done with ping %d of %d\n", 
			(j-jbeg+1), (jend-jbeg+1));
		  }

		  /* reset ping values */
		  for (j=jbeg;j<=jend;j++)
		    {
		    if (datakind == MBFILTER_SS)
		      for (i=0;i<pixels_ss;i++)
			ping[j].ss[i] = ping[j].dataprocess[i];
		    else if (datakind == MBFILTER_AMP)
		      for (i=0;i<beams_amp;i++)
			ping[j].amp[i] = ping[j].dataprocess[i];
		    else if (datakind == MBFILTER_BATH)
		      for (i=0;i<beams_bath;i++)
			ping[j].bath[i] = ping[j].dataprocess[i];
		    }
		}

		/* do contrast enhancement */
		iteration = 0;
		while (contrast_mode != MBFILTER_CONTRAST_NONE
			&& iteration < contrast_iter)
		{
		/* increment iteration counter */
		iteration++;
		if (verbose > 0)
			fprintf(stderr, "doing contrast iteration %d...\n", iteration);

		for (j=jbeg;j<=jend;j++)
		  {

		  /* get beginning and end pings */
		  ja = j - contrast_ndl;
		  jb = j + contrast_ndl;
		  if (ja < 0) ja = 0;
		  if (jb >= ndata) jb = ndata - 1;

		  /* get data arrays and sizes to be used */
		  if (datakind == MBFILTER_BATH)
		    {
		    dataptr0 = ping[j].bath;
		    ndatapts = beams_bath;
		    }
		  else if (datakind == MBFILTER_AMP)
		    {
		    dataptr0 = ping[j].amp;
		    ndatapts = beams_amp;
		    }
		  else if (datakind == MBFILTER_SS)
		    {
		    dataptr0 = ping[j].ss;
		    ndatapts = pixels_ss;
		    }

		  /* loop over each value */
		  for (i=0;i<ndatapts;i++)
		    {
		    /* check if data is valid */
		    if (dataptr0[i] > 0.0)
		      {
		      /* get beginning and end values */
		      ia = i - contrast_ndx;
		      ib = i + contrast_ndx;
		      if (ia < 0) ia = 0;
		      if (ib >= ndatapts) ib = ndatapts - 1;

		      /* loop over surrounding pings and values */
		      nweight = 1;
		      values[0] = dataptr0[i];
		      distances[0] = 0.0;
		      for (jj=ja;jj<=jb;jj++)
		        for (ii=ia;ii<=ib;ii++)
		          {
			  if (datakind == MBFILTER_BATH)
			    dataptr1 = ping[jj].bath;
			  else if (datakind == MBFILTER_AMP)
			    dataptr1 = ping[jj].amp;
			  else if (datakind == MBFILTER_SS)
			    dataptr1 = ping[jj].ss;
			  if (jj != j || ii != i 
			    && dataptr1[ii] > 0.0)
			    {
			    values[nweight] = dataptr1[ii];
			    ddis = 0.0;
			    if (contrast_ndx > 0)
				ddis += (ii - i)*(ii - i)/(contrast_ndx*contrast_ndx);
			    if (contrast_ndl > 0)
				ddis += (jj - j)*(jj - j)/(contrast_ndl*contrast_ndl);
			    distances[nweight] = sqrt(ddis);
			    nweight++;
			    }
			  }

		      /* get contrast enhanced value */
		      if (contrast_mode == MBFILTER_CONTRAST_EDGE)
		        contrast_edge(verbose, nweight, values, weights, 
				&ping[j].dataprocess[i], &error);
		      else if (contrast_mode == MBFILTER_CONTRAST_GRADIENT)
		        contrast_gradient(verbose, nweight, values, weights, 
				&ping[j].dataprocess[i], &error);
		      }
		    }

		  /* print out progress */
		  if (verbose > 0)
		    fprintf(stderr, "done with ping %d of %d\n", 
			(j-jbeg+1), (jend-jbeg+1));
		  }

		  /* reset ping values */
		  for (j=jbeg;j<=jend;j++)
		    {
		    if (datakind == MBFILTER_SS)
		      for (i=0;i<pixels_ss;i++)
			ping[j].ss[i] = ping[j].dataprocess[i];
		    else if (datakind == MBFILTER_AMP)
		      for (i=0;i<beams_amp;i++)
			ping[j].amp[i] = ping[j].dataprocess[i];
		    else if (datakind == MBFILTER_BATH)
		      for (i=0;i<beams_bath;i++)
			ping[j].bath[i] = ping[j].dataprocess[i];
		    }
		}

		/* reset pings in buffer */
		for (j=jbeg;j<=jend;j++)
		  {
		  status = mb_buffer_insert(verbose,
				buff_ptr,imbio_ptr,ping[j].id,
				ping[j].time_i,ping[j].time_d,
				ping[j].navlon,ping[j].navlat,
				ping[j].speed,ping[j].heading,
				beams_bath,beams_amp,pixels_ss,
				ping[j].bath,ping[j].amp,
				ping[j].bathacrosstrack,
				ping[j].bathalongtrack,
				ping[j].ss,
				ping[j].ssacrosstrack,
				ping[j].ssalongtrack,
				comment,&error);
		  }

		/* find number of pings to hold */
		if (done == MB_YES)
			nhold = 0;
		else if (ndata > MBFILTER_HOLD)
			{
			nhold = nbuff - ping[ndata-MBFILTER_HOLD].id + 1;
			}
		else if (ndata > 0)
			{
			nhold = nbuff - ping[0].id + 1;
			}
		else
			nhold = 0;

		/* dump data from the buffer */
		ndump = 0;
		if (nbuff > 0)
			{
			status = mb_buffer_dump(verbose,buff_ptr,ombio_ptr,
				nhold,&ndump,&nbuff,&error);
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
	for (j=0;j<MBFILTER_BUFFER;j++)
		{
		mb_free(verbose,&ping[j].bath,&error); 
		mb_free(verbose,&ping[j].bathacrosstrack,&error); 
		mb_free(verbose,&ping[j].bathalongtrack,&error); 
		mb_free(verbose,&ping[j].amp,&error); 
		mb_free(verbose,&ping[j].ss,&error); 
		mb_free(verbose,&ping[j].ssacrosstrack,&error); 
		mb_free(verbose,&ping[j].ssalongtrack,&error); 
		mb_free(verbose,&ping[j].dataprocess,&error); 
		}

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
	exit(status);
}
/*--------------------------------------------------------------------*/
int hipass_mean(verbose, n, val, wgt, off, 
		hipass, error)
int	verbose;
int	n;
double	*val;
double	*wgt;
double	off;
double	*hipass;
int	*error;
{
	char	*function_name = "hipass_mean";
	int	status = MB_SUCCESS;
	int	i, nn;
	

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBFILTER function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:         %d\n",verbose);
		fprintf(stderr,"dbg2       n:               %d\n",n);
		fprintf(stderr,"dbg2       val:             %d\n",val);
		fprintf(stderr,"dbg2       wgt:             %d\n",wgt);
		fprintf(stderr,"dbg2       off:             %f\n",off);
		for (i=0;i<n;i++)
			fprintf(stderr,"dbg2       val[%d]: %f\n", i, val[i]);
		}

	/* get mean */
	*hipass = 0.0;
	nn = 0;
	for (i=0;i<n;i++)
		{
		if (val[i] > 0.0)
			{
			*hipass += val[i];
			nn++;
			}
		}
	if (nn > 0)
		*hipass = val[0] - *hipass/nn + off;

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBFILTER function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       hipass:          %f\n",*hipass);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:          %d\n",status);
		}

	/* return status */
	return(status);
}
/*--------------------------------------------------------------------*/
int hipass_gaussian(verbose, n, val, wgt, dis, off, 
		hipass, error)
int	verbose;
int	n;
double	*val;
double	*wgt;
double	*dis;
double	off;
double	*hipass;
int	*error;
{
	char	*function_name = "hipass_gaussian";
	int	status = MB_SUCCESS;
	double	wgtsum;
	int	i, nn;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBFILTER function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:         %d\n",verbose);
		fprintf(stderr,"dbg2       n:               %d\n",n);
		fprintf(stderr,"dbg2       val:             %d\n",val);
		fprintf(stderr,"dbg2       wgt:             %d\n",wgt);
		fprintf(stderr,"dbg2       dis:             %d\n",dis);
		fprintf(stderr,"dbg2       off:             %f\n",off);
		for (i=0;i<n;i++)
			fprintf(stderr,"dbg2       val[%d]: %f  dis[%d]: %f\n", 
				i, val[i], i, dis[i]);
		}

	/* get weights */
	*hipass = 0.0;
	wgtsum = 0.0;
	for (i=0;i<n;i++)
		{
		if (val[i] > 0.0)
			{
			wgt[i] = exp(-dis[i]*dis[i]);
			wgtsum += wgt[i];
			}
		}

	if (wgtsum > 0.0)
		{
		/* get value */
		*hipass = 0.0;
		for (i=0;i<n;i++)
			{
			if (val[i] > 0.0)
				{
				*hipass += wgt[i]*val[i];
				}
			}
		*hipass = val[0] - *hipass/wgtsum + off;
		}

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBFILTER function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       hipass:          %f\n",*hipass);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:          %d\n",status);
		}

	/* return status */
	return(status);
}
/*--------------------------------------------------------------------*/
int hipass_median(verbose, n, val, wgt, off, 
		hipass, error)
int	verbose;
int	n;
double	*val;
double	*wgt;
double	off;
double	*hipass;
int	*error;
{
	char	*function_name = "hipass_median";
	int	status = MB_SUCCESS;
	int	i;	

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBFILTER function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:         %d\n",verbose);
		fprintf(stderr,"dbg2       n:               %d\n",n);
		fprintf(stderr,"dbg2       val:             %d\n",val);
		fprintf(stderr,"dbg2       wgt:             %d\n",wgt);
		fprintf(stderr,"dbg2       off:             %f\n",off);
		for (i=0;i<n;i++)
			fprintf(stderr,"dbg2       val[%d]: %f\n", i, val[i]);
		}

	/* start off */
	*hipass = 0.0;

	/* sort values and get median value */
	if (n > 0)
		{
		sort(n, val);
		*hipass = val[0] - val[n/2] + off;
		}

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBFILTER function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       hipass:          %f\n",*hipass);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:          %d\n",status);
		}

	/* return status */
	return(status);
}
/*--------------------------------------------------------------------*/
int smooth_mean(verbose, n, val, wgt, 
		smooth, error)
int	verbose;
int	n;
double	*val;
double	*wgt;
double	*smooth;
int	*error;
{
	char	*function_name = "smooth_mean";
	int	status = MB_SUCCESS;
	int	i, nn;
	

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBFILTER function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:         %d\n",verbose);
		fprintf(stderr,"dbg2       n:               %d\n",n);
		fprintf(stderr,"dbg2       val:             %d\n",val);
		fprintf(stderr,"dbg2       wgt:             %d\n",wgt);
		for (i=0;i<n;i++)
			fprintf(stderr,"dbg2       val[%d]: %f\n", i, val[i]);
		}

	/* get mean */
	*smooth = 0.0;
	nn = 0;
	for (i=0;i<n;i++)
		{
		if (val[i] > 0.0)
			{
			*smooth += val[i];
			nn++;
			}
		}
	if (nn > 0)
		*smooth = *smooth/nn;

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBFILTER function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       smooth:          %f\n",*smooth);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:          %d\n",status);
		}

	/* return status */
	return(status);
}
/*--------------------------------------------------------------------*/
int smooth_gaussian(verbose, n, val, wgt, dis, 
		smooth, error)
int	verbose;
int	n;
double	*val;
double	*wgt;
double	*dis;
double	*smooth;
int	*error;
{
	char	*function_name = "smooth_gaussian";
	int	status = MB_SUCCESS;
	double	wgtsum;
	int	i, nn;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBFILTER function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:         %d\n",verbose);
		fprintf(stderr,"dbg2       n:               %d\n",n);
		fprintf(stderr,"dbg2       val:             %d\n",val);
		fprintf(stderr,"dbg2       wgt:             %d\n",wgt);
		fprintf(stderr,"dbg2       dis:             %d\n",dis);
		for (i=0;i<n;i++)
			fprintf(stderr,"dbg2       val[%d]: %f  dis[%d]: %f\n", 
				i, val[i], i, dis[i]);
		}

	/* get weights */
	*smooth = 0.0;
	wgtsum = 0.0;
	for (i=0;i<n;i++)
		{
		if (val[i] > 0.0)
			{
			wgt[i] = exp(-dis[i]*dis[i]);
			wgtsum += wgt[i];
			}
		}

	if (wgtsum > 0.0)
		{
		/* get value */
		*smooth = 0.0;
		for (i=0;i<n;i++)
			{
			if (val[i] > 0.0)
				{
				*smooth += wgt[i]*val[i];
				}
			}
		*smooth = *smooth/wgtsum;
		}

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBFILTER function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       smooth:          %f\n",*smooth);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:          %d\n",status);
		}

	/* return status */
	return(status);
}
/*--------------------------------------------------------------------*/
int smooth_median(verbose, n, val, wgt, 
		smooth, error)
int	verbose;
int	n;
double	*val;
double	*wgt;
double	*smooth;
int	*error;
{
	char	*function_name = "smooth_median";
	int	status = MB_SUCCESS;
	int	i;	

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBFILTER function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:         %d\n",verbose);
		fprintf(stderr,"dbg2       n:               %d\n",n);
		fprintf(stderr,"dbg2       val:             %d\n",val);
		fprintf(stderr,"dbg2       wgt:             %d\n",wgt);
		for (i=0;i<n;i++)
			fprintf(stderr,"dbg2       val[%d]: %f\n", i, val[i]);
		}

	/* start off */
	*smooth = 0.0;

	/* sort values and get median value */
	if (n > 0)
		{
		sort(n, val);
		*smooth = val[n/2];
		}

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBFILTER function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       smooth:          %f\n",*smooth);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:          %d\n",status);
		}

	/* return status */
	return(status);
}
/*--------------------------------------------------------------------*/
int smooth_gradient(verbose, n, val, wgt, 
		smooth, error)
int	verbose;
int	n;
double	*val;
double	*wgt;
double	*smooth;
int	*error;
{
	char	*function_name = "smooth_gradient";
	int	status = MB_SUCCESS;
	double	wgtsum;
	double	diff;
	int	i, nn;
	

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBFILTER function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:         %d\n",verbose);
		fprintf(stderr,"dbg2       n:               %d\n",n);
		fprintf(stderr,"dbg2       val:             %d\n",val);
		fprintf(stderr,"dbg2       wgt:             %d\n",wgt);
		for (i=0;i<n;i++)
			fprintf(stderr,"dbg2       val[%d]: %f\n", i, val[i]);
		}

	/* get weights */
	*smooth = 0.0;
	wgtsum = 0.0;
	nn = 0;
	wgt[0] = 0.5;
	for (i=1;i<n;i++)
		{
		if (val[i] > 0.0)
			{
			diff = fabs(val[i] - val[0]);
			if (diff < 0.01)
				diff = 0.01;
			wgt[i] = 1.0/diff;
			wgtsum += wgt[i];
			nn++;
			}
		}
	if (nn > 0)
		{
		*smooth = wgt[0]*val[0];
		for (i=1;i<n;i++)
			{
			if (val[i] > 0.0)
				{
				*smooth += 0.5*wgt[i]*val[i]/wgtsum;
				}
			}
		}

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBFILTER function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       smooth:          %f\n",*smooth);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:          %d\n",status);
		}

	/* return status */
	return(status);
}
/*--------------------------------------------------------------------*/
int contrast_edge(verbose, n, val, wgt, 
		contrast, error)
int	verbose;
int	n;
double	*val;
double	*wgt;
double	*contrast;
int	*error;
{
	char	*function_name = "contrast_edge";
	int	status = MB_SUCCESS;
	double	wgtsum;
	double	diff;
	int	i, nn;
	

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBFILTER function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:         %d\n",verbose);
		fprintf(stderr,"dbg2       n:               %d\n",n);
		fprintf(stderr,"dbg2       val:             %d\n",val);
		fprintf(stderr,"dbg2       wgt:             %d\n",wgt);
		for (i=0;i<n;i++)
			fprintf(stderr,"dbg2       val[%d]: %f\n", i, val[i]);
		}

	/* get weights */
	*contrast = 0.0;
	wgtsum = 0.0;
	nn = 0;
	wgt[0] = 0.5;
	for (i=1;i<n;i++)
		{
		if (val[i] > 0.0)
			{
			diff = fabs(val[i] - val[0]);
			if (diff < 0.01)
				diff = 0.01;
			wgt[i] = 1.0/diff;
			wgtsum += wgt[i];
			nn++;
			}
		}
	if (nn > 0)
		{
		*contrast = wgt[0]*val[0];
		for (i=1;i<n;i++)
			{
			if (val[i] > 0.0)
				{
				*contrast += 0.5*wgt[i]*val[i]/wgtsum;
				}
			}
		}

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBFILTER function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       contrast:          %f\n",*contrast);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:          %d\n",status);
		}

	/* return status */
	return(status);
}
/*--------------------------------------------------------------------*/
int contrast_gradient(verbose, n, val, wgt, 
		contrast, error)
int	verbose;
int	n;
double	*val;
double	*wgt;
double	*contrast;
int	*error;
{
	char	*function_name = "contrast_gradient";
	int	status = MB_SUCCESS;
	double	wgtsum;
	double	diff;
	int	i, nn;
	

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBFILTER function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:         %d\n",verbose);
		fprintf(stderr,"dbg2       n:               %d\n",n);
		fprintf(stderr,"dbg2       val:             %d\n",val);
		fprintf(stderr,"dbg2       wgt:             %d\n",wgt);
		for (i=0;i<n;i++)
			fprintf(stderr,"dbg2       val[%d]: %f\n", i, val[i]);
		}

	/* get weights */
	*contrast = 0.0;
	wgtsum = 0.0;
	nn = 0;
	wgt[0] = 0.5;
	for (i=1;i<n;i++)
		{
		if (val[i] > 0.0)
			{
			diff = fabs(val[i] - val[0]);
			if (diff < 0.01)
				diff = 0.01;
			wgt[i] = 1.0/diff;
			wgtsum += wgt[i];
			nn++;
			}
		}
	if (nn > 0)
		{
		*contrast = wgt[0]*val[0];
		for (i=1;i<n;i++)
			{
			if (val[i] > 0.0)
				{
				*contrast += 0.5*wgt[i]*val[i]/wgtsum;
				}
			}
		}

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBFILTER function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       contrast:          %f\n",*contrast);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:          %d\n",status);
		}

	/* return status */
	return(status);
}
/*--------------------------------------------------------------------*/
/* 	function sort sorts the data.  
 *	Uses the shell method from Numerical Recipes.
 */
#define ALN2I 1.442695022
#define TINY 1.0e-5
int sort(n,r)
int	n;
double *r;
{
	int	status = MB_SUCCESS;
	int	nn, m, j, i, lognb2;
	double	tr;

	if (n <= 0) 
		{
		status = MB_FAILURE;
		return(status);
		}

	lognb2 = (log((double) n)*ALN2I + TINY);
	m = n;
	for (nn=1;nn<=lognb2;nn++)
		{
		m >>= 1;
		for (j=m+1;j<=n;j++)
			{
			i = j - m;
			tr = r[j];
			while (i >= 1 && r[i] > tr)
				{
				r[i+m] = r[i];
				i -= m;
				}
			r[i+m] = tr;
			}
		}

	return(status);
}
/*--------------------------------------------------------------------*/
