/*--------------------------------------------------------------------
 *    The MB-system:	mbview.h	10/9/2002
 *
 *    Copyright (c) 2002-2025 by
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
 * Date:	October 10,  2002
 */

#ifndef MBVIEW_MBVIEW_H_
#define MBVIEW_MBVIEW_H_

#ifndef MB_STATUS_DEF
#include "mb_status.h"
#endif

#ifndef MB_DEFINE_DEF
#include "mb_define.h"
#endif

/* maximum number of mbview windows */
#define MBV_MAX_WINDOWS 10

/* no window / invalid instance flag */
#define MBV_NO_WINDOW 999

/* typical number of array elements to allocate at a time */
#define MBV_ALLOC_NUM 128

/* mouse mode defines */
#define MBV_MOUSE_MOVE 0
#define MBV_MOUSE_ROTATE 1
#define MBV_MOUSE_SHADE 2
#define MBV_MOUSE_VIEWPOINT 3
#define MBV_MOUSE_AREA 4
#define MBV_MOUSE_SITE 5
#define MBV_MOUSE_ROUTE 6
#define MBV_MOUSE_NAV 7
#define MBV_MOUSE_NAVFILE 8
#define MBV_MOUSE_VECTOR 9

/* projection mode */
#define MBV_PROJECTION_GEOGRAPHIC 0
#define MBV_PROJECTION_PROJECTED 1
#define MBV_PROJECTION_ALREADYPROJECTED 2
#define MBV_PROJECTION_SPHEROID 3
#define MBV_PROJECTION_ELLIPSOID 4

/* display mode defines */
#define MBV_DISPLAY_2D 0
#define MBV_DISPLAY_3D 1

/* data type defines */
#define MBV_DATA_PRIMARY 0
#define MBV_DATA_PRIMARYSLOPE 1
#define MBV_DATA_SECONDARY 2

/* grid view mode defines */
#define MBV_GRID_VIEW_PRIMARY 0
#define MBV_GRID_VIEW_PRIMARYSLOPE 1
#define MBV_GRID_VIEW_SECONDARY 2

/* shade view mode defines */
#define MBV_SHADE_VIEW_NONE 0
#define MBV_SHADE_VIEW_ILLUMINATION 1
#define MBV_SHADE_VIEW_SLOPE 2
#define MBV_SHADE_VIEW_OVERLAY 3

/* simple view mode defines for contours, sites, routes, etc */
#define MBV_VIEW_OFF 0
#define MBV_VIEW_ON 1

/* lon lat style mode */
#define MBV_LONLAT_DEGREESDECIMAL 0
#define MBV_LONLAT_DEGREESMINUTES 1

/* colortable view mode defines */
#define MBV_COLORTABLE_NORMAL 0
#define MBV_COLORTABLE_REVERSED 1

/* colortable view mode defines */
#define MBV_COLORTABLE_HAXBY 0
#define MBV_COLORTABLE_BRIGHT 1
#define MBV_COLORTABLE_MUTED 2
#define MBV_COLORTABLE_GRAY 3
#define MBV_COLORTABLE_FLAT 4
#define MBV_COLORTABLE_SEALEVEL1 5
#define MBV_COLORTABLE_SEALEVEL2 6

/* individual color defines */
#define MBV_COLOR_BLACK 0
#define MBV_COLOR_WHITE 1
#define MBV_COLOR_RED 2
#define MBV_COLOR_YELLOW 3
#define MBV_COLOR_GREEN 4
#define MBV_COLOR_BLUEGREEN 5
#define MBV_COLOR_BLUE 6
#define MBV_COLOR_PURPLE 7

/* default no data value define */
#define MBV_DEFAULT_NODATA -9999999.9

/* selection defines */
#define MBV_SELECT_NONE -1
#define MBV_SELECT_ALL -2

