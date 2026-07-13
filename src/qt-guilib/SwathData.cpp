#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <iostream>
#include <string.h>
#include <limits>
#include <cmath>

#include "SwathData.h"
// Need to make phony typdefs for X11 types here as we need to include
// header that includes them in prototype declarations for
// functions we do not use
typedef void *  Widget;
typedef void *  XtAppContext;
typedef void *  XtPointer;
typedef void *  String;

typedef bool Boolean;

// type used in mbeditviz
typedef bool Boolean;

extern "C" {
#include "mb_define.h"
#include "mb_io.h"

// Defines mbeditviz functions and  extern variables
#include "mbeditviz.h"

}

using namespace mb_system;


SwathData::SwathData() {

  // Initialize member variables lifted from mbeditviz, assuming no
  // command-line options specified
  appName_ = strdup("SwathDataGridApp");
  mbeditviz_init(0, nullptr,
                 appName_,
                 (char *)"this is a help message",
                 (char *)"swathReaderTest filename",
                 &SwathData::showMessage,
                 &SwathData::hideMessage,
                 &SwathData::updateGui,
                 &SwathData::showErrorDialog);

}

SwathData::~SwathData() {
}


bool SwathData::readDatafile(char *swathFile) {

  int verbose = 1;
  int error = 0;

  // Check that file is accessible
  if (access(swathFile, R_OK) == -1) {
    std::cerr << "SwathData::readDatafile(): cannot access" << swathFile << "\n";
    return false;
  }

  // swathfile is either an individual swath file or a datalist (file extension
  // ".mb-1"). 
  int sonarFormat = 0;
  if (mb_get_format(verbose, (char *)swathFile, NULL, &sonarFormat, &error) !=
      MB_SUCCESS) {
    std::cerr << "Couldn't determine sonar format of " << swathFile << std::endl;
    return false;
  }

  // Determine if input swathFile is a datalist
  bool inputIsDataList = false;
  if (sonarFormat == -1) {
    inputIsDataList = true;
  }
  
  // Call mbeditviz functions to load data from swath file(s)
  // Note mbeditviz.h defines global variables prefixed with mbev_*
  
  // Get list of relevant files into global C structure
  if (!inputIsDataList) {
    // Input of single swath file (not datalist) specified
    if (mbeditviz_import_file((char *)swathFile, sonarFormat) != MB_SUCCESS) {
      std::cerr << "Couldn't import data from " << swathFile << std::endl;
      return false;
    }
    // Read swath data from first file into global structures
    // Just reading file, so no need to lock
    if (mbeditviz_load_file(0, false) != MB_SUCCESS) {
      std::cerr << "Couldn't load data from " << swathFile << std::endl;
      return false;
    }
  }
  else {
    // Import each file in the datalist
    // NOTE: all files get imported at this point
    bool done = false;
    int status;
    void *datalist;
    int fileStatus;
    int rawFormat;
    double weight;
    char rawFile[MB_PATH_MAXLINE];
    char processedFile[MB_PATH_MAXLINE];
    char dfile[MB_PATH_MAXLINE];    
    int nFile = 0;
    
    if ((status = mb_datalist_open(mbev_verbose, &datalist, swathFile,
				   MB_DATALIST_LOOK_NO,
				   &mbev_error)) == MB_SUCCESS) {
      while (!done) {
	if (mb_datalist_read2(mbev_verbose, datalist, &fileStatus,
			      rawFile, processedFile, dfile,
			      &rawFormat, &weight, &mbev_error) == MB_SUCCESS) {
	
	  mbev_status = mbeditviz_import_file(rawFile, rawFormat);
	  if (mbeditviz_load_file(nFile++, false) != MB_SUCCESS) {
	    std::cerr << "Couldn't load data from " << swathFile << std::endl;
	    return false;
	  }	  
	}
	else {
	  // Done reading datalist
	  mbev_status = mb_datalist_close(mbev_verbose, &datalist, &mbev_error);
	  done = true;
	}
      }
    }
    else {
      std::cerr << "Error " << status << " from mb_dataList_open()\n";
      return false;
    }
  }
    
  // Need to unlock file when done reading
  std::cout << "Unlock swath file" << std::endl;
  if (!unlockSwath((char *)swathFile)) {
    std::cerr << "Couldn't unlock " << swathFile << "\n";
    return false;
  }

  // Get bounds of loaded swath data
  mbeditviz_get_grid_bounds();

  // Release any previously loaded sounding memory
  mbeditviz_mb3dsoundings_dismiss();

  // Prepare grid to contain loaded swath data
  mbeditviz_setup_grid();

  // Allocate memory and load individual imported swath soundings
  // NOTE: mbeditviz_project_soundings() must be called before reading
  // navlonx/navlaty, as it fills those fields with the projected (UTM)
  // coordinates.  Before this call they are zero/uninitialised.
  mbeditviz_project_soundings();

  // Load imported sounding data into grid
  mbeditviz_make_grid();

  // Save pointer to grid struct
  gridData_ = &mbev_grid;

  // Point to swath data (still valid after mbeditviz calls above)
  mbev_file_struct* swathData = &mbev_files[0];

  // Load navigation track points for each imported swath file now that
  // projection has been applied.
  // getXYZ() uses
  // boundsutm[0] (easting) → VTK x and boundsutm[2] (northing) → VTK y,
  // so navlonx (easting) → VTK x and
  // navlaty (northing) → VTK y — no swap needed.
  // sensordepth is positive meters below surface in MB-System;
  // negate for VTK z-up.
  navTrackPoints_->Reset();
  navTrackPoints_->SetNumberOfPoints(swathData->num_pings);
  for (vtkIdType i = 0; i < swathData->num_pings; i++) {
    navTrackPoints_->SetPoint(i,
                              swathData->pings[i].navlonx,  // easting  → VTK x
                              swathData->pings[i].navlaty,  // northing → VTK y
			      // negate: positive depth → negative VTK z   
                              -swathData->pings[i].sensordepth); 
  }

  // Set grid zmin and zmax, since the above mbeditviz functions do not
  int nPts = gridData_->n_rows * gridData_->n_columns;
  gridData_->min = std::numeric_limits<float>::max();
  gridData_->max = -std::numeric_limits<float>::max();

  for (int i = 0; i < nPts; i++) {
    if (gridData_->val[i] == gridData_->nodatavalue) {
      // No z data at this point
      gridData_->val[i] = TopoData::NoData;
      /// gridData_->val[i] = std::nanf("");
      continue;
    }

    if (gridData_->val[i] < gridData_->min) {
      // Found new minimum z
      gridData_->min = gridData_->val[i];
    }
    if (gridData_->val[i] > gridData_->max) {
      // Found new maximumm z
      gridData_->max = gridData_->val[i];
    }
  }
  std::cerr << "done getting grid min/max: min=" << gridData_->min <<
    "  max=" << gridData_->max << std::endl;

  std::cout << "Done with SwathData::readDatafile()" << std::endl;
  return true;
}


