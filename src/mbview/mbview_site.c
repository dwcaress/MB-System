/*------------------------------------------------------------------------------
 *    The MB-system:	mbview_site.c	9/25/2003
 *    $Id: mbview_site.c,v 5.2 2005-02-02 08:23:51 caress Exp $
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
 * Revision 5.1  2004/02/24 22:52:28  caress
 * Added spherical projection to MBview.
 *
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
#include "MB3DSiteList.h"
#include "MB3DRouteList.h"
#include "MB3DNavList.h"

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
char	value_string[MB_PATH_MAXLINE];

static char rcs_id[]="$Id: mbview_site.c,v 5.2 2005-02-02 08:23:51 caress Exp $";

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
	*nsite = shared.shareddata.nsite;
		
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
	shared.shareddata.site_selected = MBV_SELECT_NONE;

	/* allocate memory if required */
	if (shared.shareddata.nsite_alloc < shared.shareddata.nsite + nsite)
		{
fprintf(stderr,"Have %d sites allocated but need %d + %d = %d\n",
shared.shareddata.nsite_alloc, shared.shareddata.nsite, nsite, shared.shareddata.nsite + nsite);
		shared.shareddata.nsite_alloc = shared.shareddata.nsite + nsite;
		status = mb_realloc(mbv_verbose, 
			    	shared.shareddata.nsite_alloc * sizeof(struct mbview_site_struct),
			    	&(shared.shareddata.sites), &error);
		if (status == MB_FAILURE)
			{
			shared.shareddata.nsite_alloc = 0;
			}
		else
			{
			for (i=shared.shareddata.nsite;i<shared.shareddata.nsite_alloc;i++)
				{
				shared.shareddata.sites[i].color = MBV_COLOR_GREEN;
				shared.shareddata.sites[i].size = 1;
				shared.shareddata.sites[i].name[0] = '\0';
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
		shared.shareddata.sites[shared.shareddata.nsite].point.xgrid[instance] = xgrid;
		shared.shareddata.sites[shared.shareddata.nsite].point.ygrid[instance] = ygrid;
		shared.shareddata.sites[shared.shareddata.nsite].point.xlon = sitelon[i];
		shared.shareddata.sites[shared.shareddata.nsite].point.ylat = sitelat[i];
		shared.shareddata.sites[shared.shareddata.nsite].point.zdata = zdata;
		shared.shareddata.sites[shared.shareddata.nsite].point.xdisplay[instance] = xdisplay;
		shared.shareddata.sites[shared.shareddata.nsite].point.ydisplay[instance] = ydisplay;
		shared.shareddata.sites[shared.shareddata.nsite].point.zdisplay[instance] = zdisplay;
		shared.shareddata.sites[shared.shareddata.nsite].color = sitecolor[i];
		shared.shareddata.sites[shared.shareddata.nsite].size = sitesize[i];
		strcpy(shared.shareddata.sites[shared.shareddata.nsite].name, sitename[i]);
		
		/* set grid and display coordinates for all instances */
		mbview_updatepointw(instance, &(shared.shareddata.sites[shared.shareddata.nsite].point));

		/* set nsite */
		shared.shareddata.nsite++;
		nadded++;
fprintf(stderr,"Added site %d added so far:%d total;%d\n",
shared.shareddata.nsite-1, nadded, shared.shareddata.nsite);
		}

	/* make sites viewable */
	if (nadded > 0)
		{
		data->site_view_mode = MBV_VIEW_ON;
		}
	
	/* update site list */
	mbview_updatesitelist();
		
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
		*nsite = shared.shareddata.nsite;
		for (i=0;i<*nsite;i++)
			{
			sitelon[i] = shared.shareddata.sites[i].point.xlon;
			sitelat[i] = shared.shareddata.sites[i].point.ylat ;
			sitetopo[i] = shared.shareddata.sites[i].point.zdata;
			sitecolor[i] = shared.shareddata.sites[i].color;
			sitesize[i] = shared.shareddata.sites[i].size;
			strcpy(sitename[i], shared.shareddata.sites[i].name);
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
        shared.shareddata.site_mode = MBV_SITE_VIEW;
		
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
        shared.shareddata.site_mode = MBV_SITE_EDIT;
		
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
	if (shared.shareddata.site_mode != MBV_SITE_OFF
		&& shared.shareddata.nsite > 0
		&& (which == MBV_PICK_DOWN
			|| shared.shareddata.site_selected == MBV_SELECT_NONE))
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
			shared.shareddata.site_selected = MBV_SELECT_NONE;
			
			/* now figure out which site will be selected next */
			rrmin = 1000000000.0;
			for (i=0;i<shared.shareddata.nsite;i++)
				{
				xx = xgrid - shared.shareddata.sites[i].point.xgrid[instance];
				yy = ygrid - shared.shareddata.sites[i].point.ygrid[instance];
				rr = sqrt(xx * xx + yy * yy);
				if (rr < rrmin)
					{
					rrmin = rr;
					shared.shareddata.site_selected = i;
					}
				}
			}
		else if (shared.shareddata.site_selected == MBV_SELECT_NONE)
			{
			XBell(view->dpy,100);
			}
		else
			{
			/* deselect previously selected site */
			shared.shareddata.site_selected = MBV_SELECT_NONE;
			}
		}
	
	/* only move selected sites if enabled */
	else if (shared.shareddata.site_mode != MBV_SITE_OFF
		&& shared.shareddata.nsite > 0
		&& (which == MBV_PICK_MOVE
			&& shared.shareddata.site_selected != MBV_SELECT_NONE))
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
			shared.shareddata.sites[shared.shareddata.site_selected].point.xgrid[instance] = xgrid;
			shared.shareddata.sites[shared.shareddata.site_selected].point.ygrid[instance] = ygrid;
			shared.shareddata.sites[shared.shareddata.site_selected].point.xlon = xlon;
			shared.shareddata.sites[shared.shareddata.site_selected].point.ylat = ylat;
			shared.shareddata.sites[shared.shareddata.site_selected].point.zdata = zdata;
			shared.shareddata.sites[shared.shareddata.site_selected].point.xdisplay[instance] = xdisplay;
			shared.shareddata.sites[shared.shareddata.site_selected].point.ydisplay[instance] = ydisplay;
			shared.shareddata.sites[shared.shareddata.site_selected].point.zdisplay[instance] = zdisplay;
		
			/* set grid and display coordinates for all instances */
			mbview_updatepointw(instance, &(shared.shareddata.sites[shared.shareddata.site_selected].point));
			}
		}

	/* else beep */
	else
		{
		if (shared.shareddata.site_selected != MBV_SELECT_NONE)
			{
			/* deselect previously selected site */
			shared.shareddata.site_selected = MBV_SELECT_NONE;
			}

		XBell(view->dpy,100);
		}
		
	/* set what kind of pick to annotate */
	if (shared.shareddata.site_selected != MBV_SELECT_NONE)
		{
		data->pickinfo_mode = MBV_PICK_SITE;
		}
	else
		{
		data->pickinfo_mode = data->pick_type;
		}
		
	/* set pick annotation */
	mbview_pick_text(instance);
	
	/* update site list */
	mbview_updatesitelist();
	
	/* print site debug statements */
	if (mbv_verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  Site data altered in function <%s>\n",
			function_name);
		fprintf(stderr,"dbg2  Site values:\n");
		fprintf(stderr,"dbg2       site_view_mode:      %d\n",data->site_view_mode);
		fprintf(stderr,"dbg2       site_mode:           %d\n",shared.shareddata.site_mode);
		fprintf(stderr,"dbg2       nsite:               %d\n",shared.shareddata.nsite);
		fprintf(stderr,"dbg2       nsite_alloc:         %d\n",shared.shareddata.nsite_alloc);
		fprintf(stderr,"dbg2       site_selected:       %d\n",shared.shareddata.site_selected);
		for (i=0;i<shared.shareddata.nsite;i++)
			{
			fprintf(stderr,"dbg2       site %d xgrid:       %f\n",i,shared.shareddata.sites[i].point.xgrid[instance]);
			fprintf(stderr,"dbg2       site %d ygrid:       %f\n",i,shared.shareddata.sites[i].point.ygrid[instance]);
			fprintf(stderr,"dbg2       site %d xlon:        %f\n",i,shared.shareddata.sites[i].point.xlon);
			fprintf(stderr,"dbg2       site %d ylat:        %f\n",i,shared.shareddata.sites[i].point.ylat);
			fprintf(stderr,"dbg2       site %d zdata:       %f\n",i,shared.shareddata.sites[i].point.zdata);
			fprintf(stderr,"dbg2       site %d xdisplay:    %f\n",i,shared.shareddata.sites[i].point.xdisplay[instance]);
			fprintf(stderr,"dbg2       site %d ydisplay:    %f\n",i,shared.shareddata.sites[i].point.ydisplay[instance]);
			fprintf(stderr,"dbg2       site %d zdisplay:    %f\n",i,shared.shareddata.sites[i].point.zdisplay[instance]);
			fprintf(stderr,"dbg2       site %d color:       %d\n",i,shared.shareddata.sites[i].color);
			fprintf(stderr,"dbg2       site %d size:        %d\n",i,shared.shareddata.sites[i].size);
			fprintf(stderr,"dbg2       site %d name:        %s\n",i,shared.shareddata.sites[i].name);
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
	if (shared.shareddata.site_mode == MBV_SITE_EDIT
		&& (which == MBV_PICK_DOWN
			|| shared.shareddata.site_selected == MBV_SELECT_NONE))
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
			if (shared.shareddata.site_selected == MBV_SELECT_NONE)
				inew = shared.shareddata.nsite;
			else 
				{
				inew = shared.shareddata.site_selected + 1;
				shared.shareddata.site_selected = MBV_SELECT_NONE;
				}
			
			/* now figure out which site will be selected next */
				
			/* allocate memory if required */
			if (shared.shareddata.nsite_alloc < shared.shareddata.nsite + 1)
				{
				shared.shareddata.nsite_alloc += MBV_ALLOC_NUM;
				status = mb_realloc(mbv_verbose, 
			    			shared.shareddata.nsite_alloc * sizeof(struct mbview_site_struct),
			    			&(shared.shareddata.sites), &error);
				if (status == MB_FAILURE)
					{
					shared.shareddata.nsite_alloc = 0;
					}
				else
					{
					for (i=shared.shareddata.nsite;i<shared.shareddata.nsite_alloc;i++)
						{
						shared.shareddata.sites[i].color = MBV_COLOR_GREEN;
						shared.shareddata.sites[i].size = 1;
						shared.shareddata.sites[i].name[0] = '\0';
						}
					}
				}
			
			/* move old sites if necessary */
			for (i=shared.shareddata.nsite;i>inew;i--)
				{
				shared.shareddata.sites[i] = shared.shareddata.sites[i-1];
				}
			
			/* add the new site */
			shared.shareddata.sites[inew].point.xgrid[instance] = xgrid;
			shared.shareddata.sites[inew].point.ygrid[instance] = ygrid;
			shared.shareddata.sites[inew].point.xlon = xlon;
			shared.shareddata.sites[inew].point.ylat = ylat;
			shared.shareddata.sites[inew].point.zdata = zdata;
			shared.shareddata.sites[inew].point.xdisplay[instance] = xdisplay;
			shared.shareddata.sites[inew].point.ydisplay[instance] = ydisplay;
			shared.shareddata.sites[inew].point.zdisplay[instance] = zdisplay;
			shared.shareddata.sites[inew].color = MBV_COLOR_GREEN;
			shared.shareddata.sites[inew].size = 1;
			sprintf(shared.shareddata.sites[inew].name,"Site %d", shared.shareddata.nsite);
		
			/* set grid and display coordinates for all instances */
			mbview_updatepointw(instance, &(shared.shareddata.sites[inew].point));
			
			/* set nsite */
			shared.shareddata.nsite++;
			
			/* select the new site */
			shared.shareddata.site_selected = inew;
			}
		else if (shared.shareddata.site_selected == MBV_SELECT_NONE)
			{
			XBell(view->dpy,100);
			}
		}
	
	/* only move selected sites if enabled */
	else if (shared.shareddata.site_mode != MBV_SITE_OFF
		&& shared.shareddata.nsite > 0
		&& (which == MBV_PICK_MOVE
			&& shared.shareddata.site_selected != MBV_SELECT_NONE))
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
			shared.shareddata.sites[shared.shareddata.site_selected].point.xgrid[instance] = xgrid;
			shared.shareddata.sites[shared.shareddata.site_selected].point.ygrid[instance] = ygrid;
			shared.shareddata.sites[shared.shareddata.site_selected].point.xlon = xlon;
			shared.shareddata.sites[shared.shareddata.site_selected].point.ylat = ylat;
			shared.shareddata.sites[shared.shareddata.site_selected].point.zdata = zdata;
			shared.shareddata.sites[shared.shareddata.site_selected].point.xdisplay[instance] = xdisplay;
			shared.shareddata.sites[shared.shareddata.site_selected].point.ydisplay[instance] = ydisplay;
			shared.shareddata.sites[shared.shareddata.site_selected].point.zdisplay[instance] = zdisplay;
		
			/* set grid and display coordinates for all instances */
			mbview_updatepointw(instance, &(shared.shareddata.sites[shared.shareddata.site_selected].point));
			}
		}

	/* else beep */
	else
		{
		if (shared.shareddata.site_selected != MBV_SELECT_NONE)
			{
			/* deselect previously selected site */
			shared.shareddata.site_selected = MBV_SELECT_NONE;
			}
		XBell(view->dpy,100);
		}
		
	/* set what kind of pick to annotate */
	if (shared.shareddata.site_selected != MBV_SELECT_NONE)
		{
		data->pickinfo_mode = MBV_PICK_SITE;
		}
	else
		{
		data->pickinfo_mode = data->pick_type;
		}
		
	/* set pick annotation */
	mbview_pick_text(instance);
	
	/* update site list */
	mbview_updatesitelist();
	
	/* print site debug statements */
	if (mbv_verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  Site data altered in function <%s>\n",
			function_name);
		fprintf(stderr,"dbg2  Site values:\n");
		fprintf(stderr,"dbg2       site_view_mode:      %d\n",data->site_view_mode);
		fprintf(stderr,"dbg2       site_mode:           %d\n",shared.shareddata.site_mode);
		fprintf(stderr,"dbg2       nsite:               %d\n",shared.shareddata.nsite);
		fprintf(stderr,"dbg2       nsite_alloc:         %d\n",shared.shareddata.nsite_alloc);
		fprintf(stderr,"dbg2       site_selected:       %d\n",shared.shareddata.site_selected);
		for (i=0;i<shared.shareddata.nsite;i++)
			{
			fprintf(stderr,"dbg2       site %d xgrid:       %f\n",i,shared.shareddata.sites[i].point.xgrid[instance]);
			fprintf(stderr,"dbg2       site %d ygrid:       %f\n",i,shared.shareddata.sites[i].point.ygrid[instance]);
			fprintf(stderr,"dbg2       site %d xlon:        %f\n",i,shared.shareddata.sites[i].point.xlon);
			fprintf(stderr,"dbg2       site %d ylat:        %f\n",i,shared.shareddata.sites[i].point.ylat);
			fprintf(stderr,"dbg2       site %d zdata:       %f\n",i,shared.shareddata.sites[i].point.zdata);
			fprintf(stderr,"dbg2       site %d xdisplay:    %f\n",i,shared.shareddata.sites[i].point.xdisplay[instance]);
			fprintf(stderr,"dbg2       site %d ydisplay:    %f\n",i,shared.shareddata.sites[i].point.ydisplay[instance]);
			fprintf(stderr,"dbg2       site %d zdisplay:    %f\n",i,shared.shareddata.sites[i].point.zdisplay[instance]);
			fprintf(stderr,"dbg2       site %d color:       %d\n",i,shared.shareddata.sites[i].color);
			fprintf(stderr,"dbg2       site %d size:        %d\n",i,shared.shareddata.sites[i].size);
			fprintf(stderr,"dbg2       site %d name:        %s\n",i,shared.shareddata.sites[i].name);
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
	int	i, isite;

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
	if (shared.shareddata.site_mode == MBV_SITE_EDIT
		&& shared.shareddata.site_selected != MBV_SELECT_NONE)
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
			isite = MBV_SELECT_NONE;
			for (i=0;i<shared.shareddata.nsite;i++)
				{
				xx = xgrid - shared.shareddata.sites[i].point.xgrid[instance];
				yy = ygrid - shared.shareddata.sites[i].point.ygrid[instance];
				rr = sqrt(xx * xx + yy * yy);
				if (rr < rrmin)
					{
					rrmin = rr;
					isite = i;
					}
				}
			}

		/* delete site if its the same as previously selected */
		if (found && shared.shareddata.site_selected == isite)
			{
			mbview_site_delete(instance, isite);
			}
		else
			{
			status = MB_FAILURE;
			}
		}

	/* else beep */
	else
		{
		status = MB_FAILURE;
		}

	/* beep for failure */
	if (status == MB_FAILURE)
		{
		XBell(view->dpy,100);
		}
		
	/* set what kind of pick to annotate */
	if (shared.shareddata.site_selected != MBV_SELECT_NONE)
		{
		data->pickinfo_mode = MBV_PICK_SITE;
		}
	else
		{
		data->pickinfo_mode = data->pick_type;
		}
		
	/* set pick annotation */
	mbview_pick_text(instance);
	
	/* update site list */
	mbview_updatesitelist();
	
	/* print site debug statements */
	if (mbv_verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  Site data altered in function <%s>\n",
			function_name);
		fprintf(stderr,"dbg2  Site values:\n");
		fprintf(stderr,"dbg2       site_view_mode:      %d\n",data->site_view_mode);
		fprintf(stderr,"dbg2       site_mode:           %d\n",shared.shareddata.site_mode);
		fprintf(stderr,"dbg2       nsite:               %d\n",shared.shareddata.nsite);
		fprintf(stderr,"dbg2       nsite_alloc:         %d\n",shared.shareddata.nsite_alloc);
		fprintf(stderr,"dbg2       site_selected:       %d\n",shared.shareddata.site_selected);
		for (i=0;i<shared.shareddata.nsite;i++)
			{
			fprintf(stderr,"dbg2       site %d xgrid:       %f\n",i,shared.shareddata.sites[i].point.xgrid[instance]);
			fprintf(stderr,"dbg2       site %d ygrid:       %f\n",i,shared.shareddata.sites[i].point.ygrid[instance]);
			fprintf(stderr,"dbg2       site %d xlon:        %f\n",i,shared.shareddata.sites[i].point.xlon);
			fprintf(stderr,"dbg2       site %d ylat:        %f\n",i,shared.shareddata.sites[i].point.ylat);
			fprintf(stderr,"dbg2       site %d zdata:       %f\n",i,shared.shareddata.sites[i].point.zdata);
			fprintf(stderr,"dbg2       site %d xdisplay:    %f\n",i,shared.shareddata.sites[i].point.xdisplay[instance]);
			fprintf(stderr,"dbg2       site %d ydisplay:    %f\n",i,shared.shareddata.sites[i].point.ydisplay[instance]);
			fprintf(stderr,"dbg2       site %d zdisplay:    %f\n",i,shared.shareddata.sites[i].point.zdisplay[instance]);
			fprintf(stderr,"dbg2       site %d color:       %d\n",i,shared.shareddata.sites[i].color);
			fprintf(stderr,"dbg2       site %d size:        %d\n",i,shared.shareddata.sites[i].size);
			fprintf(stderr,"dbg2       site %d name:        %s\n",i,shared.shareddata.sites[i].name);
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
int mbview_site_delete(int instance, int isite)
{

	/* local variables */
	char	*function_name = "mbview_site_delete";
	int	status = MB_SUCCESS;
	struct mbview_world_struct *view;
	struct mbview_struct *data;
	int	i;

	/* print starting debug statements */
	if (mbv_verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Version %s\n",rcs_id);
		fprintf(stderr,"dbg2  MB-system Version %s\n",MB_VERSION);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       isite:            %d\n",isite);
		fprintf(stderr,"dbg2       instance:         %d\n",instance);
		}
		
	/* get view */
	view = &(mbviews[instance]);
	data = &(view->data);

	/* delete site if its the same as previously selected */
	if (isite >= 0 && isite < shared.shareddata.nsite)
		{
		/* move site data if necessary */
		for (i=isite;i<shared.shareddata.nsite-1;i++)
			{
			shared.shareddata.sites[i] = shared.shareddata.sites[i+1];
			}

		/* set nsite */
		shared.shareddata.nsite--;

		/* no selection */
		shared.shareddata.site_selected = MBV_SELECT_NONE;
		}
	else
		{
		status = MB_FAILURE;
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
	if (shared.shareddata.site_mode != MBV_SITE_OFF
		&& data->display_mode == MBV_DISPLAY_2D
		&& data->site_view_mode == MBV_VIEW_ON
		&& shared.shareddata.nsite > 0)
		{
		/* set stride for looping over data */
		stride = 1;
	
		/* loop over the sites */
		for (isite=0;isite<shared.shareddata.nsite;isite++)
			{
			/* get grid bounds for plotting */
			ix = (shared.shareddata.sites[isite].point.xgrid[instance] - data->primary_xmin) / data->primary_dx;
			jy = (shared.shareddata.sites[isite].point.ygrid[instance] - data->primary_ymin) / data->primary_dy;
			if (ix >= 0 && ix < data->primary_nx && jy >= 0 && jy < data->primary_ny)
				{
				ixsize = MAX(data->viewbounds[1] - data->viewbounds[0] + 1, 
						data->viewbounds[3] - data->viewbounds[2] + 1);
				if (isite == shared.shareddata.site_selected)
					ixsize /= 100;
				else
					ixsize /= 125;
				if (ixsize < 1) ixsize = 1;
				jysize = (int)((ixsize * (1000.0 * data->primary_dy / data->primary_dx)) / 1000.0);
				if (jysize < 1) jysize = 1;
				ixmin = MAX(ix - ixsize, 0);
				ixmax = MIN(ix + ixsize, data->primary_nx - stride);
				jymin = MAX(jy - jysize, 0);
				jymax = MIN(jy + jysize, data->primary_ny - stride);

				/* set the color for this site */
				if (isite == shared.shareddata.site_selected)
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
		}
		
	/* draw sites in 3D */
	else if (shared.shareddata.site_mode != MBV_SITE_OFF
		&& data->display_mode == MBV_DISPLAY_3D
		&& data->site_view_mode == MBV_VIEW_ON
		&& shared.shareddata.nsite > 0)
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
stride = 1;
	
		/* loop over the sites */
		for (isite=0;isite<shared.shareddata.nsite;isite++)
			{
			/* get grid bounds for plotting */
			ix = (shared.shareddata.sites[isite].point.xgrid[instance] - data->primary_xmin) / data->primary_dx;
			jy = (shared.shareddata.sites[isite].point.ygrid[instance] - data->primary_ymin) / data->primary_dy;
			if (ix >= 0 && ix < data->primary_nx && jy >= 0 && jy < data->primary_ny)
				{
				ixsize = MAX(data->viewbounds[1] - data->viewbounds[0] + 1, 
						data->viewbounds[3] - data->viewbounds[2] + 1);
				if (isite == shared.shareddata.site_selected)
					ixsize /= 400;
				else
					ixsize /= 500;
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
				if (isite == shared.shareddata.site_selected)
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
int mbview_updatesitelist()
{
	/* local variables */
	char	*function_name = "mbview_updatesitelist";
	int	status = MB_SUCCESS;
   	XmString *xstr;
	int	isite;
	char	lonstr0[24];
	char	latstr0[24];
	

	/* print starting debug statements */
	if (mbv_verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Version %s\n",rcs_id);
		fprintf(stderr,"dbg2  MB-system Version %s\n",MB_VERSION);
		fprintf(stderr,"dbg2  Input arguments:\n");
		}
		
	/* update site list */
	if (shared.init_sitelist == MBV_WINDOW_VISIBLE)
		{
		/* remove all existing items */
		XmListDeleteAllItems(shared.mb3d_sitelist.mbview_list_sitelist);
		
		if (shared.shareddata.nsite > 0)
			{
			/* allocate array of label XmStrings */
			xstr = (XmString *) malloc(shared.shareddata.nsite * sizeof(XmString));

			/* loop over the sites */
			for (isite=0;isite<shared.shareddata.nsite;isite++)
				{
				/* add list item for each site */
				mbview_setlonlatstrings(shared.shareddata.sites[isite].point.xlon, 
							shared.shareddata.sites[isite].point.ylat, 
							lonstr0, latstr0);
				sprintf(value_string,"%3d | %s | %s | %.3f | %s | %d | %s", 
					isite, lonstr0, latstr0,
					shared.shareddata.sites[isite].point.zdata,
					mbview_colorname[shared.shareddata.sites[isite].color],
					shared.shareddata.sites[isite].size,
					shared.shareddata.sites[isite].name);
    				xstr[isite] = XmStringCreateLocalized(value_string);
				}

			/* add list items */
    			XmListAddItems(shared.mb3d_sitelist.mbview_list_sitelist,
					xstr, shared.shareddata.nsite,0);
					
			/* select list item for selected site */
			if (shared.shareddata.site_selected != MBV_SELECT_NONE)
				{
				XmListSelectPos(shared.mb3d_sitelist.mbview_list_sitelist,
						shared.shareddata.site_selected+1,0);
				XmListSetPos(shared.mb3d_sitelist.mbview_list_sitelist,
					MAX(shared.shareddata.site_selected+1-5, 1));
				}

			/* deallocate memory no longer needed */
			for (isite=0;isite<shared.shareddata.nsite;isite++)
				{
    				XmStringFree(xstr[isite]);
    				}
    			free(xstr);
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
