#ifndef TopoDataReader_h
#define TopoDataReader_h

#include "vtkAbstractPolyDataReader.h"
#include "vtkInformation.h"
#include "vtkInformationVector.h"
#include "vtkSmartPointer.h"
#include "vtkPoints.h"
#include "vtkCellArray.h"
#include "gmt.h"
#include <proj.h>
#include "TopoData.h"

namespace mb_system {

  /// Supported grid types
  enum TopoDataType {Unknown,
                     GMTGrid,
                     Swath };

  /**
     TopoDataReader reads topgraphy/bathymetry data from a file, which
     can be a GMT grid file or a swath file containing data in any sonar 
     format supported by MB-System. The data is loaded into vtkPoints vertices
     and vtkCellArray triangles, which can be accesed by the VTK visualization
     pipeliine such as the one created by QVtkRenderer.
  */
  class TopoDataReader : public vtkAbstractPolyDataReader {

  public:

    vtkTypeMacro(TopoDataReader,vtkAbstractPolyDataReader);

    /// Get a new TopoDataReader object
    /// For use with vtkSmartPointer
    static TopoDataReader *New() {
      return new TopoDataReader();
    }
  
    /// Set grid file name
    void SetFileName(
		     const char *fileName ///< [in] grid file name
		     ) override;

    /// Return pointer to gridPoints
    vtkPoints *gridPoints() { return gridPoints_; }


    /// Read TopoData from file
    TopoData *readDatafile(char *filename);
    
    /// Get span of x, y, and z values in stored grid
    void gridBounds(double *xMin, double *xMax, double *yMin, double *yMax,
                    double *zMin, double *zMax);


    /// Get x-axis units
    const char *xUnits() {
      return xUnits_;
    }

    /// Get y-axis units
    const char *yUnits() {
      return yUnits_;
    }

    /// Get z-axis units
    const char *zUnits() {
      return zUnits_;
    }

    /// Compute z-scale factor based on lat and lon ranges
    static float zScaleLatLon(float latRange, float lonRange,
                              float zRange);

    /// Compute z-scale factor based on lat and lon ranges
    float zScaleLatLon();

    /// Return true if encapsulated TopoData is in geographic CRS
    bool geographicCRS();
    
    /// Set grid type
    void setDataType(TopoDataType dataType) {
      dataType_ = dataType;
    }

    /// Return file grid type
    static TopoDataType getDataType(const char *filename);

    /// Return CRS proj-string of stored grid data
    const char *fileCRS();


    /// PROJ transform between stored and displayed grid data
    PJ *projFileToDisplay() {
      return projTransform_;
    }

    
 protected:
  
    /// Callback registered with the VariableArraySelection.
    static void SelectionModifiedCallback(vtkObject* caller, unsigned long eid,
					  void* clientdata, void* calldata);

    /// Get offset from start of data grid.
    /// It's an error if row or col are out-of-range
    vtkIdType gridOffset(unsigned nRows, unsigned nCols,
                         unsigned row, unsigned col);
  
    /// Load data from source into vtkDataSet. This function *must* call
    /// VtkAlgorithm::SetErrorCode() in case it encounters errors,
    /// so that apps that call vtkPolyDataAlgorithm::Update() can check for
    /// errors by calling vtkAlgorithm::GetErrorCode(). Data is provided as
    /// vtkPoints and vtkCellArray.
    int RequestData(vtkInformation* request, vtkInformationVector** inputVector,
		    vtkInformationVector* outputVector) override;

    /* ***
    /// Read data from file into TopoData. Returns nullptr on error
    static TopoData *readDatafileStatic(const char *file);
    *** */
    
    /// Return true if any of the specified triangle vertex IDs refer to
    /// missing z-values
    bool triangleMissingZValues(vtkIdType *triangleVertices);
    
    /// Name of TopoData file
    char *fileName_;

    /// TopoData type to read
    TopoDataType dataType_;

    /// Topometry grid data object
    TopoData *topoData_;

    /// Grid points
    vtkSmartPointer<vtkPoints> gridPoints_;

    /// Delaunay triangle vertices
    vtkSmartPointer<vtkCellArray> gridPolygons_;

    char *xUnits_;
    char *yUnits_;
    char *zUnits_;

    /// Display CRS proj-string
    char displayCRS_[64];

    /// PROJ context
    PJ_CONTEXT *projContext_;
    
    /// PROJ transformation between stored and displayed CRS
    PJ *projTransform_;
    
  private:

    
    /// Constructor - publicly accessed with New()
    TopoDataReader();
  
    /// Destructor - should be protected, accessed with Delete()
    ~TopoDataReader() override;


    TopoDataReader(const TopoDataReader&) = delete;
    void operator=(const TopoDataReader&) = delete;
  };
}


void AprojTest(char *msg);

#endif