void SwathData::getParameters(int *nRows, int *nColumns,
                                double *xMin, double *xMax,
                                double *yMin, double *yMax,
                                double *zMin, double *zMax,
                                char **xUnits, char **yUnits, char **zUnits) {

  *nRows = gridData_->n_rows;
  *nColumns = gridData_->n_columns;
  *xMin = gridData_->boundsutm[0];
  *xMax = gridData_->boundsutm[1];
  *yMin = gridData_->boundsutm[2];
  *yMax = gridData_->boundsutm[3];
  *zMin = gridData_->min;;
  *zMax = gridData_->max;

  // TBD: NOT SURE WHERE TO FIND THESE
  *xUnits = strdup("easting");
  *yUnits = strdup("northing");
  *zUnits = strdup("meters");

}


bool SwathData::getXYZ(int row, int col,
		       double *x, double *y, double *z) {

  // boundsutm[0/1] = west/east easting; boundsutm[2/3] = south/north northing.
  // col goes east-west (easting direction), row goes north-south (northing direction).
  // This matches getParameters() which labels boundsutm[0] as xMin (easting)
  // and boundsutm[2] as yMin (northing).
  *x = gridData_->boundsutm[0] + col * gridData_->dx;  // easting → VTK x
  *y = gridData_->boundsutm[2] + row * gridData_->dy;  // northing → VTK y
  int index = col * gridData_->n_rows + row;
  *z = gridData_->val[index];

  return true;
}


bool SwathData::unlockSwath(char *swathfile) {

  bool usingLocks = false;
  std::cout << "unlockSwath(" << swathfile << ")" << std::endl;
  if (usingLocks) {
    int lockError = 0;

    int status = mb_pr_unlockswathfile(mbev_verbose, swathfile,
				       MBP_LOCK_EDITBATHY, appName_,
				       &lockError);
    if (status == MB_SUCCESS) {
      return true;
    }
    else {
      return false;
    }
  }
  else {
    std::cerr << "TEST: Skipping file lock check\n";
    return true;
  }
}


bool SwathData::setProjString() {

  if (!strncmp(gridData_->projection_id,
               TopoData::UtmType_,
               strlen(TopoData::UtmType_))) {
    sprintf(projString_, "+proj=utm +zone=%s +datum=WGS84",
            gridData_->projection_id + strlen(TopoData::UtmType_));

    return true;
  }
  else {
    // Unhandled projection/CRS
    std::cerr << "unhandled projection type: " << gridData_->projection_id <<
      std::endl;
    return false;
  }
}
