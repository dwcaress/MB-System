/*--------------------------------------------------------------------
 *    The MB-system:	mbcontour.c	6/4/93
 *    $Id$
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
 * MBCONTOUR is a GMT compatible utility which creates a color postscript
 * contour map of multibeam swath bathymetry. 
 * Complete maps are made by using MBCONTOUR in conjunction with the 
 * usual GMT programs.  The contour levels and colors can be controlled
 * directly or set implicitly using contour and color change intervals.
 * Contours can also be set to have ticks pointing downhill.
 *
 * Author:	D. W. Caress
 * Date:	June 4, 1993
 *
 * $Log: mbcontour.c,v $
 * Revision 5.15  2009/03/02 18:59:05  caress
 * Moving towards 5.1.2beta1.
 *
 * Revision 5.14  2008/09/13 06:08:09  caress
 * Updates to apply suggested patches to segy handling. Also fixes to remove compiler warnings.
 *
 * Revision 5.13  2007/03/02 18:17:38  caress
 * Fixed problems plotting ping/shot number annotation.
 *
 * Revision 5.12  2006/11/10 22:05:37  caress
 * Added ping number/shot number navigation track annotation.
 *
 * Revision 5.11  2006/01/18 07:27:01  caress
 * Fixed to work with GMT4.1
 *
 * Revision 5.10  2005/11/04 20:50:19  caress
 * Programs changed to register arrays through mb_register_array() rather than allocating the memory directly with mb_realloc() or mb_malloc().
 *
 * Revision 5.9  2005/03/25 04:03:10  caress
 * Added control on filename annotation direction.
 *
 * Revision 5.8  2004/12/18 01:26:43  caress
 * Added filename annotation.
 *
 * Revision 5.7  2003/04/17 20:43:37  caress
 * Release 5.0.beta30
 *
 * Revision 5.6  2002/10/02 23:52:37  caress
 * Release 5.0.beta24
 *
 * Revision 5.5  2002/04/06 02:45:59  caress
 * Release 5.0.beta16
 *
 * Revision 5.4  2001/08/10 22:40:02  dcaress
 * Release 5.0.beta07
 *
 * Revision 5.3  2001-07-19 17:29:41-07  caress
 * Release 5.0.beta03
 *
 * Revision 5.2  2001/03/22 21:03:31  caress
 * Trying to make release 5.0.beta0.
 *
 * Revision 5.1  2001/01/22  05:03:25  caress
 * Release 5.0.beta01
 *
 * Revision 5.0  2000/12/01  22:52:16  caress
 * First cut at Version 5.0.
 *
 * Revision 4.24  2000/10/11  00:53:45  caress
 * Converted to ANSI C
 *
 * Revision 4.23  2000/09/30  06:52:17  caress
 * Snapshot for Dale.
 *
 * Revision 4.22  2000/09/11  20:09:14  caress
 * Linked to new datalist parsing functions. Now supports recursive datalists
 * and comments in datalists.
 *
 * Revision 4.21  1998/10/28  21:32:29  caress
 * Fixed handling of data with variable numbers of beams.
 *
 * Revision 4.20  1998/10/04  04:18:07  caress
 * MB-System version 4.6beta
 *
 * Revision 4.19  1997/10/07  20:10:51  caress
 * Fixed tick flag for contours read from file.
 *
 * Revision 4.18  1997/04/21  16:53:56  caress
 * MB-System 4.5 Beta Release.
 *
 * Revision 4.18  1997/04/17  15:05:49  caress
 * MB-System 4.5 Beta Release
 *
 * Revision 4.17  1996/09/05  13:58:27  caress
 * Added feature to check data bounds in ".inf" files.
 *
 * Revision 4.16  1996/04/22  13:18:44  caress
 * Now have DTR and MIN/MAX defines in mb_define.h
 *
 * Revision 4.15  1995/11/28  21:06:16  caress
 * Fixed scaling for meters to feet.
 *
 * Revision 4.14  1995/11/22  22:13:02  caress
 * Now handles bathymetry in feet with -W option.
 *
 * Revision 4.13  1995/11/02  19:24:45  caress
 * Fixed mb_error calls.
 *
 * Revision 4.12  1995/09/29  18:09:17  caress
 * Enabled mbio using larger bounds than plot bounds.
 *
 * Revision 4.11  1995/08/07  17:31:39  caress
 * Moved to GMT V3.
 *
 * Revision 4.10  1995/06/05  12:41:03  caress
 * Now checks navlon != navlon_old || navlat != navlat_old instead
 * of doing &
 *
 * Revision 4.9  1995/05/12  17:19:02  caress
 * Made exit status values consistent with Unix convention.
 * 0: ok  nonzero: error
 *
 * Revision 4.8  1995/03/06  19:39:52  caress
 * Changed include strings.h to string.h for POSIX compliance.
 *
 * Revision 4.7  1995/01/05  23:59:20  caress
 * Made it possible to read data from a single file or
 * from datalists.
 *
 * Revision 4.6  1994/12/21  20:23:30  caress
 * Allows plotting in triangle mode when ship's lon and lat
 * doesn't change.
 *
 * Revision 4.5  1994/10/21  11:34:20  caress
 * Release V4.0
 *
 * Revision 4.5  1994/10/21  11:34:20  caress
 * Release V4.0
 *
 * Revision 4.4  1994/07/29  19:04:31  caress
 * Changes associated with supporting byte swapped Lynx OS and
 * >> using unix second time base.
 *
 * Revision 4.3  1994/06/13  19:01:13  caress
 * Fixed typos in info messages.
 *
 * Revision 4.2  1994/06/13  18:39:15  caress
 * Made it possible to read data from stdin.
 *
 * Revision 4.1  1994/05/16  22:15:16  caress
 * First cut at new contouring scheme.  Additional major changes
 * include adding annotated ship track plot and isolation
 * of plot initialization.
 *
 * Revision 4.1  1994/05/16  22:15:16  caress
 * First cut at new contouring scheme.  Additional major changes
 * include adding annotated ship track plot and isolation
 * of plot initialization.
 *
 * Revision 4.0  1994/03/05  23:46:48  caress
 * First cut at version 4.0
 *
 * Revision 4.0  1994/03/05  23:46:48  caress
 * First cut at version 4.0
 *
 * Revision 4.1  1994/03/03  03:48:58  caress
 * Fixed copyright message.
 *
 * Revision 4.0  1994/02/26  23:07:51  caress
 * First cut at new version.
 *
 * Revision 3.3  1993/11/03  21:09:10  caress
 * Changed ps_plotinit call to agree with current version
 * of GMT (v2.1.4).
 *
 * Revision 3.2  1993/08/16  22:57:49  caress
 * Changed ad hoc string justification.  Used to assume letter width
 * is 0.5 letter height, but now assumes letter width is 0.37 letter
 * height.
 *
 * Revision 3.1  1993/06/21  00:04:52  caress
 * Fixed help message.
 *
 * Revision 3.0  1993/06/19  01:17:26  caress
 * Initial version
 * ls
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
#include "../../include/mb_status.h"
#include "../../include/mb_format.h"
#include "../../include/mb_define.h"
#include "../../include/mb_aux.h"

/* GMT argument handling define */
#define MBCONTOUR_GMT_ARG_MAX 128

