/*--------------------------------------------------------------------
 *    The MB-system:	mbedit_prog.h	3/27/2024
 *
 *    Copyright (c) 2009-2024 by
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
 * MBEDIT is an interactive beam editor for multibeam bathymetry data.
 * It can work with any data format supported by the MBIO library.
 * This version uses the MOTIF toolkit and has been developed using
 * the Builder Xsessory package by ICS.  This file contains
 * contains function prototypes and was added in 2009.
 *
 * Author:	D. W. Caress
 * Date:	October 14, 2009
 */

#ifndef MBEDIT_PROG_H
#define MBEDIT_PROG_H

/* plot modes */
/// Ancillary data to be plotted 
typedef enum {
  NO_ANCILL = 0,
  TIME,
  INTERVAL,
  LONGITUDE,
  LATITUDE,
  HEADING,
  SPEED,
  DEPTH,
  ALTITUDE,
  SENSORDEPTH,
  ROLL,
  PITCH,
  HEAVE
  
} PlotAncillData;


/// Plot-slice modes
typedef enum {
  WATERFALL,
  ALONGTRACK,
  ACROSSTRACK
  
} PlotSliceMode;



/// Sounding color interpretation
typedef enum {
  FLAG,        // color by flagging
  DETECT,      // color by bottom detection algorithm
  PULSE        // color by up-chirp/down-chirp pulse source
  
} SoundColorCoding;


/// Mouse edit grab modes
typedef enum {
  GRAB_START,
  GRAB_MOVE,
  GRAB_END
  
} MouseGrabMode;
  
/// Plot element colors
typedef enum {
    WHITE = 0,
    BLACK = 1,
    RED = 2,
    GREEN = 3,
    BLUE = 4,
    CORAL = 5,
    LIGHTGREY = 6,

} mbedit_color_t;


/// This source code is windowing-system-agnostic, but 
/// accepts function pointers that interact with specific windowing system
int mbedit_init(int argc, char **argv,
		/// #input files specified on command line
		int *startup_file,

		/// graphics context pointer for use by referenced
		/// graphics function (may be null)
		void *gPtr,

		/// Function to draw a line on canvas
		void (*drawLine)(void *gPtr, int x1, int y1, int x2, int y2,
				 mbedit_color_t color, int style),

		/// Function to draw a rectangle on canvas
		void (*drawRect)(void *gPtr, int x, int y,
				 int width, int height,
				 mbedit_color_t color, int style),

		/// Draw a filled rectangle on canvvas
		void (*fillRect)(void *gPtr, int x, int y,
				 int width, int height,
				 mbedit_color_t color, int style),

		/// Draw a string on canvas
		void (*drawString)(void *gPtr, int x, int y, char *string,
				   mbedit_color_t color, int style),

		/// Get dimensions of specified string drawn with active font
		void (*justifyString)(void *gPtr, char *string, int *width,
				      int *ascent, int *descent),
		
		void (*parseDatalist)(char *file, int format),

		/// Display error messages (e.g. with dialog)
		int (*showError)(char *s1, char *s2, char *s3),

		/// Display a message (e.g. with dialog)
		int (*showMessage)(char *),

		/// Hide message dialog
		int (*hideMessage)(void),

		/// Enable GUI element that specified input file
		void (*enableFileButton)(void),

		/// Disable GUI element that specified input file		
		void (*disableFileButton)(void),

		/// Enable GUI element that displays next plot
		void (*enableNextButton)(void),

		/// Disable GUI element that displays next plot
		void (*disableNextButton)(void),

		/// Reset displayed x-scale on GUI (e.g. slider)
		int (*resetScaleX)(int pwidth, int maxx,
				    int xntrvl, int yntrvl)
		);


int mbedit_set_graphics(void *xgid, int ncol, unsigned int *pixels);

int mbedit_set_scaling(int *brdr, PlotAncillData ancillData);

int mbedit_set_filters(int f_m, int f_m_t, int f_m_x, int f_m_l,
		       int f_w, int f_w_t, int f_b, int f_b_b, int f_b_e,
		       int f_d, double f_d_b, double f_d_e, int f_a,
		       double f_a_b, double f_a_e);

int mbedit_get_filters(int *b_m, double *d_m, int *f_m, int *f_m_t,
		       int *f_m_x, int *f_m_l, int *f_w, int *f_w_t, int *f_b,
                       int *f_b_b, int *f_b_e, int *f_d, double *f_d_b,
		       double *f_d_e, int *f_a, double *f_a_b, double *f_a_e);

int mbedit_get_defaults(int *plt_size_max, int *plt_size,
			SoundColorCoding *colorCoding,
			bool *showFlagSound, bool *showFlagProf,
			PlotAncillData *ancillData,
			int *buffer_size_max, int *buffer_size, int *hold_size,
			int *form, int *plwd, int *exgr,
			int *xntrvl, int *yntrvl, int *ttime_i, int *outmode);


