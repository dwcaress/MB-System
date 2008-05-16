/*--------------------------------------------------------------------
 *    The MB-system:	mbnavedit_callbacks.c	6/24/95
 *    $Id: mbnavedit_callbacks.c,v 5.13 2008-05-16 23:05:05 caress Exp $
 *
 *    Copyright (c) 1995-2008 by
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
 * MBNAVEDIT is an interactive navigation editor for swath sonar data.
 * It can work with any data format supported by the MBIO library.
 * This file contains the callback functions from the MOTIF interface.
 *
 * Author:	D. W. Caress
 * Date:	June 24,  1995
 * Date:	August 28, 2000 (New version - no buffered i/o)
 *
 * $Log: not supported by cvs2svn $
 * Revision 5.12  2006/02/16 21:15:07  caress
 * Made smooth inversion weights work as small as 0.01 in the interface. Redimensioned some strings too.
 *
 * Revision 5.11  2006/01/24 19:19:24  caress
 * Version 5.0.8 beta.
 *
 * Revision 5.10  2005/06/04 04:45:50  caress
 * Added feature to apply longitude and latitude offsets to the navigation.
 *
 * Revision 5.9  2005/03/25 04:35:56  caress
 * Added capability to interpolate over repeated values.
 *
 * Revision 5.8  2004/05/21 23:33:03  caress
 * Moved to new version of BX GUI builder
 *
 * Revision 5.7  2003/04/17 21:09:06  caress
 * Release 5.0.beta30
 *
 * Revision 5.6  2001/07/20 00:34:00  caress
 * Release 5.0.beta03
 *
 * Revision 5.5  2001/06/30  17:41:04  caress
 * Release 5.0.beta01
 *
 * Revision 5.4  2001/04/06  22:16:01  caress
 * Fixed unflagging.
 *
 * Revision 5.3  2001/03/22  21:10:37  caress
 * Trying to make release 5.0.beta0.
 *
 * Revision 5.2  2001/01/22  07:47:40  caress
 * Version 5.0.beta01
 *
 * Revision 5.1  2000/12/10  20:30:08  caress
 * Version 5.0.alpha02
 *
 * Revision 5.0  2000/12/01  22:56:08  caress
 * First cut at Version 5.0.
 *
 * Revision 4.14  2000/10/11  01:05:17  caress
 * Convert to ANSI C
 *
 * Revision 4.13  2000/09/30  07:03:14  caress
 * Snapshot for Dale.
 *
 * Revision 4.12  2000/09/30  07:02:34  caress
 * Snapshot for Dale.
 *
 * Revision 4.11  2000/08/28  22:45:11  caress
 * About to kick off new version.
 *
 * Revision 4.10  1999/09/15 21:01:04  caress
 * Version label now set from mb_format.h
 *
 * Revision 4.9  1999/04/14  04:33:10  caress
 * Final (?) version 4.6 release
 *
 * Revision 4.8  1999/04/09  22:34:08  caress
 * Added time interval plot.
 *
 * Revision 4.7  1998/10/05  19:17:39  caress
 * MB-System version 4.6beta
 *
 * Revision 4.6  1997/09/15  19:10:20  caress
 * Real Version 4.5
 *
 * Revision 4.5  1997/04/22  19:25:57  caress
 * Fixed startup mode.
 *
 * Revision 4.4  1997/04/21  17:07:38  caress
 * MB-System 4.5 Beta Release.
 *
 * Revision 4.4  1997/04/16  21:40:29  caress
 * Version for MB-System 4.5.
 *
 * Revision 4.3  1996/04/05  20:07:02  caress
 * Added GUI mode so done means quit for real. Also changed done and
 * quit handling in browse mode so that the program doesn't read the
 * entire data file before closing it.
 *
 * Revision 4.2  1995/09/28  18:01:01  caress
 * Improved handling of .mbxxx file suffix convention.
 *
 * Revision 4.1  1995/08/17  14:58:12  caress
 * Revision for release 4.3.
 *
 * Revision 4.0  1995/08/07  18:33:22  caress
 * First cut.
 *
 *
 */

/*--------------------------------------------------------------------*/

/* include files */
#include <stdio.h>
#include <stdlib.h>
#include <Xm/Xm.h>
#include <X11/cursorfont.h>
#include <sys/types.h>
#include <sys/stat.h>

#define MBNAVEDIT_DECLARE_GLOBALS
#include "mb_status.h"
#include "mb_define.h"
#include "mb_io.h"
#include "mbnavedit_extrawidgets.h"
#include "mbnavedit.h"

#include "mbnavedit_creation.h"

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
#define xgfont "-misc-fixed-bold-r-normal-*-13-*-75-75-c-70-iso8859-1"
#define EV_MASK (ButtonPressMask | KeyPressMask | KeyReleaseMask | ExposureMask)

/* XG variable declarations */
XtAppContext	app_context;
Display		*display;
Window		can_xid, root_return, child_return;
Colormap	colormap;
GC		gc;
XGCValues	xgcv;
XFontStruct	*fontStruct;

/* file opening parameters */
int	expose_plot_ok = True;

static char	*input_file;
static char	output_file[128];
int selected = 0; /* indicates an input file is selected */

int	can_xgid;		/* XG graphics id */
Cursor myCursor;
XColor closest[2];
XColor exact[2];

/* Set the colors used for this program here. */
#define NCOLORS 9
XColor colors[NCOLORS];
unsigned int mpixel_values[NCOLORS];
XColor db_color;

/* Set these to the dimensions of your canvas drawing */
/* area, minus 1, located in mbedit.uil.              */
static int mb_borders[4] =
	{ 0, 1016, 0, 552 };

int	status;
char	string[128];

void	do_fileselection_list();
void	set_label_string(Widget, String);
void	set_label_multiline_string(Widget, String);
void	get_text_string(Widget, String);
void	do_set_controls();
void	do_scroll();
void	mbnavedit_bell();
void	mbnavedit_get_position();
void	mbnavedit_pickcursor();
void	mbnavedit_selectcursor();
void	mbnavedit_deselectcursor();
void	mbnavedit_selectallcursor();
void	mbnavedit_deselectallcursor();
void	mbnavedit_setintervalcursor();
void	do_filebutton_on();
void	do_filebutton_off();
void	do_open_file(int);
void	do_checkuseprevious();
XtEventHandler do_resize( Widget w, XtPointer client_data, XEvent *event, Boolean *unused);


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

