/*------------------------------------------------------------------------------
 *    The MB-system:	mbview_site.c	9/25/2003
 *    $Id: mbview_site.c,v 5.1 2004-02-24 22:52:28 caress Exp $
 *
 *    Copyright (c) 2003 by
 *    David W. Caress (caress@mbari.org)
 *      Monterey Bay Aquarium Research Institute
 *      Moss Landing, CA 95039
 *    and Dale N. Chayes (dale@ldeo.columbia.edu)
 *      Lamont-Doherty Earth Observatory
 *      Palisades, NY 10964
 *
 *    See README file for copying and redistribution conditions.
 *------------------------------------------------------------------------------*/
/*
 *
 * Author:	D. W. Caress
 * Date:	September 25, 2003
 *
 * Note:	This code was broken out of mbview_callbacks.c, which was
 *		begun on October 7, 2002
 *
 * $Log: not supported by cvs2svn $
 * Revision 5.0  2003/12/02 20:38:32  caress
 * Making version number 5.0
 *
 * Revision 1.2  2003/11/25 01:43:19  caress
 * MBview version generated during EW0310.
 *
 *
 */
/*------------------------------------------------------------------------------*/

/* Standard includes for builtins. */
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <math.h>

/* Motif required Headers */
#include <X11/StringDefs.h>
#include <X11/cursorfont.h>
#include <Xm/Xm.h>
#include <Xm/MainW.h>
#include <Xm/DialogS.h>
#include <Xm/RepType.h>
#include <Xm/MwmUtil.h>
#include <Xm/BulletinB.h>
#include <Xm/RowColumn.h>
#include <Xm/CascadeB.h>
#include <Xm/PushB.h>
#include <Xm/Separator.h>
#include "MB3DView.h"

/* OpenGL include files */
#include <GL/gl.h>
#include <GL/glu.h>
#include "GL/GLwMDrawA.h" 
#include <GL/glx.h>

/* MBIO include files */
#include "../../include/mb_status.h"
#include "../../include/mb_define.h"

/* mbview include */
#include "mbview.h"
#include "mbviewprivate.h"

/*------------------------------------------------------------------------------*/

/* library variables */
extern int	mbv_verbose;
extern int	mbv_ninstance;
extern Widget	parent_widget;
extern XtAppContext	app_context;
extern char	value_text[MB_PATH_MAXLINE];
extern struct mbview_world_struct mbviews[MBV_MAX_WINDOWS];
extern char	*mbsystem_library_name;

/* local variables */
Cardinal 	ac = 0;
Arg      	args[256];

static char rcs_id[]="$Id: mbview_site.c,v 5.1 2004-02-24 22:52:28 caress Exp $";

/*------------------------------------------------------------------------------*/
int mbview_getsitecount(int verbose, int instance,
			int *nsite,
			int *error)

{
	/* local variables */
	char	*function_name = "mbview_getsitecount";
	int	status = MB_SUCCESS;
	struct mbview_world_struct *view;
	struct mbview_struct *data;

fprintf(stderr,"Called mbview_getsitecount:%d\n",instance);

	/* print starting debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Version %s\n",rcs_id);
		fprintf(stderr,"dbg2  MB-system Version %s\n",MB_VERSION);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:                   %d\n", verbose);
		fprintf(stderr,"dbg2       instance:                  %d\n", instance);
		}

	/* get view */
	view = &(mbviews[instance]);
	data = &(view->data);

	/* get number of sites */
	*nsite = data->nsite;
		
	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       nsite:                     %d\n", *nsite);
		fprintf(stderr,"dbg2       error:                     %d\n", *error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:                    %d\n", status);
		}

	/* return */
	return(status);
}

/*------------------------------------------------------------------------------*/
int mbview_allocsitearrays(int verbose, 
			int	nsite,
			double	**sitelon,
			double	**sitelat,
			double	**sitetopo,
			int	**sitecolor,
			int	**sitesize,
			mb_path	**sitename,
			int 	*error)

{
	/* local variables */
	char	*function_name = "mbview_allocsitearrays";
	int	status = MB_SUCCESS;

fprintf(stderr,"Called mbview_allocsitearrays: nsite:%d\n", nsite);

	/* print starting debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Version %s\n",rcs_id);
		fprintf(stderr,"dbg2  MB-system Version %s\n",MB_VERSION);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:                   %d\n", verbose);
		fprintf(stderr,"dbg2       nsite:                     %d\n", nsite);
		fprintf(stderr,"dbg2       sitelon:                   %d\n", *sitelon);
		fprintf(stderr,"dbg2       sitelat:                   %d\n", *sitelat);
		fprintf(stderr,"dbg2       sitetopo:                  %d\n", *sitetopo);
		fprintf(stderr,"dbg2       sitecolor:                 %d\n", *sitecolor);
		fprintf(stderr,"dbg2       sitesize:                  %d\n", *sitesize);
		fprintf(stderr,"dbg2       sitename:                  %d\n", *sitename);
		}

	/* allocate the arrays using mb_realloc */
	status = mb_realloc(verbose,nsite*sizeof(double),sitelon,error);
	if (status == MB_SUCCESS)
		status = mb_realloc(verbose,nsite*sizeof(double),sitelat,error);
	if (status == MB_SUCCESS)
		status = mb_realloc(verbose,nsite*sizeof(double),sitetopo,error);
	if (status == MB_SUCCESS)
		status = mb_realloc(verbose,nsite*sizeof(int),sitecolor,error);
	if (status == MB_SUCCESS)
		status = mb_realloc(verbose,nsite*sizeof(int),sitesize,error);
	if (status == MB_SUCCESS)
		status = mb_realloc(verbose,nsite*sizeof(mb_path),sitename,error);

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       sitelon:                   %d\n", *sitelon);
		fprintf(stderr,"dbg2       sitelat:                   %d\n", *sitelat);
		fprintf(stderr,"dbg2       sitetopo:                  %d\n", *sitetopo);
		fprintf(stderr,"dbg2       sitecolor:                 %d\n", *sitecolor);
		fprintf(stderr,"dbg2       sitesize:                  %d\n", *sitesize);
		fprintf(stderr,"dbg2       sitename:                  %d\n", *sitename);
		fprintf(stderr,"dbg2       error:                     %d\n", *error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:                    %d\n", status);
		}

	/* return */
	return(status);
}


