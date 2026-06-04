#ifndef TopoDataItemSettings_H
#define TopoDataItemSettings_H


#include "TopoDataItem.h"


namespace mb_system {
/// This class saves ui-visible TopoDataItem properties to a file, and
/// can read them back from the file
class TopoDataItemSettings {
 public:

  /// Write TopoDataItem properties to settings file; returns false on error
  static bool save(std::filesystem::path &path, TopoDataItem *item);  

  /// Read TopoDataItem properties from settings file; returns false on error
  static bool load(std::filesystem::path &path, TopoDataItem *item);

  /// Round double value to specified precision
  static double round(double value, int precision);
};

}  // end namespace 
#endif


