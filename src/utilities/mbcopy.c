/*--------------------------------------------------------------------
 *    The MB-system:	mbcopy.c	2/4/93
 *    $Id: mbcopy.c,v 4.15 2000-10-11 01:06:15 caress Exp $
 *
 *    Copyright (c) 1993, 1994, 2000 by
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
 * MBCOPY copies an input swath sonar data file to an output
 * swath sonar data file with the specified conversions.  Options include
 * windowing in time and space and ping averaging.  The input and
 * output data formats may differ, though not all possible combinations
 * make sense.  The default input and output streams are stdin 
 * and stdout.
 *
 * Author:	D. W. Caress
 * Date:	February 4, 1993
 *
 * $Log: not supported by cvs2svn $
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
#include <math.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

/* mbio include files */
#include "../../include/mb_status.h"
#include "../../include/mb_format.h"
#include "../../include/mb_define.h"
#include "../../include/mb_io.h"
#include "../../include/mbsys_xse.h"
#include "../../include/mbsys_elacmk2.h"

/* defines for special copying routines */
#define	MBCOPY_PARTIAL		0
#define	MBCOPY_FULL		1
#define	MBCOPY_ELACMK2_TO_XSE	2
#define	MBCOPY_XSE_TO_ELACMK2	3

/*--------------------------------------------------------------------*/

