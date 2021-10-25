#ifndef UTILITIES_H
#define UTILITIES_H

#include <vtkLookupTable.h>


namespace mb_system {

  /// Make lookup table
  void makeLookupTable(int colorScheme, vtkLookupTable *lut);

}



#endif



