/*--------------------------------------------------------------------
 *    The MB-system:	mbview.h	10/9/2002
 *    $Id: mbview.h,v 5.2 2004-02-24 22:52:29 caress Exp $
 *
 *    Copyright (c) 2002, 2003 by
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
 * Date:	October 10,  2002
 *
 * $Log: not supported by cvs2svn $
 * Revision 5.1  2004/01/06 21:11:03  caress
 * Added pick region capability.
 *
 * Revision 5.0  2003/12/02 20:38:31  caress
 * Making version number 5.0
 *
 * Revision 1.3  2003/11/25 01:43:18  caress
 * MBview version generated during EW0310.
 *
 * Revision 1.1  2003/09/23 21:29:00  caress
 * Adding first cut on mbview to cvs.
 *
 *
 */

/*--------------------------------------------------------------------*/

#ifndef MB_STATUS_DEF
#include "../../include/mb_status.h"
#endif

#ifndef MB_DEFINE_DEF
#include "../../include/mb_define.h"
#endif

#ifdef MBVIEW_DECLARE_GLOBALS
#define EXTERNAL
#else
#define EXTERNAL extern
#endif

/* maximum number of mbview windows */
#define MBV_MAX_WINDOWS		10

/* typical number of array elements to allocate at a time */
#define MBV_ALLOC_NUM		128

/* mouse mode defines */
#define MBV_MOUSE_MOVE		0
#define MBV_MOUSE_ROTATE	1
#define MBV_MOUSE_SHADE		2
#define MBV_MOUSE_VIEWPOINT	3
#define MBV_MOUSE_AREA		4
#define MBV_MOUSE_SITE		5
#define MBV_MOUSE_ROUTE		6
#define MBV_MOUSE_NAV		7

/* projection mode */
#define	MBV_PROJECTION_GEOGRAPHIC	0
#define	MBV_PROJECTION_PROJECTED	1
#define	MBV_PROJECTION_ALREADYPROJECTED	2
#define	MBV_PROJECTION_SPHEROID		3
#define	MBV_PROJECTION_ELLIPSOID	4

/* display mode defines */
#define	MBV_DISPLAY_2D		0
#define	MBV_DISPLAY_3D		1

/* grid view mode defines */
#define	MBV_GRID_VIEW_PRIMARY		0
#define	MBV_GRID_VIEW_PRIMARYSLOPE	1
#define	MBV_GRID_VIEW_SECONDARY		2

/* shade view mode defines */
#define	MBV_SHADE_VIEW_NONE		0
#define	MBV_SHADE_VIEW_ILLUMINATION	1
#define	MBV_SHADE_VIEW_SLOPE		2
#define	MBV_SHADE_VIEW_OVERLAY		3

/* simple view mode defines for contours, sites, routes, etc */
#define	MBV_VIEW_OFF		0
#define	MBV_VIEW_ON		1

/* colortable view mode defines */
#define	MBV_COLORTABLE_NORMAL		0
#define	MBV_COLORTABLE_REVERSED		1

/* colortable view mode defines */
#define	MBV_COLORTABLE_HAXBY		0
#define	MBV_COLORTABLE_BRIGHT		1
#define	MBV_COLORTABLE_MUTED		2
#define	MBV_COLORTABLE_GRAY		3
#define	MBV_COLORTABLE_FLAT		4
#define	MBV_COLORTABLE_SEALEVEL		5

/* individual color defines */
#define	MBV_COLOR_BLACK			0
#define	MBV_COLOR_WHITE			1
#define	MBV_COLOR_RED			2
#define	MBV_COLOR_YELLOW		3
#define	MBV_COLOR_GREEN			4
#define	MBV_COLOR_BLUEGREEN		5
#define	MBV_COLOR_BLUE			6
#define	MBV_COLOR_PURPLE		7

/* default no data value define */
#define	MBV_DEFAULT_NODATA		-9999999.9

/* selection defines */
#define MBV_SELECT_NONE			-1

