/************************************************************/
/*                                                          */
/* mbvelocity.c                                             */
/*                                                          */
/* THIS PROGRAM ALLOWS YOU TO EDIT THE VELOCITY PROFILE.    */
/*                                                          */
/************************************************************/
/*                                                          */
/* LANGUAGE "C"                                             */
/*                                                          */
/* WRITTEN FOR MOTIF                                        */
/*                                                          */
/************************************************************/
/*    The MB-system:	mbvelocitytool_stubs.c	6/6/93
 *    $Id: mbvelocity.c,v 4.3 1995-02-14 18:26:46 caress Exp $
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
 * MBVELOCITYTOOL is an interactive water velocity profile editor
 * used to examine multiple water velocity profiles and to create
 * new water velocity profiles which can be used for the processing
 * of multibeam sonar data.  In general, this tool is used to examine
 * water velocity profiles obtained from XBTs, CTDs, or databases,
 * and to construct new profiles consistent with these various
 * sources of information.
 *
 * Author:	D. W. Caress
 * Date:	June 6, 1993
 *
 * $Log: not supported by cvs2svn $
 * Revision 4.2  1994/11/18  18:58:19  caress
 * First gradient raytracing version.
 *
 * Revision 4.1  1994/11/10  01:16:07  caress
 * Set program to do raytracing for every ping rather than once at beginning.
 *
 * Revision 4.0  1994/10/21  12:43:44  caress
 * Release V4.0
 *
 * Revision 4.3  1994/04/12  14:32:21  caress
 * Changed a few "Hydrosweep" text strings to "Multibeam"
 *
 * Revision 4.2  1994/04/12  01:13:24  caress
 * First cut at translation from hsvelocitytool. The new program
 * mbvelocitytool will deal with all supported multibeam data
 * including travel time observations.
 *
 * Revision 4.0  1994/03/05  23:51:19  caress
 * First cut at version 4.0
 *
 * Revision 4.1  1994/03/03  03:53:26  caress
 * Fixed copyright message.
 *
 * Revision 4.0  1994/02/27  00:17:23  caress
 * First cut at new version.
 *
 * Revision 1.1  1993/08/16  23:28:30  caress
 * Initial revision
 *
 *
 */


/************************************************************/
/* INCLUDE FILES                                            */
/************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/dir.h>
#include <sys/param.h>
#include <math.h>
#include <time.h>
#include <X11/cursorfont.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Intrinsic.h>
#include <X11/keysym.h>
#include <X11/keysymdef.h>
#include <Xm/Xm.h>
#include <Xm/DialogS.h>
#include <Xm/BulletinB.h>
#include <Xm/DrawingA.h>
#include <Xm/MainW.h>
#include <Xm/PushB.h>
#include <Xm/Label.h>
#include <Xm/MessageB.h>
#include <Xm/Text.h>
#include <Xm/List.h>
#include <Xm/Scale.h>
#include <Xm/FileSB.h>
#include <Xm/RowColumn.h>
#include <Mrm/MrmAppl.h>
#include <Mrm/MrmPublic.h>
#include <Mrm/MrmDecls.h>

#include "mbvelocity_define.c"

#define MAX_WIDGETS (k_max_widget + 1)
#define ERROR (-1)

#define font1 "-adobe-helvetica-bold-r-normal--14-140-75-75-p-82-iso8859-1"

#define charset XmSTRING_DEFAULT_CHARSET

#define EV_MASK (ButtonPressMask | KeyPressMask | ExposureMask )

/************************************************************/
/* GLOBAL VARIABLES                                         */
/************************************************************/

XtAppContext app_context;
static Widget toplevel_widget, main_window_widget;
static Widget widget_array[MAX_WIDGETS]; 
static int toggle_array[MAX_WIDGETS];
Display *mb_display, *theDisplay;
Screen *screen, *can_screen;
Window frm_xid, can_xid, root_return, child_return;
Colormap theColormap, colormap;
GC mb_gc;
XGCValues xgcv;
Pixmap theIconPixmap, crosshairPix;

int screen_num, can_screen_num;
int theDepth;

static XmString latin_zero;
static MrmHierarchy s_MrmHierarchy;
static MrmType dummy_class;

int	status;
static char *ed_message;
static char *mb_message;
static char *status_message;
static char message_str[256];

Arg arglist[2];

static char	*input_file;
static char	output_file[128];

int	can_xgid;		/* XG graphics id */
int	frm_xgid;		/* XG graphics id */
Cursor myCursor;
XColor closest[2];
XColor exact[2];

/* Set the colors used for this program here. */
#define NCOLORS 6
XColor colors[NCOLORS];
unsigned long pixel_values_gui[NCOLORS];
XColor db_color;

/* Global Xwindows graphics parameters */
char	*fontname = "8x13";

