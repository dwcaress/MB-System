/*------------------------------------------------------------------------------
 *    The MB-system:	mbview_site.c	9/25/2003
 *
 *    Copyright (c) 2003-2023 by
 *    David W. Caress (caress@mbari.org)
 *      Monterey Bay Aquarium Research Institute
 *      Moss Landing, California, USA
 *    Dale N. Chayes 
 *      Center for Coastal and Ocean Mapping
 *      University of New Hampshire
 *      Durham, New Hampshire, USA
 *    Christian dos Santos Ferreira
 *      MARUM
 *      University of Bremen
 *      Bremen Germany
 *     
 *    MB-System was created by Caress and Chayes in 1992 at the
 *      Lamont-Doherty Earth Observatory
 *      Columbia University
 *      Palisades, NY 10964
 *
 *    See README.md file for copying and redistribution conditions.
 *--------------------------------------------------------------------*/
/*
 * Author:	D. W. Caress
 * Date:	September 25, 2003
 *
 * Note:	This code was broken out of mbview_callbacks.c, which was
 *		begun on October 7, 2002
 */
/*------------------------------------------------------------------------------*/

#include <ctype.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "mb_define.h"
#include "mb_status.h"

/* Need to include windows.h BEFORE the the Xm stuff otherwise VC14+ barf with conflicts */
#if defined(_MSC_VER) && (_MSC_VER >= 1800)
#	ifndef WIN32
#		define WIN32
#	endif
#	include <WinSock2.h>
#include <windows.h>
#endif

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
#include <Xm/List.h>
#include "MB3DView.h"
#include "MB3DSiteList.h"
#include "MB3DRouteList.h"
#include "MB3DNavList.h"

/* OpenGL include files */
#include <GL/gl.h>
#include <GL/glu.h>
#ifndef WIN32
#include <GL/glx.h>
#endif
#include "mb_glwdrawa.h"

#include "mbview.h"
#include "mbviewprivate.h"

/*------------------------------------------------------------------------------*/

// extern needed for Mac OSX
extern int mbv_verbose;
extern Widget parent_widget;
extern XtAppContext app_context;
extern struct mbview_world_struct mbviews[MBV_MAX_WINDOWS];
extern char *mbsystem_library_name;

static char value_string[2*MB_PATH_MAXLINE];


/*------------------------------------------------------------------------------*/
int mbview_getsitecount(int verbose, size_t instance, int *nsite, int *error)
{
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  MB-system Version %s\n", MB_VERSION);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:                   %d\n", verbose);
		fprintf(stderr, "dbg2       instance:                  %zu\n", instance);
	}

	/* get view */
	// struct mbview_world_struct *view = &(mbviews[instance]);
	// struct mbview_struct *data = &(view->data);

	/* get number of sites */
	*nsite = shared.shareddata.nsite;

	const int status = MB_SUCCESS;

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2       nsite:                     %d\n", *nsite);
		fprintf(stderr, "dbg2       error:                     %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:                    %d\n", status);
	}

	return (status);
}

/*------------------------------------------------------------------------------*/
int mbview_allocsitearrays(int verbose, int nsite, double **sitelon, double **sitelat, double **sitetopo, int **sitecolor,
                           int **sitesize, mb_path **sitename, int *error)
{
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  MB-system Version %s\n", MB_VERSION);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:                   %d\n", verbose);
		fprintf(stderr, "dbg2       nsite:                     %d\n", nsite);
		fprintf(stderr, "dbg2       sitelon:                   %p\n", *sitelon);
		fprintf(stderr, "dbg2       sitelat:                   %p\n", *sitelat);
		fprintf(stderr, "dbg2       sitetopo:                  %p\n", *sitetopo);
		fprintf(stderr, "dbg2       sitecolor:                 %p\n", *sitecolor);
		fprintf(stderr, "dbg2       sitesize:                  %p\n", *sitesize);
		fprintf(stderr, "dbg2       sitename:                  %p\n", *sitename);
	}

	/* allocate the arrays using mb_reallocd */
	int status = mb_reallocd(verbose, __FILE__, __LINE__, nsite * sizeof(double), (void **)sitelon, error);
	if (status == MB_SUCCESS)
		status = mb_reallocd(verbose, __FILE__, __LINE__, nsite * sizeof(double), (void **)sitelat, error);
	if (status == MB_SUCCESS)
		status = mb_reallocd(verbose, __FILE__, __LINE__, nsite * sizeof(double), (void **)sitetopo, error);
	if (status == MB_SUCCESS)
		status = mb_reallocd(verbose, __FILE__, __LINE__, nsite * sizeof(int), (void **)sitecolor, error);
	if (status == MB_SUCCESS)
		status = mb_reallocd(verbose, __FILE__, __LINE__, nsite * sizeof(int), (void **)sitesize, error);
	if (status == MB_SUCCESS)
		status = mb_reallocd(verbose, __FILE__, __LINE__, nsite * sizeof(mb_path), (void **)sitename, error);

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2       sitelon:                   %p\n", *sitelon);
		fprintf(stderr, "dbg2       sitelat:                   %p\n", *sitelat);
		fprintf(stderr, "dbg2       sitetopo:                  %p\n", *sitetopo);
		fprintf(stderr, "dbg2       sitecolor:                 %p\n", *sitecolor);
		fprintf(stderr, "dbg2       sitesize:                  %p\n", *sitesize);
		fprintf(stderr, "dbg2       sitename:                  %p\n", *sitename);
		fprintf(stderr, "dbg2       error:                     %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:                    %d\n", status);
	}

	return (status);
}