/* pick defines */
#define MBV_PICK_NONE			0
#define MBV_PICK_ONEPOINT		1
#define MBV_PICK_TWOPOINT		2
#define MBV_PICK_AREA			3
#define MBV_PICK_REGION			4
#define MBV_PICK_SITE			5
#define MBV_PICK_ROUTE			6
#define MBV_PICK_NAV			7

/* region defines */
#define MBV_REGION_NONE			0
#define MBV_REGION_ONEPOINT		1
#define MBV_REGION_QUAD			2

/* area defines */
#define MBV_AREA_NONE			0
#define MBV_AREA_ONEPOINT		1
#define MBV_AREA_QUAD			2

/* site defines */
#define MBV_SITE_OFF			0
#define MBV_SITE_VIEW			1
#define MBV_SITE_EDIT			2

/* route defines */
#define MBV_ROUTE_OFF			0
#define MBV_ROUTE_VIEW			1
#define MBV_ROUTE_EDIT			2

/* nav defines */
#define MBV_NAV_OFF			0
#define MBV_NAV_VIEW			1

/* stat masks */
#define MBV_STATMASK0	0x01
#define MBV_STATMASK1	0x02
#define MBV_STATMASK2	0x04
#define MBV_STATMASK3	0x08
#define MBV_STATMASK4	0x10
#define MBV_STATMASK5	0x20
#define MBV_STATMASK6	0x40
#define MBV_STATMASK7	0x80

/* pick sensitivity masks */
#define MBV_PICKMASK_NONE			0
#define MBV_PICKMASK_ONEPOINT			1
#define MBV_PICKMASK_TWOPOINT			2
#define MBV_PICKMASK_AREA			4
#define MBV_PICKMASK_REGION			8
#define MBV_PICKMASK_SITE			16
#define MBV_PICKMASK_ROUTE			32
#define MBV_PICKMASK_NAVONEPOINT		64
#define MBV_PICKMASK_NAVTWOPOINT		128
#define MBV_PICKMASK_NAVANY			256

/* structure declarations */
struct mbview_contoursegment_struct {
	float	x[2];
	float	y[2];
	float	z;
	float	level;
	};
	
struct mbview_point_struct {
	double	xgrid;
	double	ygrid;
	double	xlon;
	double	ylat;
	double	zdata;
	double	xdisplay;
	double	ydisplay;
	double	zdisplay;
	};
	
struct mbview_navpoint_struct {
	int	draped;
	int	selected;
	double	time_d;
	double	heading;
	double	speed;
	struct mbview_point_struct point;
	struct mbview_point_struct pointport;
	struct mbview_point_struct pointcntr;
	struct mbview_point_struct pointstbd;
	int	shot;
	int	cdp;
	};

struct mbview_linesegment_struct {
	struct mbview_point_struct *endpoints[2];
	int	nls;
	int	nls_alloc;
	struct mbview_point_struct *lspoints;
	};

struct mbview_pick_struct {
	struct mbview_point_struct endpoints[2];
	struct mbview_point_struct xpoints[8];
	struct mbview_linesegment_struct segment;
	struct mbview_linesegment_struct xsegments[4];
	};

struct mbview_region_struct {
	double	width;
	double	height;
	struct mbview_point_struct cornerpoints[4];
	struct mbview_linesegment_struct segments[4];
	};

struct mbview_area_struct {
	double	width;
	double	length;
	double	bearing;
	struct mbview_point_struct endpoints[2];
	struct mbview_linesegment_struct segment;
	struct mbview_point_struct cornerpoints[4];
	struct mbview_linesegment_struct segments[4];
	};
	
struct mbview_site_struct {
	struct mbview_point_struct point;
	int	color;
	int	size;
	mb_path	name;
	};

struct mbview_route_struct {
	int	color;
	int	size;
	mb_path	name;
	int	npoints;
	int	npoints_alloc;
	struct mbview_point_struct *points;
	struct mbview_linesegment_struct *segments;
	};

struct mbview_nav_struct {
	int	color;
	int	size;
	mb_path	name;
	int	swathbounds;
	int	shot;
	int	cdp;
	int	npoints;
	int	npoints_alloc;
	int	nselected;
	struct mbview_navpoint_struct *navpts;
	struct mbview_linesegment_struct *segments;
	};

