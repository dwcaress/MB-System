/*--------------------------------------------------------------------
 *    The MB-system:	mb_penface.c	5/15/94
 *    $Id: mb_penface.c,v 4.7 1995-03-06 19:42:54 caress Exp $
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
 * MB_PENFACE is a set of functions which provide an interface
 * between the program MBCONTOUR and pen plotting calls. This code 
 * is separated from MBCONTOUR so that a similar set of interface 
 * functions for plotting using the PSLIB Postscript plotting 
 * library from GMT can be linked to the same source code.
 *
 * Author:	D. W. Caress
 * Date:	May 15, 1994
 *
 * $Log: not supported by cvs2svn $
 * Revision 4.6  1995/02/17  16:48:57  caress
 * Removed debug messages.
 *
 * Revision 4.5  1995/02/17  16:03:31  caress
 * Fixed bug in saving xold and yold values which had caused fits
 * when plotting the shiptrack.
 *
 * Revision 4.4  1994/11/10  20:56:46  caress
 * Changed size of esp_geo; now factor of 10 smaller.
 *
 * Revision 4.3  1994/10/21  12:56:50  caress
 * Release V4.0
 *
 * Revision 4.2  1994/07/29  19:07:47  caress
 * Changes associated with supporting byte swapped Lynx OS and
 * using unix seconds for time base.
 *
 * Revision 4.1  1994/05/25  15:10:39  caress
 * Fixed plot_exit.
 *
 * Revision 4.0  1994/05/17  14:03:36  caress
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

/* pen defines */
#define IUP 3
#define IDN 2

/* global variables */
double	inchtolon;
double  eps_geo;
double	xold;
double	yold;

/*--------------------------------------------------------------------------*/
/* 	function plot_init initializes the plotting. */
int plot_init(verbose,argc,argv,bounds,scale,inch2lon,error)
int	verbose;
int	argc;
char	**argv;
double	*bounds;
double	*scale;
double	*inch2lon;
int	*error;
{
  	static char rcs_id[]="$Id: mb_penface.c,v 4.7 1995-03-06 19:42:54 caress Exp $";
	char	*function_name = "plot_init";
	int	status = MB_SUCCESS;
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

	/* initialize plotting */
	printf("init\n");

	/* get inches to longitude scale */
	*inch2lon = 1.0/(*scale);
	inchtolon = *inch2lon;

	/* set line width */
	eps_geo = 0.0;

	/* set original origin */
	xold = 0.0;
	yold = 0.0;

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
/* 	function plot_end ends the plotting. */
int plot_end(verbose,error)
int	verbose;
int	*error;
{
  	static char rcs_id[]="$Id: mb_penface.c,v 4.7 1995-03-06 19:42:54 caress Exp $";
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

	/* end the plot */
	printf("stop\n");

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
	exit(0);
}

/*--------------------------------------------------------------------*/
void set_colors(ncol,rd,gn,bl)
int	ncol;
int	*rd;
int	*gn;
int	*bl;
{
	return;
}

/*--------------------------------------------------------------------*/
int plot(x,y,ipen)
double x,y;
int ipen;
{
	double	mag, dx, dy;

	if (eps_geo <= 0.0 || ipen != IDN)
		printf("plot %f %f %d\n",x,y,ipen);
	else
		{
		printf("plot %f %f %d\n",x,y,ipen);

		dx = x - xold;
		dy = y - yold;
		mag = sqrt(dx*dx + dy*dy);
		if (mag > 0.0)
			{
			dx = eps_geo*dx/mag;
			dy = eps_geo*dy/mag;
			printf("plot %f %f %d\n",xold,yold,IUP);
			printf("plot %f %f %d\n",x,y,IDN);
			printf("plot %f %f %d\n",x+dy,y-dx,IDN);
			printf("plot %f %f %d\n",xold+dy,yold-dx,IDN);
			printf("plot %f %f %d\n",xold-dy,yold+dx,IDN);
			printf("plot %f %f %d\n",x-dy,y+dx,IDN);
			printf("plot %f %f %d\n",xold-dy,yold+dx,IDN);
			printf("plot %f %f %d\n",x+dy,y-dx,IDN);
			printf("plot %f %f %d\n",xold+dy,yold-dx,IDN);
			printf("plot %f %f %d\n",x,y,IDN);
			printf("plot %f %f %d\n",xold,yold,IDN);
			printf("plot %f %f %d\n",x,y,IDN);
			}	
		}
	xold = x;
	yold = y;

	return;
}
/*--------------------------------------------------------------------*/
int plot_(x,y,ipen)
float *x,*y;
int *ipen;
{
	printf("plot %f %f %d\n",((double) *x),((double) *y),*ipen);
	xold = (double) *x;
	yold = (double) *y;
	return;
}
/*--------------------------------------------------------------------*/
int newpen(ipen)
int ipen;
{
	printf("newp %d\n",ipen);
	return;
}
/*--------------------------------------------------------------------*/
int setline(linewidth)
int linewidth;
{
	eps_geo = inchtolon*0.002*linewidth;
        return;
}
/*--------------------------------------------------------------------*/
int justify_string(height,string,s)
double	height;
char	*string;
double	*s;
{
	float	hgtf;
	float	ss[4];
	int	len;
	int	i;

	len = strlen(string);
	for (i=0;i<len;i++)
		if (string[i] == ' ')
			string[i] = '_';
	hgtf = height;
	justify_(ss,&hgtf,string,&len);
	s[0] = ss[0];
	s[1] = ss[1];
	s[2] = ss[2];
	s[3] = ss[3];

	return;
}
/*--------------------------------------------------------------------*/
int plot_string(x,y,hgt,angle,label)
double	x;
double	y;
double	hgt;
double	angle;
char	*label;
{
	float	xlab, ylab, hght, ang;
	int	len;
	int	i;

	xlab = x;
	ylab = y;
	hght = hgt;
	ang = angle;
	len = strlen(label);
	for (i=0;i<len;i++)
		if (label[i] == ' ')
			label[i] = '_';
	label_(&xlab,&ylab,&hght,&ang,label,&len);

	return;
}
/*--------------------------------------------------------------------*/
