/*--------------------------------------------------------------------
 *    The MB-system:	mbviewprivate.h	9/24/2003
 *
 *    Copyright (c) 2003-2025 by
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
 * Date:	September 24,  2003
 */

#ifndef MBVIEW_MBVIEWPRIVATE_H_
#define MBVIEW_MBVIEWPRIVATE_H_

/* OpenGL Error checking */
/* #define MBV_GET_GLX_ERRORS	1 */
/* #define MBV_DEBUG_GLX		1 */

/* OpenGL plotting parameters */
#define LEFT_WIDTH 200
#define LEFT_HEIGHT 30
#define MBV_OPENGL_WIDTH 3.0
#define MBV_OPENGL_ZMIN2D -5.0
#define MBV_OPENGL_ZMAX2D 1000.0
#define MBV_OPENGL_ZMIN3D 100000.0
#define MBV_OPENGL_ZMAX3D 100000000.0
#define MBV_OPENGL_3D_CONTOUR_OFFSET 0.001
#define MBV_OPENGL_3D_LINE_OFFSET 0.005
#define MBV_OPENGL_ZPROFILE1 -100.0
#define MBV_OPENGL_ZPROFILE2 -200.0

/* OpenGL list IDs */
#define MBV_GLLIST_SITESMALL (3 * MBV_MAX_WINDOWS + 0)
#define MBV_GLLIST_SITELARGE (3 * MBV_MAX_WINDOWS + 1)
#define MBV_GLLIST_ROUTESMALL (3 * MBV_MAX_WINDOWS + 2)
#define MBV_GLLIST_ROUTELARGE (3 * MBV_MAX_WINDOWS + 3)
#define MBV_GLLIST_VECTORBALL (3 * MBV_MAX_WINDOWS + 4)

#define MBV_REZ_NONE 0
#define MBV_REZ_LOW 1
#define MBV_REZ_HIGH 2
#define MBV_REZ_FULL 3
#define MBV_BOUNDSFREQUENCY 25
#define MBV_EVENTCHECKCOARSENESS 5

#define MBV_NUMBACKGROUNDCALC 500
#define MBV_BACKGROUND_NONE 0
#define MBV_BACKGROUND_ZSCALE 1
#define MBV_BACKGROUND_COLOR 2
#define MBV_BACKGROUND_FULLPLOT 3

#define MBV_PICK_IDIVISION 15
#define MBV_PICK_DIVISION ((double)MBV_PICK_IDIVISION)
#define MBV_PICK_DOWN 1
#define MBV_PICK_MOVE 2
#define MBV_PICK_UP 3
#define MBV_AREALENGTH_DOWN 1
#define MBV_AREALENGTH_MOVE 2
#define MBV_AREALENGTH_UP 3
#define MBV_AREAASPECT_CHANGE 4
#define MBV_AREAASPECT_UP 5
#define MBV_REGION_DOWN 1
#define MBV_REGION_MOVE 2
#define MBV_REGION_UP 3

#define MBV_WINDOW_NULL 0
#define MBV_WINDOW_HIDDEN 1
#define MBV_WINDOW_VISIBLE 2

#define MBV_RAW_HISTOGRAM_DIM 1000

#define MBV_WINDOW_HEIGHT_THRESHOLD 700

#define MBV_NUM_COLORS 11

#define MBV_NUM_ACTIONS 50

/* Spheroid parameters */
#define MBV_SPHEROID_RADIUS 6371000.0

/* structure to hold single mbview windows */
struct mbview_shared_struct {
	/* flags if list windows are initialized */
	int init_sitelist;
	int init_routelist;
	int init_navlist;

	/* global lon lat print style */
	int lonlatstyle;

	/* pointer to structure holding global data */
	struct mbview_shareddata_struct shareddata;

	/* widgets and other Xwindows stuff of interest */
	Widget topLevelShell_sitelist;
	Widget mainWindow_sitelist;
	MB3DSiteListData mb3d_sitelist;
	Widget topLevelShell_routelist;
	Widget mainWindow_routelist;
	MB3DRouteListData mb3d_routelist;
	Widget topLevelShell_navlist;
	Widget mainWindow_navlist;
	MB3DNavListData mb3d_navlist;
};

/* structure to hold instances of mbview windows */
struct mbview_world_struct {
	/* flag if this instance is initialized */
	int init;

	/* pointer to structure holding data to be rendered */
	struct mbview_struct data;

	/* widgets and other Xwindows stuff of interest */
	Widget topLevelShell;
	Widget mainWindow;
	Widget glwmda;
	MB3DViewData mb3dview;
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
	int plot_recursion;
	int plot_done;
	int plot_interrupt_allowed;
	int naction;
	int actionsensitive[MBV_NUM_ACTIONS];
	Widget pushButton_action[MBV_NUM_ACTIONS];
	Widget prglwmda;
	XVisualInfo *prvi;
	int prglx_init;
#ifdef _WIN32
	HGLRC prglx_context;
#else
	GLXContext prglx_context;
#endif
	float praspect_ratio;

	/* cursors */
	Cursor TargetBlackCursor;
	Cursor TargetGreenCursor;
	Cursor TargetRedCursor;
	Cursor FleurBlackCursor;
	Cursor FleurRedCursor;
	Cursor SizingBlackCursor;
	Cursor SizingRedCursor;
	Cursor BoatBlackCursor;
	Cursor BoatRedCursor;
	Cursor WatchBlackCursor;
	Cursor WatchRedCursor;

