/*--------------------------------------------------------------------
 *    The MB-system:	mbsmooth.c	6/12/93
 *    $Id: mbsmooth.c,v 5.2 2001-07-20 00:34:38 caress Exp $
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
 * MBSMOOTH applies a spatial domain gaussian filter to multibeam
 * bathymetry data in order to smooth out noise in multibeam 
 * bathymetry data.
 * The default input and output streams are stdin and stdout.
 *
 * Author:	D. W. Caress
 * Date:	June 12, 1993
 *
 * Acknowledgments:
 * This program is inspired to a large extent by the mb-system
 * version 1 program mbsmooth by Alberto Malinverno (formerly 
 * at L-DEO, now at Schlumberger).  The smoothing mechanism
 * employed in the current version is significantly different
 * than the nearest-neighbor scheme used in Alberto's program,
 * so Alberto should not be held responsible for any shortcomings
 * in the current version.
 *
 * $Log: not supported by cvs2svn $
 * Revision 5.1  2001/03/22 21:15:49  caress
 * Trying to make release 5.0.beta0.
 *
 * Revision 5.0  2000/12/01  22:57:08  caress
 * First cut at Version 5.0.
 *
 * Revision 4.16  2000/10/11  01:06:15  caress
 * Convert to ANSI C
 *
 * Revision 4.15  2000/09/30  07:06:28  caress
 * Snapshot for Dale.
 *
 * Revision 4.14  1998/10/05  19:19:24  caress
 * MB-System version 4.6beta
 *
 * Revision 4.13  1997/10/03  18:59:04  caress
 * Removed unused sort function.
 *
 * Revision 4.12  1997/09/15  19:11:06  caress
 * Real Version 4.5
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
 * Revision 4.9  1996/04/22  13:23:05  caress
 * Now have DTR and MIN/MAX defines in mb_define.h
 *
 * Revision 4.8  1995/06/06  13:31:48  caress
 * Fixed warnings under Solaris by explicit casting of strlen result.
 *
 * Revision 4.7  1995/05/12  17:12:32  caress
 * Made exit status values consistent with Unix convention.
 * 0: ok  nonzero: error
 *
 * Revision 4.6  1995/03/06  19:37:59  caress
 * Changed include strings.h to string.h for POSIX compliance.
 *
 * Revision 4.5  1995/03/02  13:49:21  caress
 * Fixed bug related to error messages.
 *
 * Revision 4.4  1994/10/21  13:02:31  caress
 * Release V4.0
 *
 * Revision 4.3  1994/07/29  19:02:56  caress
 * Changes associated with supporting byte swapped Lynx OS and
 * using unix second time base.
 *
 * Revision 4.2  1994/04/12  00:42:00  caress
 * Changed call to mb_buffer_close in accordance with change
 * in mb_buffer source code.  The parameter list now includes
 * mbio_ptr.
 *
 * Revision 4.1  1994/03/12  01:44:37  caress
 * Added declarations of ctime and/or getenv for compatability
 * with SGI compilers.
 *
 * Revision 4.0  1994/03/06  00:13:22  caress
 * First cut at version 4.0
 *
 * Revision 4.0  1994/03/01  18:59:27  caress
 * First cut at new version. Any changes are associated with
 * support of three data types (beam bathymetry, beam amplitude,
 * and sidescan) instead of two (bathymetry and backscatter).
 *
 * Revision 3.1  1993/11/26  19:30:33  caress
 * Removed annoying print statement.
 *
 * Revision 3.0  1993/11/05  18:11:43  caress
 * Initial version.
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

/* MBIO buffer structure pointer */
#define	MBSMOOTH_BUFFER_DEFAULT	500
#define	MBSMOOTH_NUM		3
void	*buff_ptr;
int	n_buffer_max = MBSMOOTH_BUFFER_DEFAULT;
int	nwant = MBSMOOTH_BUFFER_DEFAULT;
int	nhold = 0;
int	nhold_ping = MBSMOOTH_NUM;
int	nbuff = 0;
int	nload;
int	ndump;