/* label and contour line length */
#define MBCONTOUR_LABEL_LEN 128

int ping_copy(int verbose, int one, int two, struct swath *swath, int *error);

static char rcs_id[] = "$Id$";

/*--------------------------------------------------------------------*/

int main (int argc, char **argv) 
{
#ifdef MBCONTOURFILTER
	char program_name[] = "MBCONTOURFILTER";
	char help_message[] =  "MBCONTOURFILTER is a utility which creates a pen plot \ncontour map of multibeam swath bathymetry.  \nThe primary purpose of this program is to serve as \npart of a real-time plotting system.  The contour \nlevels and colors can be controlled \ndirectly or set implicitly using contour and color change intervals. \nContours can also be set to have ticks pointing downhill.";
	char usage_message[] = "mbcontourfilter -Jparameters -Rwest/east/south/north \n\t[-Acontour_int/color_int/tick_int/label_int/tick_len/label_hgt/label_spacing \n\t-Btickinfo -byr/mon/day/hour/min/sec -Ccontourfile \n\t-Dtime_tick/time_annot/date_annot/time_tick_len -Eyr/mon/day/hour/min/sec \n\t-fformat -Fred/green/blue -Idatalist -K -Llonflip -M -O -Nnplot \n\t-P -ppings -Q -Ttimegap -U -Xx-shift -Yy-shift -Zalgorithm -#copies -V -H]";
#else
	char program_name[] = "MBCONTOUR";
	char help_message[] =  "MBCONTOUR is a GMT compatible utility which creates a color postscript \ncontour map of multibeam swath bathymetry.  \nComplete maps are made by using MBCONTOUR in conjunction with the  \nusual GMT programs.  The contour levels and colors can be controlled \ndirectly or set implicitly using contour and color change intervals. \nContours can also be set to have ticks pointing downhill.";
	char usage_message[] = "mbcontour -Jparameters -Rwest/east/south/north \n\t[-Acontour_int/color_int/tick_int/label_int/tick_len/label_hgt/label_spacing \n\t-Btickinfo -byr/mon/day/hour/min/sec -Ccontourfile -ccopies \n\t-Dtime_tick/time_annot/date_annot/time_tick_len \n\t-Eyr/mon/day/hour/min/sec \n\t-fformat -Fred/green/blue -Idatalist -K -Llonflip -Nnplot -O \n\t-P -ppings -U -Xx-shift -Yy-shift -Zalgorithm -V -H]";
#endif

	extern char *optarg;
	int     argc_gmt = 0;
	char    *argv_gmt[MBCONTOUR_GMT_ARG_MAX];
	int	errflg = 0;
	int	c;
	int	help = 0;
	int	flag = 0;

	/* MBIO status variables */
	int	status = MB_SUCCESS;
	int	verbose = 0;
	int	error = MB_ERROR_NO_ERROR;
	char	*message = NULL;

	/* MBIO read control parameters */
	char	read_file[MBCONTOUR_LABEL_LEN];
        int     read_datalist;
	int	read_data;
	void	*datalist;
	int	look_processed = MB_DATALIST_LOOK_UNSET;
	double	file_weight;
	FILE	*fp;
	int	format;
	int	pings;
	int	lonflip;
	int	lonflip_set = MB_NO;
	double	bounds[4];
	double	mb_bounds[4];
	int	btime_i[7];
	int	etime_i[7];
	double	btime_d;
	double	etime_d;
	double	speedmin;
	double	timegap;
	char	file[MB_PATH_MAXLINE];
	int	file_in_bounds;
	int	beams_bath;
	int	beams_amp;
	int	pixels_ss;
	void	*mbio_ptr = NULL;

	/* mbio read values */
	struct swath *swath_plot = NULL;
	struct ping *pingcur = NULL;
	int	kind;
	int	pings_read;
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
	double	*bathlon = NULL;
	double	*bathlat = NULL;
	double	*amp = NULL;
	double	*ss = NULL;
	double	*sslon = NULL;
	double	*sslat = NULL;
	char	comment[256];
	int	pingnumber;

	/* plot control variables */
	int	contour_algorithm;
	char	contourfile[MB_PATH_MAXLINE];
	int	plot;
	int	done;
	int	flush;
	int	save_new;
	int	*npings = NULL;
	int	nping_read;
	int	nplot;
	int	plot_contours;
	int	plot_triangles;
	int	set_contours;
	double	cont_int;
	double	col_int;
	double	tick_int;
	double	label_int;
	double	tick_len;
	double	label_hgt;
	double	label_spacing;
	double	tick_len_map;
	double	label_hgt_map;
	double	label_spacing_map;
	int 	plot_name;
	int	plotted_name;
	int	plot_track;
	double	time_tick_int;
	double	time_annot_int;
	double	date_annot_int;
	double	time_tick_len;
	double	time_tick_len_map;
	double	name_hgt;
	double	name_hgt_map;
	int	name_perp;
	double	scale;
	int	bathy_in_feet;
	int	plot_pingnumber;
	int	pingnumber_tick_int;
	int	pingnumber_annot_int;
	double	pingnumber_tick_len;
	double	pingnumber_tick_len_map;

	/* pen variables */
	int	ncolor;
	int	nlevel;
	double	*level = NULL;
	int	*red = NULL;
	int	*green = NULL;
	int	*blue = NULL;
	int	*label = NULL;
	int	*tick = NULL;

	/* inch to map scale */
	double	inchtolon;

	/* other variables */
	char	line[MBCONTOUR_LABEL_LEN];
	char	labelstr[MBCONTOUR_LABEL_LEN], tickstr[MBCONTOUR_LABEL_LEN];
	int	count;
	int	setcolors;
	double	navlon_old;
	double	navlat_old;
	int	i1, i2;
	double	d1, d2, d3, d4, d5, d6, d7;
	int	nscan;
	int	i;

	/* get current mb default values */
	status = mb_defaults(verbose,&format,&pings,&lonflip,bounds,
		btime_i,etime_i,&speedmin,&timegap);

	/* set default input to stdin */
	strcpy (read_file, "stdin");

	/* initialize some values */
	format = -1;
        read_datalist = MB_NO;
	contour_algorithm = MB_CONTOUR_OLD;
	strcpy (contourfile,"\0");
	set_contours = MB_NO;
	bounds[0] = 0.0;
	bounds[1] = 0.0;
	bounds[2] = 0.0;
	bounds[3] = 0.0;
	scale = 0.0;
	nplot = 0;
	cont_int = 25.;
	col_int = 100.;
	tick_int = 100.;
	label_int = 100.;
	label_hgt = 0.1;
	label_spacing = 0.0;
	tick_len = 0.05;
	plot_name = MB_NO;
	plot_track = MB_NO;
	time_tick_int = 0.25;
	time_annot_int = 1.0;
	date_annot_int = 4.0;
	time_tick_len = 0.1;
	name_hgt = 0.1;
	name_perp = MB_NO;
	ncolor = 4;
	nlevel = 0;
	plot_contours = MB_NO;
	plot_triangles = MB_NO;
	bathy_in_feet = MB_NO;
	plot_pingnumber = MB_NO;
	pingnumber_tick_int = 50;
	pingnumber_annot_int = 100;
	pingnumber_tick_len = 0.1;

	/* get GMT options into separate argv */
	argv_gmt[0] = argv[0];
	argc_gmt = 1;
	for (i=1;i<argc;i++)
	  {
	  if (argv[i][0] == '-')
	    {
	    switch (argv[i][1])
	        {
		case 'B':
		case 'F':
		case 'J':
		case 'j':
		case 'K':
		case 'k':
		case 'O':
		case 'o':
		case 'P':
		case 'R':
		case 'r':
		case 'U':
		case 'u':
		case 'V':
		case 'v':
		case 'X':
		case 'x':
		case 'Y':
		case 'y':
		case '#':
		        if (argc_gmt < MBCONTOUR_GMT_ARG_MAX)
			  {
			  argv_gmt[argc_gmt] = argv[i];
			  argc_gmt++;
			  break;
			  }    
		}
	    }
	  }

	/* deal with mb options */
	while ((c = getopt(argc, argv, "VvHhA:a:B:b:C:c:D:d:E:e:F:f:G:g:I:i:J:j:KkL:l:M:m:N:n:OoPp:QqR:r:S:s:T:t:UuWwX:x:Y:y:Z:z:")) != -1)
	  switch (c) 
		{
		case 'A':
		case 'a':
			nscan = sscanf (optarg, "%lf/%lf/%lf/%lf/%lf/%lf/%lf",
					&d1,&d2,&d3,&d4,&d5,&d6,&d7);
			if (nscan >= 1)
			    cont_int = d1;
			if (nscan >= 2)
			    col_int = d2;
			if (nscan >= 3)
			    tick_int = d3;
			if (nscan >= 4)
			    label_int = d4;
			if (nscan >= 5)
			    tick_len = d5;
			if (nscan >= 6)
			    label_hgt = d6;
			if (nscan >= 7)
			    label_spacing = d7;
			if (nscan >= 1)
			    plot_contours = MB_YES;
			break;
		case 'b':
			sscanf (optarg, "%d/%d/%d/%d/%d/%d",
				&btime_i[0],&btime_i[1],&btime_i[2],
				&btime_i[3],&btime_i[4],&btime_i[5]);
			btime_i[6] = 0;
			break;
		case 'C':
			sscanf (optarg,"%s", contourfile);
			plot_contours = MB_YES;
			set_contours = MB_YES;
			break;
		case 'D':
		case 'd':
			sscanf (optarg, "%lf/%lf/%lf/%lf",
				&time_tick_int,&time_annot_int,
				&date_annot_int,&time_tick_len);
			plot_track = MB_YES;
			break;
		case 'E':
		case 'e':
			sscanf (optarg, "%d/%d/%d/%d/%d/%d",
				&etime_i[0],&etime_i[1],&etime_i[2],
				&etime_i[3],&etime_i[4],&etime_i[5]);
			etime_i[6] = 0;
			break;
                case 'f':
                        sscanf (optarg, "%d",&format);
                        break;
		case 'G':
		case 'g':
			nscan = sscanf (optarg, "%lf/%d", &name_hgt, &name_perp);
			plot_name = MB_YES;
			if (nscan < 1)
			    name_hgt = 0.1;
			if (nscan < 2)
			    name_perp = MB_NO;
			break;
		case 'H':
		case 'h':
			help++;
			break;
		case 'I':
		case 'i':
			sscanf (optarg,"%s", read_file);
			flag++;
			break;
		case 'J':
			if (optarg[0] == 'm')
				sscanf (&optarg[1],"%lf", &scale);
			flag++;
			break;
		case 'L':
		case 'l':
			sscanf (optarg,"%d", &lonflip);
			lonflip_set = MB_YES;
			flag++;
			break;
		case 'M':
		case 'm':
			nscan = sscanf (optarg, "%d/%d/%lf",
					&i1,&i2,&d3);
			if (nscan >= 1)
			    pingnumber_tick_int = i1;
			if (nscan >= 2)
			    pingnumber_annot_int = i2;
			if (nscan >= 3)
			    pingnumber_tick_len = d3;
			if (nscan >= 1)
			    plot_pingnumber = MB_YES;
			break;
		case 'N':
		case 'n':
			sscanf (optarg,"%d", &nplot);
			if (nplot < 3) nplot = 3;
			break;
		case 'p':
			sscanf (optarg,"%d", &pings);
			flag++;
			break;
		case 'Q':
		case 'q':
			plot_triangles = MB_YES;
			break;
		case 'R':
		case 'r':
			sscanf(optarg,"%lf/%lf/%lf/%lf",
				&bounds[0],&bounds[1],
				&bounds[2],&bounds[3]);
			break;
		case 'S':
		case 's':
			sscanf (optarg, "%lf", &speedmin);
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
		case 'W':
		case 'w':
			bathy_in_feet = MB_YES;
			break;
		case 'Z':
		case 'z':
			sscanf (optarg,"%d", &contour_algorithm);
			flag++;
			break;
		case 'B':
		case 'F':
		case 'K':
		case 'O':
		case 'P':
		case 'U':
		case 'X':
		case 'x':
		case 'Y':
		case 'y':
		case '#':
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

	/* set number of pings to be plotted if not set */
	if (nplot == 0 && contour_algorithm == MB_CONTOUR_TRIANGLES)
		nplot = 5;
	else if (nplot == 0)
		nplot = 50;

	/* if nothing set to be plotted, plot contours and track */
	if (plot_contours == MB_NO && plot_triangles == MB_NO 
		&& plot_track == MB_NO && plot_pingnumber == MB_NO)
		{
		plot_contours = MB_YES;
		plot_track = MB_YES;
		}
	if (plot_name == MB_YES && plot_track == MB_NO 
		&& plot_pingnumber == MB_NO)
		{
		plot_track = MB_YES;
		}
	if (plot_track == MB_NO 
		&& plot_pingnumber == MB_YES)
		{
		plot_track = MB_YES;
		time_tick_int = 10000000.0;
		time_annot_int = 10000000.0;
		date_annot_int = 10000000.0;
		}

	/* print starting message */
	if (verbose == 1 || help)
		{
		fprintf(stderr,"\nProgram %s\n",program_name);
		fprintf(stderr,"Version %s\n",rcs_id);
		fprintf(stderr,"MB-system Version %s\n",MB_VERSION);
		}

	/* if help desired then print it and exit */
	if (help)
		{
		fprintf(stderr,"\n%s\n",help_message);
		fprintf(stderr,"\nusage: %s\n", usage_message);
		exit(error);
		}

	/* initialize plotting */
	status = plot_init(verbose,argc_gmt,argv_gmt,bounds,&scale,&inchtolon,&error);

	/* if error flagged then print it and exit */
	if (status == MB_FAILURE)
		{
		fprintf(stderr,"usage: %s\n", usage_message);
		fprintf(stderr,"GMT option error\n");
		fprintf(stderr,"\nProgram <%s> Terminated\n",
			program_name);
		error = MB_ERROR_BAD_USAGE;
		exit(error);
		}

	/* print starting debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  Program <%s>\n",program_name);
		fprintf(stderr,"dbg2  Version %s\n",rcs_id);
		fprintf(stderr,"dbg2  MB-system Version %s\n",MB_VERSION);
		fprintf(stderr,"dbg2  Control Parameters:\n");
		fprintf(stderr,"dbg2       verbose:              %d\n",verbose);
		fprintf(stderr,"dbg2       help:                 %d\n",help);
		fprintf(stderr,"dbg2       format:               %d\n",format);
		fprintf(stderr,"dbg2       pings:                %d\n",pings);
		fprintf(stderr,"dbg2       lonflip:              %d\n",lonflip);
		fprintf(stderr,"dbg2       btime_i[0]:           %d\n",btime_i[0]);
		fprintf(stderr,"dbg2       btime_i[1]:           %d\n",btime_i[1]);
		fprintf(stderr,"dbg2       btime_i[2]:           %d\n",btime_i[2]);
		fprintf(stderr,"dbg2       btime_i[3]:           %d\n",btime_i[3]);
		fprintf(stderr,"dbg2       btime_i[4]:           %d\n",btime_i[4]);
		fprintf(stderr,"dbg2       btime_i[5]:           %d\n",btime_i[5]);
		fprintf(stderr,"dbg2       btime_i[6]:           %d\n",btime_i[6]);
		fprintf(stderr,"dbg2       etime_i[0]:           %d\n",etime_i[0]);
		fprintf(stderr,"dbg2       etime_i[1]:           %d\n",etime_i[1]);
		fprintf(stderr,"dbg2       etime_i[2]:           %d\n",etime_i[2]);
		fprintf(stderr,"dbg2       etime_i[3]:           %d\n",etime_i[3]);
		fprintf(stderr,"dbg2       etime_i[4]:           %d\n",etime_i[4]);
		fprintf(stderr,"dbg2       etime_i[5]:           %d\n",etime_i[5]);
		fprintf(stderr,"dbg2       etime_i[6]:           %d\n",etime_i[6]);
		fprintf(stderr,"dbg2       speedmin:             %f\n",speedmin);
		fprintf(stderr,"dbg2       timegap:              %f\n",timegap);
		fprintf(stderr,"dbg2       read file:            %s\n",read_file);
		fprintf(stderr,"dbg2       bounds[0]:            %f\n",bounds[0]);
		fprintf(stderr,"dbg2       bounds[1]:            %f\n",bounds[1]);
		fprintf(stderr,"dbg2       bounds[2]:            %f\n",bounds[2]);
		fprintf(stderr,"dbg2       bounds[3]:            %f\n",bounds[3]);
		fprintf(stderr,"dbg2       contour algorithm:    %d\n",contour_algorithm);
		fprintf(stderr,"dbg2       plot contours:        %d\n",plot_contours);
		fprintf(stderr,"dbg2       plot triangles:       %d\n",plot_triangles);
		fprintf(stderr,"dbg2       plot track:           %d\n",plot_track);
		fprintf(stderr,"dbg2       plot_pingnumber:      %d\n",plot_pingnumber);
		fprintf(stderr,"dbg2       plot name:            %d\n",plot_name);
		fprintf(stderr,"dbg2       contour interval:     %f\n",cont_int);
		fprintf(stderr,"dbg2       color interval:       %f\n",col_int);
		fprintf(stderr,"dbg2       tick interval:        %f\n",tick_int);
		fprintf(stderr,"dbg2       label interval:       %f\n",label_int);
		fprintf(stderr,"dbg2       tick length:          %f\n",tick_len);
		fprintf(stderr,"dbg2       label height:         %f\n",label_hgt);
		fprintf(stderr,"dbg2       label spacing:        %f\n",label_spacing);
		fprintf(stderr,"dbg2       number contoured:     %d\n",nplot);
		fprintf(stderr,"dbg2       time tick int:        %f\n",time_tick_int);
		fprintf(stderr,"dbg2       time interval:        %f\n",time_annot_int);
		fprintf(stderr,"dbg2       date interval:        %f\n",date_annot_int);
		fprintf(stderr,"dbg2       time tick length:     %f\n",time_tick_len);
		fprintf(stderr,"dbg2       name height:          %f\n",name_hgt);
		fprintf(stderr,"dbg2       pingnumber tick int:  %d\n",pingnumber_tick_int);
		fprintf(stderr,"dbg2       pingnumber annot int: %d\n",pingnumber_annot_int);
		fprintf(stderr,"dbg2       pingnumber tick len:  %f\n",pingnumber_tick_len);
		fprintf(stderr,"dbg2       bathy_in_feet:        %d\n\n",bathy_in_feet);
		}

	/* if bounds not specified then quit */
	if (bounds[0] >= bounds[1] || bounds[2] >= bounds[3]
		|| bounds[2] <= -90.0 || bounds[3] >= 90.0)
		{
		fprintf(stderr,"\nRegion bounds not properly specified:\n\t%f %f %f %f\n",bounds[0],bounds[1],bounds[2],bounds[3]);
		fprintf(stderr,"\nProgram <%s> Terminated\n",
			program_name);
		error = MB_ERROR_BAD_PARAMETER;
		exit(error);
		}

	/* scale label and tick sizes */
	label_hgt_map = inchtolon * label_hgt;
	label_spacing_map = inchtolon * label_spacing;
	tick_len_map = inchtolon * tick_len;
	time_tick_len_map = inchtolon * time_tick_len;
	name_hgt_map = inchtolon * name_hgt;
	pingnumber_tick_len_map = inchtolon * pingnumber_tick_len;

	/* read contours from file */
	if (set_contours == MB_YES)
		{
		/* open contour file */
		if ((fp = fopen(contourfile,"r")) == NULL)
			{
			error = MB_ERROR_OPEN_FAIL;
			fprintf(stderr,"\nUnable to open contour file: %s\n",
				contourfile);
			fprintf(stderr,"\nProgram <%s> Terminated\n",
				program_name);
			exit(error);
			}

		/* count lines in file */
		nlevel = 0;
		while (fgets(line,MBCONTOUR_LABEL_LEN,fp) != NULL)
			nlevel++;
		fclose(fp);

		/* set number of colors equal to levels */
		ncolor = nlevel;

		/* allocate memory */
		status = mb_mallocd(verbose, __FILE__, __LINE__, nlevel*sizeof(double), (void **)&level,&error);
		status = mb_mallocd(verbose, __FILE__, __LINE__, nlevel*sizeof(int), (void **)&label,&error);
		status = mb_mallocd(verbose, __FILE__, __LINE__, nlevel*sizeof(int), (void **)&tick,&error);
		status = mb_mallocd(verbose, __FILE__, __LINE__, ncolor*sizeof(int), (void **)&red,&error);
		status = mb_mallocd(verbose, __FILE__, __LINE__, ncolor*sizeof(int), (void **)&green,&error);
		status = mb_mallocd(verbose, __FILE__, __LINE__, ncolor*sizeof(int), (void **)&blue,&error);

		/* reopen contour file */
		if ((fp = fopen(contourfile,"r")) == NULL)
			{
			error = MB_ERROR_OPEN_FAIL;
			fprintf(stderr,"\nUnable to open contour file: %s\n",
				contourfile);
			fprintf(stderr,"\nProgram <%s> Terminated\n",
				program_name);
			exit(error);
			}

		/* read contour levels from file */
		nlevel = 0;
		while (fgets(line,MBCONTOUR_LABEL_LEN,fp) != NULL)
			{
			count = sscanf(line,"%lf %s %s %d %d %d",
				&level[nlevel],labelstr,tickstr,
				&red[nlevel],&green[nlevel],&blue[nlevel]);
			setcolors = MB_YES;
			if (count >= 2 && labelstr[0] == 'a')
				label[nlevel] = 1;
			else if (count >= 2 && labelstr[0] == 'n')
				label[nlevel] = 0;
			else
				{
				label[nlevel] = 0;
				setcolors = MB_NO;
				}
			if (count >= 3 && tickstr[0] == 't')
				tick[nlevel] = 1;
			else if (count >= 3 && tickstr[0] == 'n')
				tick[nlevel] = 0;
			else
				{
				tick[nlevel] = 0;
				setcolors = MB_NO;
				}
			if (count < 6 || setcolors == MB_NO)
				{
				red[nlevel] = 0;
				green[nlevel] = 0;
				blue[nlevel] = 0;
				}
			if (count > 0) nlevel++;
			}
		fclose(fp);
		}

	/* else set default colors and use contour intervals */
	else
		{
		/* set defaults */
		nlevel = 0;
		ncolor = 4;

		/* allocate memory */
		status = mb_mallocd(verbose, __FILE__, __LINE__, ncolor*sizeof(int), (void **)&red,&error);
		status = mb_mallocd(verbose, __FILE__, __LINE__, ncolor*sizeof(int), (void **)&green,&error);
		status = mb_mallocd(verbose, __FILE__, __LINE__, ncolor*sizeof(int), (void **)&blue,&error);

		/* set colors */
		red[0] =   0; green[0] =   0; blue[0] =   0; /* black */
		red[1] = 255; green[1] =   0; blue[1] =   0; /* red */
		red[2] =   0; green[2] = 200; blue[2] =   0; /* green */
		red[3] =   0; green[3] =   0; blue[3] = 255; /* blue */
		}

	/* set colors */
	set_colors(ncolor,red,green,blue);

	/* set bounds for multibeam reading larger than
		map borders */
	mb_bounds[0] = bounds[0] - 0.25*(bounds[1] - bounds[0]);
	mb_bounds[1] = bounds[1] + 0.25*(bounds[1] - bounds[0]);
	mb_bounds[2] = bounds[2] - 0.25*(bounds[3] - bounds[2]);
	mb_bounds[3] = bounds[3] + 0.25*(bounds[3] - bounds[2]);
	
	/* set lonflip if possible */
	if (lonflip_set == MB_NO)
		{
		if (mb_bounds[0] < -180.0)
			lonflip = -1;
		else if (mb_bounds[1] > 180.0)
			lonflip = 1;
		else if (lonflip == -1 && mb_bounds[1] > 0.0)
			lonflip = 0;
		else if (lonflip == 1 && mb_bounds[0] < 0.0)
			lonflip = 0;
		}

	/* determine whether to read one file or a list of files */
	if (format < 0)
		read_datalist = MB_YES;

	/* open file list */
	nping_read = 0;
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
	else
	    {
	    strcpy(file,read_file);
	    read_data = MB_YES;
	    }

	/* loop over files in file list */
	if (verbose == 1) 
		fprintf(stderr,"\n");
	while (read_data == MB_YES)
	    {
	    /* check for mbinfo file - get file bounds if possible */
	    status = mb_check_info(verbose, file, lonflip, bounds, 
			    &file_in_bounds, &error);
	    if (status == MB_FAILURE)
		    {
		    file_in_bounds = MB_YES;
		    status = MB_SUCCESS;
		    error = MB_ERROR_NO_ERROR;
		    }

	    /* read if data may be in bounds */
	    if (file_in_bounds == MB_YES)
		{
		/* check for "fast bathymetry" or "fbt" file */
		if (plot_contours == MB_YES)
		    {
		    mb_get_fbt(verbose, file, &format, &error);
		    }

		/* else check for "fast nav" or "fnv" file */
		else if (plot_track == MB_YES || plot_pingnumber == MB_YES)
		    {
		    mb_get_fnv(verbose, file, &format, &error);
		    }
	    
		/* call mb_read_init() */
		if ((status = mb_read_init(
		    verbose,file,format,pings,lonflip,mb_bounds,
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
						    sizeof(double), (void **)&bathlon, &error);
		if (error == MB_ERROR_NO_ERROR)
		    status = mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_BATHYMETRY,
						    sizeof(double), (void **)&bathlat, &error);
		if (error == MB_ERROR_NO_ERROR)
		    status = mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_SIDESCAN, 
						    sizeof(double), (void **)&ss, &error);
		if (error == MB_ERROR_NO_ERROR)
		    status = mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_SIDESCAN, 
						    sizeof(double), (void **)&sslon, &error);
		if (error == MB_ERROR_NO_ERROR)
		    status = mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_SIDESCAN, 
						    sizeof(double), (void **)&sslat, &error);
	
		/* if error initializing memory then quit */
		if (error != MB_ERROR_NO_ERROR)
		    {
		    mb_error(verbose,error,&message);
		    fprintf(stderr,"\nMBIO Error allocating data arrays:\n%s\n",message);
		    fprintf(stderr,"\nProgram <%s> Terminated\n",
			    program_name);
		    exit(error);
		    }
    
		/* initialize contour controls */
		status = mb_contour_init(verbose,&swath_plot,nplot,beams_bath,
				    contour_algorithm,
				    plot_contours,plot_triangles,
				    plot_track,plot_name,plot_pingnumber,
				    cont_int,col_int,tick_int,label_int,
				    tick_len_map,label_hgt_map,label_spacing_map,
				    ncolor,nlevel,level,label,tick,
				    time_tick_int,time_annot_int,
				    date_annot_int,time_tick_len_map,name_hgt_map,
				    pingnumber_tick_int,pingnumber_annot_int,
				    pingnumber_tick_len_map,
				    &error);
		swath_plot->beams_bath = beams_bath;

		/* if error initializing memory then quit */
		if (error != MB_ERROR_NO_ERROR)
		    {
		    mb_error(verbose,error,&message);
		    fprintf(stderr,"\nMBIO Error allocating contour control structure:\n%s\n",message);
		    fprintf(stderr,"\nProgram <%s> Terminated\n",
			    program_name);
		    exit(error);
		    }
    
		/* print message */
		if (verbose >= 2) 
		    fprintf(stderr,"\n");
		if (verbose >= 1)
		    fprintf(stderr,"processing data in %s...\n",file);
    
		/* loop over reading */
		npings = &swath_plot->npings;
		*npings = 0;
		done = MB_NO;
		plotted_name = MB_NO;
		while (done == MB_NO)
		    {
		    /* read the next ping */
		    status = mb_read(verbose,mbio_ptr,&kind,
			    &pings_read,time_i,&time_d,
			    &navlon,&navlat,
			    &speed,&heading,
			    &distance,&altitude,&sonardepth,
			    &beams_bath,&beams_amp,&pixels_ss,
			    beamflag,bath,amp,bathlon,bathlat,
			    ss,sslon,sslat,
			    comment,&error);
			    
		    /* get pingnumber */
		    if (status == MB_SUCCESS)
		    	{
		    	status = mb_pingnumber(verbose,mbio_ptr,&pingnumber,&error);
			}
			    
		    /* copy data to swath_plot */
		    if (status == MB_SUCCESS)
		    	{
		        pingcur = &swath_plot->pings[*npings];
			
			/* make sure enough memory is allocated */
			if (pingcur->beams_bath_alloc < beams_bath)
				{
				status = mb_reallocd(verbose, __FILE__, __LINE__, beams_bath*sizeof(char),
						(void **)&(pingcur->beamflag),&error);
				status = mb_reallocd(verbose, __FILE__, __LINE__, beams_bath*sizeof(double),
						(void **)&(pingcur->bath),&error);
				status = mb_reallocd(verbose, __FILE__, __LINE__, beams_bath*sizeof(double),
						(void **)&(pingcur->bathlon),&error);
				status = mb_reallocd(verbose, __FILE__, __LINE__, beams_bath*sizeof(double),
						(void **)&(pingcur->bathlat),&error);
				pingcur->beams_bath_alloc = beams_bath;
				}
			
			/* insert the data */
			for (i=0;i<7;i++)
				pingcur->time_i[i] = time_i[i];
			pingcur->time_d = time_d;
			pingcur->navlon = navlon;
			pingcur->navlat = navlat;
			pingcur->heading = heading;
			pingcur->beams_bath = beams_bath;
			pingcur->pingnumber = pingnumber;
			for (i=0;i<beams_bath;i++)
				{
				pingcur->beamflag[i] = beamflag[i];
				pingcur->bath[i] = bath[i];
				pingcur->bathlon[i] = bathlon[i];
				pingcur->bathlat[i] = bathlat[i];
				}
			}

		    /* null out any unused beams for formats with
			variable numbers of beams */
		    for (i=beams_bath;i<swath_plot->beams_bath;i++)
			    beamflag[i] = MB_FLAG_NULL;
    
		    /* print debug statements */
		    if (verbose >= 2)
			    {
			    fprintf(stderr,"\ndbg2  Ping read in program <%s>\n",
				    program_name);
			    fprintf(stderr,"dbg2       kind:           %d\n",
				    kind);
			    fprintf(stderr,"dbg2       npings:         %d\n",
				    *npings);
			    fprintf(stderr,"dbg2       time:           %4d %2d %2d %2d %2d %2d %6.6d\n",
				    time_i[0],time_i[1],time_i[2],
				    time_i[3],time_i[4],time_i[5],time_i[6]);
			    fprintf(stderr,"dbg2       navigation:     %f  %f\n",
				    navlon, navlat);
			    fprintf(stderr,"dbg2       beams_bath:     %d\n",
				    beams_bath);
			    fprintf(stderr,"dbg2       beams_amp:      %d\n",
					    beams_amp);
			    fprintf(stderr,"dbg2       pixels_ss:      %d\n",
					    pixels_ss);
			    fprintf(stderr,"dbg2       error:          %d\n",
				    error);
			    fprintf(stderr,"dbg2       status:         %d\n",
				    status);
			    }
    
		    /* scale bathymetry if necessary */
		    if (error == MB_ERROR_NO_ERROR
			    && bathy_in_feet == MB_YES)
			    {
			    for (i=0;i<beams_bath;i++)
				    {
				    bath[i] = 3.2808399 * bath[i];
				    }
			    }
    
		    /* update bookkeeping */
		    if (error == MB_ERROR_NO_ERROR)
			    {
			    /*if (*npings == 0 || 
				    (*npings > 0 
				    && contour_algorithm == MB_CONTOUR_TRIANGLES)
				    || (*npings > 0 
				    && (navlon != navlon_old 
				    || navlat != navlat_old)))*/
				    {
				    nping_read += pings_read;
				    (*npings)++;
				    navlon_old = navlon;
				    navlat_old = navlat;
				    }
			    }
    
		    /* decide whether to plot, whether to 
			    save the new ping, and if done */
		    plot = MB_NO; 
		    flush = MB_NO;
		    if (*npings >= nplot)
			    plot = MB_YES;
		    if (*npings > 0 
			    && (error > MB_ERROR_NO_ERROR
			    || error == MB_ERROR_TIME_GAP
			    || error == MB_ERROR_OUT_BOUNDS
			    || error == MB_ERROR_OUT_TIME
			    || error == MB_ERROR_SPEED_TOO_SMALL))
			    {
			    plot = MB_YES;
			    flush = MB_YES;
			    }
		    save_new = MB_NO;
		    if (error == MB_ERROR_TIME_GAP)
			    save_new = MB_YES;
		    if (error > MB_ERROR_NO_ERROR)
			    done = MB_YES;
    
		    /* if enough pings read in, plot them */
		    if (plot == MB_YES)
			    {
    
			    /* print debug statements */
			    if (verbose >= 2)
				    {
				    fprintf(stderr,"\ndbg2  Plotting %d pings in program <%s>\n",
					    *npings,program_name);
				    for (i=0;i<*npings;i++)
					    {
					    pingcur = &swath_plot->pings[i];
					    fprintf(stderr,"dbg2       %4d  %4d %2d %2d %2d %2d %2d %6.6d\n",
						    i,pingcur->time_i[0],
						    pingcur->time_i[1],
						    pingcur->time_i[2],
						    pingcur->time_i[3],
						    pingcur->time_i[4],
						    pingcur->time_i[5],
						    pingcur->time_i[6]);
					    }
				    }
    
			    /* plot data */
			    if (plot_contours == MB_YES
				    || plot_triangles == MB_YES)
				    mb_contour(verbose,swath_plot,&error);
    
			    /* plot nav track */
			    if (plot_track == MB_YES)
				    mb_track(verbose,swath_plot,&error);
    
			    /* annotate pingnumber */
			    if (plot_pingnumber == MB_YES)
			    	    {
				    mb_trackpingnumber(verbose,swath_plot,&error);
				    }
   
			    if (plot_name == MB_YES && plotted_name == MB_NO) 
			    	    {
				    mb_trackname(verbose,name_perp,swath_plot,file,&error);
				    plotted_name = MB_YES;
				    }
			      
			    /* reorganize data */
			    if (flush == MB_YES && save_new == MB_YES)
				    {
				    status = ping_copy(verbose,0,*npings,
					    swath_plot,&error);
				    *npings = 1;
				    }
			    else if (flush == MB_YES)
				    {
				    *npings = 0;
				    }
			    else if (*npings > 1)
				    {
				    status = ping_copy(verbose,0,*npings-1,
						    swath_plot,&error);
				    *npings = 1;
				    }
    
			    }
		    }
		status = mb_close(verbose,&mbio_ptr,&error);
    
		/* deallocate memory for data arrays */
		status = mb_contour_deall(verbose,swath_plot,&error);
		} /* end if file in bounds */
    
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

	/* end plot */
	plot_end(verbose,&error);

	/* deallocate memory for data arrays */
	mb_freed(verbose,__FILE__, __LINE__, (void **)&level,&error);
	mb_freed(verbose,__FILE__, __LINE__, (void **)&label,&error);
	mb_freed(verbose,__FILE__, __LINE__, (void **)&tick,&error);
	mb_freed(verbose,__FILE__, __LINE__, (void **)&red,&error);
	mb_freed(verbose,__FILE__, __LINE__, (void **)&green,&error);
	mb_freed(verbose,__FILE__, __LINE__, (void **)&blue,&error);

	/* print ending info */
	if (verbose >= 1)
		fprintf(stderr,"\n%d pings read and plotted\n",
			nping_read);

	/* check memory */
	if (verbose >= 2)
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
	plot_exit(argc,argv);
	exit(status);
}
/*--------------------------------------------------------------------*/
int ping_copy(int verbose, int one, int two, struct swath *swath, int *error)
{
	char	*function_name = "ping_copy";
	int	status = MB_SUCCESS;

	struct ping	*ping1;
	struct ping	*ping2;
	int	i;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBSWATH function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       one:        %d\n",one);
		fprintf(stderr,"dbg2       two:        %d\n",two);
		fprintf(stderr,"dbg2       swath:      %lu\n",(size_t)swath);
		fprintf(stderr,"dbg2       pings:      %d\n",swath->npings);
		}

	/* copy things */
	ping1 = &swath->pings[one];
	ping2 = &swath->pings[two];
	for (i=0;i<7;i++)
		ping1->time_i[i] = ping2->time_i[i];
	ping1->time_d = ping2->time_d;
	ping1->navlon = ping2->navlon;
	ping1->navlat = ping2->navlat;
	ping1->heading = ping2->heading;
	for (i=0;i<swath->beams_bath;i++)
		{
		ping1->beamflag[i] = ping2->beamflag[i];
		ping1->bath[i] = ping2->bath[i];
		ping1->bathlon[i] = ping2->bathlon[i];
		ping1->bathlat[i] = ping2->bathlat[i];
		}

	/* assume success */
	*error = MB_ERROR_NO_ERROR;
	status = MB_SUCCESS;

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBSWATH function <%s> completed\n",
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