/*------------------------------------------------------------------------------*/
int mbview_freesitearrays(int verbose, double **sitelon, double **sitelat, double **sitetopo, int **sitecolor, int **sitesize,
                          mb_path **sitename, int *error)
{
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  MB-system Version %s\n", MB_VERSION);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:                   %d\n", verbose);
		fprintf(stderr, "dbg2       sitelon:                   %p\n", *sitelon);
		fprintf(stderr, "dbg2       sitelat:                   %p\n", *sitelat);
		fprintf(stderr, "dbg2       sitetopo:                  %p\n", *sitetopo);
		fprintf(stderr, "dbg2       sitecolor:                 %p\n", *sitecolor);
		fprintf(stderr, "dbg2       sitesize:                  %p\n", *sitesize);
		fprintf(stderr, "dbg2       sitename:                  %p\n", *sitename);
	}

	/* free the arrays using mb_freed */
	int status = mb_freed(verbose, __FILE__, __LINE__, (void **)sitelon, error);
	status = mb_freed(verbose, __FILE__, __LINE__, (void **)sitelat, error);
	status = mb_freed(verbose, __FILE__, __LINE__, (void **)sitetopo, error);
	status = mb_freed(verbose, __FILE__, __LINE__, (void **)sitecolor, error);
	status = mb_freed(verbose, __FILE__, __LINE__, (void **)sitesize, error);
	status = mb_freed(verbose, __FILE__, __LINE__, (void **)sitename, error);

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2       sitelon:                   %p\n", *sitelon);
		fprintf(stderr, "dbg2       sitelat:                   %p\n", *sitelat);
		fprintf(stderr, "dbg2       sitetopo:                  %p\n", *sitetopo);
		fprintf(stderr, "dbg2       sitecolor:                 %p\n", *sitecolor);
		fprintf(stderr, "dbg2       sitesize:                  %p\n", *sitesize);
		fprintf(stderr, "dbg2       sitename:                  %p\n", *sitename);
		fprintf(stderr, "dbg2       error:                     %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:                    %d\n", status);
	}

	return (status);
}