/* pick defines */
#define MBV_PICK_NONE 0
#define MBV_PICK_ONEPOINT 1
#define MBV_PICK_TWOPOINT 2
#define MBV_PICK_AREA 3
#define MBV_PICK_REGION 4
#define MBV_PICK_SITE 5
#define MBV_PICK_ROUTE 6
#define MBV_PICK_NAV 7
#define MBV_PICK_VECTOR 8

/* region defines */
#define MBV_REGION_REPICKWIDTH 2
#define MBV_REGION_NONE 0
#define MBV_REGION_ONEPOINT 1
#define MBV_REGION_QUAD 2
#define MBV_REGION_PICKCORNER0 0
#define MBV_REGION_PICKCORNER1 1
#define MBV_REGION_PICKCORNER2 2
#define MBV_REGION_PICKCORNER3 3

/* area defines */
#define MBV_AREA_REPICKWIDTH 2
#define MBV_AREA_NONE 0
#define MBV_AREA_ONEPOINT 1
#define MBV_AREA_QUAD 2
#define MBV_AREA_PICKENDPOINT0 0
#define MBV_AREA_PICKENDPOINT1 1

/* site defines */
#define MBV_SITE_OFF 0
#define MBV_SITE_VIEW 1
#define MBV_SITE_EDIT 2

/* route defines */
#define MBV_ROUTE_OFF 0
#define MBV_ROUTE_VIEW 1
#define MBV_ROUTE_EDIT 2
#define MBV_ROUTE_NAVADJUST 3
#define MBV_ROUTE_WAYPOINT_DELETEFLAG -1
#define MBV_ROUTE_WAYPOINT_NONE 0
#define MBV_ROUTE_WAYPOINT_SIMPLE 1
#define MBV_ROUTE_WAYPOINT_TRANSIT 2
#define MBV_ROUTE_WAYPOINT_STARTLINE 3
#define MBV_ROUTE_WAYPOINT_ENDLINE 4
#define MBV_ROUTE_WAYPOINT_STARTLINE2 5
#define MBV_ROUTE_WAYPOINT_ENDLINE2 6
#define MBV_ROUTE_WAYPOINT_STARTLINE3 7
#define MBV_ROUTE_WAYPOINT_ENDLINE3 8
#define MBV_ROUTE_WAYPOINT_STARTLINE4 9
#define MBV_ROUTE_WAYPOINT_ENDLINE4 10
#define MBV_ROUTE_WAYPOINT_STARTLINE5 11
#define MBV_ROUTE_WAYPOINT_ENDLINE5 12

/* nav defines */
#define MBV_NAV_OFF 0
#define MBV_NAV_VIEW 1
#define MBV_NAV_MBNAVADJUST 2

/* vector defines */
#define MBV_VECTOR_OFF 0
#define MBV_VECTOR_VIEW 1

/* stat masks */
#define MBV_STATMASK0 0x01
#define MBV_STATMASK1 0x02
#define MBV_STATMASK2 0x04
#define MBV_STATMASK3 0x08
#define MBV_STATMASK4 0x10
#define MBV_STATMASK5 0x20
#define MBV_STATMASK6 0x40
#define MBV_STATMASK7 0x80

