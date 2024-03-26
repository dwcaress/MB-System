#include <iostream>
#include <QFileDialog>
#include <QDebug>
#include <QDir>
#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include <getopt.h>
#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>
#include <unistd.h>
#include <QTextStream>

extern "C" {
#include "mb_define.h"
#include "mb_format.h"
#include "mb_io.h"
#include "mb_status.h"
#include "mb_process.h"
#include "mb_swap.h"
  // #include "mb_xgraphics.h"  // X-Windows dependencies
  // #include "mbedit.h"   // Motif dependencies
}

/* output mode defines */
#define MBEDIT_OUTPUT_EDIT 1
#define MBEDIT_OUTPUT_BROWSE 2

#define XG_SOLIDLINE 0
#define XG_DASHLINE 1

/* plot modes */
#define MBEDIT_PLOT_WIDE 0
#define MBEDIT_PLOT_TIME 1
#define MBEDIT_PLOT_INTERVAL 2
#define MBEDIT_PLOT_LON 3
#define MBEDIT_PLOT_LAT 4
#define MBEDIT_PLOT_HEADING 5
#define MBEDIT_PLOT_SPEED 6
#define MBEDIT_PLOT_DEPTH 7
#define MBEDIT_PLOT_ALTITUDE 8
#define MBEDIT_PLOT_SENSORDEPTH 9
#define MBEDIT_PLOT_ROLL 10
#define MBEDIT_PLOT_PITCH 11
#define MBEDIT_PLOT_HEAVE 12

/* view modes */
#define MBEDIT_VIEW_WATERFALL 0
#define MBEDIT_VIEW_ALONGTRACK 1
#define MBEDIT_VIEW_ACROSSTRACK 2
#define MBEDIT_SHOW_FLAG 0
#define MBEDIT_SHOW_DETECT 1
#define MBEDIT_SHOW_PULSE 2

using namespace mb_system;

const char MainWindow::help_message_[] =
  "This is an interactive editor used to identify and flag\n"
  "artifacts in swath sonar bathymetry data. Once a file has\n"
  "been read in, MBedit displays the bathymetry profiles from\n"
  "several pings, allowing the user to identify and flag\n"
  "anomalous beams. Flagging is handled internally by setting\n"
  "depth values negative, so that no information is lost.";
  
const char MainWindow::usage_message_[] =
  "mbedit [-Byr/mo/da/hr/mn/sc -D  -Eyr/mo/da/hr/mn/sc \n\t-Fformat "
  "-Ifile -Ooutfile -S -X -V -H]";

MainWindow::MainWindow(QWidget *parent)
  : QMainWindow(parent), ui(new Ui::MainWindow)
{
  verbose_ = 1;
  lonflip_ = 0;
  output_mode_ = MBEDIT_OUTPUT_EDIT;
  gui_mode_ = false;
  uselockfiles_ = true;
  run_mbprocess_ = false;
  dummy_ = nullptr;
  
  error_ = MB_ERROR_NO_ERROR;
  
  ui->setupUi(this);
  canvas_ = new QPixmap(ui->swathCanvas->width(),
			ui->swathCanvas->height())
    ;
  painter_ = new QPainter(canvas_);

  qDebug() << "CTR: swathcanvas width: " << ui->swathCanvas->width() <<
    ", swathcanvas height: " << ui->swathCanvas->height();    
}

MainWindow::~MainWindow()
{
  // Free unneeded memory
  delete ui;
}



void MainWindow::on_xtrackWidthSlider_sliderMoved(int position)
{
  std::cerr << "xtrackWidth = " << position << "\n";
}


void MainWindow::on_nPingsShowSlider_sliderMoved(int position)
{
  std::cerr << "nPingsShown = " << position << "\n";
}


void MainWindow::on_vertExaggSlider_sliderMoved(int position)
{
  std::cerr << "vertExagg = " << position << "\n";
}


void MainWindow::on_actionOpen_swath_file_triggered()
{
  std::cerr << "select swath file\n";
  QString fileName = QFileDialog::getOpenFileName(this,
						  tr("Open swath file"),
						  QDir::homePath(),
						  tr("swath files (*.m*)"));

  qDebug() << "open swath file " << fileName;
  std::string utf8_text = fileName.toUtf8().constData();
  std::cerr << "utf8_text: " << utf8_text << "\n";
  char *fname = (char *)utf8_text.c_str();
  std::cerr << "fname: " << fname << "\n";
  if (!processSwathfile(fname)) {
    std::cerr << "Error processing " << fname << "\n";
  }
  
}