void
do_mbnavedit_init(int argc, char **argv)
{
    int	    i;
    
    /* get additional widgets */
    fileSelectionBox_list = (Widget) 
	XmFileSelectionBoxGetChild(fileSelectionBox, 
				    XmDIALOG_LIST);
    fileSelectionBox_text = (Widget) 
	XmFileSelectionBoxGetChild(fileSelectionBox, 
				    XmDIALOG_TEXT);
    XtAddCallback(fileSelectionBox_list, 
	    XmNbrowseSelectionCallback, 
	    do_fileselection_list, NULL);
	    
    XtUnmanageChild(
	    (Widget) XmFileSelectionBoxGetChild(
				    fileSelectionBox, 
				    XmDIALOG_HELP_BUTTON));

    XtVaGetValues(scrolledWindow, 
			XmNhorizontalScrollBar, 
			&scrolledWindow_hscrollbar, 
			NULL);
    XtVaGetValues(scrolledWindow, 
			XmNverticalScrollBar, 
			&scrolledWindow_vscrollbar, 
			NULL);
    XtVaSetValues(scrolledWindow_hscrollbar, 
		XmCIncrement, 5, NULL);
    XtVaSetValues(scrolledWindow_vscrollbar, 
		XmCIncrement, 5, NULL);
    XtAddCallback(scrolledWindow_hscrollbar, 
		XmNvalueChangedCallback, 
		do_scroll, NULL);
    XtAddCallback(scrolledWindow_vscrollbar, 
		XmNvalueChangedCallback, 
		do_scroll, NULL);
    XtAddCallback(scrolledWindow_hscrollbar, 
		XmNdragCallback, 
		do_scroll, NULL);
    XtAddCallback(scrolledWindow_vscrollbar, 
		XmNdragCallback, 
		do_scroll, NULL);

    /* Setup the entire screen. */
    display = XtDisplay(drawingArea);
    colormap = DefaultColormap(display,XDefaultScreen(display));
    
    /* Setup just the "canvas" part of the screen. */
    can_xid = XtWindow(drawingArea);
    
    /* Setup the "graphics Context" for just the "canvas" */
    xgcv.background = WhitePixelOfScreen(DefaultScreenOfDisplay(display));
    xgcv.foreground = BlackPixelOfScreen(DefaultScreenOfDisplay(display));
    xgcv.line_width = 2;
    gc = XCreateGC(display,can_xid,GCBackground | GCForeground 
	     | GCLineWidth, &xgcv);
    
    /* Setup the font for just the "canvas" screen. */
    fontStruct = XLoadQueryFont(display, xgfont);
    XSetFont(display,gc,fontStruct->fid);
    
    XSelectInput(display, can_xid, EV_MASK );

    /* get resize events - add an event handler */
    XtAddEventHandler(XtParent(bulletinBoard), 
			    StructureNotifyMask, 
			    False, 
			    (XtEventHandler) do_resize, 
			    (XtPointer)NULL);

    /* Load the colors that will be used in this program. */
    status = XLookupColor(display,colormap, "white",&db_color,&colors[0]);
    if ((status = XAllocColor(display,colormap,&colors[0])) == 0)
	    fprintf(stderr,"Failure to allocate color: white\n");
    status = XLookupColor(display,colormap, "black",&db_color,&colors[1]);
    if ((status = XAllocColor(display,colormap,&colors[1])) == 0)
	    fprintf(stderr,"Failure to allocate color: black\n");
    status = XLookupColor(display,colormap, "red",&db_color,&colors[2]);
    if ((status = XAllocColor(display,colormap,&colors[2])) == 0)
	    fprintf(stderr,"Failure to allocate color: red\n");
    status = XLookupColor(display,colormap, "green",&db_color,&colors[3]);
    if ((status = XAllocColor(display,colormap,&colors[3])) == 0)
	    fprintf(stderr,"Failure to allocate color: green\n");
    status = XLookupColor(display,colormap, "blue",&db_color,&colors[4]);
    if ((status = XAllocColor(display,colormap,&colors[4])) == 0)
	    fprintf(stderr,"Failure to allocate color: blue\n");
    status = XLookupColor(display,colormap, "orange",&db_color,&colors[5]);
    if ((status = XAllocColor(display,colormap,&colors[5])) == 0)
	    fprintf(stderr,"Failure to allocate color: orange\n");
    status = XLookupColor(display,colormap, "purple",&db_color,&colors[6]);
    if ((status = XAllocColor(display,colormap,&colors[6])) == 0)
	    fprintf(stderr,"Failure to allocate color: purple\n");
    status = XLookupColor(display,colormap, "coral",&db_color,&colors[7]);
    if ((status = XAllocColor(display,colormap,&colors[7])) == 0)
	    fprintf(stderr,"Failure to allocate color: coral\n");
    status = XLookupColor(display,colormap, "lightgrey",&db_color,&colors[8]);
    if ((status = XAllocColor(display,colormap,&colors[8])) == 0)
	    fprintf(stderr,"Failure to allocate color: lightgrey\n");
    for (i=0;i<NCOLORS;i++)
	    {
	    mpixel_values[i] = colors[i].pixel;
	    }
    
    /* Setup initial cursor. This will be changed when changing "MODE". */
    myCursor = XCreateFontCursor(display, XC_target);
    XAllocNamedColor(display,colormap,"red",&closest[0],&exact[0]);
    XAllocNamedColor(display,colormap,"coral",&closest[1],&exact[1]);
    XRecolorCursor(display,myCursor,&closest[0],&closest[1]);
    XDefineCursor(display,can_xid,myCursor);
    
    /* initialize graphics */
    status = mbnavedit_init_globals();
    mb_borders[0] = 0;
    mb_borders[1] = plot_width;
    mb_borders[2] = 0;
    mb_borders[3] = number_plots*plot_height;
    can_xgid = xg_init(display, can_xid, mb_borders, xgfont);
    status = mbnavedit_set_graphics(can_xgid, 
		    NCOLORS, mpixel_values);

    /* initialize mbnavedit proper */
    status = mbnavedit_init(argc,argv);
    
    do_set_controls();
}

/*--------------------------------------------------------------------*/

