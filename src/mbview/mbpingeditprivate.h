/*--------------------------------------------------------------------
 *    The MB-system:	mb3dsoundingsprivate.h	11/19/2007
 *
 *    Copyright (c) 2007-2025 by
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
 * Date:	November 11,  2007
 */

// TODO(schwehr): Fold into mbpingedit_callbacks.c

#ifndef MBVIEW_MBPINGEDITPRIVATE_H_
#define MBVIEW_MBPINGEDITPRIVATE_H_

/* OpenGL Error checking */
/* #define MBP_GETERRORS 1 */

/* OpenGL plotting parameters */
#define LEFT_WIDTH 200
#define LEFT_HEIGHT 30
#define MBP_OPENGL_WIDTH 3.0
#define MBP_OPENGL_ZMIN2D -5.0
#define MBP_OPENGL_ZMAX2D 1000.0
#define MBP_OPENGL_ZMIN3D 100000.0
#define MBP_OPENGL_ZMAX3D 100000000.0

/* OpenGL list IDs */
#define MBP_GLLIST_3DSOUNDINGS 41

#define MBP_PICK_IDIVISION 15
#define MBP_PICK_DIVISION ((double)MBP_PICK_IDIVISION)
#define MBP_PICK_DOWN 1
#define MBP_PICK_MOVE 2
#define MBP_PICK_UP 3

#define MBP_WINDOW_NULL 0
#define MBP_WINDOW_HIDDEN 1
#define MBP_WINDOW_VISIBLE 2
#define MBP_LEFT_WIDTH 40
#define MBP_LEFT_HEIGHT 40
#define MBP_NUM_COLORS 11

#define MBP_OPENGL_WIDTH 3.0
#define MBP_OPENGL_ZMIN2D -5.0
#define MBP_OPENGL_ZMAX2D 1000.0
#define MBP_OPENGL_ZMIN3D 100000.0
#define MBP_OPENGL_ZMAX3D 100000000.0
#define MBP_OPENGL_3D_CONTOUR_OFFSET 0.001
#define MBP_OPENGL_3D_LINE_OFFSET 0.005
#define MBP_OPENGL_ZPROFILE1 -100.0
#define MBP_OPENGL_ZPROFILE2 -200.0

#define MBP_MOUSE_ROTATE 0
#define MBP_MOUSE_PANZOOM 1
#define MBP_EDIT_TOGGLE 0
#define MBP_EDIT_PICK 1
#define MBP_EDIT_ERASE 2
#define MBP_EDIT_RESTORE 3
#define MBP_EDIT_GRAB 4
#define MBP_EDIT_INFO 5
#define MBP_PICK_THRESHOLD 50
#define MBP_ERASE_THRESHOLD 15
#define MBP_EDIT_GRAB_START 0
#define MBP_EDIT_GRAB_MOVE 1
#define MBP_EDIT_GRAB_END 2

#define MBP_VIEW_PROFILES_NONE 0
#define MBP_VIEW_PROFILES_UNFLAGGED 1
#define MBP_VIEW_PROFILES_ALL 2

/* structure to hold instances of mbpingedit windows */
struct mbpingedit_world_struct {
	/* flag if this instance is initialized */
	int init;

	/* function pointers */
	void (*mbpingedit_dismiss_notify)(void);
	void (*mbpingedit_edit_notify)(int ifile, int iping, int ibeam, char beamflag, int flush);
	void (*mbpingedit_info_notify)(int ifile, int iping, int ibeam, char *infostring);

	/* pointer to structure holding data to be rendered */
	struct mbpingedit_struct *soundingdata;

	/* widgets and other Xwindows stuff of interest */
	Widget topLevelShell;
	Widget mainWindow;
	Widget glwmda;
	Mb3dsdgData mb3dsdg;
	Display *dpy;
	Window xid;
	XVisualInfo *vi;
	int glx_init;
#ifdef _WIN32
	HGLRC glx_context;
#else
	GLXContext glx_context;
#endif
	int message_on;

	/* mode parameters */
	int mouse_mode;
	int edit_mode;

	/* cursors */
	Cursor TargetBlackCursor;
	Cursor TargetGreenCursor;
	Cursor TargetRedCursor;
	Cursor TargetBlueCursor;
	Cursor ExchangeBlackCursor;
	Cursor ExchangeGreenCursor;
	Cursor ExchangeRedCursor;
	Cursor FleurBlackCursor;
	Cursor FleurRedCursor;
	Cursor SizingBlackCursor;
	Cursor SizingRedCursor;
	Cursor BoatBlackCursor;
	Cursor BoatRedCursor;
	Cursor WatchBlackCursor;
	Cursor WatchRedCursor;

	/* drawing variables */
	float elevation;
	float azimuth;
	float exageration;
	float elevation_save;
	float azimuth_save;
	float exageration_save;
	Dimension gl_xo;
	Dimension gl_yo;
	Dimension gl_width;
	Dimension gl_height;
	float right;
	float left;
	float top;
	float bottom;
	float aspect_ratio;
	float gl_offset_x;
	float gl_offset_y;
	float gl_offset_x_save;
	float gl_offset_y_save;
	float gl_size;
	float gl_size_save;

	/* button parameters */
	int button1down;
	int button2down;
	int button3down;
	int button_down_x;
	int button_down_y;
	int button_move_x;
	int button_move_y;
	int button_up_x;
	int button_up_y;