/* Global mbvelocitytool definitions */
int	edit_gui;
int	ndisplay_gui;
int	maxdepth_gui;
int	velrange_gui;
int	resrange_gui;
int	format_gui;

/* file opening parameters */
int	startup_file = 0;
int	open_files_count = 0;
struct direct **open_files;
int	open_type;
#define	OPEN_NONE		0
#define	OPEN_DISPLAY_PROFILE	1
#define	OPEN_EDIT_PROFILE	2
#define	OPEN_MULTIBEAM		3


/* Set these to the dimensions of your canvas drawing */
/* area, minus 1, located in mbvelocity.uil.              */
static int borders[4] =
	{ 0, 1098, 0, 649 };


/************************************************************/
/* END OF GLOBAL VARIABLES                                  */
/************************************************************/

/************************************************************/
/* LIST ALL UID FILES HERE                                  */
/************************************************************/
#include "mbvelocity_uid_loc.h"

/************************************************************/
/* AUTOMATICALLY DETERMINE THE NUMBER OF UID FILES          */
/************************************************************/

static int db_filename_num = (sizeof db_filename_vec /
                              sizeof db_filename_vec[0]);

/************************************************************/
/* DECLARE FUNCTIONS                                        */
/************************************************************/

static void s_error();
static void create_proc();
static void init_data();
static void display_menu();
static void action_maxdepth();
static void action_velrange();
static void action_residual_range();
static void action_process_mb();
static void action_quit();
static void action_new_profile();
static void action_menu_close_profile();
static void controls_open_file();
static void controls_open_ed_file();
static void controls_save_file();
static void open_mb_data();
static void open_file_ok();
static void mbvelocity_set_controls();
static void action_canvas_event();
static void xg_setclip();
static void xg_setwincolormap();
static void xg_drawline();
static void xg_drawrectangle();
static void xg_fillrectangle();
static void xg_drawstring();
static void xg_justify();
static void get_file_selection();

/************************************************************/
/* LIST THE CALLBACK PROCEDURES USED IN THE UIL FILES AND   */
/* THEIR ADDRESSES SO DRM CAN BIND THEM.                    */
/************************************************************/

static MrmRegisterArg reglist[] = 
	{
	  {"create_proc",		(caddr_t) create_proc},
	  {"action_maxdepth",		(caddr_t) action_maxdepth},
	  {"action_velrange",		(caddr_t) action_velrange},
	  {"action_residual_range",	(caddr_t) action_residual_range},
	  {"action_process_mb",		(caddr_t) action_process_mb},
	  {"action_quit",		(caddr_t) action_quit},
	  {"action_new_profile",	(caddr_t) action_new_profile},
	  {"action_menu_close_profile",	(caddr_t) action_menu_close_profile},
	  {"controls_open_file",	(caddr_t) controls_open_file},
	  {"controls_open_ed_file",	(caddr_t) controls_open_ed_file},
	  {"controls_save_file",	(caddr_t) controls_save_file},
	  {"open_mb_data",		(caddr_t) open_mb_data},
	  {"open_file_ok",		(caddr_t) open_file_ok},
	  {"action_canvas_event",	(caddr_t) action_canvas_event},
	  {"display_menu",		(caddr_t) display_menu}
	};


/************************************************************/
/* AUTOMATICALLY DETERMINE THE NUMBER OF PROCEDURES TO BIND */
/************************************************************/

static int reglist_num = (sizeof reglist / sizeof reglist[0]);

/************************************************************/
/* SET OPEN STATEMENT FOR FILES                             */
/************************************************************/

FILE *fopen();

