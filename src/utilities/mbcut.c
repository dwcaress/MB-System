/*--------------------------------------------------------------------
 *    The MB-system:	mbcut.c	1/26/95
 *
 *    $Id: mbcut.c,v 4.8 1997-09-15 19:11:06 caress Exp $
 *
 *    Copyright (c) 1993, 1994, 1995 by 
 *    D. W. Caress (caress@lamont.ldgo.columbia.edu)
 *    and D. N. Chayes (dale@lamont.ldgo.columbia.edu)
 *    Lamont-Doherty Earth Observatory
 *    Palisades, NY  10964
 *
 *    See README file for copying and redistribution conditions.
 *--------------------------------------------------------------------*/
/*
 * MBCUT removes swath data values that lie outside ranges
 * specified by the user. The acceptable range can be specified
 * in terms of beam or pixel numbers or acrosstrack distances.
 * The default input and output streams are stdin and stdout.
 *
 * Author:	D. W. Caress
 * Date:	January 26, 1995
 *
 * $Log: not supported by cvs2svn $
 * Revision 4.7  1997/04/21  17:19:14  caress
 * MB-System 4.5 Beta Release.
 *
 * Revision 4.7  1997/04/17  15:14:38  caress
 * MB-System 4.5 Beta Release
 *
 * Revision 4.6  1996/04/22  13:23:05  caress
 * Now have DTR and MIN/MAX defines in mb_define.h
 *
 * Revision 4.5  1996/03/11  15:29:02  caress
 * Fixed typo causing beams to be zero'd rather than flagged.
 *
 * Revision 4.4  1996/01/26  21:25:58  caress
 * Version 4.3 distribution
 *
 * Revision 4.3  1995/05/12  17:12:32  caress
 * Made exit status values consistent with Unix convention.
 * 0: ok  nonzero: error
 *
 * Revision 4.2  1995/03/06  19:37:59  caress
 * Changed include strings.h to string.h for POSIX compliance.
 *
 * Revision 4.1  1995/03/02  13:49:21  caress
 * Fixed bug related to error messages.
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

/* mbio include files */
#include "../../include/mb_status.h"
#include "../../include/mb_format.h"
#include "../../include/mb_define.h"

/* mbcut defines */
#define	MBCUT_DATA_BATH	    0
#define	MBCUT_DATA_AMP	    1
#define	MBCUT_DATA_SS	    2
#define	MBCUT_MODE_NONE	    0
#define	MBCUT_MODE_NUMBER   1
#define	MBCUT_MODE_DISTANCE 2
#define	MBCUT_RANGE_MAX	    20

/*--------------------------------------------------------------------*/

