/*------------------------------------------------------------------------------
 *    The MB-system:	mbview_plot.c	9/26/2003
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
 * Note:	This code was broken out of mbview_callbacks.c.
 */

#include <ctype.h>
#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "mb_status.h"
#include "mb_define.h"

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
#include "MB3DView.h"
#include "MB3DSiteList.h"
#include "MB3DRouteList.h"
#include "MB3DNavList.h"

#include <GL/gl.h>
#include <GL/glu.h>
#ifndef WIN32
#include <GL/glx.h>
#endif
#include "mb_glwdrawa.h"

#include "mbview.h"
#include "mbviewprivate.h"

// #define MBV_DEBUG_GLX 1
// #define MBV_GET_GLX_ERRORS 1

/*------------------------------------------------------------------------------*/
int mbview_reset_glx(size_t instance) {
	if (mbv_verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  MB-system Version %s\n", MB_VERSION);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       instance:         %zu\n", instance);
	}

	struct mbview_world_struct *view = &(mbviews[instance]);
	struct mbview_struct *data = &(view->data);
#ifdef MBV_DEBUG_GLX
		fprintf(stderr, "%s:%d:%s instance:%zu view->glx_init:%d\n", __FILE__, __LINE__, __func__, instance,
		        view->glx_init);
#endif

	/* delete old glx_context if it exists */
	if (view->glx_init) {
#ifdef MBV_DEBUG_GLX
		fprintf(stderr, "%s:%d:%s instance:%zu glXMakeCurrent(%p,%lu,%p)\n", __FILE__, __LINE__, __func__, instance,
		        view->dpy, XtWindow(view->glwmda), view->glx_context);
#endif
		//glXMakeCurrent(view->dpy, XtWindow(view->glwmda), view->glx_context);
#ifdef MBV_DEBUG_GLX
		fprintf(stderr, "%s:%d:%s instance:%zu glXDestroyContext(%p,%lu,%p)\n", __FILE__, __LINE__, __func__, instance,
		        view->dpy, XtWindow(view->glwmda), view->glx_context);
#endif
		glXDestroyContext(view->dpy, view->glx_context);
		view->glx_init = false;

#ifdef MBV_GET_GLX_ERRORS
		mbview_glerrorcheck(instance, __FILE__, __LINE__, __func__);
#endif
	}

  Cardinal ac;
  Arg args[256];

	/* set up a new opengl context */
	ac = 0;
	XtSetArg(args[ac], mbGLwNvisualInfo, &(view->vi));
	ac++;
	XtGetValues(view->glwmda, args, ac);
#ifdef MBV_DEBUG_GLX
	fprintf(stderr, "%s:%d:%s instance:%zu glXCreateContext(%p,%p)\n", __FILE__, __LINE__, __func__, instance, view->dpy,
	        view->vi);
#endif
	view->glx_context = glXCreateContext(view->dpy, view->vi, NULL, GL_TRUE);
#ifdef MBV_DEBUG_GLX
	fprintf(stderr, "%s:%d:%s instance:%zu glXMakeCurrent(%p,%lu,%p)\n", __FILE__, __LINE__, __func__, instance, view->dpy,
	        XtWindow(view->glwmda), view->glx_context);
#endif
	glXMakeCurrent(view->dpy, XtWindow(view->glwmda), view->glx_context);
	view->glx_init = true;
	glViewport(0, 0, data->width, data->height);
	view->aspect_ratio = ((float)data->width) / ((float)data->height);
	view->lastdrawrez = MBV_REZ_NONE;
	view->contourlorez = false;
	view->contourhirez = false;
	view->contourfullrez = false;

#ifdef MBV_GET_GLX_ERRORS
	mbview_glerrorcheck(instance, __FILE__, __LINE__, __func__);
#endif

	const int status = MB_SUCCESS;

	if (mbv_verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:  %d\n", status);
		fprintf(stderr, "dbg2       view->dpy:             %p\n", view->dpy);
		fprintf(stderr, "dbg2       view->vi:              %p\n", view->vi);
		fprintf(stderr, "dbg2       view->glwmda:          %p\n", view->glwmda);
		fprintf(stderr, "dbg2       view->glx_context:     %p\n", view->glx_context);
		fprintf(stderr, "dbg2       view->glx_init:        %d\n", view->glx_init);
		fprintf(stderr, "dbg2       view->lastdrawrez:     %d\n", view->lastdrawrez);
		fprintf(stderr, "dbg2       view->contourlorez:    %d\n", view->contourlorez);
		fprintf(stderr, "dbg2       view->contourhirez:    %d\n", view->contourhirez);
		fprintf(stderr, "dbg2       view->contourfullrez:  %d\n", view->contourfullrez);
	}

	return (status);
}

/*------------------------------------------------------------------------------*/
int mbview_drawdata(size_t instance, int rez) {
	if (mbv_verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  MB-system Version %s\n", MB_VERSION);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       instance:         %zu\n", instance);
		fprintf(stderr, "dbg2       rez:              %d\n", rez);
	}

	struct mbview_world_struct *view = &(mbviews[instance]);
	struct mbview_struct *data = &(view->data);

	/* get size of grid in view */
	const int nxrange = data->viewbounds[1] - data->viewbounds[0] + 1;
	const int nyrange = data->viewbounds[3] - data->viewbounds[2] + 1;

	/* set stride for looping over data */
	int stride;
	if (rez == MBV_REZ_FULL)
		stride = 1;
	else if (rez == MBV_REZ_HIGH)
		stride = MAX((int)ceil(((double)nxrange) / ((double)data->hirez_dimension)),
		             (int)ceil(((double)nyrange) / ((double)data->hirez_dimension)));
	else
		stride = MAX((int)ceil(((double)nxrange) / ((double)data->lorez_dimension)),
		             (int)ceil(((double)nyrange) / ((double)data->lorez_dimension)));

	/* enable depth test for 3D plots */
	if (data->display_mode == MBV_DISPLAY_3D || data->display_projection_mode == MBV_PROJECTION_SPHEROID)
		glEnable(GL_DEPTH_TEST);
#ifdef MBV_GET_GLX_ERRORS
	mbview_glerrorcheck(instance, __FILE__, __LINE__, __func__);
#endif

	/* set color parameters */
	mbview_setcolorparms(instance);

	/* calculate histogram equalization if needed */
	float *histogram = NULL;
	bool make_histogram = false;
	int which_data;
	if (data->grid_mode == MBV_GRID_VIEW_PRIMARY && data->primary_histogram) {
		if (!view->primary_histogram_set) {
			make_histogram = true;
			which_data = MBV_DATA_PRIMARY;
		}
		histogram = view->primary_histogram;
	}
	else if (data->grid_mode == MBV_GRID_VIEW_PRIMARYSLOPE && data->primaryslope_histogram) {
		if (!view->primaryslope_histogram_set) {
			make_histogram = true;
			which_data = MBV_DATA_PRIMARYSLOPE;
		}
		histogram = view->primaryslope_histogram;
	}
	else if (data->grid_mode == MBV_GRID_VIEW_SECONDARY && data->secondary_histogram) {
		if (!view->secondary_histogram_set) {
			make_histogram = true;
			which_data = MBV_DATA_SECONDARY;
		}
		histogram = view->secondary_histogram;
	}
	if (make_histogram)
		mbview_make_histogram(view, data, which_data);
	if (view->shade_mode == MBV_SHADE_VIEW_OVERLAY && data->secondary_histogram &&
	    !view->secondary_histogram_set)
		mbview_make_histogram(view, data, MBV_DATA_SECONDARY);

	/*fprintf(stderr,"mbview_drawdata: %d %d stride:%d\n", instance,rez,stride);*/

	/* draw the data as triangle strips */
	if (data->grid_mode != MBV_GRID_VIEW_SECONDARY) {
		for (int i = data->viewbounds[0]; i <= data->viewbounds[1] - stride; i += stride) {
			bool on = false;
			bool flip = false;
			for (int j = data->viewbounds[2]; j <= data->viewbounds[3]; j += stride) {
				const int k = i * data->primary_n_rows + j;
				const int l = (i + stride) * data->primary_n_rows + j;
				int ikk;
				int kk;
				int ill;
				int ll;
				if (flip) {
					ikk = i + stride;
					kk = l;
					ill = i;
					ll = k;
				} else {
					ikk = i;
					kk = k;
					ill = i + stride;
					ll = l;
				}
				if (data->primary_data[kk] != data->primary_nodatavalue) {
					if (!on) {
						glBegin(GL_TRIANGLE_STRIP);
						on = true;
						if (kk == k)
							flip = false;
						else
							flip = true;
					}
					if (!(data->primary_stat_z[kk / 8] & statmask[kk % 8]))
						mbview_zscalegridpoint(instance, kk);
					if (!(data->primary_stat_color[kk / 8] & statmask[kk % 8])) {
						mbview_colorpoint(view, data, histogram, ikk, j, kk);
					}
					glColor3f(data->primary_r[kk], data->primary_g[kk], data->primary_b[kk]);
					glVertex3f(data->primary_x[kk], data->primary_y[kk], data->primary_z[kk]);
					/*fprintf(stderr,"Drawing triangles: origin: %f %f %f  pt:%f %f %f\n",
					view->xorigin,view->yorigin,view->zorigin,
					data->primary_x[kk],data->primary_y[kk],data->primary_z[kk]);*/
				}
				else {
					if (on) {
						glEnd();
#ifdef MBV_GET_GLX_ERRORS
						mbview_glerrorcheck(instance, __FILE__, __LINE__, __func__);
#endif
						on = false;
					}
					flip = false;
				}
				if (data->primary_data[ll] != data->primary_nodatavalue) {
					if (!on) {
						glBegin(GL_TRIANGLE_STRIP);
						on = true;
						if (ll == l)
							flip = false;
						else
							flip = true;
					}
					if (!(data->primary_stat_z[ll / 8] & statmask[ll % 8]))
						mbview_zscalegridpoint(instance, ll);
					if (!(data->primary_stat_color[ll / 8] & statmask[ll % 8])) {
						mbview_colorpoint(view, data, histogram, ill, j, ll);
					}
					glColor3f(data->primary_r[ll], data->primary_g[ll], data->primary_b[ll]);
					glVertex3f(data->primary_x[ll], data->primary_y[ll], data->primary_z[ll]);
				}
				else {
					if (on) {
						glEnd();
#ifdef MBV_GET_GLX_ERRORS
						mbview_glerrorcheck(instance, __FILE__, __LINE__, __func__);
#endif
						on = false;
					}
					flip = false;
				}
			}
			if (on) {
				glEnd();
#ifdef MBV_GET_GLX_ERRORS
				mbview_glerrorcheck(instance, __FILE__, __LINE__, __func__);
#endif
				on = false;
				flip = false;
			}

			/* check for pending event */
			if (!view->plot_done && view->plot_interrupt_allowed && i % MBV_EVENTCHECKCOARSENESS == 0) {
				do_mbview_xevents();
			}

			/* dump out of loop if plotting already done at a higher recursion */
			if (view->plot_done)
				i = data->primary_n_columns;
		}
	}

	else /* if (data->grid_mode == MBV_GRID_VIEW_SECONDARY) */ {
		for (int i = data->viewbounds[0]; i < data->viewbounds[1] - stride; i += stride) {
			bool on = false;
			bool flip = false;
			for (int j = data->viewbounds[2]; j <= data->viewbounds[3]; j += stride) {
				const int k = i * data->primary_n_rows + j;
				const int l = (i + stride) * data->primary_n_rows + j;
				int ikk;
				int kk;
				int ill;
				int ll;
				if (flip) {
					ikk = i + stride;
					kk = l;
					ill = i;
					ll = k;
				}
				else {
					ikk = i;
					kk = k;
					ill = i + stride;
					ll = l;
				}
				double secondary_value;
				if (data->secondary_sameas_primary)
					secondary_value = data->secondary_data[kk];
				else
					mbview_getsecondaryvalue(view, data, ikk, j, &secondary_value);
				if (data->primary_data[kk] != data->primary_nodatavalue
					&& secondary_value != data->secondary_nodatavalue) {
					if (!on) {
						glBegin(GL_TRIANGLE_STRIP);
						on = true;
						if (kk == k)
							flip = false;
						else
							flip = true;
					}
          // TODO: 8 March 2020 D W Caress
          // The addition of "stride == 1" below forces the code to recolor all
          // vertices when plotting at full resolution. If not, sometimes the
          // secondary data are partly mislocated - this bug is not understood
          // - somehow the color grids are being written or overwritten incorrectly
          // before.
					if (stride == 1 || !(data->primary_stat_z[kk / 8] & statmask[kk % 8]))
						mbview_zscalegridpoint(instance, kk);
					if (stride == 1 || !(data->primary_stat_color[kk / 8] & statmask[kk % 8])) {
						mbview_colorpoint(view, data, histogram, ikk, j, kk);
					}
					glColor3f(data->primary_r[kk], data->primary_g[kk], data->primary_b[kk]);
					glVertex3f(data->primary_x[kk], data->primary_y[kk], data->primary_z[kk]);
					/*fprintf(stderr,"Drawing triangles: origin: %f %f %f  pt:%f %f %f\n",
					view->xorigin,view->yorigin,view->zorigin,
					data->primary_x[kk],data->primary_y[kk],data->primary_z[kk]);*/
				}
				else {
					if (on) {
						glEnd();
#ifdef MBV_GET_GLX_ERRORS
						mbview_glerrorcheck(instance, __FILE__, __LINE__, __func__);
#endif
						on = false;
					}
					flip = false;
				}
				if (data->secondary_sameas_primary)
					secondary_value = data->secondary_data[ll];
				else
					mbview_getsecondaryvalue(view, data, ill, j, &secondary_value);
				if (data->primary_data[ll] != data->primary_nodatavalue
					&& secondary_value != data->secondary_nodatavalue) {
					if (!on) {
						glBegin(GL_TRIANGLE_STRIP);
						on = true;
						if (ll == l)
							flip = false;
						else
							flip = true;
					}
          // TODO: 8 March 2020 D W Caress
          // The addition of "stride == 1" below forces the code to recolor all
          // vertices when plotting at full resolution. If not, sometimes the
          // secondary data are partly mislocated - this bug is not understood
          // - somehow the color grids are being written or overwritten incorrectly
          // before.
					if (stride == 1 || !(data->primary_stat_z[ll / 8] & statmask[ll % 8]))
						mbview_zscalegridpoint(instance, ll);
					if (stride == 1 || !(data->primary_stat_color[ll / 8] & statmask[ll % 8])) {
						mbview_colorpoint(view, data, histogram, ill, j, ll);
					}
					glColor3f(data->primary_r[ll], data->primary_g[ll], data->primary_b[ll]);
					glVertex3f(data->primary_x[ll], data->primary_y[ll], data->primary_z[ll]);
				}
				else {
					if (on) {
						glEnd();
#ifdef MBV_GET_GLX_ERRORS
						mbview_glerrorcheck(instance, __FILE__, __LINE__, __func__);
#endif
						on = false;
					}
					flip = false;
				}
			}
			if (on) {
				glEnd();
#ifdef MBV_GET_GLX_ERRORS
				mbview_glerrorcheck(instance, __FILE__, __LINE__, __func__);
#endif
				on = false;
				flip = false;
			}

			/* check for pending event */
			if (!view->plot_done && view->plot_interrupt_allowed && i % MBV_EVENTCHECKCOARSENESS == 0) {
				do_mbview_xevents();
			}

			/* dump out of loop if plotting already done at a higher recursion */
			if (view->plot_done)
				i = data->primary_n_columns;
		}
	}