void do_set_controls()
{
	char	value_text[128];
				
	/* set about version label */
	sprintf(value_text, ":::t\"MB-System Release %s\":t\"%s\"", 
		MB_VERSION, MB_BUILD_DATE);
	set_label_multiline_string(label_about_version, value_text);

	/* set value of format text item */
	sprintf(string,"%2.2d",format);
	XmTextFieldSetString(textField_format, string);

	/* set the output mode */
	if (output_mode == OUTPUT_MODE_OUTPUT)
	    {
	    XmToggleButtonSetState(toggleButton_output_on, 
			TRUE, TRUE);
	    XtManageChild(label_filename);
	    XtManageChild(textField_output_file);
	    }
	else
	    {
	    XmToggleButtonSetState(toggleButton_output_off,
			TRUE, TRUE);
	    XtUnmanageChild(label_filename);
	    XtUnmanageChild(textField_output_file);
	    }

	/* set values of number of data shown slider */
	XtVaSetValues(scale_timespan, 
			XmNminimum, 1, 
			XmNmaximum, data_show_max, 
			XmNvalue, MAX(data_show_size, 1), 
			NULL);
	sprintf(string, "%d", data_show_max);
	XtVaSetValues(label_timespan_2, 
			XtVaTypedArg, XmNlabelString, 
			    XmRString, string, (strlen(string) + 1), 
			NULL);	

	/* set values of number of data to step slider */
	XtVaSetValues(scale_timestep, 
			XmNminimum, 1, 
			XmNmaximum, data_step_max, 
			XmNvalue, data_step_size, 
			NULL);
	sprintf(string, "%d", data_step_max);
	XtVaSetValues(label_timestep_2, 
			XtVaTypedArg, XmNlabelString, 
			    XmRString, string, (strlen(string) + 1), 
			NULL);	

	/* set the pick mode */
	if (mode_pick == PICK_MODE_PICK)
	    XmToggleButtonSetState(toggleButton_pick, 
			TRUE, TRUE);
	else if (mode_pick == PICK_MODE_SELECT)
	    XmToggleButtonSetState(toggleButton_select, 
			TRUE, TRUE);
	else if (mode_pick == PICK_MODE_DESELECT)
	    XmToggleButtonSetState(toggleButton_deselect, 
			TRUE, TRUE);
	else if (mode_pick == PICK_MODE_SELECTALL)
	    XmToggleButtonSetState(toggleButton_selectall, 
			TRUE, TRUE);
	else if (mode_pick == PICK_MODE_DESELECTALL)
	    XmToggleButtonSetState(toggleButton_deselectall, 
			TRUE, TRUE);
			
	/* set the lon, lat, speed and heading plot toggles */
	XmToggleButtonSetState(toggleButton_time, 
			plot_tint, TRUE);
	XmToggleButtonSetState(toggleButton_org_time, 
			plot_tint_org, TRUE);
	XmToggleButtonSetState(toggleButton_lon, 
			plot_lon, TRUE);
	XmToggleButtonSetState(toggleButton_org_lon, 
			plot_lon_org, TRUE);
	XmToggleButtonSetState(toggleButton_dr_lon, 
			plot_lon_dr, TRUE);
	XmToggleButtonSetState(toggleButton_lat, 
			plot_lat, TRUE);
	XmToggleButtonSetState(toggleButton_org_lat, 
			plot_lat_org, TRUE);
	XmToggleButtonSetState(toggleButton_dr_lat, 
			plot_lat_dr, TRUE);
	XmToggleButtonSetState(toggleButton_speed, 
			plot_speed, TRUE);
	XmToggleButtonSetState(toggleButton_org_speed, 
			plot_speed_org, TRUE);
	XmToggleButtonSetState(toggleButton_show_smg, 
			plot_smg, TRUE);
	XmToggleButtonSetState(toggleButton_heading, 
			plot_heading, TRUE);
	XmToggleButtonSetState(toggleButton_org_heading, 
			plot_heading_org, TRUE);
	XmToggleButtonSetState(toggleButton_show_cmg, 
			plot_cmg, TRUE);
	XmToggleButtonSetState(toggleButton_sonardepth, 
			plot_draft, TRUE);
	XmToggleButtonSetState(toggleButton_org_sonardepth, 
			plot_draft_org, TRUE);

	/* hide or display items according to toggle states */
	if (plot_tint == MB_YES)
		XtManageChild(toggleButton_org_time);
	else
		XtUnmanageChild(toggleButton_org_time);
	if (plot_lon == MB_YES)
		{
		XtManageChild(toggleButton_org_lon);
		if (model_mode == MODEL_MODE_OFF)
		    {
		    XtUnmanageChild(toggleButton_dr_lon);
		    }
		else if (model_mode == MODEL_MODE_DR)
		    {
		    set_label_string(toggleButton_dr_lon, "Show Dead Reckoning");
		    XtManageChild(toggleButton_dr_lon);		    
		    }
		else if (model_mode == MODEL_MODE_INVERT)
		    {
		    set_label_string(toggleButton_dr_lon, "Show Smooth Inversion");
		    XtManageChild(toggleButton_dr_lon);		    
		    }
		}
	else
		{
		XtUnmanageChild(toggleButton_org_lon);
		XtUnmanageChild(toggleButton_dr_lon);
		}
	if (plot_lat == MB_YES)
		{
		XtManageChild(toggleButton_org_lat);
		if (model_mode == MODEL_MODE_OFF)
		    {
		    XtUnmanageChild(toggleButton_dr_lat);
		    }
		else if (model_mode == MODEL_MODE_DR)
		    {
		    set_label_string(toggleButton_dr_lat, "Show Dead Reckoning");
		    XtManageChild(toggleButton_dr_lat);		    
		    }
		else if (model_mode == MODEL_MODE_INVERT)
		    {
		    set_label_string(toggleButton_dr_lat, "Show Smooth Inversion");
		    XtManageChild(toggleButton_dr_lat);		    
		    }
		}
	else
		{
		XtUnmanageChild(toggleButton_org_lat);
		XtUnmanageChild(toggleButton_dr_lat);
		}
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
		}
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
		}
	if (plot_draft == MB_YES)
		{
		XtManageChild(toggleButton_org_sonardepth);
		}
	else
		{
		XtUnmanageChild(toggleButton_org_sonardepth);
		}

	/* get and set size of canvas */
	number_plots = 0;
	if (plot_tint == MB_YES)
		number_plots++;
	if (plot_lon == MB_YES)
		number_plots++;
	if (plot_lat == MB_YES)
		number_plots++;
	if (plot_speed == MB_YES)
		number_plots++;
	if (plot_heading == MB_YES)
		number_plots++;
	if (plot_draft == MB_YES)
		number_plots++;
	if (plot_roll == MB_YES)
		number_plots++;
	if (plot_pitch == MB_YES)
		number_plots++;
	if (plot_heave == MB_YES)
		number_plots++;
	XtVaSetValues(drawingArea, 
			XmNwidth, plot_width, 
			XmNheight, number_plots*plot_height, 
			NULL);	
			
	/* set modeling controls and hide or display buttons */
	if (model_mode == MODEL_MODE_OFF)
		{
		XmToggleButtonSetState(toggleButton_modeling_off, 
			TRUE, FALSE);
		XtUnmanageChild(pushButton_solution);
		XtUnmanageChild(pushButton_flag);
		XtUnmanageChild(pushButton_unflag);

		}
	else if (model_mode == MODEL_MODE_DR)
		{
		XmToggleButtonSetState(toggleButton_modeling_dr, 
			TRUE, FALSE);
		XtManageChild(pushButton_solution);
		XtUnmanageChild(pushButton_flag);
		XtUnmanageChild(pushButton_unflag);
		}
	if (model_mode == MODEL_MODE_INVERT)
		{
		XmToggleButtonSetState(toggleButton_modeling_inversion, 
			TRUE, FALSE);
		XtManageChild(pushButton_solution);
		XtManageChild(pushButton_flag);
		XtManageChild(pushButton_unflag);
		}
	XtVaSetValues(scale_driftlon, 
			XmNvalue, drift_lon, 
			NULL);
	XtVaSetValues(scale_driftlat, 
			XmNvalue, drift_lat, 
			NULL);
	sprintf(value_text,"%.2f",weight_speed);
	XmTextFieldSetString(
	    textField_modeling_speed, 
	    value_text);
	sprintf(value_text,"%.2f",weight_acceleration);
	XmTextFieldSetString(
	    textField_modeling_acceleration, 
	    value_text);
	    
	/* enable or disable time interpolation */
	if (timestamp_problem == MB_YES)
		{
		XtVaSetValues(pushButton_controls_timeinterpolation, 
			XmNsensitive, True, 
			NULL);
		XtVaSetValues(pushButton_controls_deletebadtimetag, 
			XmNsensitive, True, 
			NULL);
		}
	else
		{
		XtVaSetValues(pushButton_controls_timeinterpolation, 
			XmNsensitive, False, 
			NULL);
		XtVaSetValues(pushButton_controls_deletebadtimetag, 
			XmNsensitive, False, 
			NULL);
		XtUnmanageChild(bulletinBoard_deletebadtimetag);
		XtUnmanageChild(bulletinBoard_timeinterpolation);
		}
		
	/* set offset values */
	sprintf(value_text,"%.5f",offset_lon);
	XmTextFieldSetString(
	    textField_lon_offset, 
	    value_text);
	sprintf(value_text,"%.5f",offset_lat);
	XmTextFieldSetString(
	    textField_lat_offset, 
	    value_text);
	    
}

/*--------------------------------------------------------------------*/

void
do_nextbuffer( Widget w, XtPointer client_data, XtPointer call_data)
{
	int	status;
	int	quit;
	
	XmAnyCallbackStruct *acs = (XmAnyCallbackStruct*)call_data;
	    
	/* turn off expose plots */
	expose_plot_ok = False;

	/* get next buffer */
	status = mbnavedit_action_next_buffer(&quit);
	if (status == 0) mbnavedit_bell(100);
	do_unset_interval();
	    
	/* turn on expose plots */
	expose_plot_ok = True;
    
	/* quit if in GUI mode */
	if (quit)
		exit(0);
}

/*--------------------------------------------------------------------*/

void
do_done( Widget w, XtPointer client_data, XtPointer call_data)
{
	int	quit;
	int	status;
	
	XmAnyCallbackStruct *acs = (XmAnyCallbackStruct*)call_data;
	    
	/* turn off expose plots */
	expose_plot_ok = False;

	/* finish with the current file */
	status = mbnavedit_action_done(&quit);
	if (status == 0) mbnavedit_bell(100);
	do_unset_interval();
	    
	/* turn on expose plots */
	expose_plot_ok = True;
	
	/* quit if required */
	if (quit)
		(void) BxExitCB(w, client_data, call_data);
}