main (argc, argv)
int argc;
char **argv; 
{
	/* id variables */
	static char rcs_id[] = "$Id: mbcut.c,v 4.8 1997-09-15 19:11:06 caress Exp $";
	static char program_name[] = "mbcut";
	static char help_message[] = 
"MBCUT removes swath data values that lie outside ranges\n\t\
specified by the user. The acceptable ranges can be specified\n\t\
in terms of beam or pixel numbers or acrosstrack distance.\n\t\
A good data range can be specified for each data type\n\t\
in a file (bathymetry and/or amplitude and/or sidescan).\n\t\
The default input and output streams are stdin and stdout.";
	static char usage_message[] = "mbcut [-Akind/mode/min/max \
	-Byr/mo/da/hr/mn/sc -Eyr/mo/da/hr/mn/sc -Fformat -H \
	-Iinfile -Llonflip -Ooutfile -Rw/e/s/n -Sspeed -V]";

	/* parsing variables */
	extern char *optarg;
	extern int optkind;
	int	errflg = 0;
	int	c;
	int	help = 0;
	int	flag = 0;

	/* MBIO status variables */
	int	status = MB_SUCCESS;
	int	verbose = 0;
	int	error = MB_ERROR_NO_ERROR;
	char	*message;

	/* MBIO read and write control parameters */
	int	format = 0;
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
	char	ofile[128];
	char	*ombio_ptr = NULL;

	/* mbio read and write values */
	char	*store_ptr;
	int	kind;
	int	time_i[7];
	double	time_d;
	double	navlon;
	double	navlat;
	double	speed;
	double	heading;
	double	distance;
	int	nbath;
	int	namp;
	int	nss;
	double	*bath = NULL;
	double	*bathacrosstrack = NULL;
	double	*bathalongtrack = NULL;
	double	*amp = NULL;
	double	*ss = NULL;
	double	*ssacrosstrack = NULL;
	double	*ssalongtrack = NULL;
	int	*bathflag = NULL;
	int	*ampflag = NULL;
	int	*ssflag = NULL;
	int	idata = 0;
	int	icomment = 0;
	int	odata = 0;
	int	ocomment = 0;
	char	comment[256];

	/* good data criterea variables */
	int	ncut = 0;
	int	gkind[MBCUT_RANGE_MAX];
	int	gmode[MBCUT_RANGE_MAX];
	double	gmin[MBCUT_RANGE_MAX];
	double	gmax[MBCUT_RANGE_MAX];
	int	check_bath = MB_NO;
	int	check_amp = MB_NO;
	int	check_ss = MB_NO;

	/* time, user, host variables */
	long int	right_now;
	char	date[25], user[128], *user_ptr, host[128];

	int	icut, istart, iend;
	int	i, j, k, l, m;

	char	*ctime();
	char	*getenv();

	/* get current default values */
	status = mb_defaults(verbose,&format,&pings,&lonflip,bounds,
		btime_i,etime_i,&speedmin,&timegap);

	/* reset pings and timegap */
	pings = 1;
	timegap = 1000000000.0;

	/* set default input and output */
	strcpy (ifile, "stdin");
	strcpy (ofile, "stdout");

	/* process argument list */
	while ((c = getopt(argc, argv, "A:a:B:b:E:e:F:f:HhL:l:I:i:O:o:R:r:S:s:Vv")) != -1)
	  switch (c) 
		{
		case 'A':
		case 'a':
			sscanf (optarg,"%d/%d/%lf/%lf", 
			    &gkind[ncut], &gmode[ncut], 
			    &gmin[ncut], &gmax[ncut]);
			if (gkind[ncut] == MBCUT_DATA_BATH)
				check_bath = MB_YES;
			if (gkind[ncut] == MBCUT_DATA_AMP)
				check_amp = MB_YES;
			if (gkind[ncut] == MBCUT_DATA_SS)
				check_ss = MB_YES;
			if (ncut < MBCUT_RANGE_MAX)
				ncut++;
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
		case 'L':
		case 'l':
			sscanf (optarg,"%d", &lonflip);
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
			sscanf (optarg,"%lf", &speedmin);
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
		fprintf(stderr,"dbg2       verbose:         %d\n",verbose);
		fprintf(stderr,"dbg2       help:            %d\n",help);
		fprintf(stderr,"dbg2       format:          %d\n",format);
		fprintf(stderr,"dbg2       pings:           %d\n",pings);
		fprintf(stderr,"dbg2       lonflip:         %d\n",lonflip);
		fprintf(stderr,"dbg2       bounds[0]:       %f\n",bounds[0]);
		fprintf(stderr,"dbg2       bounds[1]:       %f\n",bounds[1]);
		fprintf(stderr,"dbg2       bounds[2]:       %f\n",bounds[2]);
		fprintf(stderr,"dbg2       bounds[3]:       %f\n",bounds[3]);
		fprintf(stderr,"dbg2       btime_i[0]:      %d\n",btime_i[0]);
		fprintf(stderr,"dbg2       btime_i[1]:      %d\n",btime_i[1]);
		fprintf(stderr,"dbg2       btime_i[2]:      %d\n",btime_i[2]);
		fprintf(stderr,"dbg2       btime_i[3]:      %d\n",btime_i[3]);
		fprintf(stderr,"dbg2       btime_i[4]:      %d\n",btime_i[4]);
		fprintf(stderr,"dbg2       btime_i[5]:      %d\n",btime_i[5]);
		fprintf(stderr,"dbg2       btime_i[6]:      %d\n",btime_i[6]);
		fprintf(stderr,"dbg2       etime_i[0]:      %d\n",etime_i[0]);
		fprintf(stderr,"dbg2       etime_i[1]:      %d\n",etime_i[1]);
		fprintf(stderr,"dbg2       etime_i[2]:      %d\n",etime_i[2]);
		fprintf(stderr,"dbg2       etime_i[3]:      %d\n",etime_i[3]);
		fprintf(stderr,"dbg2       etime_i[4]:      %d\n",etime_i[4]);
		fprintf(stderr,"dbg2       etime_i[5]:      %d\n",etime_i[5]);
		fprintf(stderr,"dbg2       etime_i[6]:      %d\n",etime_i[6]);
		fprintf(stderr,"dbg2       speedmin:        %f\n",speedmin);
		fprintf(stderr,"dbg2       timegap:         %f\n",timegap);
		fprintf(stderr,"dbg2       input file:      %s\n",ifile);
		fprintf(stderr,"dbg2       output file:     %s\n",ofile);
		fprintf(stderr,"dbg2       ncut:            %d\n",ncut);
		for (i=0;i<ncut;i++)
			fprintf(stderr,"dbg2         kind:%2d mode:%2d min:%f max:%f\n", 
				gkind[i], gmode[i], gmin[i], gmax[i]);
		}

	/* if help desired then print it and exit */
	if (help)
		{
		fprintf(stderr,"\n%s\n",help_message);
		fprintf(stderr,"\nusage: %s\n", usage_message);
		exit(error);
		}

	/* get format_num */
	status = mb_format(verbose,&format,&format_num,&error);

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
	status = mb_malloc(verbose,beams_bath*sizeof(double),&bath,&error);
	status = mb_malloc(verbose,beams_amp*sizeof(double),&amp,&error);
	status = mb_malloc(verbose,beams_bath*sizeof(double),
			&bathacrosstrack,&error);
	status = mb_malloc(verbose,beams_bath*sizeof(double),
			&bathalongtrack,&error);
	status = mb_malloc(verbose,pixels_ss*sizeof(double),&ss,&error);
	status = mb_malloc(verbose,pixels_ss*sizeof(double),
			&ssacrosstrack,&error);
	status = mb_malloc(verbose,pixels_ss*sizeof(double),
			&ssalongtrack,&error);
	status = mb_malloc(verbose,beams_bath*sizeof(int),
			&bathflag,&error);
	status = mb_malloc(verbose,beams_amp*sizeof(int),
			&ampflag,&error);
	status = mb_malloc(verbose,pixels_ss*sizeof(int),
			&ssflag,&error);

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
	sprintf(comment,"This data altered by program %s version %s",
		program_name,rcs_id);
	status = mb_put_comment(verbose,ombio_ptr,comment,&error);
	if (error == MB_ERROR_NO_ERROR) ocomment++;
	sprintf(comment,"MB-system Version %s",MB_VERSION);
	status = mb_put_comment(verbose,ombio_ptr,comment,&error);
	if (error == MB_ERROR_NO_ERROR) ocomment++;
	right_now = time((long *)0);
	strncpy(date,"\0",25);
	right_now = time((long *)0);
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
	if (error == MB_ERROR_NO_ERROR) ocomment++;
	sprintf(comment,"Control Parameters:");
	status = mb_put_comment(verbose,ombio_ptr,comment,&error);
	if (error == MB_ERROR_NO_ERROR) ocomment++;
	sprintf(comment,"  MBIO data format:   %d",format);
	status = mb_put_comment(verbose,ombio_ptr,comment,&error);
	if (error == MB_ERROR_NO_ERROR) ocomment++;
	sprintf(comment,"  Input file:         %s",ifile);
	status = mb_put_comment(verbose,ombio_ptr,comment,&error);
	if (error == MB_ERROR_NO_ERROR) ocomment++;
	sprintf(comment,"  Output file:        %s",ofile);
	status = mb_put_comment(verbose,ombio_ptr,comment,&error);
	if (error == MB_ERROR_NO_ERROR) ocomment++;
	sprintf(comment,"  Longitude flip:     %d",lonflip);
	status = mb_put_comment(verbose,ombio_ptr,comment,&error);
	if (error == MB_ERROR_NO_ERROR) ocomment++;
	sprintf(comment,"  Number of data cut ranges: %d",ncut);
	status = mb_put_comment(verbose,ombio_ptr,comment,&error);
	if (error == MB_ERROR_NO_ERROR) ocomment++;
	for (i=0;i<ncut;i++)
		{
		sprintf(comment,"  kind:%d mode:%d min:%f max:%f",
			gkind[i], gmode[i], gmin[i], gmax[i]);
		status = mb_put_comment(verbose,ombio_ptr,comment,&error);
		if (error == MB_ERROR_NO_ERROR) ocomment++;
		}
	sprintf(comment," ");
	status = mb_put_comment(verbose,ombio_ptr,comment,&error);
	if (error == MB_ERROR_NO_ERROR) ocomment++;

	/* read and write */
	while (error <= MB_ERROR_NO_ERROR)
		{
		/* read some data */
		error = MB_ERROR_NO_ERROR;
		status = MB_SUCCESS;
		status = mb_get_all(verbose,imbio_ptr,&store_ptr,&kind,
				time_i,&time_d,&navlon,&navlat,&speed,
				&heading,&distance,
				&beams_bath,&beams_amp,&pixels_ss,
				bath,amp,bathacrosstrack,bathalongtrack,
				ss,ssacrosstrack,ssalongtrack,
				comment,&error);

		/* increment counter */
		if (error <= MB_ERROR_NO_ERROR 
			&& kind == MB_DATA_DATA)
			idata = idata + pings;
		else if (error <= MB_ERROR_NO_ERROR 
			&& kind == MB_DATA_COMMENT)
			icomment++;

		/* time gaps do not matter to mbcut */
		if (error == MB_ERROR_TIME_GAP)
			{
			status = MB_SUCCESS;
			error = MB_ERROR_NO_ERROR;
			}

		/* non-survey data do not matter to mbcut */
		if (error == MB_ERROR_OTHER)
			{
			status = MB_SUCCESS;
			error = MB_ERROR_NO_ERROR;
			}

		/* output error messages */
		if (verbose >= 1 && error == MB_ERROR_COMMENT)
			{
			if (icomment == 1)
				fprintf(stderr,"\nComments in Input:\n");
			fprintf(stderr,"%s\n",comment);
			}
		else if (verbose >= 1 && error < MB_ERROR_NO_ERROR
			&& error > MB_ERROR_OTHER)
			{
			mb_error(verbose,error,&message);
			fprintf(stderr,"\nNonfatal MBIO Error:\n%s\n",message);
			fprintf(stderr,"Input Record: %d\n",idata);
			fprintf(stderr,"Time: %d %d %d %d %d %d %d\n",
				time_i[0],time_i[1],time_i[2],
				time_i[3],time_i[4],time_i[5],
				time_i[6]);
			}
		else if (verbose >= 1 && error < MB_ERROR_NO_ERROR)
			{
			mb_error(verbose,error,&message);
			fprintf(stderr,"\nNonfatal MBIO Error:\n%s\n",message);
			fprintf(stderr,"Input Record: %d\n",idata);
			}
		else if (verbose >= 1 && error != MB_ERROR_NO_ERROR 
			&& error != MB_ERROR_EOF)
			{
			mb_error(verbose,error,&message);
			fprintf(stderr,"\nFatal MBIO Error:\n%s\n",message);
			fprintf(stderr,"Last Good Time: %d %d %d %d %d %d %d\n",
				time_i[0],time_i[1],time_i[2],
				time_i[3],time_i[4],time_i[5],
				time_i[6]);
			}

		/* check bathymetry data */
		if (error == MB_ERROR_NO_ERROR && kind == MB_DATA_DATA 
			&& check_bath == MB_YES)
		    {
		    /* set up check flags assuming 
			    everything will be saved */
		    for (i=0;i<beams_bath;i++)
			bathflag[i] = MB_YES;

		    /* now loop over good data ranges */
		    for (icut=0;icut<ncut;icut++)
			{
			/* flag data according to beam number range */
			if (gkind[icut] == MBCUT_DATA_BATH
			    && gmode[icut] == MBCUT_MODE_NUMBER)
			    {
			    istart = MAX((int)gmin[icut], 0);
			    iend = MIN((int)gmax[icut], beams_bath - 1);
			    for (i=istart;i<=iend;i++)
				{
				bathflag[i] = MB_NO;
				}
			    }

			/* flag data according to beam 
				acrosstrack distance */
			if (gkind[icut] == MBCUT_DATA_BATH
			    && gmode[icut] == MBCUT_MODE_DISTANCE)
			    {
			    for (i=0;i<=beams_bath;i++)
				{
				if (bathacrosstrack[i] >= gmin[icut]
				    && bathacrosstrack[i] <= gmax[icut])
					bathflag[i] = MB_NO;
				}
			    }
			}

		    /* now apply flags */
		    for (i=0;i<beams_bath;i++)
			{
			if (bathflag[i] == MB_NO && bath[i] > 0.0
			    && mb_bath_flag_table[format_num] == MB_NO)
			    bath[i] = -bath[i];
			else if (bathflag[i] == MB_NO
			    && bath[i] > 0.0)
			    bath[i] = 0.0;
			}		    
		    }

		/* check amplitude data */
		if (error == MB_ERROR_NO_ERROR && kind == MB_DATA_DATA 
			&& check_amp == MB_YES)
		    {
		    /* set up check flags assuming 
			    everything will be saved */
		    for (i=0;i<beams_amp;i++)
			ampflag[i] = MB_YES;

		    /* now loop over good data ranges */
		    for (icut=0;icut<ncut;icut++)
			{
			/* flag data according to beam number range */
			if (gkind[icut] == MBCUT_DATA_AMP
			    && gmode[icut] == MBCUT_MODE_NUMBER)
			    {
			    istart = MAX((int)gmin[icut], 0);
			    iend = MIN((int)gmax[icut], beams_amp - 1);
			    for (i=istart;i<=iend;i++)
				{
				ampflag[i] = MB_NO;
				}
			    }

			/* flag data according to beam 
				acrosstrack distance */
			if (gkind[icut] == MBCUT_DATA_AMP
			    && gmode[icut] == MBCUT_MODE_DISTANCE)
			    {
			    for (i=0;i<=beams_amp;i++)
				{
				if (bathacrosstrack[i] >= gmin[icut]
				    && bathacrosstrack[i] <= gmax[icut])
					ampflag[i] = MB_NO;
				}
			    }
			}

		    /* now apply flags */
		    for (i=0;i<beams_amp;i++)
			{
			if (ampflag[i] == MB_NO && amp[i] > 0.0
			    && mb_amp_flag_table[format_num])
			    amp[i] = -amp[i];
			else if (ampflag[i] == MB_NO 
			    && amp[i] > 0.0)
			    amp[i] = 0.0;
			}
		    }

		/* check sidescan data */
		if (error == MB_ERROR_NO_ERROR && kind == MB_DATA_DATA 
			&& check_ss == MB_YES)
		    {
		    /* set up check flags assuming 
			    everything will be saved */
		    for (i=0;i<pixels_ss;i++)
			ssflag[i] = MB_YES;

		    /* now loop over good data ranges */
		    for (icut=0;icut<ncut;icut++)
			{
			/* flag data according to pixel number range */
			if (gkind[icut] == MBCUT_DATA_SS
			    && gmode[icut] == MBCUT_MODE_NUMBER)
			    {
			    istart = MAX((int)gmin[icut], 0);
			    iend = MIN((int)gmax[icut], pixels_ss - 1);
			    for (i=istart;i<=iend;i++)
				{
				ssflag[i] = MB_NO;
				}
			    }

			/* flag data according to pixel 
				acrosstrack distance */
			if (gkind[icut] == MBCUT_DATA_SS
			    && gmode[icut] == MBCUT_MODE_DISTANCE)
			    {
			    for (i=0;i<=pixels_ss;i++)
				{
				if (ssacrosstrack[i] >= gmin[icut]
				    && ssacrosstrack[i] <= gmax[icut])
					ssflag[i] = MB_NO;
				}
			    }
			}

		    /* now apply flags */
		    for (i=0;i<pixels_ss;i++)
			{
			if (ssflag[i] == MB_NO && ss[i] > 0.0
			    && mb_ss_flag_table[format_num])
			    ss[i] = -ss[i];
			else if (ssflag[i] == MB_NO 
			    && ss[i] > 0.0)
			    ss[i] = 0.0;
			}		    
		    }

		/* write some data */
		if (error == MB_ERROR_NO_ERROR
			|| kind == MB_DATA_COMMENT)
			{
			status = mb_put_all(verbose,ombio_ptr,
					store_ptr,MB_YES,kind,
					time_i,time_d,
					navlon,navlat,speed,heading,
					beams_bath,beams_amp,pixels_ss,
					bath,amp,bathacrosstrack,bathalongtrack,
					ss,ssacrosstrack,ssalongtrack,
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

	/* deallocate memory for data arrays */
	mb_free(verbose,&bath,&error); 
	mb_free(verbose,&amp,&error); 
	mb_free(verbose,&bathacrosstrack,&error); 
	mb_free(verbose,&bathalongtrack,&error); 
	mb_free(verbose,&ss,&error); 
	mb_free(verbose,&ssacrosstrack,&error); 
	mb_free(verbose,&ssalongtrack,&error); 
	mb_free(verbose,&bathflag,&error); 
	mb_free(verbose,&ampflag,&error); 
	mb_free(verbose,&ssflag,&error); 

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