#ifdef MBV_GET_GLX_ERRORS
	mbview_glerrorcheck(instance, __FILE__, __LINE__, __func__);
#endif

	/* draw the triangle outlines */
/*	glColor3f(1.0, 0.0, 0.0);
	for (int i = 0;i<data->primary_n_columns-1;i++) {
    	for (int j = 0;j<data->primary_n_rows-1;j++) {
    	    int k = i * data->primary_n_rows + j;
    	    int l = (i + 1) * data->primary_n_rows + j;
    	    int m = i * data->primary_n_rows + j + 1;
    	    int n = (i + 1) * data->primary_n_rows + j + 1;
    	    if (data->primary_data[k] != data->primary_nodatavalue
    	        && data->primary_data[l] != data->primary_nodatavalue
    	        && data->primary_data[m] != data->primary_nodatavalue) {
    	        glBegin(GL_LINE_LOOP);
    	        glVertex3f(data->primary_x[k],
    	            data->primary_y[k],
    	            data->primary_z[k]);
    	        glVertex3f(data->primary_x[l],
    	            data->primary_y[l],
    	            data->primary_z[l]);
    	        glVertex3f(data->primary_x[m],
    	            data->primary_y[m],
    	            data->primary_z[m]);
    	        glEnd();
    	    }
    	    if (data->primary_data[l] != data->primary_nodatavalue
    	        && data->primary_data[m] != data->primary_nodatavalue
    	        && data->primary_data[n] != data->primary_nodatavalue) {
    	        glBegin(GL_LINE_LOOP);
    	        glVertex3f(data->primary_x[l],
    	            data->primary_y[l],
    	            data->primary_z[l]);
    	        glVertex3f(data->primary_x[n],
    	            data->primary_y[n],
    	            data->primary_z[n]);
    	        glVertex3f(data->primary_x[m],
    	            data->primary_y[m],
    	            data->primary_z[m]);
    	        glEnd();
    	    }
    	}
	}*/

	/* draw contours */
	if (data->grid_contour_mode == MBV_VIEW_ON) {
		if (rez == MBV_REZ_FULL && view->contourfullrez)
			glCallList((GLuint)(3 * instance + 3));
		else if (rez == MBV_REZ_HIGH && view->contourhirez)
			glCallList((GLuint)(3 * instance + 2));
		else if (rez == MBV_REZ_LOW && view->contourlorez)
			glCallList((GLuint)(3 * instance + 1));
	}

#ifdef MBV_GET_GLX_ERRORS
	mbview_glerrorcheck(instance, __FILE__, __LINE__, __func__);
#endif

	/* draw current pick */
	mbview_drawpick(instance);

	/* draw current area */
	mbview_drawarea(instance);

	/* draw current region */
	mbview_drawregion(instance);

	/* draw current navpick */
	mbview_drawnavpick(instance);

	/* draw sites */
	mbview_drawsite(instance, rez);

	/* draw routes */
	mbview_drawroute(instance, rez);

	/* draw nav */
	mbview_drawnav(instance, rez);

	/* draw vectors */
	mbview_drawvector(instance, rez);

	/* make sure depth test is off */
	glDisable(GL_DEPTH_TEST);

	/* set lastdrawrez flag */
	view->lastdrawrez = rez;

	const int status = MB_SUCCESS;

	if (mbv_verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:  %d\n", status);
	}

	return (status);
}
/*------------------------------------------------------------------------------*/
int mbview_plotlowall(size_t instance) {
	if (mbv_verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  MB-system Version %s\n", MB_VERSION);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       instance:         %zu\n", instance);
	}

	/* replot all active instances except for instance
	    which should already be replotted */
	for (unsigned int i = 0; i < MBV_MAX_WINDOWS; i++) {
		if (i != instance && mbviews[i].data.active)
			mbview_plotlow(i);
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
int mbview_plotlowhighall(size_t instance) {
	if (mbv_verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  MB-system Version %s\n", MB_VERSION);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       instance:         %zu\n", instance);
	}

	/* replot all active instances except for instance
	    which should already be replotted */
	mbview_plotlowall(instance);
	mbview_plothighall(instance);

	const int status = MB_SUCCESS;

	if (mbv_verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:  %d\n", status);
	}

	return (status);
}

/*------------------------------------------------------------------------------*/
int mbview_plothighall(size_t instance) {
	if (mbv_verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  MB-system Version %s\n", MB_VERSION);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       instance:         %zu\n", instance);
	}

	/* replot all active instances except for instance
	    which should already be replotted */
	for (unsigned int i = 0; i < MBV_MAX_WINDOWS; i++) {
		if (i != instance && mbviews[i].data.active)
			mbview_plothigh(i);
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
int mbview_plotlow(size_t instance) {
	if (mbv_verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  MB-system Version %s\n", MB_VERSION);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       instance:         %zu\n", instance);
	}

	struct mbview_world_struct *view = &(mbviews[instance]);
	struct mbview_struct *data = &(view->data);

	int status = MB_SUCCESS;

	/* only plot if mbview active for this instance */
	if (data->active) {
		/* set plot_done to false and increment the plot recursion level */
		view->plot_done = false;
		view->plot_recursion++;

		status = mbview_plot(instance, MBV_REZ_LOW);

		/* the plot_done flag will still be false if this
		   is the highest recursion level to be reached - finish the plot
		   only in this case */
		if (!view->plot_done) {
			/* set plot_done to true */
			view->plot_done = true;
			if (mbv_verbose >= 2)
				fprintf(stderr, "Plot finished! instance:%zu recursion:%d\n", instance, view->plot_recursion);
		}

		/* decrement the plot recursion level */
		view->plot_recursion--;

		if (view->message_on && view->plot_recursion == 0)
			do_mbview_status("Done.", instance);
		if (mbv_verbose >= 2)
			fprintf(stderr, "Done with mbview_plotlow %zd  recursion:%d\n\n", instance, view->plot_recursion);
	}

	if (mbv_verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:  %d\n", status);
	}

	return (status);
}

