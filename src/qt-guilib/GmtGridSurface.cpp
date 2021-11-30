#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <math.h>
#include <QVector>
#include <QVector3D>
#include <QDebug>
#include "GmtGridSurface.h"
#include "colorTables.h"

using namespace mb_system;

GmtGridSurface::GmtGridSurface() {

  // Create Haxby color table
  QList<QVector3D> haxbyScale;
  haxbyScale.append(QVector3D(0.950, 0.950, 0.950));
  haxbyScale.append(QVector3D(1.000, 0.729, 0.522));
  haxbyScale.append(QVector3D(1.000, 0.631, 0.267));
  haxbyScale.append(QVector3D(1.000, 0.741, 0.341));
  haxbyScale.append(QVector3D(0.941, 0.925, 0.475));
  haxbyScale.append(QVector3D(0.804, 1.000, 0.635));
  haxbyScale.append(QVector3D(0.541, 0.925, 0.682));
  haxbyScale.append(QVector3D(0.416, 0.922, 1.000));
  haxbyScale.append(QVector3D(0.196, 0.745, 1.000));
  haxbyScale.append(QVector3D(0.157, 0.498, 0.984));
  haxbyScale.append(QVector3D(0.145, 0.224, 0.686));

  colorMap_ = new ColorMap();
  colorMap_->initialize(haxbyScale);

}

GmtGridSurface::~GmtGridSurface() {
}



bool GmtGridSurface::build(const char *gridFile) {

  void *gmtApi = nullptr;
  GMT_GRID *gmtGrid = readGridFile(gridFile, &gmtApi);
  if (!gmtGrid) {
    fprintf(stderr, "Couldn't open grid file \"%s\"\n", gridFile);
    return false;
  }

 setData(gmtApi, gmtGrid);

 /* ***
 qDebug() << "normals().constData():";
 qDebug() << normals().constData();
 for (int i = 0; i < normals().size(); i++) {
   qDebug() << "i=" << i << ": " << normals().constData()[i];
 }
 *** */

 float min, max;
 qDebug() << "xSpan(): " << xSpan(&min, &max);
 qDebug() << "ySpan(): " << ySpan(&min, &max);
 qDebug() << "zSpan(): " << zSpan(&min, &max);
 
  return true;
}


GMT_GRID *GmtGridSurface::readGridFile(const char *gridFile,
					  void **api) {
    fprintf(stderr, "readGridFile(): gridFile: %s\n", gridFile);
    // Check for file existence and readability
    struct stat fileStatus;

    if (stat(gridFile, &fileStatus) != 0
            || (fileStatus.st_mode & S_IFMT) == S_IFDIR
            || fileStatus.st_size <= 0) {
        qCritical() << "Can not read \"" << gridFile << "\"";
        return nullptr;
    }

    fprintf(stderr, "readGridFile(): create session\n");
    // Create GMT API
    *api =
            GMT_Create_Session("Topography::loadGrid()", 2U, 0U, nullptr);

    if (!*api) {
        qCritical() << "Could not get GMT API for \"" << gridFile << "\"";
        return nullptr;
    }

    fprintf(stderr, "gridFile now: %s\n", gridFile);

    GMT_GRID *grid = nullptr;
    // Try to read header and grid
    for (int nTry = 0; nTry < 100; nTry++) {
        grid = (struct GMT_GRID *)GMT_Read_Data(*api, GMT_IS_GRID, GMT_IS_FILE, GMT_IS_SURFACE,
                                                GMT_GRID_ALL, nullptr, gridFile, nullptr);
        if (grid) break;
        usleep(1000);
    }

    if (!grid) {
        qCritical() << "Unable to read GMT grid from \"" << gridFile << "\"";
        return nullptr;
    }
    return grid;
}