bool MainWindow::processSwathfile(char *fname) {
    
  if (!grid_.readDatafile(fname)) {
    std::cerr << "Error reading file " << fname << "\n";
    return false;
  }

  // Draw swath data to pixmap
  if (!plot(&grid_)) {
    std::cerr << "Error plotting data from " << fname << "\n";
    return false;
  }
  
  // Add pixmap to UI label
  qDebug() << "Draw on GUI\n";
  ui->swathCanvas->setPixmap(*canvas_);
  return true;
}



bool MainWindow::plot(SwathGridData *grid) {
  qDebug() << "plot(): canvas width: " << canvas_->width() <<
    ", canvas height: " << canvas_->height();

  qDebug() << "plot(): swathcanvas width: " << ui->swathCanvas->width() <<
    ", swathcanvas height: " << ui->swathCanvas->height();  

  painter_->eraseRect(0, 0, canvas_->width(), canvas_->height());

  //// TEST TEST TEST
  xg_fillrectangle(dummy_, 0, 0, canvas_->width(), canvas_->height(),
		   WHITE, XG_SOLIDLINE);

  xg_fillrectangle(dummy_, 100, 100,
		   canvas_->width()-200, canvas_->height()-200,
		   RED, XG_SOLIDLINE);  

  xg_drawline(dummy_, 0, 0, canvas_->width(), canvas_->height(),
	      BLACK, XG_SOLIDLINE);

  xg_drawstring(dummy_, 100, 100, "hello sailor!", BLACK, XG_SOLIDLINE);
  
  return true;
}


int MainWindow::mbedit_init(int argc, char **argv,
			    bool *inputSwathfileSpecd) {

  char *program_name = argv[0];
  
  int status =
    mb_defaults(verbose_, &format_, &pings_, &lonflip_, bounds_,
		btime_i_, etime_i_, &speedmin_, &timegap_);

  status = mb_uselockfiles(verbose_, &uselockfiles_);
  format_ = 0;
  pings_ = 1;
  lonflip_ = 0;
  bounds_[0] = -360.;
  bounds_[1] = 360.;
  bounds_[2] = -90.;
  bounds_[3] = 90.;
  btime_i_[0] = 1962;
  btime_i_[1] = 2;
  btime_i_[2] = 21;
  btime_i_[3] = 10;
  btime_i_[4] = 30;
  btime_i_[5] = 0;
  btime_i_[6] = 0;
  etime_i_[0] = 2062;
  etime_i_[1] = 2;
  etime_i_[2] = 21;
  etime_i_[3] = 10;
  etime_i_[4] = 30;
  etime_i_[5] = 0;
  etime_i_[6] = 0;
  speedmin_ = 0.0;
  timegap_ = 1000000000.0;
  strcpy(inputFilename_, "");

  int fileflag = 0;

  int errflg = 0;
  int c;
  int help = 0;

  /* process argument list */
  while ((c = getopt(argc, argv, "VvHhB:b:DdE:e:F:f:GgI:i:SsXx")) != -1) {
    switch (c) {
    case 'H':
    case 'h':
      help++;
      break;
    case 'V':
    case 'v':
      verbose_++;
      break;
    case 'B':
    case 'b':
      sscanf(optarg, "%d/%d/%d/%d/%d/%d",
	     &btime_i_[0], &btime_i_[1], &btime_i_[2],
	     &btime_i_[3], &btime_i_[4], &btime_i_[5]);
      btime_i_[6] = 0;
      break;
    case 'D':
    case 'd':
      output_mode_ = MBEDIT_OUTPUT_BROWSE;
      break;
    case 'E':
    case 'e':
      sscanf(optarg, "%d/%d/%d/%d/%d/%d",
	     &etime_i_[0], &etime_i_[1], &etime_i_[2],
	     &etime_i_[3], &etime_i_[4], &etime_i_[5]);
      etime_i_[6] = 0;
      break;
    case 'F':
    case 'f':
      sscanf(optarg, "%d", &format_);
      break;
    case 'G':
    case 'g':
      gui_mode_ = true;
      break;
    case 'I':
    case 'i':
      sscanf(optarg, "%s", inputFilename_);
      // parseDatalist(inputFilename_, format_);
      fileflag++;
      break;
    case 'X':
    case 'x':
      run_mbprocess_ = true;
      break;
    case '?':
      errflg++;
    }
  }

  /* if error flagged then print it and exit */
  if (errflg) {
    fprintf(stderr, "usage: %s %s\n", program_name, usage_message_);
    fprintf(stderr, "\nProgram <%s> Terminated\n", program_name);
    error_ = MB_ERROR_BAD_USAGE;
    exit(error_);
  }

  /* print starting message */
  if (verbose_ == 1 || help) {
    fprintf(stderr, "\nProgram %s\n", program_name);
    fprintf(stderr, "MB-system Version %s\n", MB_VERSION);
  }

  /* print starting debug statements */
  if (verbose_ >= 2) {
    fprintf(stderr, "\ndbg2  Program <%s>\n", program_name);
    fprintf(stderr, "dbg2  MB-system Version %s\n", MB_VERSION);
    fprintf(stderr, "dbg2  Control Parameters:\n");
    fprintf(stderr, "dbg2       verbose:         %d\n", verbose_);
    fprintf(stderr, "dbg2       help:            %d\n", help);
    fprintf(stderr, "dbg2       format:          %d\n", format_);
    fprintf(stderr, "dbg2       input file:      %s\n",
	    inputFilename_);
    fprintf(stderr, "dbg2       output mode:     %d\n",
	    output_mode_);
  }

  if (help) {
    fprintf(stderr, "\n%s\n", help_message_);
    fprintf(stderr, "\nusage: %s\n", usage_message_);
    exit(error_);
  }

  if (verbose_ >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       argc:      %d\n", argc);
    for (int i = 0; i < argc; i++)
      fprintf(stderr, "dbg2       argv[%d]:    %s\n", i, argv[i]);
  }

  /* if file specified then use it */
  *inputSwathfileSpecd = fileflag > 0;

  if (verbose_ >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2  inputFileSpecd: %d\n", *inputSwathfileSpecd);
    fprintf(stderr, "dbg2       error:        %d\n", error_);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:  %d\n", status);
  }

  // If input swath file specified on command line, process it
  if (strlen(inputFilename_) > 0) {
    processSwathfile(inputFilename_);
  }

  
  return (status);
}



