#include <stdio.h>
#include <string>
#include <sstream>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include "vtkDataSet.h"
#include "vtkErrorCode.h"
#include "vtkSetGet.h"
#include "vtkCallbackCommand.h"
#include "vtk_netcdf.h"
#include "SwathReader.h"

#include "mb_format.h"

#define VTK_CREATE(type, name) vtkSmartPointer<type> name = vtkSmartPointer<type>::New()


using namespace mb_system;

SwathReader::SwathReader() :
  fileName_(nullptr) {
  swathFormat_ = MB_SYS_NONE;
  mbioPtr_ = nullptr;
  
  points_ = vtkSmartPointer<vtkPoints>::New();
  points_->SetDataTypeToFloat();
  polygons_ = vtkSmartPointer<vtkCellArray>::New();  

  this->SetNumberOfInputPorts(0);

  VTK_CREATE(vtkCallbackCommand, cbc);
  cbc->SetCallback(&SwathReader::SelectionModifiedCallback); // what's this?
  cbc->SetClientData(this);  // what's this?
}

SwathReader::~SwathReader() {
}




int SwathReader::RequestData(vtkInformation* request,
                             vtkInformationVector** inputVector,
                             vtkInformationVector* outputVector) {

  std::cerr << "SwathReader::RequestData()" << std::endl;
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

  std::cerr << "PREMATURE END to SwathReader::requestData()" << std::endl;
  return 0;  ///// TEST TEST TEST
  /* ***
  // Read grid file
  std::cerr << "SwathReader::RequestData() - readGridFile()" << std::endl;  
  void *gmtApi;
  gmtGrid_ = readGridFile(fileName_, &gmtApi);
  if (!gmtGrid_) {
  std::cerr << "error while reading " << fileName_ << std::endl;
  SetErrorCode(vtkErrorCode::CannotOpenFileError);
  return 0;
  }

  // Reset/clear points
  points_->Reset();

  unsigned nRows = gmtGrid_->header->n_rows;
  unsigned nCols = gmtGrid_->header->n_columns;
  
  // Pre-allocate points memory
  if (!points_->Allocate(nRows * nCols)) {
  std::cerr << "failed to allocat "
  <<  nRows * nCols << " points"
  << std::endl;
  }

  // Load points read from grid file
  std::cerr << "SwathReader::RequestData() - load points" << std::endl;    
  for (unsigned row = 0; row < gmtGrid_->header->n_rows; row++) {
  for (unsigned col = 0; col < gmtGrid_->header->n_columns; col++) {
  unsigned dataIndex = GMT_Get_Index(gmtApi, gmtGrid_->header, row, col);
  points_->InsertNextPoint(row, col,
  gmtGrid_->data[dataIndex]);
  }
  }

  // Set triangle vertices
  if (!polygons_->Allocate(nRows * nCols * 2)) {
  std::cerr << "failed to allocat "
  <<  nRows * nCols *2 << " polygons"
  << std::endl;
  }    
  vtkIdType triangleVertexId[3];
  int nCells = 0;
  // Triangles must stay within row and column bounds
  for (unsigned row = 0; row < nRows-1; row++) {
  for (unsigned col = 0; col < nCols-1; col++) {

  // First triangle
  triangleVertexId[0] = gridOffset(nRows, nCols, row, col);
  triangleVertexId[1] = gridOffset(nRows, nCols, row, col+1);
  triangleVertexId[2] = gridOffset(nRows, nCols, row+1, col+1);      
  polygons_->InsertNextCell(3, triangleVertexId);
  nCells++;
      
  // Second triangle
  triangleVertexId[0] = gridOffset(nRows, nCols, row, col);
  triangleVertexId[1] = gridOffset(nRows, nCols, row+1, col+1);
  triangleVertexId[2] = gridOffset(nRows, nCols, row+1, col);      
  polygons_->InsertNextCell(3, triangleVertexId);
  nCells++;
  }
  }
  std::cout << "nCells=" << nCells << std::endl;

  polyOutput->SetPoints(points_);
  // std::cerr << "SKIPPING POLYGONS FOR NOW" << std::endl;
  polyOutput->SetPolys(polygons_);  
  std::cerr << "SwathReader::RequestData() - done" << std::endl;
  return 1;
  *** */
}


