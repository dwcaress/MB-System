/*--------------------------------------------------------------------------*/
/*
 *	naverr -	part of the navadjust interactive multi-beam 
 *			bathymetry navigation adjustment package
 *
 *			naverr graphically finds the relative mislocation 
 *			between overlapping or crossing swaths.
 *
 *			naverr uses sunview graphics which are specific
 *			to sun workstations - this is primarily a result
 *			of stupidity and laziness on the programmer's part.
 *
 *			David W. Caress
 *			Lamont-Doherty Geological Observatory
 *			begun August 1990
 *			satisfactory version completed May 1991
 */

/* standard global include files */
#include <stdio.h>
#include <math.h>
#include <strings.h>

/* sunview global include files */
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sun/fbio.h>
#include <suntool/sunview.h>
#include <suntool/canvas.h>
#include <pixrect/pixrect_hs.h>
#include <suntool/panel.h>
#include <suntool/alert.h>

/* global sunview variables */
Frame	frame, corr_frame;
Canvas	canvas, corr_canvas;
Pixwin	*screen, *corr_screen;
Pixrect	*store1, *store2;
Panel	panel, corr_panel;
Panel_item	close_item;
Panel_item	quit_item;
Panel_item	next_item;
Panel_item	skip_item;
Panel_item	none_item;
Panel_item	redo_item;
Panel_item	reset_item;
Panel_item	save_item;
Panel_item	redraw_item;
Panel_item	contour_item;
Panel_item	color_item;
Panel_item	contint_item;
Panel_item	colrint_item;
Panel_item	depth_item;
Panel_item	drag_item;
Panel_item	blowup_item;
Panel_item	blowdown_item;
Panel_item	sextras_item;
Panel_item	hextras_item;
Panel_item	time1_item;
Panel_item	time2_item;
Panel_item	uncertainty_item;
Panel_item	correlation_item;
Panel_item	best_item;

/* panel control procedures */
void	close_proc(), quit_proc(), next_proc(), skip_proc(), none_proc();
void	redo_proc(), reset_proc(), save_proc(), redraw_proc();
void	contour_proc(), color_proc(), depth_proc(), track_proc();
void	show_contour(), drag_proc(), blowup_proc(), blowdown_proc(), zoom_proc();
void	sextras_proc(), hextras_proc(), time1_proc(), time2_proc(), alert_proc();
void	uncertainty_proc(), spacing_proc();
void	correlation_proc(), corr_track_proc(), best_proc();
Panel_setting	contint_proc(), colrint_proc();

/* void function declarations */
void	clear_screen(), clear_store1(), clear_store2();
void	get_bounds(), do_scale(), km_scale();
void	plot(), plot_pings(), plot_vectors();
void	show_time(), erase_time(), scale_track();

/* global array dimensions etc */
#define MAXLINE 256
#define MAXPINGS 1000
#define MAXVECTOR 100000
#define MAXDIM 31
#define RGBSIZE 32
#define PI 3.1415926
#define DTR PI/180.
#define ZERO 0.0
#define BLOWUP 1.2
#define IUP 3
#define IDN 2
#define IOR -3
#define ICL 0

/* global plotting control variables */
unsigned char red[RGBSIZE], green[RGBSIZE], blue[RGBSIZE];
int	draw_op, clear_op, transfer_op, time_op;
int	screen_width;
int	screen_height;
int	screen_depth;
int	canvas_width;
int	canvas_height;
int	corr_width;
int	corr_height;

/* defining plot frame icon */
Icon	icon;
static short icon_image[] = {
#include "naverr_icon"
};
mpr_static(naverr_icon,64,64,1,icon_image);

/* global structure definitions */
struct section
	{
	int	fileid;
	int	format;
	int	prior;
	int	post;
	int	btime_i[6];
	int	etime_i[6];
	int	output;
	int	nrec;
	double	distance;
	};
struct ping
	{
	double	*dep;
	double	*lon;
	double	*lat;
	};
struct swath
	{
	int	format;
	int	beams;
	int	npings;
	struct ping data[MAXPINGS];
	};

/* global control variables */
int	status;
int	ncross;
int	isec1, isec2;
struct section sec1, sec1i, sec1f, sec2, sec2i, sec2f;
char	outfile[128], reportfile[128];
FILE	*ofp, *rfp;
double	xoff, yoff;
int	ixoff, iyoff, ixoff1, iyoff1, ixoff2, iyoff2;
int	event_num = 0;
char	datalist[128], navlist[128], cmdfil[128];

/* global multibeam data arrays */
struct swath	swath1, swath1i, swath1f;
struct swath	swath2, swath2i, swath2f;

/* global multibeam ping time arrays */
int	ntime1, ntime2;
double	ttime1[3*MAXPINGS], tlon1[3*MAXPINGS], tlat1[3*MAXPINGS];
double	ttime2[3*MAXPINGS], tlon2[3*MAXPINGS], tlat2[3*MAXPINGS];
int	time1set, time2set;

/* global plot scaling variables */
double	xmin, xmax, ymin, ymax;
double	xmins, xmaxs, ymins, ymaxs;
double	ox, oy, xscale, yscale;
double	spaceval_cur, ox_corr, oy_corr, xoffmin, yoffmin;
int	ix, iy, ixo, iyo;

/* global other stuff */
int	color[5];
double	contour_int, color_int, tick_int, tick_len, label_hgt;
double	uncertval, spaceval;
int	drag, blowup, sector, showextras, depthlab, showcorr;
int	time1_val, time2_val;
char	contour_str[20], color_str[20], uncertval_str[20], spaceval_str[20];
char	title_str[128];
int	nold, *old1, *old2;
int	*nvector, nvector1, nvector2;
double	*vector, vector1[3*MAXVECTOR], vector2[3*MAXVECTOR];

main(argc, argv)
int argc;
char **argv; 
{
	int	i;
	int	quit;
	FILE	*fp;
	int	filemod = 493;

	/* initialize filenames */
	strcpy(datalist,"data.list");
	strcpy(navlist,"nav.list");
	strcpy(cmdfil,"navsolve.cmd");
	quit = 0;

	/* process argument list */
	for (i = 1; i < argc; i++) 
		{
		if (argv[i][0] == '-') 
			{
			switch (argv[i][1]) 
				{
				case 'I':
				case 'i':
					strcpy (datalist, &argv[i][2]);
					break;
				case 'N':
				case 'n':
					strcpy (navlist, &argv[i][2]);
					break;
				case 'Q':
				case 'q':
					quit = 1;
					break;
				}
			}
		}

	/* if forced to quit act as if finished */
	if (quit)
		{

		/* output navsolve command file */
		if ((fp = fopen(cmdfil,"w")) == NULL)
			{
			printf("unable to open cmd file:%s\n",cmdfil);
			exit(-1);
			}
		fprintf(fp,"# command file to set up navigation adjustment\n");
		fprintf(fp,"# inverse problem\n");
		fprintf(fp,"navsolve -I%s -N%s $1\n",datalist,navlist);
		fclose(fp);
		status = chmod(cmdfil,filemod);

		printf("naverr forced to finish prematurely\n");
		exit(-1);
		}


	/* get started */
	if ((status = io_init()) != 0)
		{
		printf("input/output initialization failed\n");
		exit(-1);
		}

	/* initialize screen graphics */
	if ((status = screen_init()) != 0)
		{
		printf("sunview plotting initialization failed\n");
		exit(-1);
		}

	/* read in and plot the first crossover */
	skip_proc();

	/* enter main loop */
	window_main_loop(frame);

	/* end (everything taken care of in quit_proc) */
	exit(0);
}

/*--------------------------------------------------------------------------*/
/* 	function io_init sets up input and output.
 *	A status int is returned. */
int io_init()
{
	int	status = 0;
	int	i, i1, i2;

	/* first look for old output file */
	strcpy(reportfile,"naverr.report");
	if ((rfp = fopen(reportfile,"r")) == NULL)
		{

		/* open new output files */
		status = 0;
		fclose(rfp);
		if ((rfp = fopen(reportfile,"w")) == NULL)
			{
			printf("cannot open output file:%s\n",reportfile);
			status = 1;
			}
		strcpy(outfile,"naverr.list");
		if ((ofp = fopen(outfile,"w")) == NULL)
			{
			printf("cannot open output file:%s\n",outfile);
			status = 1;
			}
		return(status);
		}

	/* find number of crossovers already dealt with */
	nold = 0;
	while ((status = fscanf(rfp,"%d %d",&i1,&i2)) == 2)
		nold++;
	fclose(rfp);
	fclose(ofp);

	/* read in old crossovers */
	if (nold > 0)
		{
		if ((old1 = (int *) calloc(nold,sizeof(int))) == NULL) 
			exit(-1);
		if ((old2 = (int *) calloc(nold,sizeof(int))) == NULL) 
			exit(-1);
		if ((rfp = fopen(reportfile,"r")) == NULL)
			{
			printf("cannot open report file:%s\n",reportfile);
			return(status = 1);
			}
		for (i=0;i<nold;i++)
			fscanf(rfp,"%d %d",&old1[i],&old2[i]);
		close(rfp);
		}

	/* if nothing in report file open new output files */
	if (nold == 0)
		{
		status = 0;
		if ((rfp = fopen(reportfile,"w")) == NULL)
			{
			printf("cannot open output file:%s\n",reportfile);
			status = 1;
			}
		strcpy(outfile,"naverr.list");
		if ((ofp = fopen(outfile,"w")) == NULL)
			{
			printf("cannot open output file:%s\n",outfile);
			status = 1;
			}
		return(status);
		}

	/* open old output files to be appended */
	status = 0;
	if ((rfp = fopen(reportfile,"a")) == NULL)
		{
		printf("cannot open old output file:%s\n",reportfile);
		status = 1;
		}
	strcpy(outfile,"naverr.list");
	if ((ofp = fopen(outfile,"a")) == NULL)
		{
		printf("cannot open old output file:%s\n",outfile);
		status = 1;
		}

	return(status);
}

