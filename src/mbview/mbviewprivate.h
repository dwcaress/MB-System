/*--------------------------------------------------------------------
 *    The MB-system:	mbviewprivate.h	9/24/2003
 *    $Id: mbviewprivate.h,v 5.0 2003-12-02 20:38:34 caress Exp $
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
 *--------------------------------------------------------------------*/
/*
 *
 * Author:	D. W. Caress
 * Date:	September 24,  2003
 *
 * $Log: not supported by cvs2svn $
 * Revision 1.3  2003/11/25 22:14:03  caress
 * Fixed problem with display of mouse mode buttons.
 *
 * Revision 1.2  2003/11/25 02:52:55  caress
 * MBview version generated during EW0310.
 *
 *
 *
 */

/*--------------------------------------------------------------------*/

#define LEFT_WIDTH 200
#define LEFT_HEIGHT 30
#define MBV_OPENGL_WIDTH 3.0
#define MBV_OPENGL_ZMIN2D -5.0
#define MBV_OPENGL_ZMAX2D 1000.0
#define MBV_OPENGL_ZMIN3D 100000.0
#define MBV_OPENGL_ZMAX3D 100000000.0
#define MBV_OPENGL_3D_CONTOUR_OFFSET 0.001
#define MBV_OPENGL_3D_LINE_OFFSET 0.005

#define MBV_REZ_NONE 	0
#define MBV_REZ_LOW 	1
#define MBV_REZ_HIGH 	2
#define MBV_REZ_FULL 	3
#define MBV_EVENTCHECKCOARSENESS	5

#define	MBV_NUMBACKGROUNDCALC	500
#define	MBV_BACKGROUND_ZSCALE	1
#define	MBV_BACKGROUND_COLOR	2
#define	MBV_BACKGROUND_FULLPLOT	3

#define MBVIEW_PICK_IDIVISION 15
#define MBVIEW_PICK_DIVISION ((double)MBVIEW_PICK_IDIVISION)
#define MBV_PICK_DOWN	1
#define MBV_PICK_MOVE	2
#define MBV_PICK_UP	3
#define MBV_AREALENGTH_DOWN	1
#define MBV_AREALENGTH_MOVE	2
#define MBV_AREALENGTH_UP	3
#define MBV_AREAASPECT_CHANGE	4

#define MBV_WINDOW_NULL 	0
#define MBV_WINDOW_HIDDEN 	1
#define MBV_WINDOW_VISIBLE 	2

#define MBV_WINDOW_HEIGHT_THRESHOLD 	700

#define	MBV_NUM_COLORS   11

/* library variables */
/*   note that global mbview variables will only be defined when
     this code is included in mbview_callbacks.c where the
     MBVIEWGLOBAL flag is defined - other blocks of code will
     have these variables declared as extern  */
#ifdef MBVIEWGLOBAL
/* general library global variables */
int	mbv_verbose;
int	mbv_ninstance;
Widget	parent_widget;
XtAppContext	app_context;
int	work_function_set;
int	timer_count;
struct mbview_world_struct mbviews[MBV_MAX_WINDOWS];
char	*mbsystem_library_name;

/* library colortables */
float	colortable_haxby_red[MBV_NUM_COLORS] =
                { 0.950, 1.000, 1.000, 1.000, 0.941, 0.804, 
                  0.541, 0.416, 0.196, 0.157, 0.145 };
float	colortable_haxby_green[MBV_NUM_COLORS] =
                { 0.950, 0.729, 0.631, 0.741, 0.925, 1.000, 
                  0.925, 0.922, 0.745, 0.498, 0.224 };
float	colortable_haxby_blue[MBV_NUM_COLORS] =
                { 0.950, 0.522, 0.267, 0.341, 0.475, 0.635, 
                  0.682, 1.000, 1.000, 0.984, 0.686 };
float	colortable_bright_red[MBV_NUM_COLORS] =
                { 1.000, 1.000, 1.000, 1.000, 0.500, 0.000, 
                  0.000, 0.000, 0.000, 0.500, 1.000 };
float	colortable_bright_green[MBV_NUM_COLORS] =
                { 0.000, 0.250, 0.500, 1.000, 1.000, 1.000, 
                  1.000, 0.500, 0.000, 0.000, 0.000 };
float	colortable_bright_blue[MBV_NUM_COLORS] =
                { 0.000, 0.000, 0.000, 0.000, 0.000, 0.000, 
                  1.000, 1.000, 1.000, 1.000, 1.000 };
float	colortable_muted_red[MBV_NUM_COLORS] =
                { 0.784, 0.761, 0.702, 0.553, 0.353, 0.000, 
                  0.000, 0.000, 0.000, 0.353, 0.553 };
float	colortable_muted_green[MBV_NUM_COLORS] =
                { 0.000, 0.192, 0.353, 0.553, 0.702, 0.784, 
                  0.553, 0.353, 0.000, 0.000, 0.000 };