	/* projections */
	int primary_pj_init;
	void *primary_pjptr;
	int secondary_pj_init;
	void *secondary_pjptr;
	int display_pj_init;
	void *display_pjptr;
	double mtodeglon;
	double mtodeglat;
	double sphere_reflon;
	double sphere_reflat;
	double sphere_refx;
	double sphere_refy;
	double sphere_refz;
	double sphere_eulerforward[9];
	double sphere_eulerreverse[9];

	/* drawing variables */
	Dimension gl_width;
	Dimension gl_height;
	float gl_xmin;
	float right;
	float left;
	float top;
	float bottom;
	float aspect_ratio;
	int projected;
	int globalprojected;
	int lastdrawrez;
	int viewboundscount;
	int zscaledonecount;
	int colordonecount;
	int contourlorez;
	int contourhirez;
	int contourfullrez;

	/* color and shade variables */
	double min;
	double max;
	double minuse;
	double maxuse;
	double mag2;
	double illum_x;
	double illum_y;
	double illum_z;
	double intensity;
	int colortable;
	int colortable_mode;
	float *colortable_red;
	float *colortable_blue;
	float *colortable_green;
	int shade_mode;
	double sign;
	int primary_histogram_set;
	int primaryslope_histogram_set;
	int secondary_histogram_set;
	float primary_histogram[3 * MBV_NUM_COLORS];
	float primaryslope_histogram[3 * MBV_NUM_COLORS];
	float secondary_histogram[3 * MBV_NUM_COLORS];

	/* grid display bounds */
	double xmin;
	double xmax;
	double ymin;
	double ymax;
	double xorigin;
	double yorigin;
	double zorigin;
	double scale;

	/* relevant mbio defaults */
	int lonflip;
	double timegap;

	float offset2d_x;
	float offset2d_y;
	float offset2d_x_save;
	float offset2d_y_save;
	float size2d;
	float size2d_save;
	float offset3d_x;
	float offset3d_y;
	float offset3d_z;
	float viewoffset3d_z;
	float offset3d_x_save;
	float offset3d_y_save;
	float offset3d_z_save;
	float viewoffset3d_z_save;
	float areaaspect;
	float areaaspect_save;
	double exageration_save;
	double modelelevation3d_save;
	double modelazimuth3d_save;
	double viewelevation3d_save;
	double viewazimuth3d_save;
	double illuminate_magnitude_save;
	double illuminate_elevation_save;
	double illuminate_azimuth_save;
	double slope_magnitude_save;
	double overlay_shade_magnitude_save;

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
};

/* library variables */
/*   note that global mbview variables will only be defined when
     this code is included in mbview_callbacks.c where the
     MBVIEWGLOBAL flag is defined - other blocks of code will
     have these variables declared as extern  */
#ifdef MBVIEWGLOBAL
#define MBVIEW_EXTERNAL
#else
#define MBVIEW_EXTERNAL extern
#endif

/* general library global variables */
MBVIEW_EXTERNAL int mbv_verbose;
MBVIEW_EXTERNAL int mbv_ninstance;
MBVIEW_EXTERNAL Widget parent_widget;
MBVIEW_EXTERNAL XtAppContext app_context;
MBVIEW_EXTERNAL int work_function_enabled;
MBVIEW_EXTERNAL int work_function_set;
MBVIEW_EXTERNAL unsigned long timer_timeout_time;
MBVIEW_EXTERNAL int timer_timeout_count;
MBVIEW_EXTERNAL int timer_count;
MBVIEW_EXTERNAL struct mbview_world_struct mbviews[MBV_MAX_WINDOWS];
MBVIEW_EXTERNAL struct mbview_shared_struct shared;
MBVIEW_EXTERNAL char *mbsystem_library_name;

/* global declarations */
#ifdef MBVIEWGLOBAL

/* library colortables */
float colortable_haxby_red[MBV_NUM_COLORS] = {0.950, 1.000, 1.000, 1.000, 0.941, 0.804, 0.541, 0.416, 0.196, 0.157, 0.145};
float colortable_haxby_green[MBV_NUM_COLORS] = {0.950, 0.729, 0.631, 0.741, 0.925, 1.000, 0.925, 0.922, 0.745, 0.498, 0.224};
float colortable_haxby_blue[MBV_NUM_COLORS] = {0.950, 0.522, 0.267, 0.341, 0.475, 0.635, 0.682, 1.000, 1.000, 0.984, 0.686};
float colortable_bright_red[MBV_NUM_COLORS] = {1.000, 1.000, 1.000, 1.000, 0.500, 0.000, 0.000, 0.000, 0.000, 0.500, 1.000};
float colortable_bright_green[MBV_NUM_COLORS] = {0.000, 0.250, 0.500, 1.000, 1.000, 1.000, 1.000, 0.500, 0.000, 0.000, 0.000};
float colortable_bright_blue[MBV_NUM_COLORS] = {0.000, 0.000, 0.000, 0.000, 0.000, 0.000, 1.000, 1.000, 1.000, 1.000, 1.000};
float colortable_muted_red[MBV_NUM_COLORS] = {0.784, 0.761, 0.702, 0.553, 0.353, 0.000, 0.000, 0.000, 0.000, 0.353, 0.553};
float colortable_muted_green[MBV_NUM_COLORS] = {0.000, 0.192, 0.353, 0.553, 0.702, 0.784, 0.553, 0.353, 0.000, 0.000, 0.000};
float colortable_muted_blue[MBV_NUM_COLORS] = {0.000, 0.000, 0.000, 0.000, 0.000, 0.000, 0.553, 0.702, 0.784, 0.702, 0.553};
float colortable_redtoblue_red[MBV_NUM_COLORS] =   {1.000, 1.000, 1.000, 1.000, 1.000, 0.750, 0.500, 0.000, 0.000, 0.000, 0.000};
float colortable_redtoblue_green[MBV_NUM_COLORS] = {0.000, 0.250, 0.500, 0.750, 1.000, 1.000, 1.000, 1.000, 1.000, 0.500, 0.000};
float colortable_redtoblue_blue[MBV_NUM_COLORS] =  {0.000, 0.000, 0.000, 0.000, 0.000, 0.000, 0.000, 0.000, 1.000, 1.000, 1.000};
float colortable_gray_red[MBV_NUM_COLORS] = {0.000, 0.100, 0.200, 0.300, 0.400, 0.500, 0.600, 0.700, 0.800, 0.900, 1.000};
float colortable_gray_green[MBV_NUM_COLORS] = {0.000, 0.100, 0.200, 0.300, 0.400, 0.500, 0.600, 0.700, 0.800, 0.900, 1.000};
float colortable_gray_blue[MBV_NUM_COLORS] = {0.000, 0.100, 0.200, 0.300, 0.400, 0.500, 0.600, 0.700, 0.800, 0.900, 1.000};
float colortable_flat_red[MBV_NUM_COLORS] = {0.500, 0.500, 0.500, 0.500, 0.500, 0.500, 0.500, 0.500, 0.500, 0.500, 0.500};
float colortable_flat_green[MBV_NUM_COLORS] = {0.500, 0.500, 0.500, 0.500, 0.500, 0.500, 0.500, 0.500, 0.500, 0.500, 0.500};
float colortable_flat_blue[MBV_NUM_COLORS] = {0.500, 0.500, 0.500, 0.500, 0.500, 0.500, 0.500, 0.500, 0.500, 0.500, 0.500};
float colortable_abovesealevel1_red[MBV_NUM_COLORS + 1] = {0.980, 0.960, 0.941, 0.921, 0.902, 0.882,
                                                           0.862, 0.843, 0.823, 0.804, 0.784};