/*--------------------------------------------------------------------------*/
/* 	function screen_init initializes the sunview graphics. */
int screen_init()
{
	int	status, i;
		WIN_WIDTH,		corr_width+10,
		WIN_HEIGHT,		corr_height+63,

/*	Determine particulars of graphics device.  */
	screen_width = 1152;
	screen_height = 900;
	canvas_width = 900;
	canvas_height = 830;
	screen_depth = 8;
	icon = icon_create(ICON_IMAGE, &naverr_icon, 0);

/*	Create the frame and its canvas and panels.  */
	frame = window_create(NULL, FRAME,
		WIN_WIDTH, 		screen_width,
		WIN_HEIGHT, 		screen_height, 
		WIN_X,			0,
		WIN_Y,			0,
		FRAME_ICON,		icon,
		FRAME_LABEL,		"NAVERR",
		WIN_ERROR_MSG,		"Fatal error:  Suntools not active!",
		FRAME_NO_CONFIRM,	TRUE,
		0);
	canvas = window_create(frame, CANVAS,
		CANVAS_AUTO_EXPAND,	FALSE,
		CANVAS_AUTO_SHRINK,	FALSE,
		CANVAS_WIDTH, 		canvas_width,
		CANVAS_HEIGHT, 		canvas_height, 
		WIN_X,			0,
		WIN_Y,			0,
		WIN_EVENT_PROC,		track_proc,
		0);
	panel = window_create(frame, PANEL,
		WIN_X,			950,
		WIN_Y,			0,
		0);
	close_item = panel_create_item(panel, PANEL_BUTTON,
		PANEL_ITEM_X,		ATTR_COL(1),
		PANEL_ITEM_Y,		ATTR_ROW(1),
		PANEL_LABEL_IMAGE,	
		panel_button_image(panel, "close", 0, 0),
		PANEL_LABEL_BOLD,	TRUE,
		PANEL_NOTIFY_PROC,	close_proc,
		0);
	quit_item = panel_create_item(panel, PANEL_BUTTON,
		PANEL_ITEM_X,		ATTR_COL(9),
		PANEL_ITEM_Y,		ATTR_ROW(1),
		PANEL_LABEL_IMAGE,	
		panel_button_image(panel, "quit", 0, 0),
		PANEL_LABEL_BOLD,	TRUE,
		PANEL_NOTIFY_PROC,	quit_proc,
		0);
	next_item = panel_create_item(panel, PANEL_BUTTON,
		PANEL_ITEM_X,		ATTR_COL(1),
		PANEL_ITEM_Y,		ATTR_ROW(3),
		PANEL_LABEL_IMAGE,	
		panel_button_image(panel, "next", 0, 0),
		PANEL_LABEL_BOLD,	TRUE,
		PANEL_NOTIFY_PROC,	next_proc,
		0);
	skip_item = panel_create_item(panel, PANEL_BUTTON,
		PANEL_ITEM_X,		ATTR_COL(9),
		PANEL_ITEM_Y,		ATTR_ROW(3),
		PANEL_LABEL_IMAGE,	
		panel_button_image(panel, "skip", 0, 0),
		PANEL_LABEL_BOLD,	TRUE,
		PANEL_NOTIFY_PROC,	skip_proc,
		0);
	save_item = panel_create_item(panel, PANEL_BUTTON,
		PANEL_ITEM_X,		ATTR_COL(1),
		PANEL_ITEM_Y,		ATTR_ROW(5),
		PANEL_LABEL_IMAGE,	
		panel_button_image(panel, "save", 0, 0),
		PANEL_LABEL_BOLD,	TRUE,
		PANEL_NOTIFY_PROC,	save_proc,
		0);
	none_item = panel_create_item(panel, PANEL_BUTTON,
		PANEL_ITEM_X,		ATTR_COL(9),
		PANEL_ITEM_Y,		ATTR_ROW(5),
		PANEL_LABEL_IMAGE,	
		panel_button_image(panel, "none", 0, 0),
		PANEL_LABEL_BOLD,	TRUE,
		PANEL_NOTIFY_PROC,	none_proc,
		0);
	redo_item = panel_create_item(panel, PANEL_BUTTON,
		PANEL_ITEM_X,		ATTR_COL(1),
		PANEL_ITEM_Y,		ATTR_ROW(7),
		PANEL_LABEL_IMAGE,	
		panel_button_image(panel, "redo", 0, 0),
		PANEL_LABEL_BOLD,	TRUE,
		PANEL_NOTIFY_PROC,	redo_proc,
		0);
	reset_item = panel_create_item(panel, PANEL_BUTTON,
		PANEL_ITEM_X,		ATTR_COL(9),
		PANEL_ITEM_Y,		ATTR_ROW(7),
		PANEL_LABEL_IMAGE,	
		panel_button_image(panel, "reset", 0, 0),
		PANEL_LABEL_BOLD,	TRUE,
		PANEL_NOTIFY_PROC,	reset_proc,
		0);
	redraw_item = panel_create_item(panel, PANEL_BUTTON,
		PANEL_ITEM_X,		ATTR_COL(1),
		PANEL_ITEM_Y,		ATTR_ROW(9),
		PANEL_LABEL_IMAGE,	
		panel_button_image(panel, "redraw", 0, 0),
		PANEL_LABEL_BOLD,	TRUE,
		PANEL_NOTIFY_PROC,	redraw_proc,
		0);
	contour_item = panel_create_item(panel, PANEL_BUTTON,
		PANEL_ITEM_X,		ATTR_COL(1),
		PANEL_ITEM_Y,		ATTR_ROW(11),
		PANEL_LABEL_IMAGE,	
		panel_button_image(panel, "contour", 0, 0),
		PANEL_LABEL_BOLD,	TRUE,
		PANEL_NOTIFY_PROC,	contour_proc,
		0);
	contour_int = 25.0;
	sprintf(contour_str,"%8.1f",contour_int);
	contint_item = panel_create_item(panel, PANEL_TEXT,
		PANEL_ITEM_X,		ATTR_COL(0),
		PANEL_ITEM_Y,		ATTR_ROW(11),
		PANEL_LABEL_STRING,	"contour interval: ",
		PANEL_VALUE,		contour_str,
		PANEL_VALUE_DISPLAY_LENGTH,	20,
		PANEL_LABEL_BOLD,	TRUE,
		PANEL_NOTIFY_PROC,	contint_proc,
		PANEL_LAYOUT,		PANEL_VERTICAL,
		PANEL_SHOW_ITEM,	FALSE,
		0);
	color_item = panel_create_item(panel, PANEL_BUTTON,
		PANEL_ITEM_X,		ATTR_COL(1),
		PANEL_ITEM_Y,		ATTR_ROW(13),
		PANEL_LABEL_IMAGE,	
		panel_button_image(panel, "color", 0, 0),
		PANEL_LABEL_BOLD,	TRUE,
		PANEL_NOTIFY_PROC,	color_proc,
		0);
	color_int = 100.0;
	tick_int = color_int;
	sprintf(color_str,"%8.1f",color_int);
	colrint_item = panel_create_item(panel, PANEL_TEXT,
		PANEL_ITEM_X,		ATTR_COL(0),
		PANEL_ITEM_Y,		ATTR_ROW(13),
		PANEL_LABEL_STRING,	"color interval: ",
		PANEL_VALUE,		color_str,
		PANEL_VALUE_DISPLAY_LENGTH,	20,
		PANEL_LABEL_BOLD,	TRUE,
		PANEL_NOTIFY_PROC,	colrint_proc,
		PANEL_LAYOUT,		PANEL_VERTICAL,
		PANEL_SHOW_ITEM,	FALSE,
		0);
	depth_item = panel_create_item(panel, PANEL_BUTTON,
		PANEL_ITEM_X,		ATTR_COL(1),
		PANEL_ITEM_Y,		ATTR_ROW(15),
		PANEL_LABEL_IMAGE,	
		panel_button_image(panel, "hide depths", 0, 0),
		PANEL_LABEL_BOLD,	TRUE,
		PANEL_NOTIFY_PROC,	depth_proc,
		0);
	drag_item = panel_create_item(panel, PANEL_CHOICE,
		PANEL_ITEM_X,		ATTR_COL(1),
		PANEL_ITEM_Y,		ATTR_ROW(17),
		PANEL_LABEL_STRING,	"drag:",
		PANEL_LABEL_BOLD,	TRUE,
		PANEL_LAYOUT,		PANEL_VERTICAL,
		PANEL_NOTIFY_PROC,	drag_proc,
		PANEL_CHOICE_STRINGS,	"section 1","section 2",0,
		0);
	blowup_item = panel_create_item(panel, PANEL_BUTTON,
		PANEL_ITEM_X,		ATTR_COL(1),
		PANEL_ITEM_Y,		ATTR_ROW(21),
		PANEL_LABEL_IMAGE,	
		panel_button_image(panel, "blowup", 0, 0),
		PANEL_LABEL_BOLD,	TRUE,
		PANEL_NOTIFY_PROC,	blowup_proc,
		0);
	blowdown_item = panel_create_item(panel, PANEL_BUTTON,
		PANEL_ITEM_X,		ATTR_COL(1),
		PANEL_ITEM_Y,		ATTR_ROW(21),
		PANEL_LABEL_IMAGE,	
		panel_button_image(panel, "blowdown", 0, 0),
		PANEL_LABEL_BOLD,	TRUE,
		PANEL_NOTIFY_PROC,	blowdown_proc,
		PANEL_SHOW_ITEM,	FALSE,
		0);
	sextras_item = panel_create_item(panel, PANEL_BUTTON,
		PANEL_ITEM_X,		ATTR_COL(1),
		PANEL_ITEM_Y,		ATTR_ROW(23),
		PANEL_LABEL_IMAGE,	
		panel_button_image(panel, "show more", 0, 0),
		PANEL_LABEL_BOLD,	TRUE,
		PANEL_NOTIFY_PROC,	sextras_proc,
		0);
	hextras_item = panel_create_item(panel, PANEL_BUTTON,
		PANEL_ITEM_X,		ATTR_COL(1),
		PANEL_ITEM_Y,		ATTR_ROW(23),
		PANEL_LABEL_IMAGE,	
		panel_button_image(panel, "show less", 0, 0),
		PANEL_LABEL_BOLD,	TRUE,
		PANEL_NOTIFY_PROC,	hextras_proc,
		PANEL_SHOW_ITEM,	FALSE,
		0);
	time1_item = panel_create_item(panel, PANEL_SLIDER,
		PANEL_ITEM_X,		ATTR_COL(1),
		PANEL_ITEM_Y,		ATTR_ROW(24),
		PANEL_VALUE,		50,
		PANEL_MIN_VALUE,	0,
		PANEL_MAX_VALUE,	100,
		PANEL_WIDTH,		100,
		PANEL_LABEL_STRING,	"section 1 time:",
		PANEL_LABEL_BOLD,	TRUE,
		PANEL_SHOW_VALUE,	FALSE,
		PANEL_SHOW_RANGE,	FALSE,
		PANEL_LAYOUT,		PANEL_VERTICAL,
		PANEL_NOTIFY_LEVEL,	PANEL_ALL,
		PANEL_NOTIFY_PROC,	time1_proc,
		PANEL_SHOW_ITEM,	TRUE,
		0);
	time2_item = panel_create_item(panel, PANEL_SLIDER,
		PANEL_ITEM_X,		ATTR_COL(1),
		PANEL_ITEM_Y,		ATTR_ROW(26),
		PANEL_VALUE,		50,
		PANEL_MIN_VALUE,	0,
		PANEL_MAX_VALUE,	100,
		PANEL_WIDTH,		100,
		PANEL_LABEL_STRING,	"section 2 time:",
		PANEL_LABEL_BOLD,	TRUE,
		PANEL_SHOW_VALUE,	FALSE,
		PANEL_SHOW_RANGE,	FALSE,
		PANEL_LAYOUT,		PANEL_VERTICAL,
		PANEL_NOTIFY_LEVEL,	PANEL_ALL,
		PANEL_NOTIFY_PROC,	time2_proc,
		PANEL_SHOW_ITEM,	TRUE,
		0);
	correlation_item = panel_create_item(panel, PANEL_BUTTON,
		PANEL_ITEM_X,		ATTR_COL(1),
		PANEL_ITEM_Y,		ATTR_ROW(29),
		PANEL_LABEL_IMAGE,	
		panel_button_image(panel, "correlation", 0, 0),
		PANEL_LABEL_BOLD,	TRUE,
		PANEL_NOTIFY_PROC,	correlation_proc,
		0);
	uncertval = 0.030;
	uncertainty_item = panel_create_item(panel, PANEL_CHOICE,
		PANEL_ITEM_X,		ATTR_COL(1),
		PANEL_ITEM_Y,		ATTR_ROW(31),
		PANEL_LABEL_STRING,	"uncertainty:",
		PANEL_LABEL_BOLD,	TRUE,
		PANEL_LAYOUT,		PANEL_VERTICAL,
		PANEL_NOTIFY_PROC,	uncertainty_proc,
		PANEL_CHOICE_STRINGS,	" 30 m"," 50 m","100 m","250 m",0,
		0);
	window_fit(panel);
	window_fit(canvas);
	window_fit(frame);
	screen = canvas_pixwin(canvas);

	/* enable canvas mouse events */
	window_set(canvas, WIN_CONSUME_PICK_EVENTS,WIN_NO_EVENTS,
		WIN_MOUSE_BUTTONS,LOC_MOVE,0,0);

	/* setup colortable */
	red[0] = 255;
	red[1] = 0;
	red[2] = 255;
	red[3] =   0;
	red[4] =   0;
	red[5] =   0;
	red[6] = 127;
	red[7] =  63;
	red[8] =   0;
	red[9] =   0;
	red[10] =   0;
	red[11] =   0;
	red[12] =   0;
	red[13] =   0;
	red[14] =   0;
	red[15] = 127;
	red[16] = 191;
	red[17] = 255;
	red[18] = 255;
	red[19] = 255;
	red[20] = 255;
	red[21] = 255;
	red[31] = 255;
	green[0] = 255;
	green[1] = 0;
	green[2] =   0;
	green[3] = 200;
	green[4] =   0;
	green[5] =   0;
	green[6] = 0;
	green[7] = 0;
	green[8] = 0;
	green[9] = 63;
	green[10] = 127;
	green[11] = 191;
	green[12] = 255;
	green[13] = 255;
	green[14] = 255;
	green[15] = 255;
	green[16] = 255;
	green[17] = 255;
	green[18] = 191;
	green[19] = 127;
	green[20] = 63;
	green[21] = 0;
	green[31] = 0;
	blue[0] = 255;
	blue[1] = 0;
	blue[2] =   0;
	blue[3] =   0;
	blue[4] = 255;
	blue[5] =   0;
	blue[6] = 255;
	blue[7] = 255;
	blue[8] = 255;
	blue[9] = 255;
	blue[10] = 255;
	blue[11] = 255;
	blue[12] = 255;
	blue[13] = 191;
	blue[14] = 127;
	blue[15] = 0;
	blue[16] = 0;
	blue[17] = 0;
	blue[18] = 0;
	blue[19] = 0;
	blue[20] = 0;
	blue[20] = 0;
	blue[21] = 0;
	blue[31] = 0;
	pw_setcmsname(screen,"cms_naverr");
	pw_putcolormap(screen,0,RGBSIZE,red,green,blue);
	color[0] = 1;
	color[1] = 2;
	color[2] = 3;
	color[3] = 4;
	color[4] = 0;
	draw_op = PIX_SRC | PIX_COLOR(color[0]);
	clear_op = PIX_SRC |  PIX_COLOR(color[4]);
	transfer_op = PIX_SRC | PIX_DST;
	time_op = PIX_SRC |  PIX_COLOR(color[0]);

	/* make frame visible */
	window_set(frame, WIN_SHOW, TRUE, 0 );

	/* setup memory pixrects for storing plot images */
	store1 = mem_create(canvas_width,canvas_height,screen_depth);
	store2 = mem_create(canvas_width,canvas_height,screen_depth);

	/* initialize plotting */
	ixo = 0;
	iyo = canvas_height;
	ox = 0.0;
	oy = 0.0;
	ix = 0;
	iy = 0;
	ixoff1 = 0;
	iyoff1 = 0;
	ixoff2 = 0;
	iyoff2 = 0;
	xoff = 0.0;
	ixoff = 0;
	yoff = 0.0;
	iyoff = 0;
	nvector1 = 0;
	nvector2 = 0;
	nvector = &nvector1;

	/* initialize other stuff */
	drag = 0;
	blowup = 0;
	sector = 0;
	depthlab = 0;
	showcorr = 0;
	isec1 = -1;
	isec2 = -1;
	showextras = 0;
	(void) clear_screen();

	status = 0;
	if (screen == NULL) status = 1;
	return(status);
}

