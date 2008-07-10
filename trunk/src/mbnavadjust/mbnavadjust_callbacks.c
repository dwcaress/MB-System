/*--------------------------------------------------------------------
 *    The MB-system:	mbnavadjust_callbacks.c	2/22/2000
 *    $Id: mbnavadjust_callbacks.c,v 5.15 2008-07-10 17:41:18 caress Exp $
 *
 *    Copyright (c) 2000-2008 by
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
 * mbnavadjust is an interactive navigation adjustment package
 * for swath sonar data.
 * It can work with any data format supported by the MBIO library.
 * This file contains the callback functions from the MOTIF interface.
 *
 * Author:	D. W. Caress
 * Date:	March 22, 2000
 *
 * $Log: not supported by cvs2svn $
 * Revision 5.13  2008/05/16 22:42:32  caress
 * Release 5.1.1beta18 - working towards use of 3D uncertainty.
 *
 * Revision 5.12  2008/01/14 18:15:46  caress
 * Minor fixes.
 *
 * Revision 5.11  2007/05/14 06:34:11  caress
 * Many changes to mbnavadjust, including adding z offsets and 3D search grids.
 *
 * Revision 5.10  2006/06/16 19:30:58  caress
 * Check in after the Santa Monica Basin Mapping AUV Expedition.
 *
 * Revision 5.9  2006/01/24 19:18:42  caress
 * Version 5.0.8 beta.
 *
 * Revision 5.8  2005/06/04 04:34:06  caress
 * Added notion of "truecrossings", so it's possible to process the data while only looking at crossing tracks and ignoring overlap points.
 *
 * Revision 5.7  2004/12/02 06:34:27  caress
 * Fixes while supporting Reson 7k data.
 *
 * Revision 5.6  2004/05/21 23:31:28  caress
 * Moved to new version of BX GUI builder
 *
 * Revision 5.5  2003/04/17 21:07:49  caress
 * Release 5.0.beta30
 *
 * Revision 5.4  2002/08/28 01:32:45  caress
 * Finished first cut at man page.
 *
 * Revision 5.3  2002/03/26 07:43:57  caress
 * Release 5.0.beta15
 *
 * Revision 5.2  2001/07/20 00:33:43  caress
 * Release 5.0.beta03
 *
 * Revision 5.1  2001/01/22 07:45:59  caress
 * Version 5.0.beta01
 *
 * Revision 5.0  2000/12/01  22:55:48  caress
 * First cut at Version 5.0.
 *
 * Revision 4.0  2000/09/30  07:00:06  caress
 * Snapshot for Dale.
 *
 *
 */
/*--------------------------------------------------------------------*/

/* include files */
#include <stdio.h>
#include <stdlib.h>
#include <Xm/Xm.h>
#include <Xm/List.h>
#include <X11/cursorfont.h>
#include <math.h>

#define MBNAVADJUST_DECLARE_GLOBALS
#include "mbnavadjust_extrawidgets.h"
#include "mbnavadjust.h"
#include "mb_define.h"
#include "mb_status.h"

#include "mbnavadjust_creation.h"

/*--------------------------------------------------------------------*/

/*
 * Standard includes for builtins.
 */
#include <stdio.h>
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

/* XG variable declarations */
#define xgfont "-misc-fixed-bold-r-normal-*-13-*-75-75-c-70-iso8859-1"
#define EV_MASK (ButtonPressMask | ButtonReleaseMask | ButtonMotionMask | KeyPressMask | KeyReleaseMask | ExposureMask)
XtAppContext	app_context;
Display		*display;
Window		cont_xid, corr_xid, zoff_xid;
Colormap	colormap;
GC		cont_gc, corr_gc, modp_gc;
XGCValues	xgcv;
XFontStruct	*fontStruct;
#define	XG_SOLIDLINE	0
#define	XG_DASHLINE	1
int	cont_xgid = 0;		/* XG graphics id */
int	corr_xgid = 0;		/* XG graphics id */
int	zoff_xgid = 0;		/* XG graphics id */
int	modp_xgid = 0;		/* XG graphics id */
Cursor myCursor;

/* Set the colors used for this program here. */
#define NCOLORS 256
XColor colors[NCOLORS];
unsigned int mpixel_values[NCOLORS];
XColor db_color;

/* Set these to the dimensions of your canvas drawing */
/* areas, minus 1, located in the uil file             */
static int cont_borders[4] = { 0, 600, 0, 600 };
static int corr_borders[4] = { 0, 301, 0, 301 };
static int zoff_borders[4] = { 0, 300, 0, 60 };
static int modp_borders[4];

/* file opening parameters */
#define FILE_MODE_NONE 		0
#define FILE_MODE_NEW 		1
#define FILE_MODE_OPEN 		2
#define FILE_MODE_IMPORT 	3
int	file_mode = FILE_MODE_NONE;
int	format = 0;
int	expose_plot_ok = True;
int selected = 0; /* indicates an input file is selected */

/* button parameters */
static int button1down = MB_NO;
static int button2down = MB_NO;
static int button3down = MB_NO;
static int loc_x, loc_y;
static int mod_loc_x, mod_loc_y;

int	status;
char	string[STRING_MAX];

Cardinal  ac = 0;
Arg       args[256];
int       argok;
XmString  tmp0;

/*--------------------------------------------------------------------*/

/* local function prototype definitions */
void	do_list_data_select( Widget w, XtPointer client_data, XtPointer call_data);
void	do_naverr_cont_expose( Widget w, XtPointer client_data, XtPointer call_data);
void	do_naverr_corr_expose( Widget w, XtPointer client_data, XtPointer call_data);
void	do_naverr_cont_input( Widget w, XtPointer client_data, XtPointer call_data);
void	do_naverr_corr_input( Widget w, XtPointer client_data, XtPointer call_data);
void	do_naverr_zcorr_input( Widget w, XtPointer client_data, XtPointer call_data);
void	do_naverr_previous( Widget w, XtPointer client_data, XtPointer call_data);
void	do_naverr_next( Widget w, XtPointer client_data, XtPointer call_data);
void	do_naverr_nextunset( Widget w, XtPointer client_data, XtPointer call_data);
void	do_naverr_addtie( Widget w, XtPointer client_data, XtPointer call_data);
void	do_naverr_deletetie( Widget w, XtPointer client_data, XtPointer call_data);
void	do_naverr_selecttie( Widget w, XtPointer client_data, XtPointer call_data);
void	do_naverr_settie( Widget w, XtPointer client_data, XtPointer call_data);
void	do_naverr_resettie( Widget w, XtPointer client_data, XtPointer call_data);
void	do_naverr_setnone( Widget w, XtPointer client_data, XtPointer call_data);
void	do_naverr_zerooffset( Widget w, XtPointer client_data, XtPointer call_data);
void	do_naverr_zerozoffset( Widget w, XtPointer client_data, XtPointer call_data);
void	do_dismiss_naverr( Widget w, XtPointer client_data, XtPointer call_data);
void	do_naverr_minmisfit( Widget w, XtPointer client_data, XtPointer call_data);
void	do_naverr_misfitcenter( Widget w, XtPointer client_data, XtPointer call_data);
void	do_file_new( Widget w, XtPointer client_data, XtPointer call_data);
void	do_file_open( Widget w, XtPointer client_data, XtPointer call_data);
void	do_file_close( Widget w, XtPointer client_data, XtPointer call_data);
void	do_file_importdata( Widget w, XtPointer client_data, XtPointer call_data);
void	do_quit( Widget w, XtPointer client_data, XtPointer call_data);
void	do_fileselection_mode( Widget w, XtPointer client_data, XtPointer call_data);
void	do_fileselection_ok( Widget w, XtPointer client_data, XtPointer call_data);
void	do_fileselection_cancel( Widget w, XtPointer client_data, XtPointer call_data);
void	do_view_showdata( Widget w, XtPointer client_data, XtPointer call_data);
void	do_view_showcrossings( Widget w, XtPointer client_data, XtPointer call_data);
void	do_view_showgoodcrossings( Widget w, XtPointer client_data, XtPointer call_data);
void	do_view_showtruecrossings( Widget w, XtPointer client_data, XtPointer call_data);
void	do_view_showties( Widget w, XtPointer client_data, XtPointer call_data);
void	do_action_autopick( Widget w, XtPointer client_data, XtPointer call_data);
void	do_action_analyzecrossings( Widget w, XtPointer client_data, XtPointer call_data);
void	do_action_invertnav( Widget w, XtPointer client_data, XtPointer call_data);
void	do_fileselection_list(Widget w, XtPointer client, XtPointer call);
int	do_wait_until_viewed(XtAppContext app);
void	set_label_string(Widget w, String str);
void	set_label_multiline_string(Widget w, String str);
void	get_text_string(Widget w, String str);
int	do_info_add(char *info, int timetag);
void	do_modelplot_resize( Widget w, XtPointer client_data, XEvent *event, Boolean *unused);

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
}

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

#ifdef VMS
#include <stdlib.h>
#endif

/* ARGSUSED */
void
BxExitCB ARGLIST((w, client, call))
UARG( Widget, w)
ARG( XtPointer, client)
GRAU( XtPointer, call)
{
    long	exitValue = (long)client;
    exit(exitValue);
}

/*--------------------------------------------------------------------*/

void
do_mbnavadjust_init(int argc, char **argv)
{
    int	    i,j;
    String translations = 
	    "<Btn1Down>:	DrawingAreaInput() ManagerGadgetArm() \n\
	     <Btn1Up>:		DrawingAreaInput() ManagerGadgetActivate() \n\
	     <Btn1Motion>:	DrawingAreaInput() ManagerGadgetButtonMotion() \n\
	     <Btn2Down>:	DrawingAreaInput() ManagerGadgetArm() \n\
	     <Btn2Up>:		DrawingAreaInput() ManagerGadgetActivate() \n\
	     <Btn2Motion>:	DrawingAreaInput() ManagerGadgetButtonMotion() \n\
	     <Btn3Down>:	DrawingAreaInput() ManagerGadgetArm() \n\
	     <Btn3Up>:		DrawingAreaInput() ManagerGadgetActivate() \n\
	     <Btn3Motion>:	DrawingAreaInput() ManagerGadgetButtonMotion() \n\
	     <KeyDown>:		DrawingAreaInput() \n\
	     <KeyUp>:		DrawingAreaInput() ManagerGadgetKeyInput()";

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
    ac = 0;
    tmp0 = (XmString) BX_CONVERT(fileSelectionBox, "*.nvh", 
                                          XmRXmString, 0, &argok);
    XtSetArg(args[ac], XmNpattern, tmp0); ac++;
    XtSetValues(fileSelectionBox, args, ac);
    XmStringFree((XmString)tmp0);
				    
    /* reset translation table for drawingArea widgets */
    XtVaSetValues(drawingArea_naverr_cont,
			XmNtranslations, XtParseTranslationTable(translations),
			NULL);
    XtVaSetValues(drawingArea_naverr_corr,
			XmNtranslations, XtParseTranslationTable(translations),
			NULL);
    XtVaSetValues(drawingArea_naverr_zcorr,
			XmNtranslations, XtParseTranslationTable(translations),
			NULL);
    XtVaSetValues(drawingArea_modelplot,
			XmNtranslations, XtParseTranslationTable(translations),
			NULL);

    /* add resize event handler to modelplot */
    XtAddEventHandler(bulletinBoard_modelplot, 
			StructureNotifyMask, 
			False, 
			do_modelplot_resize, 
			(XtPointer)0);
				
    /* Setup the entire screen. */
    display = XtDisplay(form_mbnavadjust);
    colormap = DefaultColormap(display,XDefaultScreen(display));
    
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
    status = XLookupColor(display,colormap, "coral",&db_color,&colors[5]);
    if ((status = XAllocColor(display,colormap,&colors[5])) == 0)
	    fprintf(stderr,"Failure to allocate color: coral\n");
    status = XLookupColor(display,colormap, "yellow",&db_color,&colors[6]);
    if ((status = XAllocColor(display,colormap,&colors[6])) == 0)
	    fprintf(stderr,"Failure to allocate color: yellow\n");
    j = 7;
    for (i=0;i<16;i++)
    	    {
    	    colors[j+i].red = 65535;
    	    colors[j+i].green = i * 4096;
    	    colors[j+i].blue = 0;
	    status = XAllocColor(display,colormap,&colors[j+i]);
            if (status == 0)
	    	{
	    	fprintf(stderr,"Failure to allocate color[%d]: %d %d %d\n",
	    		j+i,colors[j+i].red,
	    		colors[j+i].green,
	    		colors[j+i].blue);
	    	}
    	    }
    j += 16;
    for (i=0;i<16;i++)
    	    {
    	    colors[j+i].red = 65535 - i * 4096;
    	    colors[j+i].green = 65535;
    	    colors[j+i].blue = 0;
	    status = XAllocColor(display,colormap,&colors[j+i]);
            if (status == 0)
	    	{
	    	fprintf(stderr,"Failure to allocate color[%d]: %d %d %d\n",
	    		j+i,colors[j+i].red,
	    		colors[j+i].green,
	    		colors[j+i].blue);
	    	}
    	    }
    j += 16;
    for (i=0;i<16;i++)
    	    {
    	    colors[j+i].red = 0;
    	    colors[j+i].green = 65535;
    	    colors[j+i].blue = i * 4096;
	    status = XAllocColor(display,colormap,&colors[j+i]);
            if (status == 0)
	    	{
	    	fprintf(stderr,"Failure to allocate color[%d]: %d %d %d\n",
	    		j+i,colors[j+i].red,
	    		colors[j+i].green,
	    		colors[j+i].blue);
	    	}
    	    }
    j += 16;
    for (i=0;i<16;i++)
    	    {
    	    colors[j+i].red = 0;
    	    colors[j+i].green = 65535 - i * 4096;
    	    colors[j+i].blue = 65535;
	    status = XAllocColor(display,colormap,&colors[j+i]);
            if (status == 0)
	    	{
	    	fprintf(stderr,"Failure to allocate color[%d]: %d %d %d\n",
	    		j+i,colors[j+i].red,
	    		colors[j+i].green,
	    		colors[j+i].blue);
	    	}
    	    }
    j += 16;
    for (i=0;i<16;i++)
    	    {
    	    colors[j+i].red = i * 4096;
    	    colors[j+i].green = 0;
    	    colors[j+i].blue = 65535;
	    status = XAllocColor(display,colormap,&colors[j+i]);
            if (status == 0)
	    	{
	    	fprintf(stderr,"Failure to allocate color[%d]: %d %d %d\n",
	    		j+i,colors[j+i].red,
	    		colors[j+i].green,
	    		colors[j+i].blue);
	    	}
    	    }
    j += 16;
    colors[j].red = 65535;
    colors[j].green = 0;
    colors[j].blue = 65535;
    status = XAllocColor(display,colormap,&colors[j]);
    if (status == 0)
	{
	fprintf(stderr,"Failure to allocate color[%d]: %d %d %d\n",
	    		j,colors[j].red,
	    		colors[j].green,
	    		colors[j].blue);
	}
    for (i=0;i<NCOLORS;i++)
	    {
	    mpixel_values[i] = colors[i].pixel;
	    }
    status = mbnavadjust_set_colors(NCOLORS, (int *) mpixel_values);
    status = mbnavadjust_set_borders(cont_borders, corr_borders, zoff_borders);

    /* set verbose */
    mbna_verbose = 0;
	    
    /* put up info text */
    sprintf(string, "Program MBnavadjust initialized.\nMB-System Release %s %s\n", 
		MB_VERSION, MB_BUILD_DATE);
    do_info_add(string, MB_YES);

    /* initialize mbnavadjust proper */
    status = mbnavadjust_init_globals();
    status = mbnavadjust_init(argc,argv);
    do_set_controls();
    do_update_status();
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

}
/*--------------------------------------------------------------------*/

