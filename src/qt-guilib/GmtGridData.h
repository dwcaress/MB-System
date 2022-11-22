#include "TopoGridData.h"
#include "gmt.h"


namespace mb_system {
        
  /* **
     GmtGridData encapsulates data read from a GMT grid file.
     ** */
  class GmtGridData : public TopoGridData {

  public:

    /// Get x, y, z data at specified row and column.
    /// Returns false in case of error
    bool data(int row, int col, double *x, double *y,
              double *z) override;
  
    /// Read data from GMT file
    bool readDatafile(char *filename) override;

    /// Set projString_ member to a valid proj-string corresponding
    /// to data's coordinate reference system.
    /// Returns true on success, false on error
    bool setProjString() override;
    
    
  protected:

    /// Base class calls this to get class parameter variables
    /// N.B.: xUnits, yUnits and zUnits memory should be unallocated
    /// before calling this function to prevent memory leak.
    void getParameters(int *nRows, int *nColumns,
                       double *xMin, double *xMax,
                       double *yMin, double *yMax,
                       double *zMin, double *zMax,
                       char **xUnits, char **yUnits, char **zUnits) override;

    /// GMT data grid
    GMT_GRID *gmtGrid_;

    /// gmtAPI_ is returned when reading a GMT file, and is passed to
    /// various GMT grid access functions.
    void *gmtAPI_;

    /// Read data from GMT file into GMT_GRID. Returns nullptr on error
    static GMT_GRID *readGmtFile(const char *file, void **gmtApi);
  
  };

};
