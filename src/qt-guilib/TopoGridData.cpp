#include "TopoGridData.h"
#include <limits>

// Use of very large number here (e.g. std::numeric_limits<int>)
// causes problems with VTK
const double mb_system::TopoGridData::NoData = -10000000.;