float	colortable_muted_blue[MBV_NUM_COLORS] =
                { 0.000, 0.000, 0.000, 0.000, 0.000, 0.000, 
                  0.553, 0.702, 0.784, 0.702, 0.553 };
float	colortable_gray_red[MBV_NUM_COLORS] =
                { 0.000, 0.100, 0.200, 0.300, 0.400, 0.500, 
                  0.600, 0.700, 0.800, 0.900, 1.000 };
float	colortable_gray_green[MBV_NUM_COLORS] =
                { 0.000, 0.100, 0.200, 0.300, 0.400, 0.500, 
                  0.600, 0.700, 0.800, 0.900, 1.000 };
float	colortable_gray_blue[MBV_NUM_COLORS] =
                { 0.000, 0.100, 0.200, 0.300, 0.400, 0.500, 
                  0.600, 0.700, 0.800, 0.900, 1.000 };
float	colortable_flat_red[MBV_NUM_COLORS] =
                { 0.500, 0.500, 0.500, 0.500, 0.500, 0.500, 
                  0.500, 0.500, 0.500, 0.500, 0.500 };
float	colortable_flat_green[MBV_NUM_COLORS] =
                { 0.500, 0.500, 0.500, 0.500, 0.500, 0.500, 
                  0.500, 0.500, 0.500, 0.500, 0.500 };
float	colortable_flat_blue[MBV_NUM_COLORS] =
                { 0.500, 0.500, 0.500, 0.500, 0.500, 0.500, 
                  0.500, 0.500, 0.500, 0.500, 0.500 };
float	colortable_abovesealevel_red[MBV_NUM_COLORS+1] =
                { 0.980, 0.960, 0.941, 0.921, 0.902, 0.882, 
		  0.862, 0.843, 0.823, 0.804, 0.784};
float	colortable_abovesealevel_green[MBV_NUM_COLORS+1] =
                { 0.980, 0.940, 0.901, 0.862, 0.823, 0.784, 
		  0.744, 0.705, 0.666, 0.627, 0.588};
float	colortable_abovesealevel_blue[MBV_NUM_COLORS+1] =
                { 0.471, 0.440, 0.408, 0.376, 0.345, 0.314, 
		  0.282, 0.250, 0.219, 0.188, 0.157};

float	colortable_object_red[MBV_NUM_COLORS] =
                { 0.000, 1.000, 1.000, 1.000, 0.000, 0.000, 
                  0.000, 1.000, 0.000, 0.000, 0.000 };
float	colortable_object_green[MBV_NUM_COLORS] =
                { 0.000, 1.000, 0.000, 1.000, 1.000, 1.000, 
                  0.000, 0.000, 0.000, 0.000, 0.000 };
float	colortable_object_blue[MBV_NUM_COLORS] =
                { 0.000, 1.000, 0.000, 0.000, 0.000, 1.000, 
                  1.000, 1.000, 0.000, 0.000, 0.000 };
		  
/* status mask arrays */
char	statmask[8] = { MBV_STATMASK0,
			MBV_STATMASK1,
			MBV_STATMASK2,
			MBV_STATMASK3,
			MBV_STATMASK4,
			MBV_STATMASK5,
			MBV_STATMASK6,
			MBV_STATMASK7};

/* extern declarations */
#else
extern int	mbv_verbose;
extern int	mbv_ninstance;
extern Widget	parent_widget;
extern XtAppContext	app_context;
extern int	work_function_set;
extern struct mbview_world_struct mbviews[MBV_MAX_WINDOWS];
extern char	*mbsystem_library_name;

/* library colortables */
extern float	colortable_haxby_red[MBV_NUM_COLORS];
extern float	colortable_haxby_green[MBV_NUM_COLORS];
extern float	colortable_haxby_blue[MBV_NUM_COLORS];
extern float	colortable_bright_red[MBV_NUM_COLORS];
extern float	colortable_bright_green[MBV_NUM_COLORS];
extern float	colortable_bright_blue[MBV_NUM_COLORS];
extern float	colortable_muted_red[MBV_NUM_COLORS];
extern float	colortable_muted_green[MBV_NUM_COLORS];
extern float	colortable_muted_blue[MBV_NUM_COLORS];
extern float	colortable_gray_red[MBV_NUM_COLORS];
extern float	colortable_gray_green[MBV_NUM_COLORS];
extern float	colortable_gray_blue[MBV_NUM_COLORS];
extern float	colortable_flat_red[MBV_NUM_COLORS];
extern float	colortable_flat_green[MBV_NUM_COLORS];
extern float	colortable_flat_blue[MBV_NUM_COLORS];
extern float	colortable_abovesealevel_red[MBV_NUM_COLORS+1];
extern float	colortable_abovesealevel_green[MBV_NUM_COLORS+1];
extern float	colortable_abovesealevel_blue[MBV_NUM_COLORS+1];
extern float	colortable_object_red[MBV_NUM_COLORS+1];
extern float	colortable_object_green[MBV_NUM_COLORS+1];
extern float	colortable_object_blue[MBV_NUM_COLORS+1];
		  
