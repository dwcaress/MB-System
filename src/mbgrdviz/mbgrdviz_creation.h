/*--------------------------------------------------------------------
 *    The MB-system:	mbgrdviz_creation.h	          10/9/2002
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

#ifndef MBGRDVIZ_MBGRDVIZ_CREATION_H_
#define MBGRDVIZ_MBGRDVIZ_CREATION_H_

/*
 * Global widget declarations.
 *        - EXTERNAL is set to extern if the
 *          defs file is not included from the
 *          main file.
 */
#ifdef DECLARE_BX_GLOBALS
#define EXTERNAL
#else
#define EXTERNAL extern
#endif

EXTERNAL Widget label_realtimesetup_teststatus;
EXTERNAL Widget radioBox_realtimesetup_pathmode;
EXTERNAL Widget toggleButton_realtimesetup_recent;
EXTERNAL Widget toggleButton_realtimesetup_pointer;
EXTERNAL Widget spinText_realtimesetup_icon;
EXTERNAL Widget pushButton_realtimesetup_pathbrowse;
EXTERNAL Widget pushButton_realtimesetup_pathtest;
EXTERNAL Widget scale_realtimesetup_update;
EXTERNAL Widget textField_realtimesetup_path;
EXTERNAL Widget bulletinBoard_arearoute;
EXTERNAL Widget spinBox_arearoute_interleaving;
EXTERNAL Widget spinText_arearoute_interleaving;
EXTERNAL Widget textField_arearoute_name;
EXTERNAL Widget spinBox_arearoute_color;
EXTERNAL Widget spinText_arearoute_color;
EXTERNAL Widget spinBox_arearoute_crosslines;
EXTERNAL Widget spinText_arearoute_crosslines;
EXTERNAL Widget label_arearoute_depth;
EXTERNAL Widget label_arearoute_altitude;
EXTERNAL Widget spinBox_arearoute_altitude;
EXTERNAL Widget spinText_arearoute_altitude;
EXTERNAL Widget spinBox_arearoute_depth;
EXTERNAL Widget spinText_arearoute_depth;
EXTERNAL Widget spinBox_arearoute_direction;
EXTERNAL Widget spinText_arearoute_direction;
EXTERNAL Widget spinBox_arearoute_swathwidth;
EXTERNAL Widget spinText_arearoute_swathwidth;
EXTERNAL Widget label_arearoute_swathwidth;
EXTERNAL Widget label_arearoute_platform;
EXTERNAL Widget spinBox_arearoute_platform;
EXTERNAL Widget spinText_arearoute_platform;
EXTERNAL Widget spinBox_arearoute_linespacing;
EXTERNAL Widget spinText_arearoute_linespacing;
EXTERNAL Widget spinBox_arearoute_crosslinesfirstlast;
EXTERNAL Widget spinText_arearoute_crosslinesfirstlast;
EXTERNAL Widget label_arearoute_info;
EXTERNAL Widget spinBox_arearoute_linecontrol;
EXTERNAL Widget spinText_arearoute_linecontrol;
EXTERNAL Widget label_arearoute_linespacing;
EXTERNAL Widget pushButton_arearoute_ok;
EXTERNAL Widget dialogShell_open;
EXTERNAL Widget fileSelectionBox;
EXTERNAL Widget label_about_version;
EXTERNAL Widget pushButton_file_openprimary;
EXTERNAL Widget pushButton_opensite;
EXTERNAL Widget pushButton_openroute;
EXTERNAL Widget pushButton_opennav;
EXTERNAL Widget pushButton_openswath;
EXTERNAL Widget pushButton_openvector;
EXTERNAL Widget pushButton_savesite;
EXTERNAL Widget pushButton_saveroute;
EXTERNAL Widget pushButton_realtime_setup;
EXTERNAL Widget pushButton_realtime_start;
EXTERNAL Widget pushButton_realtime_stop;
EXTERNAL Widget pushButton_realtime_pause;
EXTERNAL Widget pushButton_realtime_resume;

#endif  // MBGRDVIZ_MBGRDVIZ_CREATION_H_