/*------------------------------------------------------------------------------*/
int mbview_addsites(int verbose, size_t instance, int nsite, double *sitelon, double *sitelat, double *sitetopo, int *sitecolor,
                    int *sitesize, mb_path *sitename, int *error)
{
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  MB-system Version %s\n", MB_VERSION);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:                   %d\n", verbose);
		fprintf(stderr, "dbg2       instance:                  %zu\n", instance);
		fprintf(stderr, "dbg2       nsite:                     %d\n", nsite);
		fprintf(stderr, "dbg2       sitelon:                   %p\n", sitelon);
		fprintf(stderr, "dbg2       sitelat:                   %p\n", sitelat);
		fprintf(stderr, "dbg2       sitetopo:                  %p\n", sitetopo);
		fprintf(stderr, "dbg2       sitecolor:                 %p\n", sitecolor);
		fprintf(stderr, "dbg2       sitesize:                  %p\n", sitesize);
		fprintf(stderr, "dbg2       sitename:                  %p\n", sitename);
		for (int i = 0; i < nsite; i++) {
			fprintf(stderr, "dbg2       site:%d lon:%f lat:%f topo:%f color:%d size:%d name:%s\n", i, sitelon[i], sitelat[i],
			        sitetopo[i], sitecolor[i], sitesize[i], sitename[i]);
		}
	}

	/* get view */
	struct mbview_world_struct *view = &(mbviews[instance]);
	struct mbview_struct *data = &(view->data);

	/* make sure no site is selected */
	shared.shareddata.site_selected = MBV_SELECT_NONE;

	int status = MB_SUCCESS;

	/* allocate memory if required */
	if (shared.shareddata.nsite_alloc < shared.shareddata.nsite + nsite) {
		fprintf(stderr, "Have %d sites allocated but need %d + %d = %d\n", shared.shareddata.nsite_alloc, shared.shareddata.nsite,
		        nsite, shared.shareddata.nsite + nsite);
		shared.shareddata.nsite_alloc = shared.shareddata.nsite + nsite;
		status = mb_reallocd(mbv_verbose, __FILE__, __LINE__, shared.shareddata.nsite_alloc * sizeof(struct mbview_site_struct),
		                     (void **)&(shared.shareddata.sites), error);
		if (status == MB_FAILURE) {
			shared.shareddata.nsite_alloc = 0;
		}
		else {
			for (int i = shared.shareddata.nsite; i < shared.shareddata.nsite_alloc; i++) {
				shared.shareddata.sites[i].active = false;
				shared.shareddata.sites[i].color = MBV_COLOR_GREEN;
				shared.shareddata.sites[i].size = 1;
				shared.shareddata.sites[i].name[0] = '\0';
			}
		}
	}

	/* loop over the sites */
	int nadded = 0;
	for (int i = 0; i < nsite; i++) {
		/* get site positions in grid coordinates */
		double xgrid, ygrid, zdata;
		status = mbview_projectll2xyzgrid(instance, sitelon[i], sitelat[i], &xgrid, &ygrid, &zdata);

		/* use provided topo */
		if (sitetopo[i] != MBV_DEFAULT_NODATA) {
			zdata = sitetopo[i];
		}

		/* get site positions in display coordinates */
		double xdisplay, ydisplay, zdisplay;
		status = mbview_projectll2display(instance, sitelon[i], sitelat[i], zdata, &xdisplay, &ydisplay, &zdisplay);

		/* check for reasonable coordinates */
		if (fabs(xdisplay) < 1000.0 && fabs(ydisplay) < 1000.0 && fabs(zdisplay) < 1000.0) {

			/* add the new site */
			shared.shareddata.sites[shared.shareddata.nsite].active = true;
			shared.shareddata.sites[shared.shareddata.nsite].color = sitecolor[i];
			shared.shareddata.sites[shared.shareddata.nsite].size = sitesize[i];
			strcpy(shared.shareddata.sites[shared.shareddata.nsite].name, sitename[i]);
			shared.shareddata.sites[shared.shareddata.nsite].point.xgrid[instance] = xgrid;
			shared.shareddata.sites[shared.shareddata.nsite].point.ygrid[instance] = ygrid;
			shared.shareddata.sites[shared.shareddata.nsite].point.xlon = sitelon[i];
			shared.shareddata.sites[shared.shareddata.nsite].point.ylat = sitelat[i];
			shared.shareddata.sites[shared.shareddata.nsite].point.zdata = zdata;
			shared.shareddata.sites[shared.shareddata.nsite].point.xdisplay[instance] = xdisplay;
			shared.shareddata.sites[shared.shareddata.nsite].point.ydisplay[instance] = ydisplay;
			shared.shareddata.sites[shared.shareddata.nsite].point.zdisplay[instance] = zdisplay;

			/* set grid and display coordinates for all instances */
			mbview_updatepointw(instance, &(shared.shareddata.sites[shared.shareddata.nsite].point));

			/* set nsite */
			shared.shareddata.nsite++;
			nadded++;
			fprintf(stderr,"Added site %d added so far:%d total:%d\n",
			shared.shareddata.nsite-1, nadded, shared.shareddata.nsite);
		}

		/* report failure due to unreasonable coordinates */
		else {
			fprintf(stderr,
			        "Failed to add site at position lon:%f lat:%f due to display coordinate projection (%f %f %f) far outside "
			        "view...\n",
			        sitelon[i], sitelat[i], xdisplay, ydisplay, zdisplay);
			XBell(view->dpy, 100);
		}
	}

	/* make sites viewable */
	if (nadded > 0) {
		data->site_view_mode = MBV_VIEW_ON;
	}

	/* update site list */
	mbview_updatesitelist();

	/* print site debug statements */
	if (mbv_verbose >= 2) {
		fprintf(stderr, "\ndbg2  Site data in function <%s>\n", __func__);
		fprintf(stderr, "dbg2  Site values:\n");
		fprintf(stderr, "dbg2       site_view_mode:      %d\n", data->site_view_mode);
		fprintf(stderr, "dbg2       site_mode:           %d\n", shared.shareddata.site_mode);
		fprintf(stderr, "dbg2       nsite:               %d\n", shared.shareddata.nsite);
		fprintf(stderr, "dbg2       nsite_alloc:         %d\n", shared.shareddata.nsite_alloc);
		fprintf(stderr, "dbg2       site_selected:       %d\n", shared.shareddata.site_selected);
		for (int i = 0; i < shared.shareddata.nsite; i++) {
			fprintf(stderr, "dbg2       site %d active:      %d\n", i, shared.shareddata.sites[i].active);
			fprintf(stderr, "dbg2       site %d color:       %d\n", i, shared.shareddata.sites[i].color);
			fprintf(stderr, "dbg2       site %d size:        %d\n", i, shared.shareddata.sites[i].size);
			fprintf(stderr, "dbg2       site %d name:        %s\n", i, shared.shareddata.sites[i].name);
			fprintf(stderr, "dbg2       site %d xgrid:       %f\n", i, shared.shareddata.sites[i].point.xgrid[instance]);
			fprintf(stderr, "dbg2       site %d ygrid:       %f\n", i, shared.shareddata.sites[i].point.ygrid[instance]);
			fprintf(stderr, "dbg2       site %d xlon:        %f\n", i, shared.shareddata.sites[i].point.xlon);
			fprintf(stderr, "dbg2       site %d ylat:        %f\n", i, shared.shareddata.sites[i].point.ylat);
			fprintf(stderr, "dbg2       site %d zdata:       %f\n", i, shared.shareddata.sites[i].point.zdata);
			fprintf(stderr, "dbg2       site %d xdisplay:    %f\n", i, shared.shareddata.sites[i].point.xdisplay[instance]);
			fprintf(stderr, "dbg2       site %d ydisplay:    %f\n", i, shared.shareddata.sites[i].point.ydisplay[instance]);
			fprintf(stderr, "dbg2       site %d zdisplay:    %f\n", i, shared.shareddata.sites[i].point.zdisplay[instance]);
		}
	}

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2       error:                     %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:                    %d\n", status);
	}

	return (status);
}
/*------------------------------------------------------------------------------*/
int mbview_getsites(int verbose, size_t instance, int *nsite, double *sitelon, double *sitelat, double *sitetopo, int *sitecolor,
                    int *sitesize, mb_path *sitename, int *error)
{
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  MB-system Version %s\n", MB_VERSION);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:                   %d\n", verbose);
		fprintf(stderr, "dbg2       instance:                  %zu\n", instance);
		fprintf(stderr, "dbg2       nsite:                     %p\n", nsite);
		fprintf(stderr, "dbg2       sitelon:                   %p\n", sitelon);
		fprintf(stderr, "dbg2       sitelat:                   %p\n", sitelat);
		fprintf(stderr, "dbg2       sitetopo:                  %p\n", sitetopo);
		fprintf(stderr, "dbg2       sitecolor:                 %p\n", sitecolor);
		fprintf(stderr, "dbg2       sitesize:                  %p\n", sitesize);
		fprintf(stderr, "dbg2       sitename:                  %p\n", sitename);
	}

	/* get view */
	struct mbview_world_struct *view = &(mbviews[instance]);
	struct mbview_struct *data = &(view->data);

	/* print site debug statements */
	if (mbv_verbose >= 2) {
		fprintf(stderr, "\ndbg2  Site data in function <%s>\n", __func__);
		fprintf(stderr, "dbg2  Site values:\n");
		fprintf(stderr, "dbg2       site_view_mode:      %d\n", data->site_view_mode);
		fprintf(stderr, "dbg2       site_mode:           %d\n", shared.shareddata.site_mode);
		fprintf(stderr, "dbg2       nsite:               %d\n", shared.shareddata.nsite);
		fprintf(stderr, "dbg2       nsite_alloc:         %d\n", shared.shareddata.nsite_alloc);
		fprintf(stderr, "dbg2       site_selected:       %d\n", shared.shareddata.site_selected);
		for (int i = 0; i < shared.shareddata.nsite; i++) {
			fprintf(stderr, "dbg2       site %d active:      %d\n", i, shared.shareddata.sites[i].active);
			fprintf(stderr, "dbg2       site %d color:       %d\n", i, shared.shareddata.sites[i].color);
			fprintf(stderr, "dbg2       site %d size:        %d\n", i, shared.shareddata.sites[i].size);
			fprintf(stderr, "dbg2       site %d name:        %s\n", i, shared.shareddata.sites[i].name);
			fprintf(stderr, "dbg2       site %d xgrid:       %f\n", i, shared.shareddata.sites[i].point.xgrid[instance]);
			fprintf(stderr, "dbg2       site %d ygrid:       %f\n", i, shared.shareddata.sites[i].point.ygrid[instance]);
			fprintf(stderr, "dbg2       site %d xlon:        %f\n", i, shared.shareddata.sites[i].point.xlon);
			fprintf(stderr, "dbg2       site %d ylat:        %f\n", i, shared.shareddata.sites[i].point.ylat);
			fprintf(stderr, "dbg2       site %d zdata:       %f\n", i, shared.shareddata.sites[i].point.zdata);
			fprintf(stderr, "dbg2       site %d xdisplay:    %f\n", i, shared.shareddata.sites[i].point.xdisplay[instance]);
			fprintf(stderr, "dbg2       site %d ydisplay:    %f\n", i, shared.shareddata.sites[i].point.ydisplay[instance]);
			fprintf(stderr, "dbg2       site %d zdisplay:    %f\n", i, shared.shareddata.sites[i].point.zdisplay[instance]);
		}
	}

	/* check that the array pointers are not NULL */
	int status = MB_SUCCESS;
	if (sitelon == NULL || sitelat == NULL || sitetopo == NULL || sitecolor == NULL || sitesize == NULL || sitename == NULL) {
		status = MB_FAILURE;
		*error = MB_ERROR_DATA_NOT_INSERTED;
	}

	/* otherwise go get the site data */
	else {
		/* loop over the sites */
    int j = 0;
		for (int i = 0; i < shared.shareddata.nsite; i++) {
      if (shared.shareddata.sites[i].active) {
  			sitelon[j] = shared.shareddata.sites[i].point.xlon;
  			sitelat[j] = shared.shareddata.sites[i].point.ylat;
  			sitetopo[j] = shared.shareddata.sites[i].point.zdata;
  			sitecolor[j] = shared.shareddata.sites[i].color;
  			sitesize[j] = shared.shareddata.sites[i].size;
  			strcpy(sitename[j], shared.shareddata.sites[i].name);
		    j += 1;
      }
      *nsite = j;
		}
	}

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2       nsite:                     %d\n", *nsite);
		for (int i = 0; i < *nsite; i++) {
			fprintf(stderr, "dbg2       site:%d lon:%f lat:%f topo:%f color:%d size:%d name:%s\n", i, sitelon[i], sitelat[i],
			        sitetopo[i], sitecolor[i], sitesize[i], sitename[i]);
		}
		fprintf(stderr, "dbg2       error:                     %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:                    %d\n", status);
	}

	return (status);
}