/* pick sensitivity masks */
#define MBV_PICKMASK_NONE 0x0
#define MBV_PICKMASK_ONEPOINT 0x1
#define MBV_PICKMASK_TWOPOINT 0x2
#define MBV_PICKMASK_AREA 0x4
#define MBV_PICKMASK_REGION 0x8
#define MBV_PICKMASK_SITE 0x10
#define MBV_PICKMASK_ROUTE 0x20
#define MBV_PICKMASK_NAVONEPOINT 0x40
#define MBV_PICKMASK_NAVTWOPOINT 0x80
#define MBV_PICKMASK_NAVANY 0x100
#define MBV_PICKMASK_NEWINSTANCE 0x200
#define MBV_EXISTMASK_SITE 0x400
#define MBV_EXISTMASK_ROUTE 0x800
#define MBV_EXISTMASK_NAV 0x1000
#define MBV_STATEMASK_13 0x2000
#define MBV_STATEMASK_14 0x4000
#define MBV_STATEMASK_15 0x8000
#define MBV_STATEMASK_16 0x10000
#define MBV_STATEMASK_17 0x20000
#define MBV_STATEMASK_18 0x40000
#define MBV_STATEMASK_19 0x80000
#define MBV_STATEMASK_20 0x100000
#define MBV_STATEMASK_21 0x200000
#define MBV_STATEMASK_22 0x400000
#define MBV_STATEMASK_23 0x800000
#define MBV_STATEMASK_24 0x1000000
#define MBV_STATEMASK_25 0x2000000
#define MBV_STATEMASK_26 0x4000000
#define MBV_STATEMASK_27 0x8000000
#define MBV_STATEMASK_28 0x10000000
#define MBV_STATEMASK_29 0x20000000
#define MBV_STATEMASK_30 0x40000000
#define MBV_STATEMASK_31 0x80000000

/* profile defines */
#define MBV_PROFILE_NONE 0
#define MBV_PROFILE_TWOPOINT 1
#define MBV_PROFILE_ROUTE 2
#define MBV_PROFILE_NAV 3
#define MBV_PROFILE_FACTOR_MAX 4.0

/* mb3dsounding mouse mode value defines */
#define MB3DSDG_MOUSE_TOGGLE 0
#define MB3DSDG_MOUSE_PICK 1
#define MB3DSDG_MOUSE_ERASE 2
#define MB3DSDG_MOUSE_RESTORE 3
#define MB3DSDG_MOUSE_GRAB 4
#define MB3DSDG_MOUSE_INFO 5
#define MB3DSDG_EDIT_NOFLUSH 0
#define MB3DSDG_EDIT_FLUSH 1
#define MB3DSDG_EDIT_FLUSHPREVIOUS 2

#define MB3DSDG_OPTIMIZEBIASVALUES_NONE 0x00
#define MB3DSDG_OPTIMIZEBIASVALUES_R 0x01
#define MB3DSDG_OPTIMIZEBIASVALUES_P 0x02
#define MB3DSDG_OPTIMIZEBIASVALUES_H 0x04
#define MB3DSDG_OPTIMIZEBIASVALUES_T 0x08
#define MB3DSDG_OPTIMIZEBIASVALUES_S 0x10
#define MB3DSDG_OPTIMIZEBIASVALUES_RP 0x03
#define MB3DSDG_OPTIMIZEBIASVALUES_RPH 0x07
#define MB3DSDG_OPTIMIZEBIASVALUES_RPHT 0x0F

/*--------------------------------------------------------------------*/

/* structure declarations */
struct mbview_contoursegment_struct {
	float x[2];
	float y[2];
	float z;
	float level;
};

struct mbview_point_struct {
	double xgrid;
	double ygrid;
	double xlon;
	double ylat;
	double zdata;
	double xdisplay;
	double ydisplay;
	double zdisplay;
};

struct mbview_pointw_struct {
	double xgrid[MBV_MAX_WINDOWS];
	double ygrid[MBV_MAX_WINDOWS];
	double xlon;
	double ylat;
	double zdata;
	double xdisplay[MBV_MAX_WINDOWS];
	double ydisplay[MBV_MAX_WINDOWS];
	double zdisplay[MBV_MAX_WINDOWS];
};

struct mbview_navpoint_struct {
	int draped;
	int selected;
	double time_d;
	double heading;
	double speed;
	struct mbview_point_struct point;
	struct mbview_point_struct pointport;
	struct mbview_point_struct pointcntr;
	struct mbview_point_struct pointstbd;
	int line;
	int shot;
	int cdp;
};

struct mbview_navpointw_struct {
	int draped;
	int selected;
	double time_d;
	double heading;
	double speed;
	struct mbview_pointw_struct point;
	struct mbview_pointw_struct pointport;
	struct mbview_pointw_struct pointcntr;
	struct mbview_pointw_struct pointstbd;
	int line;
	int shot;
	int cdp;
};

