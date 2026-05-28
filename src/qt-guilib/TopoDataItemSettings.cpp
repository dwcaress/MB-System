#include <filesystem>
#include <fstream>
#include <toml++/toml.hpp>
#include "TopoDataItemSettings.h"

using namespace mb_system;

// Define key names for consistency between save() and load()
#define Overlay "overlay"
#define ColorMap "colorMap"
#define SurfaceColoredBy "surfaceColoredBy"
#define SurfaceRenderer "surfaceRenderer"
#define ShadowSource "shadowSource"
#define Contours "contours"
#define Axes "axes"

/**
   Helper class saves and loads TopoDataItem settings from a file
*/
bool TopoDataItemSettings::save(std::filesystem::path &path, TopoDataItem *item) {
    
  std::cerr << "TopoDataItemSettings::save()\n";
  auto tbl = toml::table {{
      {Overlay, toml::table{{
	    { Contours, item->showContours() },
	    { Axes, item->showAxes() }	    
	  }} },
      { ColorMap, item->getColormapScheme() },
      { SurfaceColoredBy, item->coloredScalar() },      
      { SurfaceRenderer, item->surfaceRenderType() },
      { ShadowSource, item->shadowSource() }            
    }};

  std::filesystem::create_directories(path.parent_path());
  std::ofstream file(path);
  file << tbl;
  file << '\n';
  return true;
}


bool TopoDataItemSettings::load(std::filesystem::path &path, TopoDataItem *item) {
  if (!std::filesystem::exists(path)) {
    std::cerr << "unable to open file " << path << "\n";
    return false;
  }

  try {
    std::cerr << "parse " << path << "\n";
    auto tbl = toml::parse_file(path.string());

    item->setColormap(tbl[ColorMap].value_or("UNKNOWN"));

    item->setContours(tbl[Overlay][Contours].value_or(false));
    item->setShowAxes(tbl[Overlay][Axes].value_or(false));    
  }
  catch (const toml::parse_error &e) {
      std::cerr << "parse error: " << e.description() << "\n";
  }
  
  return true;
}