/*--------------------------------------------------------------------*/
int mbview_enableviewsites(int verbose, size_t instance, int *error)
{
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  MB-system Version %s\n", MB_VERSION);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:                   %d\n", verbose);
		fprintf(stderr, "dbg2       instance:                  %zu\n", instance);
	}

	/* set values */
	shared.shareddata.site_mode = MBV_SITE_VIEW;

	/* set widget sensitivity on all active instances */
	for (instance = 0; instance < MBV_MAX_WINDOWS; instance++) {
		/* get view */
		struct mbview_world_struct *view = &(mbviews[instance]);
		struct mbview_struct *data = &(view->data);

		/* if instance active reset action sensitivity */
		if (data->active)
			mbview_update_sensitivity(verbose, instance, error);
	}

	const int status = MB_SUCCESS;

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2       error:                     %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:                    %d\n", status);
	}

	return (status);
}

/*--------------------------------------------------------------------*/
int mbview_enableeditsites(int verbose, size_t instance, int *error)
{
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  MB-system Version %s\n", MB_VERSION);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:                   %d\n", verbose);
		fprintf(stderr, "dbg2       instance:                  %zu\n", instance);
	}

	/* get view */
	struct mbview_world_struct *view = &(mbviews[instance]);
	struct mbview_struct *data = &(view->data);

	/* set values */
	shared.shareddata.site_mode = MBV_SITE_EDIT;

	/* set widget sensitivity */
	if (data->active)
		mbview_update_sensitivity(mbv_verbose, instance, error);

	const int status = MB_SUCCESS;

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2       error:                     %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:                    %d\n", status);
	}

	return (status);
}

