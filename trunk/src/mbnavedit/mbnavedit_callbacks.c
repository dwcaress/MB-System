/*--------------------------------------------------------------------
 *    The MB-system:	mbnavedit_callbacks.c	6/24/95
 *    $Id: mbnavedit_callbacks.c,v 4.0 1995-08-07 18:33:22 caress Exp $
 *
 *    Copyright (c) 1995 by 
 *    D. W. Caress (caress@lamont.ldgo.columbia.edu)
 *    and D. N. Chayes (dale@lamont.ldgo.columbia.edu)
 *    Lamont-Doherty Earth Observatory
 *    Palisades, NY  10964
 *
 *    See README file for copying and redistribution conditions.
 *--------------------------------------------------------------------*/
/*
 * MBNAVEDIT is an interactive navigation editor for swath sonar data.
 * It can work with any data format supported by the MBIO library.
 * This file contains the callback functions from the MOTIF interface.
 *
 * Author:	D. W. Caress
 * Date:	June 24,  1995
 *
 * $Log: not supported by cvs2svn $
 *
 */

/*--------------------------------------------------------------------*/

/* include files */
#include <stdio.h>
#include <Xm/Xm.h>
#include <X11/cursorfont.h>
#include "mbnavedit_creation.h"
#include "mbnavedit_extrawidgets.h"
#include "mbnavedit.h"

/*--------------------------------------------------------------------*/
/*
 * Standard includes for builtins.
 */
#include <string.h>
#include <ctype.h>

/*
 * Macros to make code look nicer between ANSI and K&R.
 */
#ifndef ARGLIST
#if (NeedFunctionPrototypes == 0)
#define PROTOTYPE(p)	()
#define ARGLIST(p)	p
#define ARG(a, b)	a b;
#define GRA(a, b)	a b;
#define UARG(a, b)      a b;
#define GRAU(a, b)      a b;
#else
#define PROTOTYPE(p)	p
#define ARGLIST(p)	(
#define ARG(a, b)	a b,
#define GRA(a, b)	a b)
#ifdef __cplusplus
#define UARG(a, b)      a,
#define GRAU(a, b)      a)
#else
#define UARG(a, b)      a b,
#define GRAU(a, b)      a b)
#endif
#endif
#endif

Widget		BxFindTopShell PROTOTYPE((Widget));
WidgetList	BxWidgetIdsFromNames PROTOTYPE((Widget, char*, char*));

/*--------------------------------------------------------------------*/
/*      Function Name:	BxExitCB
 *
 *      Description:   	This functions expects an integer to be passed in
 *		       	client data.  It calls the exit() system call with
 *			the integer value as the argument to the function.
 *
 *      Arguments:      Widget		w: 	the activating widget.
 *			XtPointer	client:	the integer exit value.
 *			XtPointer	call:	the call data (unused).
 */

/* ARGSUSED */
void
BxExitCB ARGLIST((w, client, call))
UARG( Widget, w)
ARG( XtPointer, client)
GRAU( XtPointer, call)
{
    int		status;
    int		exitValue = (int)client;

    /* finish with the current file */
    status = mbnavedit_action_quit();
    if (status == 0) mbnavedit_bell(100);
    
    exit(exitValue);
}
/*--------------------------------------------------------------------*/
/*      Function Name: 	BxManageCB
 *
 *      Description:   	Given a string of the form:
 *		       	"(WL)[widgetName, widgetName, ...]"
 *			BxManageCB attempts to convert the name to a Widget
 *			ID and manage the widget.
 *
 *      Arguments:     	Widget	    w:      the widget activating the callback.
 *		       	XtPointer   client: the list of widget names to attempt
 *					    to find and manage.
 *		       	XtPointer   call:   the call data (unused).
 *
 *      Notes:        *	This function expects that there is an application
 *		       	shell from which all other widgets are descended.
 */

/* ARGSUSED */
void
BxManageCB ARGLIST((w, client, call))
ARG( Widget, w)
ARG( XtPointer, client)
GRAU( XtPointer, call)
{
    WidgetList		widgets;
    int			i;

    /*
     * This function returns a NULL terminated WidgetList.  The memory for
     * the list needs to be freed when it is no longer needed.
     */
    widgets = BxWidgetIdsFromNames(w, "BxManageCB", (String)client);

    i = 0;
    while( widgets && widgets[i] != NULL )
    {
	XtManageChild(widgets[i]);
	i++;
    }
    XtFree((char *)widgets);
}
/*--------------------------------------------------------------------*/
/*      Function Name: 	BxUnmanageCB
 *
 *      Description:   	Given a string of the form:
 *		       	"(WL)[widgetName, widgetName, ...]"
 *			BxUnmanageCB attempts to convert the name to a Widget
 *			ID and unmanage the widget.
 *
 *      Arguments:     	Widget	    w:      the widget activating the callback.
 *		       	XtPointer   client: the list of widget names to attempt
 *					    to find and unmanage.
 *		       	XtPointer   call:   the call data (unused).
 *
 *      Notes:        *	This function expects that there is an application
 *		       	shell from which all other widgets are descended.
 */