void do_update_status()
{
    	XmString *xstr;
    	struct mbna_file *file;
    	struct mbna_crossing *crossing;
    	struct mbna_tie *tie;
    	char	status_char;
    	char	truecrossing;
	int	ivalue, imax;
	int	tie_pos;
	double	dr1, dr2, dr3;
    	int	i, j, k;

	/* set status label */
        sprintf(string,":::t\"Project:                                       %s\":t\"Number of Files:                           %d\":t\"Number of Crossings Found:         %d\":t\"Number of Crossings Analyzed:     %d\":t\"Number of True Crossings:          %d\":t\"Number of True Crossings Analyzed:%d\":t\"Number of Ties Set:                      %d\"",
                project.name,project.num_files,
		project.num_crossings,project.num_crossings_analyzed, 
		project.num_truecrossings,project.num_truecrossings_analyzed, 
		project.num_ties);
        if (project.inversion == MBNA_INVERSION_CURRENT)
        	strcat(string,":t\"Inversion Performed:                    Current\"");
        else if (project.inversion == MBNA_INVERSION_OLD)
        	strcat(string,":t\"Inversion Performed:                    Out of Date\"");
        else
        	strcat(string,":t\"Inversion Performed:                    No\"");
	set_label_multiline_string(label_status, string);
	if (mbna_verbose > 0)
		{
        	sprintf(string,"Project:                                       %s\nNumber of Files:                           %d\nNumber of Crossings Found:         %d\nNumber of Crossings Analyzed:     %d\nNumber of True Crossings:        %d\nNumber of True Crossings Analyzed:%d\nNumber of Ties Set:                     %d\n",
                	project.name,project.num_files,
			project.num_crossings,project.num_crossings_analyzed, 
			project.num_truecrossings,project.num_truecrossings_analyzed, 
			project.num_ties);
        	if (project.inversion == MBNA_INVERSION_CURRENT)
        		strcat(string,"Inversion Performed:                    Current\n");
        	else if (project.inversion == MBNA_INVERSION_OLD)
        		strcat(string,"Inversion Performed:                    Out of Date\n");
        	else
        		strcat(string,"Inversion Performed:                    No\n");
		fprintf(stderr,string);
		}

	/* set list_data */
	XmListDeleteAllItems(list_data);
	if (mbna_view_list == MBNA_VIEW_LIST_FILES)
		{
		set_label_string(label_listdata,"Data Files:");
		if (mbna_verbose > 0)
			fprintf(stderr,"Data Files:\n");
		if (project.num_files > 0)
			{
			xstr = (XmString *) malloc(project.num_files * sizeof(XmString));
			for (i=0;i<project.num_files;i++)
				{
				file = &(project.files[i]);
				if (file->status == MBNA_FILE_FIXED)
				    sprintf(string,"%4d %4d fixed %4.1f %4.1f %s",
					    file->id,file->num_sections,
					    file->heading_bias,file->roll_bias,
					    file->file);
				else
				    sprintf(string,"%4d %4d  free %4.1f %4.1f %s",
					    file->id,file->num_sections,
					    file->heading_bias,file->roll_bias,
					    file->file);
    				xstr[i] = XmStringCreateLocalized(string);
				if (mbna_verbose > 0)
					fprintf(stderr,"%s\n",string);
 				}
    			XmListAddItems(list_data,xstr,project.num_files,0);
			for (i=0;i<project.num_files;i++)
				{
    				XmStringFree(xstr[i]);
    				}
    			free(xstr);
			}
		if (mbna_file_select != MBNA_SELECT_NONE)
			{
			XmListSelectPos(list_data,mbna_file_select+1,0);
			XmListSetPos(list_data,
				MAX(mbna_file_select+1-5, 1));
			}
		}
 	else if (mbna_view_list == MBNA_VIEW_LIST_CROSSINGS)
		{
		set_label_string(label_listdata,"Crossings:");
		if (mbna_verbose > 0)
			fprintf(stderr,"Crossings:\n");
		if (project.num_files > 0)
			{
			xstr = (XmString *) malloc(project.num_crossings * sizeof(XmString));
			for (i=0;i<project.num_crossings;i++)
				{
				crossing = &(project.crossings[i]);
				if (crossing->status == MBNA_CROSSING_STATUS_NONE)
					status_char = 'U';
				else if (crossing->status == MBNA_CROSSING_STATUS_SET)
					status_char = '*';
				else
					status_char = '-';
				if (crossing->truecrossing == MB_NO)
					truecrossing = ' ';
				else
					truecrossing = 'X';
				sprintf(string,"%c%c %4d %3.3d:%3.3d %3.3d:%3.3d %3d %2d",
					status_char, truecrossing, i,
					crossing->file_id_1,
					crossing->section_1,
					crossing->file_id_2,
					crossing->section_2,
					crossing->overlap,
					crossing->num_ties);
    				xstr[i] = XmStringCreateLocalized(string);
				if (mbna_verbose > 0)
					fprintf(stderr,"%s\n",string);
				}
    			XmListAddItems(list_data,xstr,project.num_crossings,0);
			for (i=0;i<project.num_crossings;i++)
				{
    				XmStringFree(xstr[i]);
    				}
    			free(xstr);
			}
		if (mbna_crossing_select != MBNA_SELECT_NONE)
			{
			XmListSelectPos(list_data,mbna_crossing_select+1,0);
			XmListSetPos(list_data,
				MAX(mbna_crossing_select+1-5, 1));
			}
		}
 	else if (mbna_view_list == MBNA_VIEW_LIST_GOODCROSSINGS)
		{
		set_label_string(label_listdata,">25% Overlap Crossings:");
		if (mbna_verbose > 0)
			fprintf(stderr,">25% Overlap Crossings:\n");
		if (project.num_files > 0)
			{
			xstr = (XmString *) malloc(project.num_crossings * sizeof(XmString));
			j = 0;
			for (i=0;i<project.num_crossings;i++)
				{
				crossing = &(project.crossings[i]);
				if (crossing->overlap >= MBNA_OVERLAP_THRESHOLD)
					{
					if (crossing->status == MBNA_CROSSING_STATUS_NONE)
						status_char = 'U';
					else if (crossing->status == MBNA_CROSSING_STATUS_SET)
						status_char = '*';
					else
						status_char = '-';
					if (crossing->truecrossing == MB_NO)
						truecrossing = ' ';
					else
						truecrossing = 'X';
					sprintf(string,"%c%c %4d %3.3d:%3.3d %3.3d:%3.3d %3d %2d",
						status_char, truecrossing, i,
						crossing->file_id_1,
						crossing->section_1,
						crossing->file_id_2,
						crossing->section_2,
						crossing->overlap,
						crossing->num_ties);
    					xstr[j] = XmStringCreateLocalized(string);
					if (mbna_verbose > 0)
						fprintf(stderr,"%s\n",string);
					j++;
					}
				}
    			XmListAddItems(list_data,xstr,project.num_goodcrossings,0);
			for (i=0;i<j;i++)
				{
    				XmStringFree(xstr[i]);
    				}
    			free(xstr);
			}
		if (mbna_crossing_select != MBNA_SELECT_NONE
			&& project.crossings[mbna_crossing_select].overlap >= MBNA_OVERLAP_THRESHOLD)
			{
			j = 0;
			for (i=0;i<=mbna_crossing_select;i++)
				{
				crossing = &(project.crossings[i]);
				if (crossing->overlap >= MBNA_OVERLAP_THRESHOLD)
					{
					j++;
					}
				}
			if (j > 0)
				{
				XmListSelectPos(list_data,j,0);
				XmListSetPos(list_data,
					MAX(j-5, 1));
				}
			}
		}
 	else if (mbna_view_list == MBNA_VIEW_LIST_TRUECROSSINGS)
		{
		set_label_string(label_listdata,"True Crossings:");
		if (mbna_verbose > 0)
			fprintf(stderr,"True Crossings:\n");
		if (project.num_files > 0)
			{
			xstr = (XmString *) malloc(project.num_truecrossings * sizeof(XmString));
			j = 0;
			for (i=0;i<project.num_crossings;i++)
				{
				crossing = &(project.crossings[i]);
				if (crossing->truecrossing == MB_YES)
					{
					if (crossing->status == MBNA_CROSSING_STATUS_NONE)
						status_char = 'U';
					else if (crossing->status == MBNA_CROSSING_STATUS_SET)
						status_char = '*';
					else
						status_char = '-';
					truecrossing = 'X';
					sprintf(string,"%c%c %4d %3.3d:%3.3d %3.3d:%3.3d %3d %2d",
						status_char, truecrossing, i,
						crossing->file_id_1,
						crossing->section_1,
						crossing->file_id_2,
						crossing->section_2,
						crossing->overlap,
						crossing->num_ties);
    					xstr[j] = XmStringCreateLocalized(string);
					if (mbna_verbose > 0)
						fprintf(stderr,"%s\n",string);
					j++;
					}
				}
    			XmListAddItems(list_data,xstr,project.num_truecrossings,0);
			for (i=0;i<j;i++)
				{
    				XmStringFree(xstr[i]);
    				}
    			free(xstr);
			}
		if (mbna_crossing_select != MBNA_SELECT_NONE
			&& project.crossings[mbna_crossing_select].truecrossing == MB_YES)
			{
			j = 0;
			for (i=0;i<=mbna_crossing_select;i++)
				{
				crossing = &(project.crossings[i]);
				if (crossing->truecrossing == MB_YES)
					{
					j++;
					}
				}
			if (j > 0)
				{
				XmListSelectPos(list_data,j,0);
				XmListSetPos(list_data,
					MAX(j-5, 1));
				}
			}
		}
 	else if (mbna_view_list == MBNA_VIEW_LIST_TIES)
		{
		set_label_string(label_listdata,"Ties:");
        /*sprintf(string,":::t\"Ties:\":t\" Xing  Tie Fil:Sec:Nv  Fil:Sec:Nv      OffLon      OffLat     dOffLon     dOffLat\"");
	set_label_multiline_string(label_listdata, string);*/
		if (mbna_verbose > 0)
        		fprintf(stderr,"Ties:\n Xing  Tie Fil:Sec:Nv  Fil:Sec:Nv      OffLon      OffLat     dOffLon     dOffLat\n");
		if (project.num_files > 0)
			{
			xstr = (XmString *) malloc(project.num_ties * sizeof(XmString));
			k = 0;
			tie_pos = 0;
			for (i=0;i<project.num_crossings;i++)
			    {
			    crossing = &(project.crossings[i]);
			    for (j=0;j<crossing->num_ties;j++)
				{
				tie = (struct mbna_tie *) &crossing->ties[j];
				if (tie->inversion_status == MBNA_INVERSION_CURRENT
					|| tie->inversion_status == MBNA_INVERSION_OLD)
					{
					dr1 = fabs((tie->inversion_offset_x_m - tie->offset_x_m) * tie->sigmax1[0]
						+ (tie->inversion_offset_y_m - tie->offset_y_m) * tie->sigmax1[1]
						+ (tie->inversion_offset_z_m - tie->offset_z_m) * tie->sigmax1[2]) / tie->sigmar1;
					dr2 = fabs((tie->inversion_offset_x_m - tie->offset_x_m) * tie->sigmax2[0]
						+ (tie->inversion_offset_y_m - tie->offset_y_m) * tie->sigmax2[1]
						+ (tie->inversion_offset_z_m - tie->offset_z_m) * tie->sigmax2[2]) / tie->sigmar2;
					dr3 = fabs((tie->inversion_offset_x_m - tie->offset_x_m) * tie->sigmax3[0]
						+ (tie->inversion_offset_y_m - tie->offset_y_m) * tie->sigmax3[1]
						+ (tie->inversion_offset_z_m - tie->offset_z_m) * tie->sigmax3[2]) / tie->sigmar3;
					}
				if (tie->inversion_status == MBNA_INVERSION_CURRENT)
				    sprintf(string,"%4d %2d %3.3d:%3.3d:%2.2d %3.3d:%3.3d:%2.2d %2.2d:%2.2d %8.2f %8.2f %8.2f | %8.2f %8.2f %8.2f | %4.3f %4.3f %4.3f",
					i, j,
					crossing->file_id_1,
					crossing->section_1,
					tie->snav_1,
					crossing->file_id_2,
					crossing->section_2,
					tie->snav_2,
					project.files[crossing->file_id_1].block,
					project.files[crossing->file_id_2].block,
					tie->offset_x_m,
					tie->offset_y_m,
					tie->offset_z_m,
					tie->sigmar1,
					tie->sigmar2,
					tie->sigmar3,
					dr1, dr2, dr3);
				else if (tie->inversion_status == MBNA_INVERSION_OLD)
				    sprintf(string,"%4d %2d %3.3d:%3.3d:%2.2d %3.3d:%3.3d:%2.2d %2.2d:%2.2d %8.2f %8.2f %8.2f | %8.2f %8.2f %8.2f | %4.3f %4.3f %4.3f ***",
					i, j,
					crossing->file_id_1,
					crossing->section_1,
					tie->snav_1,
					crossing->file_id_2,
					crossing->section_2,
					tie->snav_2,
					project.files[crossing->file_id_1].block,
					project.files[crossing->file_id_2].block,
					tie->offset_x_m,
					tie->offset_y_m,
					tie->offset_z_m,
					tie->sigmar1,
					tie->sigmar2,
					tie->sigmar3,
					dr1, dr2, dr3);
				else
				    sprintf(string,"%4d %2d %3.3d:%3.3d:%2.2d %3.3d:%3.3d:%2.2d %2.2d:%2.2d %8.2f %8.2f %8.2f | %8.2f %8.2f %8.2f",
					i, j,
					crossing->file_id_1,
					crossing->section_1,
					tie->snav_1,
					crossing->file_id_2,
					crossing->section_2,
					tie->snav_2,
					project.files[crossing->file_id_1].block,
					project.files[crossing->file_id_2].block,
					tie->offset_x_m,
					tie->offset_y_m,
					tie->offset_z_m,
					tie->sigmar1,
					tie->sigmar2,
					tie->sigmar3);
    				xstr[k] = XmStringCreateLocalized(string);
				if (mbna_verbose > 0)
					fprintf(stderr,"%s\n",string);
				if (i == mbna_crossing_select
				    && j == mbna_tie_select)
				    tie_pos = k;
				k++;
				}
			    }
    			XmListAddItems(list_data,xstr,project.num_ties,0);
			for (k=0;k<project.num_ties;k++)
				{
    				XmStringFree(xstr[k]);
    				}
    			free(xstr);
			}
		if (mbna_tie_select != MBNA_SELECT_NONE
			&& mbna_crossing_select != MBNA_SELECT_NONE)
			{
			XmListSelectPos(list_data,tie_pos+1,0);
			XmListSetPos(list_data,
				MAX(tie_pos+1-5, 1));
			}
		}
	
	if (mbna_view_list == MBNA_VIEW_LIST_FILES
		&& project.num_files > 0
		&& mbna_file_select != MBNA_SELECT_NONE
		&& project.files[mbna_file_select].status != MBNA_FILE_FIXED)
		{
		XtVaSetValues(pushButton_fix,
			XmNsensitive, True,
			NULL);
		XtVaSetValues(pushButton_unfix,
			XmNsensitive, False,
			NULL);
		}
	else if (mbna_view_list == MBNA_VIEW_LIST_FILES
		&& project.num_files > 0
		&& mbna_file_select != MBNA_SELECT_NONE
		&& project.files[mbna_file_select].status == MBNA_FILE_FIXED)
		{
		XtVaSetValues(pushButton_fix,
			XmNsensitive, False,
			NULL);
		XtVaSetValues(pushButton_unfix,
			XmNsensitive, True,
			NULL);
		}
	else
		{
		XtVaSetValues(pushButton_fix,
			XmNsensitive, False,
			NULL);
		XtVaSetValues(pushButton_unfix,
			XmNsensitive, False,
			NULL);
		}
   	
	if (mbna_status != MBNA_STATUS_GUI)
		{ 	
		XtVaSetValues(pushButton_new,
			XmNsensitive, False,
			NULL);
		XtVaSetValues(pushButton_open,
			XmNsensitive, False,
			NULL);
		XtVaSetValues(pushButton_close,
			XmNsensitive, False,
			NULL);
		}
	else if (project.open == MB_YES)
		{ 	
		XtVaSetValues(pushButton_new,
			XmNsensitive, False,
			NULL);
		XtVaSetValues(pushButton_open,
			XmNsensitive, False,
			NULL);
		XtVaSetValues(pushButton_close,
			XmNsensitive, True,
			NULL);
		}
	else
		{
		XtVaSetValues(pushButton_new,
			XmNsensitive, True,
			NULL);
		XtVaSetValues(pushButton_open,
			XmNsensitive, True,
			NULL);
		XtVaSetValues(pushButton_close,
			XmNsensitive, False,
			NULL);
		}
	if (mbna_status == MBNA_STATUS_GUI
		&& project.open == MB_YES
		&& project.num_files >= 0)
		{
		XtVaSetValues(pushButton_importdata,
			XmNsensitive, True,
			NULL);
		}
	else
		{
		XtVaSetValues(pushButton_importdata,
			XmNsensitive, False,
			NULL);
		}		
	if (project.open == MB_YES && project.num_files > 0)
		{
		if (mbna_view_list == MBNA_VIEW_LIST_FILES)
			{
			XtVaSetValues(pushButton_showdata,
				XmNsensitive, False,
				NULL);
			XtVaSetValues(pushButton_showcrossings,
				XmNsensitive, True,
				NULL);
			XtVaSetValues(pushButton_showgoodcrossings,
				XmNsensitive, True,
				NULL);
			XtVaSetValues(pushButton_showtruecrossings,
				XmNsensitive, True,
				NULL);
			XtVaSetValues(pushButton_showties,
				XmNsensitive, True,
				NULL);
			XtVaSetValues(pushButton_naverr_previous,
				XmNsensitive, False,
				NULL);
			XtVaSetValues(pushButton_naverr_next,
				XmNsensitive, False,
				NULL);
			XtVaSetValues(pushButton_naverr_nextunset,
				XmNsensitive, False,
				NULL);
			}
		else if (mbna_view_list == MBNA_VIEW_LIST_CROSSINGS)
			{
			XtVaSetValues(pushButton_showdata,
				XmNsensitive, True,
				NULL);
			XtVaSetValues(pushButton_showcrossings,
				XmNsensitive, False,
				NULL);
			XtVaSetValues(pushButton_showgoodcrossings,
				XmNsensitive, True,
				NULL);
			XtVaSetValues(pushButton_showtruecrossings,
				XmNsensitive, True,
				NULL);
			XtVaSetValues(pushButton_showties,
				XmNsensitive, True,
				NULL);
			XtVaSetValues(pushButton_naverr_previous,
				XmNsensitive, True,
				NULL);
			XtVaSetValues(pushButton_naverr_next,
				XmNsensitive, True,
				NULL);
			if (project.num_crossings == project.num_crossings_analyzed)
				XtVaSetValues(pushButton_naverr_nextunset,
					XmNsensitive, False,
					NULL);
			else
				XtVaSetValues(pushButton_naverr_nextunset,
					XmNsensitive, True,
					NULL);
			}
		else if (mbna_view_list == MBNA_VIEW_LIST_GOODCROSSINGS)
			{
			XtVaSetValues(pushButton_showdata,
				XmNsensitive, True,
				NULL);
			XtVaSetValues(pushButton_showcrossings,
				XmNsensitive, True,
				NULL);
			XtVaSetValues(pushButton_showgoodcrossings,
				XmNsensitive, False,
				NULL);
			XtVaSetValues(pushButton_showtruecrossings,
				XmNsensitive, True,
				NULL);
			XtVaSetValues(pushButton_showties,
				XmNsensitive, True,
				NULL);
			XtVaSetValues(pushButton_naverr_previous,
				XmNsensitive, True,
				NULL);
			XtVaSetValues(pushButton_naverr_next,
				XmNsensitive, True,
				NULL);
			if (project.num_crossings == project.num_crossings_analyzed)
				XtVaSetValues(pushButton_naverr_nextunset,
					XmNsensitive, False,
					NULL);
			else
				XtVaSetValues(pushButton_naverr_nextunset,
					XmNsensitive, True,
					NULL);
			}
		else if (mbna_view_list == MBNA_VIEW_LIST_TRUECROSSINGS)
			{
			XtVaSetValues(pushButton_showdata,
				XmNsensitive, True,
				NULL);
			XtVaSetValues(pushButton_showcrossings,
				XmNsensitive, True,
				NULL);
			XtVaSetValues(pushButton_showgoodcrossings,
				XmNsensitive, True,
				NULL);
			XtVaSetValues(pushButton_showtruecrossings,
				XmNsensitive, False,
				NULL);
			XtVaSetValues(pushButton_showties,
				XmNsensitive, True,
				NULL);
			XtVaSetValues(pushButton_naverr_previous,
				XmNsensitive, True,
				NULL);
			XtVaSetValues(pushButton_naverr_next,
				XmNsensitive, True,
				NULL);
			if (project.num_truecrossings == project.num_truecrossings_analyzed)
				XtVaSetValues(pushButton_naverr_nextunset,
					XmNsensitive, False,
					NULL);
			else
				XtVaSetValues(pushButton_naverr_nextunset,
					XmNsensitive, True,
					NULL);
			}
		else if (mbna_view_list == MBNA_VIEW_LIST_TIES)
			{
			XtVaSetValues(pushButton_showdata,
				XmNsensitive, True,
				NULL);
			XtVaSetValues(pushButton_showcrossings,
				XmNsensitive, True,
				NULL);
			XtVaSetValues(pushButton_showgoodcrossings,
				XmNsensitive, True,
				NULL);
			XtVaSetValues(pushButton_showtruecrossings,
				XmNsensitive, True,
				NULL);
			XtVaSetValues(pushButton_showties,
				XmNsensitive, False,
				NULL);
			XtVaSetValues(pushButton_naverr_previous,
				XmNsensitive, True,
				NULL);
			XtVaSetValues(pushButton_naverr_next,
				XmNsensitive, True,
				NULL);
			XtVaSetValues(pushButton_naverr_nextunset,
				XmNsensitive, False,
				NULL);
			}
		}
	else
		{
		XtVaSetValues(pushButton_showdata,
			XmNsensitive, False,
			NULL);
		XtVaSetValues(pushButton_showcrossings,
			XmNsensitive, False,
			NULL);
		XtVaSetValues(pushButton_showgoodcrossings,
			XmNsensitive, False,
			NULL);
		XtVaSetValues(pushButton_showtruecrossings,
			XmNsensitive, False,
			NULL);
		XtVaSetValues(pushButton_showties,
			XmNsensitive, False,
			NULL);
		XtVaSetValues(pushButton_naverr_previous,
			XmNsensitive, True,
			NULL);
		XtVaSetValues(pushButton_naverr_next,
			XmNsensitive, True,
			NULL);
		XtVaSetValues(pushButton_naverr_nextunset,
			XmNsensitive, True,
			NULL);
		}
		
	if (mbna_status == MBNA_STATUS_GUI
		&& project.open == MB_YES
		&& project.num_files > 0)
		{
		XtVaSetValues(pushButton_autopick,
			XmNsensitive, True,
			NULL);
		XtVaSetValues(pushButton_makegrid,
			XmNsensitive, False,
			NULL);
		XtVaSetValues(pushButton_analyzecrossings,
			XmNsensitive, True,
			NULL);
		if (project.num_truecrossings == project.num_truecrossings_analyzed
			|| project.num_crossings == project.num_truecrossings_analyzed)
			XtVaSetValues(pushButton_invertnav,
				XmNsensitive, True,
				NULL);
		else
			XtVaSetValues(pushButton_invertnav,
				XmNsensitive, False,
				NULL);
		if (project.inversion != MBNA_INVERSION_NONE)
			XtVaSetValues(pushButton_showmodelplot,
				XmNsensitive, True,
				NULL);
		else
			XtVaSetValues(pushButton_showmodelplot,
				XmNsensitive, False,
				NULL);
		XtVaSetValues(pushButton_updateproject,
			XmNsensitive, False,
			NULL);
		}
	else
		{
		XtVaSetValues(pushButton_autopick,
			XmNsensitive, False,
			NULL);
		XtVaSetValues(pushButton_makegrid,
			XmNsensitive, False,
			NULL);
		XtVaSetValues(pushButton_analyzecrossings,
			XmNsensitive, False,
			NULL);
		XtVaSetValues(pushButton_invertnav,
			XmNsensitive, False,
			NULL);
		XtVaSetValues(pushButton_updateproject,
			XmNsensitive, False,
			NULL);
		XtVaSetValues(pushButton_showmodelplot,
			XmNsensitive, False,
			NULL);
		}

	/* set values of decimation slider */
	XtVaSetValues(scale_controls_decimation, 
			XmNvalue, project.decimation, 
			NULL);

	/* set values of section length slider */
	ivalue = (int) (100 * project.section_length);
	imax = (int) (100 * 50.0);
	XtVaSetValues(scale_controls_sectionlength, 
			XmNminimum, 1, 
			XmNmaximum, imax, 
			XmNdecimalPoints, 2, 
			XmNvalue, ivalue, 
			NULL);

	/* set values of section soundings slider */
	ivalue = project.section_soundings;
	XtVaSetValues(scale_controls_sectionsoundings, 
			XmNvalue, ivalue, 
			NULL);

	/* set values of contour interval slider */
	ivalue = (int) (100 * project.cont_int);
	if (project.cont_int >= 10.0)
	    imax = (int) (100 * 400.0);
	else
	    imax = (int) (100 * 50.0);
	XtVaSetValues(scale_controls_contourinterval, 
			XmNminimum, 1, 
			XmNmaximum, imax, 
			XmNdecimalPoints, 2, 
			XmNvalue, ivalue, 
			NULL);

	/* set values of color interval slider */
	ivalue = (int) (100 * project.col_int);
	if (project.col_int >= 10.0)
	    imax = (int) (100 * 400.0);
	else
	    imax = (int) (100 * 50.0);
	XtVaSetValues(scale_controls_colorinterval, 
			XmNminimum, 1, 
			XmNmaximum, imax, 
			XmNdecimalPoints, 2, 
			XmNvalue, ivalue, 
			NULL);

	/* set values of tick interval slider */
	ivalue = (int) (100 * project.tick_int);
	if (project.tick_int >= 10.0)
	    imax = (int) (100 * 400.0);
	else
	    imax = (int) (100 * 50.0);
	XtVaSetValues(scale_controls_tickinterval, 
			XmNminimum, 1, 
			XmNmaximum, imax, 
			XmNdecimalPoints, 2, 
			XmNvalue, ivalue, 
			NULL);

	/* set values of inversion precsion slider */
	ivalue = (int) (100 * project.precision);
	XtVaSetValues(scale_controls_precision, 
			XmNdecimalPoints, 2, 
			XmNvalue, ivalue, 
			NULL);

	/* set values of z offset width slider */
	ivalue = (int) (project.zoffsetwidth);
	XtVaSetValues(scale_controls_zoffset, 
			XmNvalue, ivalue, 
			NULL);

	/* set misfit offset center toggles */
	if (mbna_misfit_center == MBNA_MISFIT_ZEROCENTER)
	    {
	    XmToggleButtonSetState(toggleButton_misfitcenter_zero, 
			TRUE, TRUE);
	    }
	else
	    {
	    XmToggleButtonSetState(toggleButton_misfitcenter_auto, 
			TRUE, TRUE);
	    }
}
/*--------------------------------------------------------------------*/

