/*--------------------------------------------------------------------
 *    The MB-system:  mbgrdviz_core.h  11/7/2026
 *
 *    Copyright (c) 2026 by
 *    Christian dos Santos Ferreira
 *      MARUM
 *      University of Bremen
 *      Bremen Germany
 *    David W. Caress (caress@mbari.org)
 *      Monterey Bay Aquarium Research Institute
 *      Moss Landing, California, USA
 *    Dale N. Chayes
 *      Center for Coastal and Ocean Mapping
 *      University of New Hampshire
 *      Durham, New Hampshire, USA
 *
 *    MB-System was created by Caress and Chayes in 1992 at the
 *      Lamont-Doherty Earth Observatory
 *      Columbia University
 *      Palisades, NY 10964
 *
 *    See README.md file for copying and redistribution conditions.
 *--------------------------------------------------------------------*/
/*
 * The toolkit-free part of mbgrdviz, shared by mbgrdviz (Motif) and
 * mbgrdviz-qt: opening the grids, sites, routes, vectors, navigation and GIS
 * data, saving sites, routes, profiles and GIS data in their formats, and the
 * constants of the program. Each program provides the user interface
 * functions declared at the end.
 */

#ifndef MBGRDVIZ_MBGRDVIZ_CORE_H
#define MBGRDVIZ_MBGRDVIZ_CORE_H

#include <stdbool.h>
#include <stddef.h>

#include "mb_aux.h"
#include "mbview.h"