int mbedit_get_viewmode(PlotSliceMode *plotSliceMode);

int mbedit_set_viewmode(PlotSliceMode plotSliceMode);


/// Open swath file and plot its data
int mbedit_action_open(char *file, int form, int fileid, int numfiles,
		       int savemode, int outmode, int plwd, int exgr,
		       int xntrvl, int yntrvl, int plt_size,
		       SoundColorCoding colorCoding,
		       bool sh_flggdsdg, bool sh_flggdprf,
		       PlotAncillData ancillData,
		       int *buffer_size, int *buffer_size_max,
                       int *hold_size, int *ndumped, int *nloaded,
		       int *nbuffer, int *ngood, int *icurrent, int *nplt);

int mbedit_action_next_buffer(int hold_size, int buffer_size, int plwd,
			      int exgr, int xntrvl, int yntrvl, int plt_size,
                              SoundColorCoding colorCoding,
			      bool sh_flggdsdg, bool sh_flggdprf,
			      PlotAncillData ancillData,
			      int *ndumped, int *nloaded,
			      int *nbuffer, int *ngood,
                              int *icurrent, int *nplt, int *quit);

int mbedit_action_close(int buffer_size, int *ndumped, int *nloaded,
			int *nbuffer, int *ngood, int *icurrent);

int mbedit_action_done(int buffer_size, int *ndumped, int *nloaded,
		       int *nbuffer, int *ngood, int *icurrent, int *quit);

int mbedit_action_quit(int buffer_size, int *ndumped, int *nloaded,
		       int *nbuffer, int *ngood, int *icurrent);

int mbedit_action_step(int step, int plwd, int exgr, int xntrvl, int yntrvl,
		       int plt_size, SoundColorCoding colorCoding,
		       bool sh_flggdsdg, bool sh_flggdprf,
		       PlotAncillData ancillData, int *nbuffer,
		       int *ngood, int *icurrent, int *nplt);

/// Plot swath data
int mbedit_action_plot(int plotWidth,
		       int verticalExagg,
		       int xInterval, int yInterval,
		       int plt_size,
		       SoundColorCoding colorCoding,
		       bool showFlagSounding,
		       bool showFlagProfile,
		       PlotAncillData ancillDataOption,
		       int *nbuffer, int *ngood,
		       int *icurrent, int *nplt);

int mbedit_action_mouse_toggle(int x_loc, int y_loc, int plwd, int exgr,
			       int xntrvl, int yntrvl, int plt_size,
			       SoundColorCoding colorCoding,
			       bool sh_flggdsdg, bool sh_flggdprf,
			       PlotAncillData ancillData,
			       int *nbuffer, int *ngood,
			       int *icurrent, int *nplt);

int mbedit_action_mouse_pick(int x_loc, int y_loc, int plwd, int exgr,
			     int xntrvl, int yntrvl, int plt_size,
			     SoundColorCoding colorCoding,
			     bool sh_flggdsdg, bool sh_flggdprf,
			     PlotAncillData ancillData, int *nbuffer,
			     int *ngood, int *icurrent, int *nplt);

int mbedit_action_mouse_erase(int x_loc, int y_loc, int plwd, int exgr,
			      int xntrvl, int yntrvl, int plt_size,
			      SoundColorCoding colorCoding,
			      bool sh_flggdsdg, bool sh_flggdprf,
			      PlotAncillData ancillData,
			      int *nbuffer, int *ngood,
			      int *icurrent, int *nplt);

int mbedit_action_mouse_restore(int x_loc, int y_loc, int plwd, int exgr,
				int xntrvl, int yntrvl, int plt_size,
				SoundColorCoding colorCoding,
				bool sh_flggdsdg, bool sh_flggdprf,
				PlotAncillData ancillData, int *nbuffer,
				int *ngood, int *icurrent, int *nplt);

int mbedit_action_mouse_grab(MouseGrabMode grabmode, int x_loc, int y_loc,
			     int plwd,
			     int exgr, int xntrvl, int yntrvl, int plt_size,
                             SoundColorCoding colorCoding,
			     bool sh_flggdsdg, bool sh_flggdprf,
			     PlotAncillData ancillData, int *nbuffer,
			     int *ngood, int *icurrent, int *nplt);

int mbedit_action_mouse_info(int x_loc, int y_loc, int plwd, int exgr,
			     int xntrvl, int yntrvl, int plt_size,
			     SoundColorCoding colorCoding,
                             bool sh_flggdsdg, bool sh_flggdprf,
			     PlotAncillData ancillData,
			     int *nbuffer, int *ngood, int *icurrent,
			     int *nplt);

int mbedit_action_zap_outbounds(int iping, int plwd, int exgr, int xntrvl,
				int yntrvl, int plt_size,
				SoundColorCoding colorCoding,
				bool sh_flggdsdg, bool sh_flggdprf,
                                PlotAncillData ancillData, int *nbuffer,
				int *ngood,
				int *icurrent, int *nplt);