/*--------------------------------------------------------------------------*/
void clear_screen()
{
	pw_rop(screen,0,0,canvas_width,canvas_height,clear_op,NULL,0,0);
	(void) notify_dispatch();
	return;
}
/*--------------------------------------------------------------------------*/
void clear_store1()
{
	pr_rop(store1,0,0,canvas_width,canvas_height,clear_op,NULL,0,0);
	(void) notify_dispatch();
	return;
}
/*--------------------------------------------------------------------------*/
void clear_store2()
{
	pr_rop(store2,0,0,canvas_width,canvas_height,clear_op,NULL,0,0);
	(void) notify_dispatch();
	return;
}

/*--------------------------------------------------------------------------*/
void close_proc()
{
	window_set(frame, FRAME_CLOSED, TRUE, 0);
	return;
}

/*--------------------------------------------------------------------------*/
void quit_proc()
{
	close(ofp);
	close(rfp);
	window_destroy(frame);
	window_destroy(corr_frame);
	exit(0);
	return;
}

/*--------------------------------------------------------------------------*/
void next_proc()
{
	int	status;
	int	time1_i[6], time2_i[6];

	/* check that times have been reset */
	if (time1set == 0 || time2set == 0)
		{
		alert_proc("The times have not been reset yet!");
		return;
		}

	/* output last crossover */
	get_date(&ttime1[time1_val],time1_i);
	get_date(&ttime2[time2_val],time2_i);
	printf(ofp,"%5d %5d  %4d %2d %2d %2d %2d %2d  %4d %2d %2d %2d %2d %2d  %9.5f %9.5f  %8.3f %8.3f %8.3f\n",
		isec1,isec2,time1_i[0],time1_i[1],time1_i[2],time1_i[3],
		time1_i[4],time1_i[5],time2_i[0],time2_i[1],time2_i[2],
		time2_i[3],time2_i[4],time2_i[5],xoff,yoff,
		ZERO,uncertval,uncertval);
	fprintf(ofp,"%5d %5d  %4d %2d %2d %2d %2d %2d  %4d %2d %2d %2d %2d %2d  %9.5f %9.5f  %8.3f %8.3f %8.3f\n",
		isec1,isec2,time1_i[0],time1_i[1],time1_i[2],time1_i[3],
		time1_i[4],time1_i[5],time2_i[0],time2_i[1],time2_i[2],
		time2_i[3],time2_i[4],time2_i[5],xoff,yoff,
		ZERO,uncertval,uncertval);
	fprintf(rfp,"%d %d\n",isec1,isec2);

	/* reset blowup */
	panel_set(blowdown_item, PANEL_SHOW_ITEM, FALSE, 0);
	panel_set(blowup_item, PANEL_SHOW_ITEM, TRUE, 0);
	blowup = 0;

	/* reset extras */
	hextras_proc();

	/* get rid of correlation window */
	if (showcorr ==1)
		{
		showcorr = 0;
		window_destroy(corr_frame);
		(void) notify_dispatch();
		}

	/* initialize plotting */
	ixo = 0;
	iyo = canvas_height;
	ox = 0.0;
	oy = 0.0;
	ix = 0;
	iy = 0;

	/* reset offsets */
	ixoff = 0;
	xoff = 0.0;
	iyoff = 0;
	yoff = 0.0;
	ixoff1 = 0;
	iyoff1 = 0;
	ixoff2 = 0;
	iyoff2 = 0;

	/* reset timesets */
	time1set = 0;
	time2set = 0;

	/* read in the next crossover */
	if ((status = get_input()) != 0)
		{
		quit_proc();
		exit(-1);
		}

	/* set the frame title */
	sprintf(title_str,"NAVERR:   CROSSING: %d   SECTION 1: %d   SECTION 2: %d   XOFF: %8.4f %5d   YOFF: %8.4f %5d",
		ncross,isec1,isec2,xoff,ixoff,yoff,iyoff);
	window_set(frame,FRAME_LABEL,title_str,0);

	/* set the time pick bounds */
	time1_val = ntime1/2;
	time2_val = ntime2/2;
	panel_set(time1_item,PANEL_MAX_VALUE,ntime1-1,PANEL_VALUE,time1_val,0);
	panel_set(time2_item,PANEL_MAX_VALUE,ntime2-1,PANEL_VALUE,time2_val,0);

	/* scale the data */
	scale_data();

	/* contour the sections */
	if ((status = get_contours()) != 0)
		{
		printf("contouring failed\n");
		quit_proc();
		exit(-1);
		}
	ox = 0.0;
	oy = 0.0;
	if ((status = plot_contours()) != 0)
		{
		printf("contouring failed\n");
		quit_proc();
		exit(-1);
		}

	/* show time */
	show_time();

	return;
}
/*--------------------------------------------------------------------------*/
void save_proc()
{
	int	status;
	int	time1_i[6], time2_i[6];

	/* check that times have been reset */
	if (time1set == 0 || time2set == 0)
		{
		alert_proc("The times have not been reset yet!");
		return;
		}

	/* output current crossover */
	get_date(&ttime1[time1_val],time1_i);
	get_date(&ttime2[time2_val],time2_i);
	fprintf(ofp,"%5d %5d  %4d %2d %2d %2d %2d %2d  %4d %2d %2d %2d %2d %2d  %9.5f %9.5f  %8.3f %8.3f %8.3f\n",
		isec1,isec2,time1_i[0],time1_i[1],time1_i[2],time1_i[3],
		time1_i[4],time1_i[5],time2_i[0],time2_i[1],time2_i[2],
		time2_i[3],time2_i[4],time2_i[5],xoff,yoff,
		ZERO,uncertval,uncertval);
	fprintf(rfp,"%d %d\n",isec1,isec2);

	/* reset timesets */
	time1set = 0;
	time2set = 0;

	return;
}

/*--------------------------------------------------------------------------*/
void skip_proc()
{
	int	status;

	/* reset blowup */
	panel_set(blowdown_item, PANEL_SHOW_ITEM, FALSE, 0);
	panel_set(blowup_item, PANEL_SHOW_ITEM, TRUE, 0);
	blowup = 0;

	/* reset extras */
	hextras_proc();

	/* get rid of correlation window */
	if (showcorr ==1)
		{
		showcorr = 0;
		window_destroy(corr_frame);
		(void) notify_dispatch();
		}

	/* initialize plotting */
	ixo = 0;
	iyo = canvas_height;
	ox = 0.0;
	oy = 0.0;
	ix = 0;
	iy = 0;

	/* reset offsets */
	ixoff = 0;
	xoff = 0.0;
	iyoff = 0;
	yoff = 0.0;
	ixoff1 = 0;
	iyoff1 = 0;
	ixoff2 = 0;
	iyoff2 = 0;

	/* reset timesets */
	time1set = 0;
	time2set = 0;

	/* read in the next crossover */
	if ((status = get_input()) != 0)
		{
		quit_proc();
		exit(-1);
		}

	/* set the frame title */
	sprintf(title_str,"NAVERR:   CROSSING: %d   SECTION 1: %d   SECTION 2: %d   XOFF: %8.4f %5d   YOFF: %8.4f %5d",
		ncross,isec1,isec2,xoff,ixoff,yoff,iyoff);
	window_set(frame,FRAME_LABEL,title_str,0);

	/* set the time pick bounds */
	time1_val = ntime1/2;
	time2_val = ntime2/2;
	panel_set(time1_item,PANEL_MAX_VALUE,ntime1-1,PANEL_VALUE,time1_val,0);
	panel_set(time2_item,PANEL_MAX_VALUE,ntime2-1,PANEL_VALUE,time2_val,0);

	/* scale the data */
	scale_data();

	/* contour the sections */
	if ((status = get_contours()) != 0)
		{
		printf("contouring failed\n");
		quit_proc();
		exit(-1);
		}
	ox = 0.0;
	oy = 0.0;
	if ((status = plot_contours()) != 0)
		{
		printf("contouring failed\n");
		quit_proc();
		exit(-1);
		}

	/* show time */
	show_time();

	return;
}
/*--------------------------------------------------------------------------*/
void none_proc()
{
	int	status;

	/* output to report file */
	if (isec1 > -1) fprintf(rfp,"%d %d\n",isec1,isec2);

	/* reset blowup */
	panel_set(blowdown_item, PANEL_SHOW_ITEM, FALSE, 0);
	panel_set(blowup_item, PANEL_SHOW_ITEM, TRUE, 0);
	blowup = 0;

	/* reset extras */
	hextras_proc();

	/* get rid of correlation window */
	if (showcorr ==1)
		{
		showcorr = 0;
		window_destroy(corr_frame);
		(void) notify_dispatch();
		}

	/* initialize plotting */
	ixo = 0;
	iyo = canvas_height;
	ox = 0.0;
	oy = 0.0;
	ix = 0;
	iy = 0;

	/* reset offsets */
	ixoff = 0;
	xoff = 0.0;
	iyoff = 0;
	yoff = 0.0;
	ixoff1 = 0;
	iyoff1 = 0;
	ixoff2 = 0;
	iyoff2 = 0;

	/* reset timesets */
	time1set = 0;
	time2set = 0;

	/* read in the next crossover */
	if ((status = get_input()) != 0)
		{
		quit_proc();
		exit(-1);
		}

	/* set the frame title */
	sprintf(title_str,"NAVERR:   CROSSING: %d   SECTION 1: %d   SECTION 2: %d   XOFF: %8.4f %5d   YOFF: %8.4f %5d",
		ncross,isec1,isec2,xoff,ixoff,yoff,iyoff);
	window_set(frame,FRAME_LABEL,title_str,0);

	/* set the time pick bounds */
	time1_val = ntime1/2;
	time2_val = ntime2/2;
	panel_set(time1_item,PANEL_MAX_VALUE,ntime1-1,PANEL_VALUE,time1_val,0);
	panel_set(time2_item,PANEL_MAX_VALUE,ntime2-1,PANEL_VALUE,time2_val,0);

	/* scale the data */
	scale_data();

	/* contour the sections */
	if ((status = get_contours()) != 0)
		{
		printf("contouring failed\n");
		quit_proc();
		exit(-1);
		}
	ox = 0.0;
	oy = 0.0;
	if ((status = plot_contours()) != 0)
		{
		printf("contouring failed\n");
		quit_proc();
		exit(-1);
		}

	/* show time */
	show_time();

	return;
}


