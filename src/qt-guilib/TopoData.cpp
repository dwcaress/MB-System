#include "TopoData.h"
#include <limits>

// Use of very large number here (e.g. std::numeric_limits<int>)
// causes problems with VTK
const double mb_system::TopoData::NoData = -10000000.;

const char *mb_system::TopoData::GeographicType_ = "Geographic";
const char *mb_system::TopoData::UtmType_ = "UTM";

mb_system::TopoData::TopoData() {
}