float colortable_abovesealevel1_green[MBV_NUM_COLORS + 1] = {0.980, 0.940, 0.901, 0.862, 0.823, 0.784,
                                                             0.744, 0.705, 0.666, 0.627, 0.588};
float colortable_abovesealevel1_blue[MBV_NUM_COLORS + 1] = {0.471, 0.440, 0.408, 0.376, 0.345, 0.314,
                                                            0.282, 0.250, 0.219, 0.188, 0.157};
float colortable_abovesealevel2_red[MBV_NUM_COLORS + 1] = {1.000, 0.824, 0.667, 0.569, 0.471, 0.471,
                                                           0.408, 0.263, 0.129, 0.000, 0.000};
float colortable_abovesealevel2_green[MBV_NUM_COLORS + 1] = {1.000, 0.784, 0.627, 0.569, 0.510, 0.392,
                                                             0.420, 0.482, 0.549, 0.627, 0.902};
float colortable_abovesealevel2_blue[MBV_NUM_COLORS + 1] = {0.392, 0.294, 0.196, 0.176, 0.157, 0.118,
                                                            0.094, 0.027, 0.000, 0.000, 0.000};

float colortable_object_red[MBV_NUM_COLORS] = {0.000, 1.000, 1.000, 1.000, 0.000, 0.000, 0.000, 1.000, 0.000, 0.000, 0.000};
float colortable_object_green[MBV_NUM_COLORS] = {0.000, 1.000, 0.000, 1.000, 1.000, 1.000, 0.000, 0.000, 0.000, 0.000, 0.000};
float colortable_object_blue[MBV_NUM_COLORS] = {0.000, 1.000, 0.000, 0.000, 0.000, 1.000, 1.000, 1.000, 0.000, 0.000, 0.000};

char *mbview_colorname[MBV_NUM_COLORS] = {"Black", "White", "Red", "Yellow", "Green", "Blue-Green", "Blue", "Purple"};

/* status mask arrays */
char statmask[8] = {MBV_STATMASK0, MBV_STATMASK1, MBV_STATMASK2, MBV_STATMASK3,
                    MBV_STATMASK4, MBV_STATMASK5, MBV_STATMASK6, MBV_STATMASK7};

/* extern declarations */
#else

/* library colortables */
extern float colortable_haxby_red[MBV_NUM_COLORS];
extern float colortable_haxby_green[MBV_NUM_COLORS];
extern float colortable_haxby_blue[MBV_NUM_COLORS];
extern float colortable_bright_red[MBV_NUM_COLORS];
extern float colortable_bright_green[MBV_NUM_COLORS];
extern float colortable_bright_blue[MBV_NUM_COLORS];
extern float colortable_muted_red[MBV_NUM_COLORS];
extern float colortable_muted_green[MBV_NUM_COLORS];
extern float colortable_muted_blue[MBV_NUM_COLORS];
extern float colortable_redtoblue_red[MBV_NUM_COLORS];
extern float colortable_redtoblue_green[MBV_NUM_COLORS];
extern float colortable_redtoblue_blue[MBV_NUM_COLORS];
extern float colortable_gray_red[MBV_NUM_COLORS];
extern float colortable_gray_green[MBV_NUM_COLORS];
extern float colortable_gray_blue[MBV_NUM_COLORS];
extern float colortable_flat_red[MBV_NUM_COLORS];
extern float colortable_flat_green[MBV_NUM_COLORS];
extern float colortable_flat_blue[MBV_NUM_COLORS];
extern float colortable_abovesealevel1_red[MBV_NUM_COLORS + 1];
extern float colortable_abovesealevel1_green[MBV_NUM_COLORS + 1];
extern float colortable_abovesealevel1_blue[MBV_NUM_COLORS + 1];
extern float colortable_abovesealevel2_red[MBV_NUM_COLORS + 1];
extern float colortable_abovesealevel2_green[MBV_NUM_COLORS + 1];
extern float colortable_abovesealevel2_blue[MBV_NUM_COLORS + 1];
extern float colortable_object_red[MBV_NUM_COLORS + 1];
extern float colortable_object_green[MBV_NUM_COLORS + 1];
extern float colortable_object_blue[MBV_NUM_COLORS + 1];
extern char *mbview_colorname[MBV_NUM_COLORS];