/*--------------------------------------------------------------------------*/
void redo_proc()
{
	int	status;

	/* reset blowup */
	panel_set(blowdown_item, PANEL_SHOW_ITEM, FALSE, 0);
	panel_set(blowup_item, PANEL_SHOW_ITEM, TRUE, 0);
	blowup = 0;

	/* reset bounds for plotting */
	xmin = xmins;
	xmax = xmaxs;
	ymin = ymins;
	ymax = ymaxs;

	/* initialize plotting */
	ixo = 0;
	iyo = canvas_height;
	ox = 0.0;
	oy = 0.0;
	ix = 0;
	iy = 0;

	/* reset offsets */
/*	ixoff = 0;
	xoff = 0.0;
	iyoff = 0;
	yoff = 0.0;
	ixoff1 = 0;
	iyoff1 = 0;
	ixoff2 = 0;
	iyoff2 = 0;*/

	/* reset timesets */
	time1set = 0;
	time2set = 0;

	/* contour the sections */
	if ((status = get_contours()) != 0)
		{
		printf("contouring failed\n");
		quit_proc();
		exit(-1);
		}

	/* rescale offsets */
	ox = 0.0;
	oy = 0.0;
	ixoff = (int) (xscale*xoff + 0.5);
	iyoff = (int) (-yscale*yoff - 0.5);
	if (drag)
		{
		ixoff1 = 0;
		iyoff1 = 0;
		ixoff2 = ixoff1 + ixoff;
		iyoff2 = iyoff1 + iyoff;
		}
	else
		{
		ixoff2 = 0;
		iyoff2 = 0;
		ixoff1 = ixoff2 - ixoff;
		iyoff1 = iyoff2 - iyoff;
		}
	if ((status = plot_contours()) != 0)
		{
		printf("contouring failed\n");
		quit_proc();
		exit(-1);
		}

	/* set the frame title */
	sprintf(title_str,"NAVERR:   CROSSING: %d   SECTION 1: %d   SECTION 2: %d   XOFF: %8.4f %5d   YOFF: %8.4f %5d",
		ncross,isec1,isec2,xoff,ixoff,yoff,iyoff);
		window_set(frame,FRAME_LABEL,title_str,0);

	/* show time */
	show_time();

	return;
}

/*--------------------------------------------------------------------------*/
void reset_proc()
{
	/* reset offsets */
	ixoff = 0;
	xoff = 0.0;
	iyoff = 0;
	yoff = 0.0;
	ixoff1 = 0;
	iyoff1 = 0;
	ixoff2 = 0;
	iyoff2 = 0;

	/* set the frame title */
	sprintf(title_str,"NAVERR:   CROSSING: %d   SECTION 1: %d   SECTION 2: %d   XOFF: %8.4f %5d   YOFF: %8.4f %5d",
		ncross,isec1,isec2,xoff,ixoff,yoff,iyoff);
	window_set(frame,FRAME_LABEL,title_str,0);

	/* show the offset contours */
	(void) show_contour();

	/* show time */
	show_time();

	return;
}
/*--------------------------------------------------------------------------*/
void redraw_proc()
{
	/* show the offset contours */
	(void) show_contour();

	/* show time */
	show_time();

	return;
}

/*--------------------------------------------------------------------------*/
void contour_proc()
{
	/* show contour interval panel */
	panel_set(contour_item, PANEL_SHOW_ITEM, FALSE, 0);
	panel_set(contint_item, PANEL_SHOW_ITEM, TRUE, 0);
	return;
}

/*--------------------------------------------------------------------------*/
Panel_setting contint_proc(item, event)
Panel_item item;
Event	*event;
{
	/* get color interval value */
	strcpy(contour_str, (char *)panel_get_value(contint_item));
	sscanf(contour_str,"%lf",&contour_int);

	/* hide contour interval panel */
	panel_set(contint_item, PANEL_SHOW_ITEM, FALSE, 0);
	panel_set(contour_item, PANEL_SHOW_ITEM, TRUE, 0);

	/* reset value */
	sprintf(contour_str,"%8.1f",contour_int);
	panel_set(contint_item, PANEL_VALUE, contour_str, 0);

	return(PANEL_NONE);
}

/*--------------------------------------------------------------------------*/
void color_proc()
{
	/* show color interval panel */
	panel_set(color_item, PANEL_SHOW_ITEM, FALSE, 0);
	panel_set(colrint_item, PANEL_SHOW_ITEM, TRUE, 0);
	return;
}