/*------------------------------------------------------------------------------*/
int mbview_plotlowhigh(size_t instance) {
	if (mbv_verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  MB-system Version %s\n", MB_VERSION);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       instance:         %zu\n", instance);
	}

	struct mbview_world_struct *view = &(mbviews[instance]);
	struct mbview_struct *data = &(view->data);

	int status = MB_SUCCESS;

	/* only plot if mbview active for this instance */
	if (data->active) {

		/* set plot_done to false and increment the plot recursion level */
		view->plot_done = false;
		view->plot_recursion++;

		status = mbview_plot(instance, MBV_REZ_LOW);

		status = mbview_plot(instance, MBV_REZ_HIGH);

		/* the plot_done flag will still be false if this
		   is the highest recursion level to be reached - finish the plot
		   only in this case */
		if (!view->plot_done) {
			/* set plot_done to true */
			view->plot_done = true;
			if (mbv_verbose >= 2)
				fprintf(stderr, "Plot finished! instance:%zu recursion:%d\n", instance, view->plot_recursion);
		}

		/* decrement the plot recursion level */
		view->plot_recursion--;

		if (view->message_on && view->plot_recursion == 0)
			do_mbview_status("Done.", instance);
		if (mbv_verbose >= 2)
			fprintf(stderr, "Done with mbview_plotlowhigh %zd  recursion:%d\n\n", instance, view->plot_recursion);
	}

	if (mbv_verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:  %d\n", status);
	}

	return (status);
}
/*------------------------------------------------------------------------------*/
int mbview_plothigh(size_t instance) {
	if (mbv_verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  MB-system Version %s\n", MB_VERSION);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       instance:         %zu\n", instance);
	}

	struct mbview_world_struct *view = &(mbviews[instance]);
	struct mbview_struct *data = &(view->data);

	int status = MB_SUCCESS;

	/* only plot if mbview active for this instance */
	if (data->active) {
		/* set plot_done to false and increment the plot recursion level */
		view->plot_done = false;
		view->plot_recursion++;

		status = mbview_plot(instance, MBV_REZ_HIGH);

		/* the plot_done flag will still be false if this
		   is the highest recursion level to be reached - finish the plot
		   only in this case */
		if (!view->plot_done) {
			/* set plot_done to true */
			view->plot_done = true;
			if (mbv_verbose >= 2)
				fprintf(stderr, "Plot finished! instance:%zu recursion:%d\n", instance, view->plot_recursion);
		}

		/* decrement the plot recursion level */
		view->plot_recursion--;

		if (view->message_on && view->plot_recursion == 0)
			do_mbview_status("Done.", instance);
		if (mbv_verbose >= 2)
			fprintf(stderr, "Done with mbview_plothigh %zd  recursion:%d\n\n", instance, view->plot_recursion);
	}

	if (mbv_verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:  %d\n", status);
	}

	return (status);
}
/*------------------------------------------------------------------------------*/
int mbview_plotfull(size_t instance) {
	if (mbv_verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  MB-system Version %s\n", MB_VERSION);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       instance:         %zu\n", instance);
	}

	struct mbview_world_struct *view = &(mbviews[instance]);
	struct mbview_struct *data = &(view->data);

	int status = MB_SUCCESS;

	/* only plot if mbview active for this instance */
	if (data->active) {

		/* set plot_done to false and increment the plot recursion level */
		view->plot_done = false;
		view->plot_recursion++;

		status = mbview_plot(instance, MBV_REZ_FULL);

		/* the plot_done flag will still be false if this
		   is the highest recursion level to be reached - finish the plot
		   only in this case */
		if (!view->plot_done) {
			/* set plot_done to true */
			view->plot_done = true;
			if (mbv_verbose >= 2)
				fprintf(stderr, "Plot finished! instance:%zu recursion:%d\n", instance, view->plot_recursion);
		}

		/* decrement the plot recursion level */
		view->plot_recursion--;

		if (view->message_on && view->plot_recursion == 0)
			do_mbview_status("Done.", instance);
		if (mbv_verbose >= 2)
			fprintf(stderr, "Done with mbview_plotfull %zd  recursion:%d\n\n", instance, view->plot_recursion);
	}

	if (mbv_verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:  %d\n", status);
	}

	return (status);
}
/*------------------------------------------------------------------------------*/
int mbview_plot(size_t instance, int rez) {
	if (mbv_verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  MB-system Version %s\n", MB_VERSION);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       instance:         %zu\n", instance);
		fprintf(stderr, "dbg2       rez:              %d\n", rez);
	}

	struct mbview_world_struct *view = &(mbviews[instance]);
	struct mbview_struct *data = &(view->data);

	/* only plot if this view is still active */
	if (view->glx_init) {

/* make correct window current for OpenGL */
#ifdef MBV_DEBUG_GLX
		fprintf(stderr, "%s:%d:%s instance:%zu glXMakeCurrent(%p,%lu,%p)\n", __FILE__, __LINE__, __func__, instance,
		        view->dpy, XtWindow(view->glwmda), view->glx_context);
#endif
		glXMakeCurrent(view->dpy, XtWindow(view->glwmda), view->glx_context);

#ifdef MBV_GET_GLX_ERRORS
		mbview_glerrorcheck(instance, __FILE__, __LINE__, __func__);
#endif

		/*fprintf(stderr,"\nmbview_plot: instance:%zu rez:%d recursion:%zu\n",instance,rez,view->plot_recursion);
		fprintf(stderr,"     view->plot_done:        %d\n",view->plot_done);
		fprintf(stderr,"     view->plot_recursion:   %d\n",view->plot_recursion);
		fprintf(stderr,"     view->projected:        %d\n",view->projected);
		fprintf(stderr,"     view->contourlorez:     %d\n",view->contourlorez);
		fprintf(stderr,"     view->contourhirez:     %d\n",view->contourhirez);
		fprintf(stderr,"     view->contourfullrez:   %d\n",view->contourfullrez);
		fprintf(stderr,"     data->pick_type:  %d\n",data->pick_type);*/

		/* apply projection if needed */
		if (!view->plot_done && !view->projected) {
			do_mbview_status("Projecting data...", instance);
			mbview_projectdata(instance);
		}

		/* apply projection to global data if needed */
		if (!view->plot_done && !view->globalprojected) {
			do_mbview_status("Projecting global data...", instance);
			mbview_projectglobaldata(instance);
		}

		/* contour if needed */
		if (!view->plot_done && (data->grid_contour_mode == MBV_VIEW_ON) &&
		    ((rez == MBV_REZ_FULL && !view->contourfullrez) || (rez == MBV_REZ_HIGH && !view->contourhirez) ||
		     (rez == MBV_REZ_LOW && !view->contourlorez))) {
			if (rez == MBV_REZ_FULL)
				do_mbview_status("Contouring data...", instance);
			mbview_contour(instance, rez);
		}

		/* get bounds of grid seen in current view */
		if (rez == MBV_REZ_FULL && data->display_mode == MBV_DISPLAY_3D) {
			data->viewbounds[0] = 0;
			data->viewbounds[1] = data->primary_n_columns - 1;
			data->viewbounds[2] = 0;
			data->viewbounds[3] = data->primary_n_rows - 1;
		}
		else if (view->viewboundscount >= MBV_BOUNDSFREQUENCY) {
			mbview_viewbounds(instance);
			view->viewboundscount = 0;

			/* regenerate 3D drape of pick marks if either 3D display
			    or the pick move is final */
			if (data->pick_type != MBV_PICK_NONE && data->display_mode == MBV_DISPLAY_3D) {
				mbview_picksize(instance);
			}
		}

		/* do the actual openGL plotting */
		if (!view->plot_done) {
			/* set projection to 2D or 3D */
			glMatrixMode(GL_PROJECTION);
			glLoadIdentity();
			view->right = MBV_OPENGL_WIDTH / view->size2d;
			view->left = -MBV_OPENGL_WIDTH / view->size2d;
			view->top = MBV_OPENGL_WIDTH / view->aspect_ratio / view->size2d;
			view->bottom = -MBV_OPENGL_WIDTH / view->aspect_ratio / view->size2d;
			if (data->display_mode == MBV_DISPLAY_2D) {
				glOrtho(view->left, view->right, view->bottom, view->top, MBV_OPENGL_ZMIN2D, MBV_OPENGL_ZMAX2D);
			}
			else {
				gluPerspective(40.0, view->aspect_ratio, 0.01 * MBV_OPENGL_WIDTH, 1000 * MBV_OPENGL_WIDTH);
			}

			/* set up translations */
			glMatrixMode(GL_MODELVIEW);
			glLoadIdentity();
			if (data->display_mode == MBV_DISPLAY_2D) {
				glTranslated(view->offset2d_x, view->offset2d_y, MBV_OPENGL_ZMIN2D);
			}
			else if (data->display_mode == MBV_DISPLAY_3D) {
				const float viewdistance = 0.48 * MBV_OPENGL_WIDTH * MBV_OPENGL_WIDTH / view->aspect_ratio;
				glTranslated(0.0, 0.0, -viewdistance + view->viewoffset3d_z);
				glRotated((float)(data->viewelevation3d - 90.0), 1.0, 0.0, 0.0);
				glRotated((float)(data->viewazimuth3d), 0.0, 1.0, 1.0);
				glTranslated(view->offset3d_x, view->offset3d_y, -viewdistance + view->offset3d_z);
				glRotated((float)(data->modelelevation3d - 90.0), 1.0, 0.0, 0.0);
				glRotated((float)(data->modelazimuth3d), 0.0, 0.0, 1.0);
			}

			/* set background color */
			glClearColor(1.0, 1.0, 1.0, 0.0);
			glClearDepth((GLclampd)(2000 * MBV_OPENGL_WIDTH));
			glDepthFunc(GL_LESS);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

			/* draw data */
			if (!view->plot_done) {
				if (rez == MBV_REZ_FULL)
					do_mbview_status("Drawing full rez...", instance);
				else if (rez == MBV_REZ_HIGH)
					do_mbview_status("Drawing high rez...", instance);
				mbview_drawdata(instance, rez);
			}
		}

		/* the plot_done flag will still be false if this
		   is the highest recursion level to be reached - finish the plot
		   only in this case */
		if (!view->plot_done) {
			/* flush opengl buffers */
			glFlush();

/* make correct window current for OpenGL (may have changed due to recursion) */
#ifdef MBV_DEBUG_GLX
			fprintf(stderr, "%s:%d:%s instance:%zu glXMakeCurrent(%p,%lu,%p)\n", __FILE__, __LINE__, __func__, instance,
			        view->dpy, XtWindow(view->glwmda), view->glx_context);
#endif
			glXMakeCurrent(view->dpy, XtWindow(view->glwmda), view->glx_context);

#ifdef MBV_GET_GLX_ERRORS
			mbview_glerrorcheck(instance, __FILE__, __LINE__, __func__);
#endif

/* swap opengl buffers */
#ifdef MBV_DEBUG_GLX
			fprintf(stderr, "%s:%d:%s instance:%zu glXSwapBuffers(%p,%lu)\n", __FILE__, __LINE__, __func__, instance,
			        view->dpy, XtWindow(view->glwmda));
#endif
			glXSwapBuffers(view->dpy, XtWindow(view->glwmda));
#ifdef MBV_GET_GLX_ERRORS
			mbview_glerrorcheck(instance, __FILE__, __LINE__, __func__);
#endif
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
int mbview_findpoint(size_t instance, int xpixel, int ypixel, bool *found, double *xgrid, double *ygrid, double *xlon,
                     double *ylat, double *zdata, double *xdisplay, double *ydisplay, double *zdisplay) {
	if (mbv_verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  MB-system Version %s\n", MB_VERSION);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       instance:         %zu\n", instance);
		fprintf(stderr, "dbg2       xpixel:           %d\n", xpixel);
		fprintf(stderr, "dbg2       ypixel:           %d\n", ypixel);
	}

	struct mbview_world_struct *view = &(mbviews[instance]);
	struct mbview_struct *data = &(view->data);

	/* only plot if this view is still active */
	if (view->glx_init) {
		/* look for point at low resolution */
		*found = false;
		bool foundsave = false;
		int ijbounds[4] = {0, data->primary_n_columns, 0, data->primary_n_rows};
		{
			const int rez = MBV_REZ_LOW;
			mbview_findpointrez(
				instance, rez, xpixel, ypixel, ijbounds, found,
				xgrid, ygrid, xlon, ylat, zdata,
				xdisplay, ydisplay, zdisplay);
//fprintf(stderr, "\n%s:%d:%s: Called mbview_findpointrez: rez:%d ij: %d %d found:%d bounds: %d %d %d %d\n",
//__FILE__, __LINE__, __FUNCTION__, rez, xpixel, ypixel, *found, ijbounds[0], ijbounds[1], ijbounds[2], ijbounds[3]);
		}
		/*fprintf(stderr,"First findpointrez: rez:%d pixels:%d %d found:%d xlon:%f ylat:%f zdata:%f\n",
		rez,xpixel,ypixel,found,xlon,ylat,zdata);*/
		double xgridsave, ygridsave;
		double xlonsave, ylatsave, zdatasave;
		double xdisplaysave, ydisplaysave, zdisplaysave;

		if (*found) {
			/* save last good results */
			foundsave = *found;
			xgridsave = *xgrid;
			ygridsave = *ygrid;
			xlonsave = *xlon;
			ylatsave = *ylat;
			zdatasave = *zdata;
			xdisplaysave = *xdisplay;
			ydisplaysave = *ydisplay;
			zdisplaysave = *zdisplay;
		}

		/* now check high rez */
		{
			const int rez = MBV_REZ_HIGH;
			mbview_findpointrez(
				instance, rez, xpixel, ypixel, ijbounds, found,
				xgrid, ygrid, xlon, ylat, zdata,
				xdisplay, ydisplay, zdisplay);
//fprintf(stderr, "%s:%d:%s: Called mbview_findpointrez: rez:%d ij: %d %d found:%d bounds: %d %d %d %d\n",
//__FILE__, __LINE__, __FUNCTION__, rez, xpixel, ypixel, *found, ijbounds[0], ijbounds[1], ijbounds[2], ijbounds[3]);
		}
		if (!(*found) && foundsave) {
			// rez = MBV_REZ_LOW;
			*found = foundsave;
			*xgrid = xgridsave;
			*ygrid = ygridsave;
			*xlon = xlonsave;
			*ylat = ylatsave;
			*xdisplay = xdisplaysave;
			*ydisplay = ydisplaysave;
			*zdisplay = zdisplaysave;
		}
/*if (*found && (ijbounds[1] != ijbounds[0] || ijbounds[3] != ijbounds[2]))
fprintf(stderr, "%s:%d:%s: Looping over mbview_findpointrez calls\n",
__FILE__, __LINE__, __FUNCTION__);
else
fprintf(stderr, "%s:%d:%s: Not looping over mbview_findpointrez calls\n",
__FILE__, __LINE__, __FUNCTION__);*/

		/* repeat until found at highest resolution possible */
		while (*found && (ijbounds[1] != ijbounds[0] || ijbounds[3] != ijbounds[2])) {
			/* save last good results */
			foundsave = *found;
			xgridsave = *xgrid;
			ygridsave = *ygrid;
			xlonsave = *xlon;
			ylatsave = *ylat;
			zdatasave = *zdata;
			xdisplaysave = *xdisplay;
			ydisplaysave = *ydisplay;
			zdisplaysave = *zdisplay;

			/* choose resolution */
			const int rez =
				(ijbounds[1] - ijbounds[0]) > data->hirez_dimension || (ijbounds[3] - ijbounds[2]) > data->hirez_dimension
				? MBV_REZ_HIGH : MBV_REZ_FULL;

			/* try again */
			mbview_findpointrez(instance, rez, xpixel, ypixel, ijbounds, found, xgrid, ygrid, xlon, ylat, zdata, xdisplay,
			                    ydisplay, zdisplay);
//fprintf(stderr, "%s:%d:%s: Called mbview_findpointrez: rez:%d ij: %d %d found:%d bounds: %d %d %d %d\n",
//__FILE__, __LINE__, __FUNCTION__, rez, xpixel, ypixel, *found, ijbounds[0], ijbounds[1], ijbounds[2], ijbounds[3]);
		}

		/* if not found and 2D get position directly from pixels */
		if (!(*found) && data->display_mode == MBV_DISPLAY_2D) {
			*xdisplay =
			    view->left - view->offset2d_x + 2.0 * MBV_OPENGL_WIDTH / view->size2d * ((double)xpixel) / ((double)data->width);
			*ydisplay = view->bottom - view->offset2d_y +
			            2.0 * MBV_OPENGL_WIDTH / view->aspect_ratio / view->size2d * ((double)ypixel) / ((double)data->height);
			*zdisplay = 0.0;
			mbview_projectdisplay2ll(instance, *xdisplay, *ydisplay, *zdisplay, xlon, ylat);
			mbview_projectll2xyzgrid(instance, *xlon, *ylat, xgrid, ygrid, zdata);
			*found = true;
		}

		/* if not found and 3D use the best pick location found */
		if (!(*found) && foundsave) {
			*found = foundsave;
			*xgrid = xgridsave;
			*ygrid = ygridsave;
			*xlon = xlonsave;
			*ylat = ylatsave;
			*zdata = zdatasave;
			*xdisplay = xdisplaysave;
			*ydisplay = ydisplaysave;
			*zdisplay = zdisplaysave;
		}
	}

	const int status = MB_SUCCESS;

	if (mbv_verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       found:           %d\n", *found);
		fprintf(stderr, "dbg2       xgrid:           %f\n", *xgrid);
		fprintf(stderr, "dbg2       ygrid:           %f\n", *ygrid);
		fprintf(stderr, "dbg2       xlon:            %f\n", *xlon);
		fprintf(stderr, "dbg2       ylat:            %f\n", *ylat);
		fprintf(stderr, "dbg2       zdata:           %f\n", *zdata);
		fprintf(stderr, "dbg2       status:          %d\n", status);
	}

	return (status);
}

/*------------------------------------------------------------------------------*/
int mbview_findpointrez(size_t instance, int rez, int xpixel, int ypixel, int ijbounds[4], bool *found, double *xgrid,
                        double *ygrid, double *xlon, double *ylat, double *zdata, double *xdisplay, double *ydisplay,
                        double *zdisplay) {
	if (mbv_verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  MB-system Version %s\n", MB_VERSION);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       instance:         %zu\n", instance);
		fprintf(stderr, "dbg2       rez:              %d\n", rez);
		fprintf(stderr, "dbg2       xpixel:           %d\n", xpixel);
		fprintf(stderr, "dbg2       ypixel:           %d\n", ypixel);
		fprintf(stderr, "dbg2       ijbounds[0]:     %d\n", ijbounds[0]);
		fprintf(stderr, "dbg2       ijbounds[1]:     %d\n", ijbounds[1]);
		fprintf(stderr, "dbg2       ijbounds[2]:     %d\n", ijbounds[2]);
		fprintf(stderr, "dbg2       ijbounds[3]:     %d\n", ijbounds[3]);
	}

	struct mbview_world_struct *view = &(mbviews[instance]);
	struct mbview_struct *data = &(view->data);

  *xgrid = 0.0;
  *ygrid = 0.0;
  *xlon = 0.0;
  *ylat = 0.0;
  *zdata = 0.0;
  *xdisplay = 0.0;
  *ydisplay = 0.0;
  *zdisplay = 0.0;

	/* only plot if this view is still active */
	if (view->glx_init) {
/* make correct window current for OpenGL */
#ifdef MBV_DEBUG_GLX
		fprintf(stderr, "%s:%d:%s instance:%zu glXMakeCurrent(%p,%lu,%p)\n", __FILE__, __LINE__, __func__, instance,
		        view->dpy, XtWindow(view->glwmda), view->glx_context);
#endif
		glXMakeCurrent(view->dpy, XtWindow(view->glwmda), view->glx_context);

if (rez <= MBV_REZ_LOW)
//fprintf(stderr,"\nmbview_findpointrez: instance:%zu point:%d %d  bounds:%d %d %d %d\n",
//instance,xpixel,ypixel,ijbounds[0],ijbounds[1],ijbounds[2],ijbounds[3]);

#ifdef MBV_GET_GLX_ERRORS
		mbview_glerrorcheck(instance, __FILE__, __LINE__, __func__);
#endif

		/* apply projection if needed */
		if (!view->projected) {
			do_mbview_status("Projecting data...", instance);
			mbview_projectdata(instance);
		}

		/* set projection to 2D or 3D */
		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();
		view->right = MBV_OPENGL_WIDTH / view->size2d;
		view->left = -MBV_OPENGL_WIDTH / view->size2d;
		view->top = MBV_OPENGL_WIDTH / view->aspect_ratio / view->size2d;
		view->bottom = -MBV_OPENGL_WIDTH / view->aspect_ratio / view->size2d;
		if (data->display_mode == MBV_DISPLAY_2D) {
			glOrtho(view->left, view->right, view->bottom, view->top, MBV_OPENGL_ZMIN2D, MBV_OPENGL_ZMAX2D);
		}
		else {
			gluPerspective(40.0, view->aspect_ratio, 0.01 * MBV_OPENGL_WIDTH, 1000 * MBV_OPENGL_WIDTH);
		}

		/* set up translations */
		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();
		if (data->display_mode == MBV_DISPLAY_2D) {
			glTranslated(view->offset2d_x, view->offset2d_y, MBV_OPENGL_ZMIN2D);
		}
		else if (data->display_mode == MBV_DISPLAY_3D) {
			const float viewdistance = 0.48 * MBV_OPENGL_WIDTH * MBV_OPENGL_WIDTH / view->aspect_ratio;
			glTranslated(0.0, 0.0, -viewdistance + view->viewoffset3d_z);
			glRotated((float)(data->viewelevation3d - 90.0), 1.0, 0.0, 0.0);
			glRotated((float)(data->viewazimuth3d), 0.0, 1.0, 1.0);
			glTranslated(view->offset3d_x, view->offset3d_y, -viewdistance + view->offset3d_z);
			glRotated((float)(data->modelelevation3d - 90.0), 1.0, 0.0, 0.0);
			glRotated((float)(data->modelazimuth3d), 0.0, 0.0, 1.0);
		}

		/* set background color */
		glClearColor(1.0, 1.0, 1.0, 1.0);
		glClearDepth((GLclampd)(2000 * MBV_OPENGL_WIDTH));
		glDepthFunc(GL_LESS);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		/* enable depth test for 3D plots */
		if (data->display_mode == MBV_DISPLAY_3D)
			glEnable(GL_DEPTH_TEST);

		/* get bounds of interest in grid */
		const int imin = ijbounds[0];
		const int imax = ijbounds[1];
		const int ni = imax - imin + 1;
		const int jmin = ijbounds[2];
		const int jmax = ijbounds[3];
		const int nj = jmax - jmin + 1;

		/* set stride for looping over data */
		int stride;
		if (rez == MBV_REZ_FULL)
			stride = 1;
		else if (rez == MBV_REZ_HIGH)
			stride = MAX((int)ceil(((double)data->primary_n_columns) / ((double)data->hirez_dimension)),
			             (int)ceil(((double)data->primary_n_rows) / ((double)data->hirez_dimension)));
		else
			stride = MAX((int)ceil(((double)data->primary_n_columns) / ((double)data->lorez_dimension)),
			             (int)ceil(((double)data->primary_n_rows) / ((double)data->lorez_dimension)));

		/* get number of grid cells used in picking */
		const int npickx = (ni / stride);
		const int ipickstride = stride * (int)floor((npickx / MBV_PICK_DIVISION) + 1);
		const int npicky = (nj / stride);
		const int jpickstride = stride * (int)floor((npicky / MBV_PICK_DIVISION) + 1);
    const int pickstride = MAX(ipickstride, jpickstride);

/*if (rez <= MBV_REZ_LOW)
fprintf(stderr,"mbview_findpointrez: stride:%d npickx:%d npicky:%d ipickstride:%d jpickstride:%d\n",
stride, npickx, npicky, ipickstride, jpickstride);*/

		/* draw the triangles */
		glBegin(GL_TRIANGLES);
		for (int ii = imin; ii <= imax - stride; ii += stride) {
			for (int jj = jmin; jj <= jmax - stride; jj += stride) {
        const int i = MIN(ii, imax - 1);
        const int j = MIN(jj, jmax - 1);
				const int k = i * data->primary_n_rows + j;
				const int l = (i + stride) * data->primary_n_rows + j;
				const int m = i * data->primary_n_rows + j + stride;
				const int n = (i + stride) * data->primary_n_rows + j + stride;

				float rgb[3] = {
					(float)floor(((double)((i - imin) / pickstride))) / (MBV_PICK_DIVISION + 1.0),
					(float)floor(((double)((j - jmin) / pickstride))) / (MBV_PICK_DIVISION + 1.0),
					0.0f};
				if (data->primary_data[k] != data->primary_nodatavalue && data->primary_data[l] != data->primary_nodatavalue &&
				    data->primary_data[m] != data->primary_nodatavalue) {
					if (!(data->primary_stat_z[k / 8] & statmask[k % 8]))
						mbview_zscalegridpoint(instance, k);
					if (!(data->primary_stat_z[l / 8] & statmask[l % 8]))
						mbview_zscalegridpoint(instance, l);
					if (!(data->primary_stat_z[m / 8] & statmask[m % 8]))
						mbview_zscalegridpoint(instance, m);
					rgb[2] = 0.25f;
/*if (rez <= MBV_REZ_LOW)
fprintf(stderr,"triangleA:%d:%d %d:%d  nxy: %d %d %d klmn: %d %d %d %d  rgb: %.3f %.3f %.3f  xyz:  %f %f %f   %f %f %f   %f %f %f \n",
ii, i, jj, j, data->primary_n_columns, data->primary_n_rows, data->primary_nxy,
k, l, m, n, rgb[0], rgb[1], rgb[2],
data->primary_x[k], data->primary_y[k], data->primary_z[k],
data->primary_x[l], data->primary_y[l], data->primary_z[l],
data->primary_x[m], data->primary_y[m], data->primary_z[m]);*/
					glColor3f(rgb[0], rgb[1], rgb[2]);
					glVertex3f(data->primary_x[k], data->primary_y[k], data->primary_z[k]);
					glColor3f(rgb[0], rgb[1], rgb[2]);
					glVertex3f(data->primary_x[l], data->primary_y[l], data->primary_z[l]);
					glColor3f(rgb[0], rgb[1], rgb[2]);
					glVertex3f(data->primary_x[m], data->primary_y[m], data->primary_z[m]);
				}
				if (data->primary_data[l] != data->primary_nodatavalue && data->primary_data[m] != data->primary_nodatavalue &&
				    data->primary_data[n] != data->primary_nodatavalue) {
					if (!(data->primary_stat_z[l / 8] & statmask[l % 8]))
						mbview_zscalegridpoint(instance, l);
					if (!(data->primary_stat_z[m / 8] & statmask[m % 8]))
						mbview_zscalegridpoint(instance, m);
					if (!(data->primary_stat_z[n / 8] & statmask[n % 8]))
						mbview_zscalegridpoint(instance, n);
					rgb[2] = 0.75f;
/*if (rez <= MBV_REZ_LOW)
fprintf(stderr,"triangleB:%d:%d %d:%d  nxy: %d %d %d klmn: %d %d %d %d  rgb: %.3f %.3f %.3f  xyz:  %f %f %f   %f %f %f   %f %f %f \n",
ii, i, jj, j, data->primary_n_columns, data->primary_n_rows, data->primary_nxy,
k, l, m, n, rgb[0], rgb[1], rgb[2],
data->primary_x[l], data->primary_y[l], data->primary_z[l],
data->primary_x[n], data->primary_y[n], data->primary_z[n],
data->primary_x[m], data->primary_y[m], data->primary_z[m]);*/
					glColor3f(rgb[0], rgb[1], rgb[2]);
					glVertex3f(data->primary_x[l], data->primary_y[l], data->primary_z[l]);
					glColor3f(rgb[0], rgb[1], rgb[2]);
					glVertex3f(data->primary_x[n], data->primary_y[n], data->primary_z[n]);
					glColor3f(rgb[0], rgb[1], rgb[2]);
					glVertex3f(data->primary_x[m], data->primary_y[m], data->primary_z[m]);
				}
			}
		}
		glEnd();

		/* flush opengl buffers */
		glFlush();

		/* make sure depth test is off */
		glDisable(GL_DEPTH_TEST);

		/* now read the color at the pick point */
		glReadBuffer(GL_BACK);
		float rgba[4];
/*for (int iii=0; iii<1000; iii++) {
for (int jjj=0; jjj<1000; jjj++) {
glReadPixels(iii, jjj, 1, 1, GL_RGBA, GL_FLOAT, rgba);
fprintf(stderr, "pixel %4d %4d  rgba %.3f %.3f %.3f %.3f\n",
iii, jjj, rgba[0], rgba[1], rgba[2], rgba[3]);
}
}*/


		glReadPixels(xpixel, ypixel, 1, 1, GL_RGBA, GL_FLOAT, rgba);
		glReadBuffer(GL_FRONT);

/*if (rez <= MBV_REZ_LOW)
fprintf(stderr, "%s:%d:%s: rgba: %f %f %f %f\n",
__FILE__, __LINE__, __FUNCTION__, rgba[0], rgba[1], rgba[2], rgba[3]);*/
		/* calculate pick location */
		if (rgba[0] != 1.0 && rgba[1] != 1.0 && (rgba[2] > 0.2 && rgba[2] < 0.8)) {
			*found = true;

			const int i = imin + pickstride * ((int)rint((MBV_PICK_DIVISION + 1.0) * rgba[0]));
			const int j = jmin + pickstride * ((int)rint((MBV_PICK_DIVISION + 1.0) * rgba[1]));
			const int k = i * data->primary_n_rows + j;
			const int l = (i + stride) * data->primary_n_rows + j;
			const int m = i * data->primary_n_rows + j + stride;
			const int n = (i + stride) * data->primary_n_rows + j + stride;
			if (rint((MBV_PICK_DIVISION + 1.0) * rgba[2]) == (MBV_PICK_DIVISION + 1.0) / 4.0) {
				*xgrid = data->primary_xmin + (3 * i + stride) * data->primary_dx / 3.0;
				*ygrid = data->primary_ymin + (3 * j + stride) * data->primary_dy / 3.0;
				*zdata = (data->primary_data[k] + data->primary_data[l] + data->primary_data[m]) / 3.0;
			}
			else {
				*xgrid = data->primary_xmin + (3 * i + 2 * stride) * data->primary_dx / 3.0;
				*ygrid = data->primary_ymin + (3 * j + 2 * stride) * data->primary_dy / 3.0;
				*zdata = (data->primary_data[l] + data->primary_data[n] + data->primary_data[m]) / 3.0;
			}
/*if (rez <= MBV_REZ_LOW)
fprintf(stderr,"pickrez:%d %d   rgb: %f %f %f %f   i:%d j:%d\n",
xpixel,ypixel, rgba[0], rgba[1], rgba[2], rgba[3], i, j);*/

			/* project grid positions to geographic and display coordinates */
			mbview_projectforward(instance, true, *xgrid, *ygrid, *zdata, xlon, ylat, xdisplay, ydisplay, zdisplay);

/*if (rez <= MBV_REZ_LOW)
fprintf(stderr," pickrez: grid: %f %f %f     lonlat: %f %f display: %f %f %f\n",
*xgrid, *ygrid, *zdata, *xlon, *ylat, *xdisplay, *ydisplay, *zdisplay);*/

			/* reset ijbounds */
			if (pickstride == 1) {
			  ijbounds[0] = i;
				ijbounds[1] = i;
			}
			else {
			  ijbounds[0] = MAX(i - pickstride, 0);
				ijbounds[1] = MIN(i + 2 * pickstride - 1, data->primary_n_columns - 1);
			}
			if (pickstride == 1) {
			  ijbounds[2] = j;
				ijbounds[3] = j;
			}
			else {
			  ijbounds[2] = MAX(j - pickstride, 0);
				ijbounds[3] = MIN(j + 2 * pickstride - 1, data->primary_n_rows - 1);
			}
		}

		else {
			*found = false;
			/*fprintf(stderr,"pickrez bad pick!!:%d %d   rgba: %f %f %f %f\n",
			xpixel,ypixel, rgba[0], rgba[1], rgba[2], rgba[3]);*/
		}

#ifdef MBV_GET_GLX_ERRORS
		mbview_glerrorcheck(instance, __FILE__, __LINE__, __func__);
#endif
	}

	const int status = MB_SUCCESS;

	if (mbv_verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       ijbounds[0]:     %d\n", ijbounds[0]);
		fprintf(stderr, "dbg2       ijbounds[1]:     %d\n", ijbounds[1]);
		fprintf(stderr, "dbg2       ijbounds[2]:     %d\n", ijbounds[2]);
		fprintf(stderr, "dbg2       ijbounds[3]:     %d\n", ijbounds[3]);
		fprintf(stderr, "dbg2       found:           %d\n", *found);
		fprintf(stderr, "dbg2       xgrid:           %f\n", *xgrid);
		fprintf(stderr, "dbg2       ygrid:           %f\n", *ygrid);
		fprintf(stderr, "dbg2       xlon:            %f\n", *xlon);
		fprintf(stderr, "dbg2       ylat:            %f\n", *ylat);
		fprintf(stderr, "dbg2       zdata:           %f\n", *zdata);
		fprintf(stderr, "dbg2       status:          %d\n", status);
	}

	return (status);
}