/************************************************************/
/* main PROGRAM                                             */
/************************************************************/
unsigned int main(argc, argv)

	int argc;
	String		argv[];
{
	int n;
	Arg arglist[2];
	int	i;


/************************************************************/
/* INITIALIZE DRM                                           */
/************************************************************/

	MrmInitialize();

/************************************************************/
/* INITIALIZE THE X TOOLKIT - WE GET BACK THE TOP LEVEL     */
/* SHELL WIDGET                                             */
/************************************************************/


	XtToolkitInitialize();

	app_context = XtCreateApplicationContext();

	mb_display = XtOpenDisplay(app_context,
		  NULL,
		  argv[0],
		  "MBVELOCITYTOOL",
		  NULL,
		  0,
		  &argc,
		  argv);

	if(mb_display == NULL)
	{
	 fprintf(stderr, "%s: CAN'T OPEN DISPLAY", argv[0]);
	 exit(1);
	}

	n = 0;
	XtSetArg(arglist[n], XmNallowShellResize, TRUE);
	n++;

	toplevel_widget = XtAppCreateShell(
			  argv[0],
			  NULL,
			  applicationShellWidgetClass,
			  mb_display,
			  arglist,
			  n);

/************************************************************/
/* OPEN THE UID FILES                                       */
/************************************************************/

	if (MrmOpenHierarchy( db_filename_num,
			      db_filename_vec,
			      NULL,
			      &s_MrmHierarchy)
		!=MrmSUCCESS) 
		{
		  s_error("CAN'T OPEN HIERARCHY");
		}

/************************************************************/
/* RUN FUNCTION TO INITIALIZE DATA                          */
/************************************************************/

	init_data();

/************************************************************/
/* REGISTER THE NAMES OF THE PROCEDURES THAT DRM NEEDS TO   */
/* BIND.                                                    */
/************************************************************/

	MrmRegisterNames(reglist, reglist_num);

/************************************************************/
/* FETCH THE MAIN WINDOW WIDGET FROM THE UID FILE.          */
/************************************************************/

	if (MrmFetchWidget(s_MrmHierarchy,
			   "window_mbvelocity",
			   toplevel_widget,
			   &main_window_widget,
			   &dummy_class)
		!= MrmSUCCESS)
		{
		  s_error("CAN'T FETCH MAIN WINDOW");
		}


/************************************************************/
/* MANAGE THE MAIN WINDOW                                   */
/************************************************************/


	XtManageChild(main_window_widget);

/************************************************************/
/* NOW REALIZE THE TOP LEVEL WIDGET - THIS WILL DISPLAY     */
/* THE WIDGET.                                              */
/************************************************************/

	XtRealizeWidget(toplevel_widget);

/************************************************************/
/* THESE WIDGETS ARE FETCHED HERE SO THAT THEY CAN BE       */
/* INITIALIZED BY OTHER PROCESSES EVEN IF THEY HAD NOT BEEN */
/* USED PREVIOUSLY.                                         */
/************************************************************/

	/* get widgets from main controls */
	if(widget_array[k_mb_main] == NULL)
	{
	  if(MrmFetchWidget(s_MrmHierarchy,
			    "main_input_board",
			    toplevel_widget,
			    &widget_array[k_mb_main],
	   		    &dummy_class)
		!=MrmSUCCESS)
		{
		  s_error("CAN'T FETCH B BOARD");
		}
	  XtManageChild(widget_array[k_mb_main]);
	}

	/* get widgets from file save dialog */
	if(widget_array[k_popup_save_ed] == NULL)
	{
          if(MrmFetchWidget(s_MrmHierarchy,
                            "popup_save_file",
                            toplevel_widget,
                            &widget_array[k_popup_save_ed],
                            &dummy_class)
                !=MrmSUCCESS)
                {
                  s_error("CAN'T FETCH POPUP SAVE ED FILE MENU");
                }
	}

	/* get widgets from file open dialog */
	if(widget_array[k_file_sel_board] == NULL)
        {
          if(MrmFetchWidget(s_MrmHierarchy,
                            "file_select_board",
                            toplevel_widget,
                            &widget_array[k_file_sel_board],
                            &dummy_class)
                !=MrmSUCCESS)
                {
                  s_error("CAN'T FETCH FILE MENU");
                }
	}

	/* get widgets from multibeam file open dialog */
	if(widget_array[k_mb_file_sel_board] == NULL)
        {
           if(MrmFetchWidget(s_MrmHierarchy,
                            "mb_file_select_board",
                            toplevel_widget,
                            &widget_array[k_mb_file_sel_board],
                            &dummy_class)
                !=MrmSUCCESS)
                {
                  s_error("CAN'T FETCH MB FILE MENU");
                }
	}
	widget_array[k_filelist_list] =
	    XmFileSelectionBoxGetChild(widget_array[k_file_sel_box], 
					XmDIALOG_LIST);
	widget_array[k_selection_text] =
	    XmFileSelectionBoxGetChild(widget_array[k_file_sel_box], 
					XmDIALOG_TEXT);
	XtAddCallback(widget_array[k_filelist_list], 
		XmNbrowseSelectionCallback, 
		get_file_selection, NULL);

/************************************************************/
/* SET UP FONTS AND CURSORS FOR THIS DISPLAY                */
/************************************************************/

	/* Setup the entire screen. */
	screen = DefaultScreenOfDisplay(mb_display);
	screen_num = XDefaultScreen(mb_display);
	frm_xid = XtWindow(widget_array[k_mb_main]);
	colormap = DefaultColormap(mb_display,screen_num);

	/* Setup just the "canvas" part of the screen. */
	theDisplay = XtDisplay(widget_array[k_main_graph]);
	can_screen = DefaultScreenOfDisplay(theDisplay);
	can_xid = XtWindow(widget_array[k_main_graph]);
	can_screen_num = XDefaultScreen(theDisplay);
	theColormap = DefaultColormap(theDisplay,can_screen_num);

	mb_gc = XCreateGC(mb_display,frm_xid,NULL,NULL);
	XSelectInput(theDisplay, can_xid, EV_MASK );

	/* Load the colors that will be used in this program. */
	status = XLookupColor(theDisplay,colormap,
		"white",&db_color,&colors[0]);
	if(status != 0)
		status = XAllocColor(theDisplay,colormap,&colors[0]);
	if (status == 0)
		{
		fprintf(stderr,"Failure to allocate color: white\n");
		exit(-1);
		}
	status = XLookupColor(theDisplay,colormap,
		"black",&db_color,&colors[1]);
	if(status != 0)
		status = XAllocColor(theDisplay,colormap,&colors[1]);
	if (status == 0)
		{
		fprintf(stderr,"Failure to allocate color: black\n");
		exit(-1);
		}
	status = XLookupColor(theDisplay,colormap,
		"red",&db_color,&colors[2]);
	if(status != 0)
		status = XAllocColor(theDisplay,colormap,&colors[2]);
	if (status == 0)
		{
		fprintf(stderr,"Failure to allocate color: red\n");
		exit(-1);
		}
	status = XLookupColor(theDisplay,colormap,
		"green",&db_color,&colors[3]);
	if(status != 0)
		status = XAllocColor(theDisplay,colormap,&colors[3]);
	if (status == 0)
		{
		fprintf(stderr,"Failure to allocate color: green\n");
		exit(-1);
		}
	status = XLookupColor(theDisplay,colormap,
		"blue",&db_color,&colors[4]);
	if(status != 0)
		status = XAllocColor(theDisplay,colormap,&colors[4]);
	if (status == 0)
		{
		fprintf(stderr,"Failure to allocate color: blue\n");
		exit(-1);
		}
	status = XLookupColor(theDisplay,colormap,
		"coral",&db_color,&colors[5]);
	if(status != 0)
		status = XAllocColor(theDisplay,colormap,&colors[5]);
	if (status == 0)
		{
		fprintf(stderr,"Failure to allocate color: coral\n");
		exit(-1);
		}
	for (i=0;i<NCOLORS;i++)
		{
		pixel_values_gui[i] = colors[i].pixel;
		}

	/* Setup cursor. */
	myCursor = XCreateFontCursor(theDisplay, XC_target);
	XAllocNamedColor(theDisplay,colormap,"red",&closest[0],&exact[0]);
	XAllocNamedColor(theDisplay,colormap,"coral",&closest[1],&exact[1]);
	XRecolorCursor(theDisplay,myCursor,&closest[0],&closest[1]);
	XDefineCursor(theDisplay,can_xid,myCursor);


/************************************************************/
/* RUN FUNCTION TO SETUP THE SCREENS                        */
/************************************************************/

	/* initialize graphics */
	/* returns pointer to the 'graphic' struct */
	can_xgid = xg_init(theDisplay, can_xid, borders, font1);

	status = mbvt_set_graphics(can_xgid, borders, 
			NCOLORS, pixel_values_gui);

	/* initialize mbvelocitytool proper */
	status = mbvt_init(argc,argv);

	/* replot everything */
	mbvelocity_set_controls();

	mbvt_plot();
	

/************************************************************/
/* NOW SIT IN MAIN LOOP AND WAIT FOR THE USER TO INPUT      */
/* DATA INTO THE USER INTERFACE                             */
/************************************************************/

	XtAppMainLoop(app_context);

}