/*--------------------------------------------------------------------------*/
Panel_setting colrint_proc(item, event)
Panel_item item;
Event	*event;
{
	/* get color interval value */
	strcpy(color_str, (char *)panel_get_value(colrint_item));
	sscanf(color_str,"%lf",&color_int);
	tick_int = color_int;

	/* hide contour interval panel */
	panel_set(colrint_item, PANEL_SHOW_ITEM, FALSE, 0);
	panel_set(color_item, PANEL_SHOW_ITEM, TRUE, 0);

	/* reset value */
	sprintf(color_str,"%8.1f",color_int);
	panel_set(colrint_item, PANEL_VALUE, color_str, 0);

	return(PANEL_NONE);
}
/*--------------------------------------------------------------------------*/
void depth_proc(item, event)
Panel_item item;
Event	*event;
{
	/* reset depth panel item */
	if (depthlab == 0)
		{
		depthlab = 1;
		panel_set(depth_item, PANEL_LABEL_IMAGE, 
			panel_button_image(panel, "show depths", 0, 0), 0);
		}
	else
		{
		depthlab = 0;
		panel_set(depth_item, PANEL_LABEL_IMAGE, 
			panel_button_image(panel, "hide depths", 0, 0), 0);
		}
	return;
}
/*--------------------------------------------------------------------------*/
void drag_proc(item,choice,event)
Panel_item	item;
int	choice;
Event	*event;
{
	drag = choice;
	return;
}
/*--------------------------------------------------------------------------*/
void blowup_proc()
{
	panel_set(blowup_item, PANEL_SHOW_ITEM, FALSE, 0);
	panel_set(blowdown_item, PANEL_SHOW_ITEM, TRUE, 0);
	window_set(canvas, WIN_EVENT_PROC, zoom_proc,0,0);
	blowup = 0;
	return;
}
/*--------------------------------------------------------------------------*/
void blowdown_proc()
{
	/* blowdown if it has been blown up */
	if (blowup == 1)
		{

		/* reset bounds for plotting */
		xmin = xmins;
		xmax = xmaxs;
		ymin = ymins;
		ymax = ymaxs;

		/* initialize plotting */
		ixo = 0;
		iyo = canvas_height;
		ox = 0.0;
		oy = 0.0;
		ix = 0;
		iy = 0;

		/* contour the sections */
		if ((status = plot_contours()) != 0)
			{
			printf("contouring failed\n");
			quit_proc();
			exit(-1);
			}

		/* rescale offsets */
		ixoff = (int) (xscale*xoff + 0.5);
		iyoff = (int) (-yscale*yoff - 0.5);
		ixoff1 = 0;
		iyoff2 = 0;
		if (drag)
			{
			ixoff2 = ixoff1 + ixoff;
			iyoff2 = iyoff1 + iyoff;
			}
		else
			{
			ixoff1 = ixoff2 - ixoff;
			iyoff1 = iyoff2 - iyoff;
			}

		/* set the frame title */
		sprintf(title_str,"NAVERR:   CROSSING: %d   SECTION 1: %d   SECTION 2: %d   XOFF: %8.4f %5d   YOFF: %8.4f %5d",
			ncross,isec1,isec2,xoff,ixoff,yoff,iyoff);
		window_set(frame,FRAME_LABEL,title_str,0);

		/* show the offset contours */
		(void) show_contour();
	}

	/* reset buttons */
	panel_set(blowdown_item, PANEL_SHOW_ITEM, FALSE, 0);
	panel_set(blowup_item, PANEL_SHOW_ITEM, TRUE, 0);
	window_set(canvas, WIN_EVENT_PROC, track_proc,0,0);
	blowup = 0;

	return;
}
/*--------------------------------------------------------------------------*/
void sextras_proc()
{
	panel_set(sextras_item, PANEL_SHOW_ITEM, FALSE, 0);
	panel_set(hextras_item, PANEL_SHOW_ITEM, TRUE, 0);
	showextras = 1;
	return;
}
/*--------------------------------------------------------------------------*/
void hextras_proc()
{
	panel_set(hextras_item, PANEL_SHOW_ITEM, FALSE, 0);
	panel_set(sextras_item, PANEL_SHOW_ITEM, TRUE, 0);
	showextras = 0;
	return;
}
/*--------------------------------------------------------------------------*/
void time1_proc(item,choice,event)
Panel_item	item;
int	choice;
Event	*event;
{
	int	status;

	/* set and show new section 1 time pick */
	erase_time();
	time1_val = choice;
	show_time();
	time1set = 1;

	return;
}
/*--------------------------------------------------------------------------*/
void time2_proc(item,choice,event)
Panel_item	item;
int	choice;
Event	*event;
{
	int	status;

	/* set and show new section 1 time pick */
	erase_time();
	time2_val = choice;
	show_time();
	time2set = 1;

	return;
}
/*--------------------------------------------------------------------------*/
void alert_proc(message)
char *message;
{
	int	status;
	Frame	alert_window;

	/* show alert */
	status = alert_prompt(alert_window,(Event *) NULL,
			ALERT_MESSAGE_STRINGS, message, 0,
			ALERT_BUTTON_YES,	"OK",
			ALERT_POSITION,	ALERT_SCREEN_CENTERED,
			0);

	return;
}
/*--------------------------------------------------------------------------*/
void uncertainty_proc(item,choice,event)
Panel_item	item;
int	choice;
Event	*event;
{
	if (choice == 0)
		uncertval = 0.030;
	else if (choice == 1)
		uncertval = 0.050;
	else if (choice == 2)
		uncertval = 0.100;
	else if (choice == 3)
		uncertval = 0.250;
	return;
}
/*--------------------------------------------------------------------------*/
void correlation_proc()
{
	int	i, j, k, ii, jj, i2, j2, k1, k2;
	int	float_size, double_size;
	int	nx, ny, nxc, nyc, ndata1, ndata2, nrng, ncorr, icmin, jcmin;
	int	icolor, box_op;
	double	*c;
	double	corr, dcorr, corrmin, corrmax;
	float	xo, yo, dx, dy, x, y, d, cay;
	float	*d1, *d2, *z1, *z2;
	float	big = 99999.9;

	/* check that the size of the correlation area is ok */
	if ((xmax - xmin) > (ymax - ymin))
		spaceval = (xmax - xmin)/(MAXDIM - 1);
	else
		spaceval = (ymax - ymin)/(MAXDIM - 1);
	nx = (xmax - xmin)/spaceval + 1;
	ny = (ymax - ymin)/spaceval + 1;

	/* if a correlation frame already exists kill it */
	if (showcorr == 1)
		{
		showcorr = 0;
		window_destroy(corr_frame);
		(void) notify_dispatch();
		}

	/* Create the correlation frame and its canvas.  */
	nxc = 2*(nx/2) + 1;
	nyc = 2*(ny/2) + 1;
	corr_width = nxc*5;
	corr_height = nyc*5;
	corr_frame = window_create(NULL, FRAME,
		WIN_WIDTH,		corr_width+10,
		WIN_HEIGHT,		corr_height+63,
		WIN_X,			20,
		WIN_Y,			20,
		FRAME_ICON,		icon,
		FRAME_LABEL,		"WORKING...",
		WIN_ERROR_MSG,		"Fatal error:  Suntools not active!",
		FRAME_NO_CONFIRM,	TRUE,
		WIN_SHOW,		FALSE,
		0);
	corr_canvas = window_create(corr_frame, CANVAS,
		CANVAS_AUTO_EXPAND,	TRUE,
		CANVAS_AUTO_SHRINK,	TRUE,
		CANVAS_WIDTH, 		corr_width,
		CANVAS_HEIGHT, 		corr_height, 
		WIN_X,			0,
		WIN_Y,			40,
		WIN_EVENT_PROC,		corr_track_proc,
		0);
	corr_panel = window_create(corr_frame, PANEL,
		WIN_X,			0,
		WIN_Y,			0,
		0);
	best_item = panel_create_item(corr_panel, PANEL_BUTTON,
		PANEL_ITEM_X,		ATTR_COL(0),
		PANEL_ITEM_Y,		ATTR_ROW(0),
		PANEL_LABEL_IMAGE,	
		panel_button_image(panel, "use best", 0, 0),
		PANEL_LABEL_BOLD,	TRUE,
		PANEL_NOTIFY_PROC,	best_proc,
		0);
	window_fit(corr_canvas);
	window_fit(corr_panel);
	window_fit(corr_frame);
	corr_screen = canvas_pixwin(corr_canvas);
	pw_setcmsname(corr_screen,"cms_naverr");
	pw_putcolormap(corr_screen,0,RGBSIZE,red,green,blue);
	window_set(corr_canvas, WIN_CONSUME_PICK_EVENTS,WIN_NO_EVENTS,
		WIN_MOUSE_BUTTONS,LOC_MOVE,0,0);
	window_set(corr_frame, WIN_SHOW, TRUE, 0 );
	showcorr = 1;
	(void) notify_dispatch();

	/* count data for first interpolation */
	ndata1 = 0;
	if (sec1.prior == 1 && isec1-1 != isec2+1)
		for (i=0;i<swath1i.npings;i++)
			for (j=0;j<swath1i.beams;j++)
				{
				x = swath1i.data[i].lon[j];
				y = swath1i.data[i].lat[j];
				d = swath1i.data[i].dep[j];
				if (d > 0 && x >= xmin && x <= xmax && y >= ymin && y <= ymax) ndata1++;
				}
	for (i=0;i<swath1.npings;i++)
		for (j=0;j<swath1.beams;j++)
			{
			x = swath1.data[i].lon[j];
			y = swath1.data[i].lat[j];
			d = swath1.data[i].dep[j];
			if (d > 0 && x >= xmin && x <= xmax && y >= ymin && y <= ymax) ndata1++;
			}
	if (sec1.post == 1)
		for (i=0;i<swath1f.npings;i++)
			for (j=0;j<swath1f.beams;j++)
				{
				x = swath1f.data[i].lon[j];
				y = swath1f.data[i].lat[j];
				d = swath1f.data[i].dep[j];
				if (d > 0 && x >= xmin && x <= xmax && y >= ymin && y <= ymax) ndata1++;
				}

	/* count data for second interpolation */
	ndata2 = 0;
	if (sec2.prior == 1 && isec2-1 != isec1+1)
		for (i=0;i<swath2i.npings;i++)
			for (j=0;j<swath2i.beams;j++)
				{
				x = swath2i.data[i].lon[j];
				y = swath2i.data[i].lat[j];
				d = swath2i.data[i].dep[j];
				if (d > 0 && x >= xmin && x <= xmax && y >= ymin && y <= ymax) ndata2++;
				}
	for (i=0;i<swath2.npings;i++)
		for (j=0;j<swath2.beams;j++)
			{
			x = swath2.data[i].lon[j];
			y = swath2.data[i].lat[j];
			d = swath2.data[i].dep[j];
			if (d > 0 && x >= xmin && x <= xmax && y >= ymin && y <= ymax) ndata2++;
			}
	if (sec2.post == 1)
		for (i=0;i<swath2f.npings;i++)
			for (j=0;j<swath2f.beams;j++)
				{
				x = swath2f.data[i].lon[j];
				y = swath2f.data[i].lat[j];
				d = swath2f.data[i].dep[j];
				if (d > 0 && x >= xmin && x <= xmax && y >= ymin && y <= ymax) ndata2++;
				}

	/* allocate arrays for interpolations */
	float_size = sizeof(float);
	if ((d1 = (float *) calloc((3*ndata1),float_size)) == NULL)
		exit(-1);
	if ((d2 = (float *) calloc((3*ndata2),float_size)) == NULL)
		exit(-1);
	if ((z1 = (float *) calloc((nx*ny),float_size)) == NULL)
		exit(-1);
	if ((z2 = (float *) calloc((nx*ny),float_size)) == NULL)
		exit(-1);
	for (i=0;i<(nx*ny);i++)
		{
		z1[i] = 0.0;
		z2[i] = 0.0;
		}

	/* obtain data for first interpolation */
	ndata1 = 0;
	if (sec1.prior == 1 && isec1-1 != isec2+1)
		for (i=0;i<swath1i.npings;i++)
			for (j=0;j<swath1i.beams;j++)
				{
				x = swath1i.data[i].lon[j];
				y = swath1i.data[i].lat[j];
				d = swath1i.data[i].dep[j];
				if (d > 0 && x >= xmin && x <= xmax && y >= ymin && y <= ymax)
					{
					d1[ndata1++] = x;
					d1[ndata1++] = y;
					d1[ndata1++] = d;
					}
				}
	for (i=0;i<swath1.npings;i++)
		for (j=0;j<swath1.beams;j++)
			{
			x = swath1.data[i].lon[j];
			y = swath1.data[i].lat[j];
			d = swath1.data[i].dep[j];
			if (d > 0 && x >= xmin && x <= xmax && y >= ymin && y <= ymax)
					{
					d1[ndata1++] = x;
					d1[ndata1++] = y;
					d1[ndata1++] = d;
					}
			}
	if (sec1.post == 1)
		for (i=0;i<swath1f.npings;i++)
			for (j=0;j<swath1f.beams;j++)
				{
				x = swath1f.data[i].lon[j];
				y = swath1f.data[i].lat[j];
				d = swath1f.data[i].dep[j];
				if (d > 0 && x >= xmin && x <= xmax && y >= ymin && y <= ymax)
					{
					d1[ndata1++] = x;
					d1[ndata1++] = y;
					d1[ndata1++] = d;
					}
				}

	/* obtain data for second interpolation */
	ndata2 = 0;
	if (sec2.prior == 1 && isec2-1 != isec1+1)
		for (i=0;i<swath2i.npings;i++)
			for (j=0;j<swath2i.beams;j++)
				{
				x = swath2i.data[i].lon[j];
				y = swath2i.data[i].lat[j];
				d = swath2i.data[i].dep[j];
				if (d > 0 && x >= xmin && x <= xmax && y >= ymin && y <= ymax)
					{
					d2[ndata2++] = x;
					d2[ndata2++] = y;
					d2[ndata2++] = d;
					}
				}
	for (i=0;i<swath2.npings;i++)
		for (j=0;j<swath2.beams;j++)
			{
			x = swath2.data[i].lon[j];
			y = swath2.data[i].lat[j];
			d = swath2.data[i].dep[j];
			if (d > 0 && x >= xmin && x <= xmax && y >= ymin && y <= ymax)
					{
					d2[ndata2++] = x;
					d2[ndata2++] = y;
					d2[ndata2++] = d;
					}
			}
	if (sec2.post == 1)
		for (i=0;i<swath2f.npings;i++)
			for (j=0;j<swath2f.beams;j++)
				{
				x = swath2f.data[i].lon[j];
				y = swath2f.data[i].lat[j];
				d = swath2f.data[i].dep[j];
				if (d > 0 && x >= xmin && x <= xmax && y >= ymin && y <= ymax)
					{
					d2[ndata2++] = x;
					d2[ndata2++] = y;
					d2[ndata2++] = d;
					}
				}
	ndata1 = ndata1/3;
	ndata2 = ndata2/3;

	/* do interpolations */
	/* z(x,y) = z[k] where k = i + j*nx */
	/* and x = xo + i*dx and y = yo + j*dy */
	dx = spaceval;
	dy = spaceval;
	xo = xmin;
	yo = ymin;
	nrng = 1;
	cay = 1.0e10;
	zgrid_(z1,&nx,&ny,&xo,&yo,&dx,&dy,d1,&ndata1,&cay,&nrng);
	zgrid_(z2,&nx,&ny,&xo,&yo,&dx,&dy,d2,&ndata2,&cay,&nrng);

	/* get cross-correlation */
	double_size = sizeof(double);
	if ((c = (double *) calloc((nxc*nyc),double_size)) == NULL)
		exit(-1);
	for (i=0;i<nxc;i++)
		for (j=0;j<nyc;j++)
			{
			(void) notify_dispatch();
			c[i+j*nxc] = 0.0;
			ixoff = i - nxc/2;
			iyoff = j - nyc/2;
			corr = 0.0;
			ncorr = 0;
			for (ii=0;ii<nx;ii++)
				for (jj=0;jj<ny;jj++)
					{
					k1 = ii + jj*nx;
					i2 = ii + ixoff;
					j2 = jj + iyoff;
					k2 = i2 + j2*nx;
					if (i2 >= 0 && i2 < nx && j2 >= 0 
						&& j2 < ny && z1[k1] < big 
						&& z2[k2] < big)
						{
						dcorr = z2[k2] - z1[k1];
						corr = corr + dcorr*dcorr;
						ncorr++;
						}
					(void) notify_dispatch();
					}
			if (ncorr > 2) c[i+j*nxc] = corr/ncorr;
			}

	/* show cross correlation in color */
	sprintf(title_str,"CROSS CORRELATION");
	window_set(corr_frame,FRAME_LABEL,title_str,0);
	corrmax = 0.0;
	corrmin = big;
	for (i=0;i<nxc;i++)
		for (j=0;j<nyc;j++)
			{
			corr = c[i+j*nxc];
			if (corr > corrmax) corrmax = corr;
			if (corr > 0.0 && corr < corrmin) 
				{
				corrmin = corr;
				icmin = i;
				jcmin = j;
				}
			(void) notify_dispatch();
			}
	xoffmin = (icmin - (int)(nxc/2))*spaceval;
	yoffmin = (jcmin - (int)(nyc/2))*spaceval;
	corrmin = corrmax - (corrmax - corrmin)*1.001;
	dcorr = log10(corrmax - corrmin)/15.99;
	for (i=0;i<nxc;i++)
		for (j=0;j<nyc;j++)
			if (c[i+j*nxc] > 0.0)
				{
				icolor = log10(c[i+j*nxc] - corrmin)/dcorr + 6;
				ii = 5*i;
				jj = 5*(nyc-1) - 5*j;
				box_op = PIX_SRC | PIX_COLOR(icolor);
				pw_rop(corr_screen,ii,jj,5,5,box_op,NULL,0,0);
				(void) notify_dispatch();
				}
	spaceval_cur = spaceval;
	ox_corr = -spaceval_cur*((int)(nxc/2));
	oy_corr = spaceval_cur*((int)(nyc/2));

	/* free up the space allocated in this routine */
	free(d1);
	free(d2);
	free(z1);
	free(z2);
	free(c);

	return;
}
/*--------------------------------------------------------------------------*/
void corr_track_proc(event_canvas,event)
Canvas	event_canvas;
Event	*event;
{
	static int jx, jy;
	int e;

	/* cull out extraneous events */
	if (event_x(event) < 0 || event_x(event) > canvas_width
		|| event_y(event) < 0 || event_y(event) > canvas_height)
		{
		return;
		}

	/* look for mouse down events */
	e = event_id(event);
	if (event_is_down(event) && 
		(e == MS_LEFT || e == MS_MIDDLE || e == MS_RIGHT))
		{
		jx = event_x(event);
		jy = event_y(event);
		xoff = -(ox_corr + spaceval_cur*jx/5);
		yoff = -(oy_corr - spaceval_cur*jy/5);
		ixoff = xoff*xscale;
		iyoff = -yoff*yscale;
		xoff = ixoff/xscale;
		yoff = -iyoff/yscale;
		ixoff2 = 0;
		iyoff2 = 0;
		ixoff1 = ixoff2 - ixoff;
		iyoff1 = iyoff2 - iyoff;
		sprintf(title_str,"NAVERR:   CROSSING: %d   SECTION 1: %d   SECTION 2: %d   XOFF: %8.4f %5d   YOFF: %8.4f %5d",
			ncross,isec1,isec2,xoff,ixoff,yoff,iyoff);
		pw_batch_on(screen);
		window_set(frame,FRAME_LABEL,title_str,0);
		pw_batch_off(screen);
		(void) show_contour();
		(void) notify_dispatch();
		}

	return;
}
/*--------------------------------------------------------------------------*/
void best_proc()
{
		xoff = -xoffmin;
		yoff = -yoffmin;
		ixoff = xoff*xscale;
		iyoff = -yoff*yscale;
		xoff = ixoff/xscale;
		yoff = -iyoff/yscale;
		ixoff1 = 0;
		iyoff1 = 0;
		ixoff1 = ixoff2 - ixoff;
		iyoff1 = iyoff2 - iyoff;
		sprintf(title_str,"NAVERR:   CROSSING: %d   SECTION 1: %d   SECTION 2: %d   XOFF: %8.4f %5d   YOFF: %8.4f %5d",
			ncross,isec1,isec2,xoff,ixoff,yoff,iyoff);
		pw_batch_on(screen);
		window_set(frame,FRAME_LABEL,title_str,0);
		pw_batch_off(screen);
		(void) show_contour();
		(void) notify_dispatch();
	return;
}