/* ARGSUSED */
void
BxUnmanageCB ARGLIST((w, client, call))
ARG( Widget, w)
ARG( XtPointer, client)
GRAU( XtPointer, call)
{
    WidgetList		widgets;
    int			i;

    /*
     * This function returns a NULL terminated WidgetList.  The memory for
     * the list needs to be freed when it is no longer needed.
     */
    widgets = BxWidgetIdsFromNames(w, "BxUnmanageCB", (String)client);

    i = 0;
    while( widgets && widgets[i] != NULL )
    {
	XtUnmanageChild(widgets[i]);
	i++;
    }
    XtFree((char *)widgets);

	/* replot */
	mbnavedit_plot_all();

	/* replot */
	mbnavedit_plot_all();

	/* replot */
	mbnavedit_plot_all();
}
/*--------------------------------------------------------------------*/
/*      Function Name:	BxSetValuesCB
 *
 *      Description:   	This function accepts a string of the form:
 *			"widgetName.resourceName = value\n..."
 *			It then attempts to convert the widget name to a widget
 *			ID and the value to a valid resource value.  It then
 *			sets the value on the given widget.
 *
 *      Arguments:      Widget		w:	the activating widget.
 *			XtPointer	client:	the set values string.
 *			XtPointer	call:	the call data (unused).
 *
 *      Notes:        * This function expects that there is an application
 *                      shell from which all other widgets are descended.
 */
#include <X11/StringDefs.h>

/* ARGSUSED */
void
BxSetValuesCB ARGLIST((w, client, call))
ARG( Widget, w)
ARG( XtPointer, client)
GRAU( XtPointer, call)
{
#define CHUNK	512

    Boolean 	first = True;
    String 	rscs = XtNewString((String)client);
    String 	*valueList = (String *)XtCalloc(CHUNK, sizeof(String));
    char 	*start;
    char 	*ptr, *cptr;
    String 	name;
    String 	rsc;
    int 	i, count = 0;
    Widget 	*current;

    for ( start = rscs; rscs && *rscs; rscs = strtok(NULL, "\n"))
    {
        if ( first )
        {
            rscs = strtok(rscs, "\n");
            first = False;
        }
        valueList[count] = XtNewString(rscs);
        count++;
        if ( count == CHUNK )
        {
            valueList = 
		(String *)XtRealloc((char *)valueList,
				    (count + CHUNK) * sizeof(String));
        }
    }
    XtFree((char *)start);

    for ( i = 0; i < count; i++ )
    {
	/*
	 * First, extract the widget name and generate a string to 
	 * pass to BxWidgetIdsFromNames().
	 */
	cptr = strrchr(valueList[i], '.');
	if ( cptr != NULL )
	{
	    *cptr = '\000';
	}
	else
	{
	    printf("Callback Error (BxSetValuesCB):\n\t\
Syntax Error - specify BxSetValuesCB data as\n\t\
    <Widget Name>.<Resource> = <Value>\n");
	    XtFree((char *)(valueList[i]));
	    continue;
	}
	name = valueList[i];
	while( (name && *name) && isspace(*name) )
	{
	    name++;
	}
	ptr = name + strlen(name) - 1;
	while( ptr && *ptr )
	{
            if ( isspace(*ptr) )
            {
                ptr--;
            }
            else
            {
                ptr++;
                break;
            }
        }
        if ( ptr && *ptr )
        {
            *ptr = '\0';
        }
	if ( ptr == NULL )
	{
	    printf("Callback Error (BxSetValuesCB):\n\t\
Syntax Error - specify BxSetValuesCB data as\n\t\
    <Widget Name>.<Resource> = <Value>\n");
	    XtFree((char *)(valueList[i]));
	    return;
	}
	
	/*
	 * Next, get the resource name to set.
	 */
	rsc = ++cptr;
	cptr = strchr(rsc, '=');
	if ( cptr != NULL )
	{
	    *cptr = '\000';
	}
	else
	{
	    printf("Callback Error (BxSetValuesCB):\n\t\
Syntax Error - specify BxSetValuesCB data as\n\t\
    <Widget Name>.<Resource> = <Value>\n");
	    XtFree((char *)(valueList[i]));
	    continue;
	}
        while( (rsc && *rsc) && isspace(*rsc) )
        {
            rsc++;
        }
	
        ptr = rsc + strlen(rsc) - 1;
        while( ptr && *ptr )
        {
            if ( isspace(*ptr) )
            {
                ptr--;
            }
            else
            {
                ptr++;
                break;
            }
        }
        if ( ptr && *ptr )
        {
            *ptr = '\0';
        }
	
	/*
	 * Lastly, get the string value to which to set the resource.
	 */
	start = ++cptr;
        while( (start && *start) && isspace(*start) )
        {
            start++;
        }

	if ( start == NULL )
	{
	    printf("Callback Error (BxSetValuesCB):\n\t\
Syntax Error - specify BxSetValuesCB data as\n\t\
    <Widget Name>.<Resource> = <Value>\n");
	    XtFree((char *)(valueList[i]));
	    return;
	}
	
        ptr = start + strlen(start) - 1;
        while( ptr && *ptr )
        {
            if ( isspace(*ptr) )
            {
                ptr--;
            }
            else
            {
                ptr++;
                break;
            }
        }
        if ( ptr && *ptr )
        {
            *ptr = '\0';
        }
	
	/*
	 * Now convert the widget name to a Widget ID
	 */
	current = BxWidgetIdsFromNames(w, "BxSetValuesCB", name);
        if ( current[0] == NULL )
        {
	    XtFree((char *)(valueList[i]));
	    continue;
        }
	
	/*
	 * If the widget name conversion succeeded, we now need to get the
	 * resource list for the widget so that we can do a resource conversion
	 * of the value.
	 */
	XtVaSetValues(*current, XtVaTypedArg, rsc, XtRString, start,
		      strlen(start)+1, NULL);
        XtFree((char *)(valueList[i]));
    }
    XtFree((char *)valueList);

#undef CHUNK
}