bool SwathReader::readSwathFile(const char *swathFile) {
  
  fprintf(stderr, "readSwathFile(): swathFile: %s\n", swathFile);
  // Check for file existence and readability
  struct stat fileStatus;

  if (stat(swathFile, &fileStatus) != 0
      || (fileStatus.st_mode & S_IFMT) == S_IFDIR
      || fileStatus.st_size <= 0) {
    std::cerr << "Can not read \"" << swathFile << "\"" << std::endl;
    return false;
  }

  fprintf(stderr, "swathFile now: %s\n", swathFile);
  int error;
  int verbose = 1;
    
  int format;
    
  // Determine sonar data format from filename
  std::cout << "call mg_get_format()" << std::endl;
  if (mb_get_format(verbose, (char *)swathFile, nullptr, &format,
                    &error) != MB_SUCCESS) {
    std::cerr << "Can't determine data format of \"" << swathFile << "\""
              << std::endl;      
    return false;
  }

  std::cout << "set variables" << std::endl;
    
  swathFormat_ = format;

  int pings = 1; // No ping averaging
  int lonRange = 0;  // -180 to +180

  // Got these values from mbedit_prog.c
  double areaBounds[] = {-180., 180., -90., 90.};
  int beginTime[] = {1962, 1, 1, 0, 0, 0, 0};
  int endTime[] = {2062, 1, 1, 0, 0, 0, 0};
  double timeGap = 1000000000.0;
  double minSpeed = 1.;
    
  double beginEpochSec, endEpochSec;
  int maxBathBeams, maxAmpBeams, maxSSPixels;

  if (mbioPtr_) {
    // Deallocate previous array memory
    std::cout << "call mg_get_close()" << std::endl;      
    if (mb_close(verbose, &mbioPtr_, &error) != MB_SUCCESS) {
      std::cerr << "mb_close() failed with error " << error 
                << std::endl;

      return false;
    }
    mbioPtr_ = nullptr;
  }
    
  // Initialize read
  std::cout << "call mg_read_init()" << std::endl;    
  if (mb_read_init(verbose, (char *)swathFile, format, pings, lonRange,
                   areaBounds, beginTime, endTime, minSpeed, timeGap,
                   &mbioPtr_, &beginEpochSec, &endEpochSec, &maxBathBeams,
                   &maxAmpBeams, &maxSSPixels, &error) != MB_SUCCESS) {
    std::cerr << "mb_read_init() failed with error " << error 
              << std::endl;

    return false;
  }
    
  // Register/allocate arrays
  double *bathLon, *bathLat, *ampLon, *ampLat, *ssLon, *ssLat;
    
  std::cout << "call registerArray()s" << std::endl;    
  if (!registerArrays(verbose, &error)) {

    std::cerr << "registerArrays() failed with error " << error 
              << std::endl;

    return false;
  }
    
  // Read data
  char comment[MB_COMMENT_MAXLINE];
  int recordType, nBath, nAmp, nSS;
  double lat, lon, speed, heading, distance, altitude, sonarDepth;
  error = MB_ERROR_NO_ERROR;
  while (error <= MB_ERROR_NO_ERROR) {
    int status = mb_read(verbose, mbioPtr_, &recordType, &pings,
                         beginTime, &beginEpochSec, &lon, &lat,
                         &speed, &heading, &distance, &altitude, &sonarDepth,
                         &nBath, &nAmp, &nSS, beamFlags_, bathymetry_,
                         amplitude_, bathymetryLon_, bathymetryLat_,
                         sideScan_, sideScanLon_, sideScanLat_,
                         comment, &error);
  }
  if (error == MB_ERROR_EOF) {
    std::cout << "At EOF" << std::endl;
  }
  else {
    std::cout << "mb_read() ended with error " << error << std::endl;
  }
  
  std::cout << "call mb_close()" << std::endl;
  mb_close(verbose, &mbioPtr_, &error);


  // Success
  return true;
}


void SwathReader::SetFileName(const char *fileName) {
  if (fileName_) {
    free((void *)fileName_);
  }
  fileName_ = strdup((char *)fileName);

}


void SwathReader::SelectionModifiedCallback(vtkObject*, unsigned long, void* clientdata, void*)
{
  static_cast<SwathReader*>(clientdata)->Modified();
}


/* ***
   void SwathReader::zBounds(float *zMin, float *zMax) {
   *zMin = gmtGrid_->header->z_min;
   *zMax = gmtGrid_->header->z_max;
   }


   void SwathReader::bounds(float *xMin, float *xMax,
   float *yMin, float *yMax,
   float *zMin, float *zMax) {
   *xMin = gmtGrid_->header->wesn[0];
   *xMax = gmtGrid_->header->wesn[1];
   *yMin = gmtGrid_->header->wesn[2];
   *yMax = gmtGrid_->header->wesn[3];
   *zMin = gmtGrid_->header->z_min;
   *zMax = gmtGrid_->header->z_max;
   }

   vtkIdType SwathReader::gridOffset(unsigned nRows, unsigned nCols, unsigned row, unsigned col) {
   if (row >= nRows || col >= nCols) { 
   // Out of bounds
   fprintf(stderr, "getGridOffset(): out-of-bounds: row=%d, nRows=%d, col=%d, nCols=%d\n",
   row, nRows, col, nCols);
   }
    
   return (col + row * nCols);
   }

   *** */

bool SwathReader::registerArrays(int verbose, int *error) {
  int status = MB_SUCCESS;

  mb_register_array(verbose, mbioPtr_, MB_MEM_TYPE_BATHYMETRY,
                    sizeof(char), (void **)&beamFlags_, error);
  
  mb_register_array(verbose, mbioPtr_, MB_MEM_TYPE_BATHYMETRY,
                    sizeof(double), (void **)&bathymetry_, error);

  mb_register_array(verbose, mbioPtr_, MB_MEM_TYPE_BATHYMETRY,
                    sizeof(double), (void **)&bathymetryLat_, error);

  mb_register_array(verbose, mbioPtr_, MB_MEM_TYPE_BATHYMETRY,
                    sizeof(double), (void **)&bathymetryLon_, error);    

  mb_register_array(verbose, mbioPtr_, MB_MEM_TYPE_AMPLITUDE,
                    sizeof(double), (void **)&amplitude_, error);  

  mb_register_array(verbose, mbioPtr_, MB_MEM_TYPE_SIDESCAN,
                    sizeof(double), (void **)&sideScan_, error);

  mb_register_array(verbose, mbioPtr_, MB_MEM_TYPE_SIDESCAN,
                    sizeof(double), (void **)&sideScanLat_, error);
  
  mb_register_array(verbose, mbioPtr_, MB_MEM_TYPE_SIDESCAN,
                    sizeof(double), (void **)&sideScanLon_, error);

  if (*error != MB_ERROR_NO_ERROR) {
    std::cerr << "Error registering arrays: error=" << *error << std::endl;
    return false;
  }

  return true;
}