/* status mask arrays */
extern char statmask[8];
#endif

/*--------------------------------------------------------------------*/

/* mbview_callbacks.c function prototypes */
int mbview_startup(int verbose, Widget parent, XtAppContext app, int *error);
int mbview_reset_shared(bool mode);
int mbview_reset(size_t instance);
int mbview_init(int verbose, size_t *instance, int *error);
int mbview_quit(int verbose, int *error);
int mbview_getdataptr(int verbose, size_t instance, struct mbview_struct **datahandle, int *error);
int mbview_getsharedptr(int verbose, struct mbview_shareddata_struct **sharedhandle, int *error);
int mbview_setwindowparms(int verbose, size_t instance, int (*mbview_dismiss_notify)(size_t), char *title, int xo, int yo,
                          int width, int height, int lorez_dimension, int hirez_dimension, int lorez_navdecimate,
                          int hirez_navdecimate, int *error);
int mbview_setviewcontrols(int verbose, size_t instance, int display_mode, int mouse_mode, int grid_mode, int primary_histogram,
                           int primaryslope_histogram, int secondary_histogram, int primary_shade_mode, int slope_shade_mode,
                           int secondary_shade_mode, int grid_contour_mode, int site_view_mode, int route_view_mode,
                           int nav_view_mode, int navswathbounds_view_mode, int navdrape_view_mode, int vector_view_mode, double exageration,
                           double modelelevation3d, double modelazimuth3d, double viewelevation3d, double viewazimuth3d,
                           double illuminate_magnitude, double illuminate_elevation, double illuminate_azimuth,
                           double slope_magnitude, double overlay_shade_magnitude, double overlay_shade_center,
                           int overlay_shade_mode, double contour_interval, int display_projection_mode,
                           char *display_projection_id, int *error);
int mbview_open(int verbose, size_t instance, int *error);
int mbview_update(int verbose, size_t instance, int *error);
int mbview_update_sensitivity(int verbose, size_t instance, int *error);
int mbview_action_sensitivityall(void);
int mbview_action_sensitivity(size_t instance);
int mbview_set_widgets(int verbose, size_t instance, int *error);
int mbview_addaction(int verbose, size_t instance, void(mbview_action_notify)(Widget, XtPointer, XtPointer), char *label,
                     int sensitive, int *error);