int MainWindow::mbedit_plot(int plotWidth, int vExagg,
			    int xntrvl, int yntrvl,
			    int plotSize, int showMode, 
			    int showTime,
			    bool autoscale) {
  if (verbose_ >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       plot_width:  %d\n", plotWidth);
    fprintf(stderr, "dbg2       exager:      %d\n", vExagg);
    fprintf(stderr, "dbg2       x_interval:  %d\n", xntrvl);
    fprintf(stderr, "dbg2       y_interval:  %d\n", yntrvl);
    fprintf(stderr, "dbg2       plot_size:   %d\n", plotSize);
    fprintf(stderr, "dbg2       show_mode:   %d\n", showMode);
    fprintf(stderr, "dbg2       show_time:   %d\n", showTime);
    fprintf(stderr, "dbg2       nplt:        %p\n", nplt);
    fprintf(stderr, "dbg2       autoscale:   %d\n", autoscale);
  }

  int nplot;
  
  // figure out which pings to plot
  if (current_id + plotSize > nbuff)
    nplot = nbuff - current_id;
  else
    nplot = plotSize;

  // get data into ping arrays and find median depth value 
  // double bathsum = 0.0;
  int nbathsum = 0;
  int nbathlist = 0;
  double xtrack_max = 0.0;
  for (int i = current_id; i < current_id + nplot; i++) {
    ping[i].record = i + ndump_total;
    ping[i].outbounds = MBEDIT_OUTBOUNDS_NONE;
    for (int j = 0; j < ping[i].beams_bath; j++) {
      if (mb_beam_ok(ping[i].beamflag[j])) {
	// bathsum += ping[i].bath[j];
	nbathsum++;
	bathlist[nbathlist] = ping[i].bath[j];
	nbathlist++;
	xtrack_max = MAX(xtrack_max, fabs(ping[i].bathacrosstrack[j]));
      }
    }
  }

  // if not enough information in unflagged bathymetry look
     into the flagged bathymetry 
  if (nbathlist <= 0 || xtrack_max <= 0.0) {
    for (int i = current_id; i < current_id + nplot; i++) {
      for (int j = 0; j < ping[i].beams_bath; j++) {
	if (!mb_beam_ok(ping[i].beamflag[j]) && !mb_beam_check_flag_unusable2(ping[i].beamflag[j])) {
	  // bathsum += ping[i].bath[j];
	  nbathsum++;
	  bathlist[nbathlist] = ping[i].bath[j];
	  nbathlist++;
	  xtrack_max = MAX(xtrack_max, fabs(ping[i].bathacrosstrack[j]));
	}
      }
    }
  }
  double bathmedian = 0.0;  // -Wmaybe-uninitialized
  if (nbathlist > 0) {
    qsort(bathlist, nbathlist, sizeof(double), mb_double_compare);
    bathmedian = bathlist[nbathlist / 2];
  }

  // reset xtrack_max if required 
  if (autoscale && xtrack_max < 0.5) {
    xtrack_max = 1000.0;
  }
  else if (autoscale && xtrack_max > 100000.0) {
    xtrack_max = 100000.0;
  }

  // if autoscale on reset plot width 
  if (autoscale && xtrack_max > 0.0) {
    plotWidth = (int)(2.4 * xtrack_max);
    const int ndec = MAX(1, (int)log10((double)plotWidth));
    int maxx = 1;
    for (int i = 0; i < ndec; i++)
      maxx = maxx * 10;
    maxx = (plotWidth / maxx + 1) * maxx;

    xntrvl = plotWidth / 10;
    if (xntrvl > 1000) {
      xntrvl = 1000 * (xntrvl / 1000);
    }
    else if (xntrvl > 500) {
      xntrvl = 500 * (xntrvl / 500);
    }
    else if (xntrvl > 250) {
      xntrvl = 250 * (xntrvl / 250);
    }
    else if (xntrvl > 100) {
      xntrvl = 100 * (xntrvl / 100);
    }
    else if (xntrvl > 50) {
      xntrvl = 50 * (xntrvl / 50);
    }
    else if (xntrvl > 25) {
      xntrvl = 25 * (xntrvl / 25);
    }
    else if (xntrvl > 10) {
      xntrvl = 10 * (xntrvl / 10);
    }
    else if (xntrvl > 5) {
      xntrvl = 5 * (xntrvl / 5);
    }
    else if (xntrvl > 2) {
      xntrvl = 2 * (xntrvl / 2);
    }
    else {
      xntrvl = 1;
    }
    yntrvl = xntrvl;
    do_reset_scale_x(plotWidth, maxx, xntrvl, yntrvl);
  }

  // print out information 
  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2       %d data records set for plotting (%d desired)\n", nplot, plotSize);
    fprintf(stderr, "dbg2       xtrack_max:  %f\n", xtrack_max);
    fprintf(stderr, "dbg2       bathmedian:  %f\n", bathmedian);
    fprintf(stderr, "dbg2       nbathlist:   %d\n", nbathlist);
    fprintf(stderr, "dbg2       nbathsum:    %d\n", nbathsum);
    for (int i = current_id; i < current_id + nplot; i++) {
      fprintf(stderr,
	      "dbg2  %4d %4d %4d  %d/%d/%d %2.2d:%2.2d:%2.2d.%6.6d  %10.3f\n",
	      i, ping[i].id, ping[i].record,
	      ping[i].time_i[1], ping[i].time_i[2], ping[i].time_i[0],
	      ping[i].time_i[3], ping[i].time_i[4],
	      ping[i].time_i[5], ping[i].time_i[6],
	      ping[i].bath[ping[i].beams_bath / 2]);
    }
  }

  // clear screen 
  xg_fillrectangle(dummy_, borders[0], borders[2],
		   borders[1] - borders[0], borders[3] - borders[2],
		   pixel_values[WHITE],
		   XG_SOLIDLINE);

  // set scaling 
  x_interval = xntrvl;
  y_interval = yntrvl;
  const int xcen = xmin + (xmax - xmin) / 2;
  const int ycen = ymin + (ymax - ymin) / 2;
  // const double dx = ((double)(xmax - xmin)) / plotSize;
  const double dy = ((double)(ymax - ymin)) / plotSize;
  xscale = 100.0 * plotWidth / (xmax - xmin);
  yscale = (xscale * 100.0) / vExagg;
  const double dxscale = 100.0 / xscale;
  const double dyscale = 100.0 / yscale;

  if (info_set) {
    mbedit_plot_info();
  }

  char string[MB_PATH_MAXLINE];
  int swidth;
  int sascent;
  int sdescent;
  int sxstart;

  if (showMode == MBEDIT_SHOW_FLAG) {
    sprintf(string,
	    "Sounding Colors by Flagging:  Unflagged  Manual  Filter  Sonar");
    
    xg_justify(dummy_, string, &swidth, &sascent, &sdescent);
    sxstart = xcen - swidth / 2;

    sprintf(string, "Sounding Colors by Flagging:  Unflagged  ");
    xg_justify(dummy_, string, &swidth, &sascent, &sdescent);
    xg_drawstring(dummy_, sxstart, ymin - margin / 2 + sascent + 5,
		  string, pixel_values[BLACK], XG_SOLIDLINE);

    sxstart += swidth;
    sprintf(string, "Manual  ");
    xg_justify(dummy_, string, &swidth, &sascent, &sdescent);
    xg_drawstring(dummy_, sxstart, ymin - margin / 2 + sascent + 5,
		  string, pixel_values[RED], XG_SOLIDLINE);

    sxstart += swidth;
    sprintf(string, "Filter  ");
    xg_justify(dummy_, string, &swidth, &sascent, &sdescent);
    xg_drawstring(dummy_, sxstart, ymin - margin / 2 + sascent + 5,
		  string, pixel_values[BLUE], XG_SOLIDLINE);

    sxstart += swidth;
    sprintf(string, "Sonar");
    xg_justify(dummy_, string, &swidth, &sascent, &sdescent);
    xg_drawstring(dummy_, sxstart, ymin - margin / 2 + sascent + 5,
		  string, pixel_values[GREEN], XG_SOLIDLINE);
  }
  else if (showMode == MBEDIT_SHOW_DETECT) {
    sprintf(string,
	    "Sounding Colors by Bottom Detection:  Amplitude  Phase  Unknown");
    xg_justify(dummy_, string, &swidth, &sascent, &sdescent);
    sxstart = xcen - swidth / 2;

    sprintf(string, "Sounding Colors by Bottom Detection:  Amplitude  ");
    xg_justify(dummy_, string, &swidth, &sascent, &sdescent);
    xg_drawstring(dummy_, sxstart, ymin - margin / 2 + sascent + 5,
		  string, pixel_values[BLACK], XG_SOLIDLINE);

    sxstart += swidth;
    sprintf(string, "Phase  ");
    xg_justify(dummy_, string, &swidth, &sascent, &sdescent);
    xg_drawstring(dummy_, sxstart, ymin - margin / 2 + sascent + 5,
		  string, pixel_values[RED], XG_SOLIDLINE);

    sxstart += swidth;
    sprintf(string, "Unknown");
    xg_justify(dummy_, string, &swidth, &sascent, &sdescent);
    xg_drawstring(dummy_, sxstart, ymin - margin / 2 + sascent + 5,
		  string, pixel_values[GREEN], XG_SOLIDLINE);
  }
  else if (showMode == MBEDIT_SHOW_PULSE) {
    sprintf(string,
	    "Sounding Colors by Source Type: CW Up-Chirp Down-Chirp  Unknown");
    xg_justify(dummy_, string, &swidth, &sascent, &sdescent);
    sxstart = xcen - swidth / 2;

    sprintf(string, "Sounding Colors by Source Type:  CW  ");
    xg_justify(dummy_, string, &swidth, &sascent, &sdescent);
    xg_drawstring(dummy_, sxstart, ymin - margin / 2 + sascent + 5,
		  string, pixel_values[BLACK], XG_SOLIDLINE);

    sxstart += swidth;
    sprintf(string, "Up-Chirp  ");
    xg_justify(dummy_, string, &swidth, &sascent, &sdescent);
    xg_drawstring(dummy_, sxstart, ymin - margin / 2 + sascent + 5,
		  string, pixel_values[RED], XG_SOLIDLINE);

    sxstart += swidth;
    sprintf(string, "Down-Chirp  ");
    xg_justify(dummy_, string, &swidth, &sascent, &sdescent);
    xg_drawstring(dummy_, sxstart, ymin - margin / 2 + sascent + 5,
		  string, pixel_values[BLUE], XG_SOLIDLINE);

    sxstart += swidth;
    sprintf(string, "Unknown");
    xg_justify(dummy_, string, &swidth, &sascent, &sdescent);
    xg_drawstring(dummy_, sxstart, ymin - margin / 2 + sascent + 5,
		  string, pixel_values[GREEN], XG_SOLIDLINE);
  }

  sprintf(string,
	  "Vertical Exageration: %4.2f   All Distances and Depths in Meters",
	  (vExagg / 100.));
  xg_justify(dummy_, string, &swidth, &sascent, &sdescent);
  xg_drawstring(dummy_, xcen - swidth / 2,
		ymin - margin / 2 + 2 * (sascent + sdescent) + 5,
		string, pixel_values[BLACK],
		XG_SOLIDLINE);

  // plot filename 
  sprintf(string, "File %d of %d:", file_id + 1, num_files);
  xg_justify(dummy_, string, &swidth, &sascent, &sdescent);
  xg_drawstring(dummy_, margin / 2, ymin - 3 * margin / 4, string,
		pixel_values[BLACK], XG_SOLIDLINE);
  char *string_ptr = strrchr(ifile, '/');
  if (string_ptr == NULL)
    string_ptr = ifile;
  else if (strlen(string_ptr) > 0)
    string_ptr++;
  xg_drawstring(dummy_, margin / 2 + 2 + swidth,
		ymin - margin / 2 - 1 * (sascent + sdescent) - 5, string_ptr,
		pixel_values[BLACK], XG_SOLIDLINE);

  // plot file position bar 
  int fpx = margin / 2 + ((4 * margin) * current_id) / nbuff;
  const int fpdx = MAX((((4 * margin) * nplot) / nbuff), 5);
  const int fpy = ymin - 5 * margin / 8;
  const int fpdy = margin / 4;
  if (fpx + fpdx > 9 * margin / 2)
    fpx = 9 * margin / 2 - fpdx;
  xg_drawrectangle(dummy_, margin / 2, ymin - 5 * margin / 8,
		   4 * margin, margin / 4, pixel_values[BLACK], XG_SOLIDLINE);
  xg_drawrectangle(dummy_, margin / 2 - 1, ymin - 5 * margin / 8 - 1,
		   4 * margin + 2, margin / 4 + 2, pixel_values[BLACK],
		   XG_SOLIDLINE);
  xg_fillrectangle(dummy_, fpx, fpy, fpdx, fpdy, pixel_values[LIGHTGREY],
		   XG_SOLIDLINE);
  xg_drawrectangle(dummy_, fpx, fpy, fpdx, fpdy, pixel_values[BLACK],
		   XG_SOLIDLINE);
  sprintf(string, "0 ");
  xg_justify(dummy_, string, &swidth, &sascent, &sdescent);
  xg_drawstring(dummy_, margin / 2 - swidth,
		ymin - margin / 2 + sascent / 2, string,
		pixel_values[BLACK], XG_SOLIDLINE);
  sprintf(string, " %d", nbuff);
  xg_drawstring(dummy_, 9 * margin / 2, ymin - margin / 2 + sascent / 2,
		string, pixel_values[BLACK], XG_SOLIDLINE);

  // plot scale bars 
  const double dx_width = (xmax - xmin) / dxscale;
  const int nx_int = (int)(0.5 * dx_width / x_interval + 1);
  const int x_int = (int)(x_interval * dxscale);
  xg_drawline(dummy_, xmin, ymax, xmax, ymax, pixel_values[BLACK],
	      XG_SOLIDLINE);
  xg_drawline(dummy_, xmin, ymin, xmax, ymin, pixel_values[BLACK],
	      XG_SOLIDLINE);
  for (int i = 0; i < nx_int; i++) {
    const int xx = i * x_int;
    const int vx = i * x_interval;
    xg_drawline(dummy_, xcen - xx, ymin, xcen - xx, ymax,
		pixel_values[BLACK], XG_DASHLINE);
    xg_drawline(dummy_, xcen + xx, ymin, xcen + xx, ymax,
		pixel_values[BLACK], XG_DASHLINE);
    sprintf(string, "%1d", vx);
    xg_justify(dummy_, string, &swidth, &sascent, &sdescent);
    xg_drawstring(dummy_, xcen + xx - swidth / 2, ymax + sascent + 5,
		  string, pixel_values[BLACK], XG_SOLIDLINE);
    xg_drawstring(dummy_, xcen - xx - swidth / 2, ymax + sascent + 5,
		  string, pixel_values[BLACK], XG_SOLIDLINE);
  }
  const double dy_height = (ymax - ymin) / dyscale;
  const int ny_int = (int)(dy_height / y_interval + 1);
  const int y_int = (int)(y_interval * dyscale);
  xg_drawline(dummy_, xmin, ymin, xmin, ymax, pixel_values[BLACK],
	      XG_SOLIDLINE);
  xg_drawline(dummy_, xmax, ymin, xmax, ymax, pixel_values[BLACK],
	      XG_SOLIDLINE);
  for (int i = 0; i < ny_int; i++) {
    const int yy = i * y_int;
    const int vy = i * y_interval;
    xg_drawline(dummy_, xmin, ymax - yy, xmax, ymax - yy,
		pixel_values[BLACK], XG_DASHLINE);
    sprintf(string, "%1d", vy);
    xg_justify(dummy_, string, &swidth, &sascent, &sdescent);
    xg_drawstring(dummy_, xmax + 5, ymax - yy + sascent / 2, string,
		  pixel_values[BLACK], XG_SOLIDLINE);
  }

  // int x0;
  // int y0;
  double tsmin;
  double tsmax;
  double tsvalue;
  double tsslope;

  // plot time series if desired 
  if (showTime > MBEDIT_PLOT_TIME) {
    // get scaling 
    mbedit_tsminmax(current_id, nplot, showTime, &tsmin, &tsmax);
    const double tsscale = 2.0 * margin / (tsmax - tsmin);

    // draw time series plot box 
    xg_drawline(dummy_, margin / 2, ymin, margin / 2, ymax,
		pixel_values[BLACK], XG_SOLIDLINE);
    xg_drawline(dummy_, margin, ymin, margin, ymax, pixel_values[BLACK],
		XG_DASHLINE);
    xg_drawline(dummy_, 3 * margin / 2, ymin, 3 * margin / 2,
		ymax, pixel_values[BLACK], XG_DASHLINE);
    xg_drawline(dummy_, 2 * margin, ymin, 2 * margin, ymax,
		pixel_values[BLACK], XG_DASHLINE);
    xg_drawline(dummy_, 5 * margin / 2, ymin, 5 * margin / 2, ymax,
		pixel_values[BLACK], XG_SOLIDLINE);
    xg_drawline(dummy_, margin / 2, ymax, 5 * margin / 2, ymax,
		pixel_values[BLACK], XG_SOLIDLINE);
    xg_drawline(dummy_, margin / 2, ymin, 5 * margin / 2, ymin,
		pixel_values[BLACK], XG_SOLIDLINE);

    // draw time series labels 
    //sprintf(string,"Heading (deg)");
    mbedit_tslabel(showTime, string);
    xg_justify(dummy_, string, &swidth, &sascent, &sdescent);
    xg_drawstring(dummy_, 3 * margin / 2 - swidth / 2, ymin - sdescent,
		  string, pixel_values[BLACK], XG_SOLIDLINE);
    sprintf(string, "%g", tsmin);
    xg_justify(dummy_, string, &swidth, &sascent, &sdescent);
    xg_drawstring(dummy_, margin / 2 - swidth / 2, ymax + sascent + 5,
		  string, pixel_values[BLACK], XG_SOLIDLINE);
    sprintf(string, "%g", tsmax);
    xg_justify(dummy_, string, &swidth, &sascent, &sdescent);
    xg_drawstring(dummy_, 5 * margin / 2 - swidth / 2, ymax + sascent + 5,
		  string, pixel_values[BLACK], XG_SOLIDLINE);

    //x0 = margin/2 + ping[current_id].heading / 360.0 * 2 * margin;
    mbedit_tsvalue(current_id, showTime, &tsvalue);
    int x0 = margin / 2 + (int)((tsvalue - tsmin) * tsscale);
    int y0 = ymax - (int)(dy / 2);
    for (int i = current_id; i < current_id + nplot; i++) {
      //x = margin/2 + ping[i].heading / 360.0 * 2 * margin;
      mbedit_tsvalue(i, showTime, &tsvalue);
      const int x = margin / 2 + (int)((tsvalue - tsmin) * tsscale);
      const int y = ymax - (int)(dy / 2) - (int)((i - current_id) * dy);
      xg_drawline(dummy_, x0, y0, x, y, pixel_values[BLACK], XG_SOLIDLINE);
      xg_fillrectangle(dummy_, x - 2, y - 2, 4, 4, pixel_values[BLACK],
		       XG_SOLIDLINE);
      x0 = x;
      y0 = y;
    }

    // if plotting roll, also plot acrosstrack slope 
    if (showTime == MBEDIT_PLOT_ROLL) {
      mbedit_xtrackslope(current_id, &tsslope);
      x0 = margin / 2 + (int)((tsslope - tsmin) * tsscale);
      y0 = ymax - (int)(dy / 2);
      for (int i = current_id; i < current_id + nplot; i++) {
	//x = margin/2 + ping[i].heading / 360.0 * 2 * margin;
	mbedit_xtrackslope(i, &tsslope);
	const int x = margin / 2 + (int)((tsslope - tsmin) * tsscale);
	const int y = ymax - (int)(dy / 2) - (int)((i - current_id) * dy);
	xg_drawline(dummy_, x0, y0, x, y, pixel_values[RED], XG_SOLIDLINE);
	x0 = x;
	y0 = y;
      }
    }

    // if plotting roll, also plot acrosstrack slope - roll 
    if (showTime == MBEDIT_PLOT_ROLL) {
      mbedit_xtrackslope(current_id, &tsslope);
      int i_tmp = 0;
      mbedit_tsvalue(i_tmp, showTime, &tsvalue);
      x0 = margin / 2 + (int)((tsvalue - tsslope - tsmin) * tsscale);
      y0 = ymax - (int)(dy / 2);
      for (int i = current_id; i < current_id + nplot; i++) {
	//x = margin/2 + ping[i].heading / 360.0 * 2 * margin;
	mbedit_xtrackslope(i, &tsslope);
	mbedit_tsvalue(i, showTime, &tsvalue);
	const int x = margin / 2 + (int)((tsvalue - tsslope - tsmin) * tsscale);
	const int y = ymax - (int)(dy / 2) - (int)((i - current_id) * dy);
	xg_drawline(dummy_, x0, y0, x, y, pixel_values[BLUE],
		    XG_SOLIDLINE);
	x0 = x;
	y0 = y;
      }
    }
  }

  int status = MB_SUCCESS;

  // plot pings 
  for (int i = current_id; i < current_id + nplot; i++) {
    // set beam plotting locations 
    // const int x = xmax - (int)(dx / 2) - (int)((i - current_id) * dx);
    const int y = ymax - (int)(dy / 2) - (int)((i - current_id) * dy);
    ping[i].label_x = xmin - 5;
    ping[i].label_y = y;
    for (int j = 0; j < ping[i].beams_bath; j++) {
      if (!mb_beam_check_flag_unusable2(ping[i].beamflag[j])) {
	if (view_mode == MBEDIT_VIEW_WATERFALL) {
	  ping[i].bath_x[j] =
	    (int)(xcen + dxscale * ping[i].bathacrosstrack[j]);
	  
	  ping[i].bath_y[j] =
	    (int)(y + dyscale * ((double)ping[i].bath[j] - bathmedian));
	}
	else if (view_mode == MBEDIT_VIEW_ALONGTRACK) {
	  ping[i].bath_x[j] =
	    (int)(xcen + dxscale * ping[i].bathacrosstrack[j]);
	  
	  ping[i].bath_y[j] =
	    (int)(ycen + dyscale * ((double)ping[i].bath[j] - bathmedian));
	}
	else {
	  // ping[i].bath_x[j] = x;
	  ping[i].bath_x[j] =
	    (int)(xcen + dxscale * (ping[i].bathalongtrack[j] +
				    ping[i].distance -
				    ping[current_id + nplot / 2].distance));
	  
	  ping[i].bath_y[j] =
	    (int)(ycen + dyscale * ((double)ping[i].bath[j] - bathmedian));
	}
      }
      else {
	ping[i].bath_x[j] = 0;
	ping[i].bath_y[j] = 0;
      }
    }

    // plot the beams 
    for (int j = 0; j < ping[i].beams_bath; j++)
      status = mbedit_plot_beam(i, j);

    // plot the ping profile 
    status = mbedit_plot_ping(i);

    // set and draw info string 
    mbedit_plot_ping_label(i, true);
  }

  // set status 
  if (nplot > 0)
    status = MB_SUCCESS;
  else
    status = MB_FAILURE;

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       nplot:       %d\n", *nplt);
    fprintf(stderr, "dbg2       error:      %d\n", error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:  %d\n", status);
  }

  return (status);
}