void
do_naverr_init()
{    
    /* manage naverr */
    XtManageChild(bulletinBoard_naverr);
    
    /* Setup just the "canvas" part of the screen. */
    cont_xid = XtWindow(drawingArea_naverr_cont);
    corr_xid = XtWindow(drawingArea_naverr_corr);
    zoff_xid = XtWindow(drawingArea_naverr_zcorr);

    /* Setup the "graphics Context" for just the "canvas" */
    xgcv.background = WhitePixelOfScreen(DefaultScreenOfDisplay(display));
    xgcv.foreground = BlackPixelOfScreen(DefaultScreenOfDisplay(display));
    xgcv.line_width = 2;
    cont_gc = XCreateGC(display,cont_xid,GCBackground | GCForeground
	     | GCLineWidth, &xgcv);
    corr_gc = XCreateGC(display,corr_xid,GCBackground | GCForeground
	     | GCLineWidth, &xgcv);

    /* Setup the font for just the "canvas" screen. */
    fontStruct = XLoadQueryFont(display, xgfont);
    XSetFont(display,cont_gc,fontStruct->fid);
    XSetFont(display,corr_gc,fontStruct->fid);

    XSelectInput(display, cont_xid, EV_MASK );
    XSelectInput(display, corr_xid, EV_MASK );

    /* Setup cursors. */
    myCursor = XCreateFontCursor(display, XC_target);
    XRecolorCursor(display,myCursor,&colors[2],&colors[5]);
    XDefineCursor(display,cont_xid,myCursor);
    XDefineCursor(display,corr_xid,myCursor);

    /* initialize graphics */
    cont_xgid = xg_init(display, cont_xid, cont_borders, xgfont);
    corr_xgid = xg_init(display, corr_xid, corr_borders, xgfont);
    zoff_xgid = xg_init(display, zoff_xid, zoff_borders, xgfont);
    status = mbnavadjust_set_graphics(cont_xgid, corr_xgid, zoff_xgid);
		
    /* set status flag */
    mbna_status = MBNA_STATUS_NAVERR;

    /* get current crossing */
    if (mbna_crossing_select != MBNA_SELECT_NONE)
   	mbna_current_crossing = mbna_crossing_select;
    if (mbna_current_crossing == -1)
    	mbnavadjust_naverr_nextunset();
    else
    	mbnavadjust_naverr_specific(mbna_crossing_select, mbna_tie_select);
		
    /* update naverr labels */
    mbnavadjust_naverr_plot(MBNA_PLOT_MODE_FIRST);
    do_update_naverr();
    if (project.modelplot == MB_YES)
	mbnavadjust_modelplot_plot();
    do_update_status();
}

