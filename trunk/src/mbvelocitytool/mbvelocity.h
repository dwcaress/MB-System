/*--------------------------------------------------------------------
 *    The MB-system:	mbvelocity.h	10/15/2009
 *    $Id: $
 *
 *    Copyright (c); 2009-2009 by
 *    David W. Caress (caress@mbari.org);
 *      Monterey Bay Aquarium Research Institute
 *      Moss Landing, CA 95039
 *    and Dale N. Chayes (dale@ldeo.columbia.edu);
 *      Lamont-Doherty Earth Observatory
 *      Palisades, NY 10964
 *
 *    See README file for copying and redistribution conditions.
 *--------------------------------------------------------------------*/
/*
 * MBVELOCITYTOOL is an interactive water velocity profile editor 
 * used to examine multiple water velocity profiles and to create 
 * new water velocity profiles which can be used for the processing 
 * of multibeam sonar data.  In general, this tool is used to examine 
 * water velocity profiles obtained from XBTs, CTDs, or databases, 
 * and to construct new profiles consistent with these various 
 * sources of information. This file contains
 * contains function prototypes and was added in 2009.
 *
 * Author:	D. W. Caress
 * Date:	October 15, 2009
 *
 * $Log: $
 *
 *
 */

/*--------------------------------------------------------------------*/

/* mbvelocitytool control defines */
#define	MAX_PROFILES	100
#define	PICK_DISTANCE	50
#define	NUM_EDIT_START	6

/* velocity profile structure definition */
struct profile
	{
	int	n;
	int	nalloc;
	char	name[MB_PATH_MAXLINE];
	double	*depth;
	double	*velocity;
	};

/* ping structure definition */
struct mbvt_ping_struct 
	{
	int	allocated;
	int	time_i[7];
	double	time_d;
	double	navlon;
	double	navlat;
	double	speed;
	double	heading;
	double	sonardepth;
	double	ssv;
	int	beams_bath;
	char	*beamflag;
	double	*bath;
	double	*bathacrosstrack;
	double	*bathalongtrack;
	double	*ttimes;
	double	*angles;
	double	*angles_forward;
	double	*angles_null;
	double	*heave;
	double	*alongtrack_offset;
	};

/* function prototypes */
void do_mbvelocity_init(int argc, char **argv);
void do_set_controls();
void do_velrange( Widget w, XtPointer client_data, XtPointer call_data);
void do_velcenter( Widget w, XtPointer client_data, XtPointer call_data);
void do_process_mb( Widget w, XtPointer client_data, XtPointer call_data);
void do_maxdepth( Widget w, XtPointer client_data, XtPointer call_data);
void do_anglemode( Widget w, XtPointer client_data, XtPointer call_data);
void do_quit( Widget w, XtPointer client_data, XtPointer call_data);
void do_fileselection_list( Widget w, XtPointer client_data, XtPointer call_data);
void do_open( Widget w, XtPointer client_data, XtPointer call_data);
void do_open_commandline(char *wfile, char *sfile, char *file, int format);
void do_new_profile( Widget w, XtPointer client_data, XtPointer call_data);
void do_residual_range( Widget w, XtPointer client_data, XtPointer call_data);
void do_canvas_event( Widget w, XtPointer client_data, XtPointer call_data);
void do_save_swath_svp( Widget w, XtPointer client_data, XtPointer call_data);
void do_save_residuals( Widget w, XtPointer client_data, XtPointer call_data);
void do_io_mode_mb( Widget w, XtPointer client_data, XtPointer call_data);
void do_io_mode_open_svp_display( Widget w, XtPointer client_data, XtPointer call_data);
void do_io_mode_save_svp( Widget w, XtPointer client_data, XtPointer call_data);
void do_io_mode_open_svp_edit( Widget w, XtPointer client_data, XtPointer call_data);
void do_expose( Widget w, XtPointer client_data, XtPointer call_data);
int do_wait_until_viewed(XtAppContext app);
int do_message_on(char *message);
int do_message_off();
int do_error_dialog(char *s1, char *s2, char *s3);
void set_label_string(Widget w, String str);
void set_label_multiline_string(Widget w, String str);
void get_text_string(Widget w, String str);

int mbvt_init(int argc, char **argv);
int mbvt_quit();
int mbvt_set_graphics(void *xgid, int *brdr, int ncol, unsigned int *pixels);
int mbvt_get_values(int *s_edit, int *s_ndisplay, double *s_maxdepth,
	double *s_velrange, double *s_velcenter, double *s_resrange, 
	int *s_anglemode, int *s_format);
int mbvt_set_values(int s_edit, int s_ndisplay, 
		double s_maxdepth, double s_velrange, double s_velcenter, 
		double s_resrange, int s_anglemode);
int mbvt_open_edit_profile(char *file);
int mbvt_new_edit_profile();
int mbvt_save_edit_profile(char *file);
int mbvt_save_swath_profile(char *file);
int mbvt_save_residuals(char *file);
int mbvt_open_display_profile(char *file);
int mbvt_get_display_names(int *nlist, char *list[MAX_PROFILES]);
int mbvt_delete_display_profile(int select);
int mbvt_plot();
int mbvt_action_select_node(int x, int y);
int mbvt_action_mouse_up(int x, int y);
int mbvt_action_drag_node(int x, int y);
int mbvt_action_add_node(int x, int y);
int mbvt_action_delete_node(int x, int y);
int mbvt_get_format(char *file, int *form);
int mbvt_open_swath_file(char *file, int form, int *numload);
int mbvt_deallocate_swath();
int mbvt_process_multibeam();

XtPointer BX_CONVERT(Widget, char *, char *, int, Boolean *);

/*--------------------------------------------------------------------*/

