/*--------------------------------------------------------------------
 *    The MB-system:	mbmask.c	6/15/93
 *    $Id: mbmask.c,v 4.0 1994-03-06 00:13:22 caress Exp $
 *
 *    Copyright (c) 1993,1994 by 
 *    D. W. Caress (caress@lamont.ldgo.columbia.edu)
 *    and D. N. Chayes (dale@lamont.ldgo.columbia.edu)
 *    Lamont-Doherty Earth Observatory
 *    Palisades, NY  10964
 *
 *    See README file for copying and redistribution conditions.
 *--------------------------------------------------------------------*/
/*
 * MBMASK reads a flagging mask file and applies it to the input
 * multibeam data file.  Flagging mask files are created from 
 * multibeam data files using the program MBGETMASK.  If the time
 * tag of a mask record matches that of a data ping, then any
 * beams marked as flagged in the mask are flagged in the data.
 * The utilities MBGETMASK and MBMASK provide a means for transferring
 * editing information from one file to another, provided the files
 * contain versions of the same data.
 * The default input and output multibeam streams are stdin and stdout.
 *
 * Author:	D. W. Caress
 * Date:	June 15, 1993
 * 
 * $Log: not supported by cvs2svn $
 * Revision 4.0  1994/03/01  18:59:27  caress
 * First cut at new version. Any changes are associated with
 * support of three data types (beam bathymetry, beam amplitude,
 * and sidescan) instead of two (bathymetry and backscatter).
 *
 * Revision 1.2  1993/08/26  23:52:22  caress
 * Fixed bug that left it in an infinite loop when the end of
 * the mask file was reached.
 *
 * Revision 1.1  1993/06/21  01:19:27  caress
 * Initial revision
 *
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

/*--------------------------------------------------------------------*/

