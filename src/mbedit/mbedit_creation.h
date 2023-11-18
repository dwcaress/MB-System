/*--------------------------------------------------------------------
 *    The MB-system:	mbedit_creation.h	1993
 *
 *    Copyright (c) 1993-2023 by
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

#ifndef MBEDIT_mbedit_creation_H_
#define MBEDIT_mbedit_creation_H_

#ifdef DECLARE_BX_GLOBALS
#define EXTERNAL
#else
#define EXTERNAL extern
#endif

EXTERNAL Widget window_mbedit;
EXTERNAL Widget setting_output_filelist;
EXTERNAL Widget setting_output_toggle_edit_filelist;
EXTERNAL Widget setting_output_toggle_browse_filelist;
EXTERNAL Widget list_filelist;
EXTERNAL Widget radioBox_mediancalc;
EXTERNAL Widget scale_median_local_ltrack;
EXTERNAL Widget scale_median_local_xtrack;
EXTERNAL Widget scale_filters_cutangleend;
EXTERNAL Widget scale_filters_cutanglestart;
EXTERNAL Widget toggleButton_filters_cutangle;
EXTERNAL Widget scale_filters_cutdistanceend;
EXTERNAL Widget scale_filters_cutdistancestart;
EXTERNAL Widget toggleButton_filters_cutdistance;
EXTERNAL Widget scale_filters_cutbeamend;
EXTERNAL Widget scale_filters_cutbeamstart;
EXTERNAL Widget toggleButton_filters_cutbeam;
EXTERNAL Widget scale_filters_wrongside;
EXTERNAL Widget toggleButton_filters_wrongside;
EXTERNAL Widget scale_filters_medianspike;
EXTERNAL Widget toggleButton_filters_medianspike;
EXTERNAL Widget slider_y_max_interval_label;
EXTERNAL Widget slider_y_interval;
EXTERNAL Widget slider_y_interval_label;
EXTERNAL Widget slider_x_max_interval_label;
EXTERNAL Widget slider_x_interval;
EXTERNAL Widget slider_x_interval_label;
EXTERNAL Widget slider_buffer_hold_max_label;
EXTERNAL Widget slider_buffer_hold;
EXTERNAL Widget slider_buffer_hold_label;
EXTERNAL Widget slider_buffer_size_max_label;
EXTERNAL Widget slider_buffer_size;
EXTERNAL Widget slider_buffer_size_label;
EXTERNAL Widget bulletinBoard_error;
EXTERNAL Widget label_error_two;
EXTERNAL Widget label_error_one;
EXTERNAL Widget label_error_three;
EXTERNAL Widget bulletinBoard_editsave;
EXTERNAL Widget bulletinBoard_message;
EXTERNAL Widget label_message;
EXTERNAL Widget label_about_version;
EXTERNAL Widget textfield_day;
EXTERNAL Widget textfield_second;
EXTERNAL Widget textfield_minute;
EXTERNAL Widget textfield_hour;
EXTERNAL Widget textfield_month;
EXTERNAL Widget textfield_year;
EXTERNAL Widget fileSelectionBox;
EXTERNAL Widget textfield_format_label;
EXTERNAL Widget textfield_format;
EXTERNAL Widget setting_output_label;
EXTERNAL Widget setting_output;
EXTERNAL Widget setting_output_toggle_edit;
EXTERNAL Widget setting_output_toggle_browse;
EXTERNAL Widget pushButton_file;
EXTERNAL Widget pushButton_end;
EXTERNAL Widget pushButton_start;
EXTERNAL Widget toggleButton_view_waterfall;
EXTERNAL Widget toggleButton_view_alongtrack;
EXTERNAL Widget toggleButton_view_acrosstrack;
EXTERNAL Widget toggleButton_show_flaggedsoundings_on;
EXTERNAL Widget toggleButton_show_flaggedprofiles_on;
EXTERNAL Widget toggleButton_show_flags;
EXTERNAL Widget toggleButton_show_detects;
EXTERNAL Widget toggleButton_show_pulsetypes;
EXTERNAL Widget toggleButton_show_wideplot;
EXTERNAL Widget toggleButton_show_time;
EXTERNAL Widget toggleButton_show_interval;
EXTERNAL Widget toggleButton_show_lon;
EXTERNAL Widget toggleButton_show_latitude;
EXTERNAL Widget toggleButton_show_heading;
EXTERNAL Widget toggleButton_show_speed;
EXTERNAL Widget toggleButton_show_depth;
EXTERNAL Widget toggleButton_show_altitude;
EXTERNAL Widget toggleButton_show_sensordepth;
EXTERNAL Widget toggleButton_show_roll;
EXTERNAL Widget toggleButton_show_pitch;
EXTERNAL Widget toggleButton_show_heave;
EXTERNAL Widget toggleButton_reverse_keys;
EXTERNAL Widget toggleButton_reverse_mouse;
EXTERNAL Widget pushButton_next;
EXTERNAL Widget pushButton_done;
EXTERNAL Widget pushButton_forward;
EXTERNAL Widget pushButton_reverse;
EXTERNAL Widget slider_scale_x_label;
EXTERNAL Widget slider_scale_x;
EXTERNAL Widget slider_scale_x_max_label;
EXTERNAL Widget slider_scale_y_label;
EXTERNAL Widget slider_scale_y;
EXTERNAL Widget slider_scale_y_max_label;
EXTERNAL Widget slider_number_pings_label;
EXTERNAL Widget slider_number_pings;
EXTERNAL Widget slider_num_pings_max_label;
EXTERNAL Widget slider_number_step_label;
EXTERNAL Widget slider_number_step;
EXTERNAL Widget slider_number_max_step_label;
EXTERNAL Widget setting_mode_toggle_toggle;
EXTERNAL Widget setting_mode_toggle_pick;
EXTERNAL Widget setting_mode_toggle_erase;
EXTERNAL Widget setting_mode_toggle_restore;
EXTERNAL Widget setting_mode_toggle_grab;
EXTERNAL Widget setting_mode_toggle_info;
EXTERNAL Widget canvas_mbedit;

#endif  // MBEDIT_mbedit_creation_H_