/*------------------------------------------------------------------------------*/
int mbview_viewbounds(size_t instance) {
	if (mbv_verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  MB-system Version %s\n", MB_VERSION);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       instance:         %zu\n", instance);
	}

	struct mbview_world_struct *view = &(mbviews[instance]);
	struct mbview_struct *data = &(view->data);

	/* only plot if this view is still active */
	if (view->glx_init) {
/* make correct window current for OpenGL */
#ifdef MBV_DEBUG_GLX
		fprintf(stderr, "%s:%d:%s instance:%zu glXMakeCurrent(%p,%lu,%p)\n", __FILE__, __LINE__, __func__, instance,
		        view->dpy, XtWindow(view->glwmda), view->glx_context);
#endif
		glXMakeCurrent(view->dpy, XtWindow(view->glwmda), view->glx_context);

#ifdef MBV_GET_GLX_ERRORS
		mbview_glerrorcheck(instance, __FILE__, __LINE__, __func__);
#endif

		/* apply projection if needed */
		if (!view->projected) {
			do_mbview_status("Projecting data...", instance);
			mbview_projectdata(instance);
		}

		// float left2d, right2d;
		// float bottom2d, top2d;
		bool found;
		float rgba[4];
		int ijbounds[4];

		/* 2D case doesn't require plotting */
		if (data->display_mode == MBV_DISPLAY_2D) {
			/*fprintf(stderr,"2D GL bounds: %f %f %f %f\n",
			view->left, view->right, view->bottom, view->top);
			fprintf(stderr,"2D GL offsets: %f %f\n",
			view->offset2d_x, view->offset2d_y);
			fprintf(stderr,"2D primary grid corners: \n");
			i = 0; j = 0; k = i * data->primary_n_rows + j;
			fprintf(stderr,"LL:%f %f\n",
			data->primary_x[k],data->primary_y[k]);
			i = data->primary_n_columns - 1; j = data->primary_n_rows - 1; k = i * data->primary_n_rows + j;
			fprintf(stderr,"LR:%f %f\n",
			data->primary_x[k],data->primary_y[k]);
			i = 0; j = 0; k = i * data->primary_n_rows + j;
			fprintf(stderr,"UL:%f %f\n",
			data->primary_x[k],data->primary_y[k]);
			i = data->primary_n_columns - 1; j = data->primary_n_rows - 1; k = i * data->primary_n_rows + j;
			fprintf(stderr,"UR:%f %f\n",
			data->primary_x[k],data->primary_y[k]);*/

			/* set stride for looping over data using rule for low rez plotting */
			const int stride = MAX((int)ceil(((double)data->primary_n_columns) / ((double)data->lorez_dimension)),
			             (int)ceil(((double)data->primary_n_rows) / ((double)data->lorez_dimension)));

			/* get 2D view bounds */
			const float left2d = view->left - view->offset2d_x;
			const float right2d = view->right - view->offset2d_x;
			const float bottom2d = view->bottom - view->offset2d_y;
			const float top2d = view->top - view->offset2d_y;
			found = false;
			data->viewbounds[0] = 0;
			data->viewbounds[1] = data->primary_n_columns - 1;
			data->viewbounds[2] = 0;
			data->viewbounds[3] = data->primary_n_rows - 1;
			for (int i = 0; i < data->primary_n_columns; i += stride) {
				for (int j = 0; j < data->primary_n_rows; j += stride) {
					const int k = i * data->primary_n_rows + j;
					if (data->primary_data[k] != data->primary_nodatavalue && data->primary_x[k] >= left2d &&
					    data->primary_x[k] <= right2d && data->primary_y[k] >= bottom2d && data->primary_y[k] <= top2d) {
            if (found) {
							data->viewbounds[0] = MIN(i, data->viewbounds[0]);
							data->viewbounds[1] = MAX(i + stride, data->viewbounds[1]);
							data->viewbounds[2] = MIN(j, data->viewbounds[2]);
							data->viewbounds[3] = MAX(j + stride, data->viewbounds[3]);
            }
						else {
							data->viewbounds[0] = i;
							data->viewbounds[1] = i + stride;
							data->viewbounds[2] = j;
							data->viewbounds[3] = j + stride;
							found = true;
						}
					}
				}
			}
			for (int i = 0; i < data->primary_n_columns; i += data->primary_n_columns - 1) {
				for (int j = 0; j < data->primary_n_rows; j += data->primary_n_rows - 1) {
					const int k = i * data->primary_n_rows + j;
					if (data->primary_data[k] != data->primary_nodatavalue && data->primary_x[k] >= left2d &&
					    data->primary_x[k] <= right2d && data->primary_y[k] >= bottom2d && data->primary_y[k] <= top2d) {
						if (found) {
							data->viewbounds[0] = MIN(i, data->viewbounds[0]);
							data->viewbounds[1] = MAX(i + stride, data->viewbounds[1]);
							data->viewbounds[2] = MIN(j, data->viewbounds[2]);
							data->viewbounds[3] = MAX(j + stride, data->viewbounds[3]);
						}
						else {
							data->viewbounds[0] = i;
							data->viewbounds[1] = i + stride;
							data->viewbounds[2] = j;
							data->viewbounds[3] = j + stride;
							found = true;
						}
					}
				}
			}
			data->viewbounds[0] = MAX(data->viewbounds[0] - stride, 0);
			data->viewbounds[1] = MIN(data->viewbounds[1] + stride, data->primary_n_columns - 1);
			data->viewbounds[2] = MAX(data->viewbounds[2] - stride, 0);
			data->viewbounds[3] = MIN(data->viewbounds[3] + stride, data->primary_n_rows - 1);
		}

		/* 3D case requires plotting */
		else {
			/* set projection to 2D or 3D */
			glMatrixMode(GL_PROJECTION);
			glLoadIdentity();
			view->right = MBV_OPENGL_WIDTH / view->size2d;
			view->left = -MBV_OPENGL_WIDTH / view->size2d;
			view->top = MBV_OPENGL_WIDTH / view->aspect_ratio / view->size2d;
			view->bottom = -MBV_OPENGL_WIDTH / view->aspect_ratio / view->size2d;
			if (data->display_mode == MBV_DISPLAY_2D) {
				glOrtho(view->left, view->right, view->bottom, view->top, MBV_OPENGL_ZMIN2D, MBV_OPENGL_ZMAX2D);
			}
			else {
				gluPerspective(40.0, view->aspect_ratio, 0.01 * MBV_OPENGL_WIDTH, 1000 * MBV_OPENGL_WIDTH);
			}

			/* set up translations */
			glMatrixMode(GL_MODELVIEW);
			glLoadIdentity();
			if (data->display_mode == MBV_DISPLAY_2D) {
				glTranslated(view->offset2d_x, view->offset2d_y, MBV_OPENGL_ZMIN2D);
			}
			else if (data->display_mode == MBV_DISPLAY_3D) {
				const float viewdistance = 0.48 * MBV_OPENGL_WIDTH * MBV_OPENGL_WIDTH / view->aspect_ratio;
				glTranslated(0.0, 0.0, -viewdistance + view->viewoffset3d_z);
				glRotated((float)(data->viewelevation3d - 90.0), 1.0, 0.0, 0.0);
				glRotated((float)(data->viewazimuth3d), 0.0, 1.0, 1.0);
				glTranslated(view->offset3d_x, view->offset3d_y, -viewdistance + view->offset3d_z);
				glRotated((float)(data->modelelevation3d - 90.0), 1.0, 0.0, 0.0);
				glRotated((float)(data->modelazimuth3d), 0.0, 0.0, 1.0);
			}

			/* set background color */
			glClearColor(1.0, 1.0, 1.0, 1.0);
			glClearDepth((GLclampd)(2000 * MBV_OPENGL_WIDTH));
			glDepthFunc(GL_LESS);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

			/* enable depth test for 3D plots */
			if (data->display_mode == MBV_DISPLAY_3D)
				glEnable(GL_DEPTH_TEST);

			/* set stride for looping over data using rule for low rez plotting */
			const int stride = MAX((int)ceil(((double)data->primary_n_columns) / ((double)data->lorez_dimension)),
			             (int)ceil(((double)data->primary_n_rows) / ((double)data->lorez_dimension)));

			/* get number of grid cells used in picking */
			const int npickx = (data->primary_n_columns / stride);
			const int ipickstride = stride * (int)floor((npickx / MBV_PICK_DIVISION) + 1);
			const int npicky = (data->primary_n_rows / stride);
			const int jpickstride = stride * (int)floor((npicky / MBV_PICK_DIVISION) + 1);

			/*fprintf(stderr,"mbview_viewbounds: stride:%d npickx:%d npicky:%d ipickstride:%d jpickstride:%d\n",
			stride, npickx, npicky, ipickstride, jpickstride);*/

			/* draw the triangles */
			glBegin(GL_TRIANGLES);
			for (int i = 0; i < data->primary_n_columns - stride; i += stride) {
				for (int j = 0; j < data->primary_n_rows - stride; j += stride) {
					const int k = i * data->primary_n_rows + j;
					const int l = (i + stride) * data->primary_n_rows + j;
					const int m = i * data->primary_n_rows + j + stride;
					const int n = (i + stride) * data->primary_n_rows + j + stride;

					float rgb[3];
					rgb[0] = (float)floor(((double)(i / ipickstride))) / (MBV_PICK_DIVISION + 1.0);
					rgb[1] = (float)floor(((double)(j / jpickstride))) / (MBV_PICK_DIVISION + 1.0);
					if (data->primary_data[k] != data->primary_nodatavalue &&
					    data->primary_data[l] != data->primary_nodatavalue &&
					    data->primary_data[m] != data->primary_nodatavalue) {
						rgb[2] = 0.25;
						/*fprintf(stderr,"triangle:%d %d   rgb: %f %f %f\n",
						i,j, rgb[0], rgb[1], rgb[2]);*/
						glColor3f(rgb[0], rgb[1], rgb[2]);
						glVertex3f(data->primary_x[k], data->primary_y[k], data->primary_z[k]);
						glColor3f(rgb[0], rgb[1], rgb[2]);
						glVertex3f(data->primary_x[l], data->primary_y[l], data->primary_z[l]);
						glColor3f(rgb[0], rgb[1], rgb[2]);
						glVertex3f(data->primary_x[m], data->primary_y[m], data->primary_z[m]);
					}
					if (data->primary_data[l] != data->primary_nodatavalue &&
					    data->primary_data[m] != data->primary_nodatavalue &&
					    data->primary_data[n] != data->primary_nodatavalue) {
						rgb[2] = 0.75;
						/*fprintf(stderr,"triangle:%d %d   rgb: %f %f %f\n",
						i,j, rgb[0], rgb[1], rgb[2]);*/
						glColor3f(rgb[0], rgb[1], rgb[2]);
						glVertex3f(data->primary_x[l], data->primary_y[l], data->primary_z[l]);
						glColor3f(rgb[0], rgb[1], rgb[2]);
						glVertex3f(data->primary_x[n], data->primary_y[n], data->primary_z[n]);
						glColor3f(rgb[0], rgb[1], rgb[2]);
						glVertex3f(data->primary_x[m], data->primary_y[m], data->primary_z[m]);
					}
				}
			}
			glEnd();

			/* flush opengl buffers */
			glFlush();

			/* make sure depth test is off */
			glDisable(GL_DEPTH_TEST);

			/* now read the color at a number of points in the screen */
			glReadBuffer(GL_BACK);
			found = false;
			data->viewbounds[0] = 0;
			data->viewbounds[1] = data->primary_n_columns - 1;
			data->viewbounds[2] = 0;
			data->viewbounds[3] = data->primary_n_rows - 1;
			const int iscreenstride = data->width / 20;
			const int jscreenstride = data->height / 20;
			for (int xpixel = 0; xpixel < data->width; xpixel += iscreenstride) {
				for (int ypixel = 0; ypixel < data->height; ypixel += jscreenstride) {
					glReadPixels(xpixel, ypixel, 1, 1, GL_RGBA, GL_FLOAT, rgba);
					/*fprintf(stderr,"xpixel:%d ypixel:%d rgba: %f %f %f %f\n",
					xpixel,ypixel, rgba[0], rgba[1], rgba[2], rgba[3]);*/
					if (rgba[0] != 1.0 && rgba[1] != 1.0) {
						const int i = ipickstride * ((int)rint((MBV_PICK_DIVISION + 1.0) * rgba[0]));
						const int j = jpickstride * ((int)rint((MBV_PICK_DIVISION + 1.0) * rgba[1]));
						if (found) {
							data->viewbounds[0] = MIN(i, data->viewbounds[0]);
							data->viewbounds[1] = MAX(i + stride, data->viewbounds[1]);
							data->viewbounds[2] = MIN(j, data->viewbounds[2]);
							data->viewbounds[3] = MAX(j + stride, data->viewbounds[3]);
						}
						else {
							data->viewbounds[0] = i;
							data->viewbounds[1] = i + stride;
							data->viewbounds[2] = j;
							data->viewbounds[3] = j + stride;
							found = true;
						}
					}
				}
			}
			for (int xpixel = 0; xpixel < data->width; xpixel += data->width - 1) {
				for (int ypixel = 0; ypixel < data->height; ypixel += data->height - 1) {
					glReadPixels(xpixel, ypixel, 1, 1, GL_RGBA, GL_FLOAT, rgba);
					/*fprintf(stderr,"xpixel:%d ypixel:%d rgba: %f %f %f %f\n",
					    xpixel,ypixel, rgba[0], rgba[1], rgba[2], rgba[3]);*/
					if (rgba[0] != 1.0 && rgba[1] != 1.0) {
						const int i = ipickstride * ((int)rint((MBV_PICK_DIVISION + 1.0) * rgba[0]));
						const int j = jpickstride * ((int)rint((MBV_PICK_DIVISION + 1.0) * rgba[1]));
						ijbounds[0] = i;
						ijbounds[2] = j;
						if (ipickstride == 1) {
							ijbounds[1] = i;
							ijbounds[3] = j;
						}
						else {
							ijbounds[1] = MIN(i + 2 * ipickstride - 1, data->primary_n_columns - 1);
							ijbounds[3] = MIN(j + 2 * jpickstride - 1, data->primary_n_rows - 1);
						}
						if (found) {
							data->viewbounds[0] = MIN(ijbounds[0], data->viewbounds[0]);
							data->viewbounds[1] = MAX(ijbounds[1], data->viewbounds[1]);
							data->viewbounds[2] = MIN(ijbounds[2], data->viewbounds[2]);
							data->viewbounds[3] = MAX(ijbounds[3], data->viewbounds[3]);
						}
						else {
							data->viewbounds[0] = ijbounds[0];
							data->viewbounds[1] = ijbounds[1];
							data->viewbounds[2] = ijbounds[2];
							data->viewbounds[3] = ijbounds[3];
							found = true;
						}
					}
				}
			}
			data->viewbounds[0] = MAX(data->viewbounds[0] - stride, 0);
			data->viewbounds[1] = MIN(data->viewbounds[1] + stride, data->primary_n_columns - 1);
			data->viewbounds[2] = MAX(data->viewbounds[2] - stride, 0);
			data->viewbounds[3] = MIN(data->viewbounds[3] + stride, data->primary_n_rows - 1);

			/* reset buffer mode */
			glReadBuffer(GL_FRONT);
		}
/*fprintf(stderr,"data->viewbounds: %d %d %d %d\n",
data->viewbounds[0], data->viewbounds[1], data->viewbounds[2], data->viewbounds[3]);*/

#ifdef MBV_GET_GLX_ERRORS
		mbview_glerrorcheck(instance, __FILE__, __LINE__, __func__);
#endif
	}

	const int status = MB_SUCCESS;

	if (mbv_verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       viewbounds[0]:   %d\n", data->viewbounds[0]);
		fprintf(stderr, "dbg2       viewbounds[1]:   %d\n", data->viewbounds[1]);
		fprintf(stderr, "dbg2       viewbounds[2]:   %d\n", data->viewbounds[2]);
		fprintf(stderr, "dbg2       viewbounds[3]:   %d\n", data->viewbounds[3]);
		fprintf(stderr, "dbg2       status:          %d\n", status);
	}

	return (status);
}
/*------------------------------------------------------------------------------*/
int mbview_drapesegment(size_t instance, struct mbview_linesegment_struct *seg) {
	if (mbv_verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  MB-system Version %s\n", MB_VERSION);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       instance:         %zu\n", instance);
		fprintf(stderr, "dbg2       seg:              %p\n", seg);
		fprintf(stderr, "dbg2       seg->endpoints:\n");
		fprintf(stderr, "dbg2            xgrid[0]:    %f\n", seg->endpoints[0].xgrid);
		fprintf(stderr, "dbg2            ygrid[0]:    %f\n", seg->endpoints[0].ygrid);
		fprintf(stderr, "dbg2            xlon[0]:     %f\n", seg->endpoints[0].xlon);
		fprintf(stderr, "dbg2            ylat[0]:     %f\n", seg->endpoints[0].ylat);
		fprintf(stderr, "dbg2            xgrid[1]:    %f\n", seg->endpoints[1].xgrid);
		fprintf(stderr, "dbg2            ygrid[1]:    %f\n", seg->endpoints[1].ygrid);
		fprintf(stderr, "dbg2            xlon[1]:     %f\n", seg->endpoints[1].xlon);
		fprintf(stderr, "dbg2            ylat[1]:     %f\n", seg->endpoints[1].ylat);
	}

	struct mbview_world_struct *view = &(mbviews[instance]);
	struct mbview_struct *data = &(view->data);

	int status = MB_SUCCESS;

	/* only plot if this view is still active */
	if (view->glx_init) {

		/* if spheroid dipslay project on great circle arc */
		if (data->display_projection_mode == MBV_PROJECTION_SPHEROID) {
			status = mbview_drapesegment_gc(instance, seg);
		}

		/* else project on straight lines in grid projection */
		else {
			status = mbview_drapesegment_grid(instance, seg);
		}
	}

	if (mbv_verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:          %d\n", status);
		fprintf(stderr, "dbg2       seg->endpoints:\n");
		fprintf(stderr, "dbg2            xgrid[0]:    %f\n", seg->endpoints[0].xgrid);
		fprintf(stderr, "dbg2            ygrid[0]:    %f\n", seg->endpoints[0].ygrid);
		fprintf(stderr, "dbg2            xlon[0]:     %f\n", seg->endpoints[0].xlon);
		fprintf(stderr, "dbg2            ylat[0]:     %f\n", seg->endpoints[0].ylat);
		fprintf(stderr, "dbg2            xgrid[1]:    %f\n", seg->endpoints[1].xgrid);
		fprintf(stderr, "dbg2            ygrid[1]:    %f\n", seg->endpoints[1].ygrid);
		fprintf(stderr, "dbg2            xlon[1]:     %f\n", seg->endpoints[1].xlon);
		fprintf(stderr, "dbg2            ylat[1]:     %f\n", seg->endpoints[1].ylat);
		fprintf(stderr, "dbg2       seg->nls:        %d\n", seg->nls);
		fprintf(stderr, "dbg2       seg->nls_alloc:  %d\n", seg->nls_alloc);
		fprintf(stderr, "dbg2       seg->lspoints:\n");
		for (int i = 0; i < seg->nls; i++) {
			fprintf(stderr, "dbg2         point[%4d]:    %f %f %f  %f %f  %f %f %f\n", i, seg->lspoints[i].xgrid,
			        seg->lspoints[i].ygrid, seg->lspoints[i].zdata, seg->lspoints[i].xlon, seg->lspoints[i].ylat,
			        seg->lspoints[i].xdisplay, seg->lspoints[i].ydisplay, seg->lspoints[i].zdisplay);
		}
	}

	return (status);
}