/* ping structure definition */
struct mbsmooth_ping_struct 
	{
	int	id;
	int	time_i[7];
	double	time_d;
	double	navlon;
	double	navlat;
	double	speed;
	double	heading;
	char	*beamflag;
	double	*bath;
	double	*bathacrosstrack;
	double	*bathalongtrack;
	double	*amp;
	double	*ss;
	double	*ssacrosstrack;
	double	*ssalongtrack;
	double	*bathx;
	double	*bathy;
	int	*bathsmooth;
	};
struct mbsmooth_ping_struct ping[MB_BUFFER_MAX];
int	nping = 0;
int	nping_start;
int	nping_end;
int	first = MB_YES;
double	save_time_d = 0.0;

/* gaussian filter parameters */
char	wfile[128];
double	*width = NULL;
double	*factor = NULL;
double	width_def = 250.0;

/*--------------------------------------------------------------------*/

main (int argc, char **argv)
{
	static char rcs_id[] = "$Id: mbsmooth.c,v 5.2 2001-07-20 00:34:38 caress Exp $";
	static char program_name[] = "MBSMOOTH";
	static char help_message[] =  "MBSMOOTH applies a spatial \
domain gaussian filter to swath \nbathymetry data in order to \
smooth out noise in the data.";
	static char usage_message[] = "mbsmooth [-Fformat -Gwidth -Iinfile \
-Llonflip -Nbuffersize -Ooutfile \n\t-Wfilterfile -V -H]";
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
	void	*imbio_ptr = NULL;

	/* MBIO write control parameters */
	char	ofile[128];
	void	*ombio_ptr = NULL;

	/* mbio read and write values */
	void	*store_ptr;
	int	kind;
	int	nrecord = 0;
	int	nbathdata = 0;
	int	ndata = 0;
	char	comment[256];

	/* location processing variables */
	double	mtodeglon;
	double	mtodeglat;
	double	headingx;
	double	headingy;

	/* time, user, host variables */
	time_t	right_now;
	char	date[25], user[128], *user_ptr, host[128];

	/* processing variables */
	int	start;

	FILE	*fp;
	int	done;
	int	nexpect;
	int	jbeg, jend;
	int	ja, jb;
	double	sum, weight, weightsum;
	double	dx, dy;
	int	i, j, ii, jj;

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
	strcpy (wfile, "\0");

	/* process argument list */
	while ((c = getopt(argc, argv, "VvHhF:f:G:g:L:l:I:i:N:n:O:o:W:w:")) != -1)
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
		case 'G':
		case 'g':
			sscanf (optarg,"%lf", &width_def);
			flag++;
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
			sscanf (optarg,"%d", &n_buffer_max);
			if (n_buffer_max > MB_BUFFER_MAX
			    || n_buffer_max < 50)
			    n_buffer_max = MBSMOOTH_BUFFER_DEFAULT;
			flag++;
			break;
		case 'O':
		case 'o':
			sscanf (optarg,"%s", ofile);
			flag++;
			break;
		case 'W':
		case 'w':
			sscanf (optarg,"%s", wfile);
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
		fprintf(stderr,"dbg2       default width:  %f\n",width_def);
		fprintf(stderr,"dbg2       filter file:    %s\n",wfile);
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
		ping[i].bathx = NULL;
		ping[i].bathy = NULL;
		ping[i].bathsmooth = NULL;
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
			&ping[i].bathx,&error);
		status = mb_malloc(verbose,beams_bath*sizeof(double),
			&ping[i].bathy,&error);
		status = mb_malloc(verbose,beams_bath*sizeof(int),
			&ping[i].bathsmooth,&error);
		}
	status = mb_malloc(verbose,beams_bath*sizeof(double),
			&width,&error);
	status = mb_malloc(verbose,beams_bath*sizeof(double),
			&factor,&error);

	/* if error initializing memory then quit */
	if (error != MB_ERROR_NO_ERROR)
		{
		mb_error(verbose,error,&message);
		fprintf(stderr,"\nMBIO Error allocating data arrays:\n%s\n",message);
		fprintf(stderr,"\nProgram <%s> Terminated\n",
			program_name);
		exit(error);
		}

	/* now read in filter widths */
	if ((int)strlen(wfile) > 0)
		{
		if ((fp = fopen(wfile, "r")) == NULL) 
			{
			error = MB_ERROR_OPEN_FAIL;
			fprintf(stderr,"\nUnable to Open Filter Width File <%s> for reading\n",wfile);
			fprintf(stderr,"\nProgram <%s> Terminated\n",
				program_name);
			exit(error);
			}
		for (i=0;i<beams_bath;i++)
		  {
		  if (fscanf(fp,"%d %lf",&j,&width[i]) < 2)
			{
			error = MB_ERROR_OPEN_FAIL;
			fprintf(stderr,"\nOnly found %d filter widths in File <%s> when %d required\n",i,wfile,beams_bath);
			fprintf(stderr,"\nProgram <%s> Terminated\n",
				program_name);
			exit(error);
			}
		  else
			{
			factor[i] = -4.0/(width[i]*width[i]);
			}
		  }
		fclose(fp);
		}
	else
		for (i=0;i<beams_bath;i++)
			{
			width[i] = width_def;
			factor[i] = -4.0/(width[i]*width[i]);
			}

	/* write comments to beginning of output file */
	kind = MB_DATA_COMMENT;
	sprintf(comment,"This bathymetry data smoothed by program %s",
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
	sprintf(comment,"Control Parameters:");
	status = mb_put_comment(verbose,ombio_ptr,comment,&error);
	sprintf(comment,"  MBIO data format:   %d",format);
	status = mb_put_comment(verbose,ombio_ptr,comment,&error);
	sprintf(comment,"  Input file:         %s",ifile);
	status = mb_put_comment(verbose,ombio_ptr,comment,&error);
	sprintf(comment,"  Output file:        %s",ofile);
	status = mb_put_comment(verbose,ombio_ptr,comment,&error);
	sprintf(comment,"  Longitude flip:     %d",lonflip);
	status = mb_put_comment(verbose,ombio_ptr,comment,&error);
	sprintf(comment,"  Filter widths file:   %s",wfile);
	status = mb_put_comment(verbose,ombio_ptr,comment,&error);
	sprintf(comment,"  Filter widths:");
	status = mb_put_comment(verbose,ombio_ptr,comment,&error);
	for (i=0;i<beams_bath;i++)
		{
		sprintf(comment,"    %2d  %10.2f",i,width[i]);
		status = mb_put_comment(verbose,ombio_ptr,comment,&error);
		}
	sprintf(comment," ");
	status = mb_put_comment(verbose,ombio_ptr,comment,&error);

	/* initialize the buffer */
	status = mb_buffer_init(verbose,&buff_ptr,&error);

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
		for (i=0;i<nbuff;i++)
			{
			status = mb_buffer_get_next_data(verbose,
				buff_ptr,imbio_ptr,start,&ping[ndata].id,
				ping[ndata].time_i,&ping[ndata].time_d,
				&ping[ndata].navlon,&ping[ndata].navlat,
				&ping[ndata].speed,&ping[ndata].heading,
				&beams_bath,&beams_amp,&pixels_ss,
				ping[ndata].beamflag,
				ping[ndata].bath,ping[ndata].amp,
				ping[ndata].bathacrosstrack,
				ping[ndata].bathalongtrack,
				ping[ndata].ss,
				ping[2].ssacrosstrack,
				ping[ndata].ssalongtrack,
				&error);
			if (status == MB_SUCCESS && first != MB_YES)
				{
				if (save_time_d == ping[ndata].time_d)
				    jbeg = ndata + 1;
				}
			if (status == MB_SUCCESS)
				{
				start = ping[ndata].id + 1;
				ndata++;
				}
			}

		/* calculate geographical positions for beams */
		if (ndata > 0)
			mb_coor_scale(verbose,ping[0].navlat,
					&mtodeglon,&mtodeglat);
		for (j=0;j<ndata;j++)
			{
			headingx = sin(ping[j].heading*DTR);
			headingy = cos(ping[j].heading*DTR);
			for (i=0;i<beams_bath;i++)
				{
				ping[j].bathx[i] = (ping[j].navlon 
					- ping[0].navlon)/mtodeglon 
					+ headingy*ping[j].bathacrosstrack[i];
				ping[j].bathy[i] = (ping[j].navlat 
					- ping[0].navlat)/mtodeglat 
					- headingx*ping[j].bathacrosstrack[i];
				}
			}

		/* loop over all of the pings and beams */
		if (first == MB_YES)
			jbeg = 0;
		if (done == MB_YES)
			jend = ndata - 1;
		else if (ndata > nhold_ping + jbeg)
			{
			jend = ndata - 1 - nhold_ping;
			save_time_d = ping[jend].time_d;
			}
		else
			{
			jend = ndata - 1;
			save_time_d = ping[jend].time_d;
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
		for (j=jbeg;j<jend;j++)
		  {
		  /* set beginning and end of search */
		  ja = j - MBSMOOTH_NUM;
		  if (ja < 0) ja = 0;
		  jb = j + MBSMOOTH_NUM;
		  if (jb >= ndata) jb = ndata - 1;
		
		  for (i=0;i<beams_bath;i++)
		    {
		    ping[j].bathsmooth[i] = ping[j].bath[i];
		    if (mb_beam_ok(ping[j].beamflag[i]))
		      {
		      sum = 0.0;
		      weightsum = 0.0;
		      for (jj=ja;jj<=jb;jj++)
			for (ii=0;ii<beams_bath;ii++)
			  {
			  if (mb_beam_ok(ping[jj].beamflag[ii]))
			    {
			    dx = ping[jj].bathx[ii] - ping[j].bathx[i];
			    dy = ping[jj].bathy[ii] - ping[j].bathy[i];
			    weight = exp(factor[i]*(dx*dx + dy*dy));
			    sum += ping[jj].bath[ii]*weight;
			    weightsum += weight;
			    }
			  }
		      if (weightsum > 0.0)
			ping[j].bathsmooth[i] = (int) (sum/weightsum);
		      }
		    }
		  }

		/* reset pings in buffer */
		for (j=jbeg;j<jend;j++)
		  {
		  status = mb_buffer_insert(verbose,
				buff_ptr,imbio_ptr,ping[j].id,
				ping[j].time_i,ping[j].time_d,
				ping[j].navlon,ping[j].navlat,
				ping[j].speed,ping[j].heading,
				beams_bath,beams_amp,pixels_ss,
				ping[j].beamflag,
				ping[j].bath,ping[j].amp,
				ping[j].bathacrosstrack,
				ping[j].bathalongtrack,
				ping[j].ss,ping[j].ssacrosstrack,
				ping[j].ssalongtrack,
				comment,&error);
		  }

		/* find number of pings to hold */
		if (done == MB_YES)
			nhold = 0;
		else if (jend <= jbeg)
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
			fprintf(stderr,"%d records dumped from buffer\n\n",ndump);
			}
		}

	/* close the files */
	status = mb_buffer_close(verbose,&buff_ptr,imbio_ptr,&error);
	status = mb_close(verbose,&imbio_ptr,&error);
	status = mb_close(verbose,&ombio_ptr,&error);

	/* free the memory */
	for (j=0;j<3;j++)
		{
		mb_free(verbose,&ping[j].beamflag,&error); 
		mb_free(verbose,&ping[j].bath,&error); 
		mb_free(verbose,&ping[j].bathacrosstrack,&error); 
		mb_free(verbose,&ping[j].bathalongtrack,&error); 
		mb_free(verbose,&ping[j].amp,&error); 
		mb_free(verbose,&ping[j].ss,&error); 
		mb_free(verbose,&ping[j].ssacrosstrack,&error); 
		mb_free(verbose,&ping[j].ssalongtrack,&error); 
		mb_free(verbose,&ping[j].bathx,&error); 
		mb_free(verbose,&ping[j].bathy,&error); 
		mb_free(verbose,&ping[j].bathsmooth,&error); 
		}
	mb_free(verbose,&width,&error); 

	/* check memory */
	if (verbose >= 4)
		status = mb_memory_list(verbose,&error);

	/* give the statistics */
	if (verbose >= 1)
		{
		fprintf(stderr,"\n%d data records processed\n",nrecord);
		fprintf(stderr,"%d survey data records processed\n",nbathdata);
		}

	/* end it all */
	exit(error);
}
/*--------------------------------------------------------------------*/
