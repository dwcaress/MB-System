#include <stdio.h>
#include <string>
#include <sstream>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <proj.h>
#include "vtkDataSet.h"
#include "vtkErrorCode.h"
#include "vtkSetGet.h"
#include "vtkCallbackCommand.h"
#include "vtk_netcdf.h"
#include "SwathReader.h"

#define VTK_CREATE(type, name) vtkSmartPointer<type> name = vtkSmartPointer<type>::New()

using namespace mb_system;

SwathReader::SwathReader() :
  fileName_(nullptr) {

  std::cout << "SwathReader ctr" << std::endl;
  swathFormat_ = 0;

  swathPoints_ = vtkSmartPointer<vtkPoints>::New();
  swathPoints_->SetDataTypeToFloat();
  swathPolygons_ = vtkSmartPointer<vtkCellArray>::New();  

  this->SetNumberOfInputPorts(0);

  VTK_CREATE(vtkCallbackCommand, cbc);
  cbc->SetCallback(&SwathReader::SelectionModifiedCallback); // what's this?
  cbc->SetClientData(this);  // what's this?

  appName_ = strdup("SwathReaderApp");
    
  // Initialize member variables lifted from mbeditviz, assuming no
  // command-line options specified
  mbeditviz_init(0, nullptr,
                 appName_,
                 (char *)"this is a test",
                 (char *)"swathReaderTest filename",
                 &SwathReader::showMessage,
                 &SwathReader::hideMessage,
                 &SwathReader::updateGui,
                 &SwathReader::showErrorDialog);
}

SwathReader::~SwathReader() {
  std::cout << "SwathReader destructor" << std::endl;
}


