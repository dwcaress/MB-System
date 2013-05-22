/*--------------------------------------------------------------------
 *    The MB-system:	mbedit.h	10/14/2009
 *    $Id: mbedit.h 1862 2010-06-07 00:26:46Z caress $
 *
 *    Copyright (c) 2009-2010 by
 *    David W. Caress (caress@mbari.org)
 *      Monterey Bay Aquarium Research Institute
 *      Moss Landing, CA 95039
 *    and Dale N. Chayes (dale@ldeo.columbia.edu)
 *      Lamont-Doherty Earth Observatory
 *      Palisades, NY 10964
 *
 *    See README file for copying and redistribution conditions.
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
 *
 * $Log: $
 *
 *
 */

/*--------------------------------------------------------------------*/

/* mode defines */
#define	MBWEDGE_DISPLAY_WEDGE		0
#define	MBWEDGE_DISPLAY_BOX		1
#define	MBWEDGE_COLORTABLE_HAXBY	0
#define	MBWEDGE_COLORTABLE_BRIGHT	1
#define	MBWEDGE_STRETCH_LINEAR		0
#define	MBWEDGE_STRETCH_LOG		1
#define	MBWEDGE_STRETCH_HISTOGRAM	2

#ifdef MBWEDGE_MAIN
#define EXTERNAL
#else
#define EXTERNAL extern
#endif

/* global status variables */
EXTERNAL	int	error;
EXTERNAL	int	verbose;

/* global variables */
EXTERNAL	int	format;
EXTERNAL	mb_path	input_file;
EXTERNAL	void	*mbwedge_xgid;
EXTERNAL	int	buffer_size;
EXTERNAL	int	nbuffer;
EXTERNAL	int	nloaded;
EXTERNAL	int	ndumped;
EXTERNAL	int	icurrent;

/* color control values */
#define	WHITE	0	
#define	BLACK	1	
#define RED	2
#define GREEN	3
#define BLUE	4
#define CORAL	5
#define LIGHTGREY	6
#define	XG_SOLIDLINE	0
#define	XG_DASHLINE	1
EXTERNAL	int	ncolors;
EXTERNAL	unsigned int	pixel_values[256];

/*--------------------------------------------------------------------*/

/* function prototypes */
void do_mbwedge_init(int argc, char **argv);
void do_fileselection_list( Widget w, XtPointer client_data, XtPointer call_data);
void do_load();

int do_wait_until_viewed(XtAppContext app);
int do_message_on(char *message);
int do_message_off();
int do_error_dialog(char *s1, char *s2, char *s3);
void set_label_string(Widget w, String str);
void set_label_multiline_string(Widget w, String str);
void get_text_string(Widget w, String str);


int mbwedge_init(int argc, char ** argv, int *startup_file);
int mbwedge_set_graphics(void *xgid, int ncol, unsigned int *pixels);
int mbwedge_action_open();
int mbwedge_action_load();
int mbwedge_action_dump();
int mbwedge_action_close();
int mbwedge_action_plot();
int mbwedge_action_quit();

XtPointer BX_CONVERT(Widget, char *, char *, int, Boolean *);

/*--------------------------------------------------------------------*/

