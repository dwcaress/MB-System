/*------------------------------------------------------------------------------
 *    The MB-system:	MB3dsdg.h	10/28/2003
 *
 *    Copyright (c) 2003-2023 by
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

#ifndef MBVIEW_Mb3dsdg_H_
#define MBVIEW_Mb3dsdg_H_

typedef struct _Mb3dsdgData {
	Widget Mb3dsdg;
	Widget pushButton_reset;
	Widget scale_timelag;
	Widget scale_snell;
	Widget toggleButton_mouse_panzoom1;
	Widget toggleButton_mouse_rotate1;
	Widget label_mousemode;
	Widget scale_headingbias;
	Widget scale_pitchbias;
	Widget scale_rollbias;
	Widget label_status;
	Widget menuBar;
	Widget cascadeButton_view;
	Widget pulldownMenu_view;
	Widget toggleButton_view_boundingbox;
	Widget separator1;
	Widget toggleButton_view_flagged;
	Widget toggleButton_view_secondary;
	Widget separator;
	Widget toggleButton_view_noconnect;
	Widget toggleButton_view_connectgood;
	Widget toggleButton_view_connectall;
	Widget separator2;
	Widget toggleButton_view_scalewithflagged;
	Widget separator3;
	Widget toggleButton_view_colorbyflag;
	Widget toggleButton_view_colorbytopo;
	Widget toggleButton_view_colorbyamp;
	Widget cascadeButton_mouse;
	Widget pulldownMenu_mouse;
	Widget toggleButton_mouse_rotate;
	Widget toggleButton_mouse_panzoom;
	Widget cascadeButton_action;
	Widget pulldownMenu_action;
	Widget pushButton_action_applybias;
	Widget separator4;
	Widget pushButton_action_flagsparsevoxels_A;
	Widget pushButton_action_flagsparsevoxels_B;
	Widget pushButton_action_flagsparsevoxels_C;
	Widget pushButton_action_flagsparsevoxels_D;
	Widget pushButton_action_flagsparsevoxels_E;
	Widget pushButton_action_flagsparsevoxels_F;
	Widget separator5;
	Widget cascadeButton_action_colorsoundings;
	Widget pulldownMenu_action_colorsoundings;
	Widget pushButton_action_colorsoundingsblack;
	Widget pushButton_action_colorsoundingsred;
	Widget pushButton_action_colorsoundingsyellow;
	Widget pushButton_action_colorsoundingsgreen;
	Widget pushButton_action_colorsoundingsbluegreen;
	Widget pushButton_action_colorsoundingsblue;
	Widget pushButton_action_colorsoundingspurple;
	Widget separator6;
	Widget cascadeButton_action_optimizebiasvalues;
	Widget pulldownMenu_action_optimizebiasvalues;
	Widget pushButton_action_optimizebiasvalues_r;
	Widget pushButton_action_optimizebiasvalues_p;
	Widget pushButton_action_optimizebiasvalues_h;
	Widget pushButton_action_optimizebiasvalues_rp;
	Widget pushButton_action_optimizebiasvalues_rph;
	Widget pushButton_action_optimizebiasvalues_t;
	Widget pushButton_action_optimizebiasvalues_s;
	Widget cascadeButton_dismiss;
	Widget pulldownMenu_dismiss;
	Widget pushButton_dismiss;
	Widget drawingArea;
	Widget radioBox_soundingsmode;
	Widget toggleButton_mouse_toggle;
	Widget toggleButton_mouse_pick;
	Widget toggleButton_mouse_erase;
	Widget toggleButton_mouse_restore;
	Widget toggleButton_mouse_grab;
	Widget toggleButton_mouse_info;
} Mb3dsdgData;

typedef struct _Mb3dsdgData *Mb3dsdgDataPtr;

Mb3dsdgDataPtr Mb3dsdgCreate(Mb3dsdgDataPtr, Widget, String, ArgList, Cardinal);

#endif  // MBVIEW_Mb3dsdg_H_