struct mbview_vectorpointw_struct {
	int draped;
	int selected;
	struct mbview_pointw_struct point;
	double data;
};

struct mbview_profilepoint_struct {
	int boundary;
	double xgrid;
	double ygrid;
	double xlon;
	double ylat;
	double zdata;
	double distance;
	double distovertopo;
	double xdisplay;
	double ydisplay;
	double navzdata;
	double navtime_d;
	double slope;
	double bearing;
};

struct mbview_linesegment_struct {
	struct mbview_point_struct endpoints[2];
	int nls;
	int nls_alloc;
	struct mbview_point_struct *lspoints;
};

struct mbview_linesegmentw_struct {
	struct mbview_pointw_struct endpoints[2];
	int nls;
	int nls_alloc;
	struct mbview_pointw_struct *lspoints;
};

struct mbview_pick_struct {
	double range;
	double bearing;
	struct mbview_point_struct endpoints[2];
	struct mbview_point_struct xpoints[8];
	struct mbview_linesegment_struct segment;
	struct mbview_linesegment_struct xsegments[4];
};

struct mbview_pickw_struct {
	struct mbview_pointw_struct endpoints[2];
	struct mbview_pointw_struct xpoints[8];
	struct mbview_linesegmentw_struct segment;
	struct mbview_linesegmentw_struct xsegments[4];
};

struct mbview_region_struct {
	double width;
	double height;
	struct mbview_point_struct cornerpoints[4];
	struct mbview_linesegment_struct segments[4];
};

struct mbview_area_struct {
	double width;
	double length;
	double bearing;
	struct mbview_point_struct endpoints[2];
	struct mbview_linesegment_struct segment;
	struct mbview_point_struct cornerpoints[4];
	struct mbview_linesegment_struct segments[4];
};

struct mbview_site_struct {
  bool active;
	struct mbview_pointw_struct point;
	int color;
	int size;
	mb_path name;
};

struct mbview_route_struct {
  bool active;
	int color;
	int size;
	int editmode;
	mb_path name;
	double distancelateral;
	double distancetopo;
	int npoints;
	int npoints_alloc;
	int nroutepoint;
	int *waypoint;
	double *distlateral;
	double *disttopo;
	struct mbview_pointw_struct *points;
	struct mbview_linesegmentw_struct *segments;
};

struct mbview_nav_struct {
  bool active;
	int color;
	int size;
	mb_path name;
	int pathstatus;
	mb_path pathraw;
	mb_path pathprocessed;
	int format;
	int swathbounds;
	int line;
	int shot;
	int cdp;
	int decimation;
	int npoints;
	int npoints_alloc;
	int nselected;
	struct mbview_navpointw_struct *navpts;
	struct mbview_linesegmentw_struct *segments;
};

struct mbview_vector_struct {
  bool active;
	int color;
	int size;
	mb_path name;
	int format;
	int npoints;
	int npoints_alloc;
	int nselected;
	double datamin;
	double datamax;
	struct mbview_vectorpointw_struct *vectorpts;
	struct mbview_linesegmentw_struct *segments;
};

struct mbview_profile_struct {
	int source;
	mb_path source_name;
	double length;
	double zmin;
	double zmax;
	int npoints;
	int npoints_alloc;
	struct mbview_profilepoint_struct *points;
};

struct mbview_shareddata_struct {
	/* nav pick */
	int navpick_type;
	struct mbview_pickw_struct navpick;

	/* site data */
	int site_mode;
	int nsite;
	int nsite_alloc;
	int site_selected;
	struct mbview_site_struct *sites;

	/* route data */
	int route_mode;
	int nroute;
	int nroute_alloc;
	int route_selected;
	int route_point_selected;
	struct mbview_route_struct *routes;