int SwathReader::RequestData(vtkInformation* request,
                             vtkInformationVector** inputVector,
                             vtkInformationVector* outputVector) {

  std::cerr << "SwathReaderRequestData()" << std::endl;
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

  std::cout << "call readSwathFile() with fileName " << fileName_ << std::endl;
  if (!readSwathFile(fileName_)) {
    // Error reading swath file
    vtkErrorMacro(<< "Error reading swath data");
    SetErrorCode(vtkErrorCode::UserError);    
    return 0;
  }

  /* ****
  if (xUnits_) {
    free((void *)xUnits_);
  }
  if (yUnits_) {
    free((void *)yUnits_);
  }
  if (zUnits_) {
    free((void *)zUnits_);
  }
  
  xUnits_ = (const char *)strdup(gmtGrid_->header->x_units);
  yUnits_ = (const char *)strdup(gmtGrid_->header->y_units);
  zUnits_ = (const char *)strdup(gmtGrid_->header->z_units);  

  PJ *proj = nullptr;
  PJ_CONTEXT *projContext = nullptr;
  bool convertToUTM = !fileInUTM();
  // If x and y are not in UTM, must convert them to UTM
  // Set up PROJ software first...

  if (convertToUTM) {
    std::cout << "file projection is not UTM" << std::endl;

    // Get UTM zone of grid's W edge
    int utmZone = ((gmtGrid_->header->wesn[0] + 180)/6 + 0.5);

    PJ_CONTEXT *projContext = proj_context_create();
    const char *srcCRS = "EPSG:4326";
    char targCRS[64];
    sprintf(targCRS, "+proj=utm +zone=%d +datum=WGS84", utmZone); 
    proj = proj_create_crs_to_crs (projContext,
                                       srcCRS,
                                       targCRS,
                                       nullptr);
    if (!proj) {
      vtkErrorMacro(<< "failed to create proj");
      SetErrorCode(vtkErrorCode::UserError);
      return 0;      
    }

    // Ensure proper coordinate order
    PJ* normProj = proj_normalize_for_visualization(projContext,
                                                    proj);
      proj_destroy(proj);
    if (!normProj) {
      vtkErrorMacro(<< "failed to create norm proj");
      SetErrorCode(vtkErrorCode::UserError);
      return 0;      
    }

    proj = normProj;
  }
  
  // Reset/clear points
  swathPoints_->Reset();

  unsigned nRows = gmtGrid_->header->n_rows;
  unsigned nCols = gmtGrid_->header->n_columns;
  
  // Pre-allocate points memory
  if (!swathPoints_->Allocate(nRows * nCols)) {
    std::cerr << "failed to allocat "
	      <<  nRows * nCols << " points"
	      << std::endl;
  }

  // Load points read from grid file
  std::cerr << "GmtGridReader::RequestData() - load points" << std::endl;    
  for (unsigned row = 0; row < gmtGrid_->header->n_rows; row++) {
    for (unsigned col = 0; col < gmtGrid_->header->n_columns; col++) {
      unsigned dataIndex = GMT_Get_Index(gmtApi, gmtGrid_->header, row, col);

      if (!convertToUTM) {
        vtkIdType id = swathPoints_->InsertNextPoint(gmtGrid_->x[col],
                                                    gmtGrid_->y[row],
                                                    gmtGrid_->data[dataIndex]);
      }
      else {
        // Convert lat/lon to UTM
        PJ_COORD latLon = proj_coord(gmtGrid_->x[col],
                                     gmtGrid_->y[row],
                                     0, 0);

        PJ_COORD utm = proj_trans(proj, PJ_FWD, latLon);
        vtkIdType id = swathPoints_->InsertNextPoint(utm.enu.e,
                                                    utm.enu.n,
                                                    gmtGrid_->data[dataIndex]);       }
    }
  }

  // Clean up proj UTM conversion stuff
  if (convertToUTM) {
    proj_destroy(proj);
    proj_context_destroy(projContext);
  }

  
  // DEBUG DEBUG DEBUG
  int row = 0;
  int col = 0;
  unsigned dataIndex = GMT_Get_Index(gmtApi, gmtGrid_->header, row, col);
  fprintf(stderr, "NW - x[%d]: %f, y[%d]: %f, data[%d]: %f\n",
          col, gmtGrid_->x[col], row, gmtGrid_->y[row],
          dataIndex, gmtGrid_->data[dataIndex]);
          

  row = gmtGrid_->header->n_rows-1;
  col = 0;
  dataIndex = GMT_Get_Index(gmtApi, gmtGrid_->header, row, col);
  fprintf(stderr, "SW - x[%d]: %f, y[%d]: %f, data[%d]: %f\n",
          col, gmtGrid_->x[col], row, gmtGrid_->y[row],
          dataIndex, gmtGrid_->data[dataIndex]);

  row = 0;
  col = gmtGrid_->header->n_columns - 1;
  dataIndex = GMT_Get_Index(gmtApi, gmtGrid_->header, row, col);
  fprintf(stderr, "NE - x[%d]: %f, y[%d]: %f, data[%d]: %f\n",
          col, gmtGrid_->x[col], row, gmtGrid_->y[row],
          dataIndex, gmtGrid_->data[dataIndex]);  

  row = gmtGrid_->header->n_rows - 1;
  col = gmtGrid_->header->n_columns - 1;
  dataIndex = GMT_Get_Index(gmtApi, gmtGrid_->header, row, col);
  fprintf(stderr, "SE - x[%d]: %f, y[%d]: %f, data[%d]: %f\n",
          col, gmtGrid_->x[col], row, gmtGrid_->y[row],
          dataIndex, gmtGrid_->data[dataIndex]);  
  
  // Set triangle vertices
  if (!swathPolygons_->Allocate(nRows * nCols * 2)) {
    std::cerr << "failed to allocat "
	      <<  nRows * nCols *2 << " polygons"
	      << std::endl;
  }

  vtkIdType triangleVertexId[3];
  int nTriangles = 0;
  // Triangles must stay within row and column bounds
  for (unsigned row = 0; row < nRows-1; row++) {
    for (unsigned col = 0; col < nCols-1; col++) {

      // First triangle
      triangleVertexId[0] = gridOffset(nRows, nCols, row, col);
      triangleVertexId[1] = gridOffset(nRows, nCols, row, col+1);
      triangleVertexId[2] = gridOffset(nRows, nCols, row+1, col+1);      
      swathPolygons_->InsertNextCell(3, triangleVertexId);
      
      // Second triangle
      triangleVertexId[0] = gridOffset(nRows, nCols, row, col);
      triangleVertexId[1] = gridOffset(nRows, nCols, row+1, col+1);
      triangleVertexId[2] = gridOffset(nRows, nCols, row+1, col);      
      swathPolygons_->InsertNextCell(3, triangleVertexId);

      nTriangles++;
    }
  }

  std::cout << "nTriangles=" << nTriangles << std::endl;
  // Save to object's points and polygons
  polyOutput->SetPoints(swathPoints_);
  polyOutput->SetPolys(swathPolygons_);  
  std::cerr << "GmtGridReader::RequestData() - done" << std::endl;
  *** */
  
  return 1;
}


