/*--------------------------------------------------------------------
 *    The MB-system:	mbcontour.c	6/4/93
 *    $Id: mbcontour.c,v 4.2 1994-06-13 18:39:15 caress Exp $
 *
 *    Copyright (c) 1993, 1994 by 
 *    D. W. Caress (caress@lamont.ldgo.columbia.edu)
 *    and D. N. Chayes (dale@lamont.ldgo.columbia.edu)
 *    Lamont-Doherty Earth Observatory
 *    Palisades, NY  10964
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
 * $Log: not supported by cvs2svn $
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
#include <math.h>
#include <strings.h>
#include <time.h>

/* MBIO include files */
#include "../../include/mb_status.h"
#include "../../include/mb_format.h"
#include "../../include/mb_contour.h"

/* local define */
#define DTR (M_PI/180.)

/*--------------------------------------------------------------------*/

main (argc, argv) 
int argc;
char **argv; 
{
	static char rcs_id[] = "$Id: mbcontour.c,v 4.2 1994-06-13 18:39:15 caress Exp $";
#ifdef MBCONTOURFILTER
	static char program_name[] = "MBCONTOURFILTER";
	static char help_message[] =  "MBCONTOURFILTER is a utility which creates a pen plot \ncontour map of multibeam swath bathymetry.  \nThe primary purpose of this program is to serve as \npart of a real-time plotting system.  The contour levels and colors can be controlled \ndirectly or set implicitly using contour and color change intervals. \nContours can also be set to have ticks pointing downhill.";
	static char usage_message[] = "mbcontourfilter -Jparameters -Rwest/east/south/north \n\t[-Acontour_int/color_int/tick_int/label_int/tick_len/label_hgt \n\t-Btickinfo -Ccontourfile \n\t-Dtime_tick/time_annot/date_annot/time_tick_len \n\t-fformat -Fred/green/blue -Idatalist -K -Llonflip -M -O -P -ppings -U \n\t-Xx-shift -Yy-shift -#copies -V -H]";
#else
	static char program_name[] = "MBCONTOUR";
	static char help_message[] =  "MBCONTOUR is a GMT compatible utility which creates a color postscript \ncontour map of multibeam swath bathymetry.  \nComplete maps are made by using MBCONTOUR in conjunction with the  \nusual GMT programs.  The contour levels and colors can be controlled \ndirectly or set implicitly using contour and color change intervals. \nContours can also be set to have ticks pointing downhill.";
	static char usage_message[] = "mbcontour -Jparameters -Rwest/east/south/north \n\t[-Acontour_int/color_int/tick_int/label_int/tick_len/label_hgt \n\t-Btickinfo -Ccontourfile \n\t-Dtime_tick/time_annot/date_annot/time_tick_len \n\t-fformat -Fred/green/blue -Idatalist -K -Llonflip -M -O -P -ppings -U \n\t-Xx-shift -Yy-shift -#copies -V -H]";
#endif

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

	/* MBIO read control parameters */
	char	filelist[128];
	FILE	*fp;
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
	char	file[128];
	int	beams_bath;
	int	beams_amp;
	int	pixels_ss;
	char	*mbio_ptr;

	/* mbio read values */
	struct swath *swath_plot;
	struct ping *pingcur;
	int	kind;
	int	pings_read;
	int	*time_i;
	double	*time_d;
	double	*navlon;
	double	*navlat;
	double	speed;
	double	*heading;
	double	distance;
	double	*bath;
	double	*bathlon;
	double	*bathlat;
	double	*amp;
	double	*ss;
	double	*sslon;
	double	*sslat;
	char	comment[256];

	/* plot control variables */
	char	contourfile[128];
	int	plot;
	int	done;
	int	flush;
	int	save_new;
	int	first;
	int	*npings;
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
	double	tick_len_map;
	double	label_hgt_map;
	int	plot_track;
	double	time_tick_int;
	double	time_annot_int;
	double	date_annot_int;
	double	time_tick_len;
	double	time_tick_len_map;
	double	scale;

	/* pen variables */
	int	ncolor;
	int	nlevel;
	double	*level;
	int	*red;
	int	*green;
	int	*blue;
	int	*label;
	int	*tick;

	/* inch to map scale */
	double	inchtolon;

	/* other variables */
	char	line[128];
	char	labelstr[128], tickstr[128];
	int	count;
	int	setcolors;
        int     use_stdin;
	int	read_data;
	int	i;

	/* get current mb default values */
	status = mb_defaults(verbose,&format,&pings,&lonflip,bounds,
		btime_i,etime_i,&speedmin,&timegap);

	/* initialize some values */
	strcpy (file, "stdin");
	strcpy (contourfile,"\0");
	set_contours = MB_NO;
	bounds[0] = 0.0;
	bounds[1] = 0.0;
	bounds[2] = 0.0;
	bounds[3] = 0.0;
	scale = 0.0;
	nplot = 20;
	cont_int = 25.;
	col_int = 100.;
	tick_int = 100.;
	label_int = 100.;
	label_hgt = 0.1;
	tick_len = 0.05;
	plot_track = MB_NO;
	time_tick_int = 0.25;
	time_annot_int = 1.0;
	date_annot_int = 4.0;
	time_tick_len = 0.1;
	ncolor = 4;
	nlevel = 0;
	plot_contours = MB_NO;
	plot_triangles = MB_NO;
        use_stdin = MB_YES;

	/* deal with mb options */
	while ((c = getopt(argc, argv, "VvHhA:a:b:C:c:D:d:E:e:f:I:i:L:l:N:n:p:QqS:s:T:t:B:F:MJ:KOPR:UX:x:Y:y:#:")) != -1)
	  switch (c) 
		{
		case 'A':
		case 'a':
			sscanf (optarg, "%lf/%lf/%lf/%lf/%lf/%lf",
					&cont_int,&col_int,
					&tick_int,&label_int,
					&tick_len,&label_hgt);
			plot_contours = MB_YES;
			break;
		case 'b':
			sscanf (optarg, "%d/%d/%d/%d/%d/%d",
				&btime_i[0],&btime_i[1],&btime_i[2],
				&btime_i[3],&btime_i[4],&btime_i[5]);
			break;
		case 'C':
		case 'c':
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
			break;
                case 'f':
                        sscanf (optarg, "%d",&format);
                        break;
		case 'H':
		case 'h':
			help++;
			break;
		case 'I':
		case 'i':
			sscanf (optarg,"%s", filelist);
			use_stdin = MB_NO;
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
			flag++;
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
		exit(MB_FAILURE);
		}

	/* if nothing set to be plotted, plot contours and track */
	if (plot_contours == MB_NO && plot_triangles == MB_NO 
		&& plot_track == MB_NO)
		{
		plot_contours = MB_YES;
		plot_track = MB_YES;
		}

	/* print starting message */
	if (verbose == 1)
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
		exit(MB_ERROR_NO_ERROR);
		}

	/* initialize plotting */
	status = plot_init(verbose,argc,argv,bounds,&scale,&inchtolon,&error);

	/* if error flagged then print it and exit */
	if (status == MB_FAILURE)
		{
		fprintf(stderr,"usage: %s\n", usage_message);
		fprintf(stderr,"GMT option error\n");
		fprintf(stderr,"\nProgram <%s> Terminated\n",
			program_name);
		exit(MB_FAILURE);
		}

	/* print starting debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  Program <%s>\n",program_name);
		fprintf(stderr,"dbg2  Version %s\n",rcs_id);
		fprintf(stderr,"dbg2  MB-system Version %s\n",MB_VERSION);
		fprintf(stderr,"dbg2  Control Parameters:\n");
		fprintf(stderr,"dbg2       verbose:          %d\n",verbose);
		fprintf(stderr,"dbg2       help:             %d\n",help);
		fprintf(stderr,"dbg2       format:           %d\n",format);
		fprintf(stderr,"dbg2       pings:            %d\n",pings);
		fprintf(stderr,"dbg2       lonflip:          %d\n",lonflip);
		fprintf(stderr,"dbg2       btime_i[0]:       %d\n",btime_i[0]);
		fprintf(stderr,"dbg2       btime_i[1]:       %d\n",btime_i[1]);
		fprintf(stderr,"dbg2       btime_i[2]:       %d\n",btime_i[2]);
		fprintf(stderr,"dbg2       btime_i[3]:       %d\n",btime_i[3]);
		fprintf(stderr,"dbg2       btime_i[4]:       %d\n",btime_i[4]);
		fprintf(stderr,"dbg2       btime_i[5]:       %d\n",btime_i[5]);
		fprintf(stderr,"dbg2       etime_i[0]:       %d\n",etime_i[0]);
		fprintf(stderr,"dbg2       etime_i[1]:       %d\n",etime_i[1]);
		fprintf(stderr,"dbg2       etime_i[2]:       %d\n",etime_i[2]);
		fprintf(stderr,"dbg2       etime_i[3]:       %d\n",etime_i[3]);
		fprintf(stderr,"dbg2       etime_i[4]:       %d\n",etime_i[4]);
		fprintf(stderr,"dbg2       etime_i[5]:       %d\n",etime_i[5]);
		fprintf(stderr,"dbg2       speedmin:         %f\n",speedmin);
		fprintf(stderr,"dbg2       timegap:          %f\n",timegap);
		fprintf(stderr,"dbg2       file list:        %s\n",filelist);
		fprintf(stderr,"dbg2       use_stdin:        %d\n",use_stdin);
		fprintf(stderr,"dbg2       bounds[0]:        %f\n",bounds[0]);
		fprintf(stderr,"dbg2       bounds[1]:        %f\n",bounds[1]);
		fprintf(stderr,"dbg2       bounds[2]:        %f\n",bounds[2]);
		fprintf(stderr,"dbg2       bounds[3]:        %f\n",bounds[3]);
		fprintf(stderr,"dbg2       plot contours:    %d\n",
			plot_contours);
		fprintf(stderr,"dbg2       plot triangles:   %d\n",
			plot_triangles);
		fprintf(stderr,"dbg2       plot track:       %d\n",
			plot_track);
		fprintf(stderr,"dbg2       contour interval: %f\n",cont_int);
		fprintf(stderr,"dbg2       color interval:   %f\n",col_int);
		fprintf(stderr,"dbg2       tick interval:    %f\n",tick_int);
		fprintf(stderr,"dbg2       label interval:   %f\n",label_int);
		fprintf(stderr,"dbg2       tick length:      %f\n",tick_len);
		fprintf(stderr,"dbg2       label height:     %f\n",label_hgt);
		fprintf(stderr,"dbg2       number contoured: %d\n",nplot);
		fprintf(stderr,"time tick interval:          %f\n",
			time_tick_int);
		fprintf(stderr,"time interval:               %f\n",
			time_annot_int);
		fprintf(stderr,"date interval:               %f\n",
			date_annot_int);
		fprintf(stderr,"time tick length:            %f\n\n",
			time_tick_len);
		}

	/* if bounds not specified then quit */
	if (bounds[0] >= bounds[1] || bounds[2] >= bounds[3]
		|| bounds[2] <= -90.0 || bounds[3] >= 90.0)
		{
		fprintf(stderr,"\nRegion bounds not properly specified:\n\t%f %f %f %f\n",bounds[0],bounds[1],bounds[2],bounds[3]);
		fprintf(stderr,"\nProgram <%s> Terminated\n",
			program_name);
		exit(MB_ERROR_BAD_PARAMETER);
		}

	/* scale label and tick sizes */
	label_hgt_map = inchtolon*label_hgt;
	tick_len_map = inchtolon*tick_len;
	time_tick_len_map = inchtolon*time_tick_len;

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
		while (fgets(line,128,fp) != NULL)
			nlevel++;
		fclose(fp);

		/* set number of colors equal to levels */
		ncolor = nlevel;

		/* allocate memory */
		status = mb_malloc(verbose,nlevel*sizeof(double),&level,&error);
		status = mb_malloc(verbose,nlevel*sizeof(int),&label,&error);
		status = mb_malloc(verbose,nlevel*sizeof(int),&tick,&error);
		status = mb_malloc(verbose,ncolor*sizeof(int),&red,&error);
		status = mb_malloc(verbose,ncolor*sizeof(int),&green,&error);
		status = mb_malloc(verbose,ncolor*sizeof(int),&blue,&error);

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
		while (fgets(line,128,fp) != NULL)
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
				tick[nlevel] = 1;
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
		status = mb_malloc(verbose,ncolor*sizeof(int),&red,&error);
		status = mb_malloc(verbose,ncolor*sizeof(int),&green,&error);
		status = mb_malloc(verbose,ncolor*sizeof(int),&blue,&error);

		/* set colors */
		red[0] =   0; green[0] =   0; blue[0] =   0; /* black */
		red[1] = 255; green[1] =   0; blue[1] =   0; /* red */
		red[2] =   0; green[2] = 200; blue[2] =   0; /* green */
		red[3] =   0; green[3] =   0; blue[3] = 255; /* blue */
		}

	/* set colors */
	set_colors(ncolor,red,green,blue);

	/* open file list */
	nping_read = 0;
	if (use_stdin == MB_NO)
	if ((fp = fopen(filelist,"r")) == NULL)
		{
		error = MB_ERROR_OPEN_FAIL;
		fprintf(stderr,"\nUnable to open data list file: %s\n",
			filelist);
		fprintf(stderr,"\nProgram <%s> Terminated\n",
			program_name);
		exit(error);
		}

	/* read first file */
	if (use_stdin == MB_NO)
		{
		if (fgets(line,128,fp) != NULL
			&& sscanf(line,"%s %d",file,&format) == 2)
			read_data = MB_YES;
		else
			read_data = MB_NO;
		}
	else
		{
		strcpy(file,"stdin");
		read_data = MB_YES;
		}

	/* loop over files in file list */
	if (verbose == 1) 
		fprintf(stderr,"\n");
	while (read_data == MB_YES)
	{

	/* initialize reading the multibeam file */
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
	status = mb_malloc(verbose,beams_amp*sizeof(double),&amp,&error);
	status = mb_malloc(verbose,pixels_ss*sizeof(double),&ss,&error);
	status = mb_malloc(verbose,pixels_ss*sizeof(double),&sslon,&error);
	status = mb_malloc(verbose,pixels_ss*sizeof(double),&sslat,&error);

	/* if error initializing memory then quit */
	if (error != MB_ERROR_NO_ERROR)
		{
		mb_error(verbose,error,message);
		fprintf(stderr,"\nMBIO Error allocating data arrays:\n%s\n",message);
		fprintf(stderr,"\nProgram <%s> Terminated\n",
			program_name);
		exit(error);
		}

	/* initialize contour controls */
	status = mb_contour_init(verbose,&swath_plot,nplot,beams_bath,
				plot_contours,plot_triangles,plot_track,
				cont_int,col_int,tick_int,label_int,
				tick_len_map,label_hgt_map,
				ncolor,nlevel,level,label,tick,
				time_tick_int,time_annot_int,
				date_annot_int,time_tick_len_map,
				&error);

	/* if error initializing memory then quit */
	if (error != MB_ERROR_NO_ERROR)
		{
		mb_error(verbose,error,message);
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
	while (done == MB_NO)
		{
		pingcur = &swath_plot->pings[*npings];
		time_i = &pingcur->time_i[0];
		time_d = &pingcur->time_d;
		navlon = &pingcur->navlon;
		navlat = &pingcur->navlat;
		heading = &pingcur->heading;
		bath = pingcur->bath;
		bathlon = pingcur->bathlon;
		bathlat = pingcur->bathlat;
		status = mb_read(verbose,mbio_ptr,&kind,
			&pings_read,time_i,time_d,
			navlon,navlat,&speed,
			heading,&distance,
			&beams_bath,&beams_amp,&pixels_ss,
			bath,amp,bathlon,bathlat,
			ss,sslon,sslat,
			comment,&error);

		/* print debug statements */
		if (verbose >= 2)
			{
			fprintf(stderr,"\ndbg2  Ping read in program <%s>\n",
				program_name);
			fprintf(stderr,"dbg2       kind:           %d\n",
				kind);
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

		/* update bookkeeping */
		if (error == MB_ERROR_NO_ERROR)
			{
			nping_read += pings_read;
			(*npings)++;
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
					fprintf(stderr,"dbg2       %4d  %4d %2d %2d %2d %2d %2d\n",
						i,pingcur->time_i[0],
						pingcur->time_i[1],
						pingcur->time_i[2],
						pingcur->time_i[3],
						pingcur->time_i[4],
						pingcur->time_i[5]);
					}
				}

			/* plot data */
			mb_contour(verbose,swath_plot,&error);

			/* plot shiptrack */
			if (plot_track)
			mb_track(verbose,swath_plot,&error);

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
	status = mb_close(verbose,mbio_ptr,&error);

	/* deallocate memory for data arrays */
	mb_free(verbose,amp,&error);
	mb_free(verbose,ss,&error);
	mb_free(verbose,sslon,&error);
	mb_free(verbose,sslat,&error);
	status = mb_contour_deall(verbose,swath_plot,&error);

	/* figure out whether and what to read next */
        if (use_stdin == MB_NO)
                {
                if (fgets(line,128,fp) != NULL
                        && sscanf(line,"%s %d",file,&format) == 2)
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
	fclose (fp);

	/* end plot */
	plot_end(verbose,&error);

	/* deallocate memory for data arrays */
	mb_free(verbose,level,&error);
	mb_free(verbose,label,&error);
	mb_free(verbose,tick,&error);
	mb_free(verbose,red,&error);
	mb_free(verbose,green,&error);
	mb_free(verbose,blue,&error);

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
}
/*--------------------------------------------------------------------*/
int ping_copy(verbose,one,two,swath,error)
int	verbose;
int	one;
int	two;
struct swath *swath;
int	*error;
{
	char	*function_name = "ping_copy";
	int	status = MB_SUCCESS;

	struct ping	*ping1;
	struct ping	*ping2;
	int	i, j;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBSWATH function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       one:        %d\n",one);
		fprintf(stderr,"dbg2       two:        %d\n",two);
		fprintf(stderr,"dbg2       swath:      %d\n",swath);
		fprintf(stderr,"dbg2       pings:      %d\n",swath->npings);
		}

	/* copy things */
	ping1 = &swath->pings[one];
	ping2 = &swath->pings[two];
	for (i=0;i<6;i++)
		ping1->time_i[i] = ping2->time_i[i];
	ping1->time_d = ping2->time_d;
	ping1->navlon = ping2->navlon;
	ping1->navlat = ping2->navlat;
	ping1->heading = ping2->heading;
	for (i=0;i<swath->beams_bath;i++)
		{
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