/*--------------------------------------------------------------------*/
/* ARGSUSED */
void
do_openmb(w, client, call)
Widget w;
XtPointer client;
XtPointer call;
{
	/*SUPPRESS 594*/XmAnyCallbackStruct *acs=(XmAnyCallbackStruct*)call;

}

/*--------------------------------------------------------------------*/
/* ARGSUSED */
void
do_opennav(w, client, call)
Widget w;
XtPointer client;
XtPointer call;
{
	/*SUPPRESS 594*/XmAnyCallbackStruct *acs=(XmAnyCallbackStruct*)call;

}

/*--------------------------------------------------------------------*/
/* ARGSUSED */
void
do_nextbuffer(w, client, call)
Widget w;
XtPointer client;
XtPointer call;
{
	int	status;
	
	/*SUPPRESS 594*/XmAnyCallbackStruct *acs=(XmAnyCallbackStruct*)call;

	/* get next buffer */
	status = mbnavedit_action_next_buffer();
	if (status == 0) mbnavedit_bell(100);
	do_unset_interval();
}

/*--------------------------------------------------------------------*/
/* ARGSUSED */
void
do_done(w, client, call)
Widget w;
XtPointer client;
XtPointer call;
{
	int	status;
	
	/*SUPPRESS 594*/XmAnyCallbackStruct *acs=(XmAnyCallbackStruct*)call;

	/* finish with the current file */
	status = mbnavedit_action_done();
	if (status == 0) mbnavedit_bell(100);
	do_unset_interval();
}

/*--------------------------------------------------------------------*/
/* ARGSUSED */
void
do_forward(w, client, call)
Widget w;
XtPointer client;
XtPointer call;
{
	int	status;
	
	/*SUPPRESS 594*/XmAnyCallbackStruct *acs=(XmAnyCallbackStruct*)call;

	/* step forward */
	status = mbnavedit_action_step(data_step_size);
	if (status == 0) mbnavedit_bell(100);
	do_unset_interval();
}

/*--------------------------------------------------------------------*/
/* ARGSUSED */
void
do_reverse(w, client, call)
Widget w;
XtPointer client;
XtPointer call;
{
	int	status;
	
	/*SUPPRESS 594*/XmAnyCallbackStruct *acs=(XmAnyCallbackStruct*)call;

	/* step back */
	status = mbnavedit_action_step(-data_step_size);
	if (status == 0) mbnavedit_bell(100);
	do_unset_interval();
}

/*--------------------------------------------------------------------*/
/* ARGSUSED */
void
do_timespan(w, client, call)
Widget w;
XtPointer client;
XtPointer call;
{
	char	label[10];

	/*SUPPRESS 594*/XmAnyCallbackStruct *acs=(XmAnyCallbackStruct*)call;

	/* get values */
	XtVaGetValues(scale_timespan, 
			XmNmaximum, &data_show_max, 
			XmNvalue, &data_show_size, 
			NULL);

	/* reset maximum if necessary */
	if (data_show_size == data_show_max
		|| data_show_size == 1)
		{
		if (data_show_size == data_show_max)
			data_show_max = 2 * data_show_max;
		else if (data_show_size == 1)
			data_show_max = data_show_max/2;
		if (data_show_max < 10)
			data_show_max = 10;
		XtVaSetValues(scale_timespan, 
			XmNmaximum, data_show_max, 
			NULL);
		sprintf(label, "%d", data_show_max);
		XtVaSetValues(label_timespan_2, 
			XtVaTypedArg, XmNlabelString, 
			    XmRString, label, (strlen(label) + 1), 
			NULL);	
		}

	/* replot */
	mbnavedit_plot_all();

}

/*--------------------------------------------------------------------*/
/* ARGSUSED */
void
do_timestep(w, client, call)
Widget w;
XtPointer client;
XtPointer call;
{
	char	label[10];

	/*SUPPRESS 594*/XmAnyCallbackStruct *acs=(XmAnyCallbackStruct*)call;

	/* get values */
	XtVaGetValues(scale_timestep, 
			XmNmaximum, &data_step_max, 
			XmNvalue, &data_step_size, 
			NULL);

	/* reset maximum if necessary */
	if (data_step_size == data_step_max
		|| data_step_size == 1)
		{
		if (data_step_size == data_step_max)
			data_step_max = 2 * data_step_max;
		else if (data_step_size == 1)
			data_step_max = data_step_max/2;
		if (data_step_max < 10)
			data_step_max = 10;
		XtVaSetValues(scale_timestep, 
			XmNmaximum, data_step_max, 
			NULL);
		sprintf(label, "%d", data_step_max);
		XtVaSetValues(label_timestep_2, 
			XtVaTypedArg, XmNlabelString, 
			    XmRString, label, (strlen(label) + 1), 
			NULL);	
		}

}

/*--------------------------------------------------------------------*/
/* ARGSUSED */
void
do_expose(w, client, call)
Widget w;
XtPointer client;
XtPointer call;
{
	/*SUPPRESS 594*/XmAnyCallbackStruct *acs=(XmAnyCallbackStruct*)call;

}