main (argc, argv)
int argc;
char **argv; 
{
	/* id variables */
	static char rcs_id[] = "$Id: mbmask.c,v 4.0 1994-03-06 00:13:22 caress Exp $";
	static char program_name[] = "MBMASK";
	static char help_message[] = "MBMASK reads a flagging mask file and applies it to the input \nmultibeam data file.  Flagging mask files are created from  \nmultibeam data files using the program MBGETMASK.  If the time \ntag of a mask record matches that of a data ping, then any \nbeams marked as flagged in the mask are flagged in the data. \nThe utilities MBGETMASK and MBMASK provide a means for transferring \nediting information from one file to another, provided the files \ncontain versions of the same data. \nThe default input and output multibeam streams are stdin and stdout.";
	static char usage_message[] = "mbmask [-Fformat -Mmaskfile -Iinfile -Ooutfile -V -H]";

	/* parsing variables */
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
	int	pings;
	int	lonflip;
	double	bounds[4];
	int	btime_i[6];
	int	etime_i[6];
	double	btime_d;
	double	etime_d;
	double	speedmin;
	double	timegap;
	char	ifile[128];
	int	beams_bath;
	int	beams_amp;
	int	pixels_ss;
	char	*imbio_ptr;

	/* MBIO write control parameters */
	char	ofile[128];
	char	*ombio_ptr;

	/* mbio read and write values */
	char	*store_ptr;
	int	kind;
	int	time_i[6];
	double	time_d;
	double	navlon;
	double	navlat;
	double	speed;
	double	heading;
	double	distance;
	int	*bath;
	int	*bathacrosstrack;
	int	*bathalongtrack;
	int	*amp;
	int	*ss;
	int	*ssacrosstrack;
	int	*ssalongtrack;
	int	idata = 0;
	int	icomment = 0;
	int	odata = 0;
	int	ocomment = 0;
	int	flagged = 0;
	int	data_use;
	char	comment[256];

	/* flagging mask variables */
	char	mfile[128];
	int	nmask;
	int	beams_bath_mask;
	int	beams_amp_mask;
	int	mask_time_i[6];
	double	mask_time_d;
	int	*bath_mask;
	int	*amp_mask;
	int	mask_done;
	double	eps = 0.02;

	/* time, user, host variables */
	long int	right_now;
	char	date[25], user[128], host[128];

	int	ready;
	FILE	*fp;
	char	line1[512], line2[512], line3[512], *result;
	int	len, start;
	int	i, j, k, l, m;

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
	etime_i[0] = 2062;
	etime_i[1] = 2;
	etime_i[2] = 21;
	etime_i[3] = 10;
	etime_i[4] = 30;
	etime_i[5] = 0;
	speedmin = 0.0;
	timegap = 1000000000.0;

	/* set default input and output */
	strcpy (ifile, "stdin");
	strcpy (ofile, "stdout");
	strcpy (mfile, "\0");

	/* process argument list */
	while ((c = getopt(argc, argv, "VvHhF:f:M:m:I:i:O:o:")) != -1)
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
		case 'F':
		case 'f':
			sscanf (optarg,"%d", &format);
			flag++;
			break;
		case 'M':
		case 'm':
			sscanf (optarg,"%s", mfile);
			flag++;
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
		fprintf(stderr,"dbg2       data format:    %d\n",format);
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
		fprintf(stderr,"dbg2       etime_i[0]:     %d\n",etime_i[0]);
		fprintf(stderr,"dbg2       etime_i[1]:     %d\n",etime_i[1]);
		fprintf(stderr,"dbg2       etime_i[2]:     %d\n",etime_i[2]);
		fprintf(stderr,"dbg2       etime_i[3]:     %d\n",etime_i[3]);
		fprintf(stderr,"dbg2       etime_i[4]:     %d\n",etime_i[4]);
		fprintf(stderr,"dbg2       etime_i[5]:     %d\n",etime_i[5]);
		fprintf(stderr,"dbg2       speedmin:       %f\n",speedmin);
		fprintf(stderr,"dbg2       timegap:        %f\n",timegap);
		fprintf(stderr,"dbg2       input file:     %s\n",ifile);
		fprintf(stderr,"dbg2       output file:    %s\n",ofile);
		fprintf(stderr,"dbg2       mask file:      %s\n",mfile);
		}

	/* if help desired then print it and exit */
	if (help)
		{
		fprintf(stderr,"\n%s\n",help_message);
		fprintf(stderr,"\nusage: %s\n", usage_message);
		exit(MB_ERROR_NO_ERROR);
		}

	/* open the flagging mask file and read through the comment records */
	nmask = 0;
	start = 1;
	if ((fp = fopen(mfile, "r")) == NULL) 
		{
		error = MB_ERROR_OPEN_FAIL;
		fprintf(stderr,"\nUnable to Open Flagging Mask File <%s> for reading\n",mfile);
		fprintf(stderr,"\nProgram <%s> Terminated\n",
			program_name);
		exit(error);
		}
	ready = MB_NO;
	do
		{
		if ((result = fgets(line1,512,fp)) != line1)
			{
			error = MB_ERROR_OPEN_FAIL;
			fprintf(stderr,"\nUnable to Read beginning of Flagging Mask File <%s>\n",mfile);
			fprintf(stderr,"\nProgram <%s> Terminated\n",
				program_name);
			exit(error);
			}
		if (line1[0] != '#')
			{
			sscanf(line1,"%d %d",
				&beams_bath_mask,&beams_amp_mask);
			ready = MB_YES;
			}
		}
	while (ready == MB_NO);

	/* allocate memory for the flagging mask arrays */
	status = mb_malloc(verbose,beams_bath_mask*sizeof(int),
			&bath_mask,&error);
	status = mb_malloc(verbose,beams_amp_mask*sizeof(int),
			&amp_mask,&error);

	/* if error initializing memory then quit */
	if (error != MB_ERROR_NO_ERROR)
		{
		mb_error(verbose,error,message);
		fprintf(stderr,"\nMBIO Error allocating data arrays:\n%s\n",message);
		fprintf(stderr,"\nProgram <%s> Terminated\n",
			program_name);
		exit(error);
		}

	/* read the first mask record */
	status = read_mask(verbose,beams_bath_mask,beams_amp_mask,fp,
			&nmask,mask_time_i,&mask_time_d,bath_mask,amp_mask,
			&error);

	/* if error reading first mask then quit */
	if (status != MB_SUCCESS)
		{
		mb_error(verbose,error,message);
		fprintf(stderr,"\nMBIO Error reading first flagging mask record:\n%s\n",message);
		fprintf(stderr,"\nProgram <%s> Terminated\n",
			program_name);
		exit(error);
		}
	else
		mask_done = MB_NO;

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
	status = mb_malloc(verbose,beams_bath*sizeof(int),&bath,&error);
	status = mb_malloc(verbose,beams_amp*sizeof(int),&amp,&error);
	status = mb_malloc(verbose,beams_bath*sizeof(int),
			&bathacrosstrack,&error);
	status = mb_malloc(verbose,beams_bath*sizeof(int),
			&bathalongtrack,&error);
	status = mb_malloc(verbose,pixels_ss*sizeof(int),&ss,&error);
	status = mb_malloc(verbose,pixels_ss*sizeof(int),
			&ssacrosstrack,&error);
	status = mb_malloc(verbose,pixels_ss*sizeof(int),
			&ssalongtrack,&error);

	/* if error initializing memory then quit */
	if (error != MB_ERROR_NO_ERROR)
		{
		mb_error(verbose,error,message);
		fprintf(stderr,"\nMBIO Error allocating data arrays:\n%s\n",message);
		fprintf(stderr,"\nProgram <%s> Terminated\n",
			program_name);
		exit(error);
		}

	/* write comments to beginning of output file */
	kind = MB_DATA_COMMENT;
	strncpy(comment,"\0",256);
	sprintf(comment,"Data flagging mask applied to this data by program %s",
		program_name);
	status = mb_put(verbose,ombio_ptr,kind,
			time_i,time_d,
			navlon,navlat,speed,heading,
			beams_bath,beams_amp,pixels_ss,
			bath,amp,bathacrosstrack,bathalongtrack,
			ss,ssacrosstrack,ssalongtrack,
			comment,&error);
	if (error == MB_ERROR_NO_ERROR) ocomment++;
	strncpy(comment,"\0",256);
	sprintf(comment,"Version %s",rcs_id);
	status = mb_put(verbose,ombio_ptr,kind,
			time_i,time_d,
			navlon,navlat,speed,heading,
			beams_bath,beams_amp,pixels_ss,
			bath,amp,bathacrosstrack,bathalongtrack,
			ss,ssacrosstrack,ssalongtrack,
			comment,&error);
	if (error == MB_ERROR_NO_ERROR) ocomment++;
	strncpy(comment,"\0",256);
	sprintf(comment,"MB-system Version %s",MB_VERSION);
	status = mb_put(verbose,ombio_ptr,kind,
			time_i,time_d,
			navlon,navlat,speed,heading,
			beams_bath,beams_amp,pixels_ss,
			bath,amp,bathacrosstrack,bathalongtrack,
			ss,ssacrosstrack,ssalongtrack,
			comment,&error);
	if (error == MB_ERROR_NO_ERROR) ocomment++;
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
			time_i,time_d,
			navlon,navlat,speed,heading,
			beams_bath,beams_amp,pixels_ss,
			bath,amp,bathacrosstrack,bathalongtrack,
			ss,ssacrosstrack,ssalongtrack,
			comment,&error);
	if (error == MB_ERROR_NO_ERROR) ocomment++;
	strncpy(comment,"\0",256);
	sprintf(comment,"Control Parameters:");
	status = mb_put(verbose,ombio_ptr,kind,
			time_i,time_d,
			navlon,navlat,speed,heading,
			beams_bath,beams_amp,pixels_ss,
			bath,amp,bathacrosstrack,bathalongtrack,
			ss,ssacrosstrack,ssalongtrack,
			comment,&error);
	if (error == MB_ERROR_NO_ERROR) ocomment++;
	strncpy(comment,"\0",256);
	sprintf(comment,"  MBIO data format:   %d",format);
	status = mb_put(verbose,ombio_ptr,kind,
			time_i,time_d,
			navlon,navlat,speed,heading,
			beams_bath,beams_amp,pixels_ss,
			bath,amp,bathacrosstrack,bathalongtrack,
			ss,ssacrosstrack,ssalongtrack,
			comment,&error);
	if (error == MB_ERROR_NO_ERROR) ocomment++;
	strncpy(comment,"\0",256);
	sprintf(comment,"  Input file:         %s",ifile);
	status = mb_put(verbose,ombio_ptr,kind,
			time_i,time_d,
			navlon,navlat,speed,heading,
			beams_bath,beams_amp,pixels_ss,
			bath,amp,bathacrosstrack,bathalongtrack,
			ss,ssacrosstrack,ssalongtrack,
			comment,&error);
	if (error == MB_ERROR_NO_ERROR) ocomment++;
	strncpy(comment,"\0",256);
	sprintf(comment,"  Output file:        %s",ofile);
	status = mb_put(verbose,ombio_ptr,kind,
			time_i,time_d,
			navlon,navlat,speed,heading,
			beams_bath,beams_amp,pixels_ss,
			bath,amp,bathacrosstrack,bathalongtrack,
			ss,ssacrosstrack,ssalongtrack,
			comment,&error);
	if (error == MB_ERROR_NO_ERROR) ocomment++;
	strncpy(comment,"\0",256);
	sprintf(comment,"  Mask file:          %s",mfile);
	status = mb_put(verbose,ombio_ptr,kind,
			time_i,time_d,
			navlon,navlat,speed,heading,
			beams_bath,beams_amp,pixels_ss,
			bath,amp,bathacrosstrack,bathalongtrack,
			ss,ssacrosstrack,ssalongtrack,
			comment,&error);
	if (error == MB_ERROR_NO_ERROR) ocomment++;
	strncpy(comment,"\0",256);
	sprintf(comment," ");
	status = mb_put(verbose,ombio_ptr,kind,
			time_i,time_d,
			navlon,navlat,speed,heading,
			beams_bath,beams_amp,pixels_ss,
			bath,amp,bathacrosstrack,bathalongtrack,
			ss,ssacrosstrack,ssalongtrack,
			comment,&error);
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

		/* time gaps do not matter to mbmask */
		if (error == MB_ERROR_TIME_GAP)
			{
			status = MB_SUCCESS;
			error = MB_ERROR_NO_ERROR;
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
			fprintf(stderr,"Time: %d %d %d %d %d %d\n",
				time_i[0],time_i[1],time_i[2],
				time_i[3],time_i[4],time_i[5]);
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
			fprintf(stderr,"Last Good Time: %d %d %d %d %d %d\n",
				time_i[0],time_i[1],time_i[2],
				time_i[3],time_i[4],time_i[5]);
			}

		/* check current mask and read in another if needed */
		if (kind == MB_DATA_DATA
			&& error == MB_ERROR_NO_ERROR)
			{
			while (mask_time_d < time_d - eps && mask_done == MB_NO)
				{
				status = read_mask(verbose,beams_bath_mask,
						beams_amp_mask,fp,
						&nmask,mask_time_i,
						&mask_time_d,
						bath_mask,amp_mask,
						&error);
				if (status == MB_FAILURE)
					{
					mask_done = MB_YES;
					error = MB_ERROR_NO_ERROR;
					status = MB_SUCCESS;	
					}
				}


			/* if the mask fits apply it */
			data_use = MB_NO;
			if (mask_done == MB_NO 
				&& time_d >= mask_time_d - eps 
				&& time_d <= mask_time_d + eps)
				{
				for (j=0;j<beams_bath;j++)
					if (bath_mask[j] == 0
						&& bath[j] > 0)
						{
						bath[j] = -bath[j];
						flagged++;
						data_use = MB_YES;
						}
				for (j=0;j<beams_amp;j++)
					if (amp_mask[j] == 0
						&& amp[j] > 0)
						{
						amp[j] = -amp[j];
						flagged++;
						data_use = MB_YES;
						}
				}
			  }

		/* write some data */
		if (error == MB_ERROR_NO_ERROR
			|| kind == MB_DATA_COMMENT)
			{
			status = mb_put_all(verbose,ombio_ptr,
					store_ptr,data_use,kind,
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
				fprintf(stderr,"Time: %d %d %d %d %d %d\n",
					time_i[0],time_i[1],time_i[2],
					time_i[3],time_i[4],time_i[5]);
				fprintf(stderr,"\nProgram <%s> Terminated\n",
					program_name);
				exit(error);
				}
			}
		}

	/* close the files */
	status = mb_close(verbose,imbio_ptr,&error);
	status = mb_close(verbose,ombio_ptr,&error);
	fclose(fp);

	/* deallocate memory for data arrays */
	mb_free(verbose,bath,&error); 
	mb_free(verbose,amp,&error); 
	mb_free(verbose,bathacrosstrack,&error); 
	mb_free(verbose,bathalongtrack,&error); 
	mb_free(verbose,ss,&error); 
	mb_free(verbose,ssacrosstrack,&error); 
	mb_free(verbose,ssalongtrack,&error); 
	mb_free(verbose,bath_mask,&error);
	mb_free(verbose,amp_mask,&error);

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
		fprintf(stderr,"%d beams flagged\n",flagged);
		}

	/* end it all */
	exit(status);
}
/*--------------------------------------------------------------------*/
int read_mask(verbose,beams_bath,beams_amp,fp,
	nmask,time_i,time_d,mask_bath,mask_amp,error)
