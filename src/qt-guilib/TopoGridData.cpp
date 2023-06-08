#include "TopoGridData.h"
#include <limits>

// Use of very large number here (e.g. std::numeric_limits<int>)
// causes problems with VTK
const double mb_system::TopoGridData::NoData = -10000000.;

const char *mb_system::TopoGridData::GeographicType_ = "Geographic";
const char *mb_system::TopoGridData::UtmType_ = "UTM";

mb_system::TopoGridData::TopoGridData() {
}
