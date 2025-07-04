/*--------------------------------------------------------------------
 *    The MB-system:	mb3dsoundingsprivate.h	5/25/2007
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
 * Date:	May 25,  2007
 */

// TODO(schwehr): Fold into mb3dsoundings_callbacks.c?

#ifndef MBVIEW_MB3DSOUNDINGSPRIVATE_H_
#define MBVIEW_MB3DSOUNDINGSPRIVATE_H_

/* OpenGL Error checking */
/* #define MBS_GETERRORS 1 */

/* OpenGL list IDs */
#define MBS_GLLIST_3DSOUNDINGS 41

/* OpenGL plotting parameters */
#define LEFT_WIDTH 200
#define LEFT_HEIGHT 30
#define MBS_PICK_IDIVISION 15
#define MBS_PICK_DIVISION ((double)MBS_PICK_IDIVISION)
#define MBS_PICK_DOWN 1
#define MBS_PICK_MOVE 2
#define MBS_PICK_UP 3

#define MBS_WINDOW_NULL 0
#define MBS_WINDOW_HIDDEN 1
#define MBS_WINDOW_VISIBLE 2
#define MBS_LEFT_WIDTH 40
#define MBS_LEFT_HEIGHT 40
#define MBS_NUM_COLORS 11

#define MBS_OPENGL_WIDTH 3.0
#define MBS_OPENGL_ZMIN2D -5.0
#define MBS_OPENGL_ZMAX2D 1000.0
#define MBS_OPENGL_ZMIN3D 100000.0
#define MBS_OPENGL_ZMAX3D 100000000.0
#define MBS_OPENGL_3D_CONTOUR_OFFSET 0.001
#define MBS_OPENGL_3D_LINE_OFFSET 0.005
#define MBS_OPENGL_ZPROFILE1 -100.0
#define MBS_OPENGL_ZPROFILE2 -200.0

#define MBS_MOUSE_ROTATE 0
#define MBS_MOUSE_PANZOOM 1
#define MBS_EDIT_TOGGLE 0
#define MBS_EDIT_PICK 1
#define MBS_EDIT_ERASE 2
#define MBS_EDIT_RESTORE 3
#define MBS_EDIT_GRAB 4
#define MBS_EDIT_INFO 5
#define MBS_PICK_THRESHOLD 50
#define MBS_ERASE_THRESHOLD 15
#define MBS_EDIT_GRAB_START 0
#define MBS_EDIT_GRAB_MOVE 1
#define MBS_EDIT_GRAB_END 2

#define MBS_VIEW_PROFILES_NONE 0
#define MBS_VIEW_PROFILES_UNFLAGGED 1
#define MBS_VIEW_PROFILES_ALL 2

#define MBS_VIEW_COLOR_FLAG 0
#define MBS_VIEW_COLOR_TOPO 1
#define MBS_VIEW_COLOR_AMP 2

/* structure to hold instances of mb3dsoundings windows */
struct mb3dsoundings_world_struct {
	/* flag if this instance is initialized */
	int init;

	/* function pointers */
	void (*mb3dsoundings_dismiss_notify)(void);
	void (*mb3dsoundings_edit_notify)(int ifile, int iping, int ibeam, char beamflag, int flush);
	void (*mb3dsoundings_info_notify)(int ifile, int iping, int ibeam, char *infostring);
	void (*mb3dsoundings_bias_notify)(double rollbias, double pitchbias, double headingbias, double timelag, double snell);
	void (*mb3dsoundings_biasapply_notify)(double rollbias, double pitchbias, double headingbias, double timelag, double snell);
	void (*mb3dsoundings_flagsparsevoxels_notify)(int sizemultiplier, int nsoundingthreshold);
	void (*mb3dsoundings_colorsoundings_notify)(int color);
	void (*mb3dsoundings_optimizebiasvalues_notify)(int mode, double *rollbias, double *pitchbias, double *headingbias,
	                                                double *timelag, double *snell);

	/* pointer to structure holding data to be rendered */
	struct mb3dsoundings_struct *soundingdata;

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
	int keyreverse_mode;
	int mousereverse_mode;

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
	bool grab_start_defined;
	bool grab_end_defined;
	int grab_start_x;
	int grab_start_y;
	int grab_end_x;
	int grab_end_y;

	/* patch test parameters */
	int irollbias;
	int ipitchbias;
	int iheadingbias;
	int itimelag;
	int isnell;