/*--------------------------------------------------------------------*/
/* ARGSUSED */
void
do_event(w, client, call)
Widget w;
XtPointer client;
XtPointer call;
{
	XmDrawingAreaCallbackStruct *cbs;
	XEvent  *event;
	static Position x_loc, y_loc;
	unsigned int mask;
	KeySym	keysym;
	int	key_num;
	char	buffer[1];
	int	actual;
	int	win_x, win_y;
	int	repeat;
	char	label[10];
	int	status;

	/*SUPPRESS 594*/XmAnyCallbackStruct *acs=(XmAnyCallbackStruct*)call;

	/* get event */
	cbs = (XmDrawingAreaCallbackStruct *) call;
	event = cbs->event;

	/* If there is input in the drawing area */
	if (cbs->reason == XmCR_INPUT)
	{
	  /* Deal with KeyPress events */
	  if(event->xany.type == KeyPress)
	  {
	  /* Get key pressed - buffer[0] */
	  actual = XLookupString((XKeyEvent *)event, 
			buffer, 1, &keysym, NULL);

	  /* process events */
	  switch (buffer[0])
		{
		case 'Y':
		case 'y':
		case 'Q':
		case 'q':
				{
				mode_pick = PICK_MODE_PICK;
				do_unset_interval();
				XmToggleButtonSetState(toggleButton_pick, 
					TRUE, TRUE);
				mbnavedit_pickcursor();
				}
			break;
		case 'U':
		case 'u':
		case 'W':
		case 'w':
				{
				mode_pick = PICK_MODE_SELECT;
				do_unset_interval();
				XmToggleButtonSetState(toggleButton_select, 
					TRUE, TRUE);
				mbnavedit_selectcursor();
				}
			break;
		case 'I':
		case 'i':
		case 'E':
		case 'e':
				{
				mode_pick = PICK_MODE_DESELECT;
				do_unset_interval();
				XmToggleButtonSetState(toggleButton_deselect, 
					TRUE, TRUE);
				mbnavedit_deselectcursor();
				}
			break;
		case 'O':
		case 'o':
		case 'R':
		case 'r':
				{
				mode_pick = PICK_MODE_SELECTALL;
				do_unset_interval();
				XmToggleButtonSetState(toggleButton_selectall, 
					TRUE, TRUE);
				mbnavedit_selectallcursor();
				}
			break;
		case 'P':
		case 'p':
		case 'T':
		case 't':
				{
				mode_pick = PICK_MODE_DESELECTALL;
				do_unset_interval();
				XmToggleButtonSetState(toggleButton_deselectall, 
					TRUE, TRUE);
				mbnavedit_deselectallcursor();
				}
			break;
		default:
			break;
	      } /* end of key switch */

	   } /* end of key press events */

	  /* Deal with KeyRelease events */
	  if(event->xany.type == KeyRelease)
	  {
	  /* Get key pressed - buffer[0] */
	  actual = XLookupString((XKeyEvent *)event, 
			buffer, 1, &keysym, NULL);

	  /* process events */
	  switch (buffer[0])
		{
		default:
			break;
	      } /* end of key switch */

	   } /* end of key release events */

	  /* Check for mouse pressed and not pressed and released. */
	  if(event->xany.type == ButtonPress)
	  {
	      /* If left mouse button is pushed then 
		    pick, erase, restore or set time interval. */
	      if(event->xbutton.button == 1)
	      {
		x_loc = event->xbutton.x;
		y_loc = event->xbutton.y;

		do
		    {
	            if (mode_set_interval == MB_YES)
			{
			status = mbnavedit_action_set_interval(
				x_loc, y_loc, 0);
			if (status == MB_FAILURE)
				mbnavedit_bell(100);
			}
	            else if (mode_pick == PICK_MODE_PICK)
			status = mbnavedit_action_mouse_pick(
				x_loc, y_loc);
		    else if (mode_pick == PICK_MODE_SELECT) 
			status = mbnavedit_action_mouse_select(
				x_loc, y_loc);
		    else if (mode_pick == PICK_MODE_DESELECT) 
			status = mbnavedit_action_mouse_deselect(
				x_loc, y_loc);
		    else if (mode_pick == PICK_MODE_SELECTALL) 
			status = mbnavedit_action_mouse_selectall(
				x_loc, y_loc);
		    else if (mode_pick == PICK_MODE_DESELECTALL) 
			status = mbnavedit_action_mouse_deselectall(
				x_loc, y_loc);

		    /* get current cursor position */
		    mbnavedit_get_position(&win_x, &win_y, &mask);
		    x_loc = win_x;
		    y_loc = win_y;

		    /* If the button is still pressed then read the location */
		    /* of the pointer and run the action mouse function again */
		    if (mask == 256 && mode_pick != PICK_MODE_PICK
			    && mode_set_interval == MB_NO)
			   repeat = MB_YES;
		    else
			   repeat = MB_NO;
		    }
		while (repeat == MB_YES);

		} /* end of left button events */

		/* If middle mouse button is pushed. */
		if(event->xbutton.button == 2)
		{
		    /* get current cursor position */
		    mbnavedit_get_position(&win_x, &win_y, &mask);
		    x_loc = win_x;
		    y_loc = win_y;

		    /* set second interval bound */
	            if (mode_set_interval == MB_YES)
			{
			status = mbnavedit_action_set_interval(
				x_loc, y_loc, 1);
			if (status == MB_FAILURE)
				mbnavedit_bell(100);
			}

		    /* scroll in reverse */
		    else
			{
			status = mbnavedit_action_step(-data_step_size);
			if (status == 0) mbnavedit_bell(100);
			}
		} /* end of middle button events */

		/* If right mouse button is pushed. */
		if(event->xbutton.button == 3)
		{
		    /* apply interval bounds */
	            if (mode_set_interval == MB_YES)
			{
			status = mbnavedit_action_set_interval(
				0, 0, 2);
			if (status == MB_FAILURE)
				mbnavedit_bell(100);
			do_unset_interval();

			/* set values of number of data shown slider */
			XtVaSetValues(scale_timespan, 
				XmNminimum, 1, 
				XmNmaximum, data_show_max, 
				XmNvalue, data_show_size, 
				NULL);
			sprintf(label, "%d", data_show_max);
			XtVaSetValues(label_timespan_2, 
				XtVaTypedArg, XmNlabelString, 
				XmRString, label, (strlen(label) + 1), 
				NULL);
			}

		    /* scroll forward */
		    else
			{
			status = mbnavedit_action_step(data_step_size);
			if (status == 0) mbnavedit_bell(100);
			}
		} /* end of right button events */	
	  } /* end of button pressed events */
	} /* end of inputs from window */
	
}

