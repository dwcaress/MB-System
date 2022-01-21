#include <stoqs/bathymetry.h>

// local includes
#include <stoqs/compression.h>

// external libraries
#include <netcdf.h>

namespace stoqs
{
	Bathymetry::Bathymetry(const Options& options)
	{
		int netcdf_id = get_netcdf_id(options.input_filepath().c_str());

		try
		{
			_side = get_dimension_length(netcdf_id, "side");
			_xysize = get_dimension_length(netcdf_id, "xysize");
			get_variable_double_array(netcdf_id, "x_range", _x_range, _side);
			get_variable_double_array(netcdf_id, "y_range", _y_range, _side);
			get_variable_double_array(netcdf_id, "z_range", _z_range, _side);
			get_variable_double_array(netcdf_id, "spacing", _spacing, _side);			
			get_variable_uint_array(netcdf_id, "dimension", _dimension, _side);
			_z = Matrix<float>(_dimension[0], _dimension[1]);
			get_variable_float_array(netcdf_id, "z", _z.data(), _xysize);
		}
		catch (const std::exception&)
		{
			nc_close(netcdf_id);

			throw;
		}

		int return_value = nc_close(netcdf_id);

		if (return_value != NC_NOERR)
			throw NetCdfError(return_value, "failed to close netCDF file");

		compress(options);
	}
	
	int Bathymetry::get_netcdf_id(const char *filepath)
	{
		int netcdf_id;
		int return_value = nc_open(filepath, NC_NOWRITE, &netcdf_id);

		if (return_value != NC_NOERR)
			throw NetCdfError(return_value, "failed to open netCDF file: "
				+ std::string(filepath));	

		return netcdf_id;
	}

	int Bathymetry::get_variable_id(int netcdf_id, const char *name)
	{
		int variable_id;
		int return_value = nc_inq_varid(netcdf_id, name, &variable_id);

		if (return_value != NC_NOERR)
			throw NetCdfError(return_value, "failed to get ID for variable '"
				+ std::string(name)
				+ "'");

		return variable_id;
	}

	int Bathymetry::get_dimension_id(int netcdf_id, const char *name)
	{
		int dimension_id;
		int return_value = nc_inq_dimid(netcdf_id, name, &dimension_id);

		if (return_value != NC_NOERR)
			throw NetCdfError(return_value, "failed to get ID for dimension '"
				+ std::string(name)
				+ "'");

		return dimension_id;
	}

	size_t Bathymetry::get_attribute_length(int netcdf_id, const char *name)
	{
		size_t length;
		int return_value = nc_inq_attlen(netcdf_id, NC_GLOBAL, name, &length);

		if (return_value != NC_NOERR)
			throw NetCdfError(return_value, "failed to get length for attribute '"
				+ std::string(name)
				+ "'");

		return length;
	}

	void Bathymetry::get_attribute_text(int netcdf_id, const char *name, char *out)
	{
		int return_value = nc_get_att_text(netcdf_id, NC_GLOBAL, name, out);

		if (return_value != NC_NOERR)
			throw NetCdfError(return_value, "failed to get text for attribute '"
				+ std::string(name)
				+ "'");
	}

	size_t Bathymetry::get_dimension_length(int netcdf_id, const char *name)
	{
		size_t out;

		int dimension_id = get_dimension_id(netcdf_id, name);
		int return_value = nc_inq_dimlen(netcdf_id, dimension_id, &out);

		if (return_value != NC_NOERR)
			throw NetCdfError(return_value, "failed to get length for dimension '"
				+ std::string(name)
				+ "'");

		return out;
	}

	void Bathymetry::get_variable_double_array(int netcdf_id, const char *name, double *out, size_t side)
	{
		const size_t start = 0;
		int variable_id = get_variable_id(netcdf_id, name);
		int return_value = nc_get_vara_double(netcdf_id, variable_id, &start, &side, out);

		if (return_value != NC_NOERR)
			throw NetCdfError(return_value, "failed to get double array data for variable '"
				+ std::string(name)
				+ "'");
	}

	void Bathymetry::get_variable_float_array(int netcdf_id, const char *name, float *out, size_t length)
	{
		size_t start = 0;
		int variable_id = get_variable_id(netcdf_id, name);
		int return_value = nc_get_vara_float(netcdf_id, variable_id, &start, &length, out);

		if (return_value != NC_NOERR)
			throw NetCdfError(return_value, "failed to get float array data for variable '"
				+ std::string(name)
				+ "'");
	}

	void Bathymetry::get_variable_uint_array(int netcdf_id, const char *name, unsigned *out, size_t length)
	{
		size_t start = 0;
		int variable_id = get_variable_id(netcdf_id, name);
		int return_value = nc_get_vara_uint(netcdf_id, variable_id, &start, &length, out);

		if (return_value != NC_NOERR)
			throw NetCdfError(return_value, "failed to get uint array data for variable '"
				+ std::string(name)
				+ "'");
	}

	void Bathymetry::compress(const Options& options)
	{
		if (!options.is_compression_set() && !options.is_max_size_set())
			return;

		Matrix<float> compressed_z = compression::compress(_z, options);

		_z = compressed_z;
		_xysize = _z.count();
		_dimension[0] = _z.size_x();
		_dimension[1] = _z.size_y();
		_spacing[0] = std::labs(_x_range[1] - _x_range[0]) / (double)(_dimension[0] - 1);
		_spacing[1] = std::labs(_y_range[1] - _y_range[0]) / (double)(_dimension[1] - 1);
	}

	std::string Bathymetry::to_string() const
	{
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
}
