#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <proj.h>
#include "vtkDataSet.h"
#include "vtkErrorCode.h"
#include "vtkSetGet.h"
#include "vtkCallbackCommand.h"
#include "vtkDelaunay2D.h"
#include "vtk_netcdf.h"
#include "TopoDataReader.h"
#include "GmtGridData.h"
#include "SwathData.h"


#define VTK_CREATE(type, name) vtkSmartPointer<type> name = vtkSmartPointer<type>::New()

#define UTM_X_NAME "Easting (meters)"
#define UTM_Y_NAME "Northing (meters)"

#define GMT_EXTENSION ".grd"
#define SWATH_EXTENSION_PREFIX ".mb"


using namespace mb_system;

void AprojTest(char *msg) {

  std::cout << "AprojTest(): " << msg << std::endl;

  const int BSIZE = 1024;
  double xMin = 0.;
  
  // Get UTM zone of grid's W edge
  int utmZone = ((xMin + 180)/6 + 0.5);

  std::cerr << "UTM zone: " << utmZone << std::endl;
  
  PJ_INFO projInfo = proj_info();
  std::cerr << "proj release: " << projInfo.release << std::endl;
  
  PJ_CONTEXT *projContext = ::proj_context_create();
  if (projContext) {
    std::cerr << "Created projContext OK" << std::endl;
  }
  else {
    std::cerr << "Error creating projContext OK" << std::endl;
  }

  const char *srcCRS = "EPSG:4326";
  char targCRS[64];
  sprintf(targCRS, "+proj=utm +zone=%d +datum=WGS84", utmZone); 
  PJ *proj = ::proj_create_crs_to_crs (projContext,
                                       srcCRS,
                                       targCRS,
                                       nullptr);
  bool error = false;
  if (!proj) {
    std::cerr << "failed to create proj" << std::endl;
    exit(1);
  }
  else {
    std::cerr << "created proj OK" << std::endl;    
  }

  if (error) {
    exit(-1);
  }
}


TopoDataReader::TopoDataReader() :
  fileName_(nullptr),
  dataType_(TopoDataType::Unknown),
  xUnits_(nullptr), yUnits_(nullptr), zUnits_(nullptr),
  projContext_(nullptr), projTransform_(nullptr)
{
  
  gridPoints_ = vtkSmartPointer<vtkPoints>::New();
  gridPoints_->SetDataTypeToDouble();
  gridPolygons_ = vtkSmartPointer<vtkCellArray>::New();  

  this->SetNumberOfInputPorts(0);

  VTK_CREATE(vtkCallbackCommand, cbc);
  cbc->SetCallback(&TopoDataReader::SelectionModifiedCallback); // what's this?
  cbc->SetClientData(this);  // what's this?

  AprojTest((char *)"from TopoDataReader constructor");
}

TopoDataReader::~TopoDataReader() {
}


