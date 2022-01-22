#ifndef STOQS_COMPRESSION_H
#define STOQS_COMPRESSION_H

// local includes
#include "matrix.h"
#include "options.h"

	namespace compression
	{
		Matrix<float> compress(const Matrix<float>& altitudes, const Options& options);
	}

#endif
