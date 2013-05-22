/*--------------------------------------------------------------------
 *    The MB-system:	mb3dsoundingsprivate.h	9/24/2003
 *    $Id$
 *
 *    Copyright (c) 2007-2013 by
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
 *
 * Author:	D. W. Caress
 * Date:	May 25,  2007
 *
 * $Log: mb3dsoundingsprivate.h,v $
 * Revision 5.4  2008/11/16 21:51:18  caress
 * Updating all recent changes, including time lag analysis using mbeditviz and improvements to the mbgrid footprint gridding algorithm.
 *
 * Revision 5.3  2007/11/16 17:26:56  caress
 * Progress on MBeditviz
 *
 * Revision 5.2  2007/10/08 16:32:08  caress
 * Code status as of 8 October 2007.
 *
 * Revision 5.1  2007/07/03 17:35:54  caress
 * Working on MBeditviz.
 *
 * Revision 5.0  2007/06/17 23:16:14  caress
 * Added MBeditviz.
 *
 *
 */

/*--------------------------------------------------------------------*/

/* OpenGL Error checking */
/* #define MBS_GETERRORS 1 */

/* OpenGL plotting parameters */
#define LEFT_WIDTH 200
#define LEFT_HEIGHT 30
#define MBS_OPENGL_WIDTH 3.0
#define MBS_OPENGL_ZMIN2D -5.0
#define MBS_OPENGL_ZMAX2D 1000.0
#define MBS_OPENGL_ZMIN3D 100000.0
#define MBS_OPENGL_ZMAX3D 100000000.0

/* OpenGL list IDs */
#define MBS_GLLIST_3DSOUNDINGS  41

#define MBS_PICK_IDIVISION 15
#define MBS_PICK_DIVISION ((double)MBS_PICK_IDIVISION)
#define MBS_PICK_DOWN	1
#define MBS_PICK_MOVE	2
#define MBS_PICK_UP	3

#define MBS_WINDOW_NULL 	0
#define MBS_WINDOW_HIDDEN 	1
#define MBS_WINDOW_VISIBLE 	2
#define MBS_LEFT_WIDTH		40
#define MBS_LEFT_HEIGHT		40
#define	MBS_NUM_COLORS   11

#define MBS_OPENGL_WIDTH 3.0
#define MBS_OPENGL_ZMIN2D -5.0
#define MBS_OPENGL_ZMAX2D 1000.0
#define MBS_OPENGL_ZMIN3D 100000.0
#define MBS_OPENGL_ZMAX3D 100000000.0
#define MBS_OPENGL_3D_CONTOUR_OFFSET 0.001
#define MBS_OPENGL_3D_LINE_OFFSET 0.005
#define MBS_OPENGL_ZPROFILE1 -100.0
#define MBS_OPENGL_ZPROFILE2 -200.0

#define	MBS_MOUSE_ROTATE	0
#define	MBS_MOUSE_PANZOOM	1
#define MBS_EDIT_TOGGLE		0
#define MBS_EDIT_PICK		1
#define MBS_EDIT_ERASE		2
#define MBS_EDIT_RESTORE	3
#define MBS_EDIT_GRAB		4
#define MBS_EDIT_INFO		5
#define MBS_PICK_THRESHOLD	50
#define MBS_ERASE_THRESHOLD	15
#define MBS_EDIT_GRAB_START	0
#define MBS_EDIT_GRAB_MOVE	1
#define MBS_EDIT_GRAB_END	2

#define	MBS_VIEW_PROFILES_NONE		0
#define	MBS_VIEW_PROFILES_UNFLAGGED	1
#define	MBS_VIEW_PROFILES_ALL		2


/* structure to hold instances of mb3dsoundings windows */
struct mb3dsoundings_world_struct
    {
    /* flag if this instance is initialized */
    int			init;

    /* function pointers */
    void (*mb3dsoundings_dismiss_notify)();
    void (*mb3dsoundings_edit_notify)(int ifile, int iping, int ibeam, char beamflag, int flush);
    void (*mb3dsoundings_info_notify)(int ifile, int iping, int ibeam, char *infostring);
    void (*mb3dsoundings_bias_notify)(double rollbias, double pitchbias, double headingbias, double timelag);
    void (*mb3dsoundings_biasapply_notify)(double rollbias, double pitchbias, double headingbias, double timelag);

    /* pointer to structure holding data to be rendered */
    struct mb3dsoundings_struct *soundingdata;

    /* widgets and other Xwindows stuff of interest */
    Widget		topLevelShell;
    Widget		mainWindow;
    Widget		glwmda;
    Mb3dsdgData		mb3dsdg;
    Display		*dpy;
    Window		xid;
    XVisualInfo 	*vi;
    int			glx_init;
    GLXContext		glx_context;
    int			message_on;

    /* mode parameters */
    int			mouse_mode;
    int			edit_mode;
    int			keyreverse_mode;
    int			mousereverse_mode;

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
    int	grab_start_x;
    int	grab_start_y;
    int	grab_end_x;
    int	grab_end_y;

    /* patch test parameters */
    int	irollbias;
    int	ipitchbias;
    int	iheadingbias;
    int	itimelag;

    /* view parameters */
    int	view_boundingbox;
    int	view_flagged;
    int	view_profiles;
    int	view_scalewithflagged;

    /* last sounding edited */
    int	last_sounding_defined;
    int	last_sounding_edited;
    };

