/*------------------------------------------------------------------------------
 *    The MB-system:	MB3DView.h	10/28/2003
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

#ifndef MBVIEW_MB3DVIEW_H_
#define MBVIEW_MB3DVIEW_H_

typedef struct _MB3DViewData {
	Widget MB3DView;
	Widget mbview_pushButton_clearpicks;
	Widget mbview_pushButton_reset;
	Widget mbview_radioBox_mouse;
	Widget mbview_toggleButton_mode_rmove;
	Widget mbview_toggleButton_mode_rrotate;
	Widget mbview_toggleButton_mode_rviewpoint;
	Widget mbview_toggleButton_mode_rshade;
	Widget mbview_toggleButton_mode_rarea;
	Widget mbview_toggleButton_mode_rsite;
	Widget mbview_toggleButton_mode_rroute;
	Widget mbview_toggleButton_mode_rnav;
	Widget mbview_toggleButton_mode_rnavfile;
	Widget mbview_label_status;
	Widget mbview_pushButton_fullrez;
	Widget mbview_label_pickinfo;
	Widget mbview_menuBar_mbview;
	Widget mbview_cascadeButton_view;
	Widget mbview_pulldownMenu_view;
	Widget mbview_toggleButton_display_2D;
	Widget mbview_toggleButton_display_3D;
	Widget mbview_separator10;
	Widget mbview_toggleButton_data_primary;
	Widget mbview_toggleButton_data_primaryslope;
	Widget mbview_toggleButton_data_secondary;
	Widget mbview_separator;
	Widget mbview_toggleButton_histogram;
	Widget mbview_separator21;
	Widget mbview_toggleButton_overlay_none;
	Widget mbview_toggleButton_overlay_illumination;
	Widget mbview_toggleButton_overlay_slope;
	Widget mbview_toggleButton_overlay_secondary;
	Widget mbview_separator1;
	Widget mbview_toggleButton_overlay_contour;
	Widget mbview_toggleButton_site;
	Widget mbview_toggleButton_route;
	Widget mbview_toggleButton_nav;
	Widget mbview_toggleButton_navswathbounds;
	Widget mbview_toggleButton_navdrape;
	Widget mbview_toggleButton_vector;
	Widget mbview_separator8;
	Widget mbview_toggleButton_colortable_haxby;
	Widget mbview_toggleButton_colortable_bright;
	Widget mbview_toggleButton_colortable_muted;
	Widget mbview_toggleButton_colortable_gray;
	Widget mbview_toggleButton_colortable_flat;
	Widget mbview_toggleButton_colortable_sealevel1;
	Widget mbview_toggleButton_colortable_sealevel2;
	Widget separator1;
	Widget mbview_toggleButton_profile;
	Widget mbview_cascadeButton_controls;
	Widget mbview_pulldownMenu_controls;
	Widget mbview_pushButton_colorbounds;
	Widget mbview_pushButton_2dview;
	Widget mbview_pushButton_3dview;
	Widget mbview_pushButton_shadeparms;
	Widget mbview_pushButton_resolution;
	Widget mbview_pushButton_projections;
	Widget mbview_pushButton_sitelist;
	Widget mbview_pushButton_routelist;
	Widget mbview_pushButton_navlist;
	Widget mbview_cascadeButton_mouse;
	Widget mbview_pulldownMenu_mouse;
	Widget mbview_toggleButton_mode_move;
	Widget mbview_toggleButton_mode_rotate;
	Widget mbview_toggleButton_mode_viewpoint;
	Widget mbview_toggleButton_mode_shade;
	Widget mbview_toggleButton_mode_area;
	Widget mbview_toggleButton_mode_site;
	Widget mbview_toggleButton_mode_route;
	Widget mbview_toggleButton_mode_nav;
	Widget mbview_toggleButton_mode_navfile;
	Widget mbview_cascadeButton_action;
	Widget mbview_pulldownMenu_action;
	Widget mbview_pushButton_help_about;
	Widget mbview_cascadeButton_dismiss;
	Widget mbview_pulldownMenu_dismiss;
	Widget mbview_pushButton_dismiss;
	Widget mbview_label_mouse;
	Widget mbview_drawingArea_mbview;
	Widget mbview_dialogShell_colorbounds;
	Widget mbview_bulletinBoard_colorbounds;
	Widget mbview_separator5;
	Widget mbview_radioBox_overlaymode;
	Widget mbview_toggleButton_overlay_ctoh;
	Widget mbview_toggleButton_overlay_htoc;
	Widget mbview_textField_overlaymax;
	Widget mbview_label_overlaymax;
	Widget mbview_textField_overlaymin;
	Widget mbview_label_overlaymin;
	Widget mbview_label_overlaybounds;
	Widget mbview_separator3;
	Widget mbview_radioBox_slopemode;
	Widget mbview_toggleButton_slope_ctoh;
	Widget mbview_toggleButton_slope_htoc;
	Widget mbview_textField_slopemax;
	Widget mbview_label_slopemax;
	Widget mbview_textField_slopemin;
	Widget mbview_label_slopemin;
	Widget mbview_label_slopebounds;
	Widget mbview_radioBox_colormode;
	Widget mbview_toggleButton_data_ctoh;
	Widget mbview_toggleButton_data_htoc;
	Widget mbview_textField_datamax;
	Widget mbview_textField_datamin;
	Widget mbview_label_colormax;
	Widget mbview_label_colormin;
	Widget mbview_label_colorbounds;
	Widget mbview_separator2;
	Widget mbview_pushButton_colorbounds_apply;
	Widget mbview_label_contour;
	Widget mbview_textField_contours;
	Widget mbview_pushButton_colorbounds_dismiss;
	Widget mbview_dialogShell_resolution;
	Widget mbview_bulletinBoard_resolution;
	Widget mbview_scale_navmediumresolution;
	Widget mbview_scale_navlowresolution;
	Widget separator;
	Widget mbview_label_navrenderdecimation;
	Widget mbview_label_gridrenderres;
	Widget mbview_scale_mediumresolution;
	Widget mbview_scale_lowresolution;
	Widget mbview_pushButton_resolution_dismiss;
	Widget mbview_dialogShell_message;
	Widget mbview_bulletinBoard_message;
	Widget mbview_label_message;
	Widget mbview_label_thanks;
	Widget mbview_dialogShell_about;
	Widget mbview_bulletinBoard_about;
	Widget mbview_label_about_version;
	Widget mbview_label_about_authors;
	Widget mbview_label_about_MBARI;
	Widget mbview_label_about_LDEO;
	Widget mbview_separator6;
	Widget mbview_label_about_mbsystem;
	Widget mbview_separator7;
	Widget mbview_label_about_title;
	Widget mbview_pushButton_about_dismiss;
	Widget mbview_dialogShell_shadeparms;
	Widget mbview_bulletinBoard_shadeparms;
	Widget mbview_separator13;
	Widget mbview_textField_overlay_center;
	Widget mbview_label_overlay_center;
	Widget mbview_label_overlayshade;
	Widget mbview_radioBox_overlay_shade;
	Widget mbview_toggleButton_overlay_shade_ctoh;
	Widget mbview_toggleButton_overlay_shade_htoc;
	Widget mbview_textField_overlay_amp;
	Widget mbview_label_overlay_amp;
	Widget mbview_separator15;
	Widget mbview_textField_slope_amp;
	Widget mbview_label_slope_amp;
	Widget mbview_label_slopeshade;
	Widget mbview_textField_illum_azi;
	Widget mbview_textField_illum_amp;
	Widget mbview_label_illum_azi;
	Widget mbview_label_illum_amp;
	Widget mbview_label_illumination;
	Widget mbview_separator16;
	Widget mbview_pushButton_shadeparms_apply;
	Widget mbview_label_illum_elev;
	Widget mbview_textField_illum_elev;
	Widget mbview_pushButton_shadeparms_dismiss2;
	Widget mbview_dialogShell_3dparms;
	Widget mbview_bulletinBoard_3dparms;
	Widget mbview_textField_model_3dzoom;
	Widget mbview_label_model_3dzoom;
	Widget mbview_separator11;
	Widget mbview_textField_view_3dzoom;
	Widget mbview_label_view_3dzoom;
	Widget mbview_textField_view_3doffsety;
	Widget mbview_label_view_3doffsety;
	Widget mbview_separator20;
	Widget mbview_textField_view_3doffsetx;
	Widget mbview_label_view_3doffsetx;
	Widget mbview_label_view_offset;
	Widget mbview_textField_view_elevation;
	Widget mbview_label_view_elevation;
	Widget mbview_separator4;
	Widget mbview_textField_view_azimuth;
	Widget mbview_label_view_azimuth;
	Widget mbview_label_view;
	Widget mbview_textField_model_elevation;
	Widget mbview_textField_model_azimuth;
	Widget mbview_label_model_elevation;
	Widget mbview_label_model_azimuth;
	Widget mbview_label_model;
	Widget mbview_separator9;
	Widget mbview_pushButton_view_3d_apply;
	Widget mbview_label_exager;
	Widget mbview_textField_exageration;
	Widget mbview_pushButton_view_3d_dismiss;
	Widget mbview_dialogShell_2dparms;
	Widget mbview_bulletinBoard_2dparms;
	Widget mbview_textField_view_2dzoom;
	Widget mbview_label_view_2dzoom;
	Widget mbview_textField_view_2doffsety;
	Widget mbview_label_view_2doffsety;
	Widget mbview_separator14;
	Widget mbview_textField_view_2doffsetx;
	Widget mbview_label_view_2doffsetx;
	Widget mbview_label_2d_offset;
	Widget mbview_pushButton_view_2d_apply;
	Widget mbview_pushButton_view_2d_dismiss;
	Widget mbview_dialogShell_projection;
	Widget mbview_bulletinBoard_projection;
	Widget mbview_label_displayprojection;
	Widget mbview_radioBox_projection;
	Widget mbview_toggleButton_geographic;
	Widget mbview_toggleButton_utm;
	Widget mbview_toggleButton_spheroid;
	Widget mbview_label_annotationstyle;
	Widget mbview_radioBox_annotation;
	Widget mbview_toggleButton_annotation_degreesminutes;
	Widget mbview_toggleButton_annotation_degreesdecimal;
	Widget mbview_label_projection;
	Widget mbview_pushButton_projection_dismiss;
	Widget mbview_dialogShell_profile;
	Widget mbview_form_profile;
	Widget mbview_scale_profile_width;
	Widget mbview_scale_profile_slope;
	Widget mbview_scrolledWindow_profile;
	Widget mbview_drawingArea_profile;
	Widget mbview_profile_label_info;
	Widget mbview_scale_profile_exager;
	Widget mbview_profile_pushButton_dismiss;
} MB3DViewData;

typedef struct _MB3DViewData *MB3DViewDataPtr;

MB3DViewDataPtr MB3DViewCreate(MB3DViewDataPtr, Widget, String, ArgList, Cardinal);

#endif  // MBVIEW_MB3DVIEW_H_
