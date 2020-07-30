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
     GmtGrideader reads data stored in a GMT grid file (NetCDF format), and outputs the data into
     a vtkDataSet.
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
    virtual void SetFileName(const char *fileName);

    /// Return pointer to gridPoints
    vtkPoints *gridPoints() { return gridPoints_; }

    /// Get span of z values
    void zBounds(float *zMin, float *zMax);

    /// Get span of x, y, and z values
    void bounds(float *xMin, float *xMax, float *yMin, float *yMax,
		float *zMin, float *zMax);
    
  protected:
  
    /// Callback registered with the VariableArraySelection.
    static void SelectionModifiedCallback(
					  vtkObject* caller, unsigned long eid, void* clientdata, void* calldata);


    /// Get offset from start of data grid.
    /// It's an error if row or col are out-of-range
    vtkIdType gridOffset(unsigned nRows, unsigned nCols, unsigned row, unsigned col);

  
    /// Load data from source into vtkDataSet
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



