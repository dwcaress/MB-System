/*--------------------------------------------------------------------
 *    The MB-system:	mb_delaun.c	4/19/94
 *    $Id$
 *
 *    Copyright (c) 1994-2015 by
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
 * The function mb_delaun.c assigns triangles to a set of x,y points
 * based on the method of:
 *	Watson, Computers and Geosciences, V8, 97-101, 1982.
 * The final network is a set of Delauney triangles with the property
 * that no vertex lies inside the circumcircle of any triangle.  This
 * system is as close to equiangular as possible.
 * This code was translated from a Fortran 77 subroutine obtained from
 * Robert Parker at the Scripps Institution of Oceanography.  It is
 * unclear whether Bob wrote the subroutine based on the Watson article
 * or obtained the code from someone else.
 *
 * The input values are:
 *   verbose:		verbosity of debug output (MBIO convention)
 *   npts:		number of input x,y points
 *   p1[npts+3]:	array of npts input x values
 *   p2[npts+3]:	array of npts input y values
 *   ed[npts+3]:	array of edge flags - points on the edge
 *			of the region are flagged by nonzero values
 *			- interior points are flagged by zero values
 *			- triangles constructed using three edge points
 *			are removed
 *
 * The output values are:
 *   ntri:		number of output triangles
 *   iv1[2*npts+1]:	array of indices of points forming the first vertex
 *			of the triangles
 *   iv2[2*npts+1]:	array of indices of points forming the second vertex
 *			of the triangles
 *   iv3[2*npts+1]:	array of indices of points forming the third vertex
 *			of the triangles
 *   ct1[2*npts+1]:	triangle connection array, value ct1[i] indicates which
 *			triangle connects to side 1 of triangle i, value
 *			of -1 indicates no connecting triangle
 *   ct2[2*npts+1]:	triangle connection array, value ct2[i] indicates which
 *			triangle connects to side 2 of triangle i
 *			of -1 indicates no connecting triangle
 *   ct3[2*npts+1]:	triangle connection array, value ct3[i] indicates which
 *			triangle connects to side 3 of triangle i
 *			of -1 indicates no connecting triangle
 *   cs1[2*npts+1]:	triangle connection array, value cs1[i] indicates which
 *			side of triangle ct1[i] connects to side 1 of triangle i
 *   cs2[2*npts+1]:	triangle connection array, value cs2[i] indicates which
 *			side of triangle ct2[i] connects to side 2 of triangle i
 *   cs3[2*npts+1]:	triangle connection array, value cs3[i] indicates which
 *			side of triangle ct3[i] connects to side 3 of triangle i
 *   error:		error value, MBIO convention
 *
 * The work arrays are passed into mb_delaun rather than allocated and
 * deallocated within mb_delaun to increase the efficiency of programs
 * which use mb_delaun repeatedly. These work arrays are:
 *   v1[2*npts+1]:	the value v1[i] stores the x value of the
 *			circumcenter of triangle i
 *   v2[2*npts+1]:	the value v2[i] stores the y value of the
 *			circumcenter of triangle i
 *   v3[2*npts+1]:	the value v3[i] stores the square of the radius
 *			of the circumcircle of triangle i
 *   istack[2*npts+1]:	This is a stack onto which is pushed the the
 *			indexes of all the triangles that need to be
 *			replaced as a result of the addition of a new
 *			point. During the construction of the new 3-tuples
 *			the index of the 3-tuple to be replaced is
 *			popped from the stack. Since the addition of each
 *			new point adds 2 more triangles to the total,
 *			istack is popped 2 more times than it is pushed
 *			for each additional point. Hence istack is
 *			initialized prior to the main part of the routine
 *			so that it correctly returns the indexes of the
 *			new triangles.
 *    kv1[6*npts+1]:	For each new point, each existing triangle is
 *    kv2[6*npts+1]:	tested in turn and replaced if the new point
 *			lies within its circumcircle. Each side of the
 *			triangle is pushed onto the arrays kv1 and kv2 to
 *			potentially form a triangle with the new point.
 *			HOWEVER, if the side is already on the kv stack it
 *			is removed from the stack.  This means that if the
 *			side is common to two triangles that are to be
 *			replaced, i.e an interior side, then it is not
 *			used as the basis for forming the new triangles.
 *
 * Author:	D. W. Caress
 * Date:	April, 1994
 *
 * $Log: mb_delaun.c,v $
 * Revision 5.1  2008/07/10 06:43:40  caress
 * Preparing for 5.1.1beta20
 *
 * Revision 5.0  2000/12/01 22:53:59  caress
 * First cut at Version 5.0.
 *
 * Revision 4.8  2000/10/11  01:00:12  caress
 * Convert to ANSI C
 *
 * Revision 4.7  2000/09/30  06:54:58  caress
 * Snapshot for Dale.
 *
 * Revision 4.6  1997/09/15  19:03:27  caress
 * Real Version 4.5
 *
 * Revision 4.5  1997/04/21  16:53:56  caress
 * MB-System 4.5 Beta Release.
 *
 * Revision 4.4  1996/04/22  13:18:44  caress
 * Now have DTR and MIN/MAX defines in mb_define.h
 *
 * Revision 4.4  1996/04/22  13:18:44  caress
 * Now have DTR and MIN/MAX defines in mb_define.h
 *
 * Revision 4.3  1995/03/06  19:39:52  caress
 * Changed include strings.h to string.h for POSIX compliance.
 *
 * Revision 4.2  1994/10/21  11:34:20  caress
 * Release V4.0
 *
 * Revision 4.1  1994/05/24  03:11:40  caress
 * Fixed include.
 *
 * Revision 4.0  1994/05/16  22:09:29  caress
 * First cut at new contouring scheme
 *
 *
 */