struct mbview_struct {
	
	/* function pointers */
	int (*mbview_dismiss_notify)(int id);
	
	/* active flag */
	int	active;

	/* widget controls */
	mb_path	title;
	int	xo;
	int	yo;
	int	width;
	int	height;
	int	lorez_dimension;
	int	hirez_dimension;

	/* mode controls */
	int	display_mode;
	int	mouse_mode;
	int	grid_mode;
	int	grid_shade_mode;
	int	grid_contour_mode;
	
	/* colortable controls */
	int	primary_colortable;
	int	primary_colortable_mode;
	double	primary_colortable_min;
	double	primary_colortable_max;
	int	primary_shade_mode;
	int	slope_colortable;
	int	slope_colortable_mode;
	double	slope_colortable_min;
	double	slope_colortable_max;
	int	slope_shade_mode;
	int	secondary_colortable;
	int	secondary_colortable_mode;
	double	secondary_colortable_min;
	double	secondary_colortable_max;
	int	secondary_shade_mode;

	/* view controls */
	double	exageration;
	double	modelelevation3d;
	double	modelazimuth3d;
	double	viewelevation3d;
	double	viewazimuth3d;
	int	viewbounds[4];
	
	/* shading controls */
	double	illuminate_magnitude;
	double	illuminate_elevation;
	double	illuminate_azimuth;
	double	slope_magnitude;
	double	overlay_shade_magnitude;
	double	overlay_shade_center;
	int	overlay_shade_mode;
	
	/* contour controls */
	double	contour_interval;
	
	/* projection controls */
	int	primary_grid_projection_mode;
	mb_path	primary_grid_projection_id;
	int	secondary_grid_projection_mode;
	mb_path	secondary_grid_projection_id;
	int	display_projection_mode;
	mb_path	display_projection_id;
	
	/* grid data */
	float	primary_nodatavalue;
	int	primary_nxy;
	int	primary_nx;
	int	primary_ny;
	double	primary_min;
	double	primary_max;
	double	primary_xmin;
	double	primary_xmax;
	double	primary_ymin;
	double	primary_ymax;
	double	primary_dx;
	double	primary_dy;
	float	*primary_data;
	float	*primary_x;
	float	*primary_y;
	float	*primary_z;
	float	*primary_dzdx;
	float	*primary_dzdy;
	float	*primary_r;
	float	*primary_g;
	float	*primary_b;
	char	*primary_stat_color;
	char	*primary_stat_z;
	int	secondary_sameas_primary;
	float	secondary_nodatavalue;
	int	secondary_nxy;
	int	secondary_nx;
	int	secondary_ny;
	double	secondary_min;
	double	secondary_max;
	double	secondary_xmin;
	double	secondary_xmax;
	double	secondary_ymin;
	double	secondary_ymax;
	double	secondary_dx;
	double	secondary_dy;
	float	*secondary_data;
	
	/* pick info flag */
	int	pickinfo_mode;
	
	/* point and line pick */
	int	pick_type;
	struct mbview_pick_struct pick;
	
	/* area data */
	int	area_type;
	struct mbview_area_struct area;
	
	/* region data */
	int	region_type;
	struct mbview_region_struct region;
	
	/* nav pick */
	int	navpick_type;
	struct mbview_pick_struct navpick;
	
	/* site data */
	int	site_mode;
	int	site_view_mode;
	int	nsite;
	int	nsite_alloc;
	int	site_selected;
	struct mbview_site_struct *sites;
	
	/* route data */
	int	route_mode;
	int	route_view_mode;
	int	nroute;
	int	nroute_alloc;
	int	route_selected;
	int	route_point_selected;
	struct mbview_route_struct *routes;
	
	/* nav data */
	int	nav_mode;
	int	nav_view_mode;
	int	navdrape_view_mode;
	int	nnav;
	int	nnav_alloc;
	int	nav_selected[2];
	int	nav_point_selected[2];
	struct mbview_nav_struct *navs;
	};

