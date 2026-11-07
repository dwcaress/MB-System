/*--------------------------------------------------------------------
 *    The MB-system:  mbgrdviz_qt_ui.c  11/7/2026
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
 * The user interface functions of mbgrdviz_core.h for mbgrdviz-qt. The file
 * dialog callbacks use the titles, file patterns and modes of the Motif
 * callbacks in mbgrdviz_callbacks.c (generated from them); the Qt part
 * (mbgrdviz_qt_window.cc) shows the dialogs.
 */

#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>

#include "mb_define.h"
#include "mb_status.h"
#include "mbv_xtypes.h"
#include "mbview.h"
#include "mbgrdviz_core.h"
#include "mbgrdviz_state.h"
#include "mbgrdviz_qt_bridge.h"

/*------------------------------------------------------------------------------*/
void mbgrdviz_ui_bell(void) { mbgrdviz_qt_bell(); }

void mbgrdviz_ui_realtime_path(char *path) { mbgrdviz_qt_realtime_path(path); }

/*------------------------------------------------------------------------------*/
/* the menu items of the main window, as the Motif do_mbgrdviz_sensitivity() */
void do_mbgrdviz_sensitivity(void) {
  bool mbview_active = false;
  bool mbview_allactive = true;
  size_t instance = MBV_NO_WINDOW;
  for (int i = 0; i < MBV_MAX_WINDOWS; i++) {
    if (mbview_id[i]) {
      mbview_active = true;
      if (instance == MBV_NO_WINDOW)
        instance = i;
    }
    else
      mbview_allactive = false;
  }
  int nsite = 0;
  int nroute = 0;
  if (mbview_active) {
    mbview_getsitecount(verbose, instance, &nsite, &error);
    mbview_getroutecount(verbose, instance, &nroute, &error);
  }
  mbgrdviz_qt_sensitivity(!mbview_allactive, mbview_active, mbview_active && nsite > 0, mbview_active && nroute > 0);
}

