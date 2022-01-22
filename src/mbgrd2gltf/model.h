#ifndef STOQS_MODEL_H
#define STOQS_MODEL_H

// local includes
#include "geometry.h"
#include "options.h"

	namespace model
	{
		void write_gltf(const Geometry& geometry, const Options& options);
	}

#endif