	/* edit grab parameters */
	int grab_start_defined;
	int grab_end_defined;
	int grab_start_x;
	int grab_start_y;
	int grab_end_x;
	int grab_end_y;

	/* patch test parameters */
	int irollbias;
	int ipitchbias;
	int iheadingbias;
	int itimelag;

	/* view parameters */
	int view_boundingbox;
	int view_flagged;
	int view_profiles;
};

/* library variables */
/*   note that global mbpingedit variables will only be defined when
     this code is included in mbpingedit_callbacks.c where the
     MBPINGEDITGLOBAL flag is defined - other blocks of code will
     have these variables declared as extern  */
#ifdef MBPINGEDITGLOBAL
#define EXTERNAL
#else
#define EXTERNAL extern
#endif

/* general library global variables */
EXTERNAL int mbp_verbose;
EXTERNAL int mbp_status;
EXTERNAL int mbp_error;
EXTERNAL int mbp_ninstance;
EXTERNAL Widget mbp_parent_widget;
EXTERNAL XtAppContext mbp_app_context;
EXTERNAL int mbp_work_function_set;
EXTERNAL int mbp_timer_count;
EXTERNAL struct mbpingedit_world_struct mbpingedit;

/* global declarations */
#ifdef MBPINGEDITGLOBAL

/* library colortables */
char *mbpingedit_colorname[MBP_NUM_COLORS] = {"Black", "White", "Red", "Yellow", "Green", "Blue-Green", "Blue", "Purple"};

/* extern declarations */
#else

/* library colortables */
extern char *mbpingedit_colorname[MBP_NUM_COLORS];
#endif

/*------------------------------------------------------------------------------*/

void do_mbpingedit_reverse_keys(Widget w, XtPointer client_data, XtPointer call_data);
void do_mbpingedit_event(Widget w, XtPointer client_data, XtPointer call_data);
void do_mbpingedit_number_pings(Widget w, XtPointer client_data, XtPointer call_data);
void do_mbpingedit_forward(Widget w, XtPointer client_data, XtPointer call_data);
void do_mbpingedit_show_time(Widget w, XtPointer client_data, XtPointer call_data);
void do_mbpingedit_mode_restore(Widget w, XtPointer client_data, XtPointer call_data);
void do_mbpingedit_reverse_mouse(Widget w, XtPointer client_data, XtPointer call_data);
void do_mbpingedit_mode_pick(Widget w, XtPointer client_data, XtPointer call_data);
void do_mbpingedit_expose(Widget w, XtPointer client_data, XtPointer call_data);
void do_mbpingedit_unflag_view(Widget w, XtPointer client_data, XtPointer call_data);
int mbpingedit_startup(int verbose, Widget parent, XtAppContext app, int *error);
void do_mbpingedit_mode_toggle(Widget w, XtPointer client_data, XtPointer call_data);
void do_mbpingedit_show_detects(Widget w, XtPointer client_data, XtPointer call_data);
void do_mbpingedit_dismiss(Widget w, XtPointer client_data, XtPointer call_data);
void do_mbpingedit_next_buffer(Widget w, XtPointer client_data, XtPointer call_data);
void do_mbpingedit_scale_x(Widget w, XtPointer client_data, XtPointer call_data);
void do_mbpingedit_scale_y(Widget w, XtPointer client_data, XtPointer call_data);
void do_mbpingedit_check_median_ltrack(Widget w, XtPointer client_data, XtPointer call_data);
void do_mbpingedit_check_median_xtrack(Widget w, XtPointer client_data, XtPointer call_data);
void do_mbpingedit_mode_grab(Widget w, XtPointer client_data, XtPointer call_data);
void do_mbpingedit_mode_erase(Widget w, XtPointer client_data, XtPointer call_data);
void do_mbpingedit_show_flagged(Widget w, XtPointer client_data, XtPointer call_data);
void do_mbpingedit_reverse(Widget w, XtPointer client_data, XtPointer call_data);
void do_mbpingedit_number_step(Widget w, XtPointer client_data, XtPointer call_data);
void do_mbpingedit_set_filters(Widget w, XtPointer client_data, XtPointer call_data);
void do_mbpingedit_view_mode(Widget w, XtPointer client_data, XtPointer call_data);
void do_mbpingedit_reset_filters(Widget w, XtPointer client_data, XtPointer call_data);
void do_mbpingedit_x_interval(Widget w, XtPointer client_data, XtPointer call_data);
void do_mbpingedit_y_interval(Widget w, XtPointer client_data, XtPointer call_data);
void do_mbpingedit_mode_info(Widget w, XtPointer client_data, XtPointer call_data);
void do_mbpingedit_unflag_all(Widget w, XtPointer client_data, XtPointer call_data);
void do_mbpingedit_flag_view(Widget w, XtPointer client_data, XtPointer call_data);

void BxUnmanageCB(Widget w, XtPointer client, XtPointer call);
void BxManageCB(Widget w, XtPointer client, XtPointer call);
void BxPopupCB(Widget w, XtPointer client, XtPointer call);
XtPointer BX_CONVERT(Widget w, char *from_string, char *to_type, int to_size, Boolean *success);
void BxExitCB(Widget w, XtPointer client, XtPointer call);
void BxSetValuesCB(Widget w, XtPointer client, XtPointer call);

#endif  // MBVIEW_MBPINGEDITPRIVATE_H_
