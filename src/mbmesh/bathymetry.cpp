#include "bathymetry.h"
#include <netcdf.h>
#include <iostream>
#include <stdexcept>
#include <vector>

Bathymetry::Bathymetry(const std::string &filepath) {
	std::cout << "DEBUG: Opening file: " << filepath << std::endl;
	read_netcdf_file(filepath);
}

void Bathymetry::read_netcdf_file(const std::string &filepath) {
	int ncid;
	int status = nc_open(filepath.c_str(), NC_NOWRITE, &ncid);
	if (status != NC_NOERR) {
		throw std::runtime_error("Failed to open NetCDF file: " + filepath);
	}

	// Detect convention type: GMT, COARDS, or CF
	enum class GridConvention { GMT, COARDS, CF, UNKNOWN };
	GridConvention convention = GridConvention::UNKNOWN;

	int varid;
	if (nc_inq_varid(ncid, "x_range", &varid) == NC_NOERR && nc_inq_varid(ncid, "y_range", &varid) == NC_NOERR) {
		convention = GridConvention::GMT;
	}
	else if (nc_inq_varid(ncid, "lon", &varid) == NC_NOERR && nc_inq_varid(ncid, "lat", &varid) == NC_NOERR) {
		convention = GridConvention::COARDS;
	}
	else if (nc_inq_varid(ncid, "longitude", &varid) == NC_NOERR && nc_inq_varid(ncid, "latitude", &varid) == NC_NOERR) {
		convention = GridConvention::CF;
	}

	// Assign variable names based on convention
	std::string x_name, y_name, z_name;
	std::string x_range_name, y_range_name, z_range_name;

	switch (convention) {
	case GridConvention::GMT:
		x_name = "x";
		y_name = "y";
		z_name = "z";
		x_range_name = "x_range";
		y_range_name = "y_range";
		z_range_name = "z_range";
		break;
	case GridConvention::COARDS:
		x_name = "lon";
		y_name = "lat";
		z_name = "z";
		break;
	case GridConvention::CF:
		x_name = "longitude";
		y_name = "latitude";
		z_name = "elevation";
		break;
	default:
		x_name = "x";
		y_name = "y";
		z_name = "z";
	}

	// Get dimensions
	int x_dimid, y_dimid;
	status = nc_inq_dimid(ncid, x_name.c_str(), &x_dimid);
	if (status != NC_NOERR) {
		if (nc_inq_dimid(ncid, "lon", &x_dimid) != NC_NOERR) {
			nc_close(ncid);
			throw std::runtime_error("Could not find x/lon dimension");
		}
	}

	status = nc_inq_dimid(ncid, y_name.c_str(), &y_dimid);
	if (status != NC_NOERR) {
		if (nc_inq_dimid(ncid, "lat", &y_dimid) != NC_NOERR) {
			nc_close(ncid);
			throw std::runtime_error("Could not find y/lat dimension");
		}
	}

	size_t x_len, y_len;
	nc_inq_dimlen(ncid, x_dimid, &x_len);
	nc_inq_dimlen(ncid, y_dimid, &y_len);
	_width = x_len;
	_height = y_len;

	// Check elevation variable dimensions
	int z_varid;
	nc_inq_varid(ncid, z_name.c_str(), &z_varid);
	int ndims;
	nc_inq_varndims(ncid, z_varid, &ndims);
	int dimids[2];
	nc_inq_vardimid(ncid, z_varid, dimids);

	size_t z_dim0_len, z_dim1_len;
	nc_inq_dimlen(ncid, dimids[0], &z_dim0_len);
	nc_inq_dimlen(ncid, dimids[1], &z_dim1_len);

	char dim0_name[256], dim1_name[256];
	nc_inq_dimname(ncid, dimids[0], dim0_name);
	nc_inq_dimname(ncid, dimids[1], dim1_name);

	std::cout << "DEBUG: Z variable dimensions: [" << dim0_name << ":" << z_dim0_len << "] x [" << dim1_name << ":" << z_dim1_len
	          << "]" << std::endl;
	std::cout << "DEBUG: X/lon dimension (" << x_name << ") size: " << x_len << std::endl;
	std::cout << "DEBUG: Y/lat dimension (" << y_name << ") size: " << y_len << std::endl;

	// If Z dimensions don't match our x/y dimensions, we may have them backwards
	if (z_dim0_len == x_len && z_dim1_len == y_len) {
		std::cout << "DEBUG: Z dimensions match X then Y - swapping width/height!" << std::endl;
		// Z is stored as [x][y], so swap our interpretation
		std::swap(_width, _height);
		std::swap(x_len, y_len);
	}

	std::cout << "DEBUG: Final grid dimensions (width x height): " << _width << " x " << _height << std::endl;

	// Get range metadata - try range variables first, then attributes
	double x_range[2], y_range[2], z_range[2];

	if (nc_inq_varid(ncid, x_range_name.c_str(), &varid) == NC_NOERR) {
		// Use range variables (GMT convention)
		nc_get_var_double(ncid, varid, x_range);
		nc_inq_varid(ncid, y_range_name.c_str(), &varid);
		nc_get_var_double(ncid, varid, y_range);
		nc_inq_varid(ncid, z_range_name.c_str(), &varid);
		nc_get_var_double(ncid, varid, z_range);
	}
	else {
		// Fallback: read coordinate arrays or attributes
		int x_varid, y_varid, z_varid;

		if (nc_inq_varid(ncid, x_name.c_str(), &x_varid) == NC_NOERR) {
			std::vector<double> x_coords(_width);
			nc_get_var_double(ncid, x_varid, x_coords.data());
			x_range[0] = x_coords[0];
			x_range[1] = x_coords[_width - 1];
		}

		if (nc_inq_varid(ncid, y_name.c_str(), &y_varid) == NC_NOERR) {
			std::vector<double> y_coords(_height);
			nc_get_var_double(ncid, y_varid, y_coords.data());
			y_range[0] = y_coords[0];
			y_range[1] = y_coords[_height - 1];
		}

		if (nc_inq_varid(ncid, z_name.c_str(), &z_varid) == NC_NOERR) {
			// Read first and last values to get range
			float z_val;
			nc_get_var1_float(ncid, z_varid, nullptr, &z_val);
			z_range[0] = z_val;
			z_range[1] = z_val;
			// Scan through data to find min/max
			std::vector<float> z_data(_width * _height);
			nc_get_var_float(ncid, z_varid, z_data.data());
			for (auto val : z_data) {
				if (val < z_range[0])
					z_range[0] = val;
				if (val > z_range[1])
					z_range[1] = val;
			}
		}
	}

	_x_min = x_range[0];
	_x_max = x_range[1];
	_y_min = y_range[0];
	_y_max = y_range[1];
	_z_min = z_range[0];
	_z_max = z_range[1];

	_x_spacing = (_x_max - _x_min) / (_width - 1);
	_y_spacing = (_y_max - _y_min) / (_height - 1);

	std::cout << "DEBUG: X range: " << _x_min << " to " << _x_max << std::endl;
	std::cout << "DEBUG: Y range: " << _y_min << " to " << _y_max << std::endl;
	std::cout << "DEBUG: Z range: " << _z_min << " to " << _z_max << std::endl;

	// Get elevation data
	status = nc_inq_varid(ncid, z_name.c_str(), &z_varid);
	if (status != NC_NOERR) {
		nc_close(ncid);
		throw std::runtime_error("Could not find z/elevation variable");
	}
	_elevation_data.resize(_width * _height);
	nc_get_var_float(ncid, z_varid, _elevation_data.data());

	nc_close(ncid);
	std::cout << "DEBUG: Successfully loaded bathymetry grid" << std::endl;
}

float Bathymetry::get_elevation(size_t x, size_t y) const {
	if (x >= _width || y >= _height) {
		return std::numeric_limits<float>::quiet_NaN();
	}
	return _elevation_data[y * _width + x];
}