/*------------------------------------------------------------------------------*/
int mbview_pick_site_select(size_t instance, int which, int xpixel, int ypixel) {
	if (mbv_verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  MB-system Version %s\n", MB_VERSION);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       instance:         %zu\n", instance);
		fprintf(stderr, "dbg2       which:            %d\n", which);
		fprintf(stderr, "dbg2       xpixel:           %d\n", xpixel);
		fprintf(stderr, "dbg2       ypixel:           %d\n", ypixel);
	}

	/* get view */
	struct mbview_world_struct *view = &(mbviews[instance]);
	struct mbview_struct *data = &(view->data);

	/* only select sites if enabled and not in move mode */
	if (shared.shareddata.site_mode != MBV_SITE_OFF && shared.shareddata.nsite > 0 &&
	    (which == MBV_PICK_DOWN || shared.shareddata.site_selected == MBV_SELECT_NONE)) {
		/* look for point */
		bool found;
		double xgrid, ygrid;
		double xlon, ylat, zdata;
		double xdisplay, ydisplay, zdisplay;
		mbview_findpoint(instance, xpixel, ypixel, &found, &xgrid, &ygrid, &xlon, &ylat, &zdata, &xdisplay, &ydisplay, &zdisplay);

		/* look for nearest site */
		if (found) {
			/* first deselect previously selected site */
			shared.shareddata.site_selected = MBV_SELECT_NONE;

			/* now figure out which site will be selected next */
			double rrmin = 1000000000.0;
			for (int i = 0; i < shared.shareddata.nsite; i++) {
        if (shared.shareddata.sites[i].active) {
  				const double xx = xgrid - shared.shareddata.sites[i].point.xgrid[instance];
  				const double yy = ygrid - shared.shareddata.sites[i].point.ygrid[instance];
  				const double rr = sqrt(xx * xx + yy * yy);
  				if (rr < rrmin) {
  					rrmin = rr;
  					shared.shareddata.site_selected = i;
  				}
        }
			}
		}
		else if (shared.shareddata.site_selected == MBV_SELECT_NONE) {
			XBell(view->dpy, 100);
		}
		else {
			/* deselect previously selected site */
			shared.shareddata.site_selected = MBV_SELECT_NONE;
		}
	}

	/* only move selected sites if enabled */
	else if (shared.shareddata.site_mode != MBV_SITE_OFF && shared.shareddata.nsite > 0 &&
	         (which == MBV_PICK_MOVE && shared.shareddata.site_selected != MBV_SELECT_NONE)) {
		/* look for point */
		bool found;
		double xgrid, ygrid;
		double xlon, ylat, zdata;
		double xdisplay, ydisplay, zdisplay;
		mbview_findpoint(instance, xpixel, ypixel, &found, &xgrid, &ygrid, &xlon, &ylat, &zdata, &xdisplay, &ydisplay, &zdisplay);

		/* reset selected site position */
		if (found) {
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
	else {
		if (shared.shareddata.site_selected != MBV_SELECT_NONE) {
			/* deselect previously selected site */
			shared.shareddata.site_selected = MBV_SELECT_NONE;
		}

		XBell(view->dpy, 100);
	}

	/* set what kind of pick to annotate */
	if (shared.shareddata.site_selected != MBV_SELECT_NONE) {
		data->pickinfo_mode = MBV_PICK_SITE;
	}
	else {
		data->pickinfo_mode = data->pick_type;
	}

	/* set pick annotation */
	mbview_pick_text(instance);

	/* update site list */
	mbview_updatesitelist();

	/* call pick notify if defined */
	if (which == MBV_PICK_UP && shared.shareddata.site_selected != MBV_SELECT_NONE && data->mbview_picksite_notify != NULL) {
		(data->mbview_picksite_notify)(instance);
	}

	/* print site debug statements */
	if (mbv_verbose >= 2) {
		fprintf(stderr, "\ndbg2  Site data in function <%s>\n", __func__);
		fprintf(stderr, "dbg2  Site values:\n");
		fprintf(stderr, "dbg2       site_view_mode:      %d\n", data->site_view_mode);
		fprintf(stderr, "dbg2       site_mode:           %d\n", shared.shareddata.site_mode);
		fprintf(stderr, "dbg2       nsite:               %d\n", shared.shareddata.nsite);
		fprintf(stderr, "dbg2       nsite_alloc:         %d\n", shared.shareddata.nsite_alloc);
		fprintf(stderr, "dbg2       site_selected:       %d\n", shared.shareddata.site_selected);
		for (int i = 0; i < shared.shareddata.nsite; i++) {
			fprintf(stderr, "dbg2       site %d active:      %d\n", i, shared.shareddata.sites[i].active);
			fprintf(stderr, "dbg2       site %d color:       %d\n", i, shared.shareddata.sites[i].color);
			fprintf(stderr, "dbg2       site %d size:        %d\n", i, shared.shareddata.sites[i].size);
			fprintf(stderr, "dbg2       site %d name:        %s\n", i, shared.shareddata.sites[i].name);
			fprintf(stderr, "dbg2       site %d xgrid:       %f\n", i, shared.shareddata.sites[i].point.xgrid[instance]);
			fprintf(stderr, "dbg2       site %d ygrid:       %f\n", i, shared.shareddata.sites[i].point.ygrid[instance]);
			fprintf(stderr, "dbg2       site %d xlon:        %f\n", i, shared.shareddata.sites[i].point.xlon);
			fprintf(stderr, "dbg2       site %d ylat:        %f\n", i, shared.shareddata.sites[i].point.ylat);
			fprintf(stderr, "dbg2       site %d zdata:       %f\n", i, shared.shareddata.sites[i].point.zdata);
			fprintf(stderr, "dbg2       site %d xdisplay:    %f\n", i, shared.shareddata.sites[i].point.xdisplay[instance]);
			fprintf(stderr, "dbg2       site %d ydisplay:    %f\n", i, shared.shareddata.sites[i].point.ydisplay[instance]);
			fprintf(stderr, "dbg2       site %d zdisplay:    %f\n", i, shared.shareddata.sites[i].point.zdisplay[instance]);
		}
	}

	const int status = MB_SUCCESS;

	if (mbv_verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:          %d\n", status);
	}

	return (status);
}

/*------------------------------------------------------------------------------*/
int mbview_pick_site_add(size_t instance, int which, int xpixel, int ypixel) {
	if (mbv_verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  MB-system Version %s\n", MB_VERSION);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       instance:         %zu\n", instance);
		fprintf(stderr, "dbg2       which:            %d\n", which);
		fprintf(stderr, "dbg2       xpixel:           %d\n", xpixel);
		fprintf(stderr, "dbg2       ypixel:           %d\n", ypixel);
	}

	/* get view */
	struct mbview_world_struct *view = &(mbviews[instance]);
	struct mbview_struct *data = &(view->data);

	int status = MB_SUCCESS;

	int error = MB_ERROR_NO_ERROR;

	/* only add sites if enabled and not in move mode */
	if (shared.shareddata.site_mode == MBV_SITE_EDIT &&
	    (which == MBV_PICK_DOWN || shared.shareddata.site_selected == MBV_SELECT_NONE)) {
		/* look for point */
		bool found;
		double xgrid, ygrid;
		double xlon, ylat, zdata;
		double xdisplay, ydisplay, zdisplay;
		mbview_findpoint(instance, xpixel, ypixel, &found, &xgrid, &ygrid, &xlon, &ylat, &zdata, &xdisplay, &ydisplay, &zdisplay);

		/* add site */
		if (found) {
			/* add site after currently selected site if any */
			int inew;
			if (shared.shareddata.site_selected == MBV_SELECT_NONE)
				inew = shared.shareddata.nsite;
			else {
				inew = shared.shareddata.site_selected + 1;
				shared.shareddata.site_selected = MBV_SELECT_NONE;
			}

			/* now figure out which site will be selected next */

			/* allocate memory if required */
			if (shared.shareddata.nsite_alloc < shared.shareddata.nsite + 1) {
				shared.shareddata.nsite_alloc += MBV_ALLOC_NUM;
				status = mb_reallocd(mbv_verbose, __FILE__, __LINE__,
				                     shared.shareddata.nsite_alloc * sizeof(struct mbview_site_struct),
				                     (void **)&(shared.shareddata.sites), &error);
				if (status == MB_FAILURE) {
					shared.shareddata.nsite_alloc = 0;
				}
				else {
					for (int i = shared.shareddata.nsite; i < shared.shareddata.nsite_alloc; i++) {
						shared.shareddata.sites[i].active = false;
						shared.shareddata.sites[i].color = MBV_COLOR_GREEN;
						shared.shareddata.sites[i].size = 1;
						shared.shareddata.sites[i].name[0] = '\0';
					}
				}
			}

			/* move old sites if necessary */
			for (int i = shared.shareddata.nsite; i > inew; i--) {
				shared.shareddata.sites[i] = shared.shareddata.sites[i - 1];
			}

			/* add the new site */
			shared.shareddata.sites[inew].active = true;
			shared.shareddata.sites[inew].color = MBV_COLOR_GREEN;
			shared.shareddata.sites[inew].size = 1;
			sprintf(shared.shareddata.sites[inew].name, "Site %d", shared.shareddata.nsite);
			shared.shareddata.sites[inew].point.xgrid[instance] = xgrid;
			shared.shareddata.sites[inew].point.ygrid[instance] = ygrid;
			shared.shareddata.sites[inew].point.xlon = xlon;
			shared.shareddata.sites[inew].point.ylat = ylat;
			shared.shareddata.sites[inew].point.zdata = zdata;
			shared.shareddata.sites[inew].point.xdisplay[instance] = xdisplay;
			shared.shareddata.sites[inew].point.ydisplay[instance] = ydisplay;
			shared.shareddata.sites[inew].point.zdisplay[instance] = zdisplay;

			/* set grid and display coordinates for all instances */
			mbview_updatepointw(instance, &(shared.shareddata.sites[inew].point));

			/* set nsite */
			shared.shareddata.nsite++;

			/* select the new site */
			shared.shareddata.site_selected = inew;
		}
		else if (shared.shareddata.site_selected == MBV_SELECT_NONE) {
			XBell(view->dpy, 100);
		}
	}

	/* only move selected sites if enabled */
	else if (shared.shareddata.site_mode != MBV_SITE_OFF && shared.shareddata.nsite > 0 &&
	         (which == MBV_PICK_MOVE && shared.shareddata.site_selected != MBV_SELECT_NONE)) {
		/* look for point */
		bool found;
		double xgrid, ygrid;
		double xlon, ylat, zdata;
		double xdisplay, ydisplay, zdisplay;
		mbview_findpoint(instance, xpixel, ypixel, &found, &xgrid, &ygrid, &xlon, &ylat, &zdata, &xdisplay, &ydisplay, &zdisplay);

		/* reset selected site position */
		if (found) {
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
	else {
		if (shared.shareddata.site_selected != MBV_SELECT_NONE) {
			/* deselect previously selected site */
			shared.shareddata.site_selected = MBV_SELECT_NONE;
		}
		XBell(view->dpy, 100);
	}

	/* set what kind of pick to annotate */
	if (shared.shareddata.site_selected != MBV_SELECT_NONE) {
		data->pickinfo_mode = MBV_PICK_SITE;
	}
	else {
		data->pickinfo_mode = data->pick_type;
	}

	/* set pick annotation */
	mbview_pick_text(instance);

	/* update site list */
	mbview_updatesitelist();

	/* print site debug statements */
	if (mbv_verbose >= 2) {
		fprintf(stderr, "\ndbg2  Site data in function <%s>\n", __func__);
		fprintf(stderr, "dbg2  Site values:\n");
		fprintf(stderr, "dbg2       site_view_mode:      %d\n", data->site_view_mode);
		fprintf(stderr, "dbg2       site_mode:           %d\n", shared.shareddata.site_mode);
		fprintf(stderr, "dbg2       nsite:               %d\n", shared.shareddata.nsite);
		fprintf(stderr, "dbg2       nsite_alloc:         %d\n", shared.shareddata.nsite_alloc);
		fprintf(stderr, "dbg2       site_selected:       %d\n", shared.shareddata.site_selected);
		for (int i = 0; i < shared.shareddata.nsite; i++) {
			fprintf(stderr, "dbg2       site %d active:      %d\n", i, shared.shareddata.sites[i].active);
			fprintf(stderr, "dbg2       site %d color:       %d\n", i, shared.shareddata.sites[i].color);
			fprintf(stderr, "dbg2       site %d size:        %d\n", i, shared.shareddata.sites[i].size);
			fprintf(stderr, "dbg2       site %d name:        %s\n", i, shared.shareddata.sites[i].name);
			fprintf(stderr, "dbg2       site %d xgrid:       %f\n", i, shared.shareddata.sites[i].point.xgrid[instance]);
			fprintf(stderr, "dbg2       site %d ygrid:       %f\n", i, shared.shareddata.sites[i].point.ygrid[instance]);
			fprintf(stderr, "dbg2       site %d xlon:        %f\n", i, shared.shareddata.sites[i].point.xlon);
			fprintf(stderr, "dbg2       site %d ylat:        %f\n", i, shared.shareddata.sites[i].point.ylat);
			fprintf(stderr, "dbg2       site %d zdata:       %f\n", i, shared.shareddata.sites[i].point.zdata);
			fprintf(stderr, "dbg2       site %d xdisplay:    %f\n", i, shared.shareddata.sites[i].point.xdisplay[instance]);
			fprintf(stderr, "dbg2       site %d ydisplay:    %f\n", i, shared.shareddata.sites[i].point.ydisplay[instance]);
			fprintf(stderr, "dbg2       site %d zdisplay:    %f\n", i, shared.shareddata.sites[i].point.zdisplay[instance]);
		}
	}

	if (mbv_verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:          %d\n", status);
	}

	return (status);
}

/*------------------------------------------------------------------------------*/
int mbview_pick_site_delete(size_t instance, int xpixel, int ypixel) {
	if (mbv_verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  MB-system Version %s\n", MB_VERSION);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       instance:         %zu\n", instance);
		fprintf(stderr, "dbg2       xpixel:           %d\n", xpixel);
		fprintf(stderr, "dbg2       ypixel:           %d\n", ypixel);
	}

	/* get view */
	struct mbview_world_struct *view = &(mbviews[instance]);
	struct mbview_struct *data = &(view->data);

	int status = MB_SUCCESS;

	/* only delete a selected site if enabled */
	if (shared.shareddata.site_mode == MBV_SITE_EDIT && shared.shareddata.site_selected != MBV_SELECT_NONE) {
		/* look for point */
		bool found;
		double xgrid, ygrid;
		double xlon, ylat, zdata;
		double xdisplay, ydisplay, zdisplay;
		mbview_findpoint(instance, xpixel, ypixel, &found, &xgrid, &ygrid, &xlon, &ylat, &zdata, &xdisplay, &ydisplay, &zdisplay);

		/* find closest site to pick point */
		int isite;
		if (found) {
			double rrmin = 1000000000.0;
			isite = MBV_SELECT_NONE;
			for (int i = 0; i < shared.shareddata.nsite; i++) {
				const double xx = xgrid - shared.shareddata.sites[i].point.xgrid[instance];
				const double yy = ygrid - shared.shareddata.sites[i].point.ygrid[instance];
				const double rr = sqrt(xx * xx + yy * yy);
				if (rr < rrmin) {
					rrmin = rr;
					isite = i;
				}
			}
		}

		/* delete site if its the same as previously selected */
		if (found && shared.shareddata.site_selected == isite) {
			mbview_site_delete(instance, isite);
		}
		else {
			status = MB_FAILURE;
		}
	}

	/* else beep */
	else {
		status = MB_FAILURE;
	}

	/* beep for failure */
	if (status == MB_FAILURE) {
		XBell(view->dpy, 100);
	}

	/* set what kind of pick to annotate */
	if (shared.shareddata.site_selected != MBV_SELECT_NONE) {
		data->pickinfo_mode = MBV_PICK_SITE;
	}
	else {
		data->pickinfo_mode = data->pick_type;
	}

	/* set pick annotation */
	mbview_pick_text(instance);

	/* update site list */
	mbview_updatesitelist();

	/* print site debug statements */
	if (mbv_verbose >= 2) {
		fprintf(stderr, "\ndbg2  Site data in function <%s>\n", __func__);
		fprintf(stderr, "dbg2  Site values:\n");
		fprintf(stderr, "dbg2       site_view_mode:      %d\n", data->site_view_mode);
		fprintf(stderr, "dbg2       site_mode:           %d\n", shared.shareddata.site_mode);
		fprintf(stderr, "dbg2       nsite:               %d\n", shared.shareddata.nsite);
		fprintf(stderr, "dbg2       nsite_alloc:         %d\n", shared.shareddata.nsite_alloc);
		fprintf(stderr, "dbg2       site_selected:       %d\n", shared.shareddata.site_selected);
		for (int i = 0; i < shared.shareddata.nsite; i++) {
			fprintf(stderr, "dbg2       site %d active:      %d\n", i, shared.shareddata.sites[i].active);
			fprintf(stderr, "dbg2       site %d color:       %d\n", i, shared.shareddata.sites[i].color);
			fprintf(stderr, "dbg2       site %d size:        %d\n", i, shared.shareddata.sites[i].size);
			fprintf(stderr, "dbg2       site %d name:        %s\n", i, shared.shareddata.sites[i].name);
			fprintf(stderr, "dbg2       site %d xgrid:       %f\n", i, shared.shareddata.sites[i].point.xgrid[instance]);
			fprintf(stderr, "dbg2       site %d ygrid:       %f\n", i, shared.shareddata.sites[i].point.ygrid[instance]);
			fprintf(stderr, "dbg2       site %d xlon:        %f\n", i, shared.shareddata.sites[i].point.xlon);
			fprintf(stderr, "dbg2       site %d ylat:        %f\n", i, shared.shareddata.sites[i].point.ylat);
			fprintf(stderr, "dbg2       site %d zdata:       %f\n", i, shared.shareddata.sites[i].point.zdata);
			fprintf(stderr, "dbg2       site %d xdisplay:    %f\n", i, shared.shareddata.sites[i].point.xdisplay[instance]);
			fprintf(stderr, "dbg2       site %d ydisplay:    %f\n", i, shared.shareddata.sites[i].point.ydisplay[instance]);
			fprintf(stderr, "dbg2       site %d zdisplay:    %f\n", i, shared.shareddata.sites[i].point.zdisplay[instance]);
		}
	}

	if (mbv_verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:          %d\n", status);
	}

	return (status);
}

/*------------------------------------------------------------------------------*/
int mbview_site_delete(size_t instance, int isite) {
	if (mbv_verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  MB-system Version %s\n", MB_VERSION);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       isite:            %d\n", isite);
		fprintf(stderr, "dbg2       instance:         %zu\n", instance);
	}

	/* get view */
	// struct mbview_world_struct *view = &(mbviews[instance]);
	// struct mbview_struct *data = &(view->data);

	/* delete site if its the same as previously selected */
	int status = MB_SUCCESS;
	if (isite >= 0 && isite < shared.shareddata.nsite) {
		/* move site data if necessary */
		for (int i = isite; i < shared.shareddata.nsite - 1; i++) {
			shared.shareddata.sites[i] = shared.shareddata.sites[i + 1];
		}

		/* set nsite */
		shared.shareddata.nsite--;

		/* no selection */
		shared.shareddata.site_selected = MBV_SELECT_NONE;
	}
	else {
		status = MB_FAILURE;
	}

	if (mbv_verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:          %d\n", status);
	}

	return (status);
}

