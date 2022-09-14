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

extern "C" {
#include "mb_format.h"
}

#define VTK_CREATE(type, name) vtkSmartPointer<type> name = vtkSmartPointer<type>::New()

using namespace mb_system;

SwathReader::SwathReader() :
  fileName_(nullptr) {

  std::cout << "SwathReader ctr" << std::endl;
  swathFormat_ = MB_SYS_NONE;
  mbioPtr_ = nullptr;
  beamFlags_ = nullptr;
  bathymetry_ = bathymetryLat_ = bathymetryLon_ = nullptr;
  sideScan_ = sideScanLat_ = sideScanLon_ = nullptr;
  amplitude_ = nullptr;

  points_ = vtkSmartPointer<vtkPoints>::New();
  points_->SetDataTypeToDouble();
  polygons_ = vtkSmartPointer<vtkCellArray>::New();  

  this->SetNumberOfInputPorts(0);

  VTK_CREATE(vtkCallbackCommand, cbc);
  cbc->SetCallback(&SwathReader::SelectionModifiedCallback); // what's this?
  cbc->SetClientData(this);  // what's this?
}

SwathReader::~SwathReader() {
  std::cout << "SwathReader destructor" << std::endl;
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

  std::cout << "call readSwathData() with fileName " << fileName_ << std::endl;
  readSwathFile(fileName_);

  polyOutput->SetPoints(points_);

  return 1;
}