/*--------------------------------------------------------------------------*/
void track_proc(event_canvas,event)
Canvas	event_canvas;
Event	*event;
{
	static int down = 0;
	static int jxstart, jystart;
	int e;

	/* cull out extraneous events */
	if (event_x(event) < 0 || event_x(event) > canvas_width
		|| event_y(event) < 0 || event_y(event) > canvas_height)
		{
		return;
		}

	/* switch over possible event types */
	e = event_id(event);
	switch(e)
		{

		/* turn dragging on */
		case MS_LEFT:
		if (event_is_down(event))
			{
			down = 1;
			jxstart = event_x(event);
			jystart = event_y(event);
			}
		break;

		/* turn dragging off */
		case MS_MIDDLE: case MS_RIGHT:
		if (event_is_down(event) && down == 1)
			{
			down = 0;
			if (drag == 0)
				{
				ixoff1 = ixoff1 + event_x(event) - jxstart;
				iyoff1 = iyoff1 + event_y(event) - jystart;
				}
			else
				{
				ixoff2 = ixoff2 + event_x(event) - jxstart;
				iyoff2 = iyoff2 + event_y(event) - jystart;
				}
			ixoff = ixoff2 - ixoff1;
			iyoff = iyoff2 - iyoff1;
			xoff = ixoff/xscale;
			yoff = -iyoff/yscale;
			sprintf(title_str,"NAVERR:   CROSSING: %d   SECTION 1: %d   SECTION 2: %d   XOFF: %8.4f %5d   YOFF: %8.4f %5d",
				ncross,isec1,isec2,xoff,ixoff,yoff,iyoff);
			window_set(frame,FRAME_LABEL,title_str,0);
			(void) show_contour();
			(void) notify_dispatch();
			}
		break;

		/* drag the contours */
		case LOC_MOVE:
		if (down == 1)
			{
			if (drag == 0)
				{
				ixoff1 = ixoff1 + event_x(event) - jxstart;
				iyoff1 = iyoff1 + event_y(event) - jystart;
				}
			else
				{
				ixoff2 = ixoff2 + event_x(event) - jxstart;
				iyoff2 = iyoff2 + event_y(event) - jystart;
				}
			jxstart = event_x(event);
			jystart = event_y(event);
			ixoff = ixoff2 - ixoff1;
			iyoff = iyoff2 - iyoff1;
			xoff = ixoff/xscale;
			yoff = -iyoff/yscale;
			sprintf(title_str,"NAVERR:   CROSSING: %d   SECTION 1: %d   SECTION 2: %d   XOFF: %8.4f %5d   YOFF: %8.4f %5d",
				ncross,isec1,isec2,xoff,ixoff,yoff,iyoff);
			window_set(frame,FRAME_LABEL,title_str,0);
			(void) show_contour();
			(void) notify_dispatch();
			}
		break;
		}

	return;
}
/*--------------------------------------------------------------------------*/
void zoom_proc(event_canvas,event)
Canvas	event_canvas;
Event	*event;
{
	static int down = 0;
	static int box = 0;
	static int jx0, jy0, jx1, jy1, kx0, ky0, kx1, ky1;
	int e;

	/* cull out extraneous events */
	if (event_x(event) < 0 || event_x(event) > canvas_width
		|| event_y(event) < 0 || event_y(event) > canvas_height)
		{
		return;
		}

	/* switch over possible event types */
	e = event_id(event);
	switch(e)
		{

		/* turn dragging on */
		case MS_LEFT:
		if (event_is_down(event))
			{
			down = 1;
			jx0 = event_x(event);
			jy0 = event_y(event);
			}
		break;

		/* turn dragging off and show box */
		case MS_MIDDLE:
		if (event_is_down(event) && down == 1)
			{
			down = 2;
			jx1 = event_x(event);
			jy1 = event_y(event);
			if (box == 1)
				{
				pw_vector(screen,kx0,ky0,kx0,ky1,clear_op,0);
				pw_vector(screen,kx0,ky1,kx1,ky1,clear_op,0);
				pw_vector(screen,kx1,ky1,kx1,ky0,clear_op,0);
				pw_vector(screen,kx1,ky0,kx0,ky0,clear_op,0);
				}
			pw_vector(screen,jx0,jy0,jx0,jy1,draw_op,0);
			pw_vector(screen,jx0,jy1,jx1,jy1,draw_op,0);
			pw_vector(screen,jx1,jy1,jx1,jy0,draw_op,0);
			pw_vector(screen,jx1,jy0,jx0,jy0,draw_op,0);
			kx0 = jx0;
			ky0 = jy0;
			kx1 = jx1;
			ky1 = jy1;
			box = 1;
			}
		break;

		/* execute zoom */
		case MS_RIGHT:
		if (event_is_down(event) && down == 2)
			{
			down = 0;
			box = 0;
			blowup = 1;
			xmins = xmin;
			xmaxs = xmax;
			ymins = ymin;
			ymaxs = ymax;
			xmin = (kx0 - ixo)/xscale - ox;
			xmax = (kx1 - ixo)/xscale - ox;
			ymin = (iyo - ky0)/yscale - oy;
			ymax = (iyo - ky1)/yscale - oy;
			if (xmax < xmin)
				{
				xmax = (kx0 - ixo)/xscale - ox;
				xmin = (kx1 - ixo)/xscale - ox;
				}
			if (ymax < ymin)
				{
				ymax = (iyo - ky0)/yscale - oy;
				ymin = (iyo - ky1)/yscale - oy;
				}

			/* initialize plotting */
			ixo = 0;
			iyo = canvas_height;
			ox = 0.0;
			oy = 0.0;
			ix = 0;
			iy = 0;

			/* contour the sections */
			if ((status = plot_contours()) != 0)
				{
				printf("contouring failed\n");
				quit_proc();
				exit(-1);
				}

			/* rescale offsets */
			ixoff = (int) (xscale*xoff + 0.5);
			iyoff = (int) (-yscale*yoff - 0.5);
			if (drag)
				{
				ixoff2 = ixoff1 + ixoff;
				iyoff2 = iyoff1 + iyoff;
				}
			else
				{
				ixoff1 = ixoff2 - ixoff;
				iyoff1 = iyoff2 - iyoff;
				}
			sprintf(title_str,"NAVERR:   CROSSING: %d   SECTION 1: %d   SECTION 2: %d   XOFF: %8.4f %5d   YOFF: %8.4f %5d",
				ncross,isec1,isec2,xoff,ixoff,yoff,iyoff);
			window_set(frame,FRAME_LABEL,title_str,0);

			/* show the offset contours */
			(void) show_contour();			

			/* reset what can be seen */
			panel_set(blowup_item, PANEL_SHOW_ITEM, FALSE, 0);
			panel_set(blowdown_item, PANEL_SHOW_ITEM, TRUE, 0);
			window_set(canvas, WIN_EVENT_PROC, track_proc,0,0);

			}
		break;
		}

	return;
}

/*--------------------------------------------------------------------------*/
void show_contour()
{
	/* start graphics call batch */
	pw_batch_on(screen);

	/* copy the two pixrects to the screen */
	clear_screen();
	pw_rop(screen,ixoff1,iyoff1,canvas_width,
		canvas_height,transfer_op,store1,0,0);
	pw_rop(screen,ixoff2,iyoff2,canvas_width,
		canvas_height,transfer_op,store2,0,0);

	/* show time */
	show_time();

	/* turn off the graphics batch, displaying the result */
	pw_batch_off(screen);
	return;
}
/*--------------------------------------------------------------------------*/
void show_time()
{
	int	ixc, iyc;
	int	del = 10;

	pw_batch_on(screen);
	ixc = ixoff1 + ixo + (int)((tlon1[time1_val] + ox)*xscale);
	iyc = iyoff1 + iyo - (int)((tlat1[time1_val] + oy)*yscale);
	pw_vector(screen,ixc-del,iyc-del,ixc+del,iyc+del,time_op,0);
	pw_vector(screen,ixc+del,iyc-del,ixc-del,iyc+del,time_op,0);
	pw_vector(screen,ixc-del,iyc-del-1,ixc+del,iyc+del-1,time_op,0);
	pw_vector(screen,ixc+del,iyc-del-1,ixc-del,iyc+del-1,time_op,0);
	pw_vector(screen,ixc-del,iyc-del+1,ixc+del,iyc+del+1,time_op,0);
	pw_vector(screen,ixc+del,iyc-del+1,ixc-del,iyc+del+1,time_op,0);
	ixc = ixoff2 + ixo + (int)((tlon2[time2_val] + ox)*xscale);
	iyc = iyoff2 + iyo - (int)((tlat2[time2_val] + oy)*yscale);
	pw_vector(screen,ixc-del,iyc-del,ixc+del,iyc+del,time_op,0);
	pw_vector(screen,ixc+del,iyc-del,ixc-del,iyc+del,time_op,0);
	pw_vector(screen,ixc-del,iyc-del-1,ixc+del,iyc+del-1,time_op,0);
	pw_vector(screen,ixc+del,iyc-del-1,ixc-del,iyc+del-1,time_op,0);
	pw_vector(screen,ixc-del,iyc-del+1,ixc+del,iyc+del+1,time_op,0);
	pw_vector(screen,ixc+del,iyc-del+1,ixc-del,iyc+del+1,time_op,0);
	pw_batch_off(screen);
	return;
}
/*--------------------------------------------------------------------------*/
void erase_time()
{
	int	ixc, iyc;
	int	del = 10;

	pw_batch_on(screen);
	ixc = ixoff1 + ixo + (int)((tlon1[time1_val] + ox)*xscale);
	iyc = iyoff1 + iyo - (int)((tlat1[time1_val] + oy)*yscale);
	pw_vector(screen,ixc-del,iyc-del,ixc+del,iyc+del,clear_op,0);
	pw_vector(screen,ixc+del,iyc-del,ixc-del,iyc+del,clear_op,0);
	pw_vector(screen,ixc-del,iyc-del-1,ixc+del,iyc+del-1,clear_op,0);
	pw_vector(screen,ixc+del,iyc-del-1,ixc-del,iyc+del-1,clear_op,0);
	pw_vector(screen,ixc-del,iyc-del+1,ixc+del,iyc+del+1,clear_op,0);
	pw_vector(screen,ixc+del,iyc-del+1,ixc-del,iyc+del+1,clear_op,0);
	ixc = ixoff2 + ixo + (int)((tlon2[time2_val] + ox)*xscale);
	iyc = iyoff2 + iyo - (int)((tlat2[time2_val] + oy)*yscale);
	pw_vector(screen,ixc-del,iyc-del,ixc+del,iyc+del,clear_op,0);
	pw_vector(screen,ixc+del,iyc-del,ixc-del,iyc+del,clear_op,0);
	pw_vector(screen,ixc-del,iyc-del-1,ixc+del,iyc+del-1,clear_op,0);
	pw_vector(screen,ixc+del,iyc-del-1,ixc-del,iyc+del-1,clear_op,0);
	pw_vector(screen,ixc-del,iyc-del+1,ixc+del,iyc+del+1,clear_op,0);
	pw_vector(screen,ixc+del,iyc-del+1,ixc-del,iyc+del+1,clear_op,0);
	pw_batch_off(screen);
	return;
}

/*--------------------------------------------------------------------------*/
/* 	function get_input reads in section ids and data to be handled.
 *	A status int is returned. */