/* library variables */
/*   note that global mb3dsoundings variables will only be defined when
     this code is included in mb3dsoundings_callbacks.c where the
     MB3DSOUNDINGSGLOBAL flag is defined - other blocks of code will
     have these variables declared as extern  */
#ifdef MB3DSOUNDINGSGLOBAL
#define EXTERNAL
#else
#define EXTERNAL extern
#endif

/* general library global variables */
EXTERNAL int	mbs_verbose;
EXTERNAL int	mbs_status;
EXTERNAL int	mbs_error;
EXTERNAL int	mbs_ninstance;
EXTERNAL Widget	mbs_parent_widget;
EXTERNAL XtAppContext	mbs_app_context;
EXTERNAL int	mbs_work_function_set;
EXTERNAL int	mbs_timer_count;
EXTERNAL struct mb3dsoundings_world_struct mb3dsoundings;
EXTERNAL int key_g_down = 0;
EXTERNAL int key_z_down = 0;
EXTERNAL int key_s_down = 0;
EXTERNAL int key_a_down = 0;
EXTERNAL int key_d_down = 0;

/* global declarations */
#ifdef MB3DSOUNDINGSGLOBAL

/* library colortables */
char	*mb3dsoundings_colorname[MBS_NUM_COLORS] =
		{ "Black",
		  "White",
		  "Red",
		  "Yellow",
		  "Green",
		  "Blue-Green",
		  "Blue",
		  "Purple" };

/* extern declarations */
#else

/* library colortables */
extern char	*mb3dsoundings_colorname[MBS_NUM_COLORS];
#endif

/*--------------------------------------------------------------------*/

int mb3dsoundings_startup(int verbose, Widget parent, XtAppContext app, int *error);
int mb3dsoundings_updategui();
int mb3dsoundings_updatestatus();
int mb3dsoundings_reset();
int mb3dsoundings_reset_glx();
void do_mb3dsdg_resize( Widget w, XtPointer client_data, XtPointer call_data);
void do_mb3dsdg_dismiss( Widget w, XtPointer client_data, XtPointer call_data);
void do_mb3dsdg_mouse_pick( Widget w, XtPointer client_data, XtPointer call_data);
void do_mb3dsdg_mouse_grab( Widget w, XtPointer client_data, XtPointer call_data);
void do_mb3dsdg_mouse_erase( Widget w, XtPointer client_data, XtPointer call_data);
void do_mb3dsdg_mouse_toggle( Widget w, XtPointer client_data, XtPointer call_data);
void do_mb3dsdg_mouse_restore( Widget w, XtPointer client_data, XtPointer call_data);
void do_mb3dsdg_mouse_info( Widget w, XtPointer client_data, XtPointer call_data);
void do_mb3dsdg_input( Widget w, XtPointer client_data, XtPointer call_data);
void do_mb3dsdg_glwda_expose( Widget w, XtPointer client_data, XtPointer call_data);
void do_mb3dsdg_glwda_input( Widget w, XtPointer client_data, XtPointer call_data);
void do_mb3dsdg_glwda_resize( Widget w, XtPointer client_data, XtPointer call_data);
int mb3dsoundings_setzscale(int verbose, int *error);
int mb3dsoundings_scale(int verbose, int *error);
int mb3dsoundings_scalez(int verbose, int *error);
int mb3dsoundings_pick(int x, int y);
int mb3dsoundings_eraserestore(int x, int y);
int mb3dsoundings_grab(int x, int y, int grabmode);
int mb3dsoundings_info(int x, int y);
void do_mb3dsdg_rollbias( Widget w, XtPointer client_data, XtPointer call_data);
void do_mb3dsdg_pitchbias( Widget w, XtPointer client_data, XtPointer call_data);
void do_mb3dsdg_headingbias( Widget w, XtPointer client_data, XtPointer call_data);
void do_mb3dsdg_timelag( Widget w, XtPointer client_data, XtPointer call_data);
void do_mb3dsdg_view_flagged( Widget w, XtPointer client_data, XtPointer call_data);
void do_mb3dsdg_view_noprofile( Widget w, XtPointer client_data, XtPointer call_data);
void do_mb3dsdg_view_goodprofile( Widget w, XtPointer client_data, XtPointer call_data);
void do_mb3dsdg_view_allprofile( Widget w, XtPointer client_data, XtPointer call_data);
void do_mb3dsdg_action_applybias( Widget w, XtPointer client_data, XtPointer call_data);
void do_mb3dsdg_view_boundingbox( Widget w, XtPointer client_data, XtPointer call_data);
void do_mb3dsdg_view_scalewithflagged( Widget w, XtPointer client_data, XtPointer call_data);
void do_mb3dsdg_view_reset( Widget w, XtPointer client_data, XtPointer call_data);
void do_mb3dsdg_mouse_panzoom( Widget w, XtPointer client_data, XtPointer call_data);
void do_mb3dsdg_mouse_rotate( Widget w, XtPointer client_data, XtPointer call_data);
int mb3dsoundings_updatemodetoggles();
int mb3dsoundings_bad_ping();
int mb3dsoundings_good_ping();
int mb3dsoundings_left_ping();
int mb3dsoundings_right_ping();
int mb3dsoundings_flag_view();
int mb3dsoundings_unflag_view();
int mb3dsoundings_zero_ping();

/*--------------------------------------------------------------------*/
