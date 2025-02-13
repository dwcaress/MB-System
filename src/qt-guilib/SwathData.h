#ifndef SwathData_h
#define SwathData_h
#include "TopoData.h"

/// Forward declaration
struct mbev_grid_struct;

namespace mb_system {
  /**
     SwathData encapsulates data read from a swath file in any format supported
     by MB-System.
  */
  class SwathData : public TopoData {

  public:

    /// Constructor
    SwathData();

    /// Destructor
    ~SwathData();
    
    /// Get x, y, z data at specified grid row and column.
    /// Returns false in case of error
    bool data(int row, int col, double *x, double *y,
              double *z) override;
  
    /// Read data from swath file
    bool readDatafile(char *filename) override;

    /// Invoked by mbeditviz_prog C functions
    static int showMessage(char *msg) {
      std::cout << "showMessage(): " << msg << std::endl;
      return 0;
    }

    /// Invoked by mbeditviz_prog C functions    
    static int hideMessage() {
      std::cout << "hideMessage() " << std::endl;
      return 0;
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


    /// Set a proj-string corresponding to the data's coordinate
    /// reference system. 
    /// Returns true on success, false on error
    bool setProjString() override;
    
    
  protected:

    /// Base class calls this to get class parameter variables.
    /// Note: xUnits, yUnits and zUnits memory should be unallocated
    /// before calling this function to prevent memory leak.
    void getParameters(int *nRows, int *nColumns,
                       double *xMin, double *xMax,
                       double *yMin, double *yMax,
                       double *zMin, double *zMax,
                       char **xUnits, char **yUnits, char **zUnits) override;


    /// Unlock specified swath file
    void unlockSwath(char *swathFile);

    /// Swath data read from file, old mb-system struct
    struct mbev_grid_struct *gridData_;

    char *appName_;
  };

};

#endif