int TopoDataReader::RequestData(vtkInformation* request,
			       vtkInformationVector** inputVector,
			       vtkInformationVector* outputVector) {

  std::cerr << "TopoDataReader::RequestData()" << std::endl;
  (void)request;     // Unused parameter
  (void)inputVector; // Unused parameter
  
  vtkInformation* outInfo = outputVector->GetInformationObject(0);

  // Check for valid output vtkDataSet
  vtkDataSet* output = vtkDataSet::GetData(outInfo);
  if (!output)
  {
    vtkErrorMacro(<< "Bad output type.");
    SetErrorCode(vtkErrorCode::UserError);
    return 0;
  }  

  vtkPolyData* polyOutput = vtkPolyData::SafeDownCast(output);
  if (!polyOutput) {
    SetErrorCode(vtkErrorCode::UserError);
    vtkErrorMacro(<< "Output is not vtkPolyData");
    return 0;
  }
  
  // Read grid file
  std::cerr << "TopoDataReader::RequestData() - readGridfile()" << std::endl;
  topoData_ = readDatafile(fileName_);
  
  if (!topoData_) {
    // std::cerr << "Error reading " << fileName_ << ": SetErrorCode\n";
    SetErrorCode(vtkErrorCode::CannotOpenFileError);
    return 0;
  }


  if (xUnits_) {
    free((void *)xUnits_);
  }
  if (yUnits_) {
    free((void *)yUnits_);
  }
  if (zUnits_) {
    free((void *)zUnits_);
  }

  topoData_->units(&xUnits_, &yUnits_, &zUnits_);

    
  //  PJ *toUTM = nullptr;
  PJ_CONTEXT *projContext = nullptr;

  // If grid is in geographic CRM, must project it to UTM
  // to maintain same scale (meters) on x, y, and z axes
  bool convertToUTM = geographicCRS();

  double xMin, xMax, yMin, yMax, zMin, zMax;
  topoData_->bounds(&xMin, &xMax, &yMin, &yMax, &zMin, &zMax);
  std::cerr << "xmin=" << xMin << ", xMax=" << xMax
            << ", yMin=" << yMin << ", yMax=" << yMax << std::endl;

  // If geographic CRS, then compute scale factor such that z data scale
  // matches that of lat/lon axes
  if (geographicCRS()) {
    float zScale = TopoDataReader::zScaleLatLon(yMax - yMin,
                                                xMax - xMin,
                                                zMax - zMin);
    
    std::cerr << "TopoDataReader: lat-lon z-scale: " << zScale << std::endl;
  }
  
  if (projTransform_) {
    // Clean up previous transform and context
    proj_destroy(projTransform_);
    proj_context_destroy(projContext_);
    projTransform_ = nullptr;
    projContext_ = nullptr;
  }
    
  // If x and y are not in UTM, must convert them to UTM
  if (convertToUTM) {

    std::cout << "file projection is not UTM" << std::endl;
    // Set up PROJ software first...

    // Get UTM zone of grid's W edge
    int utmZone = ((xMin + 180)/6 + 0.5);

    PJ_CONTEXT *projContext = proj_context_create();

    sprintf(displayCRS_, "+proj=utm +zone=%d +datum=WGS84", utmZone); 
    projTransform_ = proj_create_crs_to_crs (projContext,
                                             topoData_->projString(),
                                             displayCRS_,
                                             nullptr);
    if (!projTransform_) {
      vtkErrorMacro(<< "failed to create PJ transform");
      SetErrorCode(vtkErrorCode::UserError);
      return 0;      
    }

    // Ensure proper coordinate order
    PJ* normT = proj_normalize_for_visualization(projContext,
                                                 projTransform_);
    proj_destroy(projTransform_);
    
    if (!normT) {
      vtkErrorMacro(<< "failed to create norm proj");
      SetErrorCode(vtkErrorCode::UserError);
      return 0;      
    }

    projTransform_ = normT;
  }
  else {
    std::cout << "Already in UTM" << std::endl;
  }

  
  unsigned nRows = topoData_->nRows();
  unsigned nColumns = topoData_->nColumns();
  std::cerr << "nRows=" << nRows << ", nColumns=" << nColumns << std::endl;
  
  // Pre-allocate points memory
  if (!gridPoints_->Allocate(nRows * nColumns)) {
    std::cerr << "failed to allocate "
	      <<  nRows * nColumns << " points"
	      << std::endl;
  }

  // Reset/clear points
  gridPoints_->Reset();

  unsigned row;
  unsigned col;

  // Load points read from grid file
  double x, y, z;
  double lat, lon;
  int nValidPoints = 0;
  bool gridMissingZValues = false;
  std::cerr << "TopoDataReader::RequestData() - load points" << std::endl;    

  for (row = 0; row < nRows; row++) {
    for (col = 0; col < nColumns; col++) {

      if (!convertToUTM) {
        // Already in UTM
        topoData_->getXYZ(row, col, &x, &y, &z);
        // Don't insert NaN-valued data
        if (std::isnan(z) || z == TopoData::NoData) {
          gridMissingZValues = true;

          // Don't insert nan values           
          if (std::isnan(z)) {
            z = TopoData::NoData;
          }
        }
        
        vtkIdType id = gridPoints_->InsertNextPoint(x, y, z);
        nValidPoints++;
      }
      else {
        topoData_->getXYZ(row, col, &lon, &lat, &z);
        if (std::isnan(z) || z == TopoData::NoData) {
          gridMissingZValues = true;
          // Don't insert nan values 
          if (std::isnan(z)) {
            z = TopoData::NoData;
          }
        }        
        // Convert lat/lon to UTM
        PJ_COORD lonLat = proj_coord(lon, lat,
                                     0, 0);

        PJ_COORD utm = proj_trans(projTransform_, PJ_FWD, lonLat);
        vtkIdType id = gridPoints_->InsertNextPoint(utm.enu.e,
                                                    utm.enu.n,
                                                    z);
      }
    }
  }
  std::cerr << "nValidPoints=" << nValidPoints << "\n";
  

  double bounds[6];
  gridPoints_->GetBounds(bounds);
  std::cerr << "gridPoints_: xMin=" << bounds[0] << ", xMax=" << bounds[1] <<
    ", yMin=" << bounds[2] << ", yMax=" << bounds[3] <<
    ", zMin=" << bounds[4] << ", zMax=" << bounds[5] << std::endl;
  
  // Set Delaunay triangle vertices
  if (!gridPolygons_->Allocate(nRows * nColumns * 2)) {
    std::cerr << "failed to allocat "
	      <<  nRows * nColumns *2 << " polygons"
	      << std::endl;
  }

  vtkIdType triangleVertexId[3];
  // Triangles must stay within row and column bounds
  for (unsigned row = 0; row < nRows-1; row++) {
    for (unsigned col = 0; col < nColumns-1; col++) {

      // First triangle
      triangleVertexId[0] = gridOffset(nRows, nColumns, row, col);
      triangleVertexId[1] = gridOffset(nRows, nColumns, row, col+1);
      triangleVertexId[2] = gridOffset(nRows, nColumns, row+1, col+1);
      // If any of this triangle's vertices refer to point with no z-data,
      // then do not insert triangle into gridPolygons
      if (!gridMissingZValues || !triangleMissingZValues(triangleVertexId)) {
        gridPolygons_->InsertNextCell(3, triangleVertexId);
      }

      // Second triangle
      triangleVertexId[0] = gridOffset(nRows, nColumns, row, col);
      triangleVertexId[1] = gridOffset(nRows, nColumns, row+1, col+1);
      triangleVertexId[2] = gridOffset(nRows, nColumns, row+1, col);
      // If any of this triangle's vertices refer to point with no z-data,
      // then do not insert triangle into gridPolygons
      if (!gridMissingZValues || !triangleMissingZValues(triangleVertexId)) {
        gridPolygons_->InsertNextCell(3, triangleVertexId);
      }      
    }
  }

  // Save to object's points and polygons (need to output both for
  // downstream processing)
  polyOutput->SetPoints(gridPoints_);
  polyOutput->SetPolys(gridPolygons_);

  std::cerr << "TopoDataReader::RequestData() nPoints: "
	    << gridPoints_->GetNumberOfPoints()
    	    << std::endl;

  std::cerr << "TopoDataReader::RequestData() nCells: "
	    << gridPolygons_->GetNumberOfCells()
    	    << std::endl;
  
  std::cerr << "TopoDataReader::RequestData() - done" << std::endl;

  return 1;
}


