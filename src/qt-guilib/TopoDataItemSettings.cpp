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
#define ShadowSrc "shadowSource"
#define Contours "contours"
#define ContourLabels "contourLabels"
#define ContourInterval "contourInterval"
#define Axes "axes"
#define VerticalExagg "verticalExagg"

/**
   Helper class saves and loads TopoDataItem settings from a file
*/
bool TopoDataItemSettings::save(std::filesystem::path &path, TopoDataItem *item) {
    
  std::cerr << "TopoDataItemSettings::save()\n";
  auto tbl = toml::table {{
      {Overlay, toml::table{{
	    { Contours, item->getShowContours() },
	    { ContourLabels, item->getShowContourLabels() },
	    { ContourInterval, item->getContourInterval() },	    	    
	    { Axes, item->showAxes() }	    
	  }} },
      { ColorMap, item->getColormapScheme() },
      { SurfaceColoredBy, item->coloredScalar() },
      { VerticalExagg, item->getVerticalExagg() },
      { SurfaceRenderer, item->surfaceRenderType() },
      { ShadowSrc, item->shadowSource() }            
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
    item->setVerticalExagg(tbl[VerticalExagg].value_or(1.));        
    item->
      setShadowSource((TopoDataItem::ShadowSource)tbl[ShadowSrc].
		      value_or(TopoDataItem::ShadowSource::Illumination));
    item->
      setColoredScalar((TopoDataItem::ColoredScalar)tbl[SurfaceColoredBy].
		       value_or(TopoDataItem::ColoredScalar::Elevation));

    item->
      setSurfaceRenderType((TopoDataItem::SurfaceRenderType)tbl[SurfaceRenderer].
			   value_or(TopoDataItem::ColoredScalar::Elevation));
    
    item->setContours(tbl[Overlay][Contours].value_or(false));
    item->setShowContourLabels(tbl[Overlay][ContourLabels].value_or(false));
    item->setContourInterval(tbl[Overlay][ContourInterval].value_or(100.));    
    item->setShowAxes(tbl[Overlay][Axes].value_or(false));
  }
  catch (const toml::parse_error &e) {
      std::cerr << "parse error: " << e.description() << "\n";
  }
  
  return true;
}