int mbview_setstate(int verbose, size_t instance, int mask, int value, int *error);
int mbview_addpicknotify(int verbose, size_t instance, int picktype, void(mbview_pick_notify)(size_t), int *error);
int mbview_setsensitivitynotify(int verbose, size_t instance, void(mbview_sensitivity_notify)(), int *error);
void mbview_resize(Widget w, XtPointer client_data, XEvent *event, Boolean *unused);
void do_mbview_projection_popup(Widget w, XtPointer client_data, XtPointer call_data);
void do_mbview_set_projection_label(size_t instance);
void do_mbview_projection_popdown(Widget w, XtPointer client_data, XtPointer call_data);
void do_mbview_display_spheroid(Widget w, XtPointer client_data, XtPointer call_data);
void do_mbview_display_geographic(Widget w, XtPointer client_data, XtPointer call_data);
void do_mbview_display_utm(Widget w, XtPointer client_data, XtPointer call_data);
void do_mbview_annotation_degreesminutes(Widget w, XtPointer client_data, XtPointer call_data);
void do_mbview_annotation_degreesdecimal(Widget w, XtPointer client_data, XtPointer call_data);
void do_mbview_glwda_expose(Widget w, XtPointer client_data, XtPointer call_data);
void do_mbview_glwda_resize(Widget w, XtPointer client_data, XtPointer call_data);
void do_mbview_glwda_input(Widget w, XtPointer client_data, XtPointer call_data);
void do_mbview_dismiss(Widget w, XtPointer client_data, XtPointer call_data);
void do_mbview_goaway(Widget w, XtPointer client_data, XtPointer call_data);
int mbview_destroy(int verbose, size_t instance, bool destroywidgets, int *error);
int mbview_quit(int verbose, int *error);
void do_mbview_display_2D(Widget w, XtPointer client_data, XtPointer call_data);
void do_mbview_display_3D(Widget w, XtPointer client_data, XtPointer call_data);
void do_mbview_data_primary(Widget w, XtPointer client_data, XtPointer call_data);
void do_mbview_data_primaryslope(Widget w, XtPointer client_data, XtPointer call_data);
void do_mbview_data_secondary(Widget w, XtPointer client_data, XtPointer call_data);
void do_mbview_histogram(Widget w, XtPointer client_data, XtPointer call_data);
void do_mbview_overlay_none(Widget w, XtPointer client_data, XtPointer call_data);
void do_mbview_overlay_slope(Widget w, XtPointer client_data, XtPointer call_data);
void do_mbview_overlay_illumination(Widget w, XtPointer client_data, XtPointer call_data);
void do_mbview_overlay_secondary(Widget w, XtPointer client_data, XtPointer call_data);
void do_mbview_overlay_contour(Widget w, XtPointer client_data, XtPointer call_data);
void do_mbview_site(Widget w, XtPointer client_data, XtPointer call_data);
void do_mbview_route(Widget w, XtPointer client_data, XtPointer call_data);
void do_mbview_nav(Widget w, XtPointer client_data, XtPointer call_data);
void do_mbview_navdrape(Widget w, XtPointer client_data, XtPointer call_data);
void do_mbview_vector(Widget w, XtPointer client_data, XtPointer call_data);
void do_mbview_colortable_haxby(Widget w, XtPointer client_data, XtPointer call_data);
void do_mbview_colortable_bright(Widget w, XtPointer client_data, XtPointer call_data);
void do_mbview_colortable_muted(Widget w, XtPointer client_data, XtPointer call_data);
void do_mbview_colortable_gray(Widget w, XtPointer client_data, XtPointer call_data);
void do_mbview_colortable_flat(Widget w, XtPointer client_data, XtPointer call_data);
void do_mbview_colortable_sealevel1(Widget w, XtPointer client_data, XtPointer call_data);
void do_mbview_colortable_sealevel2(Widget w, XtPointer client_data, XtPointer call_data);
void do_mbview_mouse_rmode(Widget w, XtPointer client_data, XtPointer call_data);
void do_mbview_mouse_mode(Widget w, XtPointer client_data, XtPointer call_data);
void set_mbview_mouse_mode(size_t instance, int mode);
void set_mbview_grid_mode(size_t instance, int mode);
void set_mbview_histogram_mode(size_t instance, bool mode);
void set_mbview_shade_mode(size_t instance, int mode);
void set_mbview_contour_mode(size_t instance, int mode);
void set_mbview_site_view_mode(size_t instance, int mode);
void set_mbview_route_view_mode(size_t instance, int mode);
void set_mbview_nav_view_mode(size_t instance, int mode);
void set_mbview_navswathbounds_view_mode(size_t instance, int mode);
void set_mbview_navdrape_view_mode(size_t instance, int mode);
void set_mbview_vector_view_mode(size_t instance, int mode);
void set_mbview_display_mode(size_t instance, int mode);
void set_mbview_colortable(size_t instance, int mode);
void set_mbview_colortable_mode(size_t instance, int mode);
void do_mbview_aboutpopdown(Widget w, XtPointer client_data, XtPointer call_data);
void do_mbview_aboutpopup(Widget w, XtPointer client_data, XtPointer call_data);
void do_mbview_colorboundspopup(Widget w, XtPointer client_data, XtPointer call_data);
void do_mbview_colorboundspopdown(Widget w, XtPointer client_data, XtPointer call_data);
void do_mbview_colorboundsapply(Widget w, XtPointer client_data, XtPointer call_data);
void do_mbview_shadeparmspopup(Widget w, XtPointer client_data, XtPointer call_data);
void do_mbview_shadeparmspopdown(Widget w, XtPointer client_data, XtPointer call_data);
void do_mbview_shadeparmsapply(Widget w, XtPointer client_data, XtPointer call_data);
int do_mbview_3dparmstext(size_t instance);
void do_mbview_3dparmspopup(Widget w, XtPointer client_data, XtPointer call_data);
void do_mbview_3dparmspopdown(Widget w, XtPointer client_data, XtPointer call_data);
void do_mbview_3dparmsapply(Widget w, XtPointer client_data, XtPointer call_data);
int do_mbview_2dparmstext(size_t instance);
void do_mbview_2dparmspopup(Widget w, XtPointer client_data, XtPointer call_data);
void do_mbview_2dparmspopdown(Widget w, XtPointer client_data, XtPointer call_data);
void do_mbview_2dparmsapply(Widget w, XtPointer client_data, XtPointer call_data);
void do_mbview_resolutionpopup(Widget w, XtPointer client_data, XtPointer call_data);
void do_mbview_resolutionpopdown(Widget w, XtPointer client_data, XtPointer call_data);
void do_mbview_resolutionchange(Widget w, XtPointer client_data, XtPointer call_data);
void do_mbview_sitelistpopup(Widget w, XtPointer client_data, XtPointer call_data);
void do_mbview_routelistpopup(Widget w, XtPointer client_data, XtPointer call_data);
void do_mbview_navlistpopup(Widget w, XtPointer client_data, XtPointer call_data);
void do_mbview_sitelistselect(Widget w, XtPointer client_data, XtPointer call_data);
void do_mbview_routelistselect(Widget w, XtPointer client_data, XtPointer call_data);
void do_mbview_navlistselect(Widget w, XtPointer client_data, XtPointer call_data);
void do_mbview_sitelist_delete(Widget w, XtPointer client_data, XtPointer call_data);
void do_mbview_routelist_delete(Widget w, XtPointer client_data, XtPointer call_data);
void do_mbview_navlist_delete(Widget w, XtPointer client_data, XtPointer call_data);
void do_mbview_sitelist_popdown(Widget w, XtPointer client_data, XtPointer call_data);
void do_mbview_routelist_popdown(Widget w, XtPointer client_data, XtPointer call_data);
void do_mbview_navlist_popdown(Widget w, XtPointer client_data, XtPointer call_data);
void do_mbview_full_render(Widget w, XtPointer client_data, XtPointer call_data);
void do_mbview_reset_view(Widget w, XtPointer client_data, XtPointer call_data);
void do_mbview_clearpicks(Widget w, XtPointer client_data, XtPointer call_data);
void do_mbview_profile_dismiss(Widget w, XtPointer client_data, XtPointer call_data);
void do_mbview_view_profile(Widget w, XtPointer client_data, XtPointer call_data);
void do_mbview_profile_resize(Widget w, XtPointer client_data, XEvent *event, Boolean *unused);
void do_mbview_profile_exager(Widget w, XtPointer client_data, XtPointer call_data);
void do_mbview_profile_width(Widget w, XtPointer client_data, XtPointer call_data);
void do_mbview_profile_slope(Widget w, XtPointer client_data, XtPointer call_data);
int do_mbview_status(char *message, size_t instance);
int do_mbview_message_on(char *message, size_t instance);
int do_mbview_message_off(size_t instance);
void set_mbview_label_string(Widget w, String str);
void set_mbview_label_multiline_string(Widget w, String str);
void get_mbview_text_string(Widget w, String str);
void do_mbview_xevents(void);
int do_mbview_setbackgroundwork(size_t instance);
int do_mbview_settimer(void);
int do_mbview_workfunction(XtPointer client_data);