/*------------------------------------------------------------------------------*/
int mbview_freesitearrays(int verbose, 
			double	**sitelon,
			double	**sitelat,
			double	**sitetopo,
			int	**sitecolor,
			int	**sitesize,
			mb_path	**sitename,
			int *error)

{
	/* local variables */
	char	*function_name = "mbview_freesitearrays";
	int	status = MB_SUCCESS;

fprintf(stderr,"Called mbview_freesitearrays:\n");

	/* print starting debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Version %s\n",rcs_id);
		fprintf(stderr,"dbg2  MB-system Version %s\n",MB_VERSION);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:                   %d\n", verbose);
		fprintf(stderr,"dbg2       sitelon:                   %d\n", *sitelon);
		fprintf(stderr,"dbg2       sitelat:                   %d\n", *sitelat);
		fprintf(stderr,"dbg2       sitetopo:                  %d\n", *sitetopo);
		fprintf(stderr,"dbg2       sitecolor:                 %d\n", *sitecolor);
		fprintf(stderr,"dbg2       sitesize:                  %d\n", *sitesize);
		fprintf(stderr,"dbg2       sitename:                  %d\n", *sitename);
		}

	/* free the arrays using mb_free */
	status = mb_free(verbose,sitelon,error);
	status = mb_free(verbose,sitelat,error);
	status = mb_free(verbose,sitetopo,error);
	status = mb_free(verbose,sitecolor,error);
	status = mb_free(verbose,sitesize,error);
	status = mb_free(verbose,sitename,error);

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       sitelon:                   %d\n", *sitelon);
		fprintf(stderr,"dbg2       sitelat:                   %d\n", *sitelat);
		fprintf(stderr,"dbg2       sitetopo:                  %d\n", *sitetopo);
		fprintf(stderr,"dbg2       sitecolor:                 %d\n", *sitecolor);
		fprintf(stderr,"dbg2       sitesize:                  %d\n", *sitesize);
		fprintf(stderr,"dbg2       sitename:                  %d\n", *sitename);
		fprintf(stderr,"dbg2       error:                     %d\n", *error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:                    %d\n", status);
		}

	/* return */
	return(status);
}


/*------------------------------------------------------------------------------*/
int mbview_addsites(int verbose, int instance,
			int	nsite,
			double	*sitelon,
			double	*sitelat,
			double	*sitetopo,
			int	*sitecolor,
			int	*sitesize,
			mb_path	*sitename,
			int *error)

{
	/* local variables */
	char	*function_name = "mbview_addsites";
	int	status = MB_SUCCESS;
	struct mbview_world_struct *view;
	struct mbview_struct *data;
	double	xgrid, ygrid, zdata;
	double	xdisplay, ydisplay, zdisplay;
	int	nadded;
	int	i;

fprintf(stderr,"Called mbview_addsites:%d\n",instance);

	/* print starting debug statements */
	if (verbose >= 0)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Version %s\n",rcs_id);
		fprintf(stderr,"dbg2  MB-system Version %s\n",MB_VERSION);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:                   %d\n", verbose);
		fprintf(stderr,"dbg2       instance:                  %d\n", instance);
		fprintf(stderr,"dbg2       nsite:                     %d\n", nsite);
		fprintf(stderr,"dbg2       sitelon:                   %d\n", sitelon);
		fprintf(stderr,"dbg2       sitelat:                   %d\n", sitelat);
		fprintf(stderr,"dbg2       sitetopo:                  %d\n", sitetopo);
		fprintf(stderr,"dbg2       sitecolor:                 %d\n", sitecolor);
		fprintf(stderr,"dbg2       sitesize:                  %d\n", sitesize);
		fprintf(stderr,"dbg2       sitename:                  %d\n", sitename);
		for (i=0;i<nsite;i++)
			{
			fprintf(stderr,"dbg2       site:%d lon:%f lat:%f topo:%f color:%d size:%d name:%s\n", 
					i, sitelon[i], sitelat[i], sitetopo[i], 
					sitecolor[i], sitesize[i], sitename[i]);
			}
		}

	/* get view */
	view = &(mbviews[instance]);
	data = &(view->data);

	/* make sure no site is selected */
	data->site_selected = MBV_SELECT_NONE;

	/* allocate memory if required */
	if (data->nsite_alloc < data->nsite + nsite)
		{
fprintf(stderr,"Have %d sites allocated but need %d + %d = %d\n",
data->nsite_alloc, data->nsite, nsite, data->nsite + nsite);
		data->nsite_alloc = data->nsite + nsite;
		status = mb_realloc(mbv_verbose, 
			    	data->nsite_alloc * sizeof(struct mbview_site_struct),
			    	&(data->sites), &error);
		if (status == MB_FAILURE)
			{
			data->nsite_alloc = 0;
			}
		else
			{
			for (i=data->nsite;i<data->nsite_alloc;i++)
				{
				data->sites[i].color = MBV_COLOR_GREEN;
				data->sites[i].size = 1;
				data->sites[i].name[0] = '\0';
				}
			}
		}
	
	/* loop over the sites */
	nadded = 0;
	for (i=0;i<nsite;i++)
		{
		/* get site positions in grid coordinates */
		status = mbview_projectll2xyzgrid(instance,
				sitelon[i], sitelat[i], 
				&xgrid, &ygrid, &zdata);
				
		/* use provided topo */
		if (sitetopo[i] != MBV_DEFAULT_NODATA)
		    {
		    zdata = sitetopo[i];
		    }

		/* get site positions in display coordinates */
		status = mbview_projectll2display(instance,
				sitelon[i], sitelat[i], zdata,
				&xdisplay, &ydisplay, &zdisplay);

		/* add the new site */
		data->sites[data->nsite].point.xgrid = xgrid;
		data->sites[data->nsite].point.ygrid = ygrid;
		data->sites[data->nsite].point.xlon = sitelon[i];
		data->sites[data->nsite].point.ylat = sitelat[i];
		data->sites[data->nsite].point.zdata = zdata;
		data->sites[data->nsite].point.xdisplay = xdisplay;
		data->sites[data->nsite].point.ydisplay = ydisplay;
		data->sites[data->nsite].point.zdisplay = zdisplay;
		data->sites[data->nsite].color = sitecolor[i];
		data->sites[data->nsite].size = sitesize[i];
		strcpy(data->sites[data->nsite].name, sitename[i]);

		/* set nsite */
		data->nsite++;
		nadded++;
fprintf(stderr,"Added site %d added so far:%d total;%d\n",
data->nsite-1, nadded, data->nsite);
		}

	/* make sites viewable */
	if (nadded > 0)
		{
		data->site_view_mode = MBV_VIEW_ON;
		}
		
	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       error:                     %d\n",*error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:                    %d\n",status);
		}

	/* return */
	return(status);
}
/*------------------------------------------------------------------------------*/
int mbview_getsites(int verbose, int instance,
			int	*nsite,
			double	*sitelon,
			double	*sitelat,
			double	*sitetopo,
			int	*sitecolor,
			int	*sitesize,
			mb_path	*sitename,
			int *error)