void TopoDataReader::SetFileName(const char *fileName) {
  fprintf(stderr, "In TopoDataReader::SetFileName()\n");

  vtkAbstractPolyDataReader::SetFileName(fileName);
  if (fileName_) {
    fprintf(stderr, "TopoDataReader::SetFileName: free fileName_\n");
    free((void *)fileName_);
  }
  fprintf(stderr, "TopoDataReader::SetFileName: call strdup()\n");
  fileName_ = strdup((char *)fileName);
  
}


void TopoDataReader::SelectionModifiedCallback(vtkObject*, unsigned long,
                                              void* clientdata, void*)
{
  static_cast<TopoDataReader*>(clientdata)->Modified();
}


void TopoDataReader::gridBounds(double *xMin, double *xMax,
                                 double *yMin, double *yMax,
                                 double *zMin, double *zMax) {

  topoData_->bounds(xMin, xMax, yMin, yMax, zMin, zMax);
  /* ***
  double bounds[6];
  gridPoints_->GetBounds(bounds);
  *xMin = bounds[0];
  *xMax = bounds[1];
  *yMin = bounds[2];
  *yMax = bounds[3];

  // zMin/zMax do no include NoData
  *zMin = bounds[4];
  *zMax = bounds[5];  
  *** */
}


vtkIdType TopoDataReader::gridOffset(unsigned nRows, unsigned nCols,
                                    unsigned row, unsigned col) {
  if (row >= nRows || col >= nCols) { 
    // Out of bounds
    fprintf(stderr,
            "getGridOffset(): out-of-bounds: row=%d, nRows=%d, col=%d, nCols=%d\n",
	    row, nRows, col, nCols);
  }

  return  (col + row * nCols);
}



