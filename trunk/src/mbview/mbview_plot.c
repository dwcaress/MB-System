/*------------------------------------------------------------------------------
 *    The MB-system:	mbview_plot.c	9/26/2003
 *    $Id: mbview_plot.c,v 5.0 2003-12-02 20:38:33 caress Exp $
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
 * Revision 1.2  2003/11/25 01:43:18  caress
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

/* local variables */
Cardinal 	ac;
Arg      	args[256];
char		value_text[MB_PATH_MAXLINE];

static char rcs_id[]="$Id: mbview_plot.c,v 5.0 2003-12-02 20:38:33 caress Exp $";

/*------------------------------------------------------------------------------*/
int mbview_reset_glx(int instance)
{
	/* local variables */
	char	*function_name = "mbview_reset_glx";
	int	status = MB_SUCCESS;
	struct mbview_world_struct *view;
	struct mbview_struct *data;

	/* print starting debug statements */
	if (mbv_verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Version %s\n",rcs_id);
		fprintf(stderr,"dbg2  MB-system Version %s\n",MB_VERSION);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       instance:         %d\n",instance);
		}
		
	/* get view */
	view = &(mbviews[instance]);
	data = &(view->data);
		
	/* delete old glx_context if it exists */
	if (view->glx_init == MB_YES)
		{
		glXDestroyContext(view->dpy, view->glx_context);
		view->glx_init = MB_NO;
		}
	
	/* set up a new opengl context */
	ac = 0;
	XtSetArg(args[ac], GLwNvisualInfo, &(view->vi));
	ac++;
	XtGetValues(view->glwmda, args, ac);
	view->glx_context = glXCreateContext(view->dpy, view->vi,
                	     NULL, GL_FALSE);
	GLwDrawingAreaMakeCurrent(view->glwmda, view->glx_context);
	view->glx_init = MB_YES;
        glViewport(0, 0, data->width, data->height);
	view->aspect_ratio = ((float)data->width) / ((float)data->height);
	view->lastdrawrez = MBV_REZ_NONE;
	view->contourlorez = MB_NO;
	view->contourhirez = MB_NO;
	view->contourfullrez = MB_NO;

	/* print output debug statements */
	if (mbv_verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:  %d\n",status);
		fprintf(stderr,"dbg2       view->dpy:             %d\n", view->dpy);
		fprintf(stderr,"dbg2       view->vi:              %d\n", view->vi);
		fprintf(stderr,"dbg2       view->glwmda:          %d\n", view->glwmda);
		fprintf(stderr,"dbg2       view->glx_context:     %d\n", view->glx_context);
		fprintf(stderr,"dbg2       view->glx_init:        %d\n", view->glx_init);
		fprintf(stderr,"dbg2       view->lastdrawrez:     %d\n", view->lastdrawrez);
		fprintf(stderr,"dbg2       view->contourlorez:    %d\n", view->contourlorez);
		fprintf(stderr,"dbg2       view->contourhirez:    %d\n", view->contourhirez);
		fprintf(stderr,"dbg2       view->contourfullrez:  %d\n", view->contourfullrez);
		}

	/* return */
	return(status);
}

		
/*------------------------------------------------------------------------------*/
int mbview_drawdata(int instance, int rez)
{
	/* local variables */
	char	*function_name = "mbview_drawdata";
	int	status = MB_SUCCESS;
	int	on, flip;
	int	stride;
	int	i, j, k, l, m, n, ikk, ill, kk, ll;
	float	xlength, xxlength;
	struct mbview_world_struct *view;
	struct mbview_struct *data;

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
		
	/* set lastdrawrez flag */
	view->lastdrawrez = rez;
	
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
	if (stride == 1)
		view->lastdrawrez = MBV_REZ_FULL;
	
	/* enable depth test for 3D plots */
	if (data->display_mode == MBV_DISPLAY_3D)
		glEnable(GL_DEPTH_TEST);
		
	/* set color parameters */
	mbview_setcolorparms(instance);
	
/*fprintf(stderr,"mbview_drawdata: %d %d stride:%d\n", instance,rez,stride);*/

	/* draw the triangle outlines */
	/*
	glColor3f(1.0, 0.0, 0.0);
	for (i=0;i<data->primary_nx-1;i++)
	{
	for (j=0;j<data->primary_ny-1;j++)
		{
		k = i * data->primary_ny + j;
		l = (i + 1) * data->primary_ny + j;
		m = i * data->primary_ny + j + 1;
		n = (i + 1) * data->primary_ny + j + 1;
		if (data->primary_data[k] != data->primary_nodatavalue
			&& data->primary_data[l] != data->primary_nodatavalue
			&& data->primary_data[m] != data->primary_nodatavalue)
			{
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
			&& data->primary_data[n] != data->primary_nodatavalue)
			{
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
	}
	*/
	
	/* draw the triangles */
	/*glBegin(GL_TRIANGLES);
	for (i=0;i<data->primary_nx-stride;i+=stride)
	{
	for (j=0;j<data->primary_ny-stride;j+=stride)
		{
		k = i * data->primary_ny + j;
		l = (i + stride) * data->primary_ny + j;
		m = i * data->primary_ny + j + stride;
		n = (i + stride) * data->primary_ny + j + stride;
		if (data->primary_data[k] != data->primary_nodatavalue
			&& data->primary_data[l] != data->primary_nodatavalue
			&& data->primary_data[m] != data->primary_nodatavalue)
			{
			glColor3f(data->primary_r[k],
				data->primary_g[k],
				data->primary_b[k]);
			glVertex3f(data->primary_x[k],
				data->primary_y[k],
				data->primary_z[k]);
			glColor3f(data->primary_r[l],
				data->primary_g[l],
				data->primary_b[l]);
			glVertex3f(data->primary_x[l],
				data->primary_y[l],
				data->primary_z[l]);
			glColor3f(data->primary_r[m],
				data->primary_g[m],
				data->primary_b[m]);
			glVertex3f(data->primary_x[m],
				data->primary_y[m],
				data->primary_z[m]);
			}
		if (data->primary_data[l] != data->primary_nodatavalue
			&& data->primary_data[m] != data->primary_nodatavalue
			&& data->primary_data[n] != data->primary_nodatavalue)
			{
			glColor3f(data->primary_r[l],
				data->primary_g[l],
				data->primary_b[l]);
			glVertex3f(data->primary_x[l],
				data->primary_y[l],
				data->primary_z[l]);
			glColor3f(data->primary_r[n],
				data->primary_g[n],
				data->primary_b[n]);
			glVertex3f(data->primary_x[n],
				data->primary_y[n],
				data->primary_z[n]);
			glColor3f(data->primary_r[m],
				data->primary_g[m],
				data->primary_b[m]);
			glVertex3f(data->primary_x[m],
				data->primary_y[m],
				data->primary_z[m]);
			}
		}
		
	/* check for pending event */
	/*if (view->plot_done == MB_NO 
		&& view->plot_interrupt_allowed == MB_YES 
		&& i % MBV_EVENTCHECKCOARSENESS == 0)
		do_mbview_xevents();*/
		
	/* dump out of loop if plotting already done at a higher recursion */
	/*if (view->plot_done == MB_YES)
		i = data->primary_nx;
	}
	glEnd();*/
		
	/* draw the data as triangle strips */
	for (i=0;i<data->primary_nx-stride;i+=stride)
	{
	on = MB_NO;
	flip = MB_NO;
	for (j=0;j<data->primary_ny;j+=stride)
		{
		k = i * data->primary_ny + j;
		l = (i + stride) * data->primary_ny + j;
		if (flip == MB_NO)
			{
			ikk = i;
			kk = k;
			ill = i + stride;
			ll = l;
			}
		else
			{
			ikk = i + stride;
			kk = l;
			ill = i;
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
			if (!(data->primary_stat_z[kk/8] & statmask[kk%8]))
				mbview_zscalepoint(view,data,kk);
			if (!(data->primary_stat_color[kk/8] & statmask[kk%8]))
				{
				mbview_colorpoint(
					view, data, ikk, j, kk);
				}
			glColor3f(data->primary_r[kk],
				data->primary_g[kk],
				data->primary_b[kk]);
			glVertex3f(data->primary_x[kk],
				data->primary_y[kk],
				data->primary_z[kk]);
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
			if (!(data->primary_stat_z[ll/8] & statmask[ll%8]))
				mbview_zscalepoint(view,data,ll);
			if (!(data->primary_stat_color[ll/8] & statmask[ll%8]))
				{
				mbview_colorpoint(
					view, data, ill, j, ll);
				}
			glColor3f(data->primary_r[ll],
				data->primary_g[ll],
				data->primary_b[ll]);
			glVertex3f(data->primary_x[ll],
				data->primary_y[ll],
				data->primary_z[ll]);
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
		
	/* check for pending event */
	if (view->plot_done == MB_NO 
		&& view->plot_interrupt_allowed == MB_YES 
		&& i % MBV_EVENTCHECKCOARSENESS == 0)
		{
		do_mbview_xevents();
		}
		
	/* dump out of loop if plotting already done at a higher recursion */
	if (view->plot_done == MB_YES)
		i = data->primary_nx;
	}

	/* draw contours */
	if (data->grid_contour_mode == MBV_VIEW_ON)
		{
		if (rez == MBV_REZ_FULL && view->contourfullrez == MB_YES)
	    		glCallList((GLuint)(3*instance+3));
		else if (rez == MBV_REZ_HIGH && view->contourhirez == MB_YES)
	    		glCallList((GLuint)(3*instance+2));
		else if (rez == MBV_REZ_LOW && view->contourlorez == MB_YES)
	    		glCallList((GLuint)(3*instance+1));
		}
		
	/* draw current pick */
	mbview_drawpick(instance);
		
	/* draw current area */
	mbview_drawarea(instance);

	/* draw current navpick */
	mbview_drawnavpick(instance);		

	/* draw sites */
	mbview_drawsite(instance, rez);
		
	/* draw routes */
	mbview_drawroute(instance, rez);
		
	/* draw nav */
	mbview_drawnav(instance, rez);

	/* make sure depth test is off */
	glDisable(GL_DEPTH_TEST);

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
int mbview_plotlow(int instance)
{
	/* local variables */
	char	*function_name = "mbview_plotlow";
	int	status = MB_SUCCESS;
	struct mbview_world_struct *view;
	struct mbview_struct *data;
	float	viewdistance;

	/* print starting debug statements */
	if (mbv_verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Version %s\n",rcs_id);
		fprintf(stderr,"dbg2  MB-system Version %s\n",MB_VERSION);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       instance:         %d\n",instance);
		}
		
	/* get view */
	view = &(mbviews[instance]);
	data = &(view->data);

	/* set plot_done to MB_NO and increment the plot recursion level */
	view->plot_done = MB_NO;
	view->plot_recursion++;
	
	status = mbview_plot(instance, MBV_REZ_LOW);

	/* the plot_done flag will still be MB_NO if this
	   is the highest recursion level to be reached - finish the plot
	   only in this case */
	if (view->plot_done == MB_NO)
		{
		/* set plot_done to MB_YES */
		view->plot_done = MB_YES;
if (mbv_verbose >= 2)
fprintf(stderr, "Plot finished! instance:%d recursion:%d\n", instance, view->plot_recursion);
		}
		
	/* decrement the plot recursion level */
	view->plot_recursion--;
			
	if (view->message_on == MB_YES && view->plot_recursion == 0)
		do_mbview_status("Done.", instance);
if (mbv_verbose >= 2)
fprintf(stderr,"Done with mbview_plotlow %d  recursion:%d\n\n",instance,view->plot_recursion);

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
int mbview_plotlowhigh(int instance)
{
	/* local variables */
	char	*function_name = "mbview_plotlowhigh";
	int	status = MB_SUCCESS;
	struct mbview_world_struct *view;
	struct mbview_struct *data;
	float	viewdistance;

	/* print starting debug statements */
	if (mbv_verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Version %s\n",rcs_id);
		fprintf(stderr,"dbg2  MB-system Version %s\n",MB_VERSION);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       instance:         %d\n",instance);
		}
		
	/* get view */
	view = &(mbviews[instance]);
	data = &(view->data);

	/* set plot_done to MB_NO and increment the plot recursion level */
	view->plot_done = MB_NO;
	view->plot_recursion++;
	
	status = mbview_plot(instance, MBV_REZ_LOW);
	
	status = mbview_plot(instance, MBV_REZ_HIGH);

	/* the plot_done flag will still be MB_NO if this
	   is the highest recursion level to be reached - finish the plot
	   only in this case */
	if (view->plot_done == MB_NO)
		{
		/* set plot_done to MB_YES */
		view->plot_done = MB_YES;
if (mbv_verbose >= 2)
fprintf(stderr, "Plot finished! instance:%d recursion:%d\n", instance, view->plot_recursion);
		}
		
	/* decrement the plot recursion level */
	view->plot_recursion--;
			
	if (view->message_on == MB_YES && view->plot_recursion == 0)
		do_mbview_status("Done.", instance);
if (mbv_verbose >= 2)
fprintf(stderr,"Done with mbview_plotlowhigh %d  recursion:%d\n\n",instance,view->plot_recursion);

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
int mbview_plothigh(int instance)
{
	/* local variables */
	char	*function_name = "mbview_plothigh";
	int	status = MB_SUCCESS;
	struct mbview_world_struct *view;
	struct mbview_struct *data;
	float	viewdistance;

	/* print starting debug statements */
	if (mbv_verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Version %s\n",rcs_id);
		fprintf(stderr,"dbg2  MB-system Version %s\n",MB_VERSION);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       instance:         %d\n",instance);
		}
		
	/* get view */
	view = &(mbviews[instance]);
	data = &(view->data);

	/* set plot_done to MB_NO and increment the plot recursion level */
	view->plot_done = MB_NO;
	view->plot_recursion++;
	
	status = mbview_plot(instance, MBV_REZ_HIGH);

	/* the plot_done flag will still be MB_NO if this
	   is the highest recursion level to be reached - finish the plot
	   only in this case */
	if (view->plot_done == MB_NO)
		{
		/* set plot_done to MB_YES */
		view->plot_done = MB_YES;
if (mbv_verbose >= 2)
fprintf(stderr, "Plot finished! instance:%d recursion:%d\n", instance, view->plot_recursion);
		}
		
	/* decrement the plot recursion level */
	view->plot_recursion--;
			
	if (view->message_on == MB_YES && view->plot_recursion == 0)
		do_mbview_status("Done.", instance);
if (mbv_verbose >= 2)
fprintf(stderr,"Done with mbview_plothigh %d  recursion:%d\n\n",instance,view->plot_recursion);

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
int mbview_plotfull(int instance)
{
	/* local variables */
	char	*function_name = "mbview_plotfull";
	int	status = MB_SUCCESS;
	struct mbview_world_struct *view;
	struct mbview_struct *data;
	float	viewdistance;

	/* print starting debug statements */
	if (mbv_verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Version %s\n",rcs_id);
		fprintf(stderr,"dbg2  MB-system Version %s\n",MB_VERSION);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       instance:         %d\n",instance);
		}
		
	/* get view */
	view = &(mbviews[instance]);
	data = &(view->data);

	/* set plot_done to MB_NO and increment the plot recursion level */
	view->plot_done = MB_NO;
	view->plot_recursion++;
	
	status = mbview_plot(instance, MBV_REZ_FULL);

	/* the plot_done flag will still be MB_NO if this
	   is the highest recursion level to be reached - finish the plot
	   only in this case */
	if (view->plot_done == MB_NO)
		{
		/* set plot_done to MB_YES */
		view->plot_done = MB_YES;
if (mbv_verbose >= 2)
fprintf(stderr, "Plot finished! instance:%d recursion:%d\n", instance, view->plot_recursion);
		}
		
	/* decrement the plot recursion level */
	view->plot_recursion--;
			
	if (view->message_on == MB_YES && view->plot_recursion == 0)
		do_mbview_status("Done.", instance);
if (mbv_verbose >= 2)
fprintf(stderr,"Done with mbview_plotfull %d  recursion:%d\n\n",instance,view->plot_recursion);

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
int mbview_plot(int instance, int rez)
{
	/* local variables */
	char	*function_name = "mbview_plot";
	int	status = MB_SUCCESS;
	struct mbview_world_struct *view;
	struct mbview_struct *data;
	float	viewdistance;

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
	
	/* make correct window current for OpenGL */
	GLwDrawingAreaMakeCurrent(view->glwmda, view->glx_context);
/*fprintf(stderr,"\nmbview_plot: instance:%d rez:%d recursion:%d\n",instance,rez,view->plot_recursion);
fprintf(stderr,"     view->plot_done:        %d\n",view->plot_done);
fprintf(stderr,"     view->plot_recursion:   %d\n",view->plot_recursion);
fprintf(stderr,"     view->projected:        %d\n",view->projected);
fprintf(stderr,"     view->contourlorez:     %d\n",view->contourlorez);
fprintf(stderr,"     view->contourhirez:     %d\n",view->contourhirez);
fprintf(stderr,"     view->contourfullrez:   %d\n",view->contourfullrez);
fprintf(stderr,"     data->pick_type:  %d\n",data->pick_type);*/
	
	/* apply projection if needed */
	if (view->plot_done == MB_NO
		&& view->projected == MB_NO)
		{
		do_mbview_status("Projecting data...", instance);
		mbview_projectdata(instance);
		}

	/* contour if needed */
	if (view->plot_done == MB_NO
		&& (data->grid_contour_mode == MBV_VIEW_ON)
		&& ((rez == MBV_REZ_FULL && view->contourfullrez == MB_NO)
			|| (rez == MBV_REZ_HIGH && view->contourhirez == MB_NO)
			|| (rez == MBV_REZ_LOW && view->contourlorez == MB_NO)))
		{
		if (rez == MBV_REZ_FULL)
			do_mbview_status("Contouring data...", instance);
	    	mbview_contour(instance, rez);
		}
	

	/* get bounds of grid seen in current view */
	/*if (view->plot_done == MB_NO)
		{
		mbview_viewbounds(instance, rez);
		}*/
	
	/* do the actual openGL plotting */
	if (view->plot_done == MB_NO)
		{
		/* set projection to 2D or 3D */
		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();
		view->right = MBV_OPENGL_WIDTH / view->size2d;
		view->left = -MBV_OPENGL_WIDTH / view->size2d;
		view->top = MBV_OPENGL_WIDTH / view->aspect_ratio / view->size2d;
		view->bottom = -MBV_OPENGL_WIDTH / view->aspect_ratio / view->size2d;
		if (data->display_mode == MBV_DISPLAY_2D)
			{
			glOrtho(view->left, 
				view->right, 
				view->bottom, 
				view->top, 
				MBV_OPENGL_ZMIN2D, MBV_OPENGL_ZMAX2D);
			}
		else
			{
			gluPerspective(40.0, 
				view->aspect_ratio, 
				0.01 * MBV_OPENGL_WIDTH,
				1000 * MBV_OPENGL_WIDTH);
			}

		/* set up translations */
		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();
		if (data->display_mode == MBV_DISPLAY_2D)
			{
			glTranslated (view->offset2d_x, 
					view->offset2d_y, 
					MBV_OPENGL_ZMIN2D);
			}
		else if (data->display_mode == MBV_DISPLAY_3D)
			{
			viewdistance = 0.48 * MBV_OPENGL_WIDTH * MBV_OPENGL_WIDTH 
					/ view->aspect_ratio;
			glTranslated (0.0, 0.0, 
					-viewdistance + view->viewoffset3d_z);
			glRotated ((float)(data->viewelevation3d - 90.0), 1.0, 0.0, 0.0); 
			glRotated ((float)(data->viewazimuth3d), 0.0, 1.0, 1.0); 
			glTranslated (view->offset3d_x, 
					view->offset3d_y, 
					-viewdistance + view->offset3d_z);
			glRotated ((float)(data->modelelevation3d - 90.0), 1.0, 0.0, 0.0); 
			glRotated ((float)(data->modelazimuth3d), 0.0, 0.0, 1.0); 
			}

		/* set background color */
		glClearColor(1.0, 1.0, 1.0, 0.0);
		glClearDepth((GLclampd)(2000 * MBV_OPENGL_WIDTH));
		glDepthFunc(GL_LESS);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		/* draw data */
		if (view->plot_done == MB_NO)
			{
			if (rez == MBV_REZ_FULL)
				do_mbview_status("Drawing full rez...", instance);
			else if (rez == MBV_REZ_HIGH)
				do_mbview_status("Drawing high rez...", instance);
			mbview_drawdata(instance, rez);
			}
		}

	/* the plot_done flag will still be MB_NO if this
	   is the highest recursion level to be reached - finish the plot
	   only in this case */
	if (view->plot_done == MB_NO)
		{
		/* flush opengl buffers */
		glFlush();

		/* swap opengl buffers */
		glXSwapBuffers (XtDisplay(view->glwmda), 
				XtWindow(view->glwmda));
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
int mbview_findpoint(int instance, int xpixel, int ypixel,
			int *found, 
			double *xgrid, double *ygrid,
			double *xlon, double *ylat, double *zdata,
			double *xdisplay, double *ydisplay, double *zdisplay)
{

	/* local variables */
	char	*function_name = "mbview_findpoint";
	int	status = MB_SUCCESS;
	struct mbview_world_struct *view;
	struct mbview_struct *data;
	int	rez;
	int	ijbounds[4];
	int	foundsave;
	double	xgridsave, ygridsave;
	double	xlonsave, ylatsave, zdatasave;
	double	xdisplaysave, ydisplaysave, zdisplaysave;

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
	
	/* look for point at low resolution */
	*found = MB_NO;
	foundsave = MB_NO;
	ijbounds[0] = 0;
	ijbounds[1] = data->primary_nx;
	ijbounds[2] = 0;
	ijbounds[3] = data->primary_ny;
	rez = MBV_REZ_LOW;
	mbview_findpointrez(instance, rez, xpixel, ypixel, 
			ijbounds, found, 
			xgrid, ygrid,
			xlon, ylat, zdata,
			xdisplay, ydisplay, zdisplay);
	if (*found == MB_YES)
		{
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
	rez = MBV_REZ_HIGH;
	mbview_findpointrez(instance, rez, xpixel, ypixel, 
			ijbounds, found, 
			xgrid, ygrid, 
			xlon, ylat, zdata,
			xdisplay, ydisplay, zdisplay);
	if (*found == MB_NO && foundsave == MB_YES)
		{
		rez = MBV_REZ_LOW;
		*found = foundsave;
		*xgrid = xgridsave;
		*ygrid = ygridsave;
		*xlon = xlonsave;
		*ylat = ylatsave;
		*xdisplay = xdisplaysave;
		*ydisplay = ydisplaysave;
		*zdisplay = zdisplaysave;
		}

	/* repeat until found at highest resolution possible */
	while (*found == MB_YES
		&& ijbounds[1] > ijbounds[0]
		&& ijbounds[3] > ijbounds[2])
		{
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
		if ((ijbounds[1] - ijbounds[0]) 
				> data->hirez_dimension
			|| (ijbounds[3] - ijbounds[2]) 
				> data->hirez_dimension)
			rez = MBV_REZ_HIGH;
		else
			rez = MBV_REZ_FULL;
		
		/* try again */
		mbview_findpointrez(instance, rez, xpixel, ypixel, 
			ijbounds, found, 
			xgrid, ygrid,
			xlon, ylat, zdata,
			xdisplay, ydisplay, zdisplay);
		}

	/* use the best pick location found */
	if (*found == MB_NO && foundsave == MB_YES)
		{
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
	
	/* print output debug statements */
	if (mbv_verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       found:           %d\n",*found);
		fprintf(stderr,"dbg2       xgrid:           %f\n",*xgrid);
		fprintf(stderr,"dbg2       ygrid:           %f\n",*ygrid);
		fprintf(stderr,"dbg2       xlon:            %f\n",*xlon);
		fprintf(stderr,"dbg2       ylat:            %f\n",*ylat);
		fprintf(stderr,"dbg2       zdata:           %f\n",*zdata);
		fprintf(stderr,"dbg2       status:          %d\n",status);
		}

	/* return */
	return(status);
}

/*------------------------------------------------------------------------------*/
int mbview_findpointrez(int instance, int rez, int xpixel, int ypixel,
			int ijbounds[4], int *found, 
			double *xgrid, double *ygrid,
			double *xlon, double *ylat, double *zdata,
			double *xdisplay, double *ydisplay, double *zdisplay)
{

	/* local variables */
	char	*function_name = "mbview_findpointrez";
	int	status = MB_SUCCESS;
	struct mbview_world_struct *view;
	struct mbview_struct *data;
	int	good_pick = MB_NO;
	float	viewdistance;
	int	stride, ipickstride, jpickstride;
	int	i, j, k, l, m, n, kk, ll;
	float	rgba[4];
	int	ni, imin, imax, nj, jmin, jmax;
	int	npickx, npicky;
	float	rgb[3];

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
		fprintf(stderr,"dbg2       xpixel:           %d\n",xpixel);
		fprintf(stderr,"dbg2       ypixel:           %d\n",ypixel);
		fprintf(stderr,"dbg2       ijbounds[0]:     %d\n",ijbounds[0]);
		fprintf(stderr,"dbg2       ijbounds[1]:     %d\n",ijbounds[1]);
		fprintf(stderr,"dbg2       ijbounds[2]:     %d\n",ijbounds[2]);
		fprintf(stderr,"dbg2       ijbounds[3]:     %d\n",ijbounds[3]);
		}
		
	/* get view */
	view = &(mbviews[instance]);
	data = &(view->data);
	
	/* make correct window current for OpenGL */
	GLwDrawingAreaMakeCurrent(view->glwmda, view->glx_context);
/*fprintf(stderr,"\nmbview_findpointrez: instance:%d point:%d %d  bounds:%d %d %d %d\n", 
instance,xpixel,ypixel,ijbounds[0],ijbounds[1],ijbounds[2],ijbounds[3]);*/
	
	/* apply projection if needed */
	if (view->projected == MB_NO)
		{
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
	if (data->display_mode == MBV_DISPLAY_2D)
		{
		glOrtho(view->left, 
			view->right, 
			view->bottom, 
			view->top, 
			MBV_OPENGL_ZMIN2D, MBV_OPENGL_ZMAX2D);
		}
	else
		{
		gluPerspective(40.0, 
			view->aspect_ratio, 
			0.01 * MBV_OPENGL_WIDTH,
			1000 * MBV_OPENGL_WIDTH);
		}

	/* set up translations */
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	if (data->display_mode == MBV_DISPLAY_2D)
		{
		glTranslated (view->offset2d_x, 
				view->offset2d_y, 
				MBV_OPENGL_ZMIN2D);
		}
	else if (data->display_mode == MBV_DISPLAY_3D)
		{
		viewdistance = 0.48 * MBV_OPENGL_WIDTH * MBV_OPENGL_WIDTH 
				/ view->aspect_ratio;
		glTranslated (0.0, 0.0, 
				-viewdistance + view->viewoffset3d_z);
		glRotated ((float)(data->viewelevation3d - 90.0), 1.0, 0.0, 0.0); 
		glRotated ((float)(data->viewazimuth3d), 0.0, 1.0, 1.0); 
		glTranslated (view->offset3d_x, 
				view->offset3d_y, 
				-viewdistance + view->offset3d_z);
		glRotated ((float)(data->modelelevation3d - 90.0), 1.0, 0.0, 0.0); 
		glRotated ((float)(data->modelazimuth3d), 0.0, 0.0, 1.0); 
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
	imin = ijbounds[0];
	imax = ijbounds[1];
	ni = imax - imin + 1;
	jmin = ijbounds[2];
	jmax = ijbounds[3];
	nj = jmax - jmin + 1;

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
				
	/* get number of grid cells used in picking */
	npickx = (ni / stride);
	ipickstride = stride * (int)floor((npickx / MBVIEW_PICK_DIVISION) + 1);
	npicky = (nj / stride);
	jpickstride = stride * (int)floor((npicky / MBVIEW_PICK_DIVISION) + 1);
	
/*fprintf(stderr,"mbview_findpointrez: stride:%d npickx:%d npicky:%d ipickstride:%d jpickstride:%d\n", 
stride, npickx, npicky, ipickstride, jpickstride);*/
	
	/* draw the triangles */
	glBegin(GL_TRIANGLES);
	for (i=imin;i<imax-stride;i+=stride)
	{
	for (j=jmin;j<jmax-stride;j+=stride)
		{
		k = i * data->primary_ny + j;
		l = (i + stride) * data->primary_ny + j;
		m = i * data->primary_ny + j + stride;
		n = (i + stride) * data->primary_ny + j + stride;
		
		rgb[0] = (float)floor(((double)((i - imin) / ipickstride))) 
				/ (MBVIEW_PICK_DIVISION + 1.0);
		rgb[1] = (float)floor(((double)((j - jmin) / jpickstride)))
				/ (MBVIEW_PICK_DIVISION + 1.0);
		if (data->primary_data[k] != data->primary_nodatavalue
			&& data->primary_data[l] != data->primary_nodatavalue
			&& data->primary_data[m] != data->primary_nodatavalue)
			{
			if (!(data->primary_stat_z[k/8] & statmask[k%8]))
				mbview_zscalepoint(view,data,k);
			if (!(data->primary_stat_z[l/8] & statmask[l%8]))
				mbview_zscalepoint(view,data,l);
			if (!(data->primary_stat_z[m/8] & statmask[m%8]))
				mbview_zscalepoint(view,data,m);
			rgb[2] = 0.25;
/*fprintf(stderr,"triangle:%d %d   rgb: %f %f %f\n",
i,j, rgb[0], rgb[1], rgb[2]);*/
			glColor3f(rgb[0], rgb[1], rgb[2]);
			glVertex3f(data->primary_x[k],
				data->primary_y[k],
				data->primary_z[k]);
			glColor3f(rgb[0], rgb[1], rgb[2]);
			glVertex3f(data->primary_x[l],
				data->primary_y[l],
				data->primary_z[l]);
			glColor3f(rgb[0], rgb[1], rgb[2]);
			glVertex3f(data->primary_x[m],
				data->primary_y[m],
				data->primary_z[m]);
			}
		if (data->primary_data[l] != data->primary_nodatavalue
			&& data->primary_data[m] != data->primary_nodatavalue
			&& data->primary_data[n] != data->primary_nodatavalue)
			{
			if (!(data->primary_stat_z[l/8] & statmask[l%8]))
				mbview_zscalepoint(view,data,l);
			if (!(data->primary_stat_z[m/8] & statmask[m%8]))
				mbview_zscalepoint(view,data,m);
			if (!(data->primary_stat_z[n/8] & statmask[n%8]))
				mbview_zscalepoint(view,data,n);
			rgb[2] = 0.75;
/*fprintf(stderr,"triangle:%d %d   rgb: %f %f %f\n",
i,j, rgb[0], rgb[1], rgb[2]);*/
			glColor3f(rgb[0], rgb[1], rgb[2]);
			glVertex3f(data->primary_x[l],
				data->primary_y[l],
				data->primary_z[l]);
			glColor3f(rgb[0], rgb[1], rgb[2]);
			glVertex3f(data->primary_x[n],
				data->primary_y[n],
				data->primary_z[n]);
			glColor3f(rgb[0], rgb[1], rgb[2]);
			glVertex3f(data->primary_x[m],
				data->primary_y[m],
				data->primary_z[m]);
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
	glReadPixels(xpixel, ypixel, 1, 1, GL_RGBA, GL_FLOAT, rgba);
	glReadBuffer(GL_FRONT);

	/* calculate pick location */
	if (rgba[0] != 1.0 
		&& rgba[1] != 1.0)
		{
		*found = MB_YES;
		
		i = imin + ipickstride 
				* ((int)rint((MBVIEW_PICK_DIVISION + 1.0) * rgba[0]));
		j = jmin + jpickstride 
				* ((int)rint((MBVIEW_PICK_DIVISION + 1.0) * rgba[1]));
		k = i * data->primary_ny + j;
		l = (i + stride) * data->primary_ny + j;
		m = i * data->primary_ny + j + stride;
		n = (i + stride) * data->primary_ny + j + stride;
		if (rint((MBVIEW_PICK_DIVISION + 1.0) * rgba[2]) 
			== (MBVIEW_PICK_DIVISION + 1.0) / 4.0)
			{
			*xgrid = data->primary_xmin 
					+ (3 * i + stride) * data->primary_dx / 3.0;
			*ygrid = data->primary_ymin 
					+ (3 * j + stride) * data->primary_dy / 3.0;
			*zdata = (data->primary_data[k] 
					+ data->primary_data[l] 
					+ data->primary_data[m]) / 3.0;
			}
		else
			{
			*xgrid = data->primary_xmin 
					+ (3 * i + 2 * stride) * data->primary_dx / 3.0;
			*ygrid = data->primary_ymin 
					+ (3 * j + 2 * stride) * data->primary_dy / 3.0;
			*zdata = (data->primary_data[l] 
					+ data->primary_data[n] 
					+ data->primary_data[m]) / 3.0;
			}
/*fprintf(stderr,"pickrez:%d %d   rgb: %f %f %f %f   i:%d j:%d\n",
xpixel,ypixel, rgba[0], rgba[1], rgba[2], rgba[3], i, j);*/

		/* project grid positions to geographic and display coordinates */
		mbview_projectforward(instance, MB_YES,
					*xgrid, *ygrid,
					xlon, ylat,
					xdisplay, ydisplay);
		*zdisplay = view->zscale * (*zdata - view->zorigin);

					
		/* reset ijbounds */
		ijbounds[0] = i;
		ijbounds[2] = j;
		if (ipickstride == 1)
			{
			ijbounds[1] = i;
			ijbounds[3] = j;
			}
		else
			{
			ijbounds[1] = MIN(i + 2 * ipickstride - 1, data->primary_nx - 1);
			ijbounds[3] = MIN(j + 2 * jpickstride - 1, data->primary_ny - 1);
			}
					
/*fprintf(stderr," pickrez: %f %f %f     geo: %f %f %f\n", 
*xx, *yy, *zz, *xlon, *ylat, *zdata);*/
		}
		
	else
		{
		*found = MB_NO;
/*fprintf(stderr,"pickrez bad pick!!:%d %d   rgba: %f %f %f %f\n",
xpixel,ypixel, rgba[0], rgba[1], rgba[2], rgba[3]);*/
		}
	
	/* print output debug statements */
	if (mbv_verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       ijbounds[0]:     %d\n",ijbounds[0]);
		fprintf(stderr,"dbg2       ijbounds[1]:     %d\n",ijbounds[1]);
		fprintf(stderr,"dbg2       ijbounds[2]:     %d\n",ijbounds[2]);
		fprintf(stderr,"dbg2       ijbounds[3]:     %d\n",ijbounds[3]);
		fprintf(stderr,"dbg2       found:           %d\n",*found);
		fprintf(stderr,"dbg2       xgrid:           %f\n",*xgrid);
		fprintf(stderr,"dbg2       ygrid:           %f\n",*ygrid);
		fprintf(stderr,"dbg2       xlon:            %f\n",*xlon);
		fprintf(stderr,"dbg2       ylat:            %f\n",*ylat);
		fprintf(stderr,"dbg2       zdata:           %f\n",*zdata);
		fprintf(stderr,"dbg2       status:          %d\n",status);
		}

	/* return */
	return(status);
}


/*------------------------------------------------------------------------------*/
int mbview_viewbounds(int instance, int rez)
{

	/* local variables */
	char	*function_name = "mbview_viewbounds";
	int	status = MB_SUCCESS;
	struct mbview_world_struct *view;
	struct mbview_struct *data;
	int	good_pick = MB_NO;
	float	viewdistance;
	int	stride, ipickstride, jpickstride;
	int	i, j, k, l, m, n;
	int	xpixel, ypixel;
	int	found;
	float	rgba[4];
	int	npickx, npicky;
	float	rgb[3];

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
	
	/* make correct window current for OpenGL */
	GLwDrawingAreaMakeCurrent(view->glwmda, view->glx_context);
	
	/* apply projection if needed */
	if (view->projected == MB_NO)
		{
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
	if (data->display_mode == MBV_DISPLAY_2D)
		{
		glOrtho(view->left, 
			view->right, 
			view->bottom, 
			view->top, 
			MBV_OPENGL_ZMIN2D, MBV_OPENGL_ZMAX2D);
		}
	else
		{
		gluPerspective(40.0, 
			view->aspect_ratio, 
			0.01 * MBV_OPENGL_WIDTH,
			1000 * MBV_OPENGL_WIDTH);
		}

	/* set up translations */
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	if (data->display_mode == MBV_DISPLAY_2D)
		{
		glTranslated (view->offset2d_x, 
				view->offset2d_y, 
				MBV_OPENGL_ZMIN2D);
		}
	else if (data->display_mode == MBV_DISPLAY_3D)
		{
		viewdistance = 0.48 * MBV_OPENGL_WIDTH * MBV_OPENGL_WIDTH 
				/ view->aspect_ratio;
		glTranslated (0.0, 0.0, 
				-viewdistance + view->viewoffset3d_z);
		glRotated ((float)(data->viewelevation3d - 90.0), 1.0, 0.0, 0.0); 
		glRotated ((float)(data->viewazimuth3d), 0.0, 1.0, 1.0); 
		glTranslated (view->offset3d_x, 
				view->offset3d_y, 
				-viewdistance + view->offset3d_z);
		glRotated ((float)(data->modelelevation3d - 90.0), 1.0, 0.0, 0.0); 
		glRotated ((float)(data->modelazimuth3d), 0.0, 0.0, 1.0); 
		}

	/* set background color */
	glClearColor(1.0, 1.0, 1.0, 1.0);
	glClearDepth((GLclampd)(2000 * MBV_OPENGL_WIDTH));
	glDepthFunc(GL_LESS);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	
	/* enable depth test for 3D plots */
	if (data->display_mode == MBV_DISPLAY_3D)
		glEnable(GL_DEPTH_TEST);
	
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
				
	/* get number of grid cells used in picking */
	npickx = (data->primary_nx / stride);
	ipickstride = stride * (int)floor((npickx / MBVIEW_PICK_DIVISION) + 1);
	npicky = (data->primary_ny / stride);
	jpickstride = stride * (int)floor((npicky / MBVIEW_PICK_DIVISION) + 1);
	
fprintf(stderr,"mbview_viewbounds: stride:%d npickx:%d npicky:%d ipickstride:%d jpickstride:%d\n", 
stride, npickx, npicky, ipickstride, jpickstride);
	
	/* draw the triangles */
	glBegin(GL_TRIANGLES);
	for (i=0;i<data->primary_nx-stride;i+=stride)
	{
	for (j=0;j<data->primary_ny-stride;j+=stride)
		{
		k = i * data->primary_ny + j;
		l = (i + stride) * data->primary_ny + j;
		m = i * data->primary_ny + j + stride;
		n = (i + stride) * data->primary_ny + j + stride;
		
		rgb[0] = (float)floor(((double)(i / ipickstride))) 
				/ (MBVIEW_PICK_DIVISION + 1.0);
		rgb[1] = (float)floor(((double)(j / jpickstride)))
				/ (MBVIEW_PICK_DIVISION + 1.0);
		if (data->primary_data[k] != data->primary_nodatavalue
			&& data->primary_data[l] != data->primary_nodatavalue
			&& data->primary_data[m] != data->primary_nodatavalue)
			{
			rgb[2] = 0.25;
/*fprintf(stderr,"triangle:%d %d   rgb: %f %f %f\n",
i,j, rgb[0], rgb[1], rgb[2]);*/
			glColor3f(rgb[0], rgb[1], rgb[2]);
			glVertex3f(data->primary_x[k],
				data->primary_y[k],
				data->primary_z[k]);
			glColor3f(rgb[0], rgb[1], rgb[2]);
			glVertex3f(data->primary_x[l],
				data->primary_y[l],
				data->primary_z[l]);
			glColor3f(rgb[0], rgb[1], rgb[2]);
			glVertex3f(data->primary_x[m],
				data->primary_y[m],
				data->primary_z[m]);
			}
		if (data->primary_data[l] != data->primary_nodatavalue
			&& data->primary_data[m] != data->primary_nodatavalue
			&& data->primary_data[n] != data->primary_nodatavalue)
			{
			rgb[2] = 0.75;
/*fprintf(stderr,"triangle:%d %d   rgb: %f %f %f\n",
i,j, rgb[0], rgb[1], rgb[2]);*/
			glColor3f(rgb[0], rgb[1], rgb[2]);
			glVertex3f(data->primary_x[l],
				data->primary_y[l],
				data->primary_z[l]);
			glColor3f(rgb[0], rgb[1], rgb[2]);
			glVertex3f(data->primary_x[n],
				data->primary_y[n],
				data->primary_z[n]);
			glColor3f(rgb[0], rgb[1], rgb[2]);
			glVertex3f(data->primary_x[m],
				data->primary_y[m],
				data->primary_z[m]);
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
	found = MB_NO;
	data->viewbounds[0] = 0;
	data->viewbounds[1] = data->primary_nx - 1;
	data->viewbounds[2] = 0;
	data->viewbounds[3] = data->primary_ny - 1;
	for (xpixel = 0; xpixel < data->width; xpixel += 25)
	{
	for (ypixel = 0; ypixel < data->height; ypixel += 25)
		{
		glReadPixels(xpixel, ypixel, 1, 1, GL_RGBA, GL_FLOAT, rgba);
/*fprintf(stderr,"xpixel:%d ypixel:%d rgba: %f %f %f %f\n",
xpixel,ypixel, rgba[0], rgba[1], rgba[2], rgba[3]);*/
		if (rgba[0] != 1.0 && rgba[1] != 1.0)
			{
			i = ipickstride * ((int)rint((MBVIEW_PICK_DIVISION + 1.0) * rgba[0]));
			j = jpickstride * ((int)rint((MBVIEW_PICK_DIVISION + 1.0) * rgba[1]));
			if (found == MB_NO)
				{
				data->viewbounds[0] = i;
				data->viewbounds[1] = i + stride;
				data->viewbounds[2] = j;
				data->viewbounds[3] = j + stride;
				found = MB_YES;
				}
			else
				{
				data->viewbounds[0] = MIN(i, data->viewbounds[0]);
				data->viewbounds[1] = MAX(i + stride, data->viewbounds[1]);
				data->viewbounds[2] = MIN(j, data->viewbounds[2]);
				data->viewbounds[3] = MAX(j + stride, data->viewbounds[3]);
				}
/*fprintf(stderr,"i:%d j:%d data->viewbounds: %d %d %d %d\n",
i,j,data->viewbounds[0],
data->viewbounds[1],
data->viewbounds[2],
data->viewbounds[3]);*/
			}
		}
	}
	glReadBuffer(GL_FRONT);
	
	/* print output debug statements */
	if (mbv_verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       viewbounds[0]:   %d\n",data->viewbounds[0]);
		fprintf(stderr,"dbg2       viewbounds[1]:   %d\n",data->viewbounds[1]);
		fprintf(stderr,"dbg2       viewbounds[2]:   %d\n",data->viewbounds[2]);
		fprintf(stderr,"dbg2       viewbounds[3]:   %d\n",data->viewbounds[3]);
		fprintf(stderr,"dbg2       status:          %d\n",status);
		}

	/* return */
	return(status);
}

/*------------------------------------------------------------------------------*/
int mbview_drapesegment(int instance, struct mbview_linesegment_struct *seg)
{

	/* local variables */
	char	*function_name = "mbview_drapesegment";
	int	status = MB_SUCCESS;
	int	error = MB_ERROR_NO_ERROR;
	struct mbview_world_struct *view;
	struct mbview_struct *data;
	int	istart, iend, iadd, jstart, jend, jadd;
	int	ni, nj;
	double	mm, bb;
	int	found, done, insert;
	double	xgrid, ygrid, zdata;
	int	i, j, k, l, ii, icnt, jcnt;
	int	iii;
	

	/* print starting debug statements */
	if (mbv_verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Version %s\n",rcs_id);
		fprintf(stderr,"dbg2  MB-system Version %s\n",MB_VERSION);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       instance:         %d\n",instance);
		fprintf(stderr,"dbg2       seg:              %d\n",seg);
		fprintf(stderr,"dbg2       seg->endpoints:\n");
		fprintf(stderr,"dbg2            xgrid[0]:    %f\n",seg->endpoints[0]->xgrid);
		fprintf(stderr,"dbg2            ygrid[0]:    %f\n",seg->endpoints[0]->ygrid);
		fprintf(stderr,"dbg2            xgrid[1]:    %f\n",seg->endpoints[1]->xgrid);
		fprintf(stderr,"dbg2            ygrid[1]:    %f\n",seg->endpoints[1]->ygrid);
		}
		
	/* get view */
	view = &(mbviews[instance]);
	data = &(view->data);
	
	/* reset done flag */
	done = MB_NO;
	
	/* figure out how many points to calculate along the segment */
	istart = (int)((seg->endpoints[0]->xgrid - data->primary_xmin)
			/ data->primary_dx);
	iend = (int)((seg->endpoints[1]->xgrid - data->primary_xmin)
			/ data->primary_dx);
	jstart = (int)((seg->endpoints[0]->ygrid - data->primary_ymin)
			/ data->primary_dy);
	jend = (int)((seg->endpoints[1]->ygrid - data->primary_ymin)
			/ data->primary_dy);
			
	/* no need to fill in if the segment doesn't cross grid boundaries */
	if (istart == iend && jstart == jend)
		{
		done = MB_YES;
		seg->nls = 0;
		}
		
	/* else allocate space for the array of points */
	else
		{
		/* allocate space for the array of points */
		if (iend > istart)
			{
			ni = iend - istart;
			iadd = 1;
			istart++;
			iend++;
			}
		else
			{
			ni = istart - iend;
			iadd = -1;
			}
		if (jend > jstart)
			{
			nj = jend - jstart;
			jadd = 1;
			jstart++;
			jend++;
			}
		else
			{
			nj = jstart - jend;
			jadd = -1;
			}
		if ((ni + nj + 2) > seg->nls_alloc)
			{
			seg->nls_alloc = (ni + nj + 2);
			status = mb_realloc(mbv_verbose, 
			    		seg->nls_alloc * sizeof(struct mbview_point_struct),
			    		&(seg->lspoints), &error);
			if (status == MB_FAILURE)
				{
				done = MB_YES;
				seg->nls_alloc = 0;
				}
			}
		}
		
	/* if points needed and space allocated do it */
	if (done == MB_NO && ni + nj > 0)
		{		
		/* get line equation */
		mm = (seg->endpoints[1]->ygrid - seg->endpoints[0]->ygrid) 
			/ (seg->endpoints[1]->xgrid - seg->endpoints[0]->xgrid);
		bb = seg->endpoints[0]->ygrid - mm * seg->endpoints[0]->xgrid;

		/* put begin point in list */
		seg->nls = 0;
		seg->lspoints[seg->nls].xgrid = seg->endpoints[0]->xgrid;
		seg->lspoints[seg->nls].ygrid = seg->endpoints[0]->ygrid;
		seg->lspoints[seg->nls].zdata = seg->endpoints[0]->zdata;
		seg->nls++;
		
		/* loop over xgrid */
		insert = 1;
		for (icnt=0;icnt<ni;icnt++)
			{
			i = istart + icnt * iadd;
			xgrid = data->primary_xmin + i * data->primary_dx;
			ygrid = mm * xgrid + bb;
			j = (int)((ygrid - data->primary_ymin)
					/ data->primary_dy);
			k = i * data->primary_ny + j;
			l = i * data->primary_ny + j + 1;
			if (i >= 0 && i < data->primary_nx - 1
				&& j >= 0 && j < data->primary_ny - 1
				&& data->primary_data[k] != data->primary_nodatavalue
				&& data->primary_data[l] != data->primary_nodatavalue)
				{
				/* interpolate zdata */
				zdata = data->primary_data[k]
					+ (ygrid - data->primary_ymin 
							- j * data->primary_dy)
						/ data->primary_dy
						* (data->primary_data[l] 
							- data->primary_data[k]);
						
				/* add point to list */
				seg->lspoints[seg->nls].xgrid = xgrid;
				seg->lspoints[seg->nls].ygrid = ygrid;
				seg->lspoints[seg->nls].zdata = zdata;
				seg->nls++;
				}
			}

		/* put end point in list */
		seg->lspoints[seg->nls].xgrid = seg->endpoints[1]->xgrid;
		seg->lspoints[seg->nls].ygrid = seg->endpoints[1]->ygrid;
		seg->lspoints[seg->nls].zdata = seg->endpoints[1]->zdata;
		seg->nls++;
		
		/* loop over ygrid */
		insert = 1;
		for (jcnt=0;jcnt<nj;jcnt++)
			{
			j = jstart + jcnt * jadd;
			ygrid = data->primary_ymin + j * data->primary_dy;
			xgrid = (ygrid - bb) / mm;
			i = (int)((xgrid - data->primary_xmin)
					/ data->primary_dx);
			k = i * data->primary_ny + j;
			l = (i + 1) * data->primary_ny + j;
			if (i >= 0 && i < data->primary_nx - 1
				&& j >= 0 && j < data->primary_ny - 1
				&& data->primary_data[k] != data->primary_nodatavalue
				&& data->primary_data[l] != data->primary_nodatavalue)
				{
				/* interpolate zdata */
				zdata = data->primary_data[k]
					+ (xgrid - data->primary_xmin 
							- i * data->primary_dx)
						/ data->primary_dx
						* (data->primary_data[l] 
							- data->primary_data[k]);
				
				/* insert point into list */
				found = MB_NO;
				done = MB_NO;
				if (jadd > 0) while (done == MB_NO)
					{
					if (ygrid > seg->lspoints[insert-1].ygrid
						&& ygrid < seg->lspoints[insert].ygrid)
						{
						found = MB_YES;
						done = MB_YES;
						}
					else if (ygrid == seg->lspoints[insert-1].ygrid
						|| ygrid == seg->lspoints[insert].ygrid)
						{
						done = MB_YES;
						}
					else if (ygrid < seg->lspoints[insert-1].ygrid)
						{
						insert--;
						}
					else if (ygrid > seg->lspoints[insert].ygrid)
						{
						insert++;
						}
					if (insert <= 0 || insert >= seg->nls)
						{
						done = MB_YES;
						}
					}
				else if (jadd < 0) while (done == MB_NO)
					{
					if (ygrid > seg->lspoints[insert].ygrid
						&& ygrid < seg->lspoints[insert-1].ygrid)
						{
						found = MB_YES;
						done = MB_YES;
						}
					else if (ygrid == seg->lspoints[insert].ygrid
						|| ygrid == seg->lspoints[insert-1].ygrid)
						{
						done = MB_YES;
						}
					else if (ygrid > seg->lspoints[insert-1].ygrid)
						{
						insert--;
						}
					else if (ygrid < seg->lspoints[insert].ygrid)
						{
						insert++;
						}
					if (insert <= 0 || insert >= seg->nls)
						{
						done = MB_YES;
						}
					}
				if (found == MB_YES)
					{
					for (ii=seg->nls;ii>insert;ii--)
						{
						seg->lspoints[ii].xgrid = seg->lspoints[ii-1].xgrid;
						seg->lspoints[ii].ygrid = seg->lspoints[ii-1].ygrid;
						seg->lspoints[ii].zdata = seg->lspoints[ii-1].zdata;
						}
					seg->lspoints[insert].xgrid = xgrid;
					seg->lspoints[insert].ygrid = ygrid;
					seg->lspoints[insert].zdata = zdata;
					seg->nls++;
					}
				}
			}
			
		/* now calculate rest of point values */
		for (icnt=0;icnt<seg->nls;icnt++)
			{
			mbview_projectforward(instance, MB_YES,
				seg->lspoints[icnt].xgrid, 
				seg->lspoints[icnt].ygrid,
				&(seg->lspoints[icnt].xlon),
				&(seg->lspoints[icnt].ylat),
				&(seg->lspoints[icnt].xdisplay),
				&(seg->lspoints[icnt].ydisplay));
			seg->lspoints[icnt].zdisplay 
				= view->zscale * (seg->lspoints[icnt].zdata - view->zorigin) 
					+ 10.0 * MBV_OPENGL_3D_CONTOUR_OFFSET;
			}
		}
	
	/* print output debug statements */
	if (mbv_verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:          %d\n",status);
		fprintf(stderr,"dbg2       seg->nls:        %d\n",seg->nls);
		fprintf(stderr,"dbg2       seg->nls_alloc:  %d\n",seg->nls_alloc);
		fprintf(stderr,"dbg2       seg->lspoints:\n");
		for (i=0;i<seg->nls;i++)
			{
			fprintf(stderr,"dbg2         point[%4d]:    %f %f %f  %f %f  %f %f %f\n",
				i, seg->lspoints[i].xgrid, seg->lspoints[i].ygrid,  seg->lspoints[i].zdata,
				 seg->lspoints[i].xlon, seg->lspoints[i].ylat, 
				 seg->lspoints[i].xdisplay, seg->lspoints[i].ydisplay,
				 seg->lspoints[i].zdisplay);
			}
		}

	/* return */
	return(status);
}

/*------------------------------------------------------------------------------*/


/*------------------------------------------------------------------------------*/