/************************************************************/
/* THIS IS THE END OF THE MAIN PROGRAM                      */
/************************************************************/

/************************************************************/
/* INITIALIZE THE widget array AND THE toggle_array TO NULL */
/* AND 0. THE WIDGET ARRAYS WILL BE SET WHEN THEY ARE       */
/* MANAGED. THE TOGGLE ARRAYS WILL BE USED FOR THE MODE     */
/* SELECTIONS.                                              */
/************************************************************/
static void init_data()
{
	int i;

	for (i = 0; i < MAX_WIDGETS;	 i++)
	  {
	    widget_array[i] = NULL;
	    toggle_array[i] = 0;
	  }

}

/************************************************************/
/* END OF FILE "mbvelocity.c".                              */
/************************************************************/
/************************************************************/
/* mbvelocity_stubs.c                                       */
/*                                                          */
/* THIS CODE HANDLES THE CALLBACKS FROM THE USER INTERFACE  */
/* FOR THE MBVELOCITY PROGRAM.                              */
/*                                                          */
/************************************************************/

/************************************************************/
/* THIS DISPLAYS THE PRELIMINARY ERROR MESSAGES.            */
/* ADDITIONAL ERROR DIALOG BOXES WILL BE ADDED TO CHECK AND */
/* NOTIFY THE USER IF BAD DATA WAS ENTERED INTO ANY FIELD.  */
/************************************************************/

static void s_error(problem_string)
        char *problem_string;
        {
          printf("%s\n", problem_string);
          exit(0);
        }


/************************************************************/
/* THIS FUNCTION CHANGES THE WIDGET ARRAY VALUE FROM "NULL" */
/* TO THE int VALUE ASSIGNED TO IT. THIS WAY YOU KNOW ITS   */
/* BEEN CALLED AND WON'T RECREATE IT AND WASTE MEMORY SPACE.*/
/************************************************************/

