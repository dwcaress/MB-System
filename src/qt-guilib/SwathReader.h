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

  /**
     SwathReader reads raw data stored in a file format that is supported
     by MB-System, and outputs the data into a vtkPoints (vertices) and 
     vtkCellArray (triangles) where data can be accessed by the VTK pipeline.
  */
  class SwathReader : public vtkAbstractPolyDataReader {

  public:

    vtkTypeMacro(SwathReader,vtkAbstractPolyDataReader);

    // TEST TEST TEST
    void dummy();
    
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

    /// Get span of z values
    void zBounds(double *zMin, ///< [out] minimum z value
                 double *zMax  ///< [out] maximum z value
                 );
    
    /// Get span of x, y, and z values
    void bounds(double *xMin, double *xMax, double *yMin, double *yMax,
		double *zMin, double *zMax);

    /// Read data from file, load into vtk geometry.
    /// @return false on error, else true
    bool readSwathFile(const char *file ///<[in] swath data file
                       );

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

    /// Format code of latest swath data file read
    int swathFormat_;
    
    vtkSmartPointer<vtkPoints> points_;
    vtkSmartPointer<vtkCellArray> polygons_;  

    /// Pointer to MBIO input/output control structure
    void *mbioPtr_;

    /** @name Arrays allocated and filled by mbio library functions
        NOTE: Important to initialize these pointers to NULL before
        calling registerArrays(). These arrays will be deallocated when
        mb_close() is called. */
    ///@{
    
    /// Beam quality flags
    char *beamFlags_;
    /// Bathymetry array
    double *bathymetry_;

    /// Sidescan array
    double *sideScan_;

    /// Latitudes corresponding to each bathymetry point
    double *bathymetryLat_;

    /// Longitudes corresponding to each bathymetry point
    double *bathymetryLon_;

    /// Latitudes corresponding to each sideScan point
    double *sideScanLat_;

    /// Longitudes corresponding to each sideScan point
    double *sideScanLon_;

    /// Amplitudes at each point
    double *amplitude_;

    ///@}
    
    /// Minimum latitude value in dataset
    double latMin_;

    /// Maximum latitude value in dataset
    double latMax_;

    /// Minimum longitude value in dataset
    double lonMin_;

    /// Maximum longitude value in dataset
    double lonMax_;
    
    /// Minimum bathymetry value in dataset
    double zMin_;
    
    /// Maximum bathymetry value in dataset
    double zMax_;

    /// Return mnemonic for specified record type
    static const char *recordTypeMnem(int type) {
      switch (type) {
      case 1:
        return "survey";
        break;

      case 2:
        return "comment";
        break;

      case 12:
        return "nav";
        break;
        
      default:
        return "unknown";
        
      }
    }
    
  private:

    // Private constructor and destructor

    /// Normally PRIVATE, but TESTING HERE
    /// Constructor - publicly accessed with New()
    SwathReader();
  
    /// Destructor - should be protected, accessed with Delete()
    ~SwathReader(); ///// TEST TEST TESToverride;

    SwathReader(const SwathReader&) = delete;
    void operator=(const SwathReader&) = delete;
  

  }; 
};

#endif



