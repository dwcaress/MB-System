/*------------------------------------------------------------------------------
 *    The MB-system:	MBpedit.h	10/28/2003
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

#ifndef MBVIEW_MBpedit_H_
#define MBVIEW_MBpedit_H_

typedef struct _MBpeditData {
	Widget MBpedit;
	Widget mbpingedit_controls;
	Widget mbpingedit_pushButton_flag_view;
	Widget mbpingedit_menuBar_view;
	Widget mbpingedit_cascadeButton_view;
	Widget mbpingedit_pulldownMenu_view;
	Widget mbpingedit_toggleButton_view_waterfall;
	Widget mbpingedit_toggleButton_view_alongtrack;
	Widget mbpingedit_toggleButton_view_acrosstrack;
	Widget mbpingedit_separator2;
	Widget mbpingedit_toggleButton_show_flagged_on;
	Widget mbpingedit_toggleButton_show_detects;
	Widget mbpingedit_separator9;
	Widget mbpingedit_toggleButton_show_wideplot;
	Widget mbpingedit_toggleButton_show_time;
	Widget mbpingedit_toggleButton_show_interval;
	Widget mbpingedit_toggleButton_show_lon;
	Widget mbpingedit_toggleButton_show_latitude;
	Widget mbpingedit_toggleButton_show_heading;
	Widget mbpingedit_toggleButton_show_speed;
	Widget mbpingedit_toggleButton_show_depth;
	Widget mbpingedit_toggleButton_show_altitude;
	Widget mbpingedit_toggleButton_show_sensordepth;
	Widget mbpingedit_toggleButton_show_roll;
	Widget mbpingedit_toggleButton_show_pitch;
	Widget mbpingedit_toggleButton_show_heave;
	Widget mbpingedit_pushButton_unflag_all;
	Widget mbpingedit_pushButton_unflag_view;
	Widget mbpingedit_menuBar_controls;
	Widget mbpingedit_cascadeButton_controls;
	Widget mbpingedit_pulldownMenu_controls;
	Widget mbpingedit_pushButton_goto;
	Widget mbpingedit_pushButton_buffer;
	Widget mbpingedit_pushButton_annotation;
	Widget mbpingedit_pushButton_filters;
	Widget mbpingedit_separator7;
	Widget mbpingedit_toggleButton_reverse_keys;
	Widget mbpingedit_toggleButton_reverse_mouse;
	Widget mbpingedit_pushButton_next;
	Widget mbpingedit_pushButton_dismiss;
	Widget mbpingedit_pushButton_forward;
	Widget mbpingedit_pushButton_reverse;
	Widget mbpingedit_slider_mbpingedit_scale_x_label;
	Widget mbpingedit_slider_mbpingedit_scale_x;
	Widget mbpingedit_slider_mbpingedit_scale_x_max_label;
	Widget mbpingedit_slider_mbpingedit_scale_y_label;
	Widget mbpingedit_slider_mbpingedit_scale_y;
	Widget mbpingedit_slider_mbpingedit_scale_y_max_label;
	Widget mbpingedit_slider_number_pings_label;
	Widget mbpingedit_slider_number_pings;
	Widget mbpingedit_slider_num_pings_max_label;
	Widget mbpingedit_slider_number_step_label;
	Widget mbpingedit_slider_number_step;
	Widget mbpingedit_slider_number_max_step_label;
	Widget mbpingedit_setting_mode_label;
	Widget mbpingedit_setting_mode;
	Widget mbpingedit_togglebutton_toggle;
	Widget mbpingedit_togglebutton_pick;
	Widget mbpingedit_togglebutton_erase;
	Widget mbpingedit_togglebutton_restore;
	Widget mbpingedit_togglebutton_grab;
	Widget mbpingedit_togglebutton_info;
	Widget mbpingedit_canvas;
	Widget mbpingedit_dialogShell_filters;
	Widget mbpingedit_form_filters;
	Widget scrolledWindow_filters;
	Widget mbpingedit_bulletinBoard_scrollfilters;
	Widget mbpingedit_radioBox_mediancalc;
	Widget mbpingedit_scale_median_local_ltrack;
	Widget mbpingedit_scale_median_local_xtrack;
	Widget mbpingedit_separator6;
	Widget mbpingedit_scale_filters_cutangleend;
	Widget mbpingedit_scale_filters_cutanglestart;
	Widget mbpingedit_toggleButton_filters_cutangle;
	Widget mbpingedit_separator5;
	Widget mbpingedit_scale_filters_cutdistanceend;
	Widget mbpingedit_scale_filters_cutdistancestart;
	Widget mbpingedit_toggleButton_filters_cutdistance;
	Widget mbpingedit_separator4;
	Widget mbpingedit_scale_filters_cutbeamend;
	Widget mbpingedit_scale_filters_cutbeamstart;
	Widget mbpingedit_toggleButton_filters_cutbeam;
	Widget mbpingedit_separator3;
	Widget mbpingedit_scale_filters_wrongside;
	Widget mbpingedit_toggleButton_filters_wrongside;
	Widget mbpingedit_scale_filters_medianspike;
	Widget mbpingedit_toggleButton_filters_medianspike;
	Widget mbpingedit_pushButton_filters_reset;
	Widget mbpingedit_pushButton_filters_apply;
	Widget mbpingedit_pushButton_filters_dismiss;
	Widget mbpingedit_dialogShell_annotation;
	Widget mbpingedit_form_annotation;
	Widget mbpingedit_pushButton_annotation_dismiss;
	Widget mbpingedit_slider_y_max_interval_label;
	Widget mbpingedit_slider_y_interval;
	Widget mbpingedit_slider_y_interval_label;
	Widget mbpingedit_slider_x_max_interval_label;
	Widget mbpingedit_slider_x_interval;
	Widget mbpingedit_slider_x_interval_label;
} MBpeditData;

typedef struct _MBpeditData *MBpeditDataPtr;

MBpeditDataPtr MBpeditCreate(MBpeditDataPtr, Widget, String, ArgList, Cardinal);

#endif  // MBVIEW_MBpedit_H_
