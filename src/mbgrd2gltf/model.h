#ifndef STOQS_MODEL_H
#define STOQS_MODEL_H

// local includes
#include <stoqs/geometry.h>
#include <stoqs/options.h>

namespace stoqs
{
	namespace model
	{
		void write_gltf(const Geometry& geometry, const Options& options);
	}
}

#endif