	/* nav data */
	int nav_mode;
	int nnav;
	int nnav_alloc;
	int nav_selected[2];
	int nav_point_selected[2];
	int nav_selected_mbnavadjust[2];
	struct mbview_nav_struct *navs;

	/* vector data */
	int vector_mode;
	int nvector;
	int nvector_alloc;
	int vector_selected;
	int vector_point_selected;
	struct mbview_vector_struct *vectors;
};

struct mbview_struct {

	/* function pointers */
	int (*mbview_dismiss_notify)(size_t id);
	void (*mbview_pickonepoint_notify)(size_t id);
	void (*mbview_picktwopoint_notify)(size_t id);
	void (*mbview_pickarea_notify)(size_t id);
	void (*mbview_pickregion_notify)(size_t id);
	void (*mbview_picksite_notify)(size_t id);
	void (*mbview_pickroute_notify)(size_t id);
	void (*mbview_picknav_notify)(size_t id);
	void (*mbview_pickvector_notify)(size_t id);
	void (*mbview_sensitivity_notify)(void);
	void (*mbview_colorchange_notify)(size_t id);

	/* active flag */
	int active;

	/* main plot widget controls */
	mb_path title;
	int xo;
	int yo;
	int width;
	int height;
	int lorez_dimension;
	int hirez_dimension;
	int lorez_navdecimate;
	int hirez_navdecimate;

	/* profile plot widget controls */
	mb_path prtitle;
	int prwidth;
	int prheight;

	/* mode controls */
	int display_mode;
	int mouse_mode;
	int grid_mode;
	int grid_shade_mode;
	int grid_contour_mode;

	/* histogram equalization controls */
	int primary_histogram;
	int primaryslope_histogram;
	int secondary_histogram;

	/* colortable controls */
	int primary_colortable;
	int primary_colortable_mode;
	double primary_colortable_min;
	double primary_colortable_max;
	int primary_shade_mode;
	int slope_colortable;
	int slope_colortable_mode;
	double slope_colortable_min;
	double slope_colortable_max;
	int slope_shade_mode;
	int secondary_colortable;
	int secondary_colortable_mode;
	double secondary_colortable_min;
	double secondary_colortable_max;
	int secondary_shade_mode;

	/* view controls */
	double exageration;
	double modelelevation3d;
	double modelazimuth3d;
	double viewelevation3d;
	double viewazimuth3d;
	int viewbounds[4];

	/* shading controls */
	double illuminate_magnitude;
	double illuminate_elevation;
	double illuminate_azimuth;
	double slope_magnitude;
	double overlay_shade_magnitude;
	double overlay_shade_center;
	int overlay_shade_mode;

	/* contour controls */
	double contour_interval;

	/* profile controls */
	double profile_exageration;
	int profile_widthfactor;
	double profile_slopethreshold;

	/* projection controls */
	int primary_grid_projection_mode;
	mb_path primary_grid_projection_id;
	int secondary_grid_projection_mode;
	mb_path secondary_grid_projection_id;
	int display_projection_mode;
	mb_path display_projection_id;

	/* grid data */
	float primary_nodatavalue;
	int primary_nxy;
	int primary_n_columns;
	int primary_n_rows;
	double primary_min;
	double primary_max;
	double primary_xmin;
	double primary_xmax;
	double primary_ymin;
	double primary_ymax;
	double primary_dx;
	double primary_dy;
	float *primary_data;
	float *primary_x;
	float *primary_y;
	float *primary_z;
	float *primary_dzdx;
	float *primary_dzdy;
	float *primary_r;
	float *primary_g;
	float *primary_b;
	char *primary_stat_color;
	char *primary_stat_z;
	bool secondary_sameas_primary;
	float secondary_nodatavalue;
	int secondary_nxy;
	int secondary_n_columns;
	int secondary_n_rows;
	double secondary_min;
	double secondary_max;
	double secondary_xmin;
	double secondary_xmax;
	double secondary_ymin;
	double secondary_ymax;
	double secondary_dx;
	double secondary_dy;
	float *secondary_data;

