#ifndef GmtGridReader_h
#define GmtGridReader_h

#include <gmt/gmt.h>

#include "vtkAbstractPolyDataReader.h"
#include "vtkPolyData.h"
#include "vtkInformation.h"
 #include "vtkInformationVector.h"
#include "vtkSmartPointer.h"
#include "vtkPoints.h"
#include "vtkCellArray.h"

namespace mb_system {
  /**
     GmtGridReader reads data stored in a GMT grid file (NetCDF format), 
     and outputs the data into a vtkPoints (vertices) and vtkCellArray 
     (triangles) where data can be accessed by the VTK pipeline.
  */
  class GmtGridReader : public vtkAbstractPolyDataReader {

  public:

    vtkTypeMacro(GmtGridReader,vtkAbstractPolyDataReader);

    /// Get a new GmtGridReader object
    /// For use with vtkSmartPointer
    static GmtGridReader *New() {
      return new GmtGridReader();
    }
  
    /// Set grid file name
    virtual void SetFileName(
                             const char *fileName ///< [in] grid file name
                             );

    /// Return pointer to gridPoints
    vtkPoints *gridPoints() { return gridPoints_; }

    /// Get span of z values
    void zBounds(double *zMin, ///< [out] minimum z value
                 double *zMax  ///< [out] maximum z value
                 );

    /// Get span of x, y, and z values in stored grid
    void gridBounds(double *xMin, double *xMax, double *yMin, double *yMax,
		double *zMin, double *zMax);

    /// Get span of x, y, and z values in stored grid;
    /// saved in order xmin,xmax,ymin,ymax,zmin,zmax
    void gridBounds(double *bounds);

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
  
    /// Read data from file into GMT_GRID. Returns nullptr on error
    static GMT_GRID *readGridFile(const char *file, void **gmtApi);

    /// Name of grid file
    char *fileName_;

    /// GMT grid
    GMT_GRID *gmtGrid_;
  
    vtkSmartPointer<vtkPoints> gridPoints_;
    vtkSmartPointer<vtkCellArray> gridPolygons_;

    const char *xUnits_;
    const char *yUnits_;
    const char *zUnits_;


  private:
  
    /// Constructor - publicly accessed with New()
    GmtGridReader();
  
    /// Destructor - should be protected, accessed with Delete()
    ~GmtGridReader() override;


    GmtGridReader(const GmtGridReader&) = delete;
    void operator=(const GmtGridReader&) = delete;
  };
}

#endif



