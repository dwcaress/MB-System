#ifndef MBNAVEDIT_PROG_H
#define MBNAVEDIT_PROG_H

#include "mb_define.h"
#include "mb_color.h"

/**
All definitions and prototypes in this header are windows-system agnostic,
since this header may be included by C++/Qt apps as well as older
C/X11/Motif apps.

*/

#ifdef MBNAVEDIT_DECLARE_GLOBALS
#define MBNAVEDIT_EXTERNAL
#else
#define MBNAVEDIT_EXTERNAL extern
#endif

/* mbnavedit global control parameters */
MBNAVEDIT_EXTERNAL int output_mode;
MBNAVEDIT_EXTERNAL bool run_mbprocess; 
MBNAVEDIT_EXTERNAL bool gui_mode; 
MBNAVEDIT_EXTERNAL int data_show_max;
MBNAVEDIT_EXTERNAL int data_show_size;
MBNAVEDIT_EXTERNAL int data_step_max;
MBNAVEDIT_EXTERNAL int data_step_size;
MBNAVEDIT_EXTERNAL int mode_pick;
MBNAVEDIT_EXTERNAL int mode_set_interval;
MBNAVEDIT_EXTERNAL int plot_tint;
MBNAVEDIT_EXTERNAL int plot_tint_org;
MBNAVEDIT_EXTERNAL int plot_lon;
MBNAVEDIT_EXTERNAL int plot_lon_org;
MBNAVEDIT_EXTERNAL int plot_lon_dr;
MBNAVEDIT_EXTERNAL int plot_lat;
MBNAVEDIT_EXTERNAL int plot_lat_org;
MBNAVEDIT_EXTERNAL int plot_lat_dr;
MBNAVEDIT_EXTERNAL int plot_speed;
MBNAVEDIT_EXTERNAL int plot_speed_org;
MBNAVEDIT_EXTERNAL int plot_smg;
MBNAVEDIT_EXTERNAL int plot_heading;
MBNAVEDIT_EXTERNAL int plot_heading_org;
MBNAVEDIT_EXTERNAL int plot_cmg;
MBNAVEDIT_EXTERNAL int plot_draft;
MBNAVEDIT_EXTERNAL int plot_draft_org;
MBNAVEDIT_EXTERNAL int plot_draft_dr;
MBNAVEDIT_EXTERNAL int plot_roll;
MBNAVEDIT_EXTERNAL int plot_pitch;
MBNAVEDIT_EXTERNAL int plot_heave;
MBNAVEDIT_EXTERNAL int mean_time_window;
MBNAVEDIT_EXTERNAL int drift_lon;
MBNAVEDIT_EXTERNAL int drift_lat;
MBNAVEDIT_EXTERNAL int timestamp_problem;  // TODO(schwehr): bool
MBNAVEDIT_EXTERNAL int use_ping_data;  // TODO(schwehr): bool
MBNAVEDIT_EXTERNAL int strip_comments;  // TODO(schwehr): bool
MBNAVEDIT_EXTERNAL int format;
MBNAVEDIT_EXTERNAL char ifile[MB_PATH_MAXLINE];
MBNAVEDIT_EXTERNAL char nfile[MB_PATHPLUS_MAXLINE];
MBNAVEDIT_EXTERNAL int nfile_defined;
MBNAVEDIT_EXTERNAL int model_mode;
MBNAVEDIT_EXTERNAL double weight_speed;
MBNAVEDIT_EXTERNAL double weight_acceleration;
MBNAVEDIT_EXTERNAL int scrollcount;
MBNAVEDIT_EXTERNAL double offset_lon;
MBNAVEDIT_EXTERNAL double offset_lat;
MBNAVEDIT_EXTERNAL double offset_lon_applied;
MBNAVEDIT_EXTERNAL double offset_lat_applied;

/* mbnavedit plot size parameters */
MBNAVEDIT_EXTERNAL int plot_width;
MBNAVEDIT_EXTERNAL int plot_height;
MBNAVEDIT_EXTERNAL int number_plots;
MBNAVEDIT_EXTERNAL int window_width;
MBNAVEDIT_EXTERNAL int window_height;

/* Mode value defines */
#define PICK_MODE_PICK 0
#define PICK_MODE_SELECT 1
#define PICK_MODE_DESELECT 2
#define PICK_MODE_SELECTALL 3
#define PICK_MODE_DESELECTALL 4
#define OUTPUT_MODE_OUTPUT 0
#define OUTPUT_MODE_BROWSE 1
#define PLOT_TINT 0
#define PLOT_LONGITUDE 1
#define PLOT_LATITUDE 2
#define PLOT_SPEED 3
#define PLOT_HEADING 4
#define PLOT_DRAFT 5
#define PLOT_ROLL 6
#define PLOT_PITCH 7
#define PLOT_HEAVE 8
#define MODEL_MODE_OFF 0
#define MODEL_MODE_MEAN 1
#define MODEL_MODE_DR 2
#define MODEL_MODE_INVERT 3
#define NUM_FILES_MAX 1000

// Function prototypes
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

		   /// Prepare for specified input file(s)
		   void (*parseInputDataList)(char *file, int format),

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

/// Set input filename
void mbnavedit_set_inputfile(char *filename);

#endif