void GmtGridSurface::setData(void *gmtApi, GMT_GRID *gmtGrid) {

  initialize();
  
  struct GMT_GRID_HEADER *header = gmtGrid->header;

  /* ***
  // Replace any nan values
  for (uint row = 0; row < header->n_rows; row++) {
    for (uint col = 0; col < header->n_columns; col++) {
        uint dataInd =
	    GMT_Get_Index(gmtApi, gmtGrid->header, row, col);
	if (isnanf(gmtGrid->data[dataInd])) {
	  gmtGrid->data[dataInd] = 0.;
	}
    }
  }
  *** */

  bool seaLevelClip = true;
  
  if (!seaLevelClip) {
    zMin_ = header->z_min;
    zMax_ = header->z_max;
  }
  else {
    // Clip min and max to be below sea level
    zMin_ = std::min<float>(header->z_min, 0.);
    zMax_ = std::min<float>(header->z_max, 0.);
  }
  qDebug() << "setData(): zMin_: " << zMin_ << ", zMax_: " << zMax_;
  
  qDebug() << "nRows: " << header->n_rows << ", nCols: " << header->n_columns;
  float zPrevRow = 0.f;
  // Populate vertices with GMT_GRID data
  for (uint row = 0; row < header->n_rows; row++) {
    
    if (gmtGrid->y[row] < yMin_) { yMin_ = gmtGrid->y[row]; }
    if (gmtGrid->y[row] > yMax_) { yMax_ = gmtGrid->y[row]; }    

    for (uint col = 0; col < header->n_columns; col++) {

          uint dataInd =
	    GMT_Get_Index(gmtApi, gmtGrid->header, row, col);

          float z = gmtGrid->data[dataInd];
	  
          float red = 0.f, green = 0.f, blue = 0.f, alpha = 1.f;

          // Assign color based on z
	  if (isnan(z) || (seaLevelClip && z > 0.)) {
	    alpha = 0.;   // transparent
	  }
	  else {
	    colorMap_->rgbValues(z, zMin_, zMax_, &red, &green, &blue);
	  }
	  
	  vertices_.push_back(Vertex(Point3D((float )gmtGrid->x[col],
						(float )gmtGrid->y[row], z),
				      Point4D(red, green, blue, alpha)));

	  if (gmtGrid->x[col] < xMin_) { xMin_ = gmtGrid->x[col]; }
	  if (gmtGrid->x[col] > xMax_) { xMax_ = gmtGrid->x[col]; }
	  
          // Estimate normal vector at this point, based on slope
          if (col > 0 && row > 0) {
              // Get z at (prev-x, y)
              uint prevInd = GMT_Get_Index(gmtApi, gmtGrid->header, row, col-1);
              float slopeX = (z - gmtGrid->data[prevInd]) / (float )(gmtGrid->x[col] - gmtGrid->x[col-1]);

              // Get z at (x, prev-y)
              // prevInd = (row - 1) * header->n_rows + col;
              prevInd = GMT_Get_Index(gmtApi, gmtGrid->header, row-1, col);
	      zPrevRow = gmtGrid->data[prevInd];
              float slopeY = (z - zPrevRow) / (float )(gmtGrid->y[row] - gmtGrid->y[row-1]);

              // Normal vector is cross-product of slopeX and slopeY
              const float planeVecX[] = {1.f, 0., slopeX};
              const float planeVecY[] = {0., 1.f, slopeY};

              float normVec[] =
              {(planeVecX[1] * planeVecY[2] - planeVecX[2] * planeVecY[1]),
               (planeVecX[2] * planeVecY[0] - planeVecX[0] * planeVecY[2]),
               (planeVecX[0] * planeVecY[1] - planeVecX[1] * planeVecY[0])};

              // Normalize the normal vector
              float length = sqrt(normVec[0]*normVec[0] +
                   normVec[1]*normVec[1] + normVec[2]*normVec[2]);

	      normals_.push_back(Point3D(normVec[0] / length,
	      	  normVec[1] / length,
	      	  normVec[2] / length));
	  }
          else {  // Point along x=0 or y=0 edge
	    // Assume some slope for points at edge of grid
	    // qDebug() << "set edge normal";
	    normals_.push_back(Point3D(0., 0., 1.));
	  }

        }
    }

  // Build triangle drawing indexes
  for (uint row = 0; row < header->n_rows - 1; row++) {
      for (uint col = 0; col < header->n_columns - 1; col++) {

          // First triangle
 	  indices_.push_back(vertexIndex(col, row, header->n_columns));
	  indices_.push_back(vertexIndex(col+1, row, header->n_columns));
	  indices_.push_back(vertexIndex(col+1, row+1, header->n_columns));
          // Second triangle
	  indices_.push_back(vertexIndex(col, row, header->n_columns));
	  indices_.push_back(vertexIndex(col+1, row+1, header->n_columns));
	  indices_.push_back(vertexIndex(col, row+1, header->n_columns));

        }
    }

  qDebug("Done with setData()");
  qDebug() << "Got " << vertices_.size() << " vertices, " <<
    normals_.size() << " normals, " << indices_.size() << " indices";
}

/* ***
void GmtGridSurface::setColor(float z, float zmin, float zrange,
			      float *red, float *green, float *blue) {


}
*** */