int	verbose;
int	beams_bath;
int	beams_amp;
FILE	*fp;
int	*nmask;
int	time_i[6];
double	*time_d;
int	*mask_bath;
int	*mask_amp;
int	*error;
{
	char	*function_name = "read_mask";
	int	status = MB_SUCCESS;
	char	line1[512], line2[512], line3[512];
	char	*result;
	int	len;
	int	i, j;
	

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  HSBATH function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       fp:         %d\n",fp);
		fprintf(stderr,"dbg2       beams_bath: %d\n",beams_bath);
		fprintf(stderr,"dbg2       beams_amp:  %d\n",beams_amp);
		}

	/* read the first flagging mask record */
	if ((result = fgets(line1,512,fp)) != line1)
		status = MB_FAILURE;
	if ((result = fgets(line2,512,fp)) != line2)
		status = MB_FAILURE;
	if ((result = fgets(line3,512,fp)) != line3)
		status = MB_FAILURE;

	/* parse the lines */
	if (status == MB_SUCCESS)
		{
		sscanf(line1,"%d %d %d %d %d %d",
			&time_i[0],
			&time_i[1],
			&time_i[2],
			&time_i[3],
			&time_i[4],
			&time_i[5]);
		mb_get_time(verbose,time_i,time_d,error);
		len = strlen(line2);
		if (len > beams_bath)
			len = beams_bath;
		for (i=0;i<len;i++)
			if (line2[i] == '0')
				mask_bath[i] = 0;
			else
				mask_bath[i] = 1;
		len = strlen(line3);
		if (len > beams_amp)
			len = beams_amp;
		for (i=0;i<len;i++)
			if (line2[i] == '0')
				mask_amp[i] = 0;
			else
				mask_amp[i] = 1;
		nmask++;
		}

	/* check success */
	if (status == MB_SUCCESS)
		*error = MB_ERROR_NO_ERROR;
	else
		*error = MB_ERROR_EOF;

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  HSBATH function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       error:      %d\n",*error);
		fprintf(stderr,"dbg2       time_i[0]:  %d\n",time_i[0]);
		fprintf(stderr,"dbg2       time_i[1]:  %d\n",time_i[1]);
		fprintf(stderr,"dbg2       time_i[2]:  %d\n",time_i[2]);
		fprintf(stderr,"dbg2       time_i[3]:  %d\n",time_i[3]);
		fprintf(stderr,"dbg2       time_i[4]:  %d\n",time_i[4]);
		fprintf(stderr,"dbg2       time_i[5]:  %d\n",time_i[5]);
		fprintf(stderr,"dbg2       time_d:     %f\n",*time_d);
		fprintf(stderr,"dbg2       mask_bath:\ndbg2       ");
		for (i=0;i<beams_bath;i++)
			fprintf(stderr,"%d",mask_bath[i]);
		fprintf(stderr,"\ndbg2       mask_amp:\ndbg2       ");
		for (i=0;i<beams_amp;i++)
			fprintf(stderr,"%d",mask_amp[i]);
		fprintf(stderr,"\n");
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:     %d\n",status);
		}

	/* return status */
	return(status);
}
/*--------------------------------------------------------------------*/