/*------------------------------------------------------------------------------*/
int mbview_drapesegment_gc(size_t instance, struct mbview_linesegment_struct *seg) {
	if (mbv_verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  MB-system Version %s\n", MB_VERSION);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       instance:         %zu\n", instance);
		fprintf(stderr, "dbg2       seg:              %p\n", seg);
		fprintf(stderr, "dbg2       seg->endpoints:\n");
		fprintf(stderr, "dbg2            xgrid[0]:    %f\n", seg->endpoints[0].xgrid);
		fprintf(stderr, "dbg2            ygrid[0]:    %f\n", seg->endpoints[0].ygrid);
		fprintf(stderr, "dbg2            xlon[0]:     %f\n", seg->endpoints[0].xlon);
		fprintf(stderr, "dbg2            ylat[0]:     %f\n", seg->endpoints[0].ylat);
		fprintf(stderr, "dbg2            xgrid[1]:    %f\n", seg->endpoints[1].xgrid);
		fprintf(stderr, "dbg2            ygrid[1]:    %f\n", seg->endpoints[1].ygrid);
		fprintf(stderr, "dbg2            xlon[1]:     %f\n", seg->endpoints[1].xlon);
		fprintf(stderr, "dbg2            ylat[1]:     %f\n", seg->endpoints[1].ylat);
	}

	struct mbview_world_struct *view = &(mbviews[instance]);
	struct mbview_struct *data = &(view->data);

	bool done = false;

	/* check if the contour offset needs to be applied in a global spherical direction or just up */
	const bool global =
		data->display_projection_mode == MBV_PROJECTION_SPHEROID &&
		view->sphere_refx == 0.0 && view->sphere_refy == 0.0 &&
		view->sphere_refz == 0.0;
	const double offset_factor =
		10.0 *
		(global
		 ? MBV_OPENGL_3D_CONTOUR_OFFSET / (view->scale * MBV_SPHEROID_RADIUS)
		 : MBV_OPENGL_3D_CONTOUR_OFFSET);

	double xlon1, ylat1;
	double xlon2, ylat2;

	/* get half characteristic distance between grid points
	    from center of primary grid */
	{
		const int i = data->primary_n_columns / 2;
		const int j = data->primary_n_rows / 2;
		mbview_projectgrid2ll(instance, (double)(data->primary_xmin + i * data->primary_dx),
	                      (double)(data->primary_ymin + j * data->primary_dy), &xlon1, &ylat1);
		mbview_projectgrid2ll(instance, (double)(data->primary_xmin + (i + 1) * data->primary_dx),
	                      (double)(data->primary_ymin + (j + 1) * data->primary_dy), &xlon2, &ylat2);
	}
	double dsegbearing;
	double dsegdist;
	mbview_greatcircle_distbearing(instance, xlon1, ylat1, xlon2, ylat2, &dsegbearing, &dsegdist);

	/* get number of preliminary points along the segment */
	double segbearing;
	double segdist;
	mbview_greatcircle_distbearing(instance, seg->endpoints[0].xlon, seg->endpoints[0].ylat, seg->endpoints[1].xlon,
	                               seg->endpoints[1].ylat, &segbearing, &segdist);
	const int nsegpoint = MAX(((int)((segdist / dsegdist) + 1)), 2);

	int status = MB_SUCCESS;

	/* no need to fill in if the segment doesn't cross grid boundaries */
	if (nsegpoint <= 2) {
		done = true;
		seg->nls = 0;
		seg->nls_alloc = 0;
	} else {
		/* get the points along the great circle arc */
		/* get effective distance between points along great circle */
		dsegdist = segdist / (nsegpoint - 1);

		/* allocate segment points */
		seg->nls_alloc = nsegpoint;
		int error = MB_ERROR_NO_ERROR;
		status = mb_reallocd(mbv_verbose, __FILE__, __LINE__, seg->nls_alloc * sizeof(struct mbview_point_struct),
		                     (void **)&(seg->lspoints), &error);
		if (status == MB_FAILURE) {
			done = true;
			seg->nls_alloc = 0;
			seg->nls = 0;
		}
	}

	/* now calculate points along great circle arc */
	if (seg->nls_alloc > 1 && !done) {
		/* put begin point in list */
		seg->nls = 0;
		seg->lspoints[seg->nls].xgrid = seg->endpoints[0].xgrid;
		seg->lspoints[seg->nls].ygrid = seg->endpoints[0].ygrid;
		seg->lspoints[seg->nls].zdata = seg->endpoints[0].zdata;
		seg->lspoints[seg->nls].xlon = seg->endpoints[0].xlon;
		seg->lspoints[seg->nls].ylat = seg->endpoints[0].ylat;
		seg->nls++;

		for (int i = 1; i < nsegpoint - 1; i++) {
			mbview_greatcircle_endposition(instance, seg->lspoints[0].xlon, seg->lspoints[0].ylat, segbearing,
			                               (double)(i * dsegdist), &(seg->lspoints[seg->nls].xlon),
			                               &(seg->lspoints[seg->nls].ylat)),
			    status = mbview_projectll2xyzgrid(instance, seg->lspoints[seg->nls].xlon, seg->lspoints[seg->nls].ylat,
			                                      &(seg->lspoints[seg->nls].xgrid), &(seg->lspoints[seg->nls].ygrid),
			                                      &(seg->lspoints[seg->nls].zdata));
			if (status == MB_SUCCESS) {
				seg->nls++;
			}
		}

		/* put end point in list */
		seg->lspoints[seg->nls].xgrid = seg->endpoints[1].xgrid;
		seg->lspoints[seg->nls].ygrid = seg->endpoints[1].ygrid;
		seg->lspoints[seg->nls].zdata = seg->endpoints[1].zdata;
		seg->lspoints[seg->nls].xlon = seg->endpoints[1].xlon;
		seg->lspoints[seg->nls].ylat = seg->endpoints[1].ylat;
		seg->nls++;

		/* now calculate rest of point values */
		for (int icnt = 0; icnt < seg->nls; icnt++) {
			mbview_projectll2display(instance, seg->lspoints[icnt].xlon, seg->lspoints[icnt].ylat, seg->lspoints[icnt].zdata,
			                         &(seg->lspoints[icnt].xdisplay), &(seg->lspoints[icnt].ydisplay),
			                         &(seg->lspoints[icnt].zdisplay));
			if (data->display_projection_mode != MBV_PROJECTION_SPHEROID) {
				seg->lspoints[icnt].zdisplay += offset_factor;
			}
			else if (global) {
				seg->lspoints[icnt].xdisplay += seg->lspoints[icnt].xdisplay * offset_factor;
				seg->lspoints[icnt].ydisplay += seg->lspoints[icnt].ydisplay * offset_factor;
				seg->lspoints[icnt].zdisplay += seg->lspoints[icnt].zdisplay * offset_factor;
			}
			else {
				seg->lspoints[icnt].zdisplay += offset_factor;
			}
		}
	}

	if (mbv_verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:          %d\n", status);
		fprintf(stderr, "dbg2       seg->endpoints:\n");
		fprintf(stderr, "dbg2            xgrid[0]:    %f\n", seg->endpoints[0].xgrid);
		fprintf(stderr, "dbg2            ygrid[0]:    %f\n", seg->endpoints[0].ygrid);
		fprintf(stderr, "dbg2            xlon[0]:     %f\n", seg->endpoints[0].xlon);
		fprintf(stderr, "dbg2            ylat[0]:     %f\n", seg->endpoints[0].ylat);
		fprintf(stderr, "dbg2            xgrid[1]:    %f\n", seg->endpoints[1].xgrid);
		fprintf(stderr, "dbg2            ygrid[1]:    %f\n", seg->endpoints[1].ygrid);
		fprintf(stderr, "dbg2            xlon[1]:     %f\n", seg->endpoints[1].xlon);
		fprintf(stderr, "dbg2            ylat[1]:     %f\n", seg->endpoints[1].ylat);
		fprintf(stderr, "dbg2       seg->nls:        %d\n", seg->nls);
		fprintf(stderr, "dbg2       seg->nls_alloc:  %d\n", seg->nls_alloc);
		fprintf(stderr, "dbg2       seg->lspoints:\n");
		for (int i = 0; i < seg->nls; i++) {
			fprintf(stderr, "dbg2         point[%4d]:    %f %f %f  %f %f  %f %f %f\n", i, seg->lspoints[i].xgrid,
			        seg->lspoints[i].ygrid, seg->lspoints[i].zdata, seg->lspoints[i].xlon, seg->lspoints[i].ylat,
			        seg->lspoints[i].xdisplay, seg->lspoints[i].ydisplay, seg->lspoints[i].zdisplay);
		}
	}

	return (status);
}

