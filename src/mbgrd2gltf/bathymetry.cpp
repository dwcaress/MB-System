/*--------------------------------------------------------------------
 *    The MB-system:	bathymetry.cpp	5/11/2023
 *
 *    Copyright (c) 2023-2024 by
 *    David W. Caress (caress@mbari.org)
 *      Monterey Bay Aquarium Research Institute
 *      Moss Landing, California, USA
 *    Dale N. Chayes
 *      Center for Coastal and Ocean Mapping
 *      University of New Hampshire
 *      Durham, New Hampshire, USA
 *    Christian dos Santos Ferreira
 *      MARUM
 *      University of Bremen
 *      Bremen Germany
 *
 *    MB-System was created by Caress and Chayes in 1992 at the
 *      Lamont-Doherty Earth Observatory
 *      Columbia University
 *      Palisades, NY 10964
 *
 *    See README.md file for copying and redistribution conditions.
 *--------------------------------------------------------------------*/
 /*
  *    The program MBgrd2gltf, including this source file, was created
  *    by a Capstone Project team at the California State University
  *    Monterey Bay (CSUMB) including Kyle Dowling, Julian Fortin,
  *    Jesse Benavides, Nicolas Porras Falconio. This team was mentored by:
  *    Mike McCann
  *      Monterey Bay Aquarium Research Institute
  *      Moss Landing, California, USA
  *--------------------------------------------------------------------*/

#include "bathymetry.h"
#include "compression.h"
#include <cmath>
#include <netcdf.h>
#include <iostream>

namespace mbgrd2gltf {

	Bathymetry::Bathymetry(const Options& options) {
		int netcdf_id = get_netcdf_id(options.input_filepath().c_str());

		// Support various arbitrary versions of metadata describing the grid through try-catch blocks
		std::string x_name = "x";
		std::string y_name = "y";
		try {
			_side = get_dimension_length(netcdf_id, "side");
		}
		catch (const std::exception&) {
			_side = 2;
		}

		try {
			_xysize = get_dimension_length(netcdf_id, "xysize");
		}
		catch (const std::exception& e) {
			std::cerr << "Failed to get xysize = " << e.what() << std::endl;
			try {
				_x = get_dimension_length(netcdf_id, x_name.c_str());
				_y = get_dimension_length(netcdf_id, y_name.c_str());
				_xysize = _x * _y;
			}
			catch (const std::exception& f) {
				std::cerr << "Trying lon and lat..." << f.what() << std::endl;
				_x = get_dimension_length(netcdf_id, "lon");
				_y = get_dimension_length(netcdf_id, "lat");
				_xysize = _x * _y;
				x_name = "lon";
				y_name = "lat";
			}
		}

		try {
			get_variable_double_array(netcdf_id, "x_range", _x_range, _side);
			get_variable_double_array(netcdf_id, "y_range", _y_range, _side);
			get_variable_double_array(netcdf_id, "z_range", _z_range, _side);
		}
		catch (const std::exception&) {
			// Use actual_range variable attributes
			get_variable_attribute_double(netcdf_id, x_name.c_str(), "actual_range", _x_range);
			get_variable_attribute_double(netcdf_id, y_name.c_str(), "actual_range", _y_range);
			get_variable_attribute_double(netcdf_id, "z", "actual_range", _z_range);
			std::swap(_y_range[0], _y_range[1]);
		}

		try {
			get_variable_double_array(netcdf_id, "spacing", _spacing, _side);
		}
		catch (const std::exception&) {
			_spacing[0] = (_x_range[1] - _x_range[0]) / _x;
			_spacing[1] = (_y_range[1] - _y_range[0]) / _y;
		}

		try {
			_start[0] = 0;
			_length[0] = _side;
			get_variable_uint_array(netcdf_id, "dimension", _dimension, _start, _length);
		}
		catch (const std::exception&) {
			_dimension[0] = _x;
			_dimension[1] = _y;
		}

		try {
			_start[0] = 0;
			_length[0] = _xysize;
			size_t _zero = 0;
			_z = Matrix<float>(_dimension[0], _dimension[1]);
			get_variable_float_array(netcdf_id, "z", _z.data(), &_zero, &_xysize);
		}
		catch (const std::exception&) {
			_start[1] = 0;
			_start[0] = 0;
			_length[1] = _x;
			_length[0] = _y;
			_z = Matrix<float>(_length[1], _length[0]);
			get_variable_float_array(netcdf_id, "z", _z.data(), _start, _length);
		}

		int return_value = nc_close(netcdf_id);
		if (return_value != NC_NOERR) {
			throw NetCdfError(return_value, "failed to close netCDF file");
		}

		compress(options);
	}

	int Bathymetry::get_netcdf_id(const char* filepath) {
		int netcdf_id;
		int return_value = nc_open(filepath, NC_NOWRITE, &netcdf_id);
		if (return_value != NC_NOERR) {
			throw NetCdfError(return_value, "failed to open netCDF file: " + std::string(filepath));
		}
		return netcdf_id;
	}

	int Bathymetry::get_variable_id(int netcdf_id, const char* name) {
		int variable_id;
		int return_value = nc_inq_varid(netcdf_id, name, &variable_id);
		if (return_value != NC_NOERR) {
			throw NetCdfError(return_value, "failed to get ID for variable '" + std::string(name) + "'");
		}
		return variable_id;
	}

