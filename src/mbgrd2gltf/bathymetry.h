#ifndef STOQS_BATHYMETRY_H
#define STOQS_BATHYMETRY_H

// local includes
#include <stoqs/matrix.h>
#include <stoqs/options.h>

// standard library
#include <string>
#include <unordered_map>

namespace stoqs
{
	class Bathymetry
	{
	public: // types

		class NetCdfError : public std::exception
		{
		private:

			int _error_code;
			std::string _msg;

		public:

			NetCdfError(int error_code, std::string&& msg) :
			_error_code(error_code),
			_msg("NetCDF error " + std::to_string(error_code) + ": " + msg)
			{}

			int error_code() const { return _error_code; }
			const char *what() const noexcept
			{
				return _msg.c_str();
			}
		};

	private: // members

		Matrix<float> _z;
		double _x_range[2];
		double _y_range[2];
		double _z_range[2];
		double _spacing[2];
		size_t _side;		
		size_t _xysize;
		unsigned _dimension[2];		

	private: // methods

		static int get_netcdf_id(const char *filepath);
		static int get_variable_id(int netcdf_id, const char *name);
		static int get_dimension_id(int netcdf_id, const char *name);
		static size_t get_attribute_length(int netcdf_id, const char *name);
		static void get_attribute_text(int netcdf_id, const char *name, char *out);
		static size_t get_dimension_length(int netcdf_id, const char *name);
		static void get_variable_double_array(int netcdf_id, const char *name, double *out, size_t length);
		static void get_variable_float_array(int netcdf_id, const char *name, float *out, size_t length);
		static void get_variable_uint_array(int netcdf_id, const char *name, unsigned *out, size_t length);

		void compress(const Options& options);

	public: // methods

		Bathymetry(const Options& options);
		Bathymetry(Bathymetry&&) = default;
		Bathymetry(const Bathymetry&) = default;

		inline const Matrix<float>& altitudes() const { return _z; }
		inline double longitude_min() const { return _x_range[0]; }
		inline double longitude_max() const { return _x_range[1]; }
		inline double latitude_min() const { return _y_range[0]; }
		inline double latitude_max() const { return _y_range[1]; }
		inline double altitude_min() const { return _z_range[0]; }
		inline double altitude_max() const { return _z_range[1]; }
		inline double longitude_spacing() const { return _spacing[0]; }
		inline double latitude_spacing() const { return _spacing[1]; }
		inline unsigned size_x() const { return _dimension[0]; }
		inline unsigned size_y() const { return _dimension[1]; }
		inline size_t side_count() const { return _side; }
		inline size_t altitudes_length() const { return _xysize; }

		std::string to_string() const;
	};
}

#endif
