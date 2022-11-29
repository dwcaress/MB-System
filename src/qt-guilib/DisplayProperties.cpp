#include "DisplayProperties.h"

using namespace mb_system;

DisplayProperties::DisplayProperties() :
  changed_(false),
  showAxes_(false),
  verticalExagg_(1.),
  topoColorMapScheme_(TopoColorMap::Scheme::Haxby),
  siteFile_(nullptr),
  sitePoints_(nullptr) {
}