/*--------------------------------------------------------------------*/
/* ARGSUSED */
void
do_resize(w, client, call)
Widget w;
XtPointer client;
XtPointer call;
{
	/*SUPPRESS 594*/XmAnyCallbackStruct *acs=(XmAnyCallbackStruct*)call;

}

/*--------------------------------------------------------------------*/
/* ARGSUSED */
void
do_toggle_lon(w, client, call)
Widget w;
XtPointer client;
XtPointer call;
{
	int	screen_height;

	/*SUPPRESS 594*/XmAnyCallbackStruct *acs=(XmAnyCallbackStruct*)call;

	plot_lon = XmToggleButtonGetState(toggleButton_lon);
	if (plot_lon == MB_YES)
		XtManageChild(toggleButton_org_lon);
	else
		{
		XtUnmanageChild(toggleButton_org_lon);
		mbnavedit_action_deselect_all(PLOT_LONGITUDE);
		}

	/* get and set size of canvas */
	number_plots = 0;
	if (plot_lon == MB_YES)
		number_plots++;
	if (plot_lat == MB_YES)
		number_plots++;
	if (plot_speed == MB_YES)
		number_plots++;
	if (plot_heading == MB_YES)
		number_plots++;
	screen_height = number_plots*plot_height;
	if (screen_height <= 0)
		screen_height = plot_height;
	XtVaSetValues(drawingArea, 
			XmNwidth, plot_width, 
			XmNheight, screen_height, 
			NULL);	

	/* replot */
	mbnavedit_plot_all();
}

/*--------------------------------------------------------------------*/
/* ARGSUSED */
void
do_toggle_lat(w, client, call)
Widget w;
XtPointer client;
XtPointer call;
{
	int	screen_height;

	/*SUPPRESS 594*/XmAnyCallbackStruct *acs=(XmAnyCallbackStruct*)call;

	plot_lat = XmToggleButtonGetState(toggleButton_lat);
	if (plot_lat == MB_YES)
		XtManageChild(toggleButton_org_lat);
	else
		{
		XtUnmanageChild(toggleButton_org_lat);
		mbnavedit_action_deselect_all(PLOT_LATITUDE);
		}

	/* get and set size of canvas */
	number_plots = 0;
	if (plot_lon == MB_YES)
		number_plots++;
	if (plot_lat == MB_YES)
		number_plots++;
	if (plot_speed == MB_YES)
		number_plots++;
	if (plot_heading == MB_YES)
		number_plots++;
	screen_height = number_plots*plot_height;
	if (screen_height <= 0)
		screen_height = plot_height;
	XtVaSetValues(drawingArea, 
			XmNwidth, plot_width, 
			XmNheight, screen_height, 
			NULL);	

	/* replot */
	mbnavedit_plot_all();
}

/*--------------------------------------------------------------------*/
/* ARGSUSED */
void
do_toggle_heading(w, client, call)
Widget w;
XtPointer client;
XtPointer call;
{
	int	screen_height;

	/*SUPPRESS 594*/XmAnyCallbackStruct *acs=(XmAnyCallbackStruct*)call;

	plot_heading = XmToggleButtonGetState(toggleButton_heading);
	if (plot_heading == MB_YES)
		{
		XtManageChild(toggleButton_org_heading);
		XtManageChild(toggleButton_show_cmg);
		XtManageChild(pushButton_heading_cmg);
		}
	else
		{
		XtUnmanageChild(toggleButton_org_heading);
		XtUnmanageChild(toggleButton_show_cmg);
		XtUnmanageChild(pushButton_heading_cmg);
		mbnavedit_action_deselect_all(PLOT_HEADING);
		}

	/* get and set size of canvas */
	number_plots = 0;
	if (plot_lon == MB_YES)
		number_plots++;
	if (plot_lat == MB_YES)
		number_plots++;
	if (plot_speed == MB_YES)
		number_plots++;
	if (plot_heading == MB_YES)
		number_plots++;
	screen_height = number_plots*plot_height;
	if (screen_height <= 0)
		screen_height = plot_height;
	XtVaSetValues(drawingArea, 
			XmNwidth, plot_width, 
			XmNheight, screen_height, 
			NULL);	

	/* replot */
	mbnavedit_plot_all();
}