#ifdef __cplusplus
extern "C" {
#endif

#define MBGRDVIZ_OPENGRID 0
#define MBGRDVIZ_OPENOVERLAY 1
#define MBGRDVIZ_OPENSITE 2
#define MBGRDVIZ_OPENROUTE 3
#define MBGRDVIZ_OPENVECTOR 4
#define MBGRDVIZ_OPENNAV 5
#define MBGRDVIZ_OPENSWATH 6
#define MBGRDVIZ_SAVEROUTE 7
#define MBGRDVIZ_SAVEROUTEREVERSED 8
#define MBGRDVIZ_SAVERISISCRIPTHEADING 9
#define MBGRDVIZ_SAVERISISCRIPTNOHEADING 10
#define MBGRDVIZ_SAVERISI2SCRIPTHEADING 11
#define MBGRDVIZ_SAVERISI2SCRIPTNOHEADING 12
#define MBGRDVIZ_SAVEDEGDECMIN 13
#define MBGRDVIZ_SAVELNW 14
#define MBGRDVIZ_SAVEGREENSEAYML 15
#define MBGRDVIZ_SAVETECDISLST 16
#define MBGRDVIZ_SAVEKONGSBERGDP 17
#define MBGRDVIZ_SAVESISASCIIPLAN1 18
#define MBGRDVIZ_SAVESISASCIIPLAN2 19
#define MBGRDVIZ_SAVESITE 20
#define MBGRDVIZ_SAVEPROFILE 21
#define MBGRDVIZ_REALTIME 22
#define MBGRDVIZ_OPENPOLYLINE 23
#define MBGRDVIZ_OPENPOLYGON 24
#define MBGRDVIZ_OPENGISPOINT 25
#define MBGRDVIZ_IMPORTSHAPEFILE 26
#define MBGRDVIZ_SAVEPOLYLINE 27
#define MBGRDVIZ_SAVEPOLYGON 28
#define MBGRDVIZ_SAVEGISPOINT 29
#define MBGRDVIZ_EXPORTPOLYLINE 30
#define MBGRDVIZ_EXPORTPOLYGON 31
#define MBGRDVIZ_EXPORTGISPOINT 32

/* colors and sizes for imported shapefile features without COLOR and SIZE attributes */
#define MBGRDVIZ_GIS_POLYLINE_COLOR MBV_COLOR_BLUEGREEN
#define MBGRDVIZ_GIS_POLYGON_COLOR MBV_COLOR_YELLOW
#define MBGRDVIZ_GIS_GISPOINT_COLOR MBV_COLOR_GREEN
#define MBGRDVIZ_GIS_LINE_SIZE 2
#define MBGRDVIZ_GIS_GISPOINT_SIZE 1

/* Projection defines */
#define ModelTypeProjected 1
#define ModelTypeGeographic 2
#define GCS_WGS_84 4326

/* Site and route file versions */
#define MBGRDVIZ_SITE_VERSION_MAJOR 2
#define MBGRDVIZ_SITE_VERSION_MINOR 0
#define MBGRDVIZ_ROUTE_VERSION_MAJOR 2
#define MBGRDVIZ_ROUTE_VERSION_MINOR 0
#define MBGRDVIZ_PROFILE_VERSION "1.00"
#define MBGRDVIZ_RISISCRIPT_VERSION "1.00"
#define MBGRDVIZ_RISI2SCRIPT_VERSION "2.00"

/* Survey planning parameters */
#define MBGRDVIZ_SURVEY_MODE_UNIFORM 0
#define MBGRDVIZ_SURVEY_MODE_VARIABLE 1
#define MBGRDVIZ_SURVEY_PLATFORM_SURFACE 0
#define MBGRDVIZ_SURVEY_PLATFORM_SUBMERGED_ALTITUDE 1
#define MBGRDVIZ_SURVEY_PLATFORM_SUBMERGED_DEPTH 2
#define MBGRDVIZ_SURVEY_DIRECTION_SW 0
#define MBGRDVIZ_SURVEY_DIRECTION_SE 1
#define MBGRDVIZ_SURVEY_DIRECTION_NW 2
#define MBGRDVIZ_SURVEY_DIRECTION_NE 3
#define MBGRDVIZ_SURVEY_MAXLINES 1000
#define MBGRDVIZ_REALTIME_ICON_SHIP 0
#define MBGRDVIZ_REALTIME_ICON_ROV 1
#define MBGRDVIZ_REALTIME_ICON_AUV 2
#define MBGRDVIZ_REALTIME_OFF 0
#define MBGRDVIZ_REALTIME_ON 1
#define MBGRDVIZ_REALTIME_PAUSE 2

/* opening and saving (mbgrdviz_core.c) */
int mbgrdviz_openfile(size_t mode, size_t instance, char *file_ptr);
void do_mbgrdviz_addgisactions(size_t instance);
int do_mbgrdviz_dismiss_notify(size_t instance);
int do_mbgrdviz_openprimary(char *input_file_ptr);
int do_mbgrdviz_openoverlay(size_t instance, char *input_file_ptr);
int do_mbgrdviz_opensite(size_t instance, char *input_file_ptr);
int do_mbgrdviz_savesite(size_t instance, char *output_file_ptr);
int do_mbgrdviz_openroute(size_t instance, char *input_file_ptr);
int do_mbgrdviz_saveroute(size_t instance, char *output_file_ptr);
int do_mbgrdviz_saveroutereversed(size_t instance, char *output_file_ptr);
int do_mbgrdviz_saverisiscriptheading(size_t instance, char *output_file_ptr);
int do_mbgrdviz_saverisiscriptnoheading(size_t instance, char *output_file_ptr);
int do_mbgrdviz_saverisi2scriptheading(size_t instance, char *output_file_ptr);
int do_mbgrdviz_saverisi2scriptnoheading(size_t instance, char *output_file_ptr);
int do_mbgrdviz_savedegdecmin(size_t instance, char *output_file_ptr);
int do_mbgrdviz_savelnw(size_t instance, char *output_file_ptr);
int do_mbgrdviz_savegreenseayml(size_t instance, char *output_file_ptr);
int do_mbgrdviz_savetecdislst(size_t instance, char *output_file_ptr);
int do_mbgrdviz_savekongsbergdp(size_t instance, char *output_file_ptr);
int do_mbgrdviz_savesisasciiplan1(size_t instance, char *output_file_ptr);
int do_mbgrdviz_savesisasciiplan2(size_t instance, char *output_file_ptr);
int do_mbgrdviz_openvector(size_t instance, char *input_file_ptr);
int do_mbgrdviz_saveprofile(size_t instance, char *output_file_ptr);
int do_mbgrdviz_opennav(size_t instance, bool swathbounds, char *input_file_ptr);
int do_mbgrdviz_readnav(size_t instance, char *swathfile, int pathstatus, char *pathraw, char *pathprocessed, int format,
                        int formatorg, double weight, int *error);
int do_mbgrdviz_opentest(size_t instance, double factor1, double factor2, double factor3, int *grid_projection_mode,
                         char *grid_projection_id, float *nodatavalue, int *nxy, int *n_columns, int *n_rows, double *min, double *max,
                         double *xmin, double *xmax, double *ymin, double *ymax, double *dx, double *dy, float **data);
int do_mbgrdviz_opengis(size_t instance, int type, char *input_file_ptr);
int do_mbgrdviz_importshapefile(size_t instance, char *input_file_ptr);
int do_mbgrdviz_savegis(size_t instance, int type, bool shapefile, char *output_file_ptr);
void do_mbgrdviz_open_region(Widget w, XtPointer client_data, XtPointer call_data);
void do_mbgrdviz_generate_survey(Widget w, XtPointer client_data, XtPointer call_data);
void do_mbgrdviz_arearoute_dismiss(Widget w, XtPointer client_data, XtPointer call_data);
void do_mbgrdviz_arearoute_info(size_t instance);

/* user interface functions that each program provides */
/* signal a refused or failed action */
void mbgrdviz_ui_bell(void);
/* the menus of the main window follow the open instances */
void do_mbgrdviz_sensitivity(void);
/* show the path of the realtime data source in the realtime setup */
void mbgrdviz_ui_realtime_path(char *path);
/* show the survey generation dialog's area and route information
   (a label string in Motif "BX" form) */
void mbgrdviz_ui_arearoute_info(char *info_text);
/* the actions that mbgrdviz adds to each instance (mbview_addaction) */
void do_mbgrdviz_fileSelectionBox(Widget w, XtPointer client_data, XtPointer call_data);
void do_mbgrdviz_fileSelectionBox_exportgispoint(Widget w, XtPointer client_data, XtPointer call_data);
void do_mbgrdviz_fileSelectionBox_exportpolygon(Widget w, XtPointer client_data, XtPointer call_data);
void do_mbgrdviz_fileSelectionBox_exportpolyline(Widget w, XtPointer client_data, XtPointer call_data);
void do_mbgrdviz_fileSelectionBox_importshapefile(Widget w, XtPointer client_data, XtPointer call_data);
void do_mbgrdviz_fileSelectionBox_opengispoint(Widget w, XtPointer client_data, XtPointer call_data);
void do_mbgrdviz_fileSelectionBox_openpolygon(Widget w, XtPointer client_data, XtPointer call_data);
void do_mbgrdviz_fileSelectionBox_openpolyline(Widget w, XtPointer client_data, XtPointer call_data);
void do_mbgrdviz_fileSelectionBox_savegispoint(Widget w, XtPointer client_data, XtPointer call_data);
void do_mbgrdviz_fileSelectionBox_savepolygon(Widget w, XtPointer client_data, XtPointer call_data);
void do_mbgrdviz_fileSelectionBox_savepolyline(Widget w, XtPointer client_data, XtPointer call_data);
void do_mbgrdviz_fileSelectionBox_opennav(Widget w, XtPointer client_data, XtPointer call_data);
void do_mbgrdviz_fileSelectionBox_openoverlay(Widget w, XtPointer client_data, XtPointer call_data);
void do_mbgrdviz_fileSelectionBox_openroute(Widget w, XtPointer client_data, XtPointer call_data);
void do_mbgrdviz_fileSelectionBox_opensite(Widget w, XtPointer client_data, XtPointer call_data);
void do_mbgrdviz_fileSelectionBox_openswath(Widget w, XtPointer client_data, XtPointer call_data);
void do_mbgrdviz_fileSelectionBox_openvector(Widget w, XtPointer client_data, XtPointer call_data);
void do_mbgrdviz_fileSelectionBox_savedegdecmin(Widget w, XtPointer client_data, XtPointer call_data);
void do_mbgrdviz_fileSelectionBox_savegreenseayml(Widget w, XtPointer client_data, XtPointer call_data);
void do_mbgrdviz_fileSelectionBox_savekongsbergdp(Widget w, XtPointer client_data, XtPointer call_data);
void do_mbgrdviz_fileSelectionBox_savelnw(Widget w, XtPointer client_data, XtPointer call_data);
void do_mbgrdviz_fileSelectionBox_saveprofile(Widget w, XtPointer client_data, XtPointer call_data);
void do_mbgrdviz_fileSelectionBox_saverisi2scriptheading(Widget w, XtPointer client_data, XtPointer call_data);
void do_mbgrdviz_fileSelectionBox_saverisi2scriptnoheading(Widget w, XtPointer client_data, XtPointer call_data);
void do_mbgrdviz_fileSelectionBox_saverisiscriptheading(Widget w, XtPointer client_data, XtPointer call_data);
void do_mbgrdviz_fileSelectionBox_saverisiscriptnoheading(Widget w, XtPointer client_data, XtPointer call_data);
void do_mbgrdviz_fileSelectionBox_saveroute(Widget w, XtPointer client_data, XtPointer call_data);
void do_mbgrdviz_fileSelectionBox_saveroutereversed(Widget w, XtPointer client_data, XtPointer call_data);
void do_mbgrdviz_fileSelectionBox_savesisasciiplan1(Widget w, XtPointer client_data, XtPointer call_data);
void do_mbgrdviz_fileSelectionBox_savesisasciiplan2(Widget w, XtPointer client_data, XtPointer call_data);
void do_mbgrdviz_fileSelectionBox_savesite(Widget w, XtPointer client_data, XtPointer call_data);
void do_mbgrdviz_fileSelectionBox_savetecdislst(Widget w, XtPointer client_data, XtPointer call_data);
void do_mbgrdviz_make_survey(Widget w, XtPointer client_data, XtPointer call_data);
void do_mbgrdviz_open_mbedit(Widget w, XtPointer client_data, XtPointer call_data);
void do_mbgrdviz_open_mbeditviz(Widget w, XtPointer client_data, XtPointer call_data);
void do_mbgrdviz_open_mbnavedit(Widget w, XtPointer client_data, XtPointer call_data);
void do_mbgrdviz_open_mbvelocitytool(Widget w, XtPointer client_data, XtPointer call_data);

#ifdef __cplusplus
}
#endif

#endif /* MBGRDVIZ_MBGRDVIZ_CORE_H */