int get_input()
{
	int	i, old, status;
	FILE	*fp;
	int	filemod = 493;

	old = 1;
	while (old == 1)
		{

		/* read next crossover */
		if ((status = scanf("%d %d",&isec1,&isec2)) != 2)
			{

			/* output navsolve command file */
			if ((fp = fopen(cmdfil,"w")) == NULL)
				{
				printf("unable to open cmd file:%s\n",cmdfil);
				exit(-1);
				}
			fprintf(fp,"# command file to set up navigation adjustment\n");
			fprintf(fp,"# inverse problem\n");
			fprintf(fp,"navsolve -I%s -N%s $1\n",datalist,navlist);
			fclose(fp);
			status = chmod(cmdfil,filemod);

			printf("all crossovers processed\n");
			return(status = 1);
			}
		ncross++;
		status = 0;

		/* check if crossover has already been processed */
		old = 0;
		for (i=0;i<nold;i++)
			{
			if (old1[i] == isec1 && old2[i] == isec2)
				old = 1;
			}
		}

	/* read appropriate info from section.list */
	if ((status = get_info()) != 0)
		{
		printf("read from section.list failed\n");
		quit_proc();
		exit(-1);
		}

	/* read section data into global arrays */
	if ((status = get_data()) != 0)
		{
		printf("read section data failed\n");
		quit_proc();
		exit(-1);
		}

	/* return */
	return(status);
}

/*--------------------------------------------------------------------------*/
/* 	function get_info reads info for current sections
 *	from the file section.list.  A status int is returned. */
int get_info()
{
	int	status;

	/* read in desired section values */
	if ((status = read_list(&sec1,isec1)) != 0)
		return(status=1);
	if ((status = read_list(&sec2,isec2)) != 0)
		return(status=1);

	/* read in prior and post sections */
	if (sec1.prior == 1)
		if ((status = read_list(&sec1i,(isec1-1))) != 0)
			return(status=1);
	if (sec1.post == 1)
		if ((status = read_list(&sec1f,(isec1+1))) != 0)
			return(status=1);
	if (sec2.prior == 1)
		if ((status = read_list(&sec2i,(isec2-1))) != 0)
			return(status=1);
	if (sec2.post == 1)
		if ((status = read_list(&sec2f,(isec2+1))) != 0)
			return(status=1);

	/* return successfully */
	return(status=0);
}

/*--------------------------------------------------------------------------*/
/* 	function read_list reads info for a section
 *	from the file section.list into the section newsec->  
 *	A status int is returned. */
int read_list(newsec,isec)
struct section *newsec;
int	isec;
{
	int	status, i, j;
	FILE	*fp;
	char	line[MAXLINE];

	/* open the file */
	if ((fp = fopen("section.list", "r")) == NULL) 
		{
		printf ("could not open file section.list\n");
		status = 1;
		return(status);
		}

	/* pass by comment lines */
	if (fgets(line,MAXLINE,fp) == NULL)
		{
		status = 1;
		return(status);
		}

	/* read until EOF or desired section is found */
	i = -1;
	status = 0;
	while (i != isec && status == 0)
		{

		/* read section values into temporary buffer */
		if (fgets(line,MAXLINE,fp) == NULL)
			return(status=1);

		if (fgets(line,MAXLINE,fp) == NULL)
			return(status=1);
		if ((status = sscanf(line,
			"global section: %d file: %d local section: %d",
			&i,&(newsec->fileid),&j)) != 3)
			return(status=1);

		if (fgets(line,MAXLINE,fp) == NULL)
			return(status=1);

		if (fgets(line,MAXLINE,fp) == NULL)
			return(status=1);
		if ((status = sscanf(line,"format: %d prior: %d post: %d ",
			&(newsec->format),&(newsec->prior),&(newsec->post)))
			!= 3)
			return(status=1);

		if (fgets(line,MAXLINE,fp) == NULL)
			return(status=1);
		if ((status = sscanf(line,"btime: %d %d %d %d:%d:%d",
			&(newsec->btime_i[0]),&(newsec->btime_i[1]),
			&(newsec->btime_i[2]),&(newsec->btime_i[3]),
			&(newsec->btime_i[4]),&(newsec->btime_i[5]))) != 6)
			return(status=1);

		if (fgets(line,MAXLINE,fp) == NULL)
			return(status=1);
		if ((status = sscanf(line,"etime: %d %d %d %d:%d:%d",
			&(newsec->etime_i[0]),&(newsec->etime_i[1]),
			&(newsec->etime_i[2]),&(newsec->etime_i[3]),
			&(newsec->etime_i[4]),&(newsec->etime_i[5]))) != 6)
			return(status=1);

		if (fgets(line,MAXLINE,fp) == NULL)
			return(status=1);
		if ((status = sscanf(line,"records: %d distance: %lf",
			&(newsec->nrec),&(newsec->distance))) != 2)
			return(status=1);
		status = 0;
		}

	fclose(fp);
	return(status);
}

/*--------------------------------------------------------------------------*/
/* 	function get_data reads in the multibeam data in the section????
 *	files.  A status int is returned. */
int get_data()
{
	int	status;

	/* reset time arrays */
	ntime1 = 0;
	ntime2 = 0;

	/*read in data */
	if (sec1.prior == 1 && isec1-1 != isec2+1)
		if ((status = read_data(isec1-1,&swath1i,&sec1i,ttime1,tlon1,tlat1,&ntime1)) != 0)
			{
			printf("unable to read data for section %d\n",isec1-1);
			quit_proc();
			exit(-1);
			}
	if ((status = read_data(isec1,&swath1,&sec1,ttime1,tlon1,tlat1,&ntime1)) != 0)
		{
		printf("unable to read data for section %d\n",isec1);
		quit_proc();
		exit(-1);
		}
	if (sec1.post == 1)
		if ((status = read_data(isec1+1,&swath1f,&sec1f,ttime1,tlon1,tlat1,&ntime1)) != 0)
			{
			printf("unable to read data for section %d\n",isec1+1);
			quit_proc();
			exit(-1);
			}
	if (sec2.prior == 1 && isec2-1 != isec1+1)
		if ((status = read_data(isec2-1,&swath2i,&sec2i,ttime2,tlon2,tlat2,&ntime2)) != 0)
			{
			printf("unable to read data for section %d\n",isec2-1);
			quit_proc();
			exit(-1);
			}
	if ((status = read_data(isec2,&swath2,&sec2,ttime2,tlon2,tlat2,&ntime2)) != 0)
		{
		printf("unable to read data for section %d\n",isec2);
		quit_proc();
		exit(-1);
		}
	if (sec2.post == 1)
		if ((status = read_data(isec2+1,&swath2f,&sec2f,ttime2,tlon2,tlat2,&ntime2)) != 0)
			{
			printf("unable to read data for section %d\n",isec2+1);
			quit_proc();
			exit(-1);
			}

	/* return */
	return(status);
}
/*--------------------------------------------------------------------------*/
/* 	function read_data reads in the multibeam data in the section????
 *	files.  A status int is returned. */
int read_data(isec,swathcur,seccur,time,clon, clat, ntime)
int	isec;
struct swath *swathcur;
struct section *seccur;
double	*time, *clon, *clat;
int	*ntime;
{
	int	status, i;
	char	file[128], suffix[5];
	int	beams, format, pings, lonflip;
	double	bounds[4];
	int	btime_i[6], etime_i[6];
	double	btime_d, etime_d;
	double	speedmin;
	double	timegap;

	int	rbeams, rpings;
	int	time_i[6];
	double	speed, heading, distance, pitch;
	struct	ping *pingcur;
	double	*depcur;
	double	*loncur;
	double	*latcur;
	int	ping_size, double_size;

	/* initialize read control variables */
	pings = 1;
	lonflip = 0;
	bounds[0] = -360.0;
	bounds[1] = 360.0;
	bounds[2] = -90.0;
	bounds[3] = 90.0;
	btime_i[0] = 1970;
	btime_i[1] = 1;
	btime_i[2] = 1;
	btime_i[3] = 0;
	btime_i[4] = 0;
	btime_i[5] = 0;
	etime_i[0] = 1999;
	etime_i[1] = 1;
	etime_i[2] = 1;
	etime_i[3] = 0;
	etime_i[4] = 0;
	etime_i[5] = 0;
	speedmin = 0.0;
	timegap = 100.0;

	/* construct filename */
	strcpy(file,"sections/section");
	sprintf(suffix,"%4.4d",isec);
	strcat(file,suffix);

	/* read in the data */
	swathcur->format = seccur->format;
	swathcur->beams = mb_format(&swathcur->format);
	double_size = sizeof(double);
	swathcur->npings = 0;

	if ((status = mb_read_init(file,&swathcur->format,&pings,
		&lonflip,bounds,btime_i,etime_i,&btime_d,&etime_d,
		&speedmin,&timegap)) != 0)
		{
		printf("naverr:  mbio read initialization error status: %d\n",
			status);
		quit_proc();
		exit(-1);
		}
	while (status <= 0)
		{

		/* allocate memory for current ping */
		pingcur = &swathcur->data[swathcur->npings];
		if ((pingcur->dep =
			(double *) calloc(swathcur->beams,double_size)) == NULL)
			exit(-1);
		if ((pingcur->lon =
			(double *) calloc(swathcur->beams,double_size)) == NULL)
			exit(-1);
		if ((pingcur->lat = 
			(double *) calloc(swathcur->beams,double_size)) == NULL)
			exit(-1);
		depcur = pingcur->dep;
		loncur = pingcur->lon;
		latcur = pingcur->lat;

		/* read in a ping */
		status = mb_read(&rbeams,&rpings,time_i,&time[*ntime],
			&clon[*ntime],&clat[*ntime],&speed,&heading,
			&distance,&pitch,depcur,loncur,latcur);
		if (status == 0) 
			{
			swathcur->npings++;
			*ntime = *ntime + 1;
			if (swathcur->npings >= MAXPINGS)
				{
				printf("\nmaximum number of pings exceeded: %d\n",
					swathcur->npings);
				exit(-1);
				}
			if (*ntime >= 3*MAXPINGS)
				{
				printf("\nmaximum number of ping times exceeded: %d\n",*ntime);
				exit(-1);
				}
			}
		else
			{
			free(pingcur->dep);
			free(pingcur->lon);
			free(pingcur->lat);
			}
		}

	/* return */
	if (swathcur->npings > 2) status = 0;
	return(status);
}


/*--------------------------------------------------------------------------*/
/* 	function get_contours calls the functions to scale the data and
 *	generate contours.  A status int is returned. */
int get_contours()
{
	int	status;
	int	ncolor = 4;
	int	nlevel = 0;
	double	xcen, ycen, label_int;

	/* clear the screen and old vectors */
	(void) clear_screen();
	(void) clear_store1();
	(void) clear_store2();

	/* set the depth label interval */
	if (depthlab == 0)
		label_int = 0.0;
	else
		label_int = color_int;

	/* set up plotting scale */
	xscale = canvas_width/(xmax - xmin);
	yscale = canvas_height/(ymax - ymin);
	if (xscale >= yscale)
		{
		xscale = yscale;
		xcen = 0.5*(xmin + xmax);
		xmin = xcen - 0.5*canvas_width/xscale;
		xmax = xcen + 0.5*canvas_width/xscale;
		}
	if (xscale < yscale)
		{
		yscale = xscale;
		ycen = 0.5*(ymin + ymax);
		ymin = ycen - 0.5*canvas_height/yscale;
		ymax = ycen + 0.5*canvas_height/yscale;
		}
	xmins = xmin;
	xmaxs = xmax;
	ymins = ymin;
	ymaxs = ymax;
	tick_len = 0.002*canvas_width/xscale;
	label_hgt = 0.007*canvas_width/xscale;
	(void) plot((-xmin),(-ymin),IOR);

	/* generate contours for section 1 */
	nvector = &nvector1;
	vector = &vector1[0];
	*nvector = 0;
	if (showextras && sec1.prior == 1 && isec1-1 != isec2+1) 
		(void) mb_contour(&swath1i,contour_int,color_int,
			tick_int,label_int,tick_len,label_hgt,
			ncolor,nlevel,NULL,NULL);
	(void) mb_contour(&swath1,contour_int,color_int,
		tick_int,label_int,tick_len,label_hgt,
		ncolor,nlevel,NULL,NULL);
	if (showextras && sec1.post == 1) 
		(void) mb_contour(&swath1f,contour_int,color_int,
			tick_int,label_int,tick_len,label_hgt,
			ncolor,nlevel,NULL,NULL);

	/* generate contours for section 2 */
	nvector = &nvector2;
	vector = &vector2[0];
	*nvector = 0;
	if (showextras && sec2.prior == 1 && isec2-1 != isec1+1) 
		(void) mb_contour(&swath2i,contour_int,color_int,
			tick_int,label_int,tick_len,label_hgt,
			ncolor,nlevel,NULL,NULL);
	(void) mb_contour(&swath2,contour_int,color_int,
		tick_int,label_int,tick_len,label_hgt,	
		ncolor,nlevel,NULL,NULL);
	if (showextras && sec2.post == 1) 
		(void) mb_contour(&swath2f,contour_int,color_int,
			tick_int,label_int,tick_len,label_hgt,
			ncolor,nlevel,NULL,NULL);

	/* return successfully */
	return(status=0);
}
/*--------------------------------------------------------------------------*/
/* 	function plot_contours plots previously created contours. */
int plot_contours()
{
	int	status;
	double	xcen, ycen;

	/* set up plotting scale */
	xscale = canvas_width/(xmax - xmin);
	yscale = canvas_height/(ymax - ymin);
	if (xscale >= yscale)
		{
		xscale = yscale;
		xcen = 0.5*(xmin + xmax);
		xmin = xcen - 0.5*canvas_width/xscale;
		xmax = xcen + 0.5*canvas_width/xscale;
		}
	if (xscale < yscale)
		{
		yscale = xscale;
		ycen = 0.5*(ymin + ymax);
		ymin = ycen - 0.5*canvas_height/yscale;
		ymax = ycen + 0.5*canvas_height/yscale;
		}
	(void) plot((-xmin),(-ymin),IOR);

	/* clear the storage pixrects */
	(void) clear_store1();
	(void) clear_store2();

	/* plot contours in storage pixrects */
	(void) plot_vectors(store1,nvector1,vector1);
	(void) plot_vectors(store2,nvector2,vector2);

	/* show the offset contours */
	(void) show_contour();

	/* return successfully */
	return(status=0);
}

