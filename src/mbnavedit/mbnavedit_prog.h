#ifndef MBNAVEDIT_PROG_H
#define MBNAVEDIT_PROG_H


int mbnavedit_init(int argc, char **argv, bool *startup_file,
		   void *gPtr,
		   void (*drawLineArg)(void *gPtr, int x1, int y1,
				       int x2, int y2,
				       unsigned int color, int style),
		   void (*drawRectArg)(void *gPtr, int x, int y,
				       int width, int height,
				       unsigned int color, int style),
		   void (*fillRectArg)(void *gPtr, int x, int y,
				       int width, int height,
				       unsigned int color, int style),
		   void (*drawStringArg)(void *gPtr, int x, int y, char *string,
					 unsigned int color, int style),
		   void (*justifyStringArg)(void *gPtr, char *string,
					    int *width,
					    int *ascent, int *descent));

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

