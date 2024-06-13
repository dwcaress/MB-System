#ifndef MBNAVEDIT_PROG_H
#define MBNAVEDIT_PROG_H

#include "mb_color.h"

int mbnavedit_init(int argc, char **argv, bool *startup_file,
		   /// graphics context pointer for use by referenced
		   /// graphics function (may be null)
		   void *gPtr,

		   /// Function to draw a line on canvas
		   void (*drawLine)(void *gPtr, int x1, int y1, int x2, int y2,
				    DrawingColor color, int style),

		   /// Function to draw a rectangle on canvas
		   void (*drawRect)(void *gPtr, int x, int y,
				    int width, int height,
				    DrawingColor color, int style),

		   /// Draw a filled rectangle on canvvas
		   void (*fillRect)(void *gPtr, int x, int y,
				    int width, int height,
				    DrawingColor color, int style),

		   /// Draw a string on canvas
		   void (*drawString)(void *gPtr, int x, int y, char *string,
				      DrawingColor color, int style),

		   /// Get dimensions of specified string drawn with active font
		   void (*justifyString)(void *gPtr, char *string, int *width,
					 int *ascent, int *descent),

		   /// Prepare for specified input file
		   void (*prepForInputFile)(char *file, int format),

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

		   void (*setUielements)(void));



int mbnavedit_set_graphics(void *xgid, int ncol);

int mbnavedit_init_globals(void);
  
int mbnavedit_clear_screen(void);

int mbnavedit_action_open(bool useprevious);

int mbnavedit_open_file(bool useprevious);

int mbnavedit_close_file(void);

int mbnavedit_dump_data(int hold);


int mbnavedit_load_data(void);

int mbnavedit_action_next_buffer(bool *quit);


int mbnavedit_action_offset(void);

int mbnavedit_action_close(void);

int mbnavedit_action_done(bool *quit);

int mbnavedit_action_quit(void);

int mbnavedit_action_step(int step);

int mbnavedit_action_end(void);

int mbnavedit_action_start(void);

int mbnavedit_action_mouse_pick(int xx, int yy);

int mbnavedit_action_mouse_select(int xx, int yy);

int mbnavedit_action_mouse_deselect(int xx, int yy);

int mbnavedit_action_mouse_selectall(int xx, int yy);

int mbnavedit_action_mouse_deselectall(int xx, int yy);

int mbnavedit_action_deselect_all(int type);

int mbnavedit_action_set_interval(int xx, int yy, int which);

int mbnavedit_action_use_dr(void);


int mbnavedit_action_use_smg(void);

int mbnavedit_action_use_cmg(void);

int mbnavedit_action_interpolate(void);

int mbnavedit_action_interpolaterepeats(void);

int mbnavedit_action_revert(void);

int mbnavedit_action_flag(void);

int mbnavedit_action_unflag(void);

int mbnavedit_action_fixtime(void);

int mbnavedit_action_deletebadtime(void);

int mbnavedit_action_showall(void);

int mbnavedit_get_smgcmg(int i);

int mbnavedit_get_model(void);

int mbnavedit_get_gaussianmean(void);

int mbnavedit_get_dr(void);

int mbnavedit_get_inversion(void);

int mbnavedit_plot_all(void);

int mbnavedit_plot_tint(int iplot);

int mbnavedit_plot_lon(int iplot);

int mbnavedit_plot_lat(int iplot);

int mbnavedit_plot_speed(int iplot);

int mbnavedit_plot_heading(int iplot);

int mbnavedit_plot_draft(int iplot);

int mbnavedit_plot_roll(int iplot);

int mbnavedit_plot_pitch(int iplot);

int mbnavedit_plot_heave(int iplot);

int mbnavedit_plot_tint_value(int iplot, int iping);

int mbnavedit_plot_lon_value(int iplot, int iping);

int mbnavedit_plot_lat_value(int iplot, int iping);

int mbnavedit_plot_speed_value(int iplot, int iping);

int mbnavedit_plot_heading_value(int iplot, int iping);

int mbnavedit_plot_draft_value(int iplot, int iping);

#endif