/*--------------------------------------------------------------------------*/
/* 	function scale_data scales the lon lat depth data to x y depth
 *	with x and y in km. */
int scale_data()
{
	int	i, j, ifirst;
	double	lonmin, lonmax, latmin, latmax;
	double	clon, clat, deglontokm, deglattokm;

	/* get bounds on the data */
	(void) get_bounds(1,&lonmin,&lonmax,&latmin,&latmax,&swath1);
	(void) get_bounds(0,&lonmin,&lonmax,&latmin,&latmax,&swath2);
	if (showextras && sec1.prior == 1 && isec1-1 != isec2+1)
		(void) get_bounds(0,&lonmin,&lonmax,&latmin,&latmax,&swath1i);
	if (showextras && sec1.post == 1)
		(void) get_bounds(0,&lonmin,&lonmax,&latmin,&latmax,&swath1f);
	if (showextras && sec2.prior == 1 && isec2-1 != isec1+1)
		(void) get_bounds(0,&lonmin,&lonmax,&latmin,&latmax,&swath2i);
	if (showextras && sec2.post == 1)
		(void) get_bounds(0,&lonmin,&lonmax,&latmin,&latmax,&swath2f);

	/* get scaling */
	clon = 0.5*(lonmin + lonmax);
	clat = 0.5*(latmin + latmax);
	(void) km_scale(clat,&deglontokm,&deglattokm);
	xmin = BLOWUP*deglontokm*(lonmin - clon);
	xmax = BLOWUP*deglontokm*(lonmax - clon);
	ymin = BLOWUP*deglattokm*(latmin - clat);
	ymax = BLOWUP*deglattokm*(latmax - clat);

	/* do scaling */
	(void) do_scale(&swath1,deglontokm,deglattokm,clon,clat);
	(void) do_scale(&swath2,deglontokm,deglattokm,clon,clat);
	if (sec1.prior == 1 && isec1-1 != isec2+1)
		(void) do_scale(&swath1i,deglontokm,deglattokm,clon,clat);
	if (sec1.post == 1)
		(void) do_scale(&swath1f,deglontokm,deglattokm,clon,clat);
	if (sec2.prior == 1 && isec2-1 != isec1+1)
		(void) do_scale(&swath2i,deglontokm,deglattokm,clon,clat);
	if (sec2.post == 1)
		(void) do_scale(&swath2f,deglontokm,deglattokm,clon,clat);
	(void) scale_track(tlon1,tlat1,ntime1,deglontokm,deglattokm,clon,clat);
	(void) scale_track(tlon2,tlat2,ntime2,deglontokm,deglattokm,clon,clat);

	/* return successfully */
	return(status=0);
}

/*--------------------------------------------------------------------------*/
/* 	function get_bounds finds the lon and lat bounds of the data
 *	in the swath swathcur. */
void get_bounds(first,lonmin,lonmax,latmin,latmax,swathcur)
int	first;
double	*lonmin, *lonmax, *latmin, *latmax;
struct swath *swathcur;
{
	int	i, j, k;

	k = first;
	for (i=0;i<swathcur->npings;i++)
		for (j=0;j<swathcur->beams;j++)
			if (swathcur->data[i].dep[j] > 0.0)
				{
				if (k == 1)
					{
					*lonmin = swathcur->data[i].lon[j];
					*lonmax = swathcur->data[i].lon[j];
					*latmin = swathcur->data[i].lat[j];
					*latmax = swathcur->data[i].lat[j];
					k = 0;
					}
				if (swathcur->data[i].lon[j] < *lonmin)
					*lonmin = swathcur->data[i].lon[j];
				if (swathcur->data[i].lon[j] > *lonmax)
					*lonmax = swathcur->data[i].lon[j];
				if (swathcur->data[i].lat[j] < *latmin)
					*latmin = swathcur->data[i].lat[j];
				if (swathcur->data[i].lat[j] > *latmax)
					*latmax = swathcur->data[i].lat[j];
				}
	return;
}

/*--------------------------------------------------------------------------*/
/* 	function do_scale scales the lon lat depth data to x y depth
 *	with x and y in km. */
void do_scale(swathcur,deglontokm,deglattokm,clon,clat)
struct swath *swathcur;
double	deglontokm, deglattokm, clon, clat;
{
	int	i, j;

	/* do scaling */
	for (i=0;i<swathcur->npings;i++)
		for (j=0;j<swathcur->beams;j++)
			if (swathcur->data[i].dep[j] > 0.0)
				{
				swathcur->data[i].lon[j] = deglontokm*
					(swathcur->data[i].lon[j] - clon);
				swathcur->data[i].lat[j] = deglattokm*
					(swathcur->data[i].lat[j] - clat);
				}
	return;
}
/*--------------------------------------------------------------------------*/
/* 	function scale_track scales the lon lat track data to x y
 *	with x and y in km. */
void scale_track(tlon,tlat,ntrack,deglontokm,deglattokm,clon,clat)
double	*tlon, *tlat;
int	ntrack;
double	deglontokm, deglattokm, clon, clat;
{
	int	i;

	/* do scaling */
	for (i=0;i<ntrack;i++)
		{
		tlon[i] = deglontokm*(tlon[i] - clon);
		tlat[i] = deglattokm*(tlat[i] - clat);
		}

	return;
}
/*--------------------------------------------------------------------------*/
/* 	function km_scale returns scaling factors to turn lat and lon 
 *	differences in distance in km. 
 *	- formula based on world geodetic system ellipsoid of 1972.
 *	-  see bowditch (h.o. 9 -- american practical navigator) */
void km_scale(lat,deglontokm,deglattokm)
double lat,*deglontokm,*deglattokm;
#define C1 111412.84
#define C2 -93.5
#define C3 0.118
#define C4 111132.92
#define C5 -559.82
#define C6 1.175
#define C7 0.0023
{
	double avlat;
	avlat = DTR*lat;
	*deglontokm = 0.001*fabs(C1*cos(avlat) + C2*cos(3*avlat) 
			+ C3*cos(5*avlat));
	*deglattokm = 0.001*fabs(C4 + C5*cos(2*avlat) 
			+ C6*cos(4*avlat) + C7*cos(6*avlat));
	return;
}

/*--------------------------------------------------------------------------*/
void plot(xx,yy,ipen)
double xx,yy;
int ipen;
{
	double x,y;
	int ixx,iyy;
	int ivector;

	/* check if vector array is full */
	if (*nvector >= MAXVECTOR)
		{
		printf("\nmaximum number of vectors exceeded: %d\n",*nvector);
		exit(-1);
		}

	x = xx + ox;
	y = yy + oy;
/*	printf("%f %f %d  depthlab:%d\n",xx,yy,ipen,depthlab);*/

	/* move pen */
	if (ipen == IUP)
		{
		/* save move in vector array */
		ivector = *nvector*3;
		vector[ivector] = xx;
		vector[ivector+1] = yy;
		vector[ivector+2] = ipen;
		(*nvector)++;

		/* move on screen */
		ix = ixo + (int)(x*xscale);
		iy = iyo - (int)(y*yscale);
		(void) notify_dispatch();
		}

	/* plot */
	else if (ipen == IDN)
		{	
		/* save move in vector array */
		ivector = *nvector*3;
		vector[ivector] = xx;
		vector[ivector+1] = yy;
		vector[ivector+2] = ipen;
		(*nvector)++;

		/* plot on screen */
		ixx = ixo + (int)(x*xscale);
		iyy = iyo - (int)(y*yscale);
		pw_vector(screen,ix,iy,ixx,iyy,draw_op,1);
		ix = ixx;
		iy = iyy;
		(void) notify_dispatch();
		}

	/* change origin */
	else if (ipen << 0)
		{
		ox = x;
		oy = y;
		}

	return;
}

/*
 **********************************************************************
 *
 *	plot_(x,y,ipen)
 *
 **********************************************************************
 */
void
plot_(x,y,ipen)
float *x,*y;
int *ipen;
{
	plot((double)*x,(double)*y,*ipen);
}

/*--------------------------------------------------------------------------*/
void newpen(icolor)
int icolor;
{
	int	ivector;

	/* check if vector array is full */
	if (*nvector >= MAXVECTOR)
		{
		printf("\nmaximum number of vectors exceeded: %d\n",*nvector);
		exit(-1);
		}

	/* change pen color */
	draw_op = PIX_SRC | PIX_COLOR(color[icolor]);

	/* save pen change in vector array */
	ivector = *nvector*3;
	vector[ivector] = color[icolor];
	vector[ivector+1] = color[icolor];
	vector[ivector+2] = ICL;
	(*nvector)++;
	return;
}


/*--------------------------------------------------------------------------*/
void plot_vectors(pr,nvec,vec)
Pixrect *pr;
int	nvec;
double	*vec;
{
	int	ixx, iyy, ipen;
	int	i,j;
	double	x, y;

	/* read and plot from the vector file */
	for (i=0;i<nvec;i++)
		{
		j = 3*i;
		x = ox + vec[j];
		y = oy + vec[j+1];
		ipen = vec[j+2];

		/* move pen */
		if (ipen == IUP)
			{
			/* move on screen */
			ix = ixo + (int)(x*xscale);
			iy = iyo - (int)(y*yscale);
			(void) notify_dispatch();
			}

		/* plot */
		else if (ipen == IDN)
			{	
			/* plot on screen */
			ixx = ixo + (int)(x*xscale);
			iyy = iyo - (int)(y*yscale);
			pr_vector(pr,ix,iy,ixx,iyy,draw_op,1);
			ix = ixx;
			iy = iyy;
			(void) notify_dispatch();
			}

		/* change pen color */
		else if (ipen == ICL)
			{	
			/* change pen color */
			draw_op = PIX_SRC | PIX_COLOR((int)(vec[j] + 0.5));
			(void) notify_dispatch();
			}
		}

	return;
}

/*--------------------------------------------------------------------------*/
/* 	function plot_pings plots the data locations. */
void plot_pings(swathcur)
struct swath *swathcur;
{
	int	i, j, k;

	for (i=0;i<swathcur->npings;i++)
		{
		k = 1;
		newpen(i%4);
		for (j=0;j<swathcur->beams;j++)
			if (swathcur->data[i].dep[j] > 0.0)
				{
				if (k == 1)
					{
					(void) plot(swathcur->data[i].lon[j],
						swathcur->data[i].lat[j],IUP);
					k = 0;
					}
				else
					(void) plot(swathcur->data[i].lon[j],
						swathcur->data[i].lat[j],IDN);
				}
		}
	return;
}