	/* pick info flag */
	int pickinfo_mode;

	/* point and line pick */
	int pick_type;
	struct mbview_pick_struct pick;

	/* area data */
	int area_type;
	int area_pickendpoint;
	struct mbview_area_struct area;

	/* region data */
	int region_type;
	int region_pickcorner;
	struct mbview_region_struct region;

	/* profile data */
	struct mbview_profile_struct profile;

	/* global data view modes */
	int site_view_mode;
	int route_view_mode;
	int nav_view_mode;
	int navswathbounds_view_mode;
	int navdrape_view_mode;
	int vector_view_mode;
	int profile_view_mode;

  /* general use state variables to turn action button sensitivity on and off */
  int state13;
  int state14;
  int state15;
  int state16;
  int state17;
  int state18;
  int state19;
  int state20;
  int state21;
  int state22;
  int state23;
  int state24;
  int state25;
  int state26;
  int state27;
  int state28;
  int state29;
  int state30;
  int state31;
};

/*--------------------------------------------------------------------*/

/* mb3dsounding data structures */
struct mb3dsoundings_sounding_struct {
	int ifile;
	int iping;
	int ibeam;
	int beamcolor;
	char beamflag;
	char beamflagorg;
	double x;
	double y;
	double z;
  double a;
	float glx;
	float gly;
	float glz;
  float r;
  float g;
  float b;
	int winx;
	int winy;
};

struct mb3dsoundings_struct {
  /* display flag */
  bool displayed;

	/* location and scale parameters */
	double xorigin;
	double yorigin;
	double zorigin;
	double xmin;
	double ymin;
	double zmin;
	double xmax;
	double ymax;
	double zmax;
	double bearing;
	double sinbearing;
	double cosbearing;
	double scale;
	double zscale;

	/* sounding data */
	int num_soundings;
	int num_soundings_unflagged;
	int num_soundings_flagged;
	int num_soundings_alloc;
	struct mb3dsoundings_sounding_struct *soundings;
};

/*--------------------------------------------------------------------*/

/* mbview_callbacks.c function prototypes */
int mbview_startup(int verbose, Widget parent, XtAppContext app, int *error);
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
int mbview_set_widgets(int verbose, size_t instance, int *error);
int mbview_addaction(int verbose, size_t instance, void(mbview_action_notify)(Widget, XtPointer, XtPointer), char *label,
                     int sensitive, int *error);
int mbview_setstate(int verbose, size_t instance, int mask, int value, int *error);
int mbview_addpicknotify(int verbose, size_t instance, int picktype, void(mbview_pick_notify)(size_t), int *error);
int mbview_setsensitivitynotify(int verbose, size_t instance, void(mbview_sensitivity_notify)(), int *error);
int mbview_setcolorchangenotify(int verbose, size_t instance, void(mbview_colorchange_notify)(size_t), int *error);
void mbview_resize(Widget w, XtPointer client_data, XEvent *event, Boolean *unused);
int mbview_destroy(int verbose, size_t instance, bool destroywidgets, int *error);
int mbview_quit(int verbose, int *error);
int do_mbview_message_on(char *message, size_t instance);
int do_mbview_message_off(size_t instance);
void set_mbview_label_string(Widget w, String str);
void set_mbview_label_multiline_string(Widget w, String str);

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
int mbview_getzdata(size_t instance, double xgrid, double ygrid, bool *found, double *zdata);
int mbview_colorvalue_instance(size_t instance, double value, float *r, float *g, float *b);

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
int mbview_drapesegment(size_t instance, struct mbview_linesegment_struct *seg);

/* mbview_pick.c function prototypes */
int mbview_clearpicks(size_t instance);
int mbview_clearnavpicks(size_t instance);

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
int mbview_enableadjustnavs(int verbose, size_t instance, int *error);
int mbview_picknavbyname(int verbose, size_t instance, char *name, int *error);
int mbview_setnavactivebyname(int verbose, size_t instance, char *name, bool active, bool updatelist, int *error);

