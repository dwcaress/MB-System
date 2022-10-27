#ifndef TopoGridReader_h
#define TopoGridReader_h

#include "vtkAbstractPolyDataReader.h"
#include "vtkPolyData.h"
#include "vtkInformation.h"
#include "vtkInformationVector.h"
#include "vtkSmartPointer.h"
#include "vtkPoints.h"
#include "vtkCellArray.h"
#include "gmt.h"
#include "TopoGridData.h"

namespace mb_system {

  enum TopoGridType {Unknown,
                      GMTGrid,
                      SwathGrid};

  
  /**
     TopoGridReader reads data stored in a data file 
     and outputs the data into a vtkPoints (vertices) and vtkCellArray 
     (triangles) where data can be accessed by the VTK pipeline.
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
    virtual void SetFileName(
                             const char *fileName ///< [in] grid file name
                             );

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

    /// Return true if corresponding file stores x-y values as UTM
    bool fileInUTM();

    /// Set grid type
    void setGridType(TopoGridType gridType) {
      gridType_ = gridType;
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
    /// errors by calling vtkAlgorithm::GetErrorCode().
    int RequestData(vtkInformation* request, vtkInformationVector** inputVector,
		    vtkInformationVector* outputVector) override;
  
    /// Read data from file into GridData. Returns nullptr on error
    static TopoGridData *readGridFile(const char *file);

    /// Name of grid file
    char *fileName_;

    /// Grid type to read
    TopoGridType gridType_;

    /// Topometry grid data object
    TopoGridData *grid_;
  
    vtkSmartPointer<vtkPoints> gridPoints_;
    vtkSmartPointer<vtkCellArray> gridPolygons_;

    char *xUnits_;
    char *yUnits_;
    char *zUnits_;


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



