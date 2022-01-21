// local includes
#include <stoqs/bathymetry.h>
#include <stoqs/geometry.h>
#include <stoqs/model.h>
#include <stoqs/options.h>

// standard library
#include <iostream>

// external libraries
#include <netcdf.h>

using namespace stoqs;

int main(int argc, char *argv[])
{
	try
	{
		Options options((unsigned)argc, (const char**)argv);

		if (options.is_help())
			return 0;

		Bathymetry bathymetry(options);
		Geometry geometry(bathymetry, options);
		model::write_gltf(geometry, options);
	}
	catch (const std::exception& e)
	{
		std::cout << "error: " << e.what() << std::endl;

		return 1;
	}

	return 0;
}