/* mbview API function prototypes */
int mbview_startup(int verbose, Widget parent, XtAppContext app, int *error);
int mbview_quit(int verbose, int *error);
int mbview_init(int verbose, int *instance, int *error);
int mbview_getdataptr(int verbose, int instance, struct mbview_struct **datahandle, int *error);

int mbview_setwindowparms(int verbose, int instance,
			int	(*mbview_dismiss_notify)(int),
			char	*title,
			int	xo,
			int	yo,
			int	width,
			int	height,
			int	lorez_dimension,
			int	hirez_dimension,
			int	*error);
int mbview_setviewcontrols(int verbose, int instance,
			int	display_mode,
			int	mouse_mode,
			int	grid_mode,
			int	primary_shade_mode,
			int	slope_shade_mode,
			int	secondary_shade_mode,
			int	grid_contour_mode,
			int	site_view_mode,
			int	route_view_mode,
			int	nav_view_mode,
			int	navdrape_view_mode,
			double	exageration,
			double	modelelevation3d,
			double	modelazimuth3d,
			double	viewelevation3d,
			double	viewazimuth3d,
			double	illuminate_magnitude,
			double	illuminate_elevation,
			double	illuminate_azimuth,
			double	slope_magnitude,
			double	overlay_shade_magnitude,
			double	overlay_shade_center,
			int	overlay_shade_mode,
			double	contour_interval,
			int	display_projection_mode,
			char	*display_projection_id,
			int *error);
int mbview_setprimarygrid(int verbose, int instance,
			int	primary_grid_projection_mode,
			char	*primary_grid_projection_id,
			float	primary_nodatavalue,
			int	primary_nx,
			int	primary_ny,
			double	primary_min,
			double	primary_max,
			double	primary_xmin,
			double	primary_xmax,
			double	primary_ymin,
			double	primary_ymax,
			double	primary_dx,
			double	primary_dy,
			float	*primary_data,
			int *error);
int mbview_setsecondarygrid(int verbose, int instance,
			int	secondary_grid_projection_mode,
			char	*secondary_grid_projection_id,
			float	secondary_nodatavalue,
			int	secondary_nx,
			int	secondary_ny,
			double	secondary_min,
			double	secondary_max,
			double	secondary_xmin,
			double	secondary_xmax,
			double	secondary_ymin,
			double	secondary_ymax,
			double	secondary_dx,
			double	secondary_dy,
			float	*secondary_data,
			int *error);
int mbview_setprimarycolortable(int verbose, int instance,
			int	primary_colortable,
			int	primary_colortable_mode,
			double	primary_colortable_min,
			double	primary_colortable_max,
			int *error);
int mbview_setslopecolortable(int verbose, int instance,
			int	slope_colortable,
			int	slope_colortable_mode,
			double	slope_colortable_min,
			double	slope_colortable_max,
			int *error);
int mbview_setsecondarycolortable(int verbose, int instance,
			int	secondary_colortable,
			int	secondary_colortable_mode,
			double	secondary_colortable_min,
			double	secondary_colortable_max,
			double	overlay_shade_magnitude,
			double	overlay_shade_center,
			int	overlay_shade_mode,
			int *error);
int mbview_getsitecount(int verbose, int instance,
			int	*nsite,
			int *error);
int mbview_allocsitearrays(int verbose, 
			int	nsite,
			double	**sitelon,
			double	**sitelat,
			double	**sitetopo,
			int	**sitecolor,
			int	**sitesize,
			mb_path	**sitename,
			int *error);
int mbview_freesitearrays(int verbose,
			double	**sitelon,
			double	**sitelat,
			double	**sitetopo,
			int	**sitecolor,
			int	**sitesize,
			mb_path	**sitename,
			int *error);
int mbview_addsites(int verbose, int instance,
			int	nsite,
			double	*sitelon,
			double	*sitelat,
			double	*sitetopo,
			int	*sitecolor,
			int	*sitesize,
			mb_path	*sitename,
			int *error);
int mbview_getsites(int verbose, int instance,
			int	*nsite,
			double	*sitelon,
			double	*sitelat,
			double	*sitetopo,
			int	*sitecolor,
			int	*sitesize,
			mb_path	*sitename,
			int *error);
