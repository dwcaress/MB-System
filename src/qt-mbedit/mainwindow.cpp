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
  
  error_ = MB_ERROR_NO_ERROR;
  
    ui->setupUi(this);
    canvas_ = new QPixmap();
    painter_ = new QPainter(canvas_);
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
						  QDir::homePath(), tr("swath files (*.m*)"));

  qDebug() << "open swath file " << fileName;
  std::string utf8_text = fileName.toUtf8().constData();
  std::cerr << "utf8_text: " << utf8_text << "\n";
  char *fname = (char *)utf8_text.c_str();
  std::cerr << "fname: " << fname << "\n";
  if (!grid_.readDatafile(fname)) {
    std::cerr << "Error reading file " << fname << "\n";
    return;
  }

    // Draw swath data to pixmap
  if (!plot(painter_, &grid_)) {
    std::cerr << "Error plotting data from " << fname << "\n";
  }
  
  // Add pixmap to UI label
  qDebug() << "Draw on GUI\n";
  ui->swathCanvas->setPixmap(*canvas_);
}


bool MainWindow::plot(QPainter *painter, SwathGridData *grid) {
  return false;
}


/*--------------------------------------------------------------------*/
int MainWindow::mbedit_init(int argc, char **argv, int *startup_file) {

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
	*startup_file = fileflag > 0;

	if (verbose_ >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2       startup_file: %d\n", *startup_file);
		fprintf(stderr, "dbg2       error:        %d\n", error_);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:  %d\n", status);
	}

	return (status);
}


// Derived from do_parse_datalist() in mbedit_callbacks.c
void MainWindow::parseDatalist(char *file, int form) {
  return;

  /* ***
  int format;

  // try to resolve format if necessary
  if (form == 0)
    mbedit_get_format(file, &format);
  else
    format = form;

  // read in a single file
  if (format > 0 && numfiles < NUM_FILES_MAX) {
    strcpy(filepaths[numfiles], file);
    fileformats[numfiles] = format;
    filelocks[numfiles] = -1;
    fileesfs[numfiles] = -1;
    numfiles++;
  } else if (format == -1) {
    // read in datalist if format = -1
    const int verbose = 0;
    void *datalist;
    int error = MB_ERROR_NO_ERROR;
    const int datalist_status =
      mb_datalist_open(verbose, &datalist, file, MB_DATALIST_LOOK_NO, &error);
    if (datalist_status == MB_SUCCESS) {
      bool done = false;
      double weight;
      int filestatus;
      int fileformat;
      char fileraw[MB_PATH_MAXLINE];
      char fileprocessed[MB_PATH_MAXLINE];
      char dfile[MB_PATH_MAXLINE];
      while (!done) {
	if ((mb_datalist_read2(verbose, datalist, &filestatus, fileraw,
			       fileprocessed, dfile,
			       &fileformat, &weight, &error)) == MB_SUCCESS) {
	  if (numfiles < NUM_FILES_MAX) {
	    strcpy(filepaths[numfiles], fileraw);
	    fileformats[numfiles] = fileformat;
	    filelocks[numfiles] = -1;
	    fileesfs[numfiles] = -1;
	    numfiles++;
	  }
	} else {
	  mb_datalist_close(verbose, &datalist, &error);
	  done = true;
	}
      }
    }
  }
  *** */
}