/*------------------------------------------------------------------------------*/
int mbview_drapesegment_grid(size_t instance, struct mbview_linesegment_struct *seg) {
	if (mbv_verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  MB-system Version %s\n", MB_VERSION);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       instance:         %zu\n", instance);
		fprintf(stderr, "dbg2       seg:              %p\n", seg);
		fprintf(stderr, "dbg2       seg->endpoints:\n");
		fprintf(stderr, "dbg2            xgrid[0]:    %f\n", seg->endpoints[0].xgrid);
		fprintf(stderr, "dbg2            ygrid[0]:    %f\n", seg->endpoints[0].ygrid);
		fprintf(stderr, "dbg2            xgrid[1]:    %f\n", seg->endpoints[1].xgrid);
		fprintf(stderr, "dbg2            ygrid[1]:    %f\n", seg->endpoints[1].ygrid);
	}

	struct mbview_world_struct *view = &(mbviews[instance]);
	struct mbview_struct *data = &(view->data);

	/* check if the contour offset needs to be applied in a global spherical direction or just up */
	const bool global =
		data->display_projection_mode == MBV_PROJECTION_SPHEROID && view->sphere_refx == 0.0 &&
		view->sphere_refy == 0.0 && view->sphere_refz == 0.0;
	const double offset_factor =
		10.0 *
		(global
		 ? MBV_OPENGL_3D_CONTOUR_OFFSET / (view->scale * MBV_SPHEROID_RADIUS)
		 : MBV_OPENGL_3D_CONTOUR_OFFSET);

	/* figure out how many points to calculate along the segment */
	int istart = (int)((seg->endpoints[0].xgrid - data->primary_xmin) / data->primary_dx);
	int iend = (int)((seg->endpoints[1].xgrid - data->primary_xmin) / data->primary_dx);
	int jstart = (int)((seg->endpoints[0].ygrid - data->primary_ymin) / data->primary_dy);
	int jend = (int)((seg->endpoints[1].ygrid - data->primary_ymin) / data->primary_dy);

	int status = MB_SUCCESS;
	int error = MB_ERROR_NO_ERROR;
	int ni, nj;
	bool done = false;
	int iadd, jadd;

	/* no need to fill in if the segment doesn't cross grid boundaries */
	if (istart == iend && jstart == jend) {
		done = true;
		seg->nls = 0;
	} else {
		// allocate space for the array of points
		if (iend > istart) {
			ni = iend - istart;
			iadd = 1;
			istart++;
			// iend++;
		} else {
			ni = istart - iend;
			iadd = -1;
		}

		if (jend > jstart) {
			nj = jend - jstart;
			jadd = 1;
			jstart++;
			// jend++;
		} else {
			nj = jstart - jend;
			jadd = -1;
		}
		if ((ni + nj + 2) > seg->nls_alloc) {
			seg->nls_alloc = (ni + nj + 2);
			status = mb_reallocd(mbv_verbose, __FILE__, __LINE__, seg->nls_alloc * sizeof(struct mbview_point_struct),
			                     (void **)&(seg->lspoints), &error);
			if (status == MB_FAILURE) {
				done = true;
				seg->nls_alloc = 0;
			}
		}
	}

	/* if points needed and space allocated do it */
	if (!done && ni + nj > 0) {
		/* put begin point in list */
		seg->nls = 0;
		seg->lspoints[seg->nls].xgrid = seg->endpoints[0].xgrid;
		seg->lspoints[seg->nls].ygrid = seg->endpoints[0].ygrid;
		seg->lspoints[seg->nls].zdata = seg->endpoints[0].zdata;
		seg->nls++;

		/* get line equation */
		double mm;
		double bb;
		if (ni > 0 && seg->endpoints[1].xgrid != seg->endpoints[0].xgrid) {
			mm = (seg->endpoints[1].ygrid - seg->endpoints[0].ygrid) / (seg->endpoints[1].xgrid - seg->endpoints[0].xgrid);
			bb = seg->endpoints[0].ygrid - mm * seg->endpoints[0].xgrid;
		}

		double xgrid, ygrid, zdata;

		/* loop over xgrid */
		int insert = 1;
		for (int icnt = 0; icnt < ni; icnt++) {
			const int i = istart + icnt * iadd;
			xgrid = data->primary_xmin + i * data->primary_dx;
			ygrid = mm * xgrid + bb;
			const int j = (int)((ygrid - data->primary_ymin) / data->primary_dy);
			const int k = i * data->primary_n_rows + j;
			const int l = i * data->primary_n_rows + j + 1;
			if (i >= 0 && i < data->primary_n_columns - 1 && j >= 0 && j < data->primary_n_rows - 1 &&
			    data->primary_data[k] != data->primary_nodatavalue && data->primary_data[l] != data->primary_nodatavalue) {
				/* interpolate zdata */
				zdata = data->primary_data[k] + (ygrid - data->primary_ymin - j * data->primary_dy) / data->primary_dy *
				                                    (data->primary_data[l] - data->primary_data[k]);

				/* add point to list */
				seg->lspoints[seg->nls].xgrid = xgrid;
				seg->lspoints[seg->nls].ygrid = ygrid;
				seg->lspoints[seg->nls].zdata = zdata;
				seg->nls++;
			}
		}

		/* put end point in list */
		seg->lspoints[seg->nls].xgrid = seg->endpoints[1].xgrid;
		seg->lspoints[seg->nls].ygrid = seg->endpoints[1].ygrid;
		seg->lspoints[seg->nls].zdata = seg->endpoints[1].zdata;
		seg->nls++;

		/* get line equation */
		if (nj > 0 && seg->endpoints[1].ygrid != seg->endpoints[0].ygrid) {
			mm = (seg->endpoints[1].xgrid - seg->endpoints[0].xgrid) / (seg->endpoints[1].ygrid - seg->endpoints[0].ygrid);
			bb = seg->endpoints[0].xgrid - mm * seg->endpoints[0].ygrid;
		}

		/* loop over ygrid */
		insert = 1;
		for (int jcnt = 0; jcnt < nj; jcnt++) {
			const int j = jstart + jcnt * jadd;
			ygrid = data->primary_ymin + j * data->primary_dy;
			xgrid = mm * ygrid + bb;
			const int i = (int)((xgrid - data->primary_xmin) / data->primary_dx);
			const int k = i * data->primary_n_rows + j;
			const int l = (i + 1) * data->primary_n_rows + j;
			if (i >= 0 && i < data->primary_n_columns - 1 && j >= 0 && j < data->primary_n_rows - 1 &&
			    data->primary_data[k] != data->primary_nodatavalue && data->primary_data[l] != data->primary_nodatavalue) {
				/* interpolate zdata */
				zdata = data->primary_data[k] + (xgrid - data->primary_xmin - i * data->primary_dx) / data->primary_dx *
				                                    (data->primary_data[l] - data->primary_data[k]);

				/* insert point into list */
				double found = false;
				done = false;
				if (jadd > 0)
					while (!done) {
						if (ygrid > seg->lspoints[insert - 1].ygrid && ygrid < seg->lspoints[insert].ygrid) {
							found = true;
							done = true;
						}
						else if (ygrid == seg->lspoints[insert - 1].ygrid || ygrid == seg->lspoints[insert].ygrid) {
							done = true;
						}
						else if (ygrid < seg->lspoints[insert - 1].ygrid) {
							insert--;
						}
						else if (ygrid > seg->lspoints[insert].ygrid) {
							insert++;
						}
						if (insert <= 0 || insert >= seg->nls) {
							done = true;
						}
					}
				else if (jadd < 0)
					while (!done) {
						if (ygrid > seg->lspoints[insert].ygrid && ygrid < seg->lspoints[insert - 1].ygrid) {
							found = true;
							done = true;
						}
						else if (ygrid == seg->lspoints[insert].ygrid || ygrid == seg->lspoints[insert - 1].ygrid) {
							done = true;
						}
						else if (ygrid > seg->lspoints[insert - 1].ygrid) {
							insert--;
						}
						else if (ygrid < seg->lspoints[insert].ygrid) {
							insert++;
						}
						if (insert <= 0 || insert >= seg->nls) {
							done = true;
						}
					}
				if (found) {
					for (int ii = seg->nls; ii > insert; ii--) {
						seg->lspoints[ii].xgrid = seg->lspoints[ii - 1].xgrid;
						seg->lspoints[ii].ygrid = seg->lspoints[ii - 1].ygrid;
						seg->lspoints[ii].zdata = seg->lspoints[ii - 1].zdata;
					}
					seg->lspoints[insert].xgrid = xgrid;
					seg->lspoints[insert].ygrid = ygrid;
					seg->lspoints[insert].zdata = zdata;
					seg->nls++;
				}
			}
		}

		// calculate rest of point values
		for (int icnt = 0; icnt < seg->nls; icnt++) {
			mbview_projectforward(instance, true, seg->lspoints[icnt].xgrid, seg->lspoints[icnt].ygrid,
			                      seg->lspoints[icnt].zdata, &(seg->lspoints[icnt].xlon), &(seg->lspoints[icnt].ylat),
			                      &(seg->lspoints[icnt].xdisplay), &(seg->lspoints[icnt].ydisplay),
			                      &(seg->lspoints[icnt].zdisplay));
			if (data->display_projection_mode != MBV_PROJECTION_SPHEROID) {
				seg->lspoints[icnt].zdisplay += offset_factor;
			}
			else if (global) {
				seg->lspoints[icnt].xdisplay += seg->lspoints[icnt].xdisplay * offset_factor;
				seg->lspoints[icnt].ydisplay += seg->lspoints[icnt].ydisplay * offset_factor;
				seg->lspoints[icnt].zdisplay += seg->lspoints[icnt].zdisplay * offset_factor;
			}
			else {
				seg->lspoints[icnt].zdisplay += offset_factor;
			}
		}
	}

	if (mbv_verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:          %d\n", status);
		fprintf(stderr, "dbg2       seg->nls:        %d\n", seg->nls);
		fprintf(stderr, "dbg2       seg->nls_alloc:  %d\n", seg->nls_alloc);
		fprintf(stderr, "dbg2       seg->lspoints:\n");
		for (int i = 0; i < seg->nls; i++) {
			fprintf(stderr, "dbg2         point[%4d]:    %f %f %f  %f %f  %f %f %f\n", i, seg->lspoints[i].xgrid,
			        seg->lspoints[i].ygrid, seg->lspoints[i].zdata, seg->lspoints[i].xlon, seg->lspoints[i].ylat,
			        seg->lspoints[i].xdisplay, seg->lspoints[i].ydisplay, seg->lspoints[i].zdisplay);
		}
	}

	return (status);
}