/* mbview_vector.c function prototypes */
int mbview_getvectorcount(int verbose, size_t instance, int *nvec, int *error);
int mbview_getvectorpointcount(int verbose, size_t instance, int vec, int *npoint, int *nintpoint, int *error);
int mbview_allocvectorarrays(int verbose, int npointtotal, double **veclon, double **veclat, double **vecz, double **vecdata,
                             int *error);
int mbview_freevectorarrays(int verbose, double **veclon, double **veclat, double **vecz, double **vecdata, int *error);
int mbview_addvector(int verbose, size_t instance, int npoint, double *veclon, double *veclat, double *vecz, double *vecdata,
                     int veccolor, int vecsize, mb_path vecname, double vecdatamin, double vecdatamax, int *error);
int mbview_enableviewvectors(int verbose, size_t instance, int *error);
int mbview_pick_vector_select(size_t instance, int select, int which, int xpixel, int ypixel);
int mbview_vector_delete(size_t instance, int ivec);
int mbview_drawvectorpick(size_t instance);
int mbview_drawvector(size_t instance, int rez);

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
int mbview_deleteallroutes(int verbose, size_t instance, int *error);
int mbview_getroute(int verbose, size_t instance, int route, int *npointtotal, double *routelon, double *routelat, int *waypoint,
                    double *routetopo, double *routebearing, double *distlateral, double *distovertopo, double *slope,
                    int *routecolor, int *routesize, int *routeeditmode, mb_path routename, int *error);
int mbview_enableviewroutes(int verbose, size_t instance, int *error);
int mbview_enableeditroutes(int verbose, size_t instance, int *error);
int mbview_enableviewties(int verbose, size_t instance, int *error);
int mbview_route_add(int verbose, size_t instance, int inew, int jnew, int waypoint, double xgrid, double ygrid, double xlon,
                     double ylat, double zdata, double xdisplay, double ydisplay, double zdisplay);
int mbview_updateroutelist(void);
int mbview_pick_routebyname(int verbose, size_t instance, char *name, int *error);
int mbview_pick_route_select(int verbose, size_t instance, int which, int xpixel, int ypixel);
int mbview_pick_route_delete(int verbose, size_t instance, int xpixel, int ypixel);

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

/*--------------------------------------------------------------------*/

/* mb3dsounding API functions */
int mb3dsoundings_startup(int verbose, Widget parent, XtAppContext app, int *error);
int mb3dsoundings_open(int verbose, struct mb3dsoundings_struct *soundingdata, int *error);
int mb3dsoundings_end(int verbose, int *error);
int mb3dsoundings_set_dismiss_notify(int verbose, void(dismiss_notify)(), int *error);
int mb3dsoundings_set_edit_notify(int verbose, void(edit_notify)(int, int, int, char, int), int *error);
int mb3dsoundings_set_info_notify(int verbose, void(edit_notify)(int, int, int, char *), int *error);
int mb3dsoundings_set_bias_notify(int verbose, void(bias_notify)(double, double, double, double, double), int *error);
int mb3dsoundings_set_biasapply_notify(int verbose, void(biasapply_notify)(double, double, double, double, double), int *error);
int mb3dsoundings_set_flagsparsevoxels_notify(int verbose, void(flagsparsevoxels_notify)(int, int), int *error);
int mb3dsoundings_set_colorsoundings_notify(int verbose, void(colorsoundings_notify)(int), int *error);
int mb3dsoundings_set_optimizebiasvalues_notify(int verbose,
                                                void(optimizebiasvalues_notify)(int, double *, double *, double *, double *, double *),
                                                int *error);
int mb3dsoundings_plot(int verbose, int *error);
int mb3dsoundings_get_bias_values(int verbose, double *rollbias, double *pitchbias, double *headingbias, double *timelag,
                                  double *snell, int *error);

#endif  // MBVIEW_MBVIEW_H_
