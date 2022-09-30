#include "BathyGridData.h"
#include "gmt.h"

/* **
GmtGridData encapsulates data read from a GMT grid file.
 ** */
class GmtGridData : public BathyGridData {

public:

  /// Get x, y, z data at specified row and column.
  /// Returns false in case of error
  bool data(int row, int col, double *x, double *y, double *z) override;
  
  /// Read data from GMT file
  bool readDatafile(char *filename) override;

  
protected:

  /// Base class calls this to get class parameter variables
  /// N.B.: xUnits, yUnits and zUnits memory should be unallocated
  /// before calling this function to prevent memory leak.
  void getParameters(int *nRows, int *nColumns,
                     double *xMin, double *xMax,
                     double *yMin, double *yMax,
                     double *zMin, double *zMax,
                     char **xUnits, char **yUnits, char **zUnits) override;
  
  GMT_GRID *gmtGrid_;

  /// Use this member variable in frequently-called data-fetch functions
  /// for efficiency.
  unsigned dataIndex_;
  
  /// gmtAPI_ is returned when reading a GMT file, and is passed to
  /// various GMT grid access functions.
  void *gmtAPI_;

  /// Read data from GMT file into GMT_GRID. Returns nullptr on error
  static GMT_GRID *readGmtFile(const char *file, void **gmtApi);
  
};