void SwathReader::dummy() {
  fprintf(stderr, "We are in dummy() now\n");
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
    
  int swathFormat;
    
  // Determine sonar data format from filename
  std::cout << "call mg_get_format()" << std::endl;
  if (mb_get_format(verbose, (char *)swathFile, nullptr, &swathFormat,
                    &error) != MB_SUCCESS) {
    std::cerr << "Can't determine data format of \"" << swathFile << "\""
              << std::endl;      
    return false;
  }

  swathFormat_ = swathFormat;

  int pings = 1; // No ping averaging
  int lonRange = 0;  // -180 to +180

  // Got these initial values from mbedit_prog.c
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

  // Initialize read based on metadata/data in file
  std::cout << "call mg_read_init()" << std::endl;    
  if (mb_read_init(verbose, (char *)swathFile, swathFormat, pings, lonRange,
                   areaBounds, beginTime, endTime, minSpeed, timeGap,
                   &mbioPtr_, &beginEpochSec, &endEpochSec, &maxBathBeams,
                   &maxAmpBeams, &maxSSPixels, &error) != MB_SUCCESS) {
    std::cerr << "mb_read_init() failed with error " << error 
              << std::endl;

    return false;
  }

  // Access mbioPtr_ structure members
  mb_io_struct *mbioStruct = (mb_io_struct *)mbioPtr_;
  
  // Register/allocate arrays
  std::cout << "call registerArray()s" << std::endl;    
  if (!registerArrays(verbose, &error)) {

    std::cerr << "registerArrays() failed with error " << error 
              << std::endl;

    return false;
  }

  std::cout << "beams_bath_alloc: " << mbioStruct->beams_bath_alloc
            << std::endl;
  
  zMin_ = std::numeric_limits<double>::max();
  zMax_ = std::numeric_limits<double>::lowest();
  latMin_ = std::numeric_limits<double>::max();
  latMax_ = std::numeric_limits<double>::lowest();
  lonMin_ = std::numeric_limits<double>::max();
  lonMax_ = std::numeric_limits<double>::lowest();
  
  // Read data
  char comment[MB_COMMENT_MAXLINE];
  int recordType, nBath, nAmp, nSS;
  double lat, lon, speed, heading, distance, altitude, sonarDepth;

  points_->Initialize();
  bool pointsAllocated = false;
  error = MB_ERROR_NO_ERROR;
  int nRec = 0;
  int nPoints = 0;

  // Initialize proj structure; needed to convert lat/lon to utm
  char projection[1024] = "Geographic";
  void *projPtr = nullptr;
  if (mb_proj_init(verbose, projection, &projPtr, &error) != MB_SUCCESS) {
    std::cerr << "mb_proj_init() failed" << std::endl;
    return false;
  }
  else {
    std::cerr << "mb_proj_init() OK" << std::endl;    
  }
  
  error = MB_ERROR_NO_ERROR;  
  while (error <= MB_ERROR_NO_ERROR) {
    int status = mb_read(verbose, mbioPtr_, &recordType, &pings,
                         beginTime, &beginEpochSec, &lon, &lat,
                         &speed, &heading, &distance, &altitude, &sonarDepth,
                         &nBath, &nAmp, &nSS, beamFlags_, bathymetry_,
                         amplitude_, bathymetryLon_, bathymetryLat_,
                         sideScan_, sideScanLon_, sideScanLat_,
                         comment, &error);


    if (error == MB_ERROR_EOF) {
      std::cout << "At EOF" << std::endl;
      break;
    }
    else if (error != MB_ERROR_NO_ERROR) {
      std::cout << "mb_read(): error " << error << std::endl;
      continue;
    }

    /* ***
    if (verbose > 0) {
      std::cout << "recordType: " << recordType << " (" <<
        recordTypeMnem(recordType) << ") nBath: " << nBath << std::endl;
      
      std::cout << "bathymetryLon_[0]: " << bathymetryLon_[0] << std::endl;
    }
    *** */
    
    if (recordType == MB_DATA_DATA) {
      // Survey data record
      nPoints += nBath;
      // Determine dataset geometric bounds
      for (int i = 0; i < nBath; i++) {

        if (bathymetryLat_[i] < latMin_) {
          latMin_ = bathymetryLat_[i];
          std::cout << "new latMin: " << latMin_ << std::endl;
        }
        
        if (bathymetryLat_[i] > latMax_) {
          latMax_ = bathymetryLat_[i];
          std::cout << "new latMax: " << latMax_ << std::endl;          
        }

        if (bathymetryLon_[i] < lonMin_) {
          lonMin_ = bathymetryLon_[i];
          std::cout << "new lonMin: " << lonMin_ << std::endl;
        }
        
        if (bathymetryLon_[i] > lonMax_) {
          lonMax_ = bathymetryLon_[i];
          std::cout << "new lonMax: " << lonMax_ << std::endl;          
        }        
        
        if (bathymetry_[i] < zMin_) {
          zMin_ = bathymetry_[i];
          std::cout << "new zMin: " << zMin_ << std::endl;
        }
        
        if (bathymetry_[i] > zMax_) {
          zMax_ = bathymetry_[i];
          std::cout << "new zMax: " << zMax_ << std::endl;          
        }
      }
    }

    if (!pointsAllocated) {
      // Working on first scan - Allocate initial row of VTK points
      if (!points_->Allocate(nPoints)) {
        std::cerr << "Error allocating VTK points" << std::endl;
        break;
      }
      
      pointsAllocated = true;
    }
    else {
      // Allocate another row of VTK points
      if (!points_->Resize(nPoints)) {
        std::cerr << "Error resizing VTK points" << std::endl;        
      }
    }

    // Add this bathymetry swath to VTK points
    for (int i = 0; i < nBath; i++) {
      
      // Convert lat/lon to utm
      double northing, easting;
      mb_proj_forward(verbose, projPtr, bathymetryLon_[i], bathymetryLat_[i],
                      &easting, &northing, &error);

      if (verbose) {
        std::cout << "lat: " << bathymetryLat_[i] << " lon: " <<
          bathymetryLon_[i] << " easting: " << easting << " northing: " <<
          northing << std::endl;
      }
      
      points_->InsertNextPoint(easting, northing, bathymetry_[i]);
    }
    
    nRec++;
  }

  mb_proj_free(verbose, &projPtr, &error);
  
  double xMin, xMax, yMin, yMax, zMin, zMax;
  bounds(&xMin, &xMax, &yMin, &yMax, &zMin, &zMax);

  std::cout << "xMin: " << xMin << ", xMax: " << xMax << std::endl;
  std::cout << "yMin: " << yMin << ", yMax: " << yMax << std::endl;
  std::cout << "zMin: " << zMin << ", zMax: " << zMax << std::endl;

  std::cout << "nRec: " << nRec << ", nPoints: " << nPoints << std::endl;

  // Deallocate registered arrays, release resources
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

/* ***
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

  std::cerr << "register beamFlags" << std::endl;
  mb_register_array(verbose, mbioPtr_, MB_MEM_TYPE_BATHYMETRY,
                    sizeof(char), (void **)&beamFlags_, error);

  std::cerr << "register bathymetry" << std::endl;  
  mb_register_array(verbose, mbioPtr_, MB_MEM_TYPE_BATHYMETRY,
                    sizeof(double), (void **)&bathymetry_, error);

  std::cerr << "register bathymetryLat" << std::endl;  
  mb_register_array(verbose, mbioPtr_, MB_MEM_TYPE_BATHYMETRY,
                    sizeof(double), (void **)&bathymetryLat_, error);

  std::cerr << "register bathymetryLon" << std::endl;    
  mb_register_array(verbose, mbioPtr_, MB_MEM_TYPE_BATHYMETRY,
                    sizeof(double), (void **)&bathymetryLon_, error);    

  std::cerr << "register amplitude" << std::endl;      
  mb_register_array(verbose, mbioPtr_, MB_MEM_TYPE_AMPLITUDE,
                    sizeof(double), (void **)&amplitude_, error);  

  std::cerr << "register sidescan" << std::endl;    
  mb_register_array(verbose, mbioPtr_, MB_MEM_TYPE_SIDESCAN,
                    sizeof(double), (void **)&sideScan_, error);

  std::cerr << "register sidescanLat" << std::endl;
  // fprintf(stderr, "register sideScanLat_ = %p\n", sideScanLat_);
  sideScanLat_ = nullptr;
  mb_register_array(verbose, mbioPtr_, MB_MEM_TYPE_SIDESCAN,
                    sizeof(double), (void **)&sideScanLat_, error);

    std::cerr << "register sidescanLon" << std::endl;      
  mb_register_array(verbose, mbioPtr_, MB_MEM_TYPE_SIDESCAN,
                    sizeof(double), (void **)&sideScanLon_, error);

  if (*error != MB_ERROR_NO_ERROR) {
    std::cerr << "Error registering arrays: error=" << *error << std::endl;
    return false;
  }

  std::cout << "Return from registerArrays(), no errors" << std::endl;
  return true;
}