/* status mask arrays */
extern char	statmask[8];
#endif

/* structure to hold instances of mbview windows */
struct mbview_world_struct
    {
    /* flag if this instance is initialized */
    int			init;
    
    /* pointer to structure holding data to be rendered */
    struct mbview_struct data;
    
    /* widgets and other Xwindows stuff of interest */
    Widget		topLevelShell;
    Widget		mainWindow;
    Widget		glwmda;
    MB3DViewData	mb3dview;
    Display		*dpy;
    Window		xid;
    XVisualInfo 	*vi;
    int			glx_init;
    GLXContext		glx_context;
    int			message_on;
    int			plot_recursion;
    int			plot_done;
    int			plot_interrupt_allowed;
    int			work_function_set;
       
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
    int lastdrawrez;
    int	zscaledonecount;
    int	colordonecount;
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
    int	shade_mode;
    double sign;

    /* grid display bounds */
    double xmin;
    double xmax;
    double ymin;
    double ymax;
    double xorigin;
    double yorigin;
    double zorigin;
    double scale;
    double zscale;
    
    /* relevant mbio defaults */
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

	
/*--------------------------------------------------------------------*/

void do_mbview_glwda_expose( Widget w, XtPointer client_data, XtPointer call_data);
void do_mbview_glwda_resize( Widget w, XtPointer client_data, XtPointer call_data);
void do_mbview_glwda_input( Widget w, XtPointer client_data, XtPointer call_data);
void mbview_resize( Widget w, XtPointer client_data, XEvent *event, Boolean *unused);
void do_mbview_dismiss( Widget w, XtPointer client_data, XtPointer call_data);
void do_mbview_data_primary( Widget w, XtPointer client_data, XtPointer call_data);
void do_mbview_data_primaryslope( Widget w, XtPointer client_data, XtPointer call_data);
void do_mbview_data_secondary( Widget w, XtPointer client_data, XtPointer call_data);
void do_mbview_overlay_none( Widget w, XtPointer client_data, XtPointer call_data);
void do_mbview_overlay_slope( Widget w, XtPointer client_data, XtPointer call_data);
void do_mbview_overlay_illumination( Widget w, XtPointer client_data, XtPointer call_data);
void do_mbview_overlay_secondary( Widget w, XtPointer client_data, XtPointer call_data);
void do_mbview_overlay_contour( Widget w, XtPointer client_data, XtPointer call_data);
void do_mbview_display_2D( Widget w, XtPointer client_data, XtPointer call_data);
void do_mbview_display_3D( Widget w, XtPointer client_data, XtPointer call_data);
void do_mbview_mouse_mode( Widget w, XtPointer client_data, XtPointer call_data);
void do_mbview_reset_view( Widget w, XtPointer client_data, XtPointer call_data);
void set_mbview_mouse_mode(int instance, int mode);
void set_mbview_grid_mode(int instance, int mode);
void set_mbview_shade_mode(int instance, int mode);
void set_mbview_contour_mode(int instance, int mode);
void set_mbview_site_view_mode(int instance, int mode);
void set_mbview_route_view_mode(int instance, int mode);
void set_mbview_nav_view_mode(int instance, int mode);
void set_mbview_navdrape_view_mode(int instance, int mode);
void set_mbview_display_mode(int instance, int mode);
void set_mbview_colortable_mode(int instance, int mode);
void set_mbview_colortable(int instance, int mode);
void set_mbview_label_string(Widget w, String str);
void set_mbview_label_multiline_string(Widget w, String str);
void get_mbview_text_string(Widget w, String str);
void do_mbview_xevents();
int mbview_projectforward(int instance, int needlonlat,
				double xgrid, double ygrid,
				double *xlon, double *ylat,
				double *xdisplay, double *ydisplay);
int mbview_projectinverse(int instance, int needlonlat,
				double xdisplay, double ydisplay,
				double *xlon, double *ylat,
				double *xgrid, double *ygrid);
int mbview_getzdata(int instance, 
			double xgrid, double ygrid,
			int *found, double *zdata);
int mbview_pick(int instance, int which, int xpixel, int ypixel);
int mbview_findpoint(int instance, int xpixel, int ypixel,
			int *found, 
			double *xgrid, double *ygrid,
			double *xlon, double *ylat, double *zdata,
			double *xdisplay, double *ydisplay, double *zdisplay);
int mbview_findpointrez(int instance, int rez, int xpixel, int ypixel,
			int ijbounds[4], int *found, 
			double *xgrid, double *ygrid,
			double *xlon, double *ylat, double *zdata,
			double *xdisplay, double *ydisplay, double *zdisplay);
int mbview_viewbounds(int instance, int rez);
int mbview_drapesegment(int instance, struct mbview_linesegment_struct *seg);
int do_mbview_setbackgroundwork(int instance);
int do_mbview_settimer();
int do_mbview_workfunction(XtPointer client_data);
int mbview_setcolorparms(int instance);

/*--------------------------------------------------------------------*/