void MainWindow::xg_drawline(void *dummy,
			     int x1, int y1, int x2, int y2,
			     Color color, int style) {

  setPenColorAndStyle(color, style);
  
  painter_->drawLine(x1, y1, x2, y2);
}


void MainWindow::xg_drawrectangle(void *dummy,
				  int x, int y, int width, int height,
				  Color color, int style) {

  setPenColorAndStyle(color, style);

  painter_->drawRect(x, y, width, height);
}


void MainWindow::xg_drawstring(void *dummy, int x, int y, char *string,
			       Color color, int style) {

  setPenColorAndStyle(color, style);
  QTextStream(&qTextBuf_) << string;
  painter_->drawText(x, y, qTextBuf_);
}


void MainWindow::xg_fillrectangle(void *dummy,
				  int x, int y, int width, int height,
				  Color color, int style) {
  setPenColorAndStyle(color, style);

  // Set fill color
 const char *fillColor;
  
  switch (color) {
  case WHITE:
    fillColor = "white";
    break;

  case BLACK:
    fillColor = "black";
    break;

  case RED:
    fillColor = "red";
    break;

  case BLUE:
    fillColor = "blue";
    break;

  case CORAL:
    fillColor = "coral";
    break;

  case LIGHTGREY:
    fillColor = "lightGray";
    break;

  default:
    std::cerr << "xg_fillrectange(): unknown fill color!\n";
    fillColor = "white";
  }  
  painter_->fillRect(x, y, width, height, fillColor);
}


void MainWindow::setPenColorAndStyle(Color color, int style) {

  QPen pen = painter_->pen();
  
  switch (color) {
  case WHITE:
    pen.setColor(Qt::white);
    break;

  case BLACK:
    pen.setColor(Qt::black);
    break;

  case RED:
    pen.setColor(Qt::red);
    break;

  case BLUE:
    pen.setColor(Qt::blue);
    break;

  case CORAL:
    pen.setColor("coral");
    break;

  case LIGHTGREY:
    pen.setColor("lightGray");
    break;

  default:
    std::cerr << "setPenColorAndStyle(): unknown color!\n";
  }


  if (style == XG_DASHLINE) {
    pen.setStyle(Qt::DashLine);
  }
  else {
    pen.setStyle(Qt::SolidLine);    
  }
  
}