float TopoDataReader::zScaleLatLon(float latRange, float lonRange,
                                  float zRange) {

  float avgLatLonRange = (latRange + lonRange) / 2.;

  return avgLatLonRange / zRange;
}


float TopoDataReader::zScaleLatLon() {

  // Not really implemented yet. Do we still need it?
  return 1.;

  /* ***
  if (!geographicCRS()) {
    // Projected, not geographic - no need to scale z
    return 1.;
  }
  
  double xMin, xMax, yMin, yMax, zMin, zMax;
  topoData_->bounds(&xMin, &xMax, &yMin, &yMax, &zMin, &zMax);
  
  return TopoDataReader::zScaleLatLon(yMax - yMin,
                                      xMax - xMin,
                                      zMax - zMin);
                                      *** */
}





bool TopoDataReader::geographicCRS() {

  const char *projString = topoData_->projString();
  if (strstr(projString, "EPSG:4326")) {
    // Appears to be in geographic CRS (are there other geographic EPSGs?)
    return true;
  }

  // Assume projected CRS
  return false;
}


TopoData *TopoDataReader::readDatafile(char *filename) {

  TopoData *topoData = nullptr;

  SwathData *swathTest = nullptr;
  
  switch (dataType_) {
  case TopoDataType::GMTGrid:
    topoData = new GmtGridData();
    break;

  case TopoDataType::Swath:
    topoData = new SwathData();
    break;
    
  default:
    // Unknown data type
    std::cerr << filename << ": uhandled topo data type: " << dataType_
              << std::endl;    
    return nullptr;
  }

  if (!topoData->readDatafile(filename)) {
    std::cerr << "Error reading file " << filename << std::endl;
    return nullptr;
  }

  // Set grid parameters based on data just read from file
  topoData->setParameters();

  // Set proj-string for grid's CRS
  if (topoData->setProjString()) {
    std::cerr << "proj-string: " << topoData->projString() << std::endl;
  }
  else {
    // Unhandled projection
    std::cerr << "unhandled projection type in " << filename << std::endl;    
    return nullptr;
  }

  
  return topoData;
}


TopoDataType TopoDataReader::getDataType(const char *filename) {

  // Get file extension
  char *extension = strrchr((char *)filename, '.');
  if (!extension) {
    return TopoDataType::Unknown;
  }
  
  if (!strcmp(extension, GMT_EXTENSION)) {
    return TopoDataType::GMTGrid;
  }
  else if (!strncmp(extension, SWATH_EXTENSION_PREFIX,
                    strlen(SWATH_EXTENSION_PREFIX))) {
    return TopoDataType::Swath;
  }

  return TopoDataType::Unknown;

}


bool TopoDataReader::triangleMissingZValues(vtkIdType *vertices) {

  double *point;
  for (int v = 0; v < 3; v++) {
    point = gridPoints_->GetPoint(vertices[v]);
    if (point[2] == TopoData::NoData) {
      // Missing at least one z-value
      return true;
    }
  }

  // Vertices not missing any z-values
  return false;
}



const char *TopoDataReader::fileCRS() {
  if (!topoData_) {
    std::cerr << "No grid defined" << std::endl;
    return nullptr;
  }

  return topoData_->projString();
}