/* mbview_primary.c function prototypes */
int mbview_setprimarygrid(int verbose, size_t instance, int primary_grid_projection_mode, char *primary_grid_projection_id,
                          float primary_nodatavalue, int primary_n_columns, int primary_n_rows, double primary_min, double primary_max,
                          double primary_xmin, double primary_xmax, double primary_ymin, double primary_ymax, double primary_dx,
                          double primary_dy, float *primary_data, int *error);
int mbview_updateprimarygrid(int verbose, size_t instance, int primary_n_columns, int primary_n_rows, float *primary_data, int *error);
int mbview_updateprimarygridcell(int verbose, size_t instance, int primary_ix, int primary_jy, float value, int *error);
int mbview_setprimarycolortable(int verbose, size_t instance, int primary_colortable, int primary_colortable_mode,
                                double primary_colortable_min, double primary_colortable_max, int *error);
int mbview_setslopecolortable(int verbose, size_t instance, int slope_colortable, int slope_colortable_mode,
                              double slope_colortable_min, double slope_colortable_max, int *error);

/* mbview_secondary.c function prototypes */
int mbview_setsecondarygrid(int verbose, size_t instance, int secondary_grid_projection_mode, char *secondary_grid_projection_id,
                            float secondary_nodatavalue, int secondary_n_columns, int secondary_n_rows, double secondary_min,
                            double secondary_max, double secondary_xmin, double secondary_xmax, double secondary_ymin,
                            double secondary_ymax, double secondary_dx, double secondary_dy, float *secondary_data, int *error);
int mbview_updatesecondarygrid(int verbose, size_t instance, int secondary_n_columns, int secondary_n_rows, float *secondary_data,
                               int *error);
int mbview_updatesecondarygridcell(int verbose, size_t instance, int primary_ix, int primary_jy, float value, int *error);
int mbview_setsecondarycolortable(int verbose, size_t instance, int secondary_colortable, int secondary_colortable_mode,
                                  double secondary_colortable_min, double secondary_colortable_max,
                                  double overlay_shade_magnitude, double overlay_shade_center, int overlay_shade_mode,
                                  int *error);
int mbview_setsecondaryname(int verbose, size_t instance, char *name, int *error);

/* mbview_process.c function prototypes */
int mbview_projectdata(size_t instance);
int mbview_derivative(size_t instance, int i, int j);
int mbview_projectglobaldata(size_t instance);
int mbview_zscalegridpoint(size_t instance, int k);
int mbview_zscalepoint(size_t instance, int globalview, double offset_factor, struct mbview_point_struct *point);
int mbview_zscalepointw(size_t instance, int globalview, double offset_factor, struct mbview_pointw_struct *pointw);
int mbview_updatepointw(size_t instance, struct mbview_pointw_struct *pointw);
int mbview_updatesegmentw(size_t instance, struct mbview_linesegmentw_struct *segmentw);
int mbview_zscale(size_t instance);
int mbview_projectforward(size_t instance, bool needlonlat, double xgrid, double ygrid, double zdata, double *xlon, double *ylat,
                          double *xdisplay, double *ydisplay, double *zdisplay);
int mbview_projectinverse(size_t instance, bool needlonlat, double xdisplay, double ydisplay, double zdisplay, double *xlon,
                          double *ylat, double *xgrid, double *ygrid);
int mbview_projectfromlonlat(size_t instance, double xlon, double ylat, double zdata, double *xgrid, double *ygrid,
                             double *xdisplay, double *ydisplay, double *zdisplay);
int mbview_projectgrid2ll(size_t instance, double xgrid, double ygrid, double *xlon, double *ylat);
int mbview_projectll2xygrid(size_t instance, double xlon, double ylat, double *xgrid, double *ygrid);
int mbview_projectll2xyzgrid(size_t instance, double xlon, double ylat, double *xgrid, double *ygrid, double *zdata);
int mbview_projectll2display(size_t instance, double xlon, double ylat, double zdata, double *xdisplay, double *ydisplay,
                             double *zdisplay);
int mbview_projectdisplay2ll(size_t instance, double xdisplay, double ydisplay, double zdisplay, double *xlon, double *ylat);
int mbview_projectdistance(size_t instance, double xlon1, double ylat1, double zdata1, double xlon2, double ylat2, double zdata2,
                           double *distancelateral, double *distanceoverground, double *slope);
int mbview_sphere_setup(size_t instance, bool earthcentered, double xlon, double ylat);
int mbview_sphere_forward(size_t instance, double xlon, double ylat, double *xx, double *yy, double *zz);
int mbview_sphere_inverse(size_t instance, double xx, double yy, double zz, double *xlon, double *ylat);
int mbview_sphere_matrix(double phi, double theta, double psi, double *eulermatrix);
int mbview_sphere_rotate(double *eulermatrix, double *v, double *vr);
int mbview_greatcircle_distbearing(size_t instance, double lon1, double lat1, double lon2, double lat2, double *bearing,
                                   double *distance);
int mbview_greatcircle_dist(size_t instance, double lon1, double lat1, double lon2, double lat2, double *distance);
int mbview_greatcircle_endposition(size_t instance, double lon1, double lat1, double bearing, double distance, double *lon2,
                                   double *lat2);