/* standard global include files */
#include <stdio.h>
#include <math.h>
#include <string.h>

/* mbio include files */
#include "mb_status.h"
#include "mb_define.h"

/* some defines */
#define	LARGE	1.0e10

static char rcs_id[] = "$Id$";

/*--------------------------------------------------------------------------*/
/* 	function mb_delaun creates a network of triangles connecting an
	input set of points, where the triangles are as close to equiangular
	as possible. */
int mb_delaun(
	int	verbose,
	int	npts,
	double	*p1,
	double	*p2,
	int	*ed,
	int	*ntri,
	int	*iv1,
	int	*iv2,
	int	*iv3,
	int	*ct1,
	int	*ct2,
	int	*ct3,
	int	*cs1,
	int	*cs2,
	int	*cs3,
	double	*v1,
	double	*v2,
	double	*v3,
	int	*istack,
	int	*kv1,
	int	*kv2,
	int	*error)
{
	char	*function_name = "mb_delaun";
	int	status = MB_SUCCESS;
	int	itemp[2][3];
	int	addside;
	int	n1;
	double	xmin, xmax, ymin, ymax;
	double	cx, cy, crsq, rad, rsq;
	int	maxn;
	int	isp, id;
	int	nuc, km, jt, kt, i1, i2;
	int	l1, l2;
	int	*ivs1, *ivs2;
	double	denom, s;
	double	xproduct;
	int	notfound;
	int	i, j, k;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBBA function <%s> called\n",function_name);
		fprintf(stderr,"dbg2  Revision id: %s\n",rcs_id);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:          %d\n",verbose);
		fprintf(stderr,"dbg2       npts:             %d\n",npts);
		fprintf(stderr,"dbg2       p1:               %p\n",p1);
		fprintf(stderr,"dbg2       p2:               %p\n",p2);
		fprintf(stderr,"dbg2       ntri:             %d\n",*ntri);
		fprintf(stderr,"dbg2       iv1:              %p\n",iv1);
		fprintf(stderr,"dbg2       iv2:              %p\n",iv2);
		fprintf(stderr,"dbg2       iv3:              %p\n",iv3);
		fprintf(stderr,"dbg2       ct1:              %p\n",ct1);
		fprintf(stderr,"dbg2       ct2:              %p\n",ct2);
		fprintf(stderr,"dbg2       ct3:              %p\n",ct3);
		fprintf(stderr,"dbg2       cs1:              %p\n",cs1);
		fprintf(stderr,"dbg2       cs2:              %p\n",cs2);
		fprintf(stderr,"dbg2       cs3:              %p\n",cs3);
		fprintf(stderr,"dbg2       v1:               %p\n",v1);
		fprintf(stderr,"dbg2       v2:               %p\n",v2);
		fprintf(stderr,"dbg2       v3:               %p\n",v3);
		fprintf(stderr,"dbg2       istack:           %p\n",istack);
		fprintf(stderr,"dbg2       kv1:              %p\n",kv1);
		fprintf(stderr,"dbg2       kv2:              %p\n",kv2);
		fprintf(stderr,"dbg2       error:            %d\n",*error);
		if (verbose >= 5)
			{
			fprintf(stderr,"dbg5       Input data:\n");
			for (i=0;i<npts;i++)
				fprintf(stderr,"dbg5       %d %f %f\n",
				i,p1[i],p2[i]);
			}
		}

	/* define itemp */
	itemp[0][0] = 1;
	itemp[1][0] = 2;
	itemp[0][1] = 1;
	itemp[1][1] = 3;
	itemp[0][2] = 2;
	itemp[1][2] = 3;

	/* initialize the triangle stack */
	n1 = 2*npts + 3;
	for (i=0;i<n1;i++)
		istack[i] = i;

	/* determine the extremes of the data */
	xmin = p1[0];
	xmax = p1[0];
	ymin = p2[0];
	ymax = p2[0];
	for (i=0;i<npts;i++)
		{
		xmin = MIN(xmin,p1[i]);
		xmax = MAX(xmax,p1[i]);
		ymin = MIN(ymin,p2[i]);
		ymax = MAX(ymax,p2[i]);
		}

	/* enclose the data region in an equilateral triangle
		- put circumcenter and radius-squared into the list */
	cx = xmax - xmin;
	cy = ymax - ymin;
	crsq = 1.2*(cx*cx + cy*cy);
	cx = 0.5*(xmin + xmax);
	cy = 0.5*(ymin + ymax);
	v1[0] = cx;
	v2[0] = cy;
	v3[0] = crsq;

	/* put vertex coordinates in the end of the p array */
	rad = sqrt(v3[0]);
	maxn = npts + 3;
	for (i=0;i<3;i++)
		{
		p1[npts+2-i] = v1[0] + rad*cos(2.0944*(i+1));
		p2[npts+2-i] = v2[0] + rad*sin(2.0944*(i+1));
		}
	iv1[0] = npts + 2;
	iv2[0] = npts + 1;
	iv3[0] = npts;

	/* scan through the data backwards */
	isp = 1;
	id = 1;
	for (nuc=npts-1;nuc>-1;nuc--)
	  {
	  km = 0;
/*fprintf(stderr,"\nnuc:%d\n",nuc);
for (jt=0;jt<isp;jt++)
fprintf(stderr,"jt:%d  %d %d %d\n",jt,iv1[jt],iv2[jt],iv3[jt]);*/

	  /* loop through the established 3-tuples */
	  for (jt=0;jt<isp;jt++)
	    {
	    i1 = iv3[jt];
	    /* calculate the distance of the
		point from the jt circumcenter */
	    rsq = (p1[nuc] - p1[i1])*(p1[nuc] + p1[i1] - 2*v1[jt])
		+ (p2[nuc] - p2[i1])*(p2[nuc] + p2[i1] - 2*v2[jt]);

	    /* If the point lies within circumcircle of triangle
		(3-tuple) jt then delete this triangle. Save the
		edges provided that they are not part of another
		triangle for which the point lies within the circumcircle
		Thus "interior" edges are eliminated prior to
		construction of the new triangles (3 tuples). */
/*if (rsq <= 0.0 && !(rsq < 0.0))
{
fprintf(stderr,"NOTICE: rsq:%g\n",rsq);
fprintf(stderr,"nuc:%d p1:%f p2:%f  i1:%d p1:%f p2:%f  jt:%d v1:%f v2:%f\n",
nuc,p1[nuc],p2[nuc],i1,p1[i1],p2[i1],jt,v1[jt],v2[jt]);
fprintf(stderr,"jt:%d iv1:%d %f %f\n",jt,iv1[jt],p1[iv1[jt]],p2[iv1[jt]]);
fprintf(stderr,"jt:%d iv2:%d %f %f\n",jt,iv2[jt],p1[iv2[jt]],p2[iv2[jt]]);
fprintf(stderr,"jt:%d iv3:%d %f %f\n",jt,iv3[jt],p1[iv3[jt]],p2[iv3[jt]]);
}*/
	    if (rsq <= 0.0)
	      {
	      /* triangle needs replacing => push the index on the stack */
	      id = id - 1;
	      istack[id] = jt;
/*fprintf(stderr,"delete triangle jt:%d\n",jt);*/

	      /* add edges to kv but delete if already present */
	      for (i=0;i<3;i++)
		{
		/* cycle through the edges of the triangle */
		l1 = itemp[0][i];
		l2 = itemp[1][i];
		if (l1 == 1) ivs1 = iv1;
		if (l1 == 2) ivs1 = iv2;
		if (l1 == 3) ivs1 = iv3;
		if (l2 == 1) ivs2 = iv1;
		if (l2 == 2) ivs2 = iv2;
		if (l2 == 3) ivs2 = iv3;
		addside = MB_YES;

		/* Check if the side is already stored in kv. If it
			is then Side common to more than one of the
			current triangles. So remove it from list. */
		j = 0;
		while ((j < km) && addside)
		  {
		  if (ivs1[jt] == kv1[j] && ivs2[jt] == kv2[j])
		    {
		    addside = MB_NO;
		    km--;
		    for (k=j;k<km;k++)
		      {
		      kv1[k] = kv1[k+1];
		      kv2[k] = kv2[k+1];
		      }
		    }
		  j++;
		  }

		/* side was not found in list so add it now */
		if (addside)
		  {
		  km++;
		  if (km > 6*npts+1)
		    {
		    fprintf(stderr,"Error in mb_delaun - kv array too small at dimension %d\n",6*npts+1);
		    *error = 99;
		    status = MB_FAILURE;
		    return(status);
		    }
		  kv1[km-1] = ivs1[jt];
		  kv2[km-1] = ivs2[jt];
		  }
		} /* end: for (i=0;i<3;i++) */
	      } /* end: if (rsq < 0.0) */

	    } /* end: for (jt=0;jt<isp;jt++) */

	  /* form new 3-tuples */
	  for (i=0;i<km;i++)
	    {
	    /* pop the triangle index from the stack */
	    kt = istack[id];
	    id++;

	    /* calculate the circumcircle and radius squared */
	    i1 = kv1[i];
	    i2 = kv2[i];
	    denom = ((p1[i1] - p1[nuc])*(p2[i2] - p2[nuc])
			- (p2[i1] - p2[nuc])*(p1[i2] - p1[nuc]));

	    /* check if the three points form a true triangle */
	    if (denom != 0.0)
	      {
	      s = ((p1[i1] - p1[nuc])*(p1[i1] - p1[i2])
			+ (p2[i1] - p2[nuc])*(p2[i1] - p2[i2]))/denom;
	      v1[kt] = 0.5*(p1[i2] + p1[nuc] + s*(p2[i2] - p2[nuc]));
	      v2[kt] = 0.5*(p2[i2] + p2[nuc] - s*(p1[i2] - p1[nuc]));
	      }

	    /* The three points are degenerate and lie on a line.
		The "true" circumcenter is at infinity and the radius
		is also infinite. All points on the plane lie within
		the circumcircle. Thus simulate this by using
		circumcenter and circumcircle of the enclosing
		equilateral triangle */
	    else
	      {
              fprintf(stderr,"\nmb_delaun Warning. Zero denominator\n");
              fprintf(stderr,"%d %f %f\n",i1,p1[i1],p2[i1]);
              fprintf(stderr,"%d %f %f\n",i2,p1[i2],p2[i2]);
              fprintf(stderr,"%d %f %f\n",nuc,p1[nuc],p2[nuc]);
	      v1[kt] = cx;
	      v2[kt] = cy;
	      v3[kt] = nuc;
	      }
	    iv1[kt] = kv1[i];
	    iv2[kt] = kv2[i];
	    iv3[kt] = nuc;
	    } /* end: for (i=0;i<km;i++) */

	  /* adding a point adds two more triangles to the total
		number of triangles */
	  isp = isp + 2;

	  } /* end: for (nuc=npts-1;nuc>-1;nuc--) */

	/* set number of triangles */
	*ntri = isp;

	/* remove triangles using added points and triangles made
		up of three flagged edge points */
	for (i=*ntri-1;i>-1;i--)
		{
		if (iv1[i] >= npts || iv2[i] >= npts || iv3[i] >= npts)
			{
			for (j=i;j<isp-1;j++)
				{
				iv1[j] = iv1[j+1];
				iv2[j] = iv2[j+1];
				iv3[j] = iv3[j+1];
				}
			isp--;
			}
		else if (ed[iv1[i]] != 0 && ed[iv2[i]] != 0 && ed[iv3[i]] != 0)
			{
			for (j=i;j<isp-1;j++)
				{
				iv1[j] = iv1[j+1];
				iv2[j] = iv2[j+1];
				iv3[j] = iv3[j+1];
				}
			isp--;
			}
		}
	*ntri = isp;

	/* make sure all triangles are defined clockwise */
	for (i=0;i<*ntri;i++)
		{
		xproduct = -(p1[iv2[i]] - p1[iv1[i]])*(p2[iv3[i]] - p2[iv2[i]])
			+ (p1[iv3[i]] - p1[iv2[i]])*(p2[iv2[i]] - p2[iv1[i]]);
		if (xproduct < 0.0)
			{
			j = iv2[i];
			iv2[i] = iv3[i];
			iv3[i] = j;
			}
		}

	/* now get connectivity */
	for (i=0;i<*ntri;i++)
		{
		ct1[i] = -1;
		ct2[i] = -1;
		ct3[i] = -1;
		cs1[i] = -1;
		cs2[i] = -1;
		cs3[i] = -1;
		}
	for (i=0;i<*ntri;i++)
		{
		/* check side 1 of triangle i */
		if (ct1[i] == -1)
		  {
		  notfound = MB_YES;
		  for (j=0;notfound && j<*ntri;j++)
			{
			if (notfound && iv1[i] == iv2[j] && iv2[i] == iv1[j])
				{
				ct1[i] = j;
				cs1[i] = 0;
				ct1[j] = i;
				cs1[j] = 0;
				notfound = MB_NO;
				}
			if (notfound && iv1[i] == iv3[j] && iv2[i] == iv2[j])
				{
				ct1[i] = j;
				cs1[i] = 1;
				ct2[j] = i;
				cs2[j] = 0;
				notfound = MB_NO;
				}
			if (notfound && iv1[i] == iv1[j] && iv2[i] == iv3[j])
				{
				ct1[i] = j;
				cs1[i] = 2;
				ct3[j] = i;
				cs3[j] = 0;
				notfound = MB_NO;
				}
			}
		  }
		/* check side 2 of triangle i */
		if (ct2[i] == -1)
		  {
		  notfound = MB_YES;
		  for (j=0;notfound && j<*ntri;j++)
			{
			if (notfound && iv2[i] == iv2[j] && iv3[i] == iv1[j])
				{
				ct2[i] = j;
				cs2[i] = 0;
				ct1[j] = i;
				cs1[j] = 1;
				notfound = MB_NO;
				}
			if (notfound && iv2[i] == iv3[j] && iv3[i] == iv2[j])
				{
				ct2[i] = j;
				cs2[i] = 1;
				ct2[j] = i;
				cs2[j] = 1;
				notfound = MB_NO;
				}
			if (notfound && iv2[i] == iv1[j] && iv3[i] == iv3[j])
				{
				ct2[i] = j;
				cs2[i] = 2;
				ct3[j] = i;
				cs3[j] = 1;
				notfound = MB_NO;
				}
			}
		  }
		/* check side 3 of triangle i */
		if (ct3[i] == -1)
		  {
		  notfound = MB_YES;
		  for (j=0;notfound && j<*ntri;j++)
			{
			if (notfound && iv3[i] == iv2[j] && iv1[i] == iv1[j])
				{
				ct3[i] = j;
				cs3[i] = 0;
				ct1[j] = i;
				cs1[j] = 2;
				notfound = MB_NO;
				}
			if (notfound && iv3[i] == iv3[j] && iv1[i] == iv2[j])
				{
				ct3[i] = j;
				cs3[i] = 1;
				ct2[j] = i;
				cs2[j] = 2;
				notfound = MB_NO;
				}
			if (notfound && iv3[i] == iv1[j] && iv1[i] == iv3[j])
				{
				ct3[i] = j;
				cs3[i] = 2;
				ct3[j] = i;
				cs3[j] = 2;
				notfound = MB_NO;
				}
			}
		  }
		}

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",function_name);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       ntri:             %d\n",*ntri);
		if (verbose >= 5)
			{
			fprintf(stderr,"dbg5       Output vertices:\n");
			for (i=0;i<*ntri;i++)
				fprintf(stderr,"dbg5       %3d  %3d %3d %3d\n",
				i,iv1[i],iv2[i],iv3[i]);
			fprintf(stderr,"dbg5       Output connectivity:\n");
			for (i=0;i<*ntri;i++)
				fprintf(stderr,"dbg5       %3d   %3d %3d   %3d %3d   %3d %3d\n",
				i,ct1[i],cs1[i],ct2[i],cs2[i],ct3[i],cs3[i]);
			}
		fprintf(stderr,"dbg2       error:      %d\n",*error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:     %d\n",status);
		}

	return(status);
}
/*--------------------------------------------------------------------------*/