/*--------------------------------------------------------------------*/

void
do_forward( Widget w, XtPointer client_data, XtPointer call_data)
{
	int	status;
	
	XmAnyCallbackStruct *acs = (XmAnyCallbackStruct*)call_data;

	/* step forward */
	status = mbnavedit_action_step(data_step_size);
	if (status == 0) mbnavedit_bell(100);
	do_unset_interval();
}

/*--------------------------------------------------------------------*/

void
do_reverse( Widget w, XtPointer client_data, XtPointer call_data)
{
	int	status;
	
	XmAnyCallbackStruct *acs = (XmAnyCallbackStruct*)call_data;

	/* step back */
	status = mbnavedit_action_step(-data_step_size);
	if (status == 0) mbnavedit_bell(100);
	do_unset_interval();
}

/*--------------------------------------------------------------------*/

void
do_timespan( Widget w, XtPointer client_data, XtPointer call_data)
{
	XmAnyCallbackStruct *acs = (XmAnyCallbackStruct*)call_data;

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
		sprintf(string, "%d", data_show_max);
		set_label_string(label_timespan_2, string);
		}

	/* replot */
	mbnavedit_plot_all();

}

/*--------------------------------------------------------------------*/

void
do_timestep( Widget w, XtPointer client_data, XtPointer call_data)
{
	XmAnyCallbackStruct *acs = (XmAnyCallbackStruct*)call_data;

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
		sprintf(string, "%d", data_step_max);
		set_label_string(label_timestep_2, string);
		}

}

/*--------------------------------------------------------------------*/

void
do_expose( Widget w, XtPointer client_data, XtPointer call_data)
{
	XmAnyCallbackStruct *acs = (XmAnyCallbackStruct*)call_data;

	/* replot */
	if (expose_plot_ok == True)
	    mbnavedit_plot_all();
}

/*--------------------------------------------------------------------*/