{
	/* local variables */
	char	*function_name = "mbview_getsites";
	int	status = MB_SUCCESS;
	struct mbview_world_struct *view;
	struct mbview_struct *data;
	int	i;

fprintf(stderr,"Called mbview_getsites:%d\n",instance);

	/* print starting debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Version %s\n",rcs_id);
		fprintf(stderr,"dbg2  MB-system Version %s\n",MB_VERSION);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:                   %d\n", verbose);
		fprintf(stderr,"dbg2       instance:                  %d\n", instance);
		fprintf(stderr,"dbg2       nsite:                     %d\n", nsite);
		fprintf(stderr,"dbg2       sitelon:                   %d\n", sitelon);
		fprintf(stderr,"dbg2       sitelat:                   %d\n", sitelat);
		fprintf(stderr,"dbg2       sitetopo:                  %d\n", sitetopo);
		fprintf(stderr,"dbg2       sitecolor:                 %d\n", sitecolor);
		fprintf(stderr,"dbg2       sitesize:                  %d\n", sitesize);
		fprintf(stderr,"dbg2       sitename:                  %d\n", sitename);
		}

	/* get view */
	view = &(mbviews[instance]);
	data = &(view->data);
	
	/* check that the array pointers are not NULL */
	if (sitelon == NULL || sitelat == NULL || sitetopo == NULL 
		|| sitecolor == NULL || sitesize == NULL || sitename == NULL)
		{
		status = MB_FAILURE;
		*error = MB_ERROR_DATA_NOT_INSERTED;
		}
	
	/* otherwise go get the site data */
	else
		{	
		/* loop over the sites */
		*nsite = data->nsite;
		for (i=0;i<*nsite;i++)
			{
			sitelon[i] = data->sites[i].point.xlon;
			sitelat[i] = data->sites[i].point.ylat ;
			sitetopo[i] = data->sites[i].point.zdata;
			sitecolor[i] = data->sites[i].color;
			sitesize[i] = data->sites[i].size;
			strcpy(sitename[i], data->sites[i].name);
			}
		}
		
	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       nsite:                     %d\n", *nsite);
		for (i=0;i<*nsite;i++)
			{
			fprintf(stderr,"dbg2       site:%d lon:%f lat:%f topo:%f color:%d size:%d name:%s\n", 
					i, sitelon[i], sitelat[i], sitetopo[i], 
					sitecolor[i], sitesize[i],
					sitename[i]);
			}
		fprintf(stderr,"dbg2       error:                     %d\n",*error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:                    %d\n",status);
		}

	/* return */
	return(status);
}

/*--------------------------------------------------------------------*/
int mbview_enableviewsites(int verbose, int instance,
			int *error)

{
	/* local variables */
	char	*function_name = "mbview_enableviewsites";
	int	status = MB_SUCCESS;
	struct mbview_world_struct *view;
	struct mbview_struct *data;

fprintf(stderr,"Called mbview_enableviewsites:%d\n",instance);

	/* print starting debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Version %s\n",rcs_id);
		fprintf(stderr,"dbg2  MB-system Version %s\n",MB_VERSION);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:                   %d\n", verbose);
		fprintf(stderr,"dbg2       instance:                  %d\n", instance);
		}

	/* get view */
	view = &(mbviews[instance]);
	data = &(view->data);
	
	/* set values */
        data->site_mode = MBV_SITE_VIEW;
		
	/* set widget sensitivity */
	if (data->active == MB_YES)
		mbview_update_sensitivity(mbv_verbose, instance, error);
		
	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       error:                     %d\n",*error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:                    %d\n",status);
		}

	/* return */
	return(status);
}

/*--------------------------------------------------------------------*/
int mbview_enableeditsites(int verbose, int instance,
			int *error)