/*--------------------------------------------------------------------*/

void
do_update_naverr()
{
    struct mbna_crossing *crossing;
    struct mbna_tie *tie;
    double	plot_width, misfit_width;
    double	zoom_factor;
    double	timediff;

    if (mbna_current_crossing >= 0)
    	{
    	/* get zoom factor */
    	if ((mbna_plot_lon_max - mbna_plot_lon_min) > 0.0)
    		zoom_factor = 100.0 *
    			MAX((mbna_lon_max - mbna_lon_min)
    				/ (mbna_plot_lon_max - mbna_plot_lon_min),
    			    (mbna_lat_max - mbna_lat_min)
    				/ (mbna_plot_lat_max - mbna_plot_lat_min));
    	else
    		zoom_factor = 0.0;
    	plot_width = (mbna_plot_lon_max - mbna_plot_lon_min)
    			/ mbna_mtodeglon;
    	misfit_width = (mbna_plot_lon_max - mbna_plot_lon_min)
    			/ mbna_mtodeglon;
			
	/* get time difference */
    	timediff = (project.files[mbna_file_id_1].sections[mbna_section_1].btime_d 
		- project.files[mbna_file_id_2].sections[mbna_section_2].btime_d) / 86400.0;

    	/* set main naverr status label */
	crossing = &project.crossings[mbna_current_crossing];
	tie = &crossing->ties[mbna_current_tie];
	if (crossing->status == MBNA_CROSSING_STATUS_NONE)
		{
        	sprintf(string,":::t\"Crossing: %d of %d\"\
:t\"Sections: %4.4d:%4.4d and %4.4d:%4.4d\"\
:t\"Time Difference: %f days \"\
:t\"Status: Unset \"\
:t\"Plot Widths (m): Contour: %.2f Misfit: %.2f Z: %.2f\"\
:t\"Zoom Factor: %.2f \"\
:t\"Relative Offsets:   None None None\"",
               	 	mbna_current_crossing, project.num_crossings,
                	crossing->file_id_1,crossing->section_1,
                	crossing->file_id_2,crossing->section_2,
			timediff,
               	 	plot_width, misfit_width, project.zoffsetwidth, zoom_factor);
                }
	else if (crossing->status == MBNA_CROSSING_STATUS_SET)
		{
        	sprintf(string,":::t\"Crossing: %d of %d\"\
:t\"Sections: %4.4d:%4.4d and %4.4d:%4.4d\"\
:t\"Time Difference: %f days \"\
:t\"Current Tie Point: %2d of %2d  Nav Points: %4d %4d\"\
:t\"Plot Widths (m): Contour: %.2f Misfit: %.2f Z: %.2f\"\
:t\"Zoom Factor: %.2f \"\
:t\"Relative Offsets (m):   %.3f %.3f %.3f\"\
:t\"Sigma (m):   %.3f %.3f %.3f\"",
               	 	mbna_current_crossing, project.num_crossings,
                	crossing->file_id_1,crossing->section_1,
                	crossing->file_id_2,crossing->section_2,
			timediff,
                	mbna_current_tie, crossing->num_ties,
                	tie->snav_1,
			tie->snav_2,
               	 	plot_width, misfit_width, project.zoffsetwidth, zoom_factor,
                	tie->offset_x_m,
			tie->offset_y_m,
			tie->offset_z_m,
			tie->sigmar1,
			tie->sigmar2,
			tie->sigmar3);
                }
	else
		{
        	sprintf(string,":::t\"Crossing: %d of %d\"\
:t\"Sections: %4.4d:%4.4d and %4.4d:%4.4d\"\
:t\"Time Difference: %f days \"\
:t\"Status: Skipped \"\
:t\"Plot Widths (m): Contour: %.2f Misfit: %.2f Z: %.2f\"\
:t\"Zoom Factor: %.2f \"\
:t\"Relative Offsets:   Skipped Skipped Skipped\"",
               	 	mbna_current_crossing, project.num_crossings,
                 	crossing->file_id_1,crossing->section_1,
                	crossing->file_id_2,crossing->section_2,
			timediff,
              	 	plot_width, misfit_width, project.zoffsetwidth, zoom_factor);
                }
	set_label_multiline_string(label_naverr_status, string);

	/* set some button sensitivities */
	XtVaSetValues(pushButton_naverr_deletetie,
		XmNsensitive, (mbna_current_tie >= 0),
		NULL);
	XtVaSetValues(pushButton_naverr_selecttie,
		XmNsensitive, (crossing->num_ties > 0),
		NULL);
	XtVaSetValues(pushButton_naverr_fullsize,
		XmNsensitive, (mbna_plot_lon_min != mbna_lon_min
				|| mbna_plot_lon_max != mbna_lon_max
				|| mbna_plot_lat_min != mbna_lat_min
				|| mbna_plot_lat_max != mbna_lat_max),
		NULL);
	if (mbna_view_list == MBNA_VIEW_LIST_FILES)
		{
		XtVaSetValues(pushButton_naverr_previous,
			XmNsensitive, False,
			NULL);
		XtVaSetValues(pushButton_naverr_next,
			XmNsensitive, False,
			NULL);
		XtVaSetValues(pushButton_naverr_nextunset,
			XmNsensitive, False,
			NULL);
		}
	else if (mbna_view_list == MBNA_VIEW_LIST_CROSSINGS)
		{
		XtVaSetValues(pushButton_naverr_previous,
			XmNsensitive, True,
			NULL);
		XtVaSetValues(pushButton_naverr_next,
			XmNsensitive, True,
			NULL);
		if (project.num_crossings == project.num_crossings_analyzed)
			XtVaSetValues(pushButton_naverr_nextunset,
				XmNsensitive, False,
				NULL);
		else
			XtVaSetValues(pushButton_naverr_nextunset,
				XmNsensitive, True,
				NULL);
		}
	else if (mbna_view_list == MBNA_VIEW_LIST_GOODCROSSINGS)
		{
		XtVaSetValues(pushButton_naverr_previous,
			XmNsensitive, True,
			NULL);
		XtVaSetValues(pushButton_naverr_next,
			XmNsensitive, True,
			NULL);
		if (project.num_crossings == project.num_crossings_analyzed)
			XtVaSetValues(pushButton_naverr_nextunset,
				XmNsensitive, False,
				NULL);
		else
			XtVaSetValues(pushButton_naverr_nextunset,
				XmNsensitive, True,
				NULL);
		}
	else if (mbna_view_list == MBNA_VIEW_LIST_TRUECROSSINGS)
		{
		XtVaSetValues(pushButton_naverr_previous,
			XmNsensitive, True,
			NULL);
		XtVaSetValues(pushButton_naverr_next,
			XmNsensitive, True,
			NULL);
		if (project.num_truecrossings == project.num_truecrossings_analyzed)
			XtVaSetValues(pushButton_naverr_nextunset,
				XmNsensitive, False,
				NULL);
		else
			XtVaSetValues(pushButton_naverr_nextunset,
				XmNsensitive, True,
				NULL);
		}
	else if (mbna_view_list == MBNA_VIEW_LIST_TIES)
		{
		XtVaSetValues(pushButton_naverr_previous,
			XmNsensitive, True,
			NULL);
		XtVaSetValues(pushButton_naverr_next,
			XmNsensitive, True,
			NULL);
		XtVaSetValues(pushButton_naverr_nextunset,
			XmNsensitive, True,
			NULL);
		}
    	
    	do_naverr_offsetlabel();
    	}
}

/*--------------------------------------------------------------------*/

void
do_naverr_offsetlabel()
{
    /* look at current crossing */
    if (mbna_current_crossing >= 0)
    	{
    	/* set main naverr status label */
        sprintf(string,":::t\"Working Offsets (m): %.3f %.3f %.3f %d:%d\":t\"Sigma (m): %.3f %.3f %.3f\"",
        		mbna_offset_x / mbna_mtodeglon,
			mbna_offset_y / mbna_mtodeglat,
			mbna_offset_z,
			mbna_snav_1, mbna_snav_2,
			mbna_minmisfit_sr1,mbna_minmisfit_sr2,mbna_minmisfit_sr3);
    	}

    else
    	{
    	/* set main naverr status label */
        sprintf(string,":::t\"Working Offsets (m): %.3f %.3f %.3f\":t\"Working Tie Points: %d:%d\"",
        		0.0, 0.0, 0.0, 0, 0);
    	}
    set_label_multiline_string(label_naverr_offsets, string);

    /* set button sensitivity for setting or resetting offsets */
    XtVaSetValues(pushButton_naverr_settie,
	    XmNsensitive, mbna_allow_set_tie,
	    NULL);
    XtVaSetValues(pushButton_naverr_resettie,
	    XmNsensitive, mbna_allow_set_tie,
	    NULL);

}

/*--------------------------------------------------------------------*/

void
do_naverr_test_graphics()
{
    int	    	i, j, k;
    int		ox, oy, dx, dy;
    double	rx, ry, rr, r;

    /* now test graphics */
    ox = 0;
    oy = 0;
    dx = (cont_borders[1] - cont_borders[0]) / 16;
    dy = (cont_borders[3] - cont_borders[2]) / 16;
    rx = cont_borders[1] - ox;
    ry = cont_borders[3] - oy;
    rr = sqrt(rx * rx + ry * ry);
    for (i=0;i<16;i++)
    	for (j=0;j<16;j++)
    	    {
    	    k = 16 * j + i;
    	    ox = i * dx;
    	    oy = j * dy;
     	    xg_fillrectangle(cont_xgid,ox,oy,dx,dy,mpixel_values[k],0);
    	    xg_fillrectangle(cont_xgid,ox+dx/4,oy+dy/4,dx/2,dy/2,k,0);
    	    }
    ox = (corr_borders[1] - corr_borders[0]) / 2;
    oy = (corr_borders[3] - corr_borders[2]) / 2;
    rx = corr_borders[1] - ox;
    ry = corr_borders[3] - oy;
    rr = sqrt(rx * rx + ry * ry);
    for (i=corr_borders[0];i<corr_borders[1];i++)
    	for (j=corr_borders[2];j<corr_borders[3];j++)
    	    {
    	    rx = i - ox;
    	    ry = j - oy;
    	    r = sqrt(rx * rx + ry * ry);
    	    k = 6 + (int) (80 * r / rr);
    	    xg_fillrectangle(corr_xgid,i,j,1,1,mpixel_values[k],0);
    	    }
}
/*--------------------------------------------------------------------*/

void
do_list_data_select( Widget w, XtPointer client_data, XtPointer call_data)
{
    XmAnyCallbackStruct *acs = (XmAnyCallbackStruct*)call_data;
    struct mbna_crossing *crossing;
    int	*position_list = NULL;
    int position_count = 0;
    int	i, j, k;

    if (XmListGetSelectedPos(list_data,&position_list,&position_count))
    	{
	if (mbna_view_list == MBNA_VIEW_LIST_FILES)
		{
		mbna_file_select = position_list[0] - 1;
		mbna_crossing_select = MBNA_SELECT_NONE;
		mbna_tie_select = MBNA_SELECT_NONE;
		}
	else if (mbna_view_list == MBNA_VIEW_LIST_CROSSINGS)
		{
		mbna_file_select = MBNA_SELECT_NONE;
		mbna_crossing_select = position_list[0] - 1;
		mbna_tie_select = MBNA_SELECT_NONE;
		
		/* bring up naverr window if required */
		if (mbna_naverr_load == MB_NO)
		    {
		    do_naverr_init();
		    }
		    
		/* else if naverr window is up, load selected crossing */
		else
		    {
		    mbnavadjust_naverr_specific(mbna_crossing_select, mbna_tie_select);
		    mbnavadjust_naverr_plot(MBNA_PLOT_MODE_FIRST);
		    do_update_naverr();
		    if (project.modelplot == MB_YES)
			    mbnavadjust_modelplot_plot();
		    do_update_status();
		    }
		}
	else if (mbna_view_list == MBNA_VIEW_LIST_GOODCROSSINGS)
		{
		mbna_file_select = MBNA_SELECT_NONE;
		mbna_crossing_select = MBNA_SELECT_NONE;
		mbna_tie_select = MBNA_SELECT_NONE;
		
		/* get crossing and tie from list */
		k = 0;
		for (i=0;i<project.num_crossings && mbna_crossing_select == MBNA_SELECT_NONE;i++)
			{
			crossing = &(project.crossings[i]);
			if (crossing->overlap >= MBNA_OVERLAP_THRESHOLD)
			    {
			    if (k == position_list[0] - 1)
				{
				mbna_crossing_select = i;
				}
			    k++;
			    }
			}
		
		/* bring up naverr window if required */
		if (mbna_naverr_load == MB_NO)
		    {
		    do_naverr_init();
		    }
		    
		/* else if naverr window is up, load selected crossing */
		else
		    {
		    mbnavadjust_naverr_specific(mbna_crossing_select, mbna_tie_select);
		    mbnavadjust_naverr_plot(MBNA_PLOT_MODE_FIRST);
		    do_update_naverr();
		    if (project.modelplot == MB_YES)
			    mbnavadjust_modelplot_plot();
		    do_update_status();
		    }
		}
	else if (mbna_view_list == MBNA_VIEW_LIST_TRUECROSSINGS)
		{
		mbna_file_select = MBNA_SELECT_NONE;
		mbna_crossing_select = MBNA_SELECT_NONE;
		mbna_tie_select = MBNA_SELECT_NONE;
		
		/* get crossing and tie from list */
		k = 0;
		for (i=0;i<project.num_crossings && mbna_crossing_select == MBNA_SELECT_NONE;i++)
			{
			crossing = &(project.crossings[i]);
			if (crossing->truecrossing == MB_YES)
			    {
			    if (k == position_list[0] - 1)
				{
				mbna_crossing_select = i;
				}
			    k++;
			    }
			}
		
		/* bring up naverr window if required */
		if (mbna_naverr_load == MB_NO)
		    {
		    do_naverr_init();
		    }
		    
		/* else if naverr window is up, load selected crossing */
		else
		    {
		    mbnavadjust_naverr_specific(mbna_crossing_select, mbna_tie_select);
		    mbnavadjust_naverr_plot(MBNA_PLOT_MODE_FIRST);
		    do_update_naverr();
		    if (project.modelplot == MB_YES)
			    mbnavadjust_modelplot_plot();
		    do_update_status();
		    }
		}
	else if (mbna_view_list == MBNA_VIEW_LIST_TIES)
		{
		mbna_file_select = MBNA_SELECT_NONE;
		
		/* get crossing and tie from list */
		k = 0;
		for (i=0;i<project.num_crossings;i++)
		    for (j=0;j<project.crossings[i].num_ties;j++)
			{
			if (k == position_list[0] - 1)
			    {
			    mbna_crossing_select = i;
			    mbna_tie_select = j;
			    }
			k++;
			}
		
		/* bring up naverr window if required */
		if (mbna_naverr_load == MB_NO)
		    {
		    do_naverr_init();
		    }
		    
		/* else if naverr window is up, load selected crossing */
		else
		    {
		    mbnavadjust_naverr_specific(mbna_crossing_select, mbna_tie_select);
		    mbnavadjust_naverr_plot(MBNA_PLOT_MODE_FIRST);
		    do_update_naverr();
		    if (project.modelplot == MB_YES)
			    mbnavadjust_modelplot_plot();
		    do_update_status();
		    }
		}
     	free(position_list);
      	}
    else
    	{
	mbna_file_select = MBNA_SELECT_NONE;
	mbna_crossing_select = MBNA_SELECT_NONE;
    	}
    do_update_status();

}