/*------------------------------------------------------------------------------*/
int mbview_drapesegmentw(size_t instance, struct mbview_linesegmentw_struct *seg) {
	if (mbv_verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  MB-system Version %s\n", MB_VERSION);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       instance:         %zu\n", instance);
		fprintf(stderr, "dbg2       seg:              %p\n", seg);
		fprintf(stderr, "dbg2       seg->endpoints:\n");
		fprintf(stderr, "dbg2            xgrid[0]:    %f\n", seg->endpoints[0].xgrid[instance]);
		fprintf(stderr, "dbg2            ygrid[0]:    %f\n", seg->endpoints[0].ygrid[instance]);
		fprintf(stderr, "dbg2            xlon[0]:     %f\n", seg->endpoints[0].xlon);
		fprintf(stderr, "dbg2            ylat[0]:     %f\n", seg->endpoints[0].ylat);
		fprintf(stderr, "dbg2            xgrid[1]:    %f\n", seg->endpoints[1].xgrid[instance]);
		fprintf(stderr, "dbg2            ygrid[1]:    %f\n", seg->endpoints[1].ygrid[instance]);
		fprintf(stderr, "dbg2            xlon[1]:     %f\n", seg->endpoints[1].xlon);
		fprintf(stderr, "dbg2            ylat[1]:     %f\n", seg->endpoints[1].ylat);
	}

	struct mbview_world_struct *view = &(mbviews[instance]);
	struct mbview_struct *data = &(view->data);

	int status = MB_SUCCESS;

	/* if spheroid dipslay project on great circle arc */
	if (data->display_projection_mode == MBV_PROJECTION_SPHEROID) {
		status = mbview_drapesegmentw_gc(instance, seg);
	}

	/* else project on straight lines in grid projection */
	else {
		status = mbview_drapesegmentw_grid(instance, seg);
	}

	if (mbv_verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:          %d\n", status);
		fprintf(stderr, "dbg2       seg->endpoints:\n");
		fprintf(stderr, "dbg2            xgrid[0]:    %f\n", seg->endpoints[0].xgrid[instance]);
		fprintf(stderr, "dbg2            ygrid[0]:    %f\n", seg->endpoints[0].ygrid[instance]);
		fprintf(stderr, "dbg2            xlon[0]:     %f\n", seg->endpoints[0].xlon);
		fprintf(stderr, "dbg2            ylat[0]:     %f\n", seg->endpoints[0].ylat);
		fprintf(stderr, "dbg2            xgrid[1]:    %f\n", seg->endpoints[1].xgrid[instance]);
		fprintf(stderr, "dbg2            ygrid[1]:    %f\n", seg->endpoints[1].ygrid[instance]);
		fprintf(stderr, "dbg2            xlon[1]:     %f\n", seg->endpoints[1].xlon);
		fprintf(stderr, "dbg2            ylat[1]:     %f\n", seg->endpoints[1].ylat);
		fprintf(stderr, "dbg2       seg->nls:        %d\n", seg->nls);
		fprintf(stderr, "dbg2       seg->nls_alloc:  %d\n", seg->nls_alloc);
		fprintf(stderr, "dbg2       seg->lspoints:\n");
		for (int i = 0; i < seg->nls; i++) {
			fprintf(stderr, "dbg2         point[%4d]:    %f %f %f  %f %f  %f %f %f\n", i, seg->lspoints[i].xgrid[instance],
			        seg->lspoints[i].ygrid[instance], seg->lspoints[i].zdata, seg->lspoints[i].xlon, seg->lspoints[i].ylat,
			        seg->lspoints[i].xdisplay[instance], seg->lspoints[i].ydisplay[instance],
			        seg->lspoints[i].zdisplay[instance]);
		}
	}

	return (status);
}

/*------------------------------------------------------------------------------*/
int mbview_drapesegmentw_gc(size_t instance, struct mbview_linesegmentw_struct *seg) {
	if (mbv_verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  MB-system Version %s\n", MB_VERSION);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       instance:         %zu\n", instance);
		fprintf(stderr, "dbg2       seg:              %p\n", seg);
		fprintf(stderr, "dbg2       seg->endpoints:\n");
		fprintf(stderr, "dbg2            xgrid[0]:    %f\n", seg->endpoints[0].xgrid[instance]);
		fprintf(stderr, "dbg2            ygrid[0]:    %f\n", seg->endpoints[0].ygrid[instance]);
		fprintf(stderr, "dbg2            xlon[0]:     %f\n", seg->endpoints[0].xlon);
		fprintf(stderr, "dbg2            ylat[0]:     %f\n", seg->endpoints[0].ylat);
		fprintf(stderr, "dbg2            xgrid[1]:    %f\n", seg->endpoints[1].xgrid[instance]);
		fprintf(stderr, "dbg2            ygrid[1]:    %f\n", seg->endpoints[1].ygrid[instance]);
		fprintf(stderr, "dbg2            xlon[1]:     %f\n", seg->endpoints[1].xlon);
		fprintf(stderr, "dbg2            ylat[1]:     %f\n", seg->endpoints[1].ylat);
	}

	struct mbview_world_struct *view = &(mbviews[instance]);
	struct mbview_struct *data = &(view->data);

	/* check if the contour offset needs to be applied in a global spherical direction or just up */
	const bool global =
		data->display_projection_mode == MBV_PROJECTION_SPHEROID &&
		view->sphere_refx == 0.0 && view->sphere_refy == 0.0 && view->sphere_refz == 0.0;
	const double offset_factor =
		10.0 *
		(global
		 ? MBV_OPENGL_3D_CONTOUR_OFFSET / (view->scale * MBV_SPHEROID_RADIUS)
		 : MBV_OPENGL_3D_CONTOUR_OFFSET);

	/* get half characteristic distance between grid points
	    from center of primary grid */
	double xlon1, ylat1, xlon2, ylat2;
	{
		const int i = data->primary_n_columns / 2;
		const int j = data->primary_n_rows / 2;
		mbview_projectgrid2ll(instance, (double)(data->primary_xmin + i * data->primary_dx),
	                      (double)(data->primary_ymin + j * data->primary_dy), &xlon1, &ylat1);
		mbview_projectgrid2ll(instance, (double)(data->primary_xmin + (i + 1) * data->primary_dx),
	                      (double)(data->primary_ymin + (j + 1) * data->primary_dy), &xlon2, &ylat2);
        }
	double dsegbearing;
	double dsegdist;
	mbview_greatcircle_distbearing(instance, xlon1, ylat1, xlon2, ylat2, &dsegbearing, &dsegdist);

	/* get number of preliminary points along the segment */
	double segbearing, segdist;
	mbview_greatcircle_distbearing(instance, seg->endpoints[0].xlon, seg->endpoints[0].ylat, seg->endpoints[1].xlon,
	                               seg->endpoints[1].ylat, &segbearing, &segdist);
	const int nsegpoint = MAX(((int)((segdist / dsegdist) + 1)), 2);

	int status = MB_SUCCESS;

	bool done = false;
	/* no need to fill in if the segment doesn't cross grid boundaries */
	if (nsegpoint <= 2) {
		done = true;
		seg->nls = 0;
		seg->nls_alloc = 0;
	} else {
		/* get the points along the great circle arc */

		/* get effective distance between points along great circle */
		dsegdist = segdist / (nsegpoint - 1);

		/* allocate segment points */
		seg->nls_alloc = nsegpoint;
	int error = MB_ERROR_NO_ERROR;
		status = mb_reallocd(mbv_verbose, __FILE__, __LINE__, seg->nls_alloc * sizeof(struct mbview_pointw_struct),
		                     (void **)&(seg->lspoints), &error);
		if (status == MB_FAILURE) {
			done = true;
			seg->nls_alloc = 0;
			seg->nls = 0;
		}
	}

	/* now calculate points along great circle arc */
	if (seg->nls_alloc > 1 && !done) {
		/* put begin point in list */
		seg->nls = 0;
		seg->lspoints[seg->nls].xgrid[instance] = seg->endpoints[0].xgrid[instance];
		seg->lspoints[seg->nls].ygrid[instance] = seg->endpoints[0].ygrid[instance];
		seg->lspoints[seg->nls].zdata = seg->endpoints[0].zdata;
		seg->lspoints[seg->nls].xlon = seg->endpoints[0].xlon;
		seg->lspoints[seg->nls].ylat = seg->endpoints[0].ylat;
		seg->nls++;

		for (int i = 1; i < nsegpoint - 1; i++) {
			mbview_greatcircle_endposition(instance, seg->lspoints[0].xlon, seg->lspoints[0].ylat, segbearing,
			                               (double)(i * dsegdist), &(seg->lspoints[seg->nls].xlon),
			                               &(seg->lspoints[seg->nls].ylat)),
			    status = mbview_projectll2xyzgrid(instance, seg->lspoints[seg->nls].xlon, seg->lspoints[seg->nls].ylat,
			                                      &(seg->lspoints[seg->nls].xgrid[instance]),
			                                      &(seg->lspoints[seg->nls].ygrid[instance]), &(seg->lspoints[seg->nls].zdata));
			if (status == MB_SUCCESS) {
				seg->nls++;
			}
		}

		/* put end point in list */
		seg->lspoints[seg->nls].xgrid[instance] = seg->endpoints[1].xgrid[instance];
		seg->lspoints[seg->nls].ygrid[instance] = seg->endpoints[1].ygrid[instance];
		seg->lspoints[seg->nls].zdata = seg->endpoints[1].zdata;
		seg->lspoints[seg->nls].xlon = seg->endpoints[1].xlon;
		seg->lspoints[seg->nls].ylat = seg->endpoints[1].ylat;
		seg->nls++;

		/* now calculate rest of point values */
		for (int icnt = 0; icnt < seg->nls; icnt++) {
			mbview_projectll2display(instance, seg->lspoints[icnt].xlon, seg->lspoints[icnt].ylat, seg->lspoints[icnt].zdata,
			                         &(seg->lspoints[icnt].xdisplay[instance]), &(seg->lspoints[icnt].ydisplay[instance]),
			                         &(seg->lspoints[icnt].zdisplay[instance]));
			if (data->display_projection_mode != MBV_PROJECTION_SPHEROID) {
				seg->lspoints[icnt].zdisplay[instance] += offset_factor;
			}
			else if (global) {
				seg->lspoints[icnt].xdisplay[instance] += seg->lspoints[icnt].xdisplay[instance] * offset_factor;
				seg->lspoints[icnt].ydisplay[instance] += seg->lspoints[icnt].ydisplay[instance] * offset_factor;
				seg->lspoints[icnt].zdisplay[instance] += seg->lspoints[icnt].zdisplay[instance] * offset_factor;
			}
			else {
				seg->lspoints[icnt].zdisplay[instance] += offset_factor;
			}
		}
	}

	if (mbv_verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:          %d\n", status);
		fprintf(stderr, "dbg2       seg->endpoints:\n");
		fprintf(stderr, "dbg2            xgrid[0]:    %f\n", seg->endpoints[0].xgrid[instance]);
		fprintf(stderr, "dbg2            ygrid[0]:    %f\n", seg->endpoints[0].ygrid[instance]);
		fprintf(stderr, "dbg2            xlon[0]:     %f\n", seg->endpoints[0].xlon);
		fprintf(stderr, "dbg2            ylat[0]:     %f\n", seg->endpoints[0].ylat);
		fprintf(stderr, "dbg2            xgrid[1]:    %f\n", seg->endpoints[1].xgrid[instance]);
		fprintf(stderr, "dbg2            ygrid[1]:    %f\n", seg->endpoints[1].ygrid[instance]);
		fprintf(stderr, "dbg2            xlon[1]:     %f\n", seg->endpoints[1].xlon);
		fprintf(stderr, "dbg2            ylat[1]:     %f\n", seg->endpoints[1].ylat);
		fprintf(stderr, "dbg2       seg->nls:        %d\n", seg->nls);
		fprintf(stderr, "dbg2       seg->nls_alloc:  %d\n", seg->nls_alloc);
		fprintf(stderr, "dbg2       seg->lspoints:\n");
		for (int i = 0; i < seg->nls; i++) {
			fprintf(stderr, "dbg2         point[%4d]:    %f %f %f  %f %f  %f %f %f\n", i, seg->lspoints[i].xgrid[instance],
			        seg->lspoints[i].ygrid[instance], seg->lspoints[i].zdata, seg->lspoints[i].xlon, seg->lspoints[i].ylat,
			        seg->lspoints[i].xdisplay[instance], seg->lspoints[i].ydisplay[instance],
			        seg->lspoints[i].zdisplay[instance]);
		}
	}

	return (status);
}