/*------------------------------------------------------------------------------*/
int mbview_drawsite(size_t instance, int rez) {
	if (mbv_verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  MB-system Version %s\n", MB_VERSION);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       instance:         %zu\n", instance);
		fprintf(stderr, "dbg2       rez:              %d\n", rez);
	}

	/* get view */
	struct mbview_world_struct *view = &(mbviews[instance]);
	struct mbview_struct *data = &(view->data);

	/* Generate GL lists to be plotted */
	if (shared.shareddata.site_mode != MBV_SITE_OFF && data->site_view_mode == MBV_VIEW_ON && shared.shareddata.nsite > 0) {
		/* get size according to viewbounds */
		const int k0 = data->viewbounds[0] * data->primary_n_rows + data->viewbounds[2];
		const int k1 = data->viewbounds[1] * data->primary_n_rows + data->viewbounds[3];
		const double xx = data->primary_x[k1] - data->primary_x[k0];
		const double yy = data->primary_y[k1] - data->primary_y[k0];
		const double sitesizesmall = 0.004 * sqrt(xx * xx + yy * yy);
		const double sitesizelarge = 1.4 * sitesizesmall;

		/* Use disks for 2D plotting */
		if (data->display_mode == MBV_DISPLAY_2D) {
			/* make list for small site */
			glNewList((GLuint)MBV_GLLIST_SITESMALL, GL_COMPILE);
			GLUquadricObj *globj = gluNewQuadric();
			gluDisk(globj, 0.0, sitesizesmall, 10, 1);
			gluDeleteQuadric(globj);
			int icolor = MBV_COLOR_BLACK;
			glColor3f(colortable_object_red[icolor], colortable_object_green[icolor], colortable_object_blue[icolor]);
			globj = gluNewQuadric();
			gluDisk(globj, 0.8 * sitesizesmall, sitesizesmall, 10, 1);
			gluDeleteQuadric(globj);
			glEndList();

			/* make list for large site */
			glNewList((GLuint)MBV_GLLIST_SITELARGE, GL_COMPILE);
			globj = gluNewQuadric();
			gluDisk(globj, 0.0, sitesizelarge, 10, 1);
			gluDeleteQuadric(globj);
			icolor = MBV_COLOR_BLACK;
			glColor3f(colortable_object_red[icolor], colortable_object_green[icolor], colortable_object_blue[icolor]);
			globj = gluNewQuadric();
			gluDisk(globj, 0.8 * sitesizelarge, sitesizelarge, 10, 1);
			gluDeleteQuadric(globj);
			glEndList();
		}

		/* Use spheres for 3D plotting */
		else if (data->display_mode == MBV_DISPLAY_3D) {
			/* make list for small site */
			glNewList((GLuint)MBV_GLLIST_SITESMALL, GL_COMPILE);
			GLUquadricObj *globj = gluNewQuadric();
			gluSphere(globj, sitesizesmall, 10, 10);
			gluDeleteQuadric(globj);
			glEndList();

			/* make list for large site */
			glNewList((GLuint)MBV_GLLIST_SITELARGE, GL_COMPILE);
			globj = gluNewQuadric();
			gluSphere(globj, sitesizelarge, 10, 10);
			gluDeleteQuadric(globj);
			glEndList();
		}

		/* loop over the sites */
		for (int isite = 0; isite < shared.shareddata.nsite; isite++) {
      if (shared.shareddata.sites[isite].active) {

  			/* set the color for this site */
  			int icolor;
  			if (isite == shared.shareddata.site_selected)
  				icolor = MBV_COLOR_RED;
  			else
  				icolor = shared.shareddata.sites[isite].color;
  			glColor3f(colortable_object_red[icolor], colortable_object_green[icolor], colortable_object_blue[icolor]);

  			/* draw the site as a disk or sphere using GLUT */
  			glTranslatef(shared.shareddata.sites[isite].point.xdisplay[instance],
  			             shared.shareddata.sites[isite].point.ydisplay[instance],
  			             shared.shareddata.sites[isite].point.zdisplay[instance]);
  			/*fprintf(stderr,"site:%d position: %f %f %f\n",
  			isite,shared.shareddata.sites[isite].point.xdisplay[instance],
  			shared.shareddata.sites[isite].point.ydisplay[instance],
  			shared.shareddata.sites[isite].point.zdisplay[instance]);*/
  			if (isite == shared.shareddata.site_selected)
  				glCallList((GLuint)MBV_GLLIST_SITELARGE);
  			else
  				glCallList((GLuint)MBV_GLLIST_SITESMALL);
  			glTranslatef(-shared.shareddata.sites[isite].point.xdisplay[instance],
  			             -shared.shareddata.sites[isite].point.ydisplay[instance],
  			             -shared.shareddata.sites[isite].point.zdisplay[instance]);
  		}
    }
	}