static void create_proc(w, tag, reason)
	Widget w;
	int *tag;
	unsigned long *reason;
{
	  int widget_num = *tag;
	  widget_array[widget_num] = w;

}


/********************************************************************/
/* THIS FUNCTION IS USED TO DISPLAY MENUS CALLED FROM PULLDOWN MENUS*/
/********************************************************************/
static void display_menu(w, tag, list) 
	Widget w;
	int *tag;
	XmListCallbackStruct *list;
{
	int widget_num = *tag;

	switch (widget_num)
	{

	  /* Display the Save Editable Profile screen. */

	  case k_save_ed_file:
	  {
	    if(widget_array[k_popup_save_ed] == NULL)
            {
              if(MrmFetchWidget(s_MrmHierarchy,
                            "popup_save_file",
                            toplevel_widget,
                            &widget_array[k_popup_save_ed],
                            &dummy_class)
                !=MrmSUCCESS)
                {
                  s_error("CAN'T FETCH POPUP SAVE ED FILE MENU");
                }

              XtManageChild(widget_array[k_popup_save_ed]);
             }
	     else
	     {
              XtUnmanageChild(widget_array[k_popup_save_ed]);
              XtManageChild(widget_array[k_popup_save_ed]);
	     }
	  }
	  break;

	  /* Remove the Save Editable Profile Screen if "Cancel"*/
	  /*   was selected. */

	  case k_cancel_ed_file:
	  {
	     /* Remove popup screen */
             XtUnmanageChild(widget_array[k_popup_save_ed]);

	     /* replot everything */
	     mbvelocity_set_controls();
	     mbvt_plot();

	  }
	  break;

	  /* display the file selection menu for a display file */

	  case k_file_menu:
	  {
 	    /* set file type flag */
	    open_type = OPEN_DISPLAY_PROFILE;
		
	    if(widget_array[k_file_sel_board] == NULL)
            {
              if(MrmFetchWidget(s_MrmHierarchy,
                            "file_select_board",
                            toplevel_widget,
                            &widget_array[k_file_sel_board],
                            &dummy_class)
                !=MrmSUCCESS)
                {
                  s_error("CAN'T FETCH FILE MENU");
                }

              XtManageChild(widget_array[k_file_sel_board]);
             }
	     else
	     {
              XtUnmanageChild(widget_array[k_file_sel_board]);
              XtManageChild(widget_array[k_file_sel_board]);
	     }
	  }
	  break;


	  /* display the file selection menu for an editable file */

	  case k_open_ed_file:
	  {
	    /* set file type flag */
	    open_type = OPEN_EDIT_PROFILE;
		
	    if(widget_array[k_file_sel_board] == NULL)
            {
              if(MrmFetchWidget(s_MrmHierarchy,
                            "file_select_board",
                            toplevel_widget,
                            &widget_array[k_file_sel_board],
                            &dummy_class)
                !=MrmSUCCESS)
                {
                  s_error("CAN'T FETCH FILE MENU");
                }

              XtManageChild(widget_array[k_file_sel_board]);
             }
	     else
	     {
              XtUnmanageChild(widget_array[k_file_sel_board]);
              XtManageChild(widget_array[k_file_sel_board]);
	     }
	  }
	  break;

	  /* display the file selection menu for MultiBeam files */

	  case k_open_mb_data:
	  {
	    /* set file type flag */
	    open_type = OPEN_MULTIBEAM;
		
	    if(widget_array[k_mb_file_sel_board] == NULL)
            {
              if(MrmFetchWidget(s_MrmHierarchy,
                            "mb_file_select_board",
                            toplevel_widget,
                            &widget_array[k_mb_file_sel_board],
                            &dummy_class)
                !=MrmSUCCESS)
                {
                  s_error("CAN'T FETCH MB FILE MENU");
                }

              XtManageChild(widget_array[k_mb_file_sel_board]);
             }
	     else
	     {
              XtUnmanageChild(widget_array[k_mb_file_sel_board]);
              XtManageChild(widget_array[k_mb_file_sel_board]);
	     }
	  }
	  break;

	  /* Remove the file selection box. */
	  case k_main_graph:
	  {

	     /* replot everything */
	     mbvelocity_set_controls();
	     mbvt_plot();

	  }
	  break;

	  /* Remove the multibeam file selection box. */
	  case k_cancel_mb_file_sel_box:
	  {
	     /* Remove popup screen */
              XtUnmanageChild(widget_array[k_mb_file_sel_board]);

	     /* replot everything */
	     mbvelocity_set_controls();
	     mbvt_plot();

	  }
	  break;

	  /* Remove the display and editable file selection box. */
	  case k_cancel_file_sel_box:
	  {
	     /* Remove popup screen */
              XtUnmanageChild(widget_array[k_file_sel_board]);

	     /* replot everything */
	     mbvelocity_set_controls();
	     mbvt_plot();

	  }
	  break;

	  default:
	  break;
	} /* end switch */
} /* end display menu */