/*------------------------------------------------------------------------------*/
int mbview_drapesegmentw_grid(size_t instance, struct mbview_linesegmentw_struct *seg) {
	if (mbv_verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  MB-system Version %s\n", MB_VERSION);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       instance:         %zu\n", instance);
		fprintf(stderr, "dbg2       seg:              %p\n", seg);
		fprintf(stderr, "dbg2       seg->endpoints:\n");
		fprintf(stderr, "dbg2            xgrid[0]:    %f\n", seg->endpoints[0].xgrid[instance]);
		fprintf(stderr, "dbg2            ygrid[0]:    %f\n", seg->endpoints[0].ygrid[instance]);
		fprintf(stderr, "dbg2            xgrid[1]:    %f\n", seg->endpoints[1].xgrid[instance]);
		fprintf(stderr, "dbg2            ygrid[1]:    %f\n", seg->endpoints[1].ygrid[instance]);
	}

	struct mbview_world_struct *view = &(mbviews[instance]);
	struct mbview_struct *data = &(view->data);

	/* check if the contour offset needs to be applied in a global spherical direction or just up */
	int global;
	double offset_factor;
	if (data->display_projection_mode == MBV_PROJECTION_SPHEROID && view->sphere_refx == 0.0 && view->sphere_refy == 0.0 &&
	    view->sphere_refz == 0.0) {
		global = true;
		offset_factor = 10.0 * MBV_OPENGL_3D_CONTOUR_OFFSET / (view->scale * MBV_SPHEROID_RADIUS);
	}
	else {
		global = false;
		offset_factor = 10.0 * MBV_OPENGL_3D_CONTOUR_OFFSET;
	}

	/* figure out how many points to calculate along the segment */
	const double xgridstart = seg->endpoints[0].xgrid[instance];
	const double xgridend = seg->endpoints[1].xgrid[instance];
	const double ygridstart = seg->endpoints[0].ygrid[instance];
	const double ygridend = seg->endpoints[1].ygrid[instance];
	int istart = (int)((xgridstart - data->primary_xmin) / data->primary_dx);
	int iend = (int)((xgridend - data->primary_xmin) / data->primary_dx);
	int jstart = (int)((ygridstart - data->primary_ymin) / data->primary_dy);
	int jend = (int)((ygridend - data->primary_ymin) / data->primary_dy);
	if (istart < 0)
		istart = 0;
	if (istart >= data->primary_n_columns)
		istart = data->primary_n_columns - 1;
	if (iend < 0)
		iend = 0;
	if (iend >= data->primary_n_columns)
		iend = data->primary_n_columns - 1;
	if (jstart < 0)
		jstart = 0;
	if (jstart >= data->primary_n_columns)
		jstart = data->primary_n_rows - 1;
	if (jend < 0)
		jend = 0;
	if (jend >= data->primary_n_columns)
		jend = data->primary_n_rows - 1;

	int iadd;
	int jadd;
	int ni;
	int nj;
	int status = MB_SUCCESS;

	/* no need to fill in if the segment doesn't cross grid boundaries */
	bool done = false;
	if (istart == iend && jstart == jend) {
		done = true;
		seg->nls = 0;
	} else {
		/* else allocate space for the array of points */

		/* allocate space for the array of points */
		if (iend > istart) {
			ni = iend - istart;
			iadd = 1;
			istart++;
			// iend++;
		} else {
			ni = istart - iend;
			iadd = -1;
		} if (jend > jstart) {
			nj = jend - jstart;
			jadd = 1;
			jstart++;
			// jend++;
		} else {
			nj = jstart - jend;
			jadd = -1;
		}
		if ((ni + nj + 2) > seg->nls_alloc) {
			seg->nls_alloc = (ni + nj + 2);
			int error = MB_ERROR_NO_ERROR;
			status = mb_reallocd(mbv_verbose, __FILE__, __LINE__, seg->nls_alloc * sizeof(struct mbview_pointw_struct),
			                     (void **)&(seg->lspoints), &error);
			if (status == MB_FAILURE) {
				done = true;
				seg->nls_alloc = 0;
			}
		}
	}

	/* if points needed and space allocated do it */
	if (!done && ni + nj > 0) {
		/* put begin point in list */
		seg->nls = 0;
		seg->lspoints[seg->nls].xgrid[instance] = seg->endpoints[0].xgrid[instance];
		seg->lspoints[seg->nls].ygrid[instance] = seg->endpoints[0].ygrid[instance];
		seg->lspoints[seg->nls].zdata = seg->endpoints[0].zdata;
		seg->nls++;

		/* get line equation */
		double mm, bb;
		if (ni > 0 && seg->endpoints[1].xgrid[instance] != seg->endpoints[0].xgrid[instance]) {
			mm = (seg->endpoints[1].ygrid[instance] - seg->endpoints[0].ygrid[instance]) /
			     (seg->endpoints[1].xgrid[instance] - seg->endpoints[0].xgrid[instance]);
			bb = seg->endpoints[0].ygrid[instance] - mm * seg->endpoints[0].xgrid[instance];
		}

		/* loop over xgrid */
		for (int icnt = 0; icnt < ni; icnt++) {
			const int i = istart + icnt * iadd;
			const double xgrid = data->primary_xmin + i * data->primary_dx;
			const double ygrid = mm * xgrid + bb;
			const int j = (int)((ygrid - data->primary_ymin) / data->primary_dy);
			const int k = i * data->primary_n_rows + j;
			const int l = i * data->primary_n_rows + j + 1;
			if (i >= 0 && i < data->primary_n_columns - 1 && j >= 0 && j < data->primary_n_rows - 1 &&
			    data->primary_data[k] != data->primary_nodatavalue && data->primary_data[l] != data->primary_nodatavalue) {
				/* interpolate zdata */
				const double zdata = data->primary_data[k] + (ygrid - data->primary_ymin - j * data->primary_dy) / data->primary_dy *
				                                    (data->primary_data[l] - data->primary_data[k]);

				/* add point to list */
				seg->lspoints[seg->nls].xgrid[instance] = xgrid;
				seg->lspoints[seg->nls].ygrid[instance] = ygrid;
				seg->lspoints[seg->nls].zdata = zdata;
				seg->nls++;
				/*fprintf(stderr,"new ni point: nls:%d icnt:%d i:%d j:%d k:%d l:%d xgrid:%f ygrid:%f zdata:%f\n",
				seg->nls,icnt,i,j,k,l,xgrid,ygrid,zdata);*/
			}
		}

		/* put end point in list */
		seg->lspoints[seg->nls].xgrid[instance] = seg->endpoints[1].xgrid[instance];
		seg->lspoints[seg->nls].ygrid[instance] = seg->endpoints[1].ygrid[instance];
		seg->lspoints[seg->nls].zdata = seg->endpoints[1].zdata;
		seg->nls++;

		/* get line equation */
		if (nj > 0 && seg->endpoints[1].ygrid[instance] != seg->endpoints[0].ygrid[instance]) {
			mm = (seg->endpoints[1].xgrid[instance] - seg->endpoints[0].xgrid[instance]) /
			     (seg->endpoints[1].ygrid[instance] - seg->endpoints[0].ygrid[instance]);
			bb = seg->endpoints[0].xgrid[instance] - mm * seg->endpoints[0].ygrid[instance];
		}

		/* loop over ygrid */
		int insert = 1;
		for (int jcnt = 0; jcnt < nj; jcnt++) {
			const int j = jstart + jcnt * jadd;
			const double ygrid = data->primary_ymin + j * data->primary_dy;
			const double xgrid = mm * ygrid + bb;
			const int i = (int)((xgrid - data->primary_xmin) / data->primary_dx);
			const int k = i * data->primary_n_rows + j;
			const int l = (i + 1) * data->primary_n_rows + j;
			if (i >= 0 && i < data->primary_n_columns - 1 && j >= 0 && j < data->primary_n_rows - 1 &&
			    data->primary_data[k] != data->primary_nodatavalue && data->primary_data[l] != data->primary_nodatavalue) {
				/* interpolate zdata */
				const double zdata = data->primary_data[k] + (xgrid - data->primary_xmin - i * data->primary_dx) / data->primary_dx *
				                                    (data->primary_data[l] - data->primary_data[k]);

				/* insert point into list */
				bool found = false;
				done = false;
				if (jadd > 0)
					while (!done) {
						if (ygrid > seg->lspoints[insert - 1].ygrid[instance] && ygrid < seg->lspoints[insert].ygrid[instance]) {
							found = true;
							done = true;
						}
						else if (ygrid == seg->lspoints[insert - 1].ygrid[instance] ||
						         ygrid == seg->lspoints[insert].ygrid[instance]) {
							done = true;
						}
						else if (ygrid < seg->lspoints[insert - 1].ygrid[instance]) {
							insert--;
						}
						else if (ygrid > seg->lspoints[insert].ygrid[instance]) {
							insert++;
						}
						if (insert <= 0 || insert >= seg->nls) {
							done = true;
						}
						/*fprintf(stderr,"jadd>0: insert:%d found:%d done:%d\n",insert,found,done);*/
					}
				else if (jadd < 0)
					while (!done) {
						if (ygrid > seg->lspoints[insert].ygrid[instance] && ygrid < seg->lspoints[insert - 1].ygrid[instance]) {
							found = true;
							done = true;
						}
						else if (ygrid == seg->lspoints[insert].ygrid[instance] ||
						         ygrid == seg->lspoints[insert - 1].ygrid[instance]) {
							done = true;
						}
						else if (ygrid > seg->lspoints[insert - 1].ygrid[instance]) {
							insert--;
						}
						else if (ygrid < seg->lspoints[insert].ygrid[instance]) {
							insert++;
						}
						if (insert <= 0 || insert >= seg->nls) {
							done = true;
						}
						/*fprintf(stderr,"jadd<0: insert:%d found:%d done:%d\n",insert,found,done);*/
					}
				if (insert < 0)
					insert = 0;
				else if (insert > seg->nls)
					insert = seg->nls;
				if (found) {
					for (int ii = seg->nls; ii > insert; ii--) {
						seg->lspoints[ii].xgrid[instance] = seg->lspoints[ii - 1].xgrid[instance];
						seg->lspoints[ii].ygrid[instance] = seg->lspoints[ii - 1].ygrid[instance];
						seg->lspoints[ii].zdata = seg->lspoints[ii - 1].zdata;
					}
					seg->lspoints[insert].xgrid[instance] = xgrid;
					seg->lspoints[insert].ygrid[instance] = ygrid;
					seg->lspoints[insert].zdata = zdata;
					seg->nls++;
					/*fprintf(stderr,"new nj point: nls:%d jcnt:%d insert:%d jadd:%d i:%d j:%d k:%d l:%d xgrid:%f ygrid:%f
					zdata:%f\n", seg->nls,jcnt,insert,jadd,i,j,k,l,xgrid,ygrid,zdata);*/
				}
				if (insert <= 0)
					insert = 1;
				else if (insert >= seg->nls)
					insert = seg->nls - 1;
			}
		}

		/* now calculate rest of point values */
		for (int icnt = 0; icnt < seg->nls; icnt++) {
			mbview_projectforward(instance, true, seg->lspoints[icnt].xgrid[instance], seg->lspoints[icnt].ygrid[instance],
			                      seg->lspoints[icnt].zdata, &(seg->lspoints[icnt].xlon), &(seg->lspoints[icnt].ylat),
			                      &(seg->lspoints[icnt].xdisplay[instance]), &(seg->lspoints[icnt].ydisplay[instance]),
			                      &(seg->lspoints[icnt].zdisplay[instance]));
			if (data->display_projection_mode != MBV_PROJECTION_SPHEROID) {
				seg->lspoints[icnt].zdisplay[instance] += offset_factor;
			}
			else if (global) {
				seg->lspoints[icnt].xdisplay[instance] += seg->lspoints[icnt].xdisplay[instance] * offset_factor;
				seg->lspoints[icnt].ydisplay[instance] += seg->lspoints[icnt].ydisplay[instance] * offset_factor;
				seg->lspoints[icnt].zdisplay[instance] += seg->lspoints[icnt].zdisplay[instance] * offset_factor;
			}
			else {
				seg->lspoints[icnt].zdisplay[instance] += offset_factor;
			}
		}
	}

	if (mbv_verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:          %d\n", status);
		fprintf(stderr, "dbg2       seg->nls:        %d\n", seg->nls);
		fprintf(stderr, "dbg2       seg->nls_alloc:  %d\n", seg->nls_alloc);
		fprintf(stderr, "dbg2       seg->lspoints:\n");
		for (int i = 0; i < seg->nls; i++) {
			fprintf(stderr, "dbg2         point[%4d]:    %f %f %f  %f %f  %f %f %f\n", i, seg->lspoints[i].xgrid[instance],
			        seg->lspoints[i].ygrid[instance], seg->lspoints[i].zdata, seg->lspoints[i].xlon, seg->lspoints[i].ylat,
			        seg->lspoints[i].xdisplay[instance], seg->lspoints[i].ydisplay[instance],
			        seg->lspoints[i].zdisplay[instance]);
		}
	}

	return (status);
}

/*------------------------------------------------------------------------------*/
int mbview_glerrorcheck(size_t instance, char *sourcefile, int line, char *sourcefunction) {
	if (mbv_verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  MB-system Version %s\n", MB_VERSION);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       instance:         %zu\n", instance);
		fprintf(stderr, "dbg2       sourcefile:       %s\n", sourcefile);
		fprintf(stderr, "dbg2       line:             %d\n", line);
		fprintf(stderr, "dbg2       sourcefunction:   %s\n", sourcefunction);
	}

	/* check for OpenGL error if MBV_GET_GLX_ERRORS set */
	const GLenum gl_error = (GLenum)glGetError();
	const GLubyte *gl_error_msg = (GLubyte *)gluErrorString(gl_error);
	if (gl_error != GL_NO_ERROR)
		fprintf(stderr, "GLerror: Instance:%zu %s:%d Function %s: OpenGL error: %s\n", instance, sourcefile, line, sourcefunction,
		        gl_error_msg);

	const int status = MB_SUCCESS;

	if (mbv_verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:          %d\n", status);
		fprintf(stderr, "dbg2       gl_error:        %d\n", gl_error);
		fprintf(stderr, "dbg2       gl_error_msg:    %s\n", gl_error_msg);
	}

	return (status);
}
/*------------------------------------------------------------------------------*/