/*------------------------------------------------------------------------------*/
/* file dialogs: client_data is the instance (0 from the main window menu) */
void do_mbgrdviz_fileSelectionBox(Widget w, XtPointer client_data, XtPointer call_data) {
  (void)w;
  (void)call_data;
  mbgrdviz_qt_file_dialog(MBGRDVIZ_OPENGRID, (size_t)client_data, "Open GMT Grid File", "*.grd");
}
void do_mbgrdviz_fileSelectionBox_exportgispoint(Widget w, XtPointer client_data, XtPointer call_data) {
  (void)w;
  (void)call_data;
  mbgrdviz_qt_file_dialog(MBGRDVIZ_EXPORTGISPOINT, (size_t)client_data, "Export Points as Shapefile", "*.shp");
}
void do_mbgrdviz_fileSelectionBox_exportpolygon(Widget w, XtPointer client_data, XtPointer call_data) {
  (void)w;
  (void)call_data;
  mbgrdviz_qt_file_dialog(MBGRDVIZ_EXPORTPOLYGON, (size_t)client_data, "Export Polygons as Shapefile", "*.shp");
}
void do_mbgrdviz_fileSelectionBox_exportpolyline(Widget w, XtPointer client_data, XtPointer call_data) {
  (void)w;
  (void)call_data;
  mbgrdviz_qt_file_dialog(MBGRDVIZ_EXPORTPOLYLINE, (size_t)client_data, "Export Polylines as Shapefile", "*.shp");
}
void do_mbgrdviz_fileSelectionBox_importshapefile(Widget w, XtPointer client_data, XtPointer call_data) {
  (void)w;
  (void)call_data;
  mbgrdviz_qt_file_dialog(MBGRDVIZ_IMPORTSHAPEFILE, (size_t)client_data, "Import Shapefile", "*.shp");
}
void do_mbgrdviz_fileSelectionBox_opengispoint(Widget w, XtPointer client_data, XtPointer call_data) {
  (void)w;
  (void)call_data;
  mbgrdviz_qt_file_dialog(MBGRDVIZ_OPENGISPOINT, (size_t)client_data, "Open Point File", "*.pts");
}
void do_mbgrdviz_fileSelectionBox_openpolygon(Widget w, XtPointer client_data, XtPointer call_data) {
  (void)w;
  (void)call_data;
  mbgrdviz_qt_file_dialog(MBGRDVIZ_OPENPOLYGON, (size_t)client_data, "Open Polygon File", "*.plg");
}
void do_mbgrdviz_fileSelectionBox_openpolyline(Widget w, XtPointer client_data, XtPointer call_data) {
  (void)w;
  (void)call_data;
  mbgrdviz_qt_file_dialog(MBGRDVIZ_OPENPOLYLINE, (size_t)client_data, "Open Polyline File", "*.pln");
}
void do_mbgrdviz_fileSelectionBox_savegispoint(Widget w, XtPointer client_data, XtPointer call_data) {
  (void)w;
  (void)call_data;
  mbgrdviz_qt_file_dialog(MBGRDVIZ_SAVEGISPOINT, (size_t)client_data, "Save Point File", "*.pts");
}
void do_mbgrdviz_fileSelectionBox_savepolygon(Widget w, XtPointer client_data, XtPointer call_data) {
  (void)w;
  (void)call_data;
  mbgrdviz_qt_file_dialog(MBGRDVIZ_SAVEPOLYGON, (size_t)client_data, "Save Polygon File", "*.plg");
}
void do_mbgrdviz_fileSelectionBox_savepolyline(Widget w, XtPointer client_data, XtPointer call_data) {
  (void)w;
  (void)call_data;
  mbgrdviz_qt_file_dialog(MBGRDVIZ_SAVEPOLYLINE, (size_t)client_data, "Save Polyline File", "*.pln");
}
void do_mbgrdviz_fileSelectionBox_opennav(Widget w, XtPointer client_data, XtPointer call_data) {
  (void)w;
  (void)call_data;
  mbgrdviz_qt_file_dialog(MBGRDVIZ_OPENNAV, (size_t)client_data, "Open Navigation Datalist File", "*.mb-1");
}
void do_mbgrdviz_fileSelectionBox_openoverlay(Widget w, XtPointer client_data, XtPointer call_data) {
  (void)w;
  (void)call_data;
  mbgrdviz_qt_file_dialog(MBGRDVIZ_OPENOVERLAY, (size_t)client_data, "Open Overlay GMT Grid File", "*.grd");
}
void do_mbgrdviz_fileSelectionBox_openroute(Widget w, XtPointer client_data, XtPointer call_data) {
  (void)w;
  (void)call_data;
  mbgrdviz_qt_file_dialog(MBGRDVIZ_OPENROUTE, (size_t)client_data, "Open Route File", "*.rte");
}
void do_mbgrdviz_fileSelectionBox_opensite(Widget w, XtPointer client_data, XtPointer call_data) {
  (void)w;
  (void)call_data;
  mbgrdviz_qt_file_dialog(MBGRDVIZ_OPENSITE, (size_t)client_data, "Open Site File", "*.ste");
}
void do_mbgrdviz_fileSelectionBox_openswath(Widget w, XtPointer client_data, XtPointer call_data) {
  (void)w;
  (void)call_data;
  mbgrdviz_qt_file_dialog(MBGRDVIZ_OPENSWATH, (size_t)client_data, "Open Swath Datalist File", "*.mb-1");
}
void do_mbgrdviz_fileSelectionBox_openvector(Widget w, XtPointer client_data, XtPointer call_data) {
  (void)w;
  (void)call_data;
  mbgrdviz_qt_file_dialog(MBGRDVIZ_OPENVECTOR, (size_t)client_data, "Open Vector File", "*");
}
void do_mbgrdviz_fileSelectionBox_savedegdecmin(Widget w, XtPointer client_data, XtPointer call_data) {
  (void)w;
  (void)call_data;
  mbgrdviz_qt_file_dialog(MBGRDVIZ_SAVEDEGDECMIN, (size_t)client_data, "Save Route as Degrees + Decimal Minutes File", "*");
}
void do_mbgrdviz_fileSelectionBox_savegreenseayml(Widget w, XtPointer client_data, XtPointer call_data) {
  (void)w;
  (void)call_data;
  mbgrdviz_qt_file_dialog(MBGRDVIZ_SAVEGREENSEAYML, (size_t)client_data, "Save Route as Greensea YML File", "*");
}
void do_mbgrdviz_fileSelectionBox_savekongsbergdp(Widget w, XtPointer client_data, XtPointer call_data) {
  (void)w;
  (void)call_data;
  mbgrdviz_qt_file_dialog(MBGRDVIZ_SAVEKONGSBERGDP, (size_t)client_data, "Save Route as Kongsberg DP Waypoint File", "*");
}
void do_mbgrdviz_fileSelectionBox_savelnw(Widget w, XtPointer client_data, XtPointer call_data) {
  (void)w;
  (void)call_data;
  mbgrdviz_qt_file_dialog(MBGRDVIZ_SAVELNW, (size_t)client_data, "Save Route as Hypack LNW File", "*");
}
void do_mbgrdviz_fileSelectionBox_saveprofile(Widget w, XtPointer client_data, XtPointer call_data) {
  (void)w;
  (void)call_data;
  mbgrdviz_qt_file_dialog(MBGRDVIZ_SAVEPROFILE, (size_t)client_data, "Save Profile File", "*");
}
void do_mbgrdviz_fileSelectionBox_saverisi2scriptheading(Widget w, XtPointer client_data, XtPointer call_data) {
  (void)w;
  (void)call_data;
  mbgrdviz_qt_file_dialog(MBGRDVIZ_SAVERISI2SCRIPTHEADING, (size_t)client_data, "Save Risi Script File (heading varies)", "*");
}
void do_mbgrdviz_fileSelectionBox_saverisi2scriptnoheading(Widget w, XtPointer client_data, XtPointer call_data) {
  (void)w;
  (void)call_data;
  mbgrdviz_qt_file_dialog(MBGRDVIZ_SAVERISI2SCRIPTNOHEADING, (size_t)client_data, "Save Risi Script File (heading static)", "*");
}
void do_mbgrdviz_fileSelectionBox_saverisiscriptheading(Widget w, XtPointer client_data, XtPointer call_data) {
  (void)w;
  (void)call_data;
  mbgrdviz_qt_file_dialog(MBGRDVIZ_SAVERISISCRIPTHEADING, (size_t)client_data, "Save Risi Script File (heading varies)", "*");
}
void do_mbgrdviz_fileSelectionBox_saverisiscriptnoheading(Widget w, XtPointer client_data, XtPointer call_data) {
  (void)w;
  (void)call_data;
  mbgrdviz_qt_file_dialog(MBGRDVIZ_SAVERISISCRIPTNOHEADING, (size_t)client_data, "Save Risi Script File (heading static)", "*");
}
void do_mbgrdviz_fileSelectionBox_saveroute(Widget w, XtPointer client_data, XtPointer call_data) {
  (void)w;
  (void)call_data;
  mbgrdviz_qt_file_dialog(MBGRDVIZ_SAVEROUTE, (size_t)client_data, "Save Route File", "*");
}
void do_mbgrdviz_fileSelectionBox_saveroutereversed(Widget w, XtPointer client_data, XtPointer call_data) {
  (void)w;
  (void)call_data;
  mbgrdviz_qt_file_dialog(MBGRDVIZ_SAVEROUTEREVERSED, (size_t)client_data, "Save Route File", "*");
}
void do_mbgrdviz_fileSelectionBox_savesisasciiplan1(Widget w, XtPointer client_data, XtPointer call_data) {
  (void)w;
  (void)call_data;
  mbgrdviz_qt_file_dialog(MBGRDVIZ_SAVESISASCIIPLAN1, (size_t)client_data, "Save Route as SIS ASCIIPlan Waypoint File", "*");
}
void do_mbgrdviz_fileSelectionBox_savesisasciiplan2(Widget w, XtPointer client_data, XtPointer call_data) {
  (void)w;
  (void)call_data;
  mbgrdviz_qt_file_dialog(MBGRDVIZ_SAVESISASCIIPLAN2, (size_t)client_data, "Save Route as SIS ASCIIPlan Waypoint File", "*");
}
void do_mbgrdviz_fileSelectionBox_savesite(Widget w, XtPointer client_data, XtPointer call_data) {
  (void)w;
  (void)call_data;
  mbgrdviz_qt_file_dialog(MBGRDVIZ_SAVESITE, (size_t)client_data, "Save Site File", "*");
}
void do_mbgrdviz_fileSelectionBox_savetecdislst(Widget w, XtPointer client_data, XtPointer call_data) {
  (void)w;
  (void)call_data;
  mbgrdviz_qt_file_dialog(MBGRDVIZ_SAVETECDISLST, (size_t)client_data, "Save Route as TECDIS LST File", "*");
}