/************************************************************/
/*
 * Function to get control values from mbvelocitytool proper
 * and to set the appropriate graphics control values.
 */
/************************************************************/

static void mbvelocity_set_controls()
{
	char	value_text[10];

	/* get some values from mbvelocitytool */
	mbvt_get_values(&edit_gui,&ndisplay_gui,&maxdepth_gui,
		&velrange_gui,&resrange_gui,&format_gui);

	/* set values of maximum depth slider */
	XtVaSetValues(widget_array[k_max_depth], 
			XmNvalue, maxdepth_gui, 
			NULL);

	/* set values of velocity range slider */
	XtVaSetValues(widget_array[k_action_vel], 
			XmNvalue, velrange_gui, 
			NULL);

	/* set values of residual range slider */
	XtVaSetValues(widget_array[k_action_res], 
			XmNvalue, resrange_gui, 
			NULL);

	/* set value of format text item */
	sprintf(value_text,"%2.2d",format_gui);
	XmTextFieldSetString(widget_array[k_mbio_format], value_text);
}

/********************************************************************/
/* File selector routine called by scandir().                       */
/* Return TRUE if filename is not "." or "..".                      */
/********************************************************************/

 int open_files_select(entry)
	struct direct	*entry;
{
	if ((strcmp(entry->d_name, ".") == 0) ||
	    (strcmp(entry->d_name, "..") == 0))
		return (FALSE);
	else
		return(TRUE);
}


/************************************************************/
/*
 * Menu handler for `menu_file (Open display profile)'.
 */
/************************************************************/

static void controls_open_file(w, tag, list)
	Widget w;
	int *tag;
	XmListCallbackStruct *list;
{
	/* local variables */
	int	alphasort();
	
		/* delete old scrolling list */
		if (open_files_count > 0)
			{
			open_files_count = 0;
			}

		/* load filenames into scrolling list */
		open_files_count = scandir(".",&open_files,
			open_files_select, alphasort);

		/* set file type flag */
		open_type = OPEN_DISPLAY_PROFILE;

}

/************************************************************/
/*
 * Menu handler for `menu_file (Open editable profile)'.
 */
/************************************************************/

static void controls_open_ed_file(w, tag, list)
	Widget w;
	int *tag;
	XmListCallbackStruct *list;
{
	/* local variables */
	int	alphasort();


		/* delete old scrolling list */
		if (open_files_count > 0)
			{
			open_files_count = 0;
			}

		/* load filenames into scrolling list */
		open_files_count = scandir(".",&open_files,
			open_files_select, alphasort);

		/* set file type flag */
		open_type = OPEN_EDIT_PROFILE;
		

}

/************************************************************/
/*
 * Menu handler for `menu_file (New editable profile)'.
 */
/************************************************************/

static void action_new_profile(w, tag, list)
	Widget w;
	int *tag;
	XmListCallbackStruct *list;
{
	
	/* get new edit velocity profile */
	mbvt_new_edit_profile();

	strcpy(message_str, "Open Editable Sound Velocity Profile: no filename");
	ed_message = (char *) 
		XmStringLtoRCreate(message_str,"");
	XtSetArg (arglist[0], XmNlabelString, ed_message);
	XtSetValues (widget_array[k_ed_message], arglist, 1);

	/* replot everything */
	mbvelocity_set_controls();

	mbvt_plot();
		
}

/************************************************************/
/*
 * Menu handler for `menu_file (Save editable profile)'.
 */
/************************************************************/

static void controls_save_file(w, tag, list)
	Widget w;
	int *tag;
	XmListCallbackStruct *list;
{
	char	*save_file;

	save_file = XmTextGetString(widget_array[k_save_filename]);

	mbvt_save_edit_profile(save_file);

	strcpy(message_str, "Open Editable Sound Velocity Profile: ");
	strcat(message_str, save_file);
	ed_message = (char *) 
		XmStringLtoRCreate(message_str,"");
	XtSetArg (arglist[0], XmNlabelString, ed_message);
	XtSetValues (widget_array[k_ed_message], arglist, 1);

	/* Remove popup screen */
        XtUnmanageChild(widget_array[k_popup_save_ed]);
	/* Remanage the main graph screen */
/*
	XtUnmanageChild(widget_array[k_main_graph]);
	XtManageChild(widget_array[k_main_graph]);
*/
	/* replot everything */
	mbvelocity_set_controls();
	mbvt_plot();

}

/********************************************************************/
/* Notify callback function for `button_quit'.                      */
/********************************************************************/
 