int mbedit_action_bad_ping(int plwd, int exgr, int xntrvl, int yntrvl,
			   int plt_size, SoundColorCoding colorCoding,
			   bool sh_flggdsdg, bool sh_flggdprf,
			   PlotAncillData ancillData,
                           int *nbuffer, int *ngood, int *icurrent, int *nplt);

int mbedit_action_good_ping(int plwd, int exgr, int xntrvl, int yntrvl,
			    int plt_size, SoundColorCoding colorCoding,
			    bool sh_flggdsdg, bool sh_flggdprf,
			    PlotAncillData ancillData,
                            int *nbuffer, int *ngood, int *icurrent, int *nplt);

int mbedit_action_left_ping(int plwd, int exgr, int xntrvl, int yntrvl,
			    int plt_size, SoundColorCoding colorCoding,
			    bool sh_flggdsdg,
			    bool sh_flggdprf, PlotAncillData ancillData,
                            int *nbuffer, int *ngood, int *icurrent, int *nplt);

int mbedit_action_right_ping(int plwd, int exgr, int xntrvl, int yntrvl,
			     int plt_size, SoundColorCoding colorCoding,
			     bool sh_flggdsdg,
			     bool sh_flggdprf, PlotAncillData ancillData,
			     int *nbuffer,
			     int *ngood, int *icurrent, int *nplt);

int mbedit_action_zero_ping(int plwd, int exgr, int xntrvl, int yntrvl,
			    int plt_size, SoundColorCoding colorCoding,
			    bool sh_flggdsdg,
			    bool sh_flggdprf, PlotAncillData ancillData,
                            int *nbuffer, int *ngood, int *icurrent, int *nplt);

int mbedit_action_flag_view(int plwd, int exgr, int xntrvl, int yntrvl,
			    int plt_size, SoundColorCoding colorCoding,
			    bool sh_flggdsdg,
			    bool sh_flggdprf, PlotAncillData ancillData,
                            int *nbuffer, int *ngood,
			    int *icurrent, int *nplt);

int mbedit_action_unflag_view(int plwd, int exgr, int xntrvl, int yntrvl,
			      int plt_size, SoundColorCoding colorCoding,
			      bool sh_flggdsdg,
			      bool sh_flggdprf, PlotAncillData ancillData,
                              int *nbuffer, int *ngood,
			      int *icurrent, int *nplt);

int mbedit_action_unflag_all(int plwd, int exgr, int xntrvl, int yntrvl,
			     int plt_size, SoundColorCoding colorCoding,
			     bool sh_flggdsdg,
			     bool sh_flggdprf, PlotAncillData ancillData,
			     int *nbuffer,
			     int *ngood, int *icurrent, int *nplt);

int mbedit_action_filter_all(int plwd, int exgr, int xntrvl, int yntrvl,
			     int plt_size, SoundColorCoding colorCoding,
			     bool sh_flggdsdg,
			     bool sh_flggdprf, PlotAncillData ancillData,
                             int *nbuffer, int *ngood,
			     int *icurrent, int *nplt);

int mbedit_filter_ping(int iping);

int mbedit_get_format(char *file, int *form);

int mbedit_open_file(char *file, int form, bool savemode);

int mbedit_close_file(void);

int mbedit_dump_data(int hold_size, int *ndumped, int *nbuffer);

int mbedit_load_data(int buffer_size, int *nloaded, int *nbuffer, int *ngood,
		     int *icurrent);

int mbedit_clear_screen(void);

int mbedit_plot_all(int plwd, int exgr, int xntrvl, int yntrvl, int plt_size,
		    SoundColorCoding colorCoding,
		    bool sh_flggdsdg, bool sh_flggdprf,
		    PlotAncillData ancillData,
		    int *nplt, bool autoscale);

int mbedit_plot_beam(int iping, int jbeam);

int mbedit_plot_ping(int iping);

int mbedit_plot_ping_label(int iping, bool save);

int mbedit_plot_info(void);

int mbedit_unplot_beam(int iping, int jbeam);

int mbedit_unplot_ping(int iping);

int mbedit_unplot_info(void);

int mbedit_action_goto(int ttime_i[7], int hold_size, int buffer_size, int plwd,
		       int exgr, int xntrvl, int yntrvl, int plt_size,
                       SoundColorCoding colorCoding,
		       bool sh_flggdsdg, bool sh_flggdprf,
		       PlotAncillData ancillData,
		       int *ndumped, int *nloaded, int *nbuffer,
		       int *ngood, int *icurrent, int *nplt);

int mbedit_tslabel(PlotAncillData data_id, char *label);

int mbedit_tsvalue(int iping, PlotAncillData data_id, double *value);

int mbedit_tsminmax(int iping, int nping, PlotAncillData data_id,
		    double *tsmin,
		    double *tsmax);

int mbedit_xtrackslope(int iping, double *slope);

#endif