int mbview_colorclear(size_t instance);
int mbview_zscaleclear(size_t instance);
int mbview_setcolorparms(size_t instance);
int mbview_make_histogram(struct mbview_world_struct *view, struct mbview_struct *data, int which_data);
int mbview_colorvalue(struct mbview_world_struct *view, struct mbview_struct *data,
                      float *histogram, double value, float *r, float *g, float *b);
int mbview_colorpoint(struct mbview_world_struct *view, struct mbview_struct *data, float *histogram, int i, int j, int k);
int mbview_getcolor(double value, double min, double max, int colortablemode, float below_red, float below_green,
                    float below_blue, float above_red, float above_green, float above_blue, float *colortable_red,
                    float *colortable_green, float *colortable_blue, float *red, float *green, float *blue);
int mbview_getcolor_histogram(double value, double min, double max, int colortablemode, float below_red, float below_green,
                              float below_blue, float above_red, float above_green, float above_blue, float *colortable_red,
                              float *colortable_green, float *colortable_blue, float *histogram, float *red, float *green,
                              float *blue);
int mbview_applyshade(double intensity, float *r, float *g, float *b);
int mbview_getsecondaryvalue(struct mbview_world_struct *view, struct mbview_struct *data, int i, int j, double *secondary_value);
int mbview_contour(size_t instance, int rez);
int mbview_getzdata(size_t instance, double xgrid, double ygrid, bool *found, double *zdata);

/* mbview_plot.c function prototypes */
int mbview_reset_glx(size_t instance);
int mbview_drawdata(size_t instance, int rez);
int mbview_plotlowall(size_t instance);
int mbview_plotlowhighall(size_t instance);
int mbview_plothighall(size_t instance);
int mbview_plotlow(size_t instance);
int mbview_plotlowhigh(size_t instance);
int mbview_plothigh(size_t instance);
int mbview_plotfull(size_t instance);
int mbview_plot(size_t instance, int rez);
int mbview_findpoint(size_t instance, int xpixel, int ypixel, bool *found, double *xgrid, double *ygrid, double *xlon,
                     double *ylat, double *zdata, double *xdisplay, double *ydisplay, double *zdisplay);
int mbview_findpointrez(size_t instance, int rez, int xpixel, int ypixel, int ijbounds[4], bool *found, double *xgrid,
                        double *ygrid, double *xlon, double *ylat, double *zdata, double *xdisplay, double *ydisplay,
                        double *zdisplay);
int mbview_viewbounds(size_t instance);
int mbview_drapesegment(size_t instance, struct mbview_linesegment_struct *seg);
int mbview_drapesegment_gc(size_t instance, struct mbview_linesegment_struct *seg);
int mbview_drapesegment_grid(size_t instance, struct mbview_linesegment_struct *seg);
int mbview_drapesegmentw(size_t instance, struct mbview_linesegmentw_struct *seg);
int mbview_drapesegmentw_gc(size_t instance, struct mbview_linesegmentw_struct *seg);
int mbview_drapesegmentw_grid(size_t instance, struct mbview_linesegmentw_struct *seg);
int mbview_glerrorcheck(size_t instance, char *sourcefile, int line, char *sourcefunction);

/* mbview_pick.c function prototypes */
int mbview_pick(size_t instance, int which, int xpixel, int ypixel);
int mbview_extract_pick_profile(size_t instance);
int mbview_picksize(size_t instance);
int mbview_pick_text(size_t instance);
int mbview_setlonlatstrings(double lon, double lat, char *londstring, char *latdstring, char *lonmstring, char *latmstring);
int mbview_region(size_t instance, int which, int xpixel, int ypixel);
int mbview_area(size_t instance, int which, int xpixel, int ypixel);
int mbview_drawpick(size_t instance);
int mbview_drawregion(size_t instance);
int mbview_drawarea(size_t instance);

/* mbview_nav.c function prototypes */
int mbview_getnavcount(int verbose, size_t instance, int *nnav, int *error);
int mbview_getnavpointcount(int verbose, size_t instance, int nav, int *npoint, int *nintpoint, int *error);
int mbview_allocnavarrays(int verbose, int npointtotal, double **time_d, double **navlon, double **navlat, double **navz,
                          double **heading, double **speed, double **navportlon, double **navportlat, double **navstbdlon,
                          double **navstbdlat, int **line, int **shot, int **cdp, int *error);
int mbview_freenavarrays(int verbose, double **time_d, double **navlon, double **navlat, double **navz, double **heading,
                         double **speed, double **navportlon, double **navportlat, double **navstbdlon, double **navstbdlat,
                         int **line, int **shot, int **cdp, int *error);
int mbview_addnav(int verbose, size_t instance, int npoint, double *time_d, double *navlon, double *navlat, double *navz,
                  double *heading, double *speed, double *navportlon, double *navportlat, double *navstbdlon, double *navstbdlat,
                  unsigned int *line, unsigned int *shot, unsigned int *cdp, int navcolor, int navsize, mb_path navname, int navpathstatus,
                  mb_path navpathraw, mb_path navpathprocessed, int navformat, bool navswathbounds, bool navline, bool navshot,
                  bool navcdp, int decimation, int *error);
int mbview_enableviewnavs(int verbose, size_t instance, int *error);
int mbview_pick_nav_select(size_t instance, int select, int which, int xpixel, int ypixel);
int mbview_extract_nav_profile(size_t instance);
int mbview_nav_delete(size_t instance, int inav);
int mbview_navpicksize(size_t instance);
int mbview_drawnavpick(size_t instance);
int mbview_drawnav(size_t instance, int rez);
int mbview_updatenavlist(void);