/*--------------------------------------------------------------------*/
/* ARGSUSED */
void
do_toggle_speed(w, client, call)
Widget w;
XtPointer client;
XtPointer call;
{
	int	screen_height;

	/*SUPPRESS 594*/XmAnyCallbackStruct *acs=(XmAnyCallbackStruct*)call;

	plot_speed = XmToggleButtonGetState(toggleButton_speed);
	if (plot_speed == MB_YES)
		{
		XtManageChild(toggleButton_org_speed);
		XtManageChild(toggleButton_show_smg);
		XtManageChild(pushButton_speed_smg);
		}
	else
		{
		XtUnmanageChild(toggleButton_org_speed);
		XtUnmanageChild(toggleButton_show_smg);
		XtUnmanageChild(pushButton_speed_smg);
		mbnavedit_action_deselect_all(PLOT_SPEED);
		}

	/* get and set size of canvas */
	number_plots = 0;
	if (plot_lon == MB_YES)
		number_plots++;
	if (plot_lat == MB_YES)
		number_plots++;
	if (plot_speed == MB_YES)
		number_plots++;
	if (plot_heading == MB_YES)
		number_plots++;
	screen_height = number_plots*plot_height;
	if (screen_height <= 0)
		screen_height = plot_height;
	XtVaSetValues(drawingArea, 
			XmNwidth, plot_width, 
			XmNheight, screen_height, 
			NULL);	

	/* replot */
	mbnavedit_plot_all();
}

/*--------------------------------------------------------------------*/
/* ARGSUSED */
void
do_toggle_org_lon(w, client, call)
Widget w;
XtPointer client;
XtPointer call;
{
	/*SUPPRESS 594*/XmAnyCallbackStruct *acs=(XmAnyCallbackStruct*)call;

	plot_lon_org = XmToggleButtonGetState(toggleButton_org_lon);

	/* replot */
	mbnavedit_plot_all();
}

/*--------------------------------------------------------------------*/
/* ARGSUSED */
void
do_toggle_org_lat(w, client, call)
Widget w;
XtPointer client;
XtPointer call;
{
	/*SUPPRESS 594*/XmAnyCallbackStruct *acs=(XmAnyCallbackStruct*)call;

	plot_lat_org = XmToggleButtonGetState(toggleButton_org_lat);

	/* replot */
	mbnavedit_plot_all();
}

/*--------------------------------------------------------------------*/
/* ARGSUSED */
void
do_toggle_org_speed(w, client, call)
Widget w;
XtPointer client;
XtPointer call;
{
	/*SUPPRESS 594*/XmAnyCallbackStruct *acs=(XmAnyCallbackStruct*)call;

	plot_speed_org = XmToggleButtonGetState(toggleButton_org_speed);

	/* replot */
	mbnavedit_plot_all();
}

/*--------------------------------------------------------------------*/
/* ARGSUSED */
void
do_toggle_show_smg(w, client, call)
Widget w;
XtPointer client;
XtPointer call;
{
	/*SUPPRESS 594*/XmAnyCallbackStruct *acs=(XmAnyCallbackStruct*)call;

	plot_smg = XmToggleButtonGetState(toggleButton_show_smg);

	/* replot */
	mbnavedit_plot_all();
}

/*--------------------------------------------------------------------*/
/* ARGSUSED */
void
do_toggle_org_heading(w, client, call)
Widget w;
XtPointer client;
XtPointer call;
{
	/*SUPPRESS 594*/XmAnyCallbackStruct *acs=(XmAnyCallbackStruct*)call;

	plot_heading_org = XmToggleButtonGetState(toggleButton_org_heading);

	/* replot */
	mbnavedit_plot_all();
}

/*--------------------------------------------------------------------*/
/* ARGSUSED */
void
do_toggle_show_cmg(w, client, call)
Widget w;
XtPointer client;
XtPointer call;
{
	/*SUPPRESS 594*/XmAnyCallbackStruct *acs=(XmAnyCallbackStruct*)call;

	plot_cmg = XmToggleButtonGetState(toggleButton_show_cmg);

	/* replot */
	mbnavedit_plot_all();
}

/*--------------------------------------------------------------------*/
/* ARGSUSED */
void
do_button_use_smg(w, client, call)
Widget w;
XtPointer client;
XtPointer call;
{
	/*SUPPRESS 594*/XmAnyCallbackStruct *acs=(XmAnyCallbackStruct*)call;

	/* Use speed made good for selected speed values */
	mbnavedit_action_use_smg();

	/* replot */
	mbnavedit_plot_all();
}

/*--------------------------------------------------------------------*/
/* ARGSUSED */
void
do_button_use_cmg(w, client, call)
Widget w;
XtPointer client;
XtPointer call;
{
	/*SUPPRESS 594*/XmAnyCallbackStruct *acs=(XmAnyCallbackStruct*)call;

	/* Use course made good for selected heading values */
	mbnavedit_action_use_cmg();

	/* replot */
	mbnavedit_plot_all();
}

/*--------------------------------------------------------------------*/
/* ARGSUSED */
void
do_toggle_output_on(w, client, call)
Widget w;
XtPointer client;
XtPointer call;
{
	/*SUPPRESS 594*/XmAnyCallbackStruct *acs=(XmAnyCallbackStruct*)call;

	if (XmToggleButtonGetState(toggleButton_output_on))
		output_mode = OUTPUT_MODE_OUTPUT;
	else
		output_mode = OUTPUT_MODE_BROWSE;
}

