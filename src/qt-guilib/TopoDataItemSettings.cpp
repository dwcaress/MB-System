#include <filesystem>
#include <fstream>
#include <toml++/toml.hpp>
#include "TopoDataItemSettings.h"

using namespace mb_system;

/**
   Helper class saves and loads TopoDataItem settings from a file
*/
bool TopoDataItemSettings::save(std::filesystem::path &path, TopoDataItem *item) {
    
  std::cerr << "TopoDataItemSettings::save()\n";
  auto tbl = toml::table {{
      {"overlay", toml::table{{
	    { "contours", item->showContours() },
	    { "axes", item->showAxes() },	    
	    { "colorMap", item->getColormapScheme() }	    
	  }}}
    }};

  std::filesystem::create_directories(path.parent_path());
  std::ofstream file(path);
  file << tbl;
  file << '\n';
  return true;
}
