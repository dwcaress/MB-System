/*--------------------------------------------------------------------
 *    The MB-system:	mb_pslibface.c	5/15/94
 *    $Id: mb_pslibface.c,v 4.7 1997-04-21 16:53:56 caress Exp $
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
 * MB_PSLIBFACE is a set of functions which provide an interface
 * between the program MBCONTOUR and the PSLIB Postscript plotting 
 * library from GMT.  This code is separated from MBCONTOUR so that
 * a similar set of interface functions for pen plotting can be linked
 * to the same source code.
 *
 * Author:	D. W. Caress
 * Date:	May 15, 1994
 *
 * $Log: not supported by cvs2svn $
 * Revision 4.6  1996/04/22  13:18:44  caress
 * Now have DTR and MIN/MAX defines in mb_define.h
 *
 * Revision 4.6  1996/04/22  13:18:44  caress
 * Now have DTR and MIN/MAX defines in mb_define.h
 *
 * Revision 4.5  1995/11/15  22:34:22  caress
 * Now handles non-region bounds (lower left point
 * + upper right point) properly.
 *
 * Revision 4.4  1995/08/07  17:31:39  caress
 * Moved to GMT V3.
 *
 * Revision 4.3  1995/03/06  19:39:52  caress
 * Changed include strings.h to string.h for POSIX compliance.
 *
 * Revision 4.2  1994/07/29  19:04:31  caress
 * Changes associated with supporting byte swapped Lynx OS and
 * >> using unix second time base.
 *
 * Revision 4.1  1994/05/25  15:08:49  caress
 * Fixed plot_exit.
 *
 * Revision 4.0  1994/05/16  22:14:24  caress
 * First cut at new contouring scheme.
 *
 *
 */

/* standard include files */
#include <stdio.h>
#include <math.h>
#include <string.h>
#include <time.h>

/* MBIO include files */
#include "../../include/mb_status.h"
#include "../../include/mb_define.h"

/* GMT include files */
#include "gmt.h"

/* global variables */
int	argc_save;
char	**argv_save;
double	inchtolon;
int	ncolor;
int	*red;
int	*green;
int	*blue;