/*--------------------------------------------------------------------*/
/* ARGSUSED */
void
do_toggle_output_off(w, client, call)
Widget w;
XtPointer client;
XtPointer call;
{
	/*SUPPRESS 594*/XmAnyCallbackStruct *acs=(XmAnyCallbackStruct*)call;

	if (XmToggleButtonGetState(toggleButton_output_off))
		output_mode = OUTPUT_MODE_BROWSE;
	else
		output_mode = OUTPUT_MODE_OUTPUT;
}

/*--------------------------------------------------------------------*/
/* ARGSUSED */
void
do_fileselection_cancel(w, client, call)
Widget w;
XtPointer client;
XtPointer call;
{
	/*SUPPRESS 594*/XmAnyCallbackStruct *acs=(XmAnyCallbackStruct*)call;
}

/*--------------------------------------------------------------------*/
/* ARGSUSED */
void
do_fileselection_ok(w, client, call)
Widget w;
XtPointer client;
XtPointer call;
{
	static char *selection_text;
	char	*suffix;
	int	len;
	int	form;
	int	status;
	char	label[10];

	/*SUPPRESS 594*/XmAnyCallbackStruct *acs=(XmAnyCallbackStruct*)call;

	/* get input filename */
	strncpy(ifile, 
		(char *)XmTextGetString(fileSelectionBox_text), 128);	

	/* get output filename */
	strncpy(ofile, 
		(char *)XmTextGetString(textField_output_file), 128);
	ofile_defined = MB_YES;

	/* get the file format */
	if (sscanf((char *) XmTextGetString(textField_format), 
		"%d", &form) == 1)
		format = form;

	/* open the file */
	status = mbnavedit_action_open();
	if (status == MB_FAILURE) mbnavedit_bell(100);
	do_unset_interval();

	/* set values of number of data shown slider */
	XtVaSetValues(scale_timespan, 
			XmNminimum, 1, 
			XmNmaximum, data_show_max, 
			XmNvalue, data_show_size, 
			NULL);
	sprintf(label, "%d", data_show_max);
	XtVaSetValues(label_timespan_2, 
			XtVaTypedArg, XmNlabelString, 
			    XmRString, label, (strlen(label) + 1), 
			NULL);

	/* unmanage fileselection window so plotting will show */
	XtUnmanageChild(xmDialogShell_fileselection);

	/* replot */
	if (status == MB_SUCCESS)
		mbnavedit_plot_all();
}

/*--------------------------------------------------------------------*/
/* ARGSUSED */
void
do_fileselection_filter(w, client, call)
Widget w;
XtPointer client;
XtPointer call;
{
	/*SUPPRESS 594*/XmAnyCallbackStruct *acs=(XmAnyCallbackStruct*)call;

}

/*--------------------------------------------------------------------*/
/* ARGSUSED */
void
do_fileselection_list(w, client, call)
Widget w;
XtPointer client;
XtPointer call;
{
	static char *selection_text;
	char	*suffix;
	int	len;
	int	form;
	char	value_text[10];

	/*SUPPRESS 594*/XmAnyCallbackStruct *acs=(XmAnyCallbackStruct*)call;

	/* get selected text */
	selection_text = (char *) XmTextGetString(fileSelectionBox_text);

	/* get output file */
	if((int)strlen(selection_text) > 0)
		{
		/* look for MB suffix convention */
		if ((suffix = strstr(selection_text,".mb")) != NULL)
			len = strlen(suffix);

		/* if MB suffix convention used keep it */
		if (len >= 4 && len <= 5)
			{
			/* get the output filename */
			strncpy(ofile,"\0",128);
			strncpy(ofile,selection_text,
				strlen(selection_text)-len);
			strcat(ofile,"n");
			strcat(ofile,suffix);

			/* get the file format and set the widget */
			if (sscanf(&suffix[3], "%d", &form) == 1)
				{
				format = form;
				sprintf(value_text,"%2.2d",format);
				XmTextFieldSetString(
				    textField_format, 
				    value_text);
				}
			}

		/* else just at ".ned" to file name */
		else
			{
			strcpy(ofile,selection_text);
			strcat(ofile,".ned");
			}

		/* now set the output filename text widget */
		XmTextFieldSetString(textField_output_file, 
			ofile);
		XmTextFieldSetCursorPosition(textField_output_file, 
			strlen(ofile));
	}
}

