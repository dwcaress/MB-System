/*--------------------------------------------------------------------
 *    The MB-system:	mbanglecorrect.c	1/12/95
 *    $Id: mbanglecorrect.c,v 4.1 1995-02-22 21:53:14 caress Exp $
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
 * mbanglecorrect is a tool for processing sidescan data.  This program
 * corrects the sidescan data by dividing by a model of the
 * backscatter vs grazing angle function to produce a "flat" image which
 * shows geology better than the raw data. The backscatter vs grazing
 * angle model is obtained by averaging over the input sidescan data
 * in some number of nearby pings using the same algorithm as the
 * program mbbackangle. Because the model used to correct the data
 * is locally defined,  this program is best suited to producing data
 * which shows local (fine scale) structure. A program which uses
 * a single model to correct all of the data will produce data 
 * better suited to showing large scale variability in seafloor 
 * reflectivity.
 * The default input and output streams are stdin and stdout.
 *
 * Author:	D. W. Caress
 * Date:	January 12, 1995
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

/* mode defines */
#define	MBANGLECORRECT_AMP		1
#define	MBANGLECORRECT_SS			2
#define MBANGLECORRECT_LENGTH_NUMBER	1
#define MBANGLECORRECT_LENGTH_DISTANCE	2

/* MBIO buffer structure pointer */
#define	MBANGLECORRECT_BUFFER	500
#define	MBANGLECORRECT_HOLD	50

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
	static char rcs_id[] = "$Id: mbanglecorrect.c,v 4.1 1995-02-22 21:53:14 caress Exp $";
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
-Fformat -G -Iinfile -Nnangles/angle_max -Ooutfile -Rw/e/s/n \
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
	int	nwant = MBANGLECORRECT_BUFFER;
	int	nhold = MBANGLECORRECT_HOLD;
	int	nbuff = 0;
	int	nload;
	int	ndump;
	int	nexpect;
	struct mbanglecorrect_ping_struct ping[MBANGLECORRECT_BUFFER];
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

	/* angle function variables */
	int	ampkind = MBANGLECORRECT_SS;
	char	sfile[128];
	int	use_global_statics = MB_NO;
	int	symmetry = MB_YES;
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

	/* slope calculation variables */
	int	ndepths;
	double	*depths;
	double	*depthacrosstrack;
	int	nslopes;
	double	*slopes;
	double	*slopeacrosstrack;
	int	use_slope = MB_YES;

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
	while ((c = getopt(argc, argv, "A:a:B:b:CcD:d:E:e:F:f:GgHhI:i:N:n:O:o:R:r:S:s:VvZ:z:")) != -1)
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
		exit(MB_FAILURE);
		}

	/* print starting message */
	if (verbose == 1)
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
		fprintf(stderr,"dbg2       data format:    %d\n",format);
		fprintf(stderr,"dbg2       input file:     %s\n",ifile);
		fprintf(stderr,"dbg2       output file:    %s\n",ofile);
		fprintf(stderr,"dbg2       AGA file:       %s\n",sfile);
		fprintf(stderr,"dbg2       ampkind:        %d\n",ampkind);
		fprintf(stderr,"dbg2       depth_def:      %f\n",depth_default);
		fprintf(stderr,"dbg2       length_mode:    %d\n",length_mode);
		fprintf(stderr,"dbg2       length_max:     %f\n",length_max);
		fprintf(stderr,"dbg2       use_slope:      %d\n",use_slope);
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
		exit(MB_ERROR_BAD_FORMAT);
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
	for (i=0;i<MBANGLECORRECT_BUFFER;i++)
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
		}

	/* if error initializing memory then quit */
	if (error != MB_ERROR_NO_ERROR)
		{
		mb_error(verbose,error,message);
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
			mb_error(verbose,error,message);
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
			mb_error(verbose,error,message);
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
		if (ampkind == MBANGLECORRECT_AMP)
			fprintf(stderr, "Working on beam amplitude data...\n");
		else
			fprintf(stderr, "Working on sidescan data...\n");
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
	if (ampkind == MBANGLECORRECT_AMP)
		{
		strncpy(comment,"\0",256);
		sprintf(comment,"Beam amplitude values corrected by dividing");
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
		sprintf(comment,"Sidescan values corrected by dividing");
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
	if (use_global_statics == MB_NO)
		{
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
		}
	else
		{
		strncpy(comment,"\0",256);
		sprintf(comment,"  by a user supplied function of grazing angle.");
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
	sprintf(comment,"  Data kind:         %d",ampkind);
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
	sprintf(comment,"  Default depth:      %f",depth_default);
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
	sprintf(comment,"  Length mode:        %d",length_mode);
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
	sprintf(comment,"  Length max:         %f",length_max);
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
	if (use_global_statics == MB_YES)
		{
		strncpy(comment,"\0",256);
		sprintf(comment,"  Static angle correction file: %s",sfile);
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
		sprintf(comment,"  Static sidescan corrections:");
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
		for (i=0;i<nangles;i++)
			{
			strncpy(comment,"\0",256);
			sprintf(comment,"    %f  %f",angles[i],mean[i]);
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
		}
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
		first_distance = MB_YES;
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

			/* get the seafloor slopes */
			if (status == MB_SUCCESS && beams_bath > 0)
				{
				if (beams_bath > 0)
				    set_bathyslope(verbose, 
					beams_bath,
					ping[ndata].bath,
					ping[ndata].bathacrosstrack,
					&ping[ndata].ndepths,
					ping[ndata].depths,
					ping[ndata].depthacrosstrack,
					&ping[ndata].nslopes,
					ping[ndata].slopes,
					ping[ndata].slopeacrosstrack,
					&error);
				}

			if (status == MB_SUCCESS && first_distance == MB_YES)
				{
				first_distance = MB_NO;
				ping[ndata].distance = 0.0;
				mb_coor_scale(verbose,ping[ndata].navlat,
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
				if (save_time_d == ping[ndata].time_d)
				    jbeg = ndata + 1;
				}
			if (status == MB_SUCCESS && done != MB_YES)
				{
				if (jend == 0 && ping[ndata].id >= 
				    nbuff - MBANGLECORRECT_HOLD/2)
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
		if (first == MB_YES)
			{
			jbeg = 0;
			}
		if (done == MB_YES)
			jend = ndata - 1;
		if (jend == 0 && ndata > 0)
			{
			jend = ndata - 1;
			save_time_d = ping[ndata-1].time_d;
			}
		nbathdata += (jend - jbeg + 1);
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
		      for (i=0;i<beams_amp;i++)
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
		      for (i=0;i<pixels_ss;i++)
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
				    ping[jj].ssacrosstrack,
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
		    for (i=0;i<beams_amp;i++)
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
				status = get_anglecorr(verbose, 
					    nangles, angles, mean, 
					    angle, &correction, &error);
				ping[j].dataprocess[i] = 
				    scale*ping[j].amp[i]/correction;
				}
			    }
			}

		  /* do the sidescan */
		  if (ampkind == MBANGLECORRECT_SS)
		    for (i=0;i<pixels_ss;i++)
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
				status = get_anglecorr(verbose, 
					nangles, angles, mean, 
					angle, &correction, &error);
				ping[j].dataprocess[i] = scale*
				    ping[j].ss[i]/correction;
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
		      for (i=0;i<beams_bath;i++)
		        fprintf(stderr,"dbg2       %d %f %f %f\n",
			    i, ping[j].bath[i], 
			    ping[j].bathacrosstrack[i], 
			    ping[j].bathalongtrack[i]);
		      }
		    if (beams_amp > 0 && ampkind == MBANGLECORRECT_AMP)
		      {
		      fprintf(stderr,"dbg2  Beam Intensity Data:\n");
		      for (i=0;i<beams_bath;i++)
		        fprintf(stderr,"dbg2       %d %f %f %f\n",
			    i, ping[j].dataprocess[i], 
			    ping[j].bathacrosstrack[i], 
			    ping[j].bathalongtrack[i]);
		      }
		    else if (beams_amp > 0)
		      {
		      fprintf(stderr,"dbg2  Beam Intensity Data:\n");
		      for (i=0;i<beams_bath;i++)
		        fprintf(stderr,"dbg2       %d %f %f %f\n",
			    i, ping[j].amp[i], 
			    ping[j].bathacrosstrack[i], 
			    ping[j].bathalongtrack[i]);
		      }
		    if (pixels_ss > 0 && ampkind == MBANGLECORRECT_SS)
		      {
		      fprintf(stderr,"dbg2  Sidescan Data:\n");
		      for (i=0;i<pixels_ss;i++)
		        fprintf(stderr,"dbg2       %d %f %f %f\n",
			    i, ping[j].dataprocess[i], 
			    ping[j].ssacrosstrack[i], 
			    ping[j].ssalongtrack[i]);
		      }
		    else if (pixels_ss > 0)
		      {
		      fprintf(stderr,"dbg2  Sidescan Data:\n");
		      for (i=0;i<pixels_ss;i++)
		        fprintf(stderr,"dbg2       %d %f %f %f\n",
			    i, ping[j].ss[i], 
			    ping[j].ssacrosstrack[i], 
			    ping[j].ssalongtrack[i]);
		      }
		    }


		  }

		/* reset pings in buffer */
		for (j=jbeg;j<=jend;j++)
		  {
		  if (ampkind == MBANGLECORRECT_SS)
			for (i=0;i<pixels_ss;i++)
				ping[j].ss[i] = ping[j].dataprocess[i];
		  else if (ampkind == MBANGLECORRECT_AMP)
			for (i=0;i<beams_amp;i++)
				ping[j].amp[i] = ping[j].dataprocess[i];
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
		else if (ndata > MBANGLECORRECT_HOLD)
			{
			nhold = nbuff 
				- ping[ndata-MBANGLECORRECT_HOLD].id + 1;
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
	for (j=0;j<MBANGLECORRECT_BUFFER;j++)
		{
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
	exit(status);
}
/*--------------------------------------------------------------------*/
int set_bathyslope(verbose,
	nbath,bath,bathacrosstrack,
	ndepths,depths,depthacrosstrack, 
	nslopes,slopes,slopeacrosstrack, 
	error)
int	verbose;
int	nbath;
double	*bath;
double	*bathacrosstrack;
int	*ndepths;
double	*depths;
double	*depthacrosstrack;
int	*nslopes;
double	*slopes;
double	*slopeacrosstrack;
int	*error;
{
	char	*function_name = "set_bathyslope";
	int	status = MB_SUCCESS;
	int	i;
	

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

	/* first find all depths */
	*ndepths = 0;
	for (i=0;i<nbath;i++)
		{
		if (bath[i] > 0.0)
			{
			depths[*ndepths] = bath[i];
			depthacrosstrack[*ndepths] = bathacrosstrack[i];
			(*ndepths)++;
			}
		}

	/* now calculate slopes */
	*nslopes = *ndepths + 1;
	for (i=0;i<*ndepths-1;i++)
		{
		slopes[i+1] = (depths[i+1] - depths[i])
			/(depthacrosstrack[i+1] - depthacrosstrack[i]);
		slopeacrosstrack[i+1] = 0.5*(depthacrosstrack[i+1] 
			+ depthacrosstrack[i]);
		}
	if (*ndepths > 1)
		{
		slopes[0] = 0.0;
		slopeacrosstrack[0] = depthacrosstrack[0];
		slopes[*ndepths] = 0.0;
		slopeacrosstrack[*ndepths] = 
			depthacrosstrack[*ndepths-1];
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
		for (i=0;i<*ndepths;i++)
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
	if (acrosstrack >= depthacrosstrack[0]
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
