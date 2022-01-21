#ifndef STOQS_COMPRESSION_H
#define STOQS_COMPRESSION_H

// local includes
#include <stoqs/matrix.h>
#include <stoqs/options.h>

namespace stoqs
{
	namespace compression
	{
		Matrix<float> compress(const Matrix<float>& altitudes, const Options& options);
	}
}

#endif