/*--------------------------------------------------------------------*/

void
do_naverr_cont_expose( Widget w, XtPointer client_data, XtPointer call_data)
{
    XmAnyCallbackStruct *acs = (XmAnyCallbackStruct*)call_data;

    mbnavadjust_naverr_plot(MBNA_PLOT_MODE_FIRST);
}

/*--------------------------------------------------------------------*/

void
do_naverr_corr_expose( Widget w, XtPointer client_data, XtPointer call_data)
{
    XmAnyCallbackStruct *acs = (XmAnyCallbackStruct*)call_data;

    mbnavadjust_naverr_plot(MBNA_PLOT_MODE_FIRST);
}

/*--------------------------------------------------------------------*/

void
do_naverr_cont_input( Widget w, XtPointer client_data, XtPointer call_data)
{
    XmAnyCallbackStruct *acs = (XmAnyCallbackStruct*)call_data;
    XEvent  *event = acs->event;
    double	x1, x2, y1, y2;
    
    /* If there is input in the drawing area */
    if (acs->reason == XmCR_INPUT)
    {
      /* Check for mouse pressed. */
      if (event->xany.type == ButtonPress)
      {
	  /* If left mouse button is pushed then save position. */
	  if (event->xbutton.button == 1)
		    {
		    button1down = MB_YES;
		    loc_x = event->xbutton.x;
		    loc_y = event->xbutton.y;
		    mbna_offset_x_old = mbna_offset_x;
		    mbna_offset_y_old = mbna_offset_y;
		    mbna_offset_z_old = mbna_offset_z;
		
		    /* reset offset label */
		    mbnavadjust_naverr_checkoksettie();
		    do_naverr_offsetlabel();

		    } /* end of left button events */

	    /* If middle mouse button is pushed */
	    if (event->xbutton.button == 2)
		    {
		    button2down = MB_YES;
		    mbna_zoom_x1 = event->xbutton.x;
		    mbna_zoom_y1 = event->xbutton.y;
		    mbna_zoom_x2 = event->xbutton.x;
		    mbna_zoom_y2 = event->xbutton.y;
	    	
		    /* replot contours */
		    mbnavadjust_naverr_plot(MBNA_PLOT_MODE_ZOOMFIRST);
		    } /* end of middle button events */

	    /* If right mouse button is pushed */
	    if (event->xbutton.button == 3)
		    {
		    button3down = MB_YES;
		
		    /* get new snav points */
		    mbnavadjust_naverr_snavpoints(event->xbutton.x, event->xbutton.y);
	    	
		    /* replot contours */
		    mbnavadjust_naverr_plot(MBNA_PLOT_MODE_FIRST);
   		    do_update_naverr();
 		    } /* end of right button events */	
      } /* end of button press events */

      /* Check for mouse released. */
      if (event->xany.type == ButtonRelease)
      {
	  if (event->xbutton.button == 1)
	    	{
	    	button1down = MB_NO;
	    	}
	  if (event->xbutton.button == 2)
	    	{
	    	button2down = MB_NO;
	    	mbna_zoom_x2 = event->xbutton.x;
	    	mbna_zoom_y2 = event->xbutton.y;
	    	
	    	x1 = mbna_zoom_x1 / mbna_plotx_scale +  mbna_plot_lon_min;
	    	y1 = (cont_borders[3] - mbna_zoom_y1) / mbna_ploty_scale +  mbna_plot_lat_min;
	    	x2 = mbna_zoom_x2 / mbna_plotx_scale +  mbna_plot_lon_min;
	    	y2 = (cont_borders[3] - mbna_zoom_y2) / mbna_ploty_scale +  mbna_plot_lat_min;
	    	
	    	/* get new plot bounds */
	    	mbna_plot_lon_min = MIN(x1,x2);
	    	mbna_plot_lon_max = MAX(x1,x2);
	    	mbna_plot_lat_min = MIN(y1,y2);
	    	mbna_plot_lat_max = MAX(y1,y2);
	    	
		/* replot contours and misfit */
		mbnavadjust_get_misfit();
		mbnavadjust_naverr_plot(MBNA_PLOT_MODE_FIRST);
		do_update_naverr();	    	
		}
	  if (event->xbutton.button == 3)
	    	{
	    	button3down = MB_NO;
	    	}

      } /* end of button release events */

      /* Check for mouse motion while pressed. */
      if (event->xany.type == MotionNotify)
      {
	  if (button1down == MB_YES)
		{
		/* move offset */
		mbna_offset_x = mbna_offset_x_old 
		    + (event->xmotion.x - loc_x) / mbna_plotx_scale;
		mbna_offset_y = mbna_offset_y_old 
		    - (event->xmotion.y - loc_y) / mbna_ploty_scale;
		
		/* replot contours */
		mbnavadjust_naverr_plot(MBNA_PLOT_MODE_MOVE);
		do_naverr_offsetlabel();
		
		/* reset old position */
		loc_x = event->xmotion.x;
		loc_y = event->xmotion.y;
		mbna_offset_x_old = mbna_offset_x;
		mbna_offset_y_old = mbna_offset_y;
		}
	  else if (button2down == MB_YES)
		{
	    	mbna_zoom_x2 = event->xmotion.x;
		mbna_zoom_y2 = event->xmotion.y;
		
		/* replot contours */
		mbnavadjust_naverr_plot(MBNA_PLOT_MODE_ZOOM);
	    	}
      }
    } /* end of inputs from window */

}

/*--------------------------------------------------------------------*/

void
do_naverr_corr_input( Widget w, XtPointer client_data, XtPointer call_data)
{
	XEvent *event;
    XmAnyCallbackStruct *acs = (XmAnyCallbackStruct*)call_data;
	event = acs->event;

    /* If there is input in the drawing area */
    if (acs->reason == XmCR_INPUT)
    {
      /* Check for mouse pressed. */
      if (event->xany.type == ButtonPress)
      {
	  /* If left mouse button is pushed then save position. */
	  if (event->xbutton.button == 1)
		    {
		    button1down = MB_YES;
		    mbna_offset_x_old = mbna_offset_x;
		    mbna_offset_y_old = mbna_offset_y;
		    mbna_offset_z_old = mbna_offset_z;
		    mbna_offset_x = mbna_misfit_offset_x 
				    + (event->xbutton.x
		    			- (corr_borders[0] + corr_borders[1]) / 2)
		    			/ mbna_misfit_scale;
		    mbna_offset_y = mbna_misfit_offset_y
				    - (event->xbutton.y
		    			- (corr_borders[3] + corr_borders[2]) / 2)
		    			/ mbna_misfit_scale;
	    	
		    /* replot contours */
		    mbnavadjust_naverr_plot(MBNA_PLOT_MODE_MOVE);
   		    do_update_naverr();
		    do_naverr_offsetlabel();

		    } /* end of left button events */
      } /* end of button press events */

      /* Check for mouse released. */
      if (event->xany.type == ButtonRelease)
      {
	  if (event->xbutton.button == 1)
	    	{
	    	button1down = MB_NO;
	    	}
      } /* end of button release events */

      /* Check for mouse motion while pressed. */
      if (event->xany.type == MotionNotify)
      {
	  if (button1down == MB_YES)
		{
		/* move offset */
		mbna_offset_x = mbna_misfit_offset_x 
				+ (event->xmotion.x
		    			- (corr_borders[0] + corr_borders[1]) / 2)
		    			/ mbna_misfit_scale;
		mbna_offset_y = mbna_misfit_offset_y
				- (event->xmotion.y
		    			- (corr_borders[3] + corr_borders[2]) / 2)
		    			/ mbna_misfit_scale;
		
		/* replot contours */
		mbnavadjust_naverr_plot(MBNA_PLOT_MODE_MOVE);
   		do_update_naverr();
		do_naverr_offsetlabel();
		
		/* reset old position */
		mbna_offset_x_old = mbna_offset_x;
		mbna_offset_y_old = mbna_offset_y;
		mbna_offset_z_old = mbna_offset_z;
		}
      }
    } /* end of inputs from window */
}
/*--------------------------------------------------------------------*/

void
do_naverr_zcorr_input( Widget w, XtPointer client_data, XtPointer call_data)
{
    XEvent *event;
    XmAnyCallbackStruct *acs = (XmAnyCallbackStruct*)call_data;
    event = acs->event;

    /* If there is input in the drawing area */
    if (acs->reason == XmCR_INPUT)
    {
      /* Check for mouse pressed. */
      if (event->xany.type == ButtonPress)
      {
	  /* If left mouse button is pushed then save position. */
	  if (event->xbutton.button == 1)
		{
		button1down = MB_YES;
		mbna_offset_x_old = mbna_offset_x;
		mbna_offset_y_old = mbna_offset_y;
		mbna_offset_z_old = mbna_offset_z;
		mbna_offset_z = ((event->xbutton.x - zoff_borders[0]) 
		    			/ mbna_zoff_scale_x) 
					+ mbna_misfit_offset_z - 0.5 * project.zoffsetwidth;

		/* replot contours */
		mbnavadjust_naverr_plot(MBNA_PLOT_MODE_MOVE);
   		do_update_naverr();
		do_naverr_offsetlabel();
		} /* end of left button events */
      } /* end of button press events */

      /* Check for mouse released. */
      if (event->xany.type == ButtonRelease)
      {
	  if (event->xbutton.button == 1)
	    	{
		button1down = MB_NO;
		mbnavadjust_crossing_replot();
		mbnavadjust_naverr_plot(MBNA_PLOT_MODE_FIRST);
   		do_update_naverr();
		do_naverr_offsetlabel();
	    	}
      } /* end of button release events */

      /* Check for mouse motion while pressed. */
      if (event->xany.type == MotionNotify)
      {
	  if (button1down == MB_YES)
		{
		/* move offset */
		mbna_offset_z = ((event->xbutton.x - zoff_borders[0]) 
		    			/ mbna_zoff_scale_x) 
					+ mbna_misfit_offset_z - 0.5 * project.zoffsetwidth;
fprintf(stderr,"buttonx:%d %f  mbna_misfit_offset_z:%f project.zoffsetwidth:%f  mbna_offset_z:%f\n",
event->xbutton.x,((event->xbutton.x - zoff_borders[0])/mbna_zoff_scale_x), mbna_misfit_offset_z,
project.zoffsetwidth, mbna_offset_z);
		
		/* replot contours */
		mbnavadjust_naverr_plot(MBNA_PLOT_MODE_MOVE);
   		do_update_naverr();
		do_naverr_offsetlabel();
		
		/* reset old position */
		mbna_offset_x_old = mbna_offset_x;
		mbna_offset_y_old = mbna_offset_y;
		mbna_offset_z_old = mbna_offset_z;
		}
      }
    } /* end of inputs from window */
}

/*--------------------------------------------------------------------*/

void
do_naverr_previous( Widget w, XtPointer client_data, XtPointer call_data)
{
    XmAnyCallbackStruct *acs = (XmAnyCallbackStruct*)call_data;

    mbnavadjust_naverr_previous();
    mbnavadjust_naverr_plot(MBNA_PLOT_MODE_FIRST);
    do_update_naverr();
    if (project.modelplot == MB_YES)
	mbnavadjust_modelplot_plot();
    do_update_status();
}

/*--------------------------------------------------------------------*/

void
do_naverr_next( Widget w, XtPointer client_data, XtPointer call_data)
{
    XmAnyCallbackStruct *acs = (XmAnyCallbackStruct*)call_data;

    mbnavadjust_naverr_next();
    mbnavadjust_naverr_plot(MBNA_PLOT_MODE_FIRST);
    do_update_naverr();
    if (project.modelplot == MB_YES)
	mbnavadjust_modelplot_plot();
    do_update_status();
}

/*--------------------------------------------------------------------*/

void
do_naverr_nextunset( Widget w, XtPointer client_data, XtPointer call_data)
{
    XmAnyCallbackStruct *acs = (XmAnyCallbackStruct*)call_data;

    mbnavadjust_naverr_nextunset();
    mbnavadjust_naverr_plot(MBNA_PLOT_MODE_FIRST);
    do_update_naverr();
    if (project.modelplot == MB_YES)
	mbnavadjust_modelplot_plot();
    do_update_status();
}

/*--------------------------------------------------------------------*/

void
do_naverr_addtie( Widget w, XtPointer client_data, XtPointer call_data)
{
    XmAnyCallbackStruct *acs = (XmAnyCallbackStruct*)call_data;

    mbnavadjust_naverr_addtie();
    mbnavadjust_naverr_plot(MBNA_PLOT_MODE_FIRST);
    do_update_naverr();
    if (project.modelplot == MB_YES)
	mbnavadjust_modelplot_plot();
    do_update_status();
}

/*--------------------------------------------------------------------*/

void
do_naverr_deletetie( Widget w, XtPointer client_data, XtPointer call_data)
{
    XmAnyCallbackStruct *acs = (XmAnyCallbackStruct*)call_data;

    mbnavadjust_naverr_deletetie();
    mbnavadjust_naverr_plot(MBNA_PLOT_MODE_FIRST);
    do_update_naverr();
    if (project.modelplot == MB_YES)
	mbnavadjust_modelplot_plot();
    do_update_status();
}

/*--------------------------------------------------------------------*/

