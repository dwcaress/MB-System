/*--------------------------------------------------------------------
 *    The MB-system:	mb_pslibface.c	5/15/94
 *    $Id: mb_penface.c,v 4.1 1994-05-25 15:10:39 caress Exp $
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
 * between the program MBCONTOUR and pen plotting calls. This code 
 * is separated from MBCONTOUR so that a similar set of interface 
 * functions for plotting using the PSLIB Postscript plotting 
 * library from GMT can be linked to the same source code.
 *
 * Author:	D. W. Caress
 * Date:	May 15, 1994
 *
 * $Log: not supported by cvs2svn $
 * Revision 4.0  1994/05/17  14:03:36  caress
 * First cut at new contouring scheme.
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

/* global variables */
double	inchtolon;

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
  	static char rcs_id[]="$Id: mb_penface.c,v 4.1 1994-05-25 15:10:39 caress Exp $";
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
		fprintf(stderr,"dbg2       scale:            %f\n",scale);
		}

	/* initialize plotting */
	printf("init\n");

	/* get inches to longitude scale */
	*inch2lon = 1.0/(*scale);
	inchtolon = *inch2lon;

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
		fprintf(stderr,"dbg2       scale:      %f\n",scale);
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
  	static char rcs_id[]="$Id: mb_penface.c,v 4.1 1994-05-25 15:10:39 caress Exp $";
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
	printf("plot %f %f %d\n",x,y,ipen);
	return;
}
/*--------------------------------------------------------------------*/
int plot_(x,y,ipen)
float *x,*y;
int *ipen;
{
	printf("plot %f %f %d\n",((double) *x),((double) *y),*ipen);
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