	/* view parameters */
	bool view_boundingbox;
	bool view_flagged;
  bool view_secondary;
	int view_profiles;
	bool view_scalewithflagged;
	int view_color;

	/* last sounding edited */
	bool last_sounding_defined;
	int last_sounding_edited;
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
EXTERNAL int mbs_verbose;
EXTERNAL int mbs_status;
EXTERNAL int mbs_error;
EXTERNAL int mbs_ninstance;
EXTERNAL Widget mbs_parent_widget;
EXTERNAL XtAppContext mbs_app_context;
EXTERNAL int mbs_work_function_set;
EXTERNAL int mbs_timer_count;
EXTERNAL struct mb3dsoundings_world_struct mb3dsoundings;
EXTERNAL int key_g_down = 0;
EXTERNAL int key_z_down = 0;
EXTERNAL int key_s_down = 0;
EXTERNAL int key_a_down = 0;
EXTERNAL int key_d_down = 0;

/* global declarations */
#ifdef MB3DSOUNDINGSGLOBAL

/* library colortables */
char *mb3dsoundings_colorname[MBS_NUM_COLORS] = {"Black", "White", "Red", "Yellow", "Green", "Blue-Green", "Blue", "Purple"};

/* extern declarations */
#else

/* library colortables */
extern char *mb3dsoundings_colorname[MBS_NUM_COLORS];
#endif

/*--------------------------------------------------------------------*/

int mb3dsoundings_startup(int verbose, Widget parent, XtAppContext app, int *error);
int mb3dsoundings_updatecursor(void);
int mb3dsoundings_updategui(void);
int mb3dsoundings_updatestatus(void);
int mb3dsoundings_reset(void);
int mb3dsoundings_reset_glx(void);
void do_mb3dsdg_resize(Widget w, XtPointer client_data, XtPointer call_data);
void do_mb3dsdg_dismiss(Widget w, XtPointer client_data, XtPointer call_data);
void do_mb3dsdg_mouse_pick(Widget w, XtPointer client_data, XtPointer call_data);
void do_mb3dsdg_mouse_grab(Widget w, XtPointer client_data, XtPointer call_data);
void do_mb3dsdg_mouse_erase(Widget w, XtPointer client_data, XtPointer call_data);
void do_mb3dsdg_mouse_toggle(Widget w, XtPointer client_data, XtPointer call_data);
void do_mb3dsdg_mouse_restore(Widget w, XtPointer client_data, XtPointer call_data);
void do_mb3dsdg_mouse_info(Widget w, XtPointer client_data, XtPointer call_data);
void do_mb3dsdg_input(Widget w, XtPointer client_data, XtPointer call_data);
void do_mb3dsdg_glwda_expose(Widget w, XtPointer client_data, XtPointer call_data);
void do_mb3dsdg_glwda_input(Widget w, XtPointer client_data, XtPointer call_data);
void do_mb3dsdg_glwda_resize(Widget w, XtPointer client_data, XtPointer call_data);
int mb3dsoundings_setzscale(int verbose, int *error);
int mb3dsoundings_scale(int verbose, int *error);
int mb3dsoundings_scalez(int verbose, int *error);
int mb3dsoundings_pick(int x, int y);
int mb3dsoundings_eraserestore(int x, int y);
int mb3dsoundings_grab(int x, int y, int grabmode);
int mb3dsoundings_info(int x, int y);
void do_mb3dsdg_rollbias(Widget w, XtPointer client_data, XtPointer call_data);
void do_mb3dsdg_pitchbias(Widget w, XtPointer client_data, XtPointer call_data);
void do_mb3dsdg_headingbias(Widget w, XtPointer client_data, XtPointer call_data);
void do_mb3dsdg_timelag(Widget w, XtPointer client_data, XtPointer call_data);
void do_mb3dsdg_snell(Widget w, XtPointer client_data, XtPointer call_data);
void do_mb3dsdg_view_flagged(Widget w, XtPointer client_data, XtPointer call_data);
void do_mb3dsdg_view_secondary(Widget w, XtPointer client_data, XtPointer call_data);
void do_mb3dsdg_view_noprofile(Widget w, XtPointer client_data, XtPointer call_data);
void do_mb3dsdg_view_goodprofile(Widget w, XtPointer client_data, XtPointer call_data);
void do_mb3dsdg_view_allprofile(Widget w, XtPointer client_data, XtPointer call_data);
void do_mb3dsdg_resetview(Widget w, XtPointer client_data, XtPointer call_data);
void do_mb3dsdg_action_applybias(Widget w, XtPointer client_data, XtPointer call_data);
void do_mb3dsdg_action_flagsparsevoxels_A(Widget w, XtPointer client_data, XtPointer call_data);
void do_mb3dsdg_action_flagsparsevoxels_B(Widget w, XtPointer client_data, XtPointer call_data);
void do_mb3dsdg_action_flagsparsevoxels_C(Widget w, XtPointer client_data, XtPointer call_data);
void do_mb3dsdg_action_flagsparsevoxels_D(Widget w, XtPointer client_data, XtPointer call_data);
void do_mb3dsdg_action_flagsparsevoxels_E(Widget w, XtPointer client_data, XtPointer call_data);
void do_mb3dsdg_action_flagsparsevoxels_F(Widget w, XtPointer client_data, XtPointer call_data);
void do_mb3dsdg_action_colorsoundingsblack(Widget w, XtPointer client_data, XtPointer call_data);
void do_mb3dsdg_action_colorsoundingsred(Widget w, XtPointer client_data, XtPointer call_data);
void do_mb3dsdg_action_colorsoundingsyellow(Widget w, XtPointer client_data, XtPointer call_data);
void do_mb3dsdg_action_colorsoundingsgreen(Widget w, XtPointer client_data, XtPointer call_data);
void do_mb3dsdg_action_colorsoundingsbluegreen(Widget w, XtPointer client_data, XtPointer call_data);
void do_mb3dsdg_action_colorsoundingsblue(Widget w, XtPointer client_data, XtPointer call_data);
void do_mb3dsdg_action_colorsoundingspurple(Widget w, XtPointer client_data, XtPointer call_data);
void do_mb3dsdg_action_optimizebiasvalues_r(Widget w, XtPointer client_data, XtPointer call_data);
void do_mb3dsdg_action_optimizebiasvalues_p(Widget w, XtPointer client_data, XtPointer call_data);
void do_mb3dsdg_action_optimizebiasvalues_h(Widget w, XtPointer client_data, XtPointer call_data);
void do_mb3dsdg_action_optimizebiasvalues_rp(Widget w, XtPointer client_data, XtPointer call_data);
void do_mb3dsdg_action_optimizebiasvalues_rph(Widget w, XtPointer client_data, XtPointer call_data);
void do_mb3dsdg_action_optimizebiasvalues_t(Widget w, XtPointer client_data, XtPointer call_data);
void do_mb3dsdg_action_optimizebiasvalues_s(Widget w, XtPointer client_data, XtPointer call_data);
void do_mb3dsdg_view_boundingbox(Widget w, XtPointer client_data, XtPointer call_data);
void do_mb3dsdg_view_scalewithflagged(Widget w, XtPointer client_data, XtPointer call_data);
void do_mb3dsdg_view_colorbyflag(Widget w, XtPointer client_data, XtPointer call_data);
void do_mb3dsdg_view_colorbytopo(Widget w, XtPointer client_data, XtPointer call_data);
void do_mb3dsdg_view_colorbyamp(Widget w, XtPointer client_data, XtPointer call_data);
void do_mb3dsdg_view_reset(Widget w, XtPointer client_data, XtPointer call_data);
void do_mb3dsdg_mouse_panzoom(Widget w, XtPointer client_data, XtPointer call_data);
void do_mb3dsdg_mouse_rotate(Widget w, XtPointer client_data, XtPointer call_data);
int mb3dsoundings_updatemodetoggles(void);
int mb3dsoundings_bad_ping(void);
int mb3dsoundings_good_ping(void);
int mb3dsoundings_left_ping(void);
int mb3dsoundings_right_ping(void);
int mb3dsoundings_flag_view(void);
int mb3dsoundings_unflag_view(void);
int mb3dsoundings_zero_ping(void);

void BxUnmanageCB(Widget w, XtPointer client, XtPointer call);
void BxManageCB(Widget w, XtPointer client, XtPointer call);
void BxPopupCB(Widget w, XtPointer client, XtPointer call);
XtPointer BX_CONVERT(Widget w, char *from_string, char *to_type, int to_size, Boolean *success);
void BxExitCB(Widget w, XtPointer client, XtPointer call);
void BxSetValuesCB(Widget w, XtPointer client, XtPointer call);

#endif  // MBVIEW_MB3DSOUNDINGSPRIVATE_H_