void
do_naverr_selecttie( Widget w, XtPointer client_data, XtPointer call_data)
{
    XmAnyCallbackStruct *acs = (XmAnyCallbackStruct*)call_data;

    mbnavadjust_naverr_selecttie();
    mbnavadjust_naverr_plot(MBNA_PLOT_MODE_FIRST);
    do_update_naverr();
    if (project.modelplot == MB_YES)
	mbnavadjust_modelplot_plot();
    do_update_status();
}

/*--------------------------------------------------------------------*/

void
do_naverr_setnone( Widget w, XtPointer client_data, XtPointer call_data)
{
    XmAnyCallbackStruct *acs = (XmAnyCallbackStruct*)call_data;

    mbnavadjust_naverr_skip();
    mbnavadjust_naverr_nextunset();
    mbnavadjust_naverr_plot(MBNA_PLOT_MODE_FIRST);
    do_update_naverr();
    if (project.modelplot == MB_YES)
	mbnavadjust_modelplot_plot();
    do_update_status();
}

/*--------------------------------------------------------------------*/

void
do_naverr_setoffset( Widget w, XtPointer client_data, XtPointer call_data)
{
    XmAnyCallbackStruct *acs = (XmAnyCallbackStruct*)call_data;

    mbnavadjust_naverr_save();
    mbnavadjust_naverr_plot(MBNA_PLOT_MODE_FIRST);
    do_update_naverr();
    if (project.modelplot == MB_YES)
	mbnavadjust_modelplot_plot();
    do_update_status();
}

/*--------------------------------------------------------------------*/

void
do_naverr_resettie( Widget w, XtPointer client_data, XtPointer call_data)
{
    XmAnyCallbackStruct *acs = (XmAnyCallbackStruct*)call_data;

    mbnavadjust_naverr_resettie();
    mbnavadjust_naverr_plot(MBNA_PLOT_MODE_FIRST);
    do_update_naverr();
    if (project.modelplot == MB_YES)
	mbnavadjust_modelplot_plot();
    do_update_status();
}

/*--------------------------------------------------------------------*/

void
do_dismiss_naverr( Widget w, XtPointer client_data, XtPointer call_data)
{
    XmAnyCallbackStruct *acs = (XmAnyCallbackStruct*)call_data;
		
    /* unload loaded crossing */
    if (mbna_naverr_load == MB_YES)
	    {
	    status = mbnavadjust_crossing_unload();
	    }

    /* deallocate graphics */
    mbna_status = MBNA_STATUS_GUI;
    XFreeGC(display,cont_gc);
    XFreeGC(display,corr_gc);
    xg_free(cont_xgid);
    xg_free(corr_xgid);
    mbnavadjust_naverr_checkoksettie();
    do_update_naverr();
    if (project.modelplot == MB_YES)
	mbnavadjust_modelplot_plot();
    do_update_status();
}

/*--------------------------------------------------------------------*/
void
do_naverr_fullsize( Widget w, XtPointer client_data, XtPointer call_data)
{
    XmAnyCallbackStruct *acs = (XmAnyCallbackStruct*)call_data;
    
    /* reset the plot bounds */
    mbna_plot_lon_min = mbna_lon_min;
    mbna_plot_lon_max = mbna_lon_max;
    mbna_plot_lat_min = mbna_lat_min;
    mbna_plot_lat_max = mbna_lat_max;
    mbnavadjust_get_misfit();
    mbnavadjust_naverr_plot(MBNA_PLOT_MODE_FIRST);
    do_update_naverr();
}

/*--------------------------------------------------------------------*/

void
do_naverr_zerooffset( Widget w, XtPointer client_data, XtPointer call_data)
{
    XmAnyCallbackStruct *acs = (XmAnyCallbackStruct*)call_data;
		
	/* move offset */
	mbna_offset_x = 0.0;
	mbna_offset_y = 0.0;
	mbna_offset_z = 0.0;
		
	/* replot contours */
	mbnavadjust_crossing_replot();
	mbnavadjust_naverr_plot(MBNA_PLOT_MODE_FIRST);
	do_naverr_offsetlabel();
}
/*--------------------------------------------------------------------*/

void
do_naverr_zerozoffset( Widget w, XtPointer client_data, XtPointer call_data)
{
    XmAnyCallbackStruct *acs = (XmAnyCallbackStruct*)call_data;
		
	/* move offset */
	mbna_offset_z = 0.0;
		
	/* replot contours */
	mbnavadjust_crossing_replot();
	mbnavadjust_naverr_plot(MBNA_PLOT_MODE_FIRST);
	do_naverr_offsetlabel();
}
/*--------------------------------------------------------------------*/

void
do_naverr_applyzoffset( Widget w, XtPointer client_data, XtPointer call_data)
{
    XmAnyCallbackStruct *acs = (XmAnyCallbackStruct*)call_data;
		
	/* replot contours */
	mbnavadjust_naverr_plot(MBNA_PLOT_MODE_FIRST);
	do_naverr_offsetlabel();
	if (project.modelplot == MB_YES)
		mbnavadjust_modelplot_plot();
}

/*--------------------------------------------------------------------*/

void
do_naverr_minmisfit( Widget w, XtPointer client_data, XtPointer call_data)
{
    XmAnyCallbackStruct *acs = (XmAnyCallbackStruct*)call_data;

	/* move offset */
	mbna_offset_x = mbna_minmisfit_x;
	mbna_offset_y = mbna_minmisfit_y;
	mbna_offset_z = mbna_minmisfit_z;
		
	/* replot contours */
	mbnavadjust_crossing_replot();
   	mbnavadjust_naverr_plot(MBNA_PLOT_MODE_FIRST);
    	do_update_naverr();
	if (project.modelplot == MB_YES)
		mbnavadjust_modelplot_plot();

}
/*--------------------------------------------------------------------*/

void
do_naverr_misfitcenter( Widget w, XtPointer client_data, XtPointer call_data)
{
    XmAnyCallbackStruct *acs = (XmAnyCallbackStruct*)call_data;

	if (XmToggleButtonGetState(toggleButton_misfitcenter_zero))
	    mbna_misfit_center = MBNA_MISFIT_ZEROCENTER;
	else
	    mbna_misfit_center = MBNA_MISFIT_AUTOCENTER;
		    
	/* replot contours and misfit */
	mbnavadjust_get_misfit();
	mbnavadjust_naverr_plot(MBNA_PLOT_MODE_FIRST);
	do_update_naverr();	    	
}

/*--------------------------------------------------------------------*/

void
do_biases_apply( Widget w, XtPointer client_data, XtPointer call_data)
{
    XmAnyCallbackStruct *acs = (XmAnyCallbackStruct*)call_data;
    	struct mbna_file *file1;
    	struct mbna_file *file2;
	int	ivalue;
	int	isection;
	
	/* get file structures */
	file1 = &(project.files[mbna_file_id_1]);
	file2 = &(project.files[mbna_file_id_2]);

	XtVaGetValues(scale_biases_heading1, 
			XmNvalue, &ivalue, 
			NULL);
	file1->heading_bias = 0.1 * ivalue;

	XtVaGetValues(scale_biases_roll1, 
			XmNvalue, &ivalue, 
			NULL);
	file1->roll_bias = 0.1 * ivalue;

	XtVaGetValues(scale_biases_heading2, 
			XmNvalue, &ivalue, 
			NULL);
	file2->heading_bias = 0.1 * ivalue;

	XtVaGetValues(scale_biases_roll2, 
			XmNvalue, &ivalue, 
			NULL);
	file2->roll_bias = 0.1 * ivalue;

	/* set contours out of date */
	for (isection=0;isection<file1->num_sections;isection++)
		{
		file1->sections[isection].contoursuptodate = MB_NO;
		}
	for (isection=0;isection<file2->num_sections;isection++)
		{
		file2->sections[isection].contoursuptodate = MB_NO;
		}

	mbnavadjust_crossing_replot();
	mbnavadjust_get_misfit();
	mbnavadjust_naverr_plot(MBNA_PLOT_MODE_FIRST);
}

/*--------------------------------------------------------------------*/

void
do_biases_applyall( Widget w, XtPointer client_data, XtPointer call_data)
{
    XmAnyCallbackStruct *acs = (XmAnyCallbackStruct*)call_data;
	double	heading_bias;
	double	roll_bias;
    	struct mbna_file *file;
	int	ivalue;
	int	ifile, isection;
	
	/* get bias values */
	XtVaGetValues(scale_biases_heading1, 
			XmNvalue, &ivalue, 
			NULL);
	heading_bias = 0.1 * ivalue;
	XtVaGetValues(scale_biases_roll1, 
			XmNvalue, &ivalue, 
			NULL);
	roll_bias = 0.1 * ivalue;

	/* loop over files */
	for (ifile=0;ifile<project.num_files;ifile++)
		{
		file = &(project.files[ifile]);
		file->heading_bias = heading_bias;
		file->roll_bias = roll_bias;
				
		/* set contours out of date */
		for (isection=0;isection<file->num_sections;isection++)
			{
			file->sections[isection].contoursuptodate = MB_NO;
			}
		}

	mbnavadjust_crossing_replot();
	mbnavadjust_get_misfit();
	mbnavadjust_naverr_plot(MBNA_PLOT_MODE_FIRST);
}

/*--------------------------------------------------------------------*/

void
do_biases_init( Widget w, XtPointer client_data, XtPointer call_data)
{
    XmAnyCallbackStruct *acs = (XmAnyCallbackStruct*)call_data;
    	struct mbna_file *file1;
    	struct mbna_file *file2;
	char	value_text[128];
	
	/* get file structures */
	file1 = &(project.files[mbna_file_id_1]);
	file2 = &(project.files[mbna_file_id_2]);

	/* set biases label */
	sprintf(value_text, ":::t\"Section ID\'s (file:section):\":t\"  Section 1: %4.4d:%4.4d\"\"  Section 2: %4.4d:%4.4d\"",
		mbna_file_id_1, mbna_section_1, 
		mbna_file_id_2, mbna_section_2);
	set_label_multiline_string(label_biases_files, value_text);
	
	/* set biases radiobox */
	if (file1->heading_bias == file2->heading_bias
	    && file1->roll_bias == file2->roll_bias)
	    {
	    mbna_bias_mode = MBNA_BIAS_SAME;
	    XmToggleButtonSetState(toggleButton_biases_together, 
			TRUE, TRUE);
	    }
	else
	    {
	    mbna_bias_mode = MBNA_BIAS_DIFFERENT;
	    XmToggleButtonSetState(toggleButton_biases_separate, 
			TRUE, TRUE);
	    }

	/* set values of bias sliders */
	XtVaSetValues(scale_biases_heading1, 
			XmNvalue, (int) (10 * file1->heading_bias), 
			NULL);
	XtVaSetValues(scale_biases_roll1, 
			XmNvalue, (int) (10 * file1->roll_bias), 
			NULL);
	if (mbna_bias_mode == MBNA_BIAS_DIFFERENT)
		{
		XtVaSetValues(scale_biases_heading2, 
			XmNvalue, (int) (10 * file2->heading_bias), 
			XmNsensitive, True,
			NULL);
		XtVaSetValues(scale_biases_roll2, 
			XmNvalue, (int) (10 * file2->roll_bias), 
			XmNsensitive, True,
			NULL);
		XtVaSetValues(pushButton_biases_applyall, 
			XmNsensitive, False,
			NULL);
		}
	else
		{
		XtVaSetValues(scale_biases_heading2, 
			XmNvalue, (int) (10 * file2->heading_bias), 
			XmNsensitive, False,
			NULL);
		XtVaSetValues(scale_biases_roll2, 
			XmNvalue, (int) (10 * file2->roll_bias), 
			XmNsensitive, False,
			NULL);
		XtVaSetValues(pushButton_biases_applyall, 
			XmNsensitive, True,
			NULL);
		}
}

/*--------------------------------------------------------------------*/

void
do_biases_toggle( Widget w, XtPointer client_data, XtPointer call_data)
{
    XmAnyCallbackStruct *acs = (XmAnyCallbackStruct*)call_data;
	int	ivalue;

	if (XmToggleButtonGetState(toggleButton_biases_together))
	    {
	    if (mbna_bias_mode == MBNA_BIAS_DIFFERENT)
		{
		mbna_bias_mode = MBNA_BIAS_SAME;
		XtVaGetValues(scale_biases_heading1, 
				XmNvalue, &ivalue, 
				NULL);
		XtVaSetValues(scale_biases_heading2, 
				XmNvalue, ivalue, 
				XmNsensitive, False,
				NULL);
		XtVaGetValues(scale_biases_roll1, 
				XmNvalue, &ivalue, 
				NULL);
		XtVaSetValues(scale_biases_roll2, 
				XmNvalue, ivalue, 
				XmNsensitive, False,
				NULL);
		XtVaSetValues(pushButton_biases_applyall, 
				XmNsensitive, True,
				NULL);
		}
	    }
	else
	    {
	    if (mbna_bias_mode == MBNA_BIAS_SAME)
		{
		mbna_bias_mode = MBNA_BIAS_DIFFERENT;
		XtVaSetValues(scale_biases_heading2, 
				XmNsensitive, True,
				NULL);
		XtVaSetValues(scale_biases_roll2, 
				XmNsensitive, True,
				NULL);
		XtVaSetValues(pushButton_biases_applyall, 
				XmNsensitive, False,
				NULL);
		}
	    }

}

/*--------------------------------------------------------------------*/

void
do_biases_heading( Widget w, XtPointer client_data, XtPointer call_data)
{
    XmAnyCallbackStruct *acs = (XmAnyCallbackStruct*)call_data;
	int	ivalue;

	if (mbna_bias_mode == MBNA_BIAS_SAME)
	    {
	    XtVaGetValues(scale_biases_heading1, 
			    XmNvalue, &ivalue, 
			    NULL);
	    XtVaSetValues(scale_biases_heading2, 
			    XmNvalue, ivalue, 
			    XmNsensitive, False,
			    NULL);
	    }
}

/*--------------------------------------------------------------------*/

void
do_biases_roll( Widget w, XtPointer client_data, XtPointer call_data)
{
    XmAnyCallbackStruct *acs = (XmAnyCallbackStruct*)call_data;
	int	ivalue;

	if (mbna_bias_mode == MBNA_BIAS_SAME)
	    {
	    XtVaGetValues(scale_biases_roll1, 
			    XmNvalue, &ivalue, 
			    NULL);
	    XtVaSetValues(scale_biases_roll2, 
			    XmNvalue, ivalue, 
			    XmNsensitive, False,
			    NULL);
	    }
}

/*--------------------------------------------------------------------*/