static void action_quit(w, tag, list)
	Widget w;
	int *tag;
	XmListCallbackStruct *list;
{

	mbvt_quit();

	fprintf(stderr,"\nExiting mbvelocity!\n");
	
	exit(0);

}


/********************************************************************/
/* Notify callback function for `button_load_ok'.                   */
/********************************************************************/

static void open_file_ok(fs, client_data, cbs)
        Widget fs;
        XtPointer client_data;
        XmFileSelectionBoxCallbackStruct *cbs;
{
	/* local definitions */
	int	selected;
	int	status;
        static char *format_text;

        if(!XmStringGetLtoR(cbs->value,charset, &input_file))
        {
                fprintf(stderr,"\n%s input file name\n",input_file);
        }
        else
        {
	   /* get selected filename id and file format id */
	   if (open_type == OPEN_DISPLAY_PROFILE)
		{
		/* open file */
		status = mbvt_open_display_profile(input_file);

	        /* Remove popup screen */
                XtUnmanageChild(widget_array[k_file_sel_board]);

	        /* reset status message */
		if (status == 1)
		  {
		  strcpy(message_str, "Opened Display Sound Velocity Profile: ");
		  strcat(message_str, input_file);
	          status_message = (char *) 
			XmStringLtoRCreate(message_str,"");
	          XtSetArg (arglist[0], XmNlabelString, status_message);
	          XtSetValues (widget_array[k_status_message], arglist, 1);
		  }
		}
	   else if (open_type == OPEN_EDIT_PROFILE)
		{
		/* open file */
		edit_gui = 1;
		status = mbvt_open_edit_profile(input_file);

	        /* Remove popup screen */
                XtUnmanageChild(widget_array[k_file_sel_board]);

	        /* reset status message */
		if (status == 1)
		  {
		   strcpy(message_str, "Opened Editable Sound Velocity Profile: ");
		   strcat(message_str, input_file);
		   ed_message = (char *) 
			XmStringLtoRCreate(message_str,"");
	           XtSetArg (arglist[0], XmNlabelString, ed_message);
	           XtSetValues (widget_array[k_ed_message], arglist, 1);
		  }
		}
	   else if (open_type == OPEN_MULTIBEAM)
		{
		/* get format id value */
        	format_text = XmTextGetString(widget_array[k_mbio_format]);
        	sscanf(format_text, "%d", &format_gui);

		/* open file */
		status = mbvt_open_multibeam_file(input_file,format_gui);

	        /* Remove popup screen */
                XtUnmanageChild(widget_array[k_mb_file_sel_board]);

	        /* reset status message */
		if (status == 1)
		  {
		   strcpy(message_str, "Opened Multibeam Data File: ");
		   strcat(message_str, input_file);
	           mb_message = (char *) 
			XmStringLtoRCreate(message_str,"");
	           XtSetArg (arglist[0], XmNlabelString, mb_message);
	           XtSetValues (widget_array[k_mb_message], arglist, 1);
		  }
		if (status == 1 && edit_gui != 1)
		  {
		   strcpy(message_str, "Opened Editable Sound Velocity Profile: ");
		   strcat(message_str, "no filename");
		   ed_message = (char *) 
			XmStringLtoRCreate(message_str,"");
	           XtSetArg (arglist[0], XmNlabelString, ed_message);
	           XtSetValues (widget_array[k_ed_message], arglist, 1);
		  }
		}

	   if (status != 1)
		XBell(theDisplay,100);

	   /* replot everything */
	   mbvelocity_set_controls();
	   mbvt_plot();

	}

}

/************************************************************/
/*
 * Function to set display profile filenames menu.
 */
/************************************************************/

int mbvelocitytool_set_menu()
{
	char	*files[10];

	/* get list of names */
	mbvt_get_display_names(&ndisplay_gui,files);

}

/************************************************************/
/*
 * Notify callback function for `slider_maxdepth'.
 */
/************************************************************/

static void action_maxdepth(w, tag, scale)
	Widget w;
	int *tag;
	XmScaleCallbackStruct *scale;
{
	maxdepth_gui = scale->value;
	
	mbvt_set_values(edit_gui,ndisplay_gui,maxdepth_gui,
		velrange_gui,resrange_gui);

	/* replot everything */
	mbvelocity_set_controls();

	mbvt_plot();

}

/************************************************************/
/*
 * Menu handler for `menu_close_profile'.
 */
/************************************************************/
 
static void action_menu_close_profile(w, tag, list)
	Widget w;
	int *tag;
	XmListCallbackStruct *list;
{

	/* reset menu */
	mbvelocitytool_set_menu();

	/* replot everything */

	mbvelocity_set_controls();
	mbvt_plot();

}

/************************************************************/
/*
 * Notify callback function for `slider_velrange'.
 */
/************************************************************/