int mbview_getroutecount(int verbose, int instance,
			int	*nroute,
			int *error);
int mbview_getroutepointcount(int verbose, int instance,
			int	route,
			int	*npoint,
			int	*nintpoint,
			int *error);
int mbview_allocroutearrays(int verbose, 
			int	npointtotal,
			double	**routelon,
			double	**routelat,
			int	**interpolated,
			double	**routetopo,
			double	**distlateral,
			double	**distovertopo,
			double	**slope,
			int *error);
int mbview_freeroutearrays(int verbose,
			double	**routelon,
			double	**routelat,
			int	**interpolated,
			double	**routetopo,
			double	**distlateral,
			double	**distovertopo,
			double	**slope,
			int *error);
int mbview_addroute(int verbose, int instance,
			int	npoint,
			double	*routelon,
			double	*routelat,
			int	routecolor,
			int	routesize,
			mb_path	routename,
			int *error);
int mbview_getroute(int verbose, int instance,
			int	route,
			int	*npointtotal,
			double	*routelon,
			double	*routelat,
			int	*interpolated,
			double	*routetopo,
			double	*distlateral,
			double	*distovertopo,
			double	*slope,
			int	*routecolor,
			int	*routesize,
			mb_path	routename,
			int *error);
int mbview_getnavpointcount(int verbose, int instance,
			int	nav,
			int	*npoint,
			int	*nintpoint,
			int *error);
int mbview_getnavcount(int verbose, int instance,
			int *nnav,
			int *error);
int mbview_allocnavarrays(int verbose, 
			int	npointtotal,
			double	**time_d,
			double	**navlon,
			double	**navlat,
			double	**navz,
			double	**navheading,
			double	**navspeed,
			double	**navportlon,
			double	**navportlat,
			double	**navstbdlon,
			double	**navstbdlat,
			int	**cdp,
			int	**shot,
			int *error);
int mbview_freenavarrays(int verbose,
			double	**time_d,
			double	**navlon,
			double	**navlat,
			double	**navz,
			double	**navheading,
			double	**navspeed,
			double	**navportlon,
			double	**navportlat,
			double	**navstbdlon,
			double	**navstbdlat,
			int	**cdp,
			int	**shot,
			int *error);
int mbview_addnav(int verbose, int instance,
			int	npoint,
			double	*time_d,
			double	*navlon,
			double	*navlat,
			double	*navz,
			double	*navheading,
			double	*navspeed,
			double	*navportlon,
			double	*navportlat,
			double	*navstbdlon,
			double	*navstbdlat,
			int	*cdp,
			int	*shot,
			int	navcolor,
			int	navsize,
			mb_path	navname,
			int	navswathbounds,
			int	navshot,
			int	navcdp,
			int *error);
int mbview_enableviewsites(int verbose, int instance,
			int *error);
int mbview_enableeditsites(int verbose, int instance,
			int *error);
int mbview_enableviewroutes(int verbose, int instance,
			int *error);
int mbview_enableeditroutes(int verbose, int instance,
			int *error);
int mbview_enableviewnavs(int verbose, int instance,
			int *error);
int mbview_addaction(int verbose, int instance,
			void	(mbview_action_notify)(Widget, XtPointer, XtPointer),
			char	*label,
			int	sensitive,
			int *error);

int mbview_open(int verbose, int instance, int *error);
int mbview_update(int verbose, int instance, int *error);
int mbview_destroy(int verbose, int instance, int destroywidgets, int *error);

int mbview_pick_site_select(int instance, int which, int xpixel, int ypixel);
int mbview_pick_site_add(int instance, int which, int xpixel, int ypixel);
int mbview_pick_site_delete(int instance, int xpixel, int ypixel);
int mbview_pick_route_select(int instance, int which, int xpixel, int ypixel);
int mbview_pick_route_add(int instance, int which, int xpixel, int ypixel);
int mbview_pick_route_delete(int instance, int xpixel, int ypixel);
	
/*--------------------------------------------------------------------*/