void
do_controls_apply( Widget w, XtPointer client_data, XtPointer call_data)
{
    XmAnyCallbackStruct *acs = (XmAnyCallbackStruct*)call_data;
    int	ivalue;

    /* get value of decimation slider */
    XtVaGetValues(scale_controls_decimation, 
		XmNvalue, &project.decimation, 
		NULL);

    /* get values of section length slider */
    XtVaGetValues(scale_controls_sectionlength, 
		XmNvalue, &ivalue, 
		NULL);
    project.section_length = ((double) ivalue) / 100.0;

    /* get values of section soundings slider */
    XtVaGetValues(scale_controls_sectionsoundings, 
		XmNvalue, &ivalue, 
		NULL);
    project.section_soundings = ivalue;

    /* get values of contour interval slider */
    XtVaGetValues(scale_controls_contourinterval, 
		XmNvalue, &ivalue, 
		NULL);
    project.cont_int = ((double) ivalue) / 100.0;

    /* get values of color interval slider */
    XtVaGetValues(scale_controls_colorinterval, 
		XmNvalue, &ivalue, 
		NULL);
    project.col_int = ((double) ivalue) / 100.0;

    /* get values of tick interval slider */
    XtVaGetValues(scale_controls_tickinterval, 
		XmNvalue, &ivalue, 
		NULL);
    project.tick_int = ((double) ivalue) / 100.0;
 
    /* get values of inversion precision slider */
    XtVaGetValues(scale_controls_precision, 
		XmNvalue, &ivalue, 
		NULL);
    project.precision = ((double) ivalue) / 100.0;
 
    /* get values of z offset width slider */
    XtVaGetValues(scale_controls_zoffset, 
		XmNvalue, &ivalue, 
		NULL);
    project.zoffsetwidth = ((double) ivalue);
   
    if (mbna_file_id_1 >= 0 && mbna_section_1 >= 0)
   	 project.files[mbna_file_id_1].sections[mbna_section_1].contoursuptodate = MB_NO;
    if (mbna_file_id_2 >= 0 && mbna_section_2 >= 0)
   	 project.files[mbna_file_id_2].sections[mbna_section_2].contoursuptodate = MB_NO;

    mbnavadjust_crossing_replot();
    mbnavadjust_write_project();
    mbnavadjust_get_misfit();
    mbnavadjust_naverr_plot(MBNA_PLOT_MODE_FIRST);
    do_update_naverr();
    do_update_status();
}


/*--------------------------------------------------------------------*/
void
do_scale_controls_sectionlength( Widget w, XtPointer client_data, XtPointer call_data)
{
    XmAnyCallbackStruct *acs = (XmAnyCallbackStruct*)call_data;
 }

/*--------------------------------------------------------------------*/
void
do_scale_controls_sectionsoundings( Widget w, XtPointer client_data, XtPointer call_data)
{
    XmAnyCallbackStruct *acs = (XmAnyCallbackStruct*)call_data;
    int	    ivalue;
    int	    imin, imax;

	/* get values of section soundings slider */
	XtVaGetValues(scale_controls_sectionsoundings, 
		    XmNvalue, &ivalue, 
		    XmNminimum, &imin, 
		    XmNmaximum, &imax, 
		    NULL);
		    
	/* recalculate max value if needed */
	if (ivalue == imin)
	    {
	    imax = MAX(imax / 2, 2 * imin);
	    }
	if (ivalue == imax)
	    {
	    imax = 2 * imax;
	    }
    
	/* reset values of section soundings slider */
	XtVaSetValues(scale_controls_sectionsoundings, 
			XmNmaximum, imax, 
			XmNvalue, ivalue, 
			NULL);
}

/*--------------------------------------------------------------------*/
void
do_scale_controls_decimation( Widget w, XtPointer client_data, XtPointer call_data)
{
    XmAnyCallbackStruct *acs = (XmAnyCallbackStruct*)call_data;
}

/*--------------------------------------------------------------------*/
void
do_scale_contourinterval( Widget w, XtPointer client_data, XtPointer call_data)
{
    XmAnyCallbackStruct *acs = (XmAnyCallbackStruct*)call_data;
    int	    ivalue;
    int	    imax;

	/* get values of contour interval slider */
	XtVaGetValues(scale_controls_contourinterval, 
		    XmNvalue, &ivalue, 
		    XmNmaximum, &imax, 
		    NULL);
    
	/* reset values of contour interval slider to round numbers */
	if (ivalue > 2500)
	    ivalue = ((ivalue + 500) / 1000) * 1000;
	else if (ivalue > 500)
	    ivalue = ((ivalue + 250) / 500) * 500;
	else if (ivalue > 100)
	    ivalue = ((ivalue + 50) / 100) * 100;
	else if (ivalue > 50)
	    ivalue = ((ivalue + 25) / 50) * 50;
	else if (ivalue > 10)
	    ivalue = ((ivalue + 5) / 10) * 10;
	else if (ivalue > 5)
	    ivalue = ((ivalue + 2) / 5) * 5;
	if (ivalue == 1 && imax >= 40000)
	    imax = 500;
	if (ivalue == imax && imax <= 500)
	    imax = 40000;
	XtVaSetValues(scale_controls_contourinterval, 
			XmNminimum, 1, 
			XmNmaximum, imax, 
			XmNdecimalPoints, 2, 
			XmNvalue, ivalue, 
			NULL);

}

/*--------------------------------------------------------------------*/
void
do_scale_controls_tickinterval( Widget w, XtPointer client_data, XtPointer call_data)
{
    XmAnyCallbackStruct *acs = (XmAnyCallbackStruct*)call_data;
    int	    ivalue;
    int	    imax;

	/* get values of tick interval slider */
	XtVaGetValues(scale_controls_tickinterval, 
		    XmNvalue, &ivalue, 
		    XmNmaximum, &imax, 
		    NULL);
    
	/* reset values of tick interval slider to round numbers */
	if (ivalue > 2500)
	    ivalue = ((ivalue + 500) / 1000) * 1000;
	else if (ivalue > 500)
	    ivalue = ((ivalue + 250) / 500) * 500;
	else if (ivalue > 100)
	    ivalue = ((ivalue + 50) / 100) * 100;
	else if (ivalue > 50)
	    ivalue = ((ivalue + 25) / 50) * 50;
	else if (ivalue > 10)
	    ivalue = ((ivalue + 5) / 10) * 10;
	else if (ivalue > 5)
	    ivalue = ((ivalue + 2) / 5) * 5;
	if (ivalue == 1 && imax >= 40000)
	    imax = 500;
	if (ivalue == imax && imax <= 500)
	    imax = 40000;
	XtVaSetValues(scale_controls_tickinterval, 
			XmNminimum, 1, 
			XmNmaximum, imax, 
			XmNdecimalPoints, 2, 
			XmNvalue, ivalue, 
			NULL);
}

/*--------------------------------------------------------------------*/
void
do_controls_scale_colorinterval( Widget w, XtPointer client_data, XtPointer call_data)
{
    XmAnyCallbackStruct *acs = (XmAnyCallbackStruct*)call_data;
    int	    ivalue;
    int	    imax;

	/* get values of color interval slider */
	XtVaGetValues(scale_controls_colorinterval, 
		    XmNvalue, &ivalue, 
		    XmNmaximum, &imax, 
		    NULL);
    
	/* reset values of color interval slider to round numbers */
	if (ivalue > 2500)
	    ivalue = ((ivalue + 500) / 1000) * 1000;
	else if (ivalue > 500)
	    ivalue = ((ivalue + 250) / 500) * 500;
	else if (ivalue > 100)
	    ivalue = ((ivalue + 50) / 100) * 100;
	else if (ivalue > 50)
	    ivalue = ((ivalue + 25) / 50) * 50;
	else if (ivalue > 10)
	    ivalue = ((ivalue + 5) / 10) * 10;
	else if (ivalue > 5)
	    ivalue = ((ivalue + 2) / 5) * 5;
	if (ivalue == 1 && imax >= 40000)
	    imax = 500;
	if (ivalue == imax && imax <= 500)
	    imax = 40000;
	XtVaSetValues(scale_controls_colorinterval, 
			XmNminimum, 1, 
			XmNmaximum, imax, 
			XmNdecimalPoints, 2, 
			XmNvalue, ivalue, 
			NULL);
}
/*--------------------------------------------------------------------*/

void
do_scale_controls_precision( Widget w, XtPointer client_data, XtPointer call_data)
{
    XmAnyCallbackStruct *acs = (XmAnyCallbackStruct*)call_data;
}
/*--------------------------------------------------------------------*/

void
do_scale_controls_zoffset( Widget w, XtPointer client_data, XtPointer call_data)
{
    XmAnyCallbackStruct *acs = (XmAnyCallbackStruct*)call_data;
}

/*--------------------------------------------------------------------*/
void
do_file_new( Widget w, XtPointer client_data, XtPointer call_data)
{
    XmAnyCallbackStruct *acs = (XmAnyCallbackStruct*)call_data;
fprintf(stderr,"do_file_new\n");
}

/*--------------------------------------------------------------------*/
void
do_file_open( Widget w, XtPointer client_data, XtPointer call_data)
{
    XmAnyCallbackStruct *acs = (XmAnyCallbackStruct*)call_data;
fprintf(stderr,"do_file_open\n");
}

/*--------------------------------------------------------------------*/
void
do_file_importdata( Widget w, XtPointer client_data, XtPointer call_data)
{
    XmAnyCallbackStruct *acs = (XmAnyCallbackStruct*)call_data;
fprintf(stderr,"do_file_importdata\n");
}

/*--------------------------------------------------------------------*/
void
do_file_close( Widget w, XtPointer client_data, XtPointer call_data)
{
    XmAnyCallbackStruct *acs = (XmAnyCallbackStruct*)call_data;

    mbnavadjust_close_project();
    do_update_status();
}

/*--------------------------------------------------------------------*/
void
do_quit( Widget w, XtPointer client_data, XtPointer call_data)
{
    XmAnyCallbackStruct *acs = (XmAnyCallbackStruct*)call_data;
		
    /* unload loaded crossing */
    if (mbna_naverr_load == MB_YES)
	    {
	    status = mbnavadjust_crossing_unload();
	
	    /* deallocate graphics */
	    mbna_status = MBNA_STATUS_GUI;
	    XFreeGC(display,cont_gc);
	    XFreeGC(display,corr_gc);
	    xg_free(cont_xgid);
	    xg_free(corr_xgid);
	    mbnavadjust_naverr_checkoksettie();
	    do_update_naverr();
	    do_update_status();
	    }
}

/*--------------------------------------------------------------------*/
void
do_fileselection_mode( Widget w, XtPointer client_data, XtPointer call_data)
{
    XmAnyCallbackStruct *acs = (XmAnyCallbackStruct*)call_data;

    file_mode = (int) client_data;

    /* desl with selection */
    if (file_mode == FILE_MODE_NEW)
    	{
	tmp0 = (XmString) BX_CONVERT(fileSelectionBox, "*.nvh", 
                                          XmRXmString, 0, &argok);
    	}
    else if (file_mode == FILE_MODE_OPEN)
    	{
	tmp0 = (XmString) BX_CONVERT(fileSelectionBox, "*.nvh", 
                                          XmRXmString, 0, &argok);
    	}
    else if (file_mode == FILE_MODE_IMPORT)
    	{
	tmp0 = (XmString) BX_CONVERT(fileSelectionBox, "*.mb-1", 
                                          XmRXmString, 0, &argok);
    	}
    else
    	{
	tmp0 = (XmString) BX_CONVERT(fileSelectionBox, "*.nvh", 
                                          XmRXmString, 0, &argok);
    	}

    ac = 0;
    XtSetArg(args[ac], XmNpattern, tmp0); ac++;
    XtSetValues(fileSelectionBox, args, ac);
    XmStringFree((XmString)tmp0);

}

/*--------------------------------------------------------------------*/
void
do_fileselection_ok( Widget w, XtPointer client_data, XtPointer call_data)
{
    char ifile[STRING_MAX];
    char format_text[40];
    XmAnyCallbackStruct *acs = (XmAnyCallbackStruct*)call_data;

    /* get input filename */
    get_text_string(fileSelectionBox_text, ifile);

    /* desl with selection */
    if (file_mode == FILE_MODE_NEW)
    	{
        status = mbnavadjust_file_new(ifile);
    	}
    else if (file_mode == FILE_MODE_OPEN)
    	{
        status = mbnavadjust_file_open(ifile);
    	}
    else if (file_mode == FILE_MODE_IMPORT)
    	{
	get_text_string(textField_format, format_text);
	sscanf(format_text,"%d",&format);
    	status = mbnavadjust_import_data(ifile,format);
    	}
    do_update_status();
}

/*--------------------------------------------------------------------*/
void
do_fileselection_cancel( Widget w, XtPointer client_data, XtPointer call_data)
{
    XmAnyCallbackStruct *acs = (XmAnyCallbackStruct*)call_data;

    file_mode = FILE_MODE_NONE;
}

/*--------------------------------------------------------------------*/
void
do_view_showdata( Widget w, XtPointer client_data, XtPointer call_data)
{
    XmAnyCallbackStruct *acs = (XmAnyCallbackStruct*)call_data;

    mbna_view_list = MBNA_VIEW_LIST_FILES;
    do_update_status();
}

/*--------------------------------------------------------------------*/
void
do_view_showcrossings( Widget w, XtPointer client_data, XtPointer call_data)
{
    XmAnyCallbackStruct *acs = (XmAnyCallbackStruct*)call_data;

    mbna_view_list = MBNA_VIEW_LIST_CROSSINGS;
    do_update_status();
}
/*--------------------------------------------------------------------*/

void
do_view_showgoodcrossings( Widget w, XtPointer client_data, XtPointer call_data)
{
    XmAnyCallbackStruct *acs = (XmAnyCallbackStruct*)call_data;

    mbna_view_list = MBNA_VIEW_LIST_GOODCROSSINGS;
    do_update_status();
}

/*--------------------------------------------------------------------*/
void
do_view_showtruecrossings( Widget w, XtPointer client_data, XtPointer call_data)
{
    XmAnyCallbackStruct *acs = (XmAnyCallbackStruct*)call_data;

    mbna_view_list = MBNA_VIEW_LIST_TRUECROSSINGS;
    do_update_status();
}

/*--------------------------------------------------------------------*/
void
do_view_showties( Widget w, XtPointer client_data, XtPointer call_data)
{
    XmAnyCallbackStruct *acs = (XmAnyCallbackStruct*)call_data;

    mbna_view_list = MBNA_VIEW_LIST_TIES;
    do_update_status();
}

/*--------------------------------------------------------------------*/

void
do_action_fix( Widget w, XtPointer client_data, XtPointer call_data)
{
    XmAnyCallbackStruct *acs = (XmAnyCallbackStruct*)call_data;

    mbnavadjust_fix_file();
    do_update_status();
}

 /*--------------------------------------------------------------------*/

void
do_action_unfixfix( Widget w, XtPointer client_data, XtPointer call_data)
{
    XmAnyCallbackStruct *acs = (XmAnyCallbackStruct*)call_data;

    mbnavadjust_unfix_file();
    do_update_status();
}

/*--------------------------------------------------------------------*/

