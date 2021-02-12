#ifndef SwathReader_h
#define SwathReader_h

#include "vtkAbstractPolyDataReader.h"
#include "vtkPolyData.h"
#include "vtkInformation.h"
#include "vtkInformationVector.h"
#include "vtkSmartPointer.h"
#include "vtkPoints.h"
#include "vtkCellArray.h"

#include "mb_define.h"
#include "mb_io.h"

namespace mb_system {

  typedef unsigned char SwathData;
  
  /**
     SwathReader reads raw data stored in a file format that is supported
     by MB-System, and outputs the data into a vtkPoints (vertices) and 
     vtkCellArray (triangles) where data can be accessed by the VTK pipeline.
  */
  class SwathReader : public vtkAbstractPolyDataReader {

  public:

    vtkTypeMacro(SwathReader,vtkAbstractPolyDataReader);

    /// Get a new SwathReader object
    /// For use with vtkSmartPointer
    static SwathReader *New() {
      return new SwathReader();
    }
  
    /// Set swath file name
    virtual void SetFileName(
                             const char *fileName ///< [in] swath file name
                             );

    /// Return pointer to swathPoints
    vtkPoints *swathPoints() { return points_; }

    /** Get span of z values
     */
    void zBounds(float *zMin, ///< [out] minimum z value
                 float *zMax  ///< [out] maximum z value
                 );

    /// Get span of x, y, and z values
    void bounds(float *xMin, float *xMax, float *yMin, float *yMax,
		float *zMin, float *zMax);

    /// Read data from file.
    /// @return Returns nullptr on error, else returns pointer to data
    SwathData *readSwathFile(const char *file);

    
  protected:
  
    /// Callback registered with the VariableArraySelection.
    static void SelectionModifiedCallback(vtkObject* caller, unsigned long eid,
					  void* clientdata, void* calldata);

    /// Get offset from start of data.
    /// It's an error if row or col are out-of-range
    vtkIdType dataOffset(unsigned nRows, unsigned nCols,
                         unsigned row, unsigned col);

  
    /// Load data from source into vtkDataSet. This function *must* call
    /// VtkAlgorithm::SetErrorCode() in case it encounters errors,
    /// so that apps that call vtkPolyDataAlgorithm::Update() can check for
    /// errors by calling vtkAlgorithm::GetErrorCode().
    int RequestData(vtkInformation* request, vtkInformationVector** inputVector,
		    vtkInformationVector* outputVector) override;

    /// Register arrays to be filled and handled by mbio
    /// library functions
    bool registerArrays(int verbose, int *error);
    
    /// Name of data file
    char *fileName_;

    /// Swath data
    SwathData *swathData_;

    /// Format code of latest swath data file read
    int swathFormat_;
    
    vtkSmartPointer<vtkPoints> points_;
    vtkSmartPointer<vtkCellArray> polygons_;  

    /// Pointer to MBIO input/output control structure
    void *mbioPtr_;

    // Arrays allocated and filled by mbio library functions
    
    char *beamFlags_;
    double *bathymetry_;
    double *sideScan_;
    double *bathymetryLat_;
    double *bathymetryLon_;
    double *sideScanLat_;
    double *sideScanLon_;
    double *amplitude_;
    

  private:
  
    /// Constructor - publicly accessed with New()
    SwathReader();
  
    /// Destructor - should be protected, accessed with Delete()
    ~SwathReader() override;


    SwathReader(const SwathReader&) = delete;
    void operator=(const SwathReader&) = delete;
  };
}

#endif