static void action_velrange(w, tag, scale)
	Widget w;
	int *tag;
	XmScaleCallbackStruct *scale;
{
	
	velrange_gui = scale->value;

	mbvt_set_values(edit_gui,ndisplay_gui,maxdepth_gui,
		velrange_gui,resrange_gui);

	/* replot everything */
	mbvelocity_set_controls();

	mbvt_plot();

}

/************************************************************/
/*
 * Event callback function for `canvas_base'.
 */
/************************************************************/
 
static void action_canvas_event(w, data, cbs)
	Widget w;
	XtPointer data;
	XmDrawingAreaCallbackStruct *cbs;
{
	static Position x_loc, y_loc;

	XEvent  *event = cbs->event;

	KeySym keysym;
	int actual;
	static char *pings_to_first_step_text;
	int *x, *y;
	int root_x_return, root_y_return,win_x,win_y;
	unsigned int mask_return;

	int	status;
	
	/* If there is input in the drawing area */
	if (cbs->reason == XmCR_INPUT)
	{
	  /* Check for mouse pressed and not pressed and released. */
	  if(event->xany.type == ButtonPress)
	  {
	      /* If left mouse button is pushed then pick, erase or restore. */
	      if(event->xbutton.button == 1)
	      {
		x_loc = event->xbutton.x;
		y_loc = event->xbutton.y;
	
		status = mbvt_action_mouse_down(
		         	x_loc,y_loc);

	   again:

			status = mbvt_action_mouse_drag(
				x_loc,y_loc);
			if (status == 0) XBell(theDisplay,100);

			status = XQueryPointer(theDisplay,can_xid,
				&root_return,&child_return,&root_x_return,
		      		&root_y_return, &win_x, &win_y, &mask_return);

			x_loc = win_x;
			y_loc = win_y;

			/* If the button is still pressed then read the location */
			/* of the pointer and run the action mouse function again */
			if(mask_return == 256 )
			   goto again;

		  /* replot graph */
		  mbvt_plot();

		}; /* end of left mouse pressed */

	   }; /* end of button press event */

	  /* button release event */
	  if(event->xany.type == ButtonRelease)
	  {
	      if(event->xbutton.button == 1)
	      {
			status = mbvt_action_mouse_up(x_loc, y_loc);
			
			if (status == 0) XBell(theDisplay,100);

	      }; /* end of button 1 button release */
	  }; /* end of button release */
	}; /* end of input to canvas */

} /* end action_canvas_event function */


/************************************************************/
/*
 * Menu handler for `menu_file (Open Multibeam data)'.
 */
/************************************************************/
 
static void open_mb_data(w, tag, list)
	Widget w;
	int *tag;
	XmListCallbackStruct *list;
{

	/* set file type flag */
	open_type = OPEN_MULTIBEAM;

	/* load filenames into scrolling list */
	open_files_count = scandir(".",&open_files,
		open_files_select, alphasort);

}

/************************************************************/
/*
 * Notify callback function for `slider_residual_range'.
 */
/************************************************************/

static void action_residual_range(w, tag, scale)
	Widget w;
	int *tag;
	XmScaleCallbackStruct *scale;
{
	resrange_gui = scale->value;
	
	mbvt_set_values(edit_gui,ndisplay_gui,maxdepth_gui,
		velrange_gui,resrange_gui);

	/* replot everything */
	mbvelocity_set_controls();

	mbvt_plot();
	
}

/************************************************************/
/*
 * Notify callback function for `button_process_hydrosweep'.
 */
/************************************************************/
 
static void action_process_mb(w, tag, list)
	Widget w;
	int *tag;
	XmListCallbackStruct *list;

{
	fprintf(stderr, "\nAbout to process data\n");
	/* process Multibeam data */
	status = mbvt_process_multibeam();
	if (status != 1)
		XBell(theDisplay,100);

	/* replot everything */
	mbvelocity_set_controls();

	mbvt_plot();
	
}
/********************************************************************/
/* User-defined action for multibeam file selection list.                     */
/********************************************************************/
 
static void get_file_selection(w, tag, list)
	Widget w;
	int *tag;
	XmListCallbackStruct *list;
{
	static char *selection_text;
	char	*suffix;
	int	len;
	int	form;
	char	value_text[10];

	/* get selected text */
	selection_text = XmTextGetString(widget_array[k_selection_text]);

	/* get output file */
	if(strlen(selection_text) > 0)
		{
		/* look for MB suffix convention */
		if ((suffix = strstr(selection_text,".mb")) != NULL)
			len = strlen(suffix);

		/* if MB suffix convention used set format */
		if (len >= 4 && len <= 5)
			{
			/* get the file format and set the widget */
			if (sscanf(&suffix[3], "%d", &form) == 1)
				{
				format_gui = form;
				sprintf(value_text,"%2.2d",format_gui);
				XmTextFieldSetString(
				    widget_array[k_mbio_format], 
				    value_text);
				}
			}
		}
}
/************************************************************/
/* END OF "mbvelocity_stubs.c"                              */
/************************************************************/

