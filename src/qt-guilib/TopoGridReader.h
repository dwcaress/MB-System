#ifndef TopoGridReader_h
#define TopoGridReader_h

#include "vtkAbstractPolyDataReader.h"
#include "vtkInformation.h"
#include "vtkInformationVector.h"
#include "vtkSmartPointer.h"
#include "vtkPoints.h"
#include "vtkCellArray.h"
#include "gmt.h"
#include <proj.h>
#include "TopoGridData.h"

namespace mb_system {

  /// Supported grid types
  enum TopoGridType {Unknown,
                     GMTGrid,
                     SwathGrid};

  /**
     TopoGridReader reads topgraphy/bathymetry data from a file, which
     can be a GMT grid file or a swath file containing data in any sonar 
     format supported by MB-System. The data is loaded into vtkPoints vertices
     and vtkCellArray triangles, which can be accesed by the VTK visualization
     pipeliine such as the one created by QVtkRenderer.
  */
  class TopoGridReader : public vtkAbstractPolyDataReader {

  public:

    vtkTypeMacro(TopoGridReader,vtkAbstractPolyDataReader);

    /// Get a new TopoGridReader object
    /// For use with vtkSmartPointer
    static TopoGridReader *New() {
      return new TopoGridReader();
    }
  
    /// Set grid file name
    void SetFileName(
		     const char *fileName ///< [in] grid file name
		     ) override;

    /// Return pointer to gridPoints
    vtkPoints *gridPoints() { return gridPoints_; }


    /// Read TopoGridData from file
    TopoGridData *readGridfile(char *filename);
    
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

    /// Return true if encapsulated TopoGridData is in geographic CRS
    bool geographicCRS();
    
    /// Set grid type
    void setGridType(TopoGridType gridType) {
      gridType_ = gridType;
    }

    /// Return file grid type
    static TopoGridType getGridType(const char *filename);

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
  
    /// Read data from file into GridData. Returns nullptr on error
    static TopoGridData *readGridFile(const char *file);

    /// Return true if any of the specified triangle vertex IDs refer to
    /// missing z-values
    bool triangleMissingZValues(vtkIdType *triangleVertices);
    
    /// Name of grid file
    char *fileName_;

    /// Grid type to read
    TopoGridType gridType_;

    /// Topometry grid data object
    TopoGridData *grid_;

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
    TopoGridReader();
  
    /// Destructor - should be protected, accessed with Delete()
    ~TopoGridReader() override;


    TopoGridReader(const TopoGridReader&) = delete;
    void operator=(const TopoGridReader&) = delete;
  };
}


void AprojTest(char *msg);

#endif