void
do_event( Widget w, XtPointer client_data, XtPointer call_data)
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
	int	status;

	XmAnyCallbackStruct *acs = (XmAnyCallbackStruct*)call_data;

	/* get event */
	cbs = (XmDrawingAreaCallbackStruct *) call_data;
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
				XmNvalue, MAX(data_show_size, 1), 
				NULL);
			sprintf(string, "%d", data_show_max);
			set_label_string(label_timespan_2, string);

			/* set values of number of data to step slider */
			XtVaSetValues(scale_timestep, 
					XmNminimum, 1, 
					XmNmaximum, data_step_max, 
					XmNvalue, data_step_size, 
					NULL);
			sprintf(string, "%d", data_step_max);
			XtVaSetValues(label_timestep_2, 
					XtVaTypedArg, XmNlabelString, 
					    XmRString, string, (strlen(string) + 1), 
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

XtEventHandler
do_resize( Widget w, XtPointer client_data, XEvent *event, Boolean *unused)
{
	XConfigureEvent *cevent = (XConfigureEvent *) event;
	Dimension   width, height;
	
	/* do this only if a resize event happens */
	if (cevent->type == ConfigureNotify)
	    {
	XtVaGetValues(bulletinBoard, 
			XmNwidth, &width, 
			XmNheight, &height, 
			NULL);
	window_width = (int)width - 220;	
	window_height = (int)height - 90;	
	XtVaSetValues(scrolledWindow, 
			XmNwidth, (Dimension)window_width, 
			XmNheight, (Dimension)window_height, 
			NULL);
	    }


}

/*--------------------------------------------------------------------*/

void
do_toggle_time( Widget w, XtPointer client_data, XtPointer call_data)
{
	int	screen_height;

	XmAnyCallbackStruct *acs = (XmAnyCallbackStruct*)call_data;

	plot_tint = XmToggleButtonGetState(toggleButton_time);
	if (plot_tint == MB_YES)
		XtManageChild(toggleButton_org_time);
	else
		{
		XtUnmanageChild(toggleButton_org_time);
		mbnavedit_action_deselect_all(PLOT_TINT);
		}

	/* get and set size of canvas */
	number_plots = 0;
	if (plot_tint == MB_YES)
		number_plots++;
	if (plot_lon == MB_YES)
		number_plots++;
	if (plot_lat == MB_YES)
		number_plots++;
	if (plot_speed == MB_YES)
		number_plots++;
	if (plot_heading == MB_YES)
		number_plots++;
	if (plot_draft == MB_YES)
		number_plots++;
	if (plot_roll == MB_YES)
		number_plots++;
	if (plot_pitch == MB_YES)
		number_plots++;
	if (plot_heave == MB_YES)
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

void
do_toggle_lon( Widget w, XtPointer client_data, XtPointer call_data)
{
	int	screen_height;

	XmAnyCallbackStruct *acs = (XmAnyCallbackStruct*)call_data;

	plot_lon = XmToggleButtonGetState(toggleButton_lon);
	if (plot_lon == MB_YES)
		{
		XtManageChild(toggleButton_org_lon);
		XtManageChild(toggleButton_dr_lon);
		}
	else
		{
		XtUnmanageChild(toggleButton_org_lon);
		XtUnmanageChild(toggleButton_dr_lon);
		mbnavedit_action_deselect_all(PLOT_LONGITUDE);
		}

	/* get and set size of canvas */
	number_plots = 0;
	if (plot_tint == MB_YES)
		number_plots++;
	if (plot_lon == MB_YES)
		number_plots++;
	if (plot_lat == MB_YES)
		number_plots++;
	if (plot_speed == MB_YES)
		number_plots++;
	if (plot_heading == MB_YES)
		number_plots++;
	if (plot_draft == MB_YES)
		number_plots++;
	if (plot_roll == MB_YES)
		number_plots++;
	if (plot_pitch == MB_YES)
		number_plots++;
	if (plot_heave == MB_YES)
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

void
do_toggle_lat( Widget w, XtPointer client_data, XtPointer call_data)
{
	int	screen_height;

	XmAnyCallbackStruct *acs = (XmAnyCallbackStruct*)call_data;

	plot_lat = XmToggleButtonGetState(toggleButton_lat);
	if (plot_lat == MB_YES)
		{
		XtManageChild(toggleButton_org_lat);
		XtManageChild(toggleButton_dr_lat);
		}
	else
		{
		XtUnmanageChild(toggleButton_org_lat);
		XtUnmanageChild(toggleButton_dr_lat);
		mbnavedit_action_deselect_all(PLOT_LATITUDE);
		}

	/* get and set size of canvas */
	number_plots = 0;
	if (plot_tint == MB_YES)
		number_plots++;
	if (plot_lon == MB_YES)
		number_plots++;
	if (plot_lat == MB_YES)
		number_plots++;
	if (plot_speed == MB_YES)
		number_plots++;
	if (plot_heading == MB_YES)
		number_plots++;
	if (plot_draft == MB_YES)
		number_plots++;
	if (plot_roll == MB_YES)
		number_plots++;
	if (plot_pitch == MB_YES)
		number_plots++;
	if (plot_heave == MB_YES)
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

void
do_toggle_heading( Widget w, XtPointer client_data, XtPointer call_data)
{
	int	screen_height;

	XmAnyCallbackStruct *acs = (XmAnyCallbackStruct*)call_data;

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
	if (plot_tint == MB_YES)
		number_plots++;
	if (plot_lon == MB_YES)
		number_plots++;
	if (plot_lat == MB_YES)
		number_plots++;
	if (plot_speed == MB_YES)
		number_plots++;
	if (plot_heading == MB_YES)
		number_plots++;
	if (plot_draft == MB_YES)
		number_plots++;
	if (plot_roll == MB_YES)
		number_plots++;
	if (plot_pitch == MB_YES)
		number_plots++;
	if (plot_heave == MB_YES)
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

void
do_toggle_speed( Widget w, XtPointer client_data, XtPointer call_data)
{
	int	screen_height;

	XmAnyCallbackStruct *acs = (XmAnyCallbackStruct*)call_data;

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
	number_plots = 0;
	if (plot_tint == MB_YES)
		number_plots++;
	if (plot_lon == MB_YES)
		number_plots++;
	if (plot_lat == MB_YES)
		number_plots++;
	if (plot_speed == MB_YES)
		number_plots++;
	if (plot_heading == MB_YES)
		number_plots++;
	if (plot_draft == MB_YES)
		number_plots++;
	if (plot_roll == MB_YES)
		number_plots++;
	if (plot_pitch == MB_YES)
		number_plots++;
	if (plot_heave == MB_YES)
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

void
do_toggle_sonardepth( Widget w, XtPointer client_data, XtPointer call_data)
{
	int	screen_height;

	XmAnyCallbackStruct *acs = (XmAnyCallbackStruct*)call_data;

	plot_draft = XmToggleButtonGetState(toggleButton_sonardepth);
	if (plot_draft == MB_YES)
		{
		XtManageChild(toggleButton_org_sonardepth);
		}
	else
		{
		XtUnmanageChild(toggleButton_org_sonardepth);
		mbnavedit_action_deselect_all(PLOT_DRAFT);
		}

	/* get and set size of canvas */
	number_plots = 0;
	number_plots = 0;
	if (plot_tint == MB_YES)
		number_plots++;
	if (plot_lon == MB_YES)
		number_plots++;
	if (plot_lat == MB_YES)
		number_plots++;
	if (plot_speed == MB_YES)
		number_plots++;
	if (plot_heading == MB_YES)
		number_plots++;
	if (plot_draft == MB_YES)
		number_plots++;
	if (plot_roll == MB_YES)
		number_plots++;
	if (plot_pitch == MB_YES)
		number_plots++;
	if (plot_heave == MB_YES)
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

void
do_toggle_org_time( Widget w, XtPointer client_data, XtPointer call_data)
{
	XmAnyCallbackStruct *acs = (XmAnyCallbackStruct*)call_data;

	plot_tint_org = XmToggleButtonGetState(toggleButton_org_time);

	/* replot */
	mbnavedit_plot_all();
}

/*--------------------------------------------------------------------*/

void
do_toggle_org_lon( Widget w, XtPointer client_data, XtPointer call_data)
{
	XmAnyCallbackStruct *acs = (XmAnyCallbackStruct*)call_data;

	plot_lon_org = XmToggleButtonGetState(toggleButton_org_lon);

	/* replot */
	mbnavedit_plot_all();
}

/*--------------------------------------------------------------------*/

void
do_toggle_org_lat( Widget w, XtPointer client_data, XtPointer call_data)
{
	XmAnyCallbackStruct *acs = (XmAnyCallbackStruct*)call_data;

	plot_lat_org = XmToggleButtonGetState(toggleButton_org_lat);

	/* replot */
	mbnavedit_plot_all();
}

/*--------------------------------------------------------------------*/

void
do_toggle_org_speed( Widget w, XtPointer client_data, XtPointer call_data)
{
	XmAnyCallbackStruct *acs = (XmAnyCallbackStruct*)call_data;

	plot_speed_org = XmToggleButtonGetState(toggleButton_org_speed);

	/* replot */
	mbnavedit_plot_all();
}
/*--------------------------------------------------------------------*/

void
do_toggle_dr_lat( Widget w, XtPointer client_data, XtPointer call_data)
{
	XmAnyCallbackStruct *acs = (XmAnyCallbackStruct*)call_data;

	plot_lat_dr = XmToggleButtonGetState(toggleButton_dr_lat);

	/* replot */
	mbnavedit_plot_all();
}
/*--------------------------------------------------------------------*/

void
do_toggle_dr_lon( Widget w, XtPointer client_data, XtPointer call_data)
{
	XmAnyCallbackStruct *acs = (XmAnyCallbackStruct*)call_data;

	plot_lon_dr = XmToggleButtonGetState(toggleButton_dr_lon);

	/* replot */
	mbnavedit_plot_all();
}

/*--------------------------------------------------------------------*/

void
do_flag( Widget w, XtPointer client_data, XtPointer call_data)
{
    XmAnyCallbackStruct *acs = (XmAnyCallbackStruct*)call_data;
	
	/* interpolate time stamps */
	mbnavedit_action_flag();
	
	/* replot */
	mbnavedit_plot_all();
		
	/* update controls */
	do_set_controls();
}

/*--------------------------------------------------------------------*/

void
do_unflag( Widget w, XtPointer client_data, XtPointer call_data)
{
    XmAnyCallbackStruct *acs = (XmAnyCallbackStruct*)call_data;
	
	/* interpolate time stamps */
	mbnavedit_action_unflag();
	
	/* replot */
	mbnavedit_plot_all();
		
	/* update controls */
	do_set_controls();
}

/*--------------------------------------------------------------------*/

void
do_modeling_apply( Widget w, XtPointer client_data, XtPointer call_data)
{
    double  dvalue;
	XmAnyCallbackStruct *acs = (XmAnyCallbackStruct*)call_data;

	get_text_string(textField_modeling_speed, string);
	if (sscanf(string, "%lf", &dvalue) == 1)
		weight_speed = dvalue;

	get_text_string(textField_modeling_acceleration, string);
	if (sscanf(string, "%lf", &dvalue) == 1)
		weight_acceleration = dvalue;
	
	do_set_controls();
	
	/* recalculate model */
	mbnavedit_get_model();

	/* replot */
	mbnavedit_plot_all();

}

/*--------------------------------------------------------------------*/

void
do_model_mode( Widget w, XtPointer client_data, XtPointer call_data)
{
	XmAnyCallbackStruct *acs = (XmAnyCallbackStruct*)call_data;

	if (XmToggleButtonGetState(toggleButton_modeling_off))
	    model_mode = MODEL_MODE_OFF;
	else if (XmToggleButtonGetState(toggleButton_modeling_dr))
	    model_mode = MODEL_MODE_DR;
	else
	    model_mode = MODEL_MODE_INVERT;
	if (model_mode != MODEL_MODE_OFF)
	    {
	    plot_lon_dr = TRUE;
	    plot_lat_dr = TRUE;
	    }
	
	do_set_controls();
	
	/* recalculate model */
	mbnavedit_get_model();

	/* replot */
	mbnavedit_plot_all();

}

/*--------------------------------------------------------------------*/

void
do_timeinterpolation_apply( Widget w, XtPointer client_data, XtPointer call_data)
{
    XmAnyCallbackStruct *acs = (XmAnyCallbackStruct*)call_data;
	
	/* interpolate time stamps */
	mbnavedit_action_fixtime();
	
	/* reset timestamp problem flag */
	timestamp_problem = MB_NO;
	
	/* replot */
	mbnavedit_plot_all();
		
	/* update controls */
	do_set_controls();

}

/*--------------------------------------------------------------------*/

void
do_deletebadtimetag_apply( Widget w, XtPointer client_data, XtPointer call_data)
{
    XmAnyCallbackStruct *acs = (XmAnyCallbackStruct*)call_data;
	
	/* interpolate time stamps */
	mbnavedit_action_deletebadtime();
	
	/* reset timestamp problem flag */
	timestamp_problem = MB_NO;
	
	/* replot */
	mbnavedit_plot_all();
		
	/* update controls */
	do_set_controls();
}

/*--------------------------------------------------------------------*/

void
do_driftlon( Widget w, XtPointer client_data, XtPointer call_data)
{
	XmAnyCallbackStruct *acs = (XmAnyCallbackStruct*)call_data;

	XtVaGetValues(scale_driftlon, 
			XmNvalue, &drift_lon, 
			NULL);
	
	/* recalculate model */
	mbnavedit_get_model();

	/* replot */
	mbnavedit_plot_all();
}
/*--------------------------------------------------------------------*/

void
do_driftlat( Widget w, XtPointer client_data, XtPointer call_data)
{
	XmAnyCallbackStruct *acs = (XmAnyCallbackStruct*)call_data;

	XtVaGetValues(scale_driftlat, 
			XmNvalue, &drift_lat, 
			NULL);
	
	/* recalculate model */
	mbnavedit_get_model();
	
	/* replot */
	mbnavedit_plot_all();
}

/*--------------------------------------------------------------------*/

void
do_offset_apply( Widget w, XtPointer client_data, XtPointer call_data)
{
	char	value_text[128];
    	double  dvalue;
	XmAnyCallbackStruct *acs = (XmAnyCallbackStruct*)call_data;

	/* get values from widgets */
	get_text_string(textField_lon_offset, string);
	if (sscanf(string, "%lf", &dvalue) == 1)
		offset_lon = dvalue;
	get_text_string(textField_lat_offset, string);
	if (sscanf(string, "%lf", &dvalue) == 1)
		offset_lat = dvalue;
		
	/* reset widgets so user sees what got applied */
	sprintf(value_text,"%.5f",offset_lon);
	XmTextFieldSetString(
	    textField_lon_offset, 
	    value_text);
	sprintf(value_text,"%.5f",offset_lat);
	XmTextFieldSetString(
	    textField_lat_offset, 
	    value_text);
	
	/* apply offsets */
	mbnavedit_action_offset();
	
	/* replot */
	mbnavedit_plot_all();
}

/*--------------------------------------------------------------------*/

void
do_toggle_show_smg( Widget w, XtPointer client_data, XtPointer call_data)
{
	XmAnyCallbackStruct *acs = (XmAnyCallbackStruct*)call_data;

	plot_smg = XmToggleButtonGetState(toggleButton_show_smg);

	/* replot */
	mbnavedit_plot_all();
}

/*--------------------------------------------------------------------*/

void
do_toggle_org_heading( Widget w, XtPointer client_data, XtPointer call_data)
{
	XmAnyCallbackStruct *acs = (XmAnyCallbackStruct*)call_data;

	plot_heading_org = XmToggleButtonGetState(toggleButton_org_heading);

	/* replot */
	mbnavedit_plot_all();
}
/*--------------------------------------------------------------------*/

void
do_toggle_org_sonardepth( Widget w, XtPointer client_data, XtPointer call_data)
{
	XmAnyCallbackStruct *acs = (XmAnyCallbackStruct*)call_data;

	plot_draft_org = XmToggleButtonGetState(toggleButton_org_sonardepth);

	/* replot */
	mbnavedit_plot_all();
}

/*--------------------------------------------------------------------*/

void
do_toggle_show_cmg( Widget w, XtPointer client_data, XtPointer call_data)
{
	XmAnyCallbackStruct *acs = (XmAnyCallbackStruct*)call_data;

	plot_cmg = XmToggleButtonGetState(toggleButton_show_cmg);

	/* replot */
	mbnavedit_plot_all();
}

/*--------------------------------------------------------------------*/

void
do_button_use_dr( Widget w, XtPointer client_data, XtPointer call_data)
{
	XmAnyCallbackStruct *acs = (XmAnyCallbackStruct*)call_data;

	/* Use dr for selected lonlat values */
	mbnavedit_action_use_dr();

	/* replot */
	mbnavedit_plot_all();
}

/*--------------------------------------------------------------------*/

void
do_button_use_smg( Widget w, XtPointer client_data, XtPointer call_data)
{
	XmAnyCallbackStruct *acs = (XmAnyCallbackStruct*)call_data;

	/* Use speed made good for selected speed values */
	mbnavedit_action_use_smg();

	/* replot */
	mbnavedit_plot_all();
}

/*--------------------------------------------------------------------*/

void
do_button_use_cmg( Widget w, XtPointer client_data, XtPointer call_data)
{
	XmAnyCallbackStruct *acs = (XmAnyCallbackStruct*)call_data;

	/* Use course made good for selected heading values */
	mbnavedit_action_use_cmg();

	/* replot */
	mbnavedit_plot_all();
}
/*--------------------------------------------------------------------*/

void
do_toggle_output_on( Widget w, XtPointer client_data, XtPointer call_data)
{
	XmAnyCallbackStruct *acs = (XmAnyCallbackStruct*)call_data;

	if (XmToggleButtonGetState(toggleButton_output_on))
		{
		output_mode = OUTPUT_MODE_OUTPUT;
		XtManageChild(textField_output_file);
		XtManageChild(label_filename);
		}
	else
		{
		output_mode = OUTPUT_MODE_BROWSE;
		XtUnmanageChild(textField_output_file);
		XtUnmanageChild(label_filename);
		}
}


/*--------------------------------------------------------------------*/

void
do_toggle_output_off( Widget w, XtPointer client_data, XtPointer call_data)
{
	XmAnyCallbackStruct *acs = (XmAnyCallbackStruct*)call_data;

	if (XmToggleButtonGetState(toggleButton_output_on))
		{
		output_mode = OUTPUT_MODE_OUTPUT;
		XtManageChild(textField_output_file);
		XtManageChild(label_filename);
		}
	else
		{
		output_mode = OUTPUT_MODE_BROWSE;
		XtUnmanageChild(textField_output_file);
		XtUnmanageChild(label_filename);
		}
}

/*--------------------------------------------------------------------*/

void
do_fileselection_cancel( Widget w, XtPointer client_data, XtPointer call_data)
{
	XmAnyCallbackStruct *acs = (XmAnyCallbackStruct*)call_data;
}

/*--------------------------------------------------------------------*/

void
do_filebutton_on()
{
		XtVaSetValues(pushButton_file, 
			XmNsensitive, True, 
			NULL);
}

/*--------------------------------------------------------------------*/

void
do_filebutton_off()
{
		XtVaSetValues(pushButton_file, 
			XmNsensitive, False, 
			NULL);
}

/*--------------------------------------------------------------------*/

void
do_fileselection_ok( Widget w, XtPointer client_data, XtPointer call_data)
{
	char	*suffix;
	int	len;
	int	form;
	struct stat file_status;
	int	fstat;
	int	status;

	XmAnyCallbackStruct *acs = (XmAnyCallbackStruct*)call_data;

	/* get input filename */
	get_text_string(fileSelectionBox_text, ifile);

	/* get output filename */
	get_text_string(textField_output_file, nfile);
	nfile_defined = MB_YES;

	/* get the file format */
	get_text_string(textField_format, string);
	if (sscanf(string, "%d", &form) == 1)
		format = form;
		
	/* check if output file exists */
	fstat = stat(nfile, &file_status);
	if (fstat != 0 
	    || (file_status.st_mode & S_IFMT) == S_IFDIR)
	    {
	    do_open_file(MB_NO);
	    }
	else
	    {
	    do_checkuseprevious();
	    }
}

/*--------------------------------------------------------------------*/
void
do_checkuseprevious()
{
	     XtManageChild(bulletinBoard_useprevious);
}

/*--------------------------------------------------------------------*/
void
do_useprevious_yes( Widget w, XtPointer client_data, XtPointer call_data)
{
	XmAnyCallbackStruct *acs = (XmAnyCallbackStruct*)call_data;

    do_open_file(MB_YES);
}

/*--------------------------------------------------------------------*/
void
do_useprevious_no( Widget w, XtPointer client_data, XtPointer call_data)
{
	XmAnyCallbackStruct *acs = (XmAnyCallbackStruct*)call_data;

    do_open_file(MB_NO);
}

/*--------------------------------------------------------------------*/

void
do_open_file(int useprevious)
{
	int	status;
	    
	/* turn off expose plots */
	expose_plot_ok = False;

	/* open the file */
	status = mbnavedit_action_open(useprevious);
	
	if (status == MB_FAILURE) 
		mbnavedit_bell(100);
	
	do_unset_interval();

	/* set values of number of data shown slider */
	XtVaSetValues(scale_timespan, 
			XmNminimum, 1, 
			XmNmaximum, data_show_max, 
			XmNvalue, MAX(data_show_size, 1), 
			NULL);
	sprintf(string, "%d", data_show_max);
	set_label_string(label_timespan_2, string);

	/* unmanage fileselection window so plotting will show */
	XtUnmanageChild(xmDialogShell_fileselection);

	/* replot */
	if (status == MB_SUCCESS)
		mbnavedit_plot_all();
	    
	/* turn on expose plots */
	expose_plot_ok = True;
	
	do_set_controls();
}


/*--------------------------------------------------------------------*/

void
do_fileselection_filter( Widget w, XtPointer client_data, XtPointer call_data)
{
	XmAnyCallbackStruct *acs = (XmAnyCallbackStruct*)call_data;

}

/*--------------------------------------------------------------------*/

void
do_fileselection_list( Widget w, XtPointer client_data, XtPointer call_data)
{
	char	fileroot[128];
	int	format_status, format_error;
	int	form;
	char	value_text[10];

	XmAnyCallbackStruct *acs = (XmAnyCallbackStruct*)call_data;

	/* get selected text */
	get_text_string(fileSelectionBox_text, string);

	/* get output file */
	if((int)strlen(string) > 0)
		{
		/* get the file format and set the widget */
		if (mb_get_format(0, string, fileroot, 
				    &form, &format_error) 
			== MB_SUCCESS)
		    {
		    format = form;
		    sprintf(value_text,"%d",format);
		    XmTextFieldSetString(
			textField_format, 
			value_text);
		    }
    
		/* get the output filename */
		strcpy(nfile,string);
		strcat(nfile,".nve");

		/* now set the output filename text widget */
		XmTextFieldSetString(textField_output_file, 
			nfile);
		XmTextFieldSetCursorPosition(textField_output_file, 
			strlen(nfile));
	}
}

/*--------------------------------------------------------------------*/

void
do_fileselection_nomatch( Widget w, XtPointer client_data, XtPointer call_data)
{
	XmAnyCallbackStruct *acs = (XmAnyCallbackStruct*)call_data;

}
/*--------------------------------------------------------------------*/

void
do_toggle_pick( Widget w, XtPointer client_data, XtPointer call_data)
{
	XmAnyCallbackStruct *acs = (XmAnyCallbackStruct*)call_data;

	mode_pick = PICK_MODE_PICK;
	do_unset_interval();
	mbnavedit_pickcursor();
}
/*--------------------------------------------------------------------*/

void
do_toggle_select( Widget w, XtPointer client_data, XtPointer call_data)
{
	XmAnyCallbackStruct *acs = (XmAnyCallbackStruct*)call_data;

	mode_pick = PICK_MODE_SELECT;
	do_unset_interval();
	mbnavedit_selectcursor();
}
/*--------------------------------------------------------------------*/

void
do_toggle_deselect( Widget w, XtPointer client_data, XtPointer call_data)
{
	XmAnyCallbackStruct *acs = (XmAnyCallbackStruct*)call_data;

	mode_pick = PICK_MODE_DESELECT;
	do_unset_interval();
	mbnavedit_deselectcursor();
}
/*--------------------------------------------------------------------*/

void
do_toggle_selectall( Widget w, XtPointer client_data, XtPointer call_data)
{
	XmAnyCallbackStruct *acs = (XmAnyCallbackStruct*)call_data;

	mode_pick = PICK_MODE_SELECTALL;
	do_unset_interval();
	mbnavedit_selectallcursor();
}
/*--------------------------------------------------------------------*/

void
do_toggle_deselectall( Widget w, XtPointer client_data, XtPointer call_data)
{
	XmAnyCallbackStruct *acs = (XmAnyCallbackStruct*)call_data;

	mode_pick = PICK_MODE_DESELECTALL;
	do_unset_interval();
	mbnavedit_deselectallcursor();
}
/*--------------------------------------------------------------------*/


void
do_quit( Widget w, XtPointer client_data, XtPointer call_data)
{
	XmAnyCallbackStruct *acs = (XmAnyCallbackStruct*)call_data;

	(void) BxExitCB(w, client_data, call_data);
}
/*--------------------------------------------------------------------*/

void
do_interpolation( Widget w, XtPointer client_data, XtPointer call_data)
{
	XmAnyCallbackStruct *acs = (XmAnyCallbackStruct*)call_data;

	/* Interpolate any current selected data */
	mbnavedit_action_interpolate();
	do_unset_interval();

	/* replot */
	mbnavedit_plot_all();
}

/*--------------------------------------------------------------------*/

void
do_interpolationrepeats( Widget w, XtPointer client_data, XtPointer call_data)
{
    XmAnyCallbackStruct *acs = (XmAnyCallbackStruct*)call_data;

	/* Interpolate any current selected data */
	mbnavedit_action_interpolaterepeats();
	do_unset_interval();

	/* replot */
	mbnavedit_plot_all();
}
/*--------------------------------------------------------------------*/

void
do_scroll( Widget w, XtPointer client_data, XtPointer call_data)
{

	XmAnyCallbackStruct *acs = (XmAnyCallbackStruct*)call_data;
}

/*--------------------------------------------------------------------*/


void
do_revert( Widget w, XtPointer client_data, XtPointer call_data)
{
	XmAnyCallbackStruct *acs = (XmAnyCallbackStruct*)call_data;

	/* Revert to original values for all selected data */
	mbnavedit_action_revert();
	do_unset_interval();

	/* replot */
	mbnavedit_plot_all();
}
/*--------------------------------------------------------------------*/

void
do_showall( Widget w, XtPointer client_data, XtPointer call_data)
{
	XmAnyCallbackStruct *acs = (XmAnyCallbackStruct*)call_data;

	/* Show entire data buffer */
	mbnavedit_action_showall();
	do_unset_interval();

	/* set values of number of data shown slider */
	XtVaSetValues(scale_timespan, 
			XmNminimum, 1, 
			XmNmaximum, data_show_max, 
			XmNvalue, MAX(data_show_size, 1), 
			NULL);
	sprintf(string, "%d", data_show_max);
	set_label_string(label_timespan_2, string);
}
/*--------------------------------------------------------------------*/

void
do_set_interval( Widget w, XtPointer client_data, XtPointer call_data)
{
	XmAnyCallbackStruct *acs = (XmAnyCallbackStruct*)call_data;

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
		
	return(MB_SUCCESS);
}
/*--------------------------------------------------------------------*/

void
do_toggle_vru( Widget w, XtPointer client_data, XtPointer call_data)
{
	int	screen_height;

        XmAnyCallbackStruct *acs=(XmAnyCallbackStruct*)call_data;

	plot_roll = XmToggleButtonGetState(toggleButton_vru);
	plot_pitch = XmToggleButtonGetState(toggleButton_vru);
	plot_heave = XmToggleButtonGetState(toggleButton_vru);

	/* get and set size of canvas */
	number_plots = 0;
	if (plot_tint == MB_YES)
		number_plots++;
	if (plot_lon == MB_YES)
		number_plots++;
	if (plot_lat == MB_YES)
		number_plots++;
	if (plot_speed == MB_YES)
		number_plots++;
	if (plot_heading == MB_YES)
		number_plots++;
	if (plot_draft == MB_YES)
		number_plots++;
	if (plot_roll == MB_YES)
		number_plots++;
	if (plot_pitch == MB_YES)
		number_plots++;
	if (plot_heave == MB_YES)
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
void
mbnavedit_bell(int length)
{
	XBell(display,length);
}
/*--------------------------------------------------------------------*/
void
mbnavedit_get_position(int *win_x, int *win_y, unsigned int *mask_return)
{
	Window	root_return, child_return;
	int	root_x_return, root_y_return;
	int	status;

	status = XQueryPointer(display, can_xid,
		&root_return, &child_return,
		&root_x_return, &root_y_return, 
		win_x, win_y, mask_return);
}
/*--------------------------------------------------------------------*/
void
mbnavedit_pickcursor()
{
	myCursor = XCreateFontCursor(display, XC_target);
	XAllocNamedColor(display,colormap,"red",&closest[0],&exact[0]);
	XAllocNamedColor(display,colormap,"coral",&closest[1],&exact[1]);
	XRecolorCursor(display,myCursor,&closest[0],&closest[1]);
	XDefineCursor(display,can_xid,myCursor);
}
/*--------------------------------------------------------------------*/
void
mbnavedit_selectcursor()
{
	myCursor = XCreateFontCursor(display, XC_exchange);
	XAllocNamedColor(display,colormap,"red",&closest[0],&exact[0]);
	XAllocNamedColor(display,colormap,"coral",&closest[1],&exact[1]);
	XRecolorCursor(display,myCursor,&closest[0],&closest[1]);
	XDefineCursor(display,can_xid,myCursor);
}
/*--------------------------------------------------------------------*/
void
mbnavedit_deselectcursor()
{
	myCursor = XCreateFontCursor(display, XC_exchange);
	XAllocNamedColor(display,colormap,"green",&closest[0],&exact[0]);
	XAllocNamedColor(display,colormap,"coral",&closest[1],&exact[1]);
	XRecolorCursor(display,myCursor,&closest[0],&closest[1]);
	XDefineCursor(display,can_xid,myCursor);
}
/*--------------------------------------------------------------------*/
void
mbnavedit_selectallcursor()
{
	myCursor = XCreateFontCursor(display, XC_cross);
	XAllocNamedColor(display,colormap,"red",&closest[0],&exact[0]);
	XAllocNamedColor(display,colormap,"coral",&closest[1],&exact[1]);
	XRecolorCursor(display,myCursor,&closest[0],&closest[1]);
	XDefineCursor(display,can_xid,myCursor);
}
/*--------------------------------------------------------------------*/
void
mbnavedit_deselectallcursor()
{
	myCursor = XCreateFontCursor(display, XC_cross);
	XAllocNamedColor(display,colormap,"green",&closest[0],&exact[0]);
	XAllocNamedColor(display,colormap,"coral",&closest[1],&exact[1]);
	XRecolorCursor(display,myCursor,&closest[0],&closest[1]);
	XDefineCursor(display,can_xid,myCursor);
}
/*--------------------------------------------------------------------*/
void
mbnavedit_setintervalcursor()
{
	myCursor = XCreateFontCursor(display, XC_crosshair);
	XAllocNamedColor(display,colormap,"red",&closest[0],&exact[0]);
	XAllocNamedColor(display,colormap,"coral",&closest[1],&exact[1]);
	XRecolorCursor(display,myCursor,&closest[0],&closest[1]);
	XDefineCursor(display,can_xid,myCursor);
}
/*--------------------------------------------------------------------*/

int
do_wait_until_viewed(XtAppContext app)
{
    Widget  topshell;
    Window  topwindow;
    XWindowAttributes	xwa;
    XEvent  event;
    
    /* set app_context */
    app_context = app;
    
    /* find the top level shell */
    for (topshell = drawingArea; 
	    !XtIsTopLevelShell(topshell);
	    topshell = XtParent(topshell))
	;
	
    /* keep processing events until it is viewed */
    if (XtIsRealized(topshell))
	{
	topwindow = XtWindow(topshell);
	
	/* wait for the window to be mapped */
	while (XGetWindowAttributes(
			XtDisplay(drawingArea), 
			topwindow, &xwa)
		&& xwa.map_state != IsViewable)
	    {
	    XtAppNextEvent(app_context, &event);
	    XtDispatchEvent(&event);
	    }
	}
	
    XmUpdateDisplay(topshell);
		
    return(MB_SUCCESS);
}

/*--------------------------------------------------------------------*/

int
do_message_on(char *message)
{
    Widget  diashell, topshell;
    Window  diawindow, topwindow;
    XWindowAttributes	xwa;
    XEvent  event;
    
    set_label_string(label_message, message);
    XtManageChild(bulletinBoard_message);
    
    /* force the label to be visible */
    for (diashell = label_message; 
	    !XtIsShell(diashell); 
	    diashell = XtParent(diashell))
	;
    for (topshell = diashell; 
	    !XtIsTopLevelShell(topshell);
	    topshell = XtParent(topshell))
	;
    if (XtIsRealized(diashell) && XtIsRealized(topshell))
	{
	diawindow = XtWindow(diashell);
	topwindow = XtWindow(topshell);
	
	/* wait for the dialog to be mapped */
	while (XGetWindowAttributes(display, diawindow, &xwa)
		&& xwa.map_state != IsViewable)
	    {
	    if (XGetWindowAttributes(display, topwindow, &xwa)
		    && xwa.map_state != IsViewable)
		break;
		
	    XtAppNextEvent(app_context, &event);
	    XtDispatchEvent(&event);
	    }
	}
	
    XmUpdateDisplay(topshell);
		
    return(MB_SUCCESS);
}

/*--------------------------------------------------------------------*/

int
do_message_off()
{
    XtUnmanageChild(bulletinBoard_message);
    XSync(XtDisplay(bulletinBoard_message), 0);
    XmUpdateDisplay(drawingArea);
		
    return(MB_SUCCESS);
}

/*--------------------------------------------------------------------*/

int
do_error_dialog(char *s1, char *s2, char *s3)
{
    set_label_string(label_error_one, s1);
    set_label_string(label_error_two, s2);
    set_label_string(label_error_three, s3);
    XtManageChild(bulletinBoard_error);
    XBell(display,100);
		
    return(MB_SUCCESS);
}

/*--------------------------------------------------------------------*/
/* Change label string cleanly, no memory leak */
/*--------------------------------------------------------------------*/

void set_label_string(Widget w, String str)
{
    XmString xstr;
    
    xstr = XmStringCreateLocalized( str );
    if ( xstr != NULL ) 
	XtVaSetValues(w, 
	    XmNlabelString, xstr, 
	    NULL);
    else 
	XtWarning("Failed to update labelString");

    XmStringFree( xstr );
}
/*--------------------------------------------------------------------*/
/* Change multiline label string cleanly, no memory leak */
/*--------------------------------------------------------------------*/

void set_label_multiline_string(Widget w, String str)
{
    XmString xstr;
    int      argok;

    xstr = (XtPointer)BX_CONVERT(w, str, XmRXmString, 0, &argok);
    if ( xstr != NULL && argok)
        XtVaSetValues(w,
            XmNlabelString, xstr,
            NULL);
    else
        XtWarning("Failed to update labelString");

    XmStringFree( xstr );
}
/*--------------------------------------------------------------------*/
/* Get text item string cleanly, no memory leak */
/*--------------------------------------------------------------------*/

void get_text_string(Widget w, String str)
{
    char	*str_tmp;
    
    str_tmp = (char *) XmTextGetString(w);
    strcpy(str, str_tmp);
    XtFree(str_tmp);
}
/*--------------------------------------------------------------------*/