/*------------------------------------------------------------------------------*/
/* actions of later steps (survey generation, region, starting other programs) */
void do_mbgrdviz_make_survey(Widget w, XtPointer client_data, XtPointer call_data) {
  (void)w;
  (void)call_data;
  mbgrdviz_qt_not_yet((size_t)client_data, "make_survey");
}
void do_mbgrdviz_open_mbedit(Widget w, XtPointer client_data, XtPointer call_data) {
  (void)w;
  (void)call_data;
  mbgrdviz_qt_not_yet((size_t)client_data, "open_mbedit");
}
void do_mbgrdviz_open_mbeditviz(Widget w, XtPointer client_data, XtPointer call_data) {
  (void)w;
  (void)call_data;
  mbgrdviz_qt_not_yet((size_t)client_data, "open_mbeditviz");
}
void do_mbgrdviz_open_mbnavedit(Widget w, XtPointer client_data, XtPointer call_data) {
  (void)w;
  (void)call_data;
  mbgrdviz_qt_not_yet((size_t)client_data, "open_mbnavedit");
}
void do_mbgrdviz_open_mbvelocitytool(Widget w, XtPointer client_data, XtPointer call_data) {
  (void)w;
  (void)call_data;
  mbgrdviz_qt_not_yet((size_t)client_data, "open_mbvelocitytool");
}
void do_mbgrdviz_open_region(Widget w, XtPointer client_data, XtPointer call_data) {
  (void)w;
  (void)call_data;
  mbgrdviz_qt_not_yet((size_t)client_data, "open_region");
}