main (int argc, char **argv)
{
	/* id variables */
	static char rcs_id[] = "$Id: mbcopy.c,v 4.15 2000-10-11 01:06:15 caress Exp $";
	static char program_name[] = "MBCOPY";
	static char help_message[] =  "MBCOPY copies an input swath sonar data file to an output \nswath sonar data file with the specified conversions.  Options include \nwindowing in time and space and ping averaging.  The input and \noutput data formats may differ, though not all possible combinations \nmake sense.  The default input and output streams are stdin and stdout.";
	static char usage_message[] = "mbcopy [-Byr/mo/da/hr/mn/sc -Ccommentfile -Eyr/mo/da/hr/mn/sc \n\t-Fiformat/oformat -H  -Iinfile -Llonflip -N -Ooutfile \n\t-Ppings -Qsleep_factor -Rw/e/s/n -Sspeed -V]";

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
	int	iformat = 0;
	int	iformat_num;
	int	pings;
	int	lonflip;
	double	bounds[4];
	int	btime_i[7];
	int	etime_i[7];
	double	btime_d;
	double	etime_d;
	double	speedmin;
	double	timegap;
	char	ifile[128];
	int	ibeams_bath;
	int	ibeams_amp;
	int	ipixels_ss;
	char	*imbio_ptr = NULL;

	/* MBIO write control parameters */
	int	oformat = 0;
	int	oformat_num;
	char	ofile[128];
	int	obeams_bath;
	int	obeams_amp;
	int	opixels_ss;
	char	*ombio_ptr = NULL;

	/* MBIO read and write values */
	struct mb_io_struct *omb_io_ptr;
	struct mb_io_struct *imb_io_ptr;
	char	*istore_ptr;
	char	*ostore_ptr;
	int	kind;
	int	time_i[7];
	double	time_d;
	double	navlon;
	double	navlat;
	double	speed;
	double	heading;
	double	distance;
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
	int	idata = 0;
	int	icomment = 0;
	int	odata = 0;
	int	ocomment = 0;
	int	nbath, namp, nss;
	int	istart_bath, iend_bath, offset_bath;
	int	istart_amp, iend_amp, offset_amp;
	int	istart_ss, iend_ss, offset_ss;
	char	comment[256];
	int	insertcomments = MB_NO;
	char	commentfile[256];
	int	stripcomments = MB_NO;
	int	copymode = MBCOPY_PARTIAL;
	int	use_sleep = MB_NO;
	
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
	int	i, j, k, l, m;

	char	*ctime();
	char	*getenv();

	/* get current default values */
	status = mb_defaults(verbose,&format,&pings,&lonflip,bounds,
		btime_i,etime_i,&speedmin,&timegap);

	/* set default input and output */
	iformat = 0;
	oformat = 0;
	strcpy (commentfile, "\0");
	strcpy (ifile, "stdin");
	strcpy (ofile, "stdout");

	/* process argument list */
	while ((c = getopt(argc, argv, "B:b:C:c:E:e:F:f:HhI:i:L:l:NnO:o:P:p:Q:q:R:r:S:s:T:t:Vv")) != -1)
	  switch (c) 
		{
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
			sscanf (optarg,"%s", commentfile);
			insertcomments = MB_YES;
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
			i = sscanf (optarg,"%d/%d", &iformat,&oformat);
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
			sscanf (optarg,"%lf/%lf/%lf/%lf", 
				&bounds[0],&bounds[1],&bounds[2],&bounds[3]);
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
		fprintf(stderr,"dbg2       input file:     %s\n",ifile);
		fprintf(stderr,"dbg2       output file:    %s\n",ofile);
		fprintf(stderr,"dbg2       insert comments:%d\n",insertcomments);
		fprintf(stderr,"dbg2       comment file:   %s\n",commentfile);
		fprintf(stderr,"dbg2       strip comments: %d\n",stripcomments);
		fprintf(stderr,"dbg2       use sleep:      %d\n",use_sleep);
		fprintf(stderr,"dbg2       sleep factor:   %f\n",sleep_factor);
		}

	/* if help desired then print it and exit */
	if (help)
		{
		fprintf(stderr,"\n%s\n",help_message);
		fprintf(stderr,"\nusage: %s\n", usage_message);
		exit(error);
		}

	/* settle the input/output formats */
	if (iformat <= 0 && oformat <= 0)
		{
		iformat = format;
		oformat = format;
		}
	else if (iformat > 0 && oformat <= 0)
		oformat = iformat;

	/* obtain format array locations - format ids will 
		be aliased to current ids if old format ids given */
	if ((status = mb_format(verbose,&iformat,&iformat_num,&error)) 
		!= MB_SUCCESS)
		{
		mb_error(verbose,error,&message);
		fprintf(stderr,"\nMBIO Error returned from function <mb_format> regarding input format %d:\n%s\n",iformat,message);
		fprintf(stderr,"\nProgram <%s> Terminated\n",
			program_name);
		exit(error);
		}
	if ((status = mb_format(verbose,&oformat,&oformat_num,&error)) 
		!= MB_SUCCESS)
		{
		mb_error(verbose,error,&message);
		fprintf(stderr,"\nMBIO Error returned from function <mb_format> regarding output format %d:\n%s\n",oformat,message);
		fprintf(stderr,"\nProgram <%s> Terminated\n",
			program_name);
		exit(error);
		}

	/* determine if full or partial copies will be made */
	if (pings == 1 
		&& mb_system_table[iformat_num] != MB_SYS_NONE 
		&& mb_system_table[iformat_num] == mb_system_table[oformat_num])
		copymode = MBCOPY_FULL;
	else if (pings == 1 
		&& mb_system_table[iformat_num] == MB_SYS_ELACMK2 
		&& mb_system_table[oformat_num] == MB_SYS_XSE)
		copymode = MBCOPY_ELACMK2_TO_XSE;
	else if (pings == 1 
		&& mb_system_table[iformat_num] == MB_SYS_XSE 
		&& mb_system_table[oformat_num] == MB_SYS_ELACMK2)
		copymode = MBCOPY_XSE_TO_ELACMK2;
	else
		copymode = MBCOPY_PARTIAL;

	/* print debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  Copy mode set in program <%s>\n",
			program_name);
		fprintf(stderr,"dbg2       pings:         %d\n",pings);
		fprintf(stderr,"dbg2       iformat:       %d\n",iformat);
		fprintf(stderr,"dbg2       oformat:       %d\n",oformat);
		fprintf(stderr,"dbg2       isystem:       %d\n",
			mb_system_table[iformat_num]);
		fprintf(stderr,"dbg2       osystem:       %d\n",
			mb_system_table[oformat_num]);
		fprintf(stderr,"dbg2       copymode:      %d\n",copymode);
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

	/* allocate memory for data arrays */
	status = mb_malloc(verbose,ibeams_bath*sizeof(char),&ibeamflag,&error);
	status = mb_malloc(verbose,ibeams_bath*sizeof(double),&ibath,&error);
	status = mb_malloc(verbose,ibeams_bath*sizeof(double),&ibathacrosstrack,
				&error);
	status = mb_malloc(verbose,ibeams_bath*sizeof(double),&ibathalongtrack,
				&error);
	status = mb_malloc(verbose,ibeams_amp*sizeof(double),&iamp,&error);
	status = mb_malloc(verbose,ipixels_ss*sizeof(double),&iss,&error);
	status = mb_malloc(verbose,ipixels_ss*sizeof(double),&issacrosstrack,
				&error);
	status = mb_malloc(verbose,ipixels_ss*sizeof(double),&issalongtrack,
				&error);
	status = mb_malloc(verbose,obeams_bath*sizeof(char),&obeamflag,&error);
	status = mb_malloc(verbose,obeams_bath*sizeof(double),&obath,&error);
	status = mb_malloc(verbose,obeams_bath*sizeof(double),&obathacrosstrack,
				&error);
	status = mb_malloc(verbose,obeams_bath*sizeof(double),&obathalongtrack,
				&error);
	status = mb_malloc(verbose,obeams_amp*sizeof(double),&oamp,&error);
	status = mb_malloc(verbose,opixels_ss*sizeof(double),&oss,&error);
	status = mb_malloc(verbose,opixels_ss*sizeof(double),&ossacrosstrack,
				&error);
	status = mb_malloc(verbose,opixels_ss*sizeof(double),&ossalongtrack,
				&error);

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
	if (variable_beams_table[oformat_num] == MB_YES
		&& obeams_bath != ibeams_bath)
		obeams_bath = ibeams_bath;
	if (variable_beams_table[oformat_num] == MB_YES
		&& obeams_amp != ibeams_amp)
		obeams_amp = ibeams_amp;
	if (variable_beams_table[oformat_num] == MB_YES
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
		sprintf(comment,"This data copied by program %s version %s",
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

	/* read and write */
	while (error <= MB_ERROR_NO_ERROR)
		{
		/* read some data */
		error = MB_ERROR_NO_ERROR;
		status = MB_SUCCESS;
		if (copymode != MBCOPY_PARTIAL)
			status = mb_get_all(verbose,imbio_ptr,&istore_ptr,&kind,
				time_i,&time_d,&navlon,&navlat,&speed,
				&heading,&distance,
				&nbath,&namp,&nss,
				ibeamflag,ibath,iamp,
				ibathacrosstrack,ibathalongtrack,
				iss,issacrosstrack,issalongtrack,
				comment,&error);
		else
			status = mb_get(verbose,imbio_ptr,&kind,&pings,
				time_i,&time_d,&navlon,&navlat,&speed,
				&heading,&distance,
				&nbath,&namp,&nss,
				ibeamflag,ibath,iamp,
				ibathacrosstrack,ibathalongtrack,
				iss,issacrosstrack,issalongtrack,
				comment,&error);

		/* increment counter */
		if (error <= MB_ERROR_NO_ERROR 
			&& kind == MB_DATA_DATA)
			idata = idata + pings;
		else if (error <= MB_ERROR_NO_ERROR 
			&& kind == MB_DATA_COMMENT)
			icomment++;

		/* check numbers of input and output beams */
		if (copymode == MBCOPY_PARTIAL
			&& kind == MB_DATA_DATA
			&& error == MB_ERROR_NO_ERROR
			&& nbath != ibeams_bath)
			{
			ibeams_bath = nbath;
			if (variable_beams_table[oformat_num] == MB_YES)
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
			if (variable_beams_table[oformat_num] == MB_YES)
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
			if (variable_beams_table[oformat_num] == MB_YES)
				opixels_ss = ipixels_ss;
			setup_transfer_rules(verbose,ipixels_ss,opixels_ss,
				&istart_ss,&iend_ss,&offset_ss,&error);
			}
			
		/* time gaps do not matter to mbcopy */
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
			/* do bathymetry */
			for (j=0;j<offset_bath;j++)
				{
				obeamflag[j] = MB_FLAG_NULL;
				obath[j] = 0.0;
				obathacrosstrack[j] = 0.0;
				obathalongtrack[j] = 0.0;
				}
			for (i=istart_bath;i<iend_bath;i++)
				{
				j = i + offset_bath;
				obeamflag[j] = ibeamflag[i];
				obath[j] = ibath[i];
				obathacrosstrack[j] = ibathacrosstrack[i];
				obathalongtrack[j] = ibathalongtrack[i];
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
			imb_io_ptr->new_kind = kind;
			status = mbcopy_elacmk2_to_xse(verbose, 
				    istore_ptr, ostore_ptr, &error);
			}
		else if (copymode == MBCOPY_XSE_TO_ELACMK2
			&& error == MB_ERROR_NO_ERROR)
			{
			ostore_ptr = omb_io_ptr->store_data;
			imb_io_ptr->new_kind = kind;
			status = mbcopy_xse_to_elacmk2(verbose, 
				    istore_ptr, ostore_ptr, &error);
			}

		/* write some data */
		if ((error == MB_ERROR_NO_ERROR && kind != MB_DATA_COMMENT)
			|| (kind == MB_DATA_COMMENT && stripcomments == MB_NO))
			{
			if (copymode != MBCOPY_PARTIAL)
				status = mb_put_all(verbose,ombio_ptr,
					ostore_ptr,MB_NO,kind,
					time_i,time_d,
					navlon,navlat,speed,heading,
					obeams_bath,obeams_amp,opixels_ss,
					obeamflag,obath,oamp,obathacrosstrack,
					obathalongtrack,
					oss,ossacrosstrack,ossalongtrack,
					comment,&error);
			else
				status = mb_put(verbose,ombio_ptr,kind,
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

	/* deallocate memory for data arrays */
	mb_free(verbose,&ibeamflag,&error); 
	mb_free(verbose,&ibath,&error); 
	mb_free(verbose,&ibathacrosstrack,&error); 
	mb_free(verbose,&ibathalongtrack,&error); 
	mb_free(verbose,&iamp,&error); 
	mb_free(verbose,&iss,&error); 
	mb_free(verbose,&issacrosstrack,&error); 
	mb_free(verbose,&issalongtrack,&error); 
	mb_free(verbose,&obeamflag,&error); 
	mb_free(verbose,&obath,&error); 
	mb_free(verbose,&obathacrosstrack,&error); 
	mb_free(verbose,&obathalongtrack,&error); 
	mb_free(verbose,&oamp,&error); 
	mb_free(verbose,&oss,&error); 
	mb_free(verbose,&ossacrosstrack,&error); 
	mb_free(verbose,&ossalongtrack,&error); 

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
		fprintf(stderr,"\ndbg2  MBCOPY function <%s> called\n",
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
		fprintf(stderr,"\ndbg2  MBCOPY function <%s> completed\n",
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
		fprintf(stderr,"\ndbg2  MBCOPY function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       istore:     %d\n",istore);
		fprintf(stderr,"dbg2       ostore:     %d\n",ostore);
		}

	/* copy the data  */
	if (istore != NULL && ostore != NULL 
		&& (char *) istore != (char *) ostore)
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
		ostore->mul_x = istore->longitude;		/* interpolated longitude in degrees */
		ostore->mul_y = istore->latitude;		/* interpolated latitude in degrees */
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
		ostore->sid_frame = MB_NO;	/* boolean flag - sidescan frame read */
		ostore->sid_source = 0;		/* sensor id */
		ostore->sid_sec = 0;		/* sec since 1/1/1901 00:00 */
		ostore->sid_usec = 0;		/* microseconds */
		ostore->sid_ping = 0;		/* ping number */
		ostore->sid_frequency = 0.0;		/* transducer frequency (Hz) */
		ostore->sid_pulse = 0.0;		/* transmit pulse length (sec) */
		ostore->sid_power = 0.0;		/* transmit power (dB) */
		ostore->sid_bandwidth = 0.0;		/* receive bandwidth (Hz) */
		ostore->sid_sample = 0.0;		/* receive sample interval (sec) */
		ostore->sid_bin_size = 0;		/* bin size in mm */
		ostore->sid_offset = 0;		/* lateral offset in mm */
		ostore->sid_num_pixels = 0;		/* number of pixels */
		for (i=0;i<MBSYS_XSE_MAXPIXELS;i++)
		    ostore->ss[i] = 0; /* sidescan amplitude in dB */
	
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
		fprintf(stderr,"\ndbg2  MBCOPY function <%s> completed\n",
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
		fprintf(stderr,"\ndbg2  MBCOPY function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       istore:     %d\n",istore);
		fprintf(stderr,"dbg2       ostore:     %d\n",ostore);
		fprintf(stderr,"dbg2       kind:       %d\n",istore->kind);
		}

	/* copy the data  */
	if (istore != NULL && ostore != NULL 
		&& (char *) istore != (char *) ostore)
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
		ostore->longitude = istore->mul_x;
		ostore->latitude = istore->mul_y;
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
		fprintf(stderr,"\ndbg2  MBCOPY function <%s> completed\n",
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