bool SwathReader::readSwathFile(const char *swathFile) {
  
  std::cout << "readSwathFile(): swathFile:" << swathFile << std::endl;

  int verbose = 1;
  int error = 0;
  
  // Call mbeditviz functions to read data from file
  int sonarFormat = 0;
  if (mb_get_format(verbose, (char *)swathFile, NULL, &sonarFormat, &error) !=
      MB_SUCCESS) {
    std::cerr << "Couldn't determine format of " << swathFile << std::endl;
    return false;
  }
  
  // Get list of relevant files
  if (mbeditviz_import_file((char *)swathFile, sonarFormat) != MB_SUCCESS) {
    std::cerr << "Couldn't import data from " << swathFile << std::endl;
    return false;
  }

  // Read swath data from first file into global structures  
  if (mbeditviz_load_file(0) != MB_SUCCESS) {
    std::cerr << "Couldn't load data from " << swathFile << std::endl;
    return false;
  }

  // Need to unlock file when done reading
  std::cout << "Unlock swath file" << std::endl;
  unlockSwath((char *)swathFile);

  
  // Point to swath data just loaded
  struct mbev_file_struct *swathData = &mbev_files[0];
  
  // Transfer swath data from global mbev_files to VTK structure
  swathPoints_->Initialize();
  bool pointsAllocated = false;
  int nRec = 0;
  int nPoints = 0;
  bool done = false;

  std::cout << "transfer data to vtk structure not yet implemented"
            << std::endl;
  
  /* ***
  while (!done) {

    if (!pointsAllocated) {
      // Working on first scan - Allocate initial row of VTK points
      if (!swathPoints_->Allocate(nPoints)) {
        std::cerr << "Error allocating VTK points" << std::endl;
        break;
      }
      
      pointsAllocated = true;
    }
    else {
      // Allocate another row of VTK points
      if (!swathPoints_->Resize(nPoints)) {
        std::cerr << "Error resizing VTK points" << std::endl;        
      }
    }

    // Add this bathymetry swath to VTK points
    for (int i = 0; i < nBath; i++) {
      
      // Convert lat/lon to utm
      double northing, easting;
      mb_proj_forward(mbVerbose, projPtr, bathymetryLon_[i], bathymetryLat_[i],
                      &easting, &northing, &error);

      bool verbose = false;
      if (verbose) {
        std::cout << "lat: " << bathymetryLat_[i] << " lon: " <<
          bathymetryLon_[i] << " easting: " << easting << " northing: " <<
          northing << std::endl;
      }
      
      swathPoints_->InsertNextPoint(easting, northing, bathymetry_[i]);
    }
    
    nRec++;
  }

  mb_proj_free(mbVerbose, &projPtr, &error);
  
  double xMin, xMax, yMin, yMax, zMin, zMax;
  bounds(&xMin, &xMax, &yMin, &yMax, &zMin, &zMax);

  std::cout << "xMin: " << xMin << ", xMax: " << xMax << std::endl;
  std::cout << "yMin: " << yMin << ", yMax: " << yMax << std::endl;
  std::cout << "zMin: " << zMin << ", zMax: " << zMax << std::endl;

  std::cout << "nRec: " << nRec << ", nPoints: " << nPoints << std::endl;

  // Deallocate registered arrays, release resources
  std::cout << "call mb_close()" << std::endl;
  mb_close(mbVerbose, &mbioPtr_, &error);

  *** */
  
  // Success
  return true;
}


void SwathReader::SetFileName(const char *fileName) {
  if (fileName_) {
    free((void *)fileName_);
  }
  fileName_ = strdup((char *)fileName);

}


void SwathReader::SelectionModifiedCallback(vtkObject*, unsigned long,
                                            void* clientdata, void*)
{
  static_cast<SwathReader*>(clientdata)->Modified();
}


void SwathReader::zBounds(double *zMin, double *zMax) {
  *zMin = zMin_;
  *zMax = zMax_;
}



void SwathReader::bounds(double *xMin, double *xMax,
                         double *yMin, double *yMax,
                         double *zMin, double *zMax) {
  *xMin = lonMin_;
  *xMax = lonMax_;
  *yMin = latMin_;
  *yMax = latMax_;
  *zMin = zMin_;
  *zMax = zMax_;
}


void mb_system::SwathReader::unlockSwath(char *swathfile) {
  bool usingLocks = true;
 std:cout << "unlockSwath(" << swathfile << ")" << std::endl;
  if (usingLocks) {
    int lockError = 0;

    mb_pr_unlockswathfile(mbev_verbose, swathfile,
                          MBP_LOCK_EDITBATHY, appName_, &lockError);
    }
}

