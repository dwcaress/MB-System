#ifndef SwathReader_h
#define SwathReader_h

#include "vtkAbstractPolyDataReader.h"
#include "vtkPolyData.h"
#include "vtkInformation.h"
#include "vtkInformationVector.h"
#include "vtkSmartPointer.h"
#include "vtkPoints.h"
#include "vtkCellArray.h"

// Need to make phony typdefs for X11 types here as we need to include
// header that includes them in prototype declarations for
// functions we do not use
typedef void *  Widget;
typedef void *  XtAppContext;
typedef void *  XtPointer;
typedef void *  String;

// Other types used in mbeditviz
typedef bool Boolean;

extern "C" {
#include "mb_define.h"
#include "mb_io.h"
// Defines extern variables used by mbeditviz functions
#include "mbeditviz.h"   
}

namespace mb_system {

  /**
     SwathReader reads raw data stored in a file format that is supported
     by MB-System, and outputs the data into a vtkPoints (vertices) and 
     vtkCellArray (triangles) where data can be accessed by the VTK pipeline.
     Integrates types, functions and variables from mb_define, mb_io, and 
     mbeditviz 'C' modules
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
    vtkPoints *swathPoints() { return swathPoints_; }

    /// Get span of z values
    void zBounds(double *zMin, ///< [out] minimum z value
                 double *zMax  ///< [out] maximum z value
                 );
    
    /// Get span of x, y, and z values
    void bounds(double *xMin, double *xMax, double *yMin, double *yMax,
		double *zMin, double *zMax);

    /// Read data from file, load into vtk geometry.
    /// @return false on error, else true
    bool readSwathFile(const char *file);

    /// Invoked by mbeditviz_prog C functions
    static int showMessage(char *msg) {
      std::cout << "showMessage(): " << msg << std::endl;
      return 0;
    }

    /// Invoked by mbeditviz_prog C functions    
    static void hideMessage() {
      std::cout << "hideMessage() " << std::endl;
    }

    /// Invoked by mbeditviz_prog C functions    
    static void updateGui() {
      std::cout << "updateGui() " << std::endl;      
    }

    /// Invoked by mbeditviz_prog C functions    
    static int showErrorDialog(char *s1, char *s2, char *s3) {
      std::cout << "showErrorDialog():\n" << s1 << "\n"
                << s2 << "\n" << s3
                << std::endl;

      return 0;
    }

    /// Unlock specified swath file
    void unlockSwath(char *swathFile);
    
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


    /// x-axis units
    const char *xUnits_;

    /// y-axis units
    const char *yUnits_;

    /// z-axis units
    const char *zUnits_;

    /// swath VTK points
    vtkSmartPointer<vtkPoints> swathPoints_;

    /// swath VTK polygons
    vtkSmartPointer<vtkCellArray> swathPolygons_;  

    /// Application name - must provide this to 
    /// mbeditviz_init() function, used in locking/unlocking files
    char *appName_;
    
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

    /// Constructor - publicly accessed with New()
    SwathReader();
  
    /// Destructor - should be protected, accessed with Delete()
    ~SwathReader();        ///// TEST TEST TESToverride;

    SwathReader(const SwathReader&) = delete;
    void operator=(const SwathReader&) = delete;
  

  }; 
};

#endif



