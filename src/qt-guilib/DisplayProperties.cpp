#include "DisplayProperties.h"

using namespace mb_system;

DisplayProperties::DisplayProperties() :
  changed(false),
  showAxes(false),
  verticalExagg(1.),
  topoColorMapScheme(TopoColorMap::Scheme::Haxby),
  siteFile(nullptr),
  sitePoints(nullptr) {
}