{
	/* local variables */
	char	*function_name = "mbview_enableeditsites";
	int	status = MB_SUCCESS;
	struct mbview_world_struct *view;
	struct mbview_struct *data;

fprintf(stderr,"Called mbview_enableeditsites:%d\n",instance);

	/* print starting debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Version %s\n",rcs_id);
		fprintf(stderr,"dbg2  MB-system Version %s\n",MB_VERSION);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:                   %d\n", verbose);
		fprintf(stderr,"dbg2       instance:                  %d\n", instance);
		}

	/* get view */
	view = &(mbviews[instance]);
	data = &(view->data);
	
	/* set values */
        data->site_mode = MBV_SITE_EDIT;
		
	/* set widget sensitivity */
	if (data->active == MB_YES)
		mbview_update_sensitivity(mbv_verbose, instance, error);
		
	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       error:                     %d\n",*error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:                    %d\n",status);
		}

	/* return */
	return(status);
}

/*------------------------------------------------------------------------------*/
int mbview_pick_site_select(int instance, int which, int xpixel, int ypixel)
{

	/* local variables */
	char	*function_name = "mbview_pick_site_select";
	int	status = MB_SUCCESS;
	struct mbview_world_struct *view;
	struct mbview_struct *data;
	int	found;
	double	xgrid, ygrid;
	double	xlon, ylat, zdata;
	double	xdisplay, ydisplay, zdisplay;
	double	xx, yy, rr, rrmin;
	int	i;

	/* print starting debug statements */
	if (mbv_verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Version %s\n",rcs_id);
		fprintf(stderr,"dbg2  MB-system Version %s\n",MB_VERSION);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       instance:         %d\n",instance);
		fprintf(stderr,"dbg2       which:            %d\n",which);
		fprintf(stderr,"dbg2       xpixel:           %d\n",xpixel);
		fprintf(stderr,"dbg2       ypixel:           %d\n",ypixel);
		}
		
	/* get view */
	view = &(mbviews[instance]);
	data = &(view->data);
	
	/* only select sites if enabled and not in move mode */
	if (data->site_mode != MBV_SITE_OFF
		&& data->nsite > 0
		&& (which == MBV_PICK_DOWN
			|| data->site_selected == MBV_SELECT_NONE))
		{
		/* look for point */
		mbview_findpoint(instance, xpixel, ypixel, 
				&found, 
				&xgrid, &ygrid,
				&xlon, &ylat, &zdata,
				&xdisplay, &ydisplay, &zdisplay);

		/* look for nearest site */
		if (found == MB_YES)
			{
			/* first deselect previously selected site */
			data->site_selected = MBV_SELECT_NONE;
			
			/* now figure out which site will be selected next */
			rrmin = 1000000000.0;
			for (i=0;i<data->nsite;i++)
				{
				xx = xgrid - data->sites[i].point.xgrid;
				yy = ygrid - data->sites[i].point.ygrid;
				rr = sqrt(xx * xx + yy * yy);
				if (rr < rrmin)
					{
					rrmin = rr;
					data->site_selected = i;
					}
				}
			}
		else if (data->site_selected == MBV_SELECT_NONE)
			{
			XBell(view->dpy,100);
			}
		else
			{
			/* deselect previously selected site */
			data->site_selected = MBV_SELECT_NONE;
			}
		}
	
	/* only move selected sites if enabled */
	else if (data->site_mode != MBV_SITE_OFF
		&& data->nsite > 0
		&& (which == MBV_PICK_MOVE
			&& data->site_selected != MBV_SELECT_NONE))
		{
		/* look for point */
		mbview_findpoint(instance, xpixel, ypixel, 
				&found, 
				&xgrid, &ygrid,
				&xlon, &ylat, &zdata,
				&xdisplay, &ydisplay, &zdisplay);
				
		/* reset selected site position */
		if (found)
			{
			data->sites[data->site_selected].point.xgrid = xgrid;
			data->sites[data->site_selected].point.ygrid = ygrid;
			data->sites[data->site_selected].point.xlon = xlon;
			data->sites[data->site_selected].point.ylat = ylat;
			data->sites[data->site_selected].point.zdata = zdata;
			data->sites[data->site_selected].point.xdisplay = xdisplay;
			data->sites[data->site_selected].point.ydisplay = ydisplay;
			data->sites[data->site_selected].point.zdisplay = zdisplay;
			}
		}

	/* else beep */
	else
		{
		if (data->site_selected != MBV_SELECT_NONE)
			{
			/* deselect previously selected site */
			data->site_selected = MBV_SELECT_NONE;
			}

		XBell(view->dpy,100);
		}
		
	/* set what kind of pick to annotate */
	if (data->site_selected != MBV_SELECT_NONE)
		{
		data->pickinfo_mode = MBV_PICK_SITE;
		}
	else
		{
		data->pickinfo_mode = data->pick_type;
		}
		
	/* set pick annotation */
	mbview_pick_text(instance);
	
	/* print site debug statements */
	if (mbv_verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  Site data altered in function <%s>\n",
			function_name);
		fprintf(stderr,"dbg2  Site values:\n");
		fprintf(stderr,"dbg2       site_view_mode:      %d\n",data->site_view_mode);
		fprintf(stderr,"dbg2       site_mode:           %d\n",data->site_mode);
		fprintf(stderr,"dbg2       nsite:               %d\n",data->nsite);
		fprintf(stderr,"dbg2       nsite_alloc:         %d\n",data->nsite_alloc);
		fprintf(stderr,"dbg2       site_selected:       %d\n",data->site_selected);
		for (i=0;i<data->nsite;i++)
			{
			fprintf(stderr,"dbg2       site %d xgrid:       %f\n",i,data->sites[i].point.xgrid);
			fprintf(stderr,"dbg2       site %d ygrid:       %f\n",i,data->sites[i].point.ygrid);
			fprintf(stderr,"dbg2       site %d xlon:        %f\n",i,data->sites[i].point.xlon);
			fprintf(stderr,"dbg2       site %d ylat:        %f\n",i,data->sites[i].point.ylat);
			fprintf(stderr,"dbg2       site %d zdata:       %f\n",i,data->sites[i].point.zdata);
			fprintf(stderr,"dbg2       site %d xdisplay:    %f\n",i,data->sites[i].point.xdisplay);
			fprintf(stderr,"dbg2       site %d ydisplay:    %f\n",i,data->sites[i].point.ydisplay);
			fprintf(stderr,"dbg2       site %d zdisplay:    %f\n",i,data->sites[i].point.zdisplay);
			fprintf(stderr,"dbg2       site %d color:       %d\n",i,data->sites[i].color);
			fprintf(stderr,"dbg2       site %d size:        %d\n",i,data->sites[i].size);
			fprintf(stderr,"dbg2       site %d name:        %s\n",i,data->sites[i].name);
			}
		}
	
	/* print output debug statements */
	if (mbv_verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:          %d\n",status);
		}

	/* return */
	return(status);
}