/* mbview_route.c function prototypes */
int mbview_getroutecount(int verbose, size_t instance, int *nroute, int *error);
int mbview_getroutepointcount(int verbose, size_t instance, int route, int *npoint, int *nintpoint, int *error);
int mbview_getrouteselected(int verbose, size_t instance, int route, bool *selected, int *error);
int mbview_getrouteinfo(int verbose, size_t instance, int working_route, int *nroutewaypoint, int *nroutpoint, char *routename,
                        int *routecolor, int *routesize, double *routedistancelateral, double *routedistancetopo, int *error);
int mbview_allocroutearrays(int verbose, int npointtotal, double **routelon, double **routelat, int **waypoint,
                            double **routetopo, double **routebearing, double **distlateral, double **distovertopo,
                            double **slope, int *error);
int mbview_freeroutearrays(int verbose, double **routelon, double **routelat, int **waypoint, double **routetopo,
                           double **routebearing, double **distlateral, double **distovertopo, double **slope, int *error);
int mbview_addroute(int verbose, size_t instance, int npoint, double *routelon, double *routelat, int *waypoint, int routecolor,
                    int routesize, int routeeditmode, mb_path routename, int *iroute, int *error);
int mbview_deleteroute(int verbose, size_t instance, int iroute, int *error);
int mbview_getroute(int verbose, size_t instance, int route, int *npointtotal, double *routelon, double *routelat, int *waypoint,
                    double *routetopo, double *routebearing, double *distlateral, double *distovertopo, double *slope,
                    int *routecolor, int *routesize, int *routeeditmode, mb_path routename, int *error);
int mbview_enableviewroutes(int verbose, size_t instance, int *error);
int mbview_enableeditroutes(int verbose, size_t instance, int *error);
int mbview_pick_route_select(int verbose, size_t instance, int which, int xpixel, int ypixel);
int mbview_extract_route_profile(size_t instance);
int mbview_pick_route_add(int verbose, size_t instance, int which, int xpixel, int ypixel);
int mbview_pick_route_delete(int verbose, size_t instance, int xpixel, int ypixel);
int mbview_route_add(int verbose, size_t instance, int inew, int jnew, int waypoint, double xgrid, double ygrid, double xlon,
                     double ylat, double zdata, double xdisplay, double ydisplay, double zdisplay);
int mbview_route_delete(size_t instance, int iroute, int ipoint);
int mbview_route_setdistance(size_t instance, int working_route);
int mbview_drawroute(size_t instance, int rez);
int mbview_updateroutelist(void);

/* mbview_site.c function prototypes */
int mbview_getsitecount(int verbose, size_t instance, int *nsite, int *error);
int mbview_allocsitearrays(int verbose, int nsite, double **sitelon, double **sitelat, double **sitetopo, int **sitecolor,
                           int **sitesize, mb_path **sitename, int *error);
int mbview_freesitearrays(int verbose, double **sitelon, double **sitelat, double **sitetopo, int **sitecolor, int **sitesize,
                          mb_path **sitename, int *error);
int mbview_addsites(int verbose, size_t instance, int nsite, double *sitelon, double *sitelat, double *sitetopo, int *sitecolor,
                    int *sitesize, mb_path *sitename, int *error);
int mbview_getsites(int verbose, size_t instance, int *nsite, double *sitelon, double *sitelat, double *sitetopo, int *sitecolor,
                    int *sitesize, mb_path *sitename, int *error);
int mbview_enableviewsites(int verbose, size_t instance, int *error);
int mbview_enableeditsites(int verbose, size_t instance, int *error);
int mbview_pick_site_select(size_t instance, int which, int xpixel, int ypixel);
int mbview_pick_site_add(size_t instance, int which, int xpixel, int ypixel);
int mbview_pick_site_delete(size_t instance, int xpixel, int ypixel);
int mbview_site_delete(size_t instance, int isite);
int mbview_drawsite(size_t instance, int rez);
int mbview_updatesitelist(void);

/* mbview_profile.c function prototypes */
int mbview_getprofilecount(int verbose, size_t instance, int *npoints, int *error);
int mbview_allocprofilepoints(int verbose, int npoints, struct mbview_profilepoint_struct **points, int *error);
int mbview_freeprofilepoints(int verbose, double **points, int *error);
int mbview_allocprofilearrays(int verbose, int npoints, double **distance, double **zdata, int **boundary, double **xlon,
                              double **ylat, double **distovertopo, double **bearing, double **slope, int *error);
int mbview_freeprofilearrays(int verbose, double **distance, double **zdata, int **boundary, double **xlon, double **ylat,
                             double **distovertopo, double **bearing, double **slope, int *error);
int mbview_getprofile(int verbose, size_t instance, mb_path source_name, double *length, double *zmin, double *zmax, int *npoints,
                      double *distance, double *zdata, int *boundary, double *xlon, double *ylat, double *distovertopo,
                      double *bearing, double *slope, int *error);
int mbview_reset_prglx(size_t instance);
int mbview_destroy_prglx(size_t instance);
int mbview_plotprofile(size_t instance);
int mbview_profile_text(size_t instance);

void BxUnmanageCB(Widget w, XtPointer client, XtPointer call);
void BxManageCB(Widget w, XtPointer client, XtPointer call);
void BxPopupCB(Widget w, XtPointer client, XtPointer call);
XtPointer BX_CONVERT(Widget w, char *from_string, char *to_type, int to_size, Boolean *success);
void BxExitCB(Widget w, XtPointer client, XtPointer call);
void BxSetValuesCB(Widget w, XtPointer client, XtPointer call);

#endif  // MBVIEW_MBVIEWPRIVATE_H_
