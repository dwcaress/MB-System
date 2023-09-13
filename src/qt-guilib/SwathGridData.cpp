#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <iostream>
#include <string.h>
#include <limits>
#include <cmath>

#include "SwathGridData.h"
// Need to make phony typdefs for X11 types here as we need to include
// header that includes them in prototype declarations for
// functions we do not use
typedef void *  Widget;
typedef void *  XtAppContext;
typedef void *  XtPointer;
typedef void *  String;

typedef bool Boolean;

extern "C" {
#include "mbeditviz.h"
}


// Other types used in mbeditviz
typedef bool Boolean;

extern "C" {
#include "mb_define.h"
#include "mb_io.h"

// Defines mbeditviz functions and  extern variables
#include "mbeditviz.h"   

}

using namespace mb_system;


SwathGridData::SwathGridData() {

  // Initialize member variables lifted from mbeditviz, assuming no
  // command-line options specified
  appName_ = strdup("SwathDataGridApp");
  mbeditviz_init(0, nullptr,
                 appName_,
                 (char *)"this is a help message",
                 (char *)"swathReaderTest filename",
                 &SwathGridData::showMessage,
                 &SwathGridData::hideMessage,
                 &SwathGridData::updateGui,
                 &SwathGridData::showErrorDialog);
  
}

SwathGridData::~SwathGridData() {
}


bool SwathGridData::readDatafile(char *swathFile) {
  
  int verbose = 1;
  int error = 0;
  
  // Call mbeditviz functions to read data from file
  int sonarFormat = 0;
  if (mb_get_format(verbose, (char *)swathFile, NULL, &sonarFormat, &error) !=
      MB_SUCCESS) {
    std::cerr << "Couldn't determine sonar format of " << swathFile << std::endl;
    return false;
  }
  
  // Get list of relevant files into global C structure
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

  // Need to unlock file when done reading
  std::cout << "Unlock swath file" << std::endl;
  unlockSwath((char *)swathFile);

  // Point to swath data just loaded into global array
  mbev_file_struct* swathData = &mbev_files[0];

  // Get bounds of loaded swath data
  mbeditviz_get_grid_bounds();

  // Release previously loaded sounding memory
  mbeditviz_mb3dsoundings_dismiss();

  // Prepare grid to contain loaded swath data
  mbeditviz_setup_grid();

  // Allocate memory and load individual swath soundings
  mbeditviz_project_soundings();

  // Load sounding data into grid
  mbeditviz_make_grid();

  // Print swath navigation UTM coords
  std::cout << "# navEast, navWest" << std::endl;
  for (int i = 0; i < swathData->num_pings; i++) {
    struct mbev_ping_struct *ping = &swathData->pings[i];
    std::cout << ping->navlonx << ", " << ping->navlaty << std::endl;
  }
  
  // Save pointer to grid struct
  gridData_ = &mbev_grid;

  // Set grid zmin and zmax, since the above mbeditviz functions do not
  int nPts = gridData_->n_rows * gridData_->n_columns;
  gridData_->min = std::numeric_limits<float>::max();
  gridData_->max = -std::numeric_limits<float>::max();
  
  for (int i = 0; i < nPts; i++) {
    if (gridData_->val[i] == gridData_->nodatavalue) {
      // No z data at this point
      gridData_->val[i] = TopoGridData::NoData;
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
  
  std::cout << "Done with SwathGridData::readDatafile()" << std::endl;
  return true;
}


void SwathGridData::getParameters(int *nRows, int *nColumns,
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

/* ***
bool SwathGridData::data(int row, int col,
                         double *y, double *x, double *z) {

  *x = gridData_->bounds[0] + col * gridData_->dx; 
  *y = gridData_->bounds[2] + row * gridData_->dy;
  int index = row * gridData_->n_columns + col;
  *z = gridData_->val[index];

  return true;
}
*** */


bool SwathGridData::data(int row, int col,
                         double *x, double *y, double *z) {

  *x = gridData_->boundsutm[2] + col * gridData_->dx; 
  *y = gridData_->boundsutm[0] + row * gridData_->dy;
  //  int index = row * gridData_->n_columns + col;
  int index = col * gridData_->n_rows + row;
  *z = gridData_->val[index];

  return true;
}


void SwathGridData::unlockSwath(char *swathfile) {
  bool usingLocks = true;
  std::cout << "unlockSwath(" << swathfile << ")" << std::endl;
  if (usingLocks) {
    int lockError = 0;

    mb_pr_unlockswathfile(mbev_verbose, swathfile,
                          MBP_LOCK_EDITBATHY, appName_, &lockError);
    }
}


bool SwathGridData::setProjString() {

  if (!strncmp(gridData_->projection_id,
               TopoGridData::UtmType_,
               strlen(TopoGridData::UtmType_))) {
    sprintf(projString_, "+proj=utm +zone=%s +datum=WGS84",
            gridData_->projection_id + strlen(TopoGridData::UtmType_));
    
    return true;
  }
  else {
    // Unhandled projection/CRS
    std::cerr << "unhandled projection type: " << gridData_->projection_id <<
      std::endl;
    return false;
  }
}
