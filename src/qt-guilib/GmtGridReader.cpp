#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include "proj.h"
#include "vtkDataSet.h"
#include "vtkErrorCode.h"
#include "vtkSetGet.h"
#include "vtkCallbackCommand.h"
#include "vtk_netcdf.h"
#include "GmtGridReader.h"

#define VTK_CREATE(type, name) vtkSmartPointer<type> name = vtkSmartPointer<type>::New()

#define UTM_X_NAME "Easting (meters)"
#define UTM_Y_NAME "Northing (meters)"


using namespace mb_system;

GmtGridReader::GmtGridReader() :
  fileName_(nullptr),
  xUnits_(nullptr), yUnits_(nullptr), zUnits_(nullptr)
{
  
  gridPoints_ = vtkSmartPointer<vtkPoints>::New();
  gridPoints_->SetDataTypeToFloat();
  gridPolygons_ = vtkSmartPointer<vtkCellArray>::New();  

  this->SetNumberOfInputPorts(0);

  VTK_CREATE(vtkCallbackCommand, cbc);
  cbc->SetCallback(&GmtGridReader::SelectionModifiedCallback); // what's this?
  cbc->SetClientData(this);  // what's this?
}

GmtGridReader::~GmtGridReader() {
}


int GmtGridReader::RequestData(vtkInformation* request,
			       vtkInformationVector** inputVector,
			       vtkInformationVector* outputVector) {

  std::cerr << "GmtGridReader::RequestData()" << std::endl;
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
  std::cerr << "GmtGridReader::RequestData() - readGridFile()" << std::endl;  
  void *gmtApi;
  gmtGrid_ = readGridFile(fileName_, &gmtApi);
  if (!gmtGrid_) {
    std::cerr << "error while reading " << fileName_ << std::endl;
    SetErrorCode(vtkErrorCode::CannotOpenFileError);
    return 0;
  }

  std::cout << "x units: " << gmtGrid_->header->x_units << std::endl;
  std::cout << "y units: " << gmtGrid_->header->y_units << std::endl;
  std::cout << "z units: " << gmtGrid_->header->z_units << std::endl;

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
  gridPoints_->Reset();

  unsigned nRows = gmtGrid_->header->n_rows;
  unsigned nCols = gmtGrid_->header->n_columns;
  
  // Pre-allocate points memory
  if (!gridPoints_->Allocate(nRows * nCols)) {
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
        vtkIdType id = gridPoints_->InsertNextPoint(gmtGrid_->x[col],
                                                    gmtGrid_->y[row],
                                                    gmtGrid_->data[dataIndex]);
      }
      else {
        // Convert lat/lon to UTM
        PJ_COORD latLon = proj_coord(gmtGrid_->x[col],
                                     gmtGrid_->y[row],
                                     0, 0);

        PJ_COORD utm = proj_trans(proj, PJ_FWD, latLon);
        vtkIdType id = gridPoints_->InsertNextPoint(utm.enu.e,
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
  if (!gridPolygons_->Allocate(nRows * nCols * 2)) {
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
      gridPolygons_->InsertNextCell(3, triangleVertexId);
      
      // Second triangle
      triangleVertexId[0] = gridOffset(nRows, nCols, row, col);
      triangleVertexId[1] = gridOffset(nRows, nCols, row+1, col+1);
      triangleVertexId[2] = gridOffset(nRows, nCols, row+1, col);      
      gridPolygons_->InsertNextCell(3, triangleVertexId);

      nTriangles++;
    }
  }

  std::cout << "nTriangles=" << nTriangles << std::endl;
  // Save to object's points and polygons
  polyOutput->SetPoints(gridPoints_);
  polyOutput->SetPolys(gridPolygons_);  
  std::cerr << "GmtGridReader::RequestData() - done" << std::endl;

  return 1;
}


GMT_GRID *GmtGridReader::readGridFile(const char *gridFile, void **api) {
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


void GmtGridReader::SetFileName(const char *fileName) {
  fprintf(stderr, "In GmtGridReader::SetFileName()\n");
  if (fileName_) {
    fprintf(stderr, "GmtGridReader::SetFileName: free fileName_\n");
    free((void *)fileName_);
  }
  fprintf(stderr, "GmtGridReader::SetFileName: call strdup()\n");
  fileName_ = strdup((char *)fileName);
}


void GmtGridReader::SelectionModifiedCallback(vtkObject*, unsigned long,
                                              void* clientdata, void*)
{
  static_cast<GmtGridReader*>(clientdata)->Modified();
}


void GmtGridReader::zBounds(double *zMin, double *zMax) {
  
  double bounds[6];
  gridPoints_->GetBounds(bounds);
  *zMin = bounds[4];
  *zMax = bounds[5];
}


void GmtGridReader::gridBounds(double *xMin, double *xMax,
                               double *yMin, double *yMax,
                               double *zMin, double *zMax) {
  double bounds[6];
  gridPoints_->GetBounds(bounds);
  *xMin = bounds[0];
  *xMax = bounds[1];
  *yMin = bounds[2];
  *yMax = bounds[3];
  *zMin = bounds[4];
  *zMax = bounds[5];
}


void GmtGridReader::gridBounds(double *bounds) {
  gridPoints_->GetBounds(bounds);
}


vtkIdType GmtGridReader::gridOffset(unsigned nRows, unsigned nCols,
                                    unsigned row, unsigned col) {
  if (row >= nRows || col >= nCols) { 
    // Out of bounds
    fprintf(stderr,
            "getGridOffset(): out-of-bounds: row=%d, nRows=%d, col=%d, nCols=%d\n",
	    row, nRows, col, nCols);
  }

  return  (col + row * nCols);
}



float GmtGridReader::zScaleLatLon(float latRange, float lonRange,
                                  float zRange) {

  float avgLatLonRange = (latRange + lonRange) / 2.;

  return avgLatLonRange / zRange;
}



bool GmtGridReader::fileInUTM() {
  if (!strcmp(xUnits_, UTM_X_NAME) &&
      !strcmp(yUnits_, UTM_Y_NAME)) {
    return true;
  }
  return false;
}