#ifdef MBV_GETERRORS
	mbview_glerrorcheck(instance, 1, __func__);
#endif

	const int status = MB_SUCCESS;

	if (mbv_verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:  %d\n", status);
	}

	return (status);
}

/*------------------------------------------------------------------------------*/
int mbview_updatesitelist() {
	if (mbv_verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  MB-system Version %s\n", MB_VERSION);
		fprintf(stderr, "dbg2  Input arguments:\n");
	}

	/* update site list */
	if (shared.init_sitelist == MBV_WINDOW_VISIBLE) {
		/* remove all existing items */
		XmListDeleteAllItems(shared.mb3d_sitelist.mbview_list_sitelist);

  	/* print site debug statements */
  	if (mbv_verbose >= 2) {
  		fprintf(stderr, "\ndbg2  Site data in function <%s>\n", __func__);
  		fprintf(stderr, "dbg2  Site values:\n");
  		fprintf(stderr, "dbg2       site_mode:           %d\n", shared.shareddata.site_mode);
  		fprintf(stderr, "dbg2       nsite:               %d\n", shared.shareddata.nsite);
  		fprintf(stderr, "dbg2       nsite_alloc:         %d\n", shared.shareddata.nsite_alloc);
  		fprintf(stderr, "dbg2       site_selected:       %d\n", shared.shareddata.site_selected);
  		for (int i = 0; i < shared.shareddata.nsite; i++) {
  			fprintf(stderr, "dbg2       site %d active:      %d\n", i, shared.shareddata.sites[i].active);
  			fprintf(stderr, "dbg2       site %d color:       %d\n", i, shared.shareddata.sites[i].color);
  			fprintf(stderr, "dbg2       site %d size:        %d\n", i, shared.shareddata.sites[i].size);
  			fprintf(stderr, "dbg2       site %d name:        %s\n", i, shared.shareddata.sites[i].name);
  			fprintf(stderr, "dbg2       site %d xlon:        %f\n", i, shared.shareddata.sites[i].point.xlon);
  			fprintf(stderr, "dbg2       site %d ylat:        %f\n", i, shared.shareddata.sites[i].point.ylat);
  			fprintf(stderr, "dbg2       site %d zdata:       %f\n", i, shared.shareddata.sites[i].point.zdata);
  		}
  	}

		if (shared.shareddata.nsite > 0) {
			/* allocate array of label XmStrings */
			XmString *xstr = (XmString *)malloc(shared.shareddata.nsite * sizeof(XmString));

			/* loop over the sites */
			for (int isite = 0; isite < shared.shareddata.nsite; isite++) {
        if (shared.shareddata.sites[isite].active) {
  				/* add list item for each site */
  				char londstr0[24], lonmstr0[24];
  				char latdstr0[24], latmstr0[24];
  				mbview_setlonlatstrings(shared.shareddata.sites[isite].point.xlon, shared.shareddata.sites[isite].point.ylat,
  				                        londstr0, latdstr0, lonmstr0, latmstr0);
  				if (shared.lonlatstyle == MBV_LONLAT_DEGREESDECIMAL)
  					sprintf(value_string, "%3d | %s | %s | %.3f | %s | %d | %s", isite, londstr0, latdstr0,
  					        shared.shareddata.sites[isite].point.zdata, mbview_colorname[shared.shareddata.sites[isite].color],
  					        shared.shareddata.sites[isite].size, shared.shareddata.sites[isite].name);
  				else
  					sprintf(value_string, "%3d | %s | %s | %.3f | %s | %d | %s", isite, lonmstr0, latmstr0,
  					        shared.shareddata.sites[isite].point.zdata, mbview_colorname[shared.shareddata.sites[isite].color],
  					        shared.shareddata.sites[isite].size, shared.shareddata.sites[isite].name);
  				xstr[isite] = XmStringCreateLocalized(value_string);
  			}
      }

			/* add list items */
			XmListAddItems(shared.mb3d_sitelist.mbview_list_sitelist, xstr, shared.shareddata.nsite, 0);

			/* select list item for selected site */
			if (shared.shareddata.site_selected != MBV_SELECT_NONE) {
				XmListSelectPos(shared.mb3d_sitelist.mbview_list_sitelist, shared.shareddata.site_selected + 1, 0);
				XmListSetPos(shared.mb3d_sitelist.mbview_list_sitelist, MAX(shared.shareddata.site_selected + 1 - 5, 1));
			}

			/* deallocate memory no longer needed */
			for (int isite = 0; isite < shared.shareddata.nsite; isite++) {
				XmStringFree(xstr[isite]);
			}
			free(xstr);
		}
	}

	const int status = MB_SUCCESS;

	if (mbv_verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:  %d\n", status);
	}

	return (status);
}

/*------------------------------------------------------------------------------*/