void
do_action_unfix( Widget w, XtPointer client_data, XtPointer call_data)
{
    XmAnyCallbackStruct *acs = (XmAnyCallbackStruct*)call_data;

    if (mbna_file_select >= 0
    	&& mbna_file_select < project.num_files)
    	{
    	project.files[mbna_file_select].status = MBNA_FILE_OK;
    	}
    do_update_status();
    sprintf(string, "Set file %d unfixed: %s\n",
		mbna_file_select,project.files[mbna_file_select].file);
    do_info_add(string, MB_YES);
}
/*--------------------------------------------------------------------*/

void
do_action_autopick( Widget w, XtPointer client_data, XtPointer call_data)
{
    XmAnyCallbackStruct *acs = (XmAnyCallbackStruct*)call_data;
    
    /* make sure that contours are generated for all of the existing crossings */
		
    mbna_status = MBNA_STATUS_MAKECONTOUR;
    mbnavadjust_autopick();
    mbna_status = MBNA_STATUS_GUI;
    
}
/*--------------------------------------------------------------------*/
void
do_action_analyzecrossings( Widget w, XtPointer client_data, XtPointer call_data)
{
    XmAnyCallbackStruct *acs = (XmAnyCallbackStruct*)call_data;
}

/*--------------------------------------------------------------------*/
void
do_action_invertnav( Widget w, XtPointer client_data, XtPointer call_data)
{
    XmAnyCallbackStruct *acs = (XmAnyCallbackStruct*)call_data;

    mbna_status = MBNA_STATUS_NAVSOLVE;
    mbnavadjust_invertnav();
    mbna_status = MBNA_STATUS_GUI;
    if (project.modelplot == MB_YES)
	mbnavadjust_modelplot_plot();
    do_update_status();
}


/*--------------------------------------------------------------------*/

void
do_modelplot_show( Widget w, XtPointer client_data, XtPointer call_data)
{
    XmAnyCallbackStruct *acs = (XmAnyCallbackStruct*)call_data;
    Window	modp_xid;
    Dimension	width, height;
    
    /* get drawingArea size */
    XtVaGetValues(drawingArea_modelplot, 
	XmNwidth, &width, 
	XmNheight, &height, 
	NULL);
    mbna_modelplot_width = width;
    mbna_modelplot_height = height;
    modp_borders[0] = 0;
    modp_borders[1] = mbna_modelplot_width - 1;
    modp_borders[2] = 0;
    modp_borders[3] = mbna_modelplot_height - 1;
    
    /* Setup just the "canvas" part of the screen. */
    modp_xid = XtWindow(drawingArea_modelplot);

    /* Setup the "graphics Context" for just the "canvas" */
    xgcv.background = WhitePixelOfScreen(DefaultScreenOfDisplay(display));
    xgcv.foreground = BlackPixelOfScreen(DefaultScreenOfDisplay(display));
    xgcv.line_width = 2;
    modp_gc = XCreateGC(display, modp_xid, GCBackground | GCForeground
	     | GCLineWidth, &xgcv);

    /* Setup the font for just the "canvas" screen. */
    fontStruct = XLoadQueryFont(display, xgfont);
    XSetFont(display,modp_gc,fontStruct->fid);
    XSelectInput(display, modp_xid, EV_MASK );

    /* Setup cursors. */
    myCursor = XCreateFontCursor(display, XC_target);
    XRecolorCursor(display,myCursor,&colors[2],&colors[5]);
    XDefineCursor(display,modp_xid,myCursor);

    /* initialize graphics */
    modp_xgid = xg_init(display, modp_xid, modp_borders, xgfont);
    status = mbnavadjust_set_modelplot_graphics(modp_xgid, modp_borders);

    /* set status flag */
    project.modelplot = MB_YES;
    mbna_modelplot_zoom_x1 = 0;
    mbna_modelplot_zoom_x2 = 0;
    mbna_modelplot_pingstart = 0;
    mbna_modelplot_pingend = project.num_pings - 1;
    mbna_modelplot_pickfile = MBNA_SELECT_NONE;
    mbna_modelplot_picksection = MBNA_SELECT_NONE;
    mbna_modelplot_picksnav = MBNA_SELECT_NONE;
    
    /* plot the model */
    mbnavadjust_modelplot_plot();
}
/*--------------------------------------------------------------------*/

void
do_modelplot_dismiss( Widget w, XtPointer client_data, XtPointer call_data)
{
    XmAnyCallbackStruct *acs = (XmAnyCallbackStruct*)call_data;

    /* deallocate graphics */
    project.modelplot = MB_NO;
    XFreeGC(display,modp_gc);
    xg_free(modp_xgid);
}

/*------------------------------------------------------------------------------*/
void 
do_modelplot_resize( Widget w, XtPointer client_data, XEvent *event, Boolean *unused)
{
    Window	modp_xid;
	XConfigureEvent *cevent = (XConfigureEvent *) event;
	Dimension	width, height;
	
	/* do this only if a resize event happens */
	if (cevent->type == ConfigureNotify)
	    {
	    /* get new shell size */
	    XtVaGetValues(bulletinBoard_modelplot, 
	    	XmNwidth, &width, 
	    	XmNheight, &height, 
	   	NULL);

	    /* do this only if the shell was REALLY resized and not just moved */
	    if (mbna_modelplot_width != width - MBNA_MODELPLOT_LEFT_WIDTH
	    	|| mbna_modelplot_height != height - MBNA_MODELPLOT_LEFT_HEIGHT)
	    	{
		/* set drawing area size */
		mbna_modelplot_width = width - MBNA_MODELPLOT_LEFT_WIDTH;
		mbna_modelplot_height = height - MBNA_MODELPLOT_LEFT_HEIGHT;
		ac = 0;
		XtSetArg(args[ac], XmNwidth, (Dimension) mbna_modelplot_width); ac++;
		XtSetArg(args[ac], XmNheight,(Dimension) mbna_modelplot_height); ac++;
		XtSetValues(drawingArea_modelplot, args, ac);

		/* deallocate graphics */
		XFreeGC(display,modp_gc);
		xg_free(modp_xgid);

		/* get drawingArea size */
		modp_borders[0] = 0;
		modp_borders[1] = mbna_modelplot_width - 1;
		modp_borders[2] = 0;
		modp_borders[3] = mbna_modelplot_height - 1;

		/* Setup just the "canvas" part of the screen. */
		modp_xid = XtWindow(drawingArea_modelplot);

		/* Setup the "graphics Context" for just the "canvas" */
		xgcv.background = WhitePixelOfScreen(DefaultScreenOfDisplay(display));
		xgcv.foreground = BlackPixelOfScreen(DefaultScreenOfDisplay(display));
		xgcv.line_width = 2;
		modp_gc = XCreateGC(display,modp_xid,GCBackground | GCForeground
			 | GCLineWidth, &xgcv);

		/* Setup the font for just the "canvas" screen. */
		fontStruct = XLoadQueryFont(display, xgfont);
		XSetFont(display,modp_gc,fontStruct->fid);
		XSelectInput(display, modp_xid, EV_MASK );

		/* Setup cursors. */
		myCursor = XCreateFontCursor(display, XC_target);
		XRecolorCursor(display,myCursor,&colors[2],&colors[5]);
		XDefineCursor(display,modp_xid,myCursor);

		/* initialize graphics */
		modp_xgid = xg_init(display, modp_xid, modp_borders, xgfont);
		status = mbnavadjust_set_modelplot_graphics(modp_xgid, modp_borders);

		/* plot the model */
		mbnavadjust_modelplot_plot();

	    	}
	    }
}

/*--------------------------------------------------------------------*/

void
do_modelplot_fullsize( Widget w, XtPointer client_data, XtPointer call_data)
{
    XmAnyCallbackStruct *acs = (XmAnyCallbackStruct*)call_data;

	/* replot model */
	mbna_modelplot_zoom_x1 = 0;
	mbna_modelplot_zoom_x2 = 0;
	mbna_modelplot_pingstart = 0;
	mbna_modelplot_pingend = project.num_pings - 1;
	mbnavadjust_modelplot_setzoom();
	mbnavadjust_modelplot_plot();
}

/*--------------------------------------------------------------------*/

void
do_modelplot_input( Widget w, XtPointer client_data, XtPointer call_data)
{
    XmAnyCallbackStruct *acs = (XmAnyCallbackStruct*)call_data;
    XEvent  *event = acs->event;
    
    /* If there is input in the drawing area */
    if (acs->reason == XmCR_INPUT)
    {
      /* Check for mouse pressed. */
      if (event->xany.type == ButtonPress)
      {
	  /* If left mouse button is pushed */
	  if (event->xbutton.button == 1)
		    {
		    button1down = MB_YES;

		    } /* end of left button events */

	    /* If middle mouse button is pushed */
	    if (event->xbutton.button == 2)
		    {
		    button2down = MB_YES;
		    mbna_modelplot_zoom_x1 = event->xbutton.x;
		    mbna_modelplot_zoom_x2 = event->xbutton.x;
	    	
		    /* replot model */
		    mbnavadjust_modelplot_plot();
		    } /* end of middle button events */

	    /* If right mouse button is pushed */
	    if (event->xbutton.button == 3)
		    {
		    button3down = MB_YES;
		    } /* end of right button events */	
      } /* end of button press events */

      /* Check for mouse released. */
      if (event->xany.type == ButtonRelease)
      {
	  /* If left mouse button is released */
	    if (event->xbutton.button == 1)
	    	  {
	    	  button1down = MB_NO;
		  
		  /* pick nearest tie point */
		  mbnavadjust_modelpot_pick(event->xbutton.x, event->xbutton.y);

		  /* replot model */
		  mbnavadjust_modelplot_plot();
	    	  }

	    /* If middle mouse button is released */
	    if (event->xbutton.button == 2)
	    	  {
	    	  button2down = MB_NO;
	    	  mbna_modelplot_zoom_x2 = event->xbutton.x;

		  /* replot model */
		  mbnavadjust_modelplot_setzoom();
		  mbnavadjust_modelplot_plot();
		  mbna_modelplot_zoom_x1 = 0;
		  mbna_modelplot_zoom_x2 = 0;
		  }

	    /* If right mouse button is released */
	    if (event->xbutton.button == 3)
	    	  {
	    	  button3down = MB_NO;
		  
		  /* pick nearest tie point */
		  mbnavadjust_modelpot_repick(event->xbutton.x, event->xbutton.y);

		  /* replot model */
		  mbnavadjust_modelplot_plot();
	    	  }

      } /* end of button release events */

      /* Check for mouse motion while pressed. */
      if (event->xany.type == MotionNotify)
      {
	    /* If middle mouse button is held during motion */
	    if (button2down == MB_YES)
		{
	    	 mbna_modelplot_zoom_x2 = event->xbutton.x;
	    	
		 /* replot model */
		 mbnavadjust_modelplot_plot();
	    	}
      }
    } /* end of inputs from window */

}

/*--------------------------------------------------------------------*/

void
do_modelplot_expose( Widget w, XtPointer client_data, XtPointer call_data)
{
    XmAnyCallbackStruct *acs = (XmAnyCallbackStruct*)call_data;

    /* replot the model */
    mbnavadjust_modelplot_plot();
}

/*--------------------------------------------------------------------*/
/* ARGSUSED */
void
do_fileselection_list(Widget w, XtPointer client, XtPointer call)
{
	int	error;
	char	fileroot[MB_PATH_MAXLINE];
	int	form;
	char	value_text[128];

	/*SUPPRESS 594*/XmAnyCallbackStruct *acs=(XmAnyCallbackStruct*)call;

	/* get selected text */
	get_text_string(fileSelectionBox_text, string);

	/* get output file */
	if((int)strlen(string) > 0)
		{
		status = mb_get_format(mbna_verbose, string, fileroot, 
					    &form, &error);
		if (status == MB_SUCCESS)
			{
			format = form;
			sprintf(value_text,"%d",format);
			XmTextFieldSetString(
			    textField_format,
			    value_text);
			}
		}
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
    for (topshell = scrolledWindow_datalist;
	    !XtIsTopLevelShell(topshell);
	    topshell = XtParent(topshell))
	;
	
    /* keep processing events until it is viewed */
    if (XtIsRealized(topshell))
	{
	topwindow = XtWindow(topshell);
	
	/* wait for the window to be mapped */
	while (XGetWindowAttributes(
			XtDisplay(form_mbnavadjust),
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
    
    if (mbna_verbose >= 1)
    	fprintf(stderr,"%s\n",message);

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
	while (XGetWindowAttributes(XtDisplay(bulletinBoard_message), diawindow, &xwa)
		&& xwa.map_state != IsViewable)
	    {
	    if (XGetWindowAttributes(XtDisplay(bulletinBoard_message), topwindow, &xwa)
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
    XmUpdateDisplay(bulletinBoard_message);
		
    return(MB_SUCCESS);
}

/*--------------------------------------------------------------------*/

int
do_info_add(char *info, int timetag)
{
    int		pos;
    char	tag[STRING_MAX];
    time_t	right_now;
    char	date[25], user[128], *user_ptr, host[128];
    char	*ctime();
    char	*getenv();
    
    /* reposition to end of text */
    pos = XmTextGetLastPosition(text_messages);
    XmTextSetInsertionPosition(text_messages, pos);
    
    /* add text */
    XmTextInsert(text_messages, pos, info);
    if (project.logfp != NULL)
	fputs(info, project.logfp);
    if (mbna_verbose > 0)
	fputs(info, stderr);
    
    /* put time tag in if requested */
    if (timetag == MB_YES)
	{
	right_now = time((time_t *)0);
	memset(date,0,25);
	strncpy(date,ctime(&right_now),24);
	if ((user_ptr = getenv("USER")) == NULL)
		user_ptr = getenv("LOGNAME");
	if (user_ptr != NULL)
		strcpy(user,user_ptr);
	else
		strcpy(user, "unknown");
	gethostname(host,128);
	sprintf(tag," > User <%s> on cpu <%s> at <%s>\n",
		user,host,date);
	pos = XmTextGetLastPosition(text_messages);
	XmTextSetInsertionPosition(text_messages, pos);
	XmTextInsert(text_messages, pos, tag);
	if (project.logfp != NULL)
	    fputs(tag, project.logfp);
	if (mbna_verbose > 0)
	    fputs(tag, stderr);
	}
    
    /* reposition to end of text */
    pos = XmTextGetLastPosition(text_messages);
    XmTextShowPosition(text_messages, pos);
    XmTextSetInsertionPosition(text_messages, pos);
		
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
    XBell(XtDisplay(form_mbnavadjust),100);
		
    return(MB_SUCCESS);
}

/*--------------------------------------------------------------------*/
void
do_bell(length)
int	length;
{
	XBell(display,length);
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

void
do_update_project( Widget w, XtPointer client_data, XtPointer call_data)
{
    XmAnyCallbackStruct *acs = (XmAnyCallbackStruct*)call_data;
}
/*--------------------------------------------------------------------*/

void
do_make_grid( Widget w, XtPointer client_data, XtPointer call_data)
{
    XmAnyCallbackStruct *acs = (XmAnyCallbackStruct*)call_data;
}