/*------------------------------------------------------------------------------*/
int mbview_pick_site_add(int instance, int which, int xpixel, int ypixel)
{

	/* local variables */
	char	*function_name = "mbview_pick_site_add";
	int	status = MB_SUCCESS;
	int	error = MB_ERROR_NO_ERROR;
	struct mbview_world_struct *view;
	struct mbview_struct *data;
	int	found;
	double	xgrid, ygrid;
	double	xlon, ylat, zdata;
	double	xdisplay, ydisplay, zdisplay;
	double	xx, yy, rr, rrmin;
	int	i, inew;

	/* print starting debug statements */
	if (mbv_verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Version %s\n",rcs_id);
		fprintf(stderr,"dbg2  MB-system Version %s\n",MB_VERSION);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       instance:         %d\n",instance);
		fprintf(stderr,"dbg2       which:            %d\n",which);
		fprintf(stderr,"dbg2       xpixel:           %d\n",xpixel);
		fprintf(stderr,"dbg2       ypixel:           %d\n",ypixel);
		}
		
	/* get view */
	view = &(mbviews[instance]);
	data = &(view->data);
	
	/* only add sites if enabled and not in move mode */
	if (data->site_mode == MBV_SITE_EDIT
		&& (which == MBV_PICK_DOWN
			|| data->site_selected == MBV_SELECT_NONE))
		{
		/* look for point */
		mbview_findpoint(instance, xpixel, ypixel, 
				&found, 
				&xgrid, &ygrid,
				&xlon, &ylat, &zdata,
				&xdisplay, &ydisplay, &zdisplay);

		/* add site */
		if (found)
			{
			/* add site after currently selected site if any */
			if (data->site_selected == MBV_SELECT_NONE)
				inew = data->nsite;
			else 
				{
				inew = data->site_selected + 1;
				data->site_selected = MBV_SELECT_NONE;
				}
			
			/* now figure out which site will be selected next */
				
			/* allocate memory if required */
			if (data->nsite_alloc < data->nsite + 1)
				{
				data->nsite_alloc += MBV_ALLOC_NUM;
				status = mb_realloc(mbv_verbose, 
			    			data->nsite_alloc * sizeof(struct mbview_site_struct),
			    			&(data->sites), &error);
				if (status == MB_FAILURE)
					{
					data->nsite_alloc = 0;
					}
				else
					{
					for (i=data->nsite;i<data->nsite_alloc;i++)
						{
						data->sites[i].color = MBV_COLOR_GREEN;
						data->sites[i].size = 1;
						data->sites[i].name[0] = '\0';
						}
					}
				}
			
			/* move old sites if necessary */
			for (i=data->nsite;i>inew;i--)
				{
				data->sites[i] = data->sites[i-1];
				}
			
			/* add the new site */
			data->sites[inew].point.xgrid = xgrid;
			data->sites[inew].point.ygrid = ygrid;
			data->sites[inew].point.xlon = xlon;
			data->sites[inew].point.ylat = ylat;
			data->sites[inew].point.zdata = zdata;
			data->sites[inew].point.xdisplay = xdisplay;
			data->sites[inew].point.ydisplay = ydisplay;
			data->sites[inew].point.zdisplay = zdisplay;
			data->sites[inew].color = MBV_COLOR_GREEN;
			data->sites[inew].size = 1;
			sprintf(data->sites[inew].name,"Site %d", data->nsite);
			
			/* set nsite */
			data->nsite++;
			
			/* select the new site */
			data->site_selected = inew;
			}
		else if (data->site_selected == MBV_SELECT_NONE)
			{
			XBell(view->dpy,100);
			}
		}
	
	/* only move selected sites if enabled */
	else if (data->site_mode != MBV_SITE_OFF
		&& data->nsite > 0
		&& (which == MBV_PICK_MOVE
			&& data->site_selected != MBV_SELECT_NONE))
		{
		/* look for point */
		mbview_findpoint(instance, xpixel, ypixel, 
				&found, 
				&xgrid, &ygrid,
				&xlon, &ylat, &zdata,
				&xdisplay, &ydisplay, &zdisplay);
				
		/* reset selected site position */
		if (found)
			{
			data->sites[data->site_selected].point.xgrid = xgrid;
			data->sites[data->site_selected].point.ygrid = ygrid;
			data->sites[data->site_selected].point.xlon = xlon;
			data->sites[data->site_selected].point.ylat = ylat;
			data->sites[data->site_selected].point.zdata = zdata;
			data->sites[data->site_selected].point.xdisplay = xdisplay;
			data->sites[data->site_selected].point.ydisplay = ydisplay;
			data->sites[data->site_selected].point.zdisplay = zdisplay;
			}
		}

	/* else beep */
	else
		{
		if (data->site_selected != MBV_SELECT_NONE)
			{
			/* deselect previously selected site */
			data->site_selected = MBV_SELECT_NONE;
			}
		XBell(view->dpy,100);
		}
		
	/* set what kind of pick to annotate */
	if (data->site_selected != MBV_SELECT_NONE)
		{
		data->pickinfo_mode = MBV_PICK_SITE;
		}
	else
		{
		data->pickinfo_mode = data->pick_type;
		}
		
	/* set pick annotation */
	mbview_pick_text(instance);
	
	/* print site debug statements */
	if (mbv_verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  Site data altered in function <%s>\n",
			function_name);
		fprintf(stderr,"dbg2  Site values:\n");
		fprintf(stderr,"dbg2       site_view_mode:      %d\n",data->site_view_mode);
		fprintf(stderr,"dbg2       site_mode:           %d\n",data->site_mode);
		fprintf(stderr,"dbg2       nsite:               %d\n",data->nsite);
		fprintf(stderr,"dbg2       nsite_alloc:         %d\n",data->nsite_alloc);
		fprintf(stderr,"dbg2       site_selected:       %d\n",data->site_selected);
		for (i=0;i<data->nsite;i++)
			{
			fprintf(stderr,"dbg2       site %d xgrid:       %f\n",i,data->sites[i].point.xgrid);
			fprintf(stderr,"dbg2       site %d ygrid:       %f\n",i,data->sites[i].point.ygrid);
			fprintf(stderr,"dbg2       site %d xlon:        %f\n",i,data->sites[i].point.xlon);
			fprintf(stderr,"dbg2       site %d ylat:        %f\n",i,data->sites[i].point.ylat);
			fprintf(stderr,"dbg2       site %d zdata:       %f\n",i,data->sites[i].point.zdata);
			fprintf(stderr,"dbg2       site %d xdisplay:    %f\n",i,data->sites[i].point.xdisplay);
			fprintf(stderr,"dbg2       site %d ydisplay:    %f\n",i,data->sites[i].point.ydisplay);
			fprintf(stderr,"dbg2       site %d zdisplay:    %f\n",i,data->sites[i].point.zdisplay);
			fprintf(stderr,"dbg2       site %d color:       %d\n",i,data->sites[i].color);
			fprintf(stderr,"dbg2       site %d size:        %d\n",i,data->sites[i].size);
			fprintf(stderr,"dbg2       site %d name:        %s\n",i,data->sites[i].name);
			}
		}
	
	/* print output debug statements */
	if (mbv_verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:          %d\n",status);
		}

	/* return */
	return(status);
}