/*--------------------------------------------------------------------------*/
/* 	function plot_init initializes the GMT plotting. */
int plot_init(verbose,argc,argv,bounds_use,scale,inch2lon,error)
int	verbose;
int	argc;
char	**argv;
double	*bounds_use;
double	*scale;
double	*inch2lon;
int	*error;
{
  	static char rcs_id[]="$Id: mb_pslibface.c,v 4.7 1997-04-21 16:53:56 caress Exp $";
	char	*function_name = "plot_init";
	int	status = MB_SUCCESS;
	int	errflg = 0;
	double	bounds[4];
	double	x1, y1, x2, y2, xx1, yy1, xx2, yy2;
	double	clipx[4], clipy[4];
	int	i;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:          %d\n",verbose);
		fprintf(stderr,"dbg2       argc:             %d\n",argc);
		fprintf(stderr,"dbg2       argv:             %d\n",argv);
		fprintf(stderr,"dbg2       bounds[0]:        %f\n",bounds[0]);
		fprintf(stderr,"dbg2       bounds[1]:        %f\n",bounds[1]);
		fprintf(stderr,"dbg2       bounds[2]:        %f\n",bounds[2]);
		fprintf(stderr,"dbg2       bounds[3]:        %f\n",bounds[3]);
		fprintf(stderr,"dbg2       scale:            %f\n",*scale);
		}

	/* save argc and argv */
	argc_save = argc;
	argv_save = argv;

	/* deal with gmt options */
	gmt_begin (argc, argv);
	for (i = 1; i < argc; i++) 
		{
		if (argv[i][0] == '-') 
			{
			switch (argv[i][1]) 
				{
				/* Common parameters */
			
				case 'B':
				case 'J':
				case 'K':
				case 'O':
				case 'P':
				case 'R':
				case 'U':
				case 'V':
				case 'X':
				case 'x':
				case 'Y':
				case 'y':
				case 'c':
				case '\0':
					errflg += get_common_args (argv[i], 
						&bounds[0], &bounds[1], 
						&bounds[2], &bounds[3]);
					break;
				
				/* Supplemental parameters */
			
				case 'F':
					sscanf (&argv[i][2], "%d/%d/%d",
						&gmtdefs.basemap_frame_rgb[0],
						&gmtdefs.basemap_frame_rgb[1],
						&gmtdefs.basemap_frame_rgb[2]);
					break;
				}
			}
		}

	/* if error flagged then return */
	if (errflg)
		{
		status = MB_FAILURE;
		return(status);
		}

	/* set up map */
	map_setup(bounds[0],bounds[1],bounds[2],bounds[3]);

	/* initialize plotting */
	ps_plotinit (NULL, gmtdefs.overlay, gmtdefs.page_orientation, 
		gmtdefs.x_origin, gmtdefs.y_origin,
		gmtdefs.global_x_scale, gmtdefs.global_y_scale, 
		gmtdefs.n_copies, gmtdefs.dpi, gmtdefs.measure_unit, 
		gmtdefs.paper_width, gmtdefs.page_rgb, gmt_epsinfo (argv[0]));
	echo_command (argc, argv);

	/* copy bounds in correct order for use by this program */
	if (project_info.region == MB_YES)
		{
		bounds_use[0] = bounds[0];
		bounds_use[1] = bounds[1];
		bounds_use[2] = bounds[2];
		bounds_use[3] = bounds[3];
		}
	else
		{
		bounds_use[0] = bounds[0];
		bounds_use[1] = bounds[2];
		bounds_use[2] = bounds[1];
		bounds_use[3] = bounds[3];
		}

	/* set clip path */
	geo_to_xy(bounds_use[0],bounds_use[2],&clipx[0],&clipy[0]);
	geo_to_xy(bounds_use[1],bounds_use[2],&clipx[1],&clipy[1]);
	geo_to_xy(bounds_use[1],bounds_use[3],&clipx[2],&clipy[2]);
	geo_to_xy(bounds_use[0],bounds_use[3],&clipx[3],&clipy[3]);
	ps_clipon(clipx,clipy,4,-1,-1,-1,3);

	/* get inches to longitude scale */
	x1 = 0.0;
	y1 = 0.0;
	x2 = 1.0;
	y2 = 0.0;
	xy_to_geo(&xx1,&yy1,x1,y1);
	xy_to_geo(&xx2,&yy2,x2,y2);
	*inch2lon = xx2 - xx1;
	inchtolon = *inch2lon;

	/* set line width */
	ps_setline(0);

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       bounds[0]:  %f\n",bounds[0]);
		fprintf(stderr,"dbg2       bounds[1]:  %f\n",bounds[1]);
		fprintf(stderr,"dbg2       bounds[2]:  %f\n",bounds[2]);
		fprintf(stderr,"dbg2       bounds[3]:  %f\n",bounds[3]);
		fprintf(stderr,"dbg2       scale:      %f\n",*scale);
		fprintf(stderr,"dbg2       inchtolon:  %d\n",*inch2lon);
		fprintf(stderr,"dbg2       error:      %d\n",*error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:     %d\n",status);
		}

	return(status);
}
/*--------------------------------------------------------------------------*/
/* 	function plot_end ends the GMT plotting. */
int plot_end(verbose,error)
int	verbose;
int	*error;
{
  	static char rcs_id[]="$Id: mb_pslibface.c,v 4.7 1997-04-21 16:53:56 caress Exp $";
	char	*function_name = "plot_end";
	int	status = MB_SUCCESS;
	int	i;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:          %d\n",verbose);
		}

	/* turn off clipping */
	ps_clipoff();

	/* plot basemap if required */
	if (frame_info.plot) 
		{
		ps_setpaint (gmtdefs.basemap_frame_rgb[0], 
			gmtdefs.basemap_frame_rgb[1], 
			gmtdefs.basemap_frame_rgb[2]);
		map_basemap ();
		ps_setpaint (0, 0, 0);
		}

	/* plot the unix timestamp if required */
	if (gmtdefs.unix_time) 
		timestamp (argc_save, argv_save);

	/* end the plot */
	ps_plotend (gmtdefs.last_page);

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       error:      %d\n",*error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:     %d\n",status);
		}

	return(status);
}
/*--------------------------------------------------------------------*/
int plot_exit(argc,argv)
int	argc;
char	**argv;
{
	gmt_end(argc, argv);
}

/*--------------------------------------------------------------------*/
void set_colors(ncol,rd,gn,bl)
int	ncol;
int	*rd;
int	*gn;
int	*bl;
{
	ncolor = ncol;
	red = rd;
	green = gn;
	blue = bl;
	return;
}

/*--------------------------------------------------------------------*/
void plot(x,y,ipen)
double x,y;
int ipen;
{
	double	xx, yy;
	geo_to_xy(x,y,&xx,&yy);
	ps_plot(xx,yy,ipen);
	return;
}
/*--------------------------------------------------------------------*/
void setline(linewidth)
int linewidth;
{
        ps_setline(linewidth);
        return;
}
/*--------------------------------------------------------------------*/
void newpen(ipen)
int ipen;
{
	if (ipen > -1 && ipen < ncolor)
		ps_setpaint(red[ipen],green[ipen],blue[ipen]);
	return;
}
/*--------------------------------------------------------------------*/
void justify_string(height,string,s)
double	height;
char	*string;
double	*s;
{
	int	len;

	len = strlen(string);
	s[0] = 0.0;
	s[1] = 0.185*height*len;
	s[2] = 0.37*len*height;
	s[3] = 0.37*len*height;

	return;
}
/*--------------------------------------------------------------------*/
void plot_string(x,y,hgt,angle,label)
double	x;
double	y;
double	hgt;
double	angle;
char	*label;
{
	int	point;
	double	height;
	double	xx, yy;

	height = hgt/inchtolon;
	point = height*72.;
	geo_to_xy(x,y,&xx,&yy);
	ps_text(xx,yy,point,label,angle,5,0);

	return;
}
/*--------------------------------------------------------------------*/