	int Bathymetry::get_dimension_id(int netcdf_id, const char* name) {
		int dimension_id;
		int return_value = nc_inq_dimid(netcdf_id, name, &dimension_id);
		if (return_value != NC_NOERR) {
			throw NetCdfError(return_value, "failed to get ID for dimension '" + std::string(name) + "'");
		}
		return dimension_id;
	}

	size_t Bathymetry::get_attribute_length(int netcdf_id, const char* name) {
		size_t length;
		int return_value = nc_inq_attlen(netcdf_id, NC_GLOBAL, name, &length);
		if (return_value != NC_NOERR) {
			throw NetCdfError(return_value, "failed to get length for attribute '" + std::string(name) + "'");
		}
		return length;
	}

	void Bathymetry::get_attribute_text(int netcdf_id, const char* name, char* out) {
		int return_value = nc_get_att_text(netcdf_id, NC_GLOBAL, name, out);
		if (return_value != NC_NOERR) {
			throw NetCdfError(return_value, "failed to get text for attribute '" + std::string(name) + "'");
		}
	}

	size_t Bathymetry::get_dimension_length(int netcdf_id, const char* name) {
		size_t out;
		int dimension_id = get_dimension_id(netcdf_id, name);
		int return_value = nc_inq_dimlen(netcdf_id, dimension_id, &out);
		if (return_value != NC_NOERR) {
			throw NetCdfError(return_value, "failed to get length for dimension '" + std::string(name) + "'");
		}
		return out;
	}

	void Bathymetry::get_variable_double_array(int netcdf_id, const char* name, double* out, size_t side) {
		const size_t start = 0;
		int variable_id = get_variable_id(netcdf_id, name);
		int return_value = nc_get_vara_double(netcdf_id, variable_id, &start, &side, out);
		if (return_value != NC_NOERR) {
			throw NetCdfError(return_value, "failed to get double array data for variable '" + std::string(name) + "'");
		}
	}

	void Bathymetry::get_variable_float_array(int netcdf_id, const char* name, float* out, size_t* start, size_t* length) {
		int variable_id = get_variable_id(netcdf_id, name);
		int return_value = nc_get_vara_float(netcdf_id, variable_id, start, length, out);
		if (return_value != NC_NOERR) {
			throw NetCdfError(return_value, "failed to get float array data for variable '" + std::string(name) + "'");
		}
	}

	void Bathymetry::get_variable_uint_array(int netcdf_id, const char* name, unsigned int* out, size_t* start, size_t* length) {
		int variable_id = get_variable_id(netcdf_id, name);
		int return_value = nc_get_vara_uint(netcdf_id, variable_id, start, length, out);
		if (return_value != NC_NOERR) {
			throw NetCdfError(return_value, "failed to get uint array data for variable '" + std::string(name) + "'");
		}
	}

	void Bathymetry::compress(const Options& options) {
		Options localOptions = options;
		if (!options.is_stride_set() && (_z.size_x() * _z.size_y() > 9000000)) {
			std::cout << "Auto setting stride to 1." << std::endl;
			localOptions.set_stride_ratio(1.0);
		}
		if (!options.is_stride_set() && !options.is_max_size_set() && (_z.size_x() * _z.size_y() < 9000000))
			return;
		Matrix<float> compressed_z = compression::compress(_z, localOptions);
		_z = compressed_z;
		_xysize = _z.count();
		_dimension[0] = _z.size_x();
		_dimension[1] = _z.size_y();
		// Allow negative values for when one or more grid axes are reversed
		_spacing[0] = (_x_range[1] - _x_range[0]) / static_cast<double>(_dimension[0] - 1);
		_spacing[1] = (_y_range[1] - _y_range[0]) / static_cast<double>(_dimension[1] - 1);
	}

	std::string Bathymetry::to_string() const {
		std::string out;
		out.reserve(256);

		out += '{';
		out += "\nDimensions:\n";
		out += "\n    Side:      " + std::to_string(_side);
		out += "\n    XYSize:    " + std::to_string(_xysize);
		out += "\n\nVariables:\n";
		out += "\n    Dimension: " + std::to_string(_dimension[0]) + ", " + std::to_string(_dimension[1]);
		out += "\n    X Range:   " + std::to_string(_x_range[0]) + ", " + std::to_string(_x_range[1]);
		out += "\n    Y Range:   " + std::to_string(_y_range[0]) + ", " + std::to_string(_y_range[1]);
		out += "\n    Z Range:   " + std::to_string(_z_range[0]) + ", " + std::to_string(_z_range[1]);
		out += "\n    Spacing:   " + std::to_string(_spacing[0]) + ", " + std::to_string(_spacing[1]);
		out += "\n}";

		return out;
	}

	void Bathymetry::get_variable_attribute_double(int netcdf_id, const char* var_name, const char* att_name, double* out) {
		int variable_id = get_variable_id(netcdf_id, var_name);
		int return_value = nc_get_att_double(netcdf_id, variable_id, att_name, out);
		if (return_value != NC_NOERR) {
			throw NetCdfError(return_value, "failed to get double value(s) for attribute '" + std::string(att_name) + "' for var_name " + std::string(var_name));
		}
	}

} // namespace mbgrd2gltf