/*--------------------------------------------------------------------*/
/* ARGSUSED */
void
do_fileselection_nomatch(w, client, call)
Widget w;
XtPointer client;
XtPointer call;
{
	/*SUPPRESS 594*/XmAnyCallbackStruct *acs=(XmAnyCallbackStruct*)call;

}
/*--------------------------------------------------------------------*/
/* ARGSUSED */
void
do_toggle_pick(w, client, call)
Widget w;
XtPointer client;
XtPointer call;
{
	/*SUPPRESS 594*/XmAnyCallbackStruct *acs=(XmAnyCallbackStruct*)call;

	mode_pick = PICK_MODE_PICK;
	do_unset_interval();
	mbnavedit_pickcursor();
}
/*--------------------------------------------------------------------*/
/* ARGSUSED */
void
do_toggle_select(w, client, call)
Widget w;
XtPointer client;
XtPointer call;
{
	/*SUPPRESS 594*/XmAnyCallbackStruct *acs=(XmAnyCallbackStruct*)call;

	mode_pick = PICK_MODE_SELECT;
	do_unset_interval();
	mbnavedit_selectcursor();
}
/*--------------------------------------------------------------------*/
/* ARGSUSED */
void
do_toggle_deselect(w, client, call)
Widget w;
XtPointer client;
XtPointer call;
{
	/*SUPPRESS 594*/XmAnyCallbackStruct *acs=(XmAnyCallbackStruct*)call;

	mode_pick = PICK_MODE_DESELECT;
	do_unset_interval();
	mbnavedit_deselectcursor();
}
/*--------------------------------------------------------------------*/
/* ARGSUSED */
void
do_toggle_selectall(w, client, call)
Widget w;
XtPointer client;
XtPointer call;
{
	/*SUPPRESS 594*/XmAnyCallbackStruct *acs=(XmAnyCallbackStruct*)call;

	mode_pick = PICK_MODE_SELECTALL;
	do_unset_interval();
	mbnavedit_selectallcursor();
}
/*--------------------------------------------------------------------*/
/* ARGSUSED */
void
do_toggle_deselectall(w, client, call)
Widget w;
XtPointer client;
XtPointer call;
{
	/*SUPPRESS 594*/XmAnyCallbackStruct *acs=(XmAnyCallbackStruct*)call;

	mode_pick = PICK_MODE_DESELECTALL;
	do_unset_interval();
	mbnavedit_deselectallcursor();
}
/*--------------------------------------------------------------------*/

/* ARGSUSED */
void
do_quit(w, client, call)
Widget w;
XtPointer client;
XtPointer call;
{
	/*SUPPRESS 594*/XmAnyCallbackStruct *acs=(XmAnyCallbackStruct*)call;

	(void) BxExitCB(w, client, call);
}
/*--------------------------------------------------------------------*/
/* ARGSUSED */
void
do_interpolation(w, client, call)
Widget w;
XtPointer client;
XtPointer call;
{
	/*SUPPRESS 594*/XmAnyCallbackStruct *acs=(XmAnyCallbackStruct*)call;

	/* Interpolate any current selected data */
	mbnavedit_action_interpolate();
	do_unset_interval();

	/* replot */
	mbnavedit_plot_all();
}
/*--------------------------------------------------------------------*/
/* ARGSUSED */
void
do_scroll(w, client, call)
Widget w;
XtPointer client;
XtPointer call;
{

	/*SUPPRESS 594*/XmAnyCallbackStruct *acs=(XmAnyCallbackStruct*)call;

	/* replot */
	mbnavedit_plot_all();
}

/*--------------------------------------------------------------------*/

/* ARGSUSED */
void
do_revert(w, client, call)
Widget w;
XtPointer client;
XtPointer call;
{
	/*SUPPRESS 594*/XmAnyCallbackStruct *acs=(XmAnyCallbackStruct*)call;

	/* Revert to original values for all selected data */
	mbnavedit_action_revert();
	do_unset_interval();

	/* replot */
	mbnavedit_plot_all();
}
/*--------------------------------------------------------------------*/
/* ARGSUSED */
void
do_showall(w, client, call)
Widget w;
XtPointer client;
XtPointer call;
{
	char	label[10];

	/*SUPPRESS 594*/XmAnyCallbackStruct *acs=(XmAnyCallbackStruct*)call;

	/* Show entire data buffer */
	mbnavedit_action_showall();
	do_unset_interval();

	/* set values of number of data shown slider */
	XtVaSetValues(scale_timespan, 
			XmNminimum, 1, 
			XmNmaximum, data_show_max, 
			XmNvalue, data_show_size, 
			NULL);
	sprintf(label, "%d", data_show_max);
	XtVaSetValues(label_timespan_2, 
			XtVaTypedArg, XmNlabelString, 
			    XmRString, label, (strlen(label) + 1), 
			NULL);
}
/*--------------------------------------------------------------------*/
/* ARGSUSED */
void
do_set_interval(w, client, call)
Widget w;
XtPointer client;
XtPointer call;
{
	/*SUPPRESS 594*/XmAnyCallbackStruct *acs=(XmAnyCallbackStruct*)call;

	/* turn on set interval mode */
	mode_set_interval = MB_YES;
	mbnavedit_setintervalcursor();
}
/*--------------------------------------------------------------------*/
int
do_unset_interval()
{

	/* turn off set interval mode */
	mbnavedit_action_set_interval(0, 0, 3);
	if (mode_set_interval == MB_YES)
		{
		mode_set_interval = MB_NO;
		if (mode_pick == PICK_MODE_PICK)
			mbnavedit_pickcursor();
		else if (mode_pick == PICK_MODE_SELECT)
			mbnavedit_selectcursor();
		else if (mode_pick == PICK_MODE_DESELECT)
			mbnavedit_deselectcursor();
		else if (mode_pick == PICK_MODE_SELECTALL)
			mbnavedit_selectallcursor();
		else if (mode_pick == PICK_MODE_DESELECTALL)
			mbnavedit_deselectallcursor();
		}
}
/*--------------------------------------------------------------------*/

void
do_toggle_vru(w, client_data, call_data)
 Widget w;
 XtPointer client_data;
 XtPointer call_data;
{
    XmAnyCallbackStruct *acs=(XmAnyCallbackStruct*)call_data;
}