/*------------------------------------------------------------------------------*/
int mbview_pick_site_delete(int instance, int xpixel, int ypixel)
{

	/* local variables */
	char	*function_name = "mbview_pick_site_delete";
	int	status = MB_SUCCESS;
	struct mbview_world_struct *view;
	struct mbview_struct *data;
	int	found;
	double	xgrid, ygrid;
	double	xlon, ylat, zdata;
	double	xdisplay, ydisplay, zdisplay;
	double	xx, yy, rr, rrmin;
	int	i, idelete;

	/* print starting debug statements */
	if (mbv_verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Version %s\n",rcs_id);
		fprintf(stderr,"dbg2  MB-system Version %s\n",MB_VERSION);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       instance:         %d\n",instance);
		fprintf(stderr,"dbg2       xpixel:           %d\n",xpixel);
		fprintf(stderr,"dbg2       ypixel:           %d\n",ypixel);
		}
		
	/* get view */
	view = &(mbviews[instance]);
	data = &(view->data);
	
	/* only delete a selected site if enabled */
	if (data->site_mode == MBV_SITE_EDIT
		&& data->site_selected != MBV_SELECT_NONE)
		{
		/* look for point */
		mbview_findpoint(instance, xpixel, ypixel, 
				&found, 
				&xgrid, &ygrid,
				&xlon, &ylat, &zdata,
				&xdisplay, &ydisplay, &zdisplay);

		/* find closest site to pick point */
		if (found)
			{
			rrmin = 1000000000.0;
			idelete = MBV_SELECT_NONE;
			for (i=0;i<data->nsite;i++)
				{
				xx = xgrid - data->sites[i].point.xgrid;
				yy = ygrid - data->sites[i].point.ygrid;
				rr = sqrt(xx * xx + yy * yy);
				if (rr < rrmin)
					{
					rrmin = rr;
					idelete = i;
					}
				}
			}

		/* delete site if its the same as previously selected */
		if (found && data->site_selected == idelete)
			{
			
			/* move site data if necessary */
			for (i=idelete;i<data->nsite-1;i++)
				{
				data->sites[i] = data->sites[i+1];
				}
			
			/* set nsite */
			data->nsite--;
			
			/* select the new site */
			data->site_selected = MBV_SELECT_NONE;
			}
		else
			{
			XBell(view->dpy,100);
			}
		}

	/* else beep */
	else
		{
		XBell(view->dpy,100);
		}
		
	/* set what kind of pick to annotate */
	if (data->site_selected != MBV_SELECT_NONE)
		{
		data->pickinfo_mode = MBV_PICK_SITE;
		}
	else
		{
		data->pickinfo_mode = data->pick_type;
		}
		
	/* set pick annotation */
	mbview_pick_text(instance);
	
	/* print site debug statements */
	if (mbv_verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  Site data altered in function <%s>\n",
			function_name);
		fprintf(stderr,"dbg2  Site values:\n");
		fprintf(stderr,"dbg2       site_view_mode:      %d\n",data->site_view_mode);
		fprintf(stderr,"dbg2       site_mode:           %d\n",data->site_mode);
		fprintf(stderr,"dbg2       nsite:               %d\n",data->nsite);
		fprintf(stderr,"dbg2       nsite_alloc:         %d\n",data->nsite_alloc);
		fprintf(stderr,"dbg2       site_selected:       %d\n",data->site_selected);
		for (i=0;i<data->nsite;i++)
			{
			fprintf(stderr,"dbg2       site %d xgrid:       %f\n",i,data->sites[i].point.xgrid);
			fprintf(stderr,"dbg2       site %d ygrid:       %f\n",i,data->sites[i].point.ygrid);
			fprintf(stderr,"dbg2       site %d xlon:        %f\n",i,data->sites[i].point.xlon);
			fprintf(stderr,"dbg2       site %d ylat:        %f\n",i,data->sites[i].point.ylat);
			fprintf(stderr,"dbg2       site %d zdata:       %f\n",i,data->sites[i].point.zdata);
			fprintf(stderr,"dbg2       site %d xdisplay:    %f\n",i,data->sites[i].point.xdisplay);
			fprintf(stderr,"dbg2       site %d ydisplay:    %f\n",i,data->sites[i].point.ydisplay);
			fprintf(stderr,"dbg2       site %d zdisplay:    %f\n",i,data->sites[i].point.zdisplay);
			fprintf(stderr,"dbg2       site %d color:       %d\n",i,data->sites[i].color);
			fprintf(stderr,"dbg2       site %d size:        %d\n",i,data->sites[i].size);
			fprintf(stderr,"dbg2       site %d name:        %s\n",i,data->sites[i].name);
			}
		}
	
	/* print output debug statements */
	if (mbv_verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:          %d\n",status);
		}

	/* return */
	return(status);
}

