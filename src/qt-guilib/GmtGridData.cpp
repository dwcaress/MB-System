#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <iostream>
#include <string.h>
#include "GmtGridData.h"

using namespace mb_system;

bool GmtGridData::readDatafile(char *filename) {
  
  gmtGrid_ = readGmtFile(filename, &gmtAPI_);
  if (!gmtGrid_) {
    std::cerr << "error while reading " << filename << std::endl;
    return false;
  }
  
  return true;
}


void GmtGridData::getParameters(int *nRows, int *nColumns,
                                double *xMin, double *xMax,
                                double *yMin, double *yMax,
                                double *zMin, double *zMax,
                                char **xUnits, char **yUnits, char **zUnits) {

  *nRows = gmtGrid_->header->n_rows;
  *nColumns = gmtGrid_->header->n_columns;
  *xMin = gmtGrid_->header->wesn[0];
  *xMax = gmtGrid_->header->wesn[1];
  *yMin = gmtGrid_->header->wesn[2];
  *yMax = gmtGrid_->header->wesn[3];
  *zMin = gmtGrid_->header->z_min;
  *zMax = gmtGrid_->header->z_max;
                
  *xUnits = strdup(gmtGrid_->header->x_units);
  *yUnits = strdup(gmtGrid_->header->y_units);
  *zUnits = strdup(gmtGrid_->header->z_units);  
  
}


GMT_GRID *GmtGridData::readGmtFile(const char *gridFile, void **api) {
    fprintf(stderr, "readGridFile(): gridFile: %s\n", gridFile);
    // Check for file existence and readability
    struct stat fileStatus;

    if (stat(gridFile, &fileStatus) != 0
            || (fileStatus.st_mode & S_IFMT) == S_IFDIR
            || fileStatus.st_size <= 0) {
      std::cerr << "Can not read \"" << gridFile << "\"" << std::endl;
        return nullptr;
    }

    fprintf(stderr, "readGridFile(): create session\n");
    // Create GMT API
    *api =
            GMT_Create_Session("Topography::loadGrid()", 2U, 0U, nullptr);

    if (!*api) {
      std::cerr << "Could not get GMT API for \"" << gridFile << "\""
                << std::endl;
        return nullptr;
    }

    fprintf(stderr, "gridFile now: %s\n", gridFile);

    GMT_GRID *grid = nullptr;
    // Try to read header and grid
    for (int nTry = 0; nTry < 100; nTry++) {
        grid =
          (struct GMT_GRID *)GMT_Read_Data(*api, GMT_IS_GRID, GMT_IS_FILE,
                                           GMT_IS_SURFACE,
                                           GMT_GRID_ALL, nullptr,
                                           gridFile, nullptr);
        if (grid) break;
        usleep(1000);
    }

    if (!grid) {
      std::cerr << "Unable to read GMT grid from \"" << gridFile <<
        "\"" << std::endl;
      return nullptr;
    }
    bool debug = false;
    if (debug) {
      // print debug info
      for (int col = 0; col < grid->header->n_columns; col++) {
        fprintf(stderr, "x[%d]: %.8f\n", col, grid->x[col]);
      }
    }
    
    return grid;
}


bool GmtGridData::data(int row, int col,
                       double *x, double *y, double *z) {

  *x = gmtGrid_->x[col];
  *y = gmtGrid_->y[row];
  unsigned index = GMT_Get_Index(gmtAPI_, gmtGrid_->header, row, col);
  *z = gmtGrid_->data[index];

  return true;
}


#define PROJ_KEYWORD "Projection: "
// #define GEOGRAPHIC_TYPE "Geographic"
// #define UTM_TYPE "UTM"

bool GmtGridData::setProjString() {

  std::cerr << "GmtGridData::setProjString()" << std::endl;
  char *remark = gmtGrid_->header->remark;
  std::cerr << "remark: " << remark << std::endl;
  char *proj = strstr(remark, PROJ_KEYWORD);
  if (proj) {

    // Point to start of projection id
    proj += strlen(PROJ_KEYWORD);
    char *endl = strchr(proj, '\n');
    if (endl) {
      char *projType = strndup(proj, endl-proj);

      if (!strcmp(projType, TopoGridData::GeographicType_)) {
        sprintf(projString_, "EPSG:4326");
        return true;
      }
      else if (!strncmp(projType,
                        TopoGridData::UtmType_,
                        strlen(TopoGridData::UtmType_))) {
        sprintf(projString_, "+proj=utm +zone=%s +datum=WGS84",
                projType + strlen(TopoGridData::UtmType_));
        
        return true;
      }
      else {
        std::cerr << "Unhandled projection type: " << projType << std::endl;
      }
      free((void *)projType);
      
    }
  }
  else {
    std::cerr << "Couldn't find PROJ_KEYWORD in remarks (" << remark <<
      ")" << std::endl;
  }
  return false;
  
  return false;
}
