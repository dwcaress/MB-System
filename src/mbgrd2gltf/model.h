#ifndef MODEL_H
#define MODEL_H

// local includes
#include "geometry.h"
#include "options.h"

namespace mbgrd2gltf
{
	namespace model
	{
		void write_gltf(const Geometry& geometry, const Options& options);
	}
}

#endif