/*------------------------------------------------------------------------------*/
int mbview_drawsite(int instance, int rez)
{
	/* local variables */
	char	*function_name = "mbview_drawsite";
	int	status = MB_SUCCESS;
	struct mbview_world_struct *view;
	struct mbview_struct *data;
	int	on, flip;
	int	stride;
	int	ixmin, ixmax, jymin, jymax;
	int	ix, jy;
	int	ixsize, jysize;
	int	isite;
	int	icolor;
	int	i, j, k, l, m, n, kk, ll;
	

	/* print starting debug statements */
	if (mbv_verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Version %s\n",rcs_id);
		fprintf(stderr,"dbg2  MB-system Version %s\n",MB_VERSION);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       instance:         %d\n",instance);
		fprintf(stderr,"dbg2       rez:              %d\n",rez);
		}
		
	/* get view */
	view = &(mbviews[instance]);
	data = &(view->data);
		
	/* draw sites in 2D */
	if (data->site_mode != MBV_SITE_OFF
		&& data->display_mode == MBV_DISPLAY_2D
		&& data->site_view_mode == MBV_VIEW_ON
		&& data->nsite > 0)
		{
		/* set stride for looping over data */
		stride = 1;
	
		/* loop over the sites */
		for (isite=0;isite<data->nsite;isite++)
			{
			/* get grid bounds for plotting */
			ix = (data->sites[isite].point.xgrid - data->primary_xmin) / data->primary_dx;
			jy = (data->sites[isite].point.ygrid - data->primary_ymin) / data->primary_dy;
			if (isite == data->site_selected)
				ixsize = MAX(data->primary_nx, data->primary_ny) / 100;
			else
				ixsize = MAX(data->primary_nx, data->primary_ny) / 125;
			if (ixsize < 1) ixsize = 1;
			jysize = (int)((ixsize * (1000.0 * data->primary_dy / data->primary_dx)) / 1000.0);
			if (ixsize < 1) ixsize = 1;
			ixmin = MAX(ix - ixsize, 0);
			ixmax = MIN(ix + ixsize, data->primary_nx - stride);
			jymin = MAX(jy - jysize, 0);
			jymax = MIN(jy + jysize, data->primary_ny - stride);

			/* set the color for this site */
			if (isite == data->site_selected)
				icolor = MBV_COLOR_RED;
			else
				icolor = MBV_COLOR_GREEN;
			glColor3f(colortable_object_red[icolor], 
				colortable_object_green[icolor], 
				colortable_object_blue[icolor]);

			/* draw the site box */
			glLineWidth(2.0);
			glBegin(GL_QUADS);
			k = ixmin * data->primary_ny + jymin;
			glVertex3f(data->primary_x[k],
				   data->primary_y[k],
				   data->primary_z[k] + MBV_OPENGL_3D_LINE_OFFSET);
			k = ixmax * data->primary_ny + jymin;
			glVertex3f(data->primary_x[k],
				   data->primary_y[k],
				   data->primary_z[k] + MBV_OPENGL_3D_LINE_OFFSET);
			k = ixmax * data->primary_ny + jymax;
			glVertex3f(data->primary_x[k],
				   data->primary_y[k],
				   data->primary_z[k] + MBV_OPENGL_3D_LINE_OFFSET);
			k = ixmin * data->primary_ny + jymax;
			glVertex3f(data->primary_x[k],
				   data->primary_y[k],
				   data->primary_z[k] + MBV_OPENGL_3D_LINE_OFFSET);
			glEnd();

			/* draw the boundary */
			glColor3f(colortable_object_red[MBV_COLOR_BLACK], 
				colortable_object_green[MBV_COLOR_BLACK], 
				colortable_object_blue[MBV_COLOR_BLACK]);
			glLineWidth(2.0);
			glBegin(GL_LINE_LOOP);
			k = ixmin * data->primary_ny + jymin;
			glVertex3f(data->primary_x[k],
				   data->primary_y[k],
				   data->primary_z[k] + MBV_OPENGL_3D_LINE_OFFSET);
			k = ixmax * data->primary_ny + jymin;
			glVertex3f(data->primary_x[k],
				   data->primary_y[k],
				   data->primary_z[k] + MBV_OPENGL_3D_LINE_OFFSET);
			k = ixmax * data->primary_ny + jymax;
			glVertex3f(data->primary_x[k],
				   data->primary_y[k],
				   data->primary_z[k] + MBV_OPENGL_3D_LINE_OFFSET);
			k = ixmin * data->primary_ny + jymax;
			glVertex3f(data->primary_x[k],
				   data->primary_y[k],
				   data->primary_z[k] + MBV_OPENGL_3D_LINE_OFFSET);
			glEnd();
			}
		}
		
	/* draw sites in 3D */
	else if (data->site_mode != MBV_SITE_OFF
		&& data->display_mode == MBV_DISPLAY_3D
		&& data->site_view_mode == MBV_VIEW_ON
		&& data->nsite > 0)
		{
		/* set stride for looping over data */
		if (rez == MBV_REZ_FULL)
		    stride = 1;
		else if (rez == MBV_REZ_HIGH)
		    stride = MAX((int)ceil(((double)data->primary_nx) 
					/ ((double)data->hirez_dimension)), 
				(int)ceil(((double)data->primary_ny) 
					/ ((double)data->hirez_dimension)));
		else
		    stride = MAX((int)ceil(((double)data->primary_nx) 
					/ ((double)data->lorez_dimension)), 
				(int)ceil(((double)data->primary_ny) 
					/ ((double)data->lorez_dimension)));
	
		/* loop over the sites */
		for (isite=0;isite<data->nsite;isite++)
			{
			/* get grid bounds for plotting */
			ix = (data->sites[isite].point.xgrid - data->primary_xmin) / data->primary_dx;
			jy = (data->sites[isite].point.ygrid - data->primary_ymin) / data->primary_dy;
			if (isite == data->site_selected)
				ixsize = MAX(data->primary_nx, data->primary_ny) / 400;
			else
				ixsize = MAX(data->primary_nx, data->primary_ny) / 500;
			if (ixsize < 1) ixsize = 1;
			jysize = (int)((ixsize * (1000.0 * data->primary_dy / data->primary_dx)) / 1000.0);
			if (ixsize < 1) ixsize = 1;
			ixmin = MAX(stride * ((ix - ixsize) / stride), 0);
			ixmax = MIN(stride * ((ix + ixsize) / stride + 1), 
					data->primary_nx - stride);
			jymin = MAX(stride * ((jy - jysize) / stride), 0);
			jymax = MIN(stride * ((jy + jysize) / stride + 1), 
					data->primary_ny - stride);

			/* set the color for this site */
			if (isite == data->site_selected)
				icolor = MBV_COLOR_RED;
			else
				icolor = MBV_COLOR_GREEN;
			glColor3f(colortable_object_red[icolor], 
				colortable_object_green[icolor], 
				colortable_object_blue[icolor]);

			/* draw the data as triangle strips */
			for (i=ixmin;i<ixmax;i+=stride)
				{
				on = MB_NO;
				flip = MB_NO;
				for (j=jymin;j<=jymax;j+=stride)
					{
					k = i * data->primary_ny + j;
					l = (i + stride) * data->primary_ny + j;
					if (flip == MB_NO)
						{
						kk = k;
						ll = l;
						}
					else
						{
						kk = l;
						ll = k;
						}
					if (data->primary_data[kk] != data->primary_nodatavalue)
						{
						if (on == MB_NO)
							{
							glBegin(GL_TRIANGLE_STRIP);
							on = MB_YES;
							if (kk == k)
								flip = MB_NO;
							else
								flip = MB_YES;
							}
						glVertex3f(data->primary_x[kk],
							data->primary_y[kk],
							data->primary_z[kk]+MBV_OPENGL_3D_LINE_OFFSET);
						}
					else
						{
						glEnd();
						on = MB_NO;
						flip = MB_NO;
						}
					if (data->primary_data[ll] != data->primary_nodatavalue)
						{
						if (on == MB_NO)
							{
							glBegin(GL_TRIANGLE_STRIP);
							on = MB_YES;
							if (ll == l)
								flip = MB_NO;
							else
								flip = MB_YES;
							}
						glVertex3f(data->primary_x[ll],
							data->primary_y[ll],
							data->primary_z[ll]+MBV_OPENGL_3D_LINE_OFFSET);
						}
					else
						{
						glEnd();
						on = MB_NO;
						flip = MB_NO;
						}
					}
				if (on == MB_YES)
					{
					glEnd();
					on = MB_NO;
					flip = MB_NO;
					}
				glEnd();
				}

			/* draw the boundary */
			glColor3f(colortable_object_red[MBV_COLOR_BLACK], 
				colortable_object_green[MBV_COLOR_BLACK], 
				colortable_object_blue[MBV_COLOR_BLACK]);
			glLineWidth(2.0);
			glBegin(GL_LINE_LOOP);
			k = ixmin * data->primary_ny + jymin;
			glVertex3f(data->primary_x[k],
				   data->primary_y[k],
				   data->primary_z[k] + MBV_OPENGL_3D_LINE_OFFSET);
			k = ixmax * data->primary_ny + jymin;
			glVertex3f(data->primary_x[k],
				   data->primary_y[k],
				   data->primary_z[k] + MBV_OPENGL_3D_LINE_OFFSET);
			k = ixmax * data->primary_ny + jymax;
			glVertex3f(data->primary_x[k],
				   data->primary_y[k],
				   data->primary_z[k] + MBV_OPENGL_3D_LINE_OFFSET);
			k = ixmin * data->primary_ny + jymax;
			glVertex3f(data->primary_x[k],
				   data->primary_y[k],
				   data->primary_z[k] + MBV_OPENGL_3D_LINE_OFFSET);
			glEnd();
			}
		}

	/* print output debug statements */
	if (mbv_verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:  %d\n",status);
		}

	/* return */
	return(status);
}

/*------------------------------------------------------------------------------*/
