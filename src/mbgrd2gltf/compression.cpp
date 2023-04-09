#include <stoqs/compression.h>

// standard library
#include <cmath>
#include <cstdint>

namespace stoqs
{
	namespace compression
	{
		struct Size
		{
			size_t x;
			size_t y;
		};

		const size_t bytes_per_vertex = sizeof(float) * 3;
		const size_t bytes_per_triangle = sizeof(uint32_t) * 3;

		// solves new width for compression
		Size get_compressed_size(const Matrix<float>& m, size_t max_bytes)
		{
			const double vert_bytes = (double)bytes_per_vertex;
			const double tri_bytes = (double)bytes_per_triangle;
			const double x = (double)m.size_x();
			const double y = (double)m.size_y();
			const double yx_ratio = y / x;
			const double a =  (vert_bytes + 2.0 * tri_bytes) * yx_ratio;
			const double b = -(tri_bytes * 2.0 * yx_ratio + tri_bytes * 2.0);
			const double c = tri_bytes * 2.0 - max_bytes;
			
			const double width = (-b + std::sqrt(b * b - 4 * a * c)) / (2.0 * a);
			const double height = width * yx_ratio;

			return { (size_t)width, (size_t)height };
		}

		size_t get_size_in_bytes(const Matrix<float>& m)
		{
			return bytes_per_vertex * m.size_x() * m.size_y() + bytes_per_triangle * (m.size_x() - 1) * (m.size_y() - 1);
		}

		size_t get_compressed_bytes(const Matrix<float>& m, double compression_ratio)
		{
			size_t size_in_bytes = get_size_in_bytes(m);
			double compressed_size = (double)size_in_bytes / compression_ratio;

			return (size_t)compressed_size;
		}		

		float get_average(const Matrix<float>& altitudes, size_t x_start, size_t y_start, size_t x_count, size_t y_count)
		{
			size_t x_end = x_start + x_count;
			size_t y_end = y_start + y_count;
			size_t vertex_count = x_count * y_count;
			float sum = 0.0;

			for (size_t y = y_start; y < y_end; ++y)
			{
				for (size_t x = x_start; x < x_end; ++x)
				{
					float value = altitudes.at(x, y);

					if (std::isnan(value) || std::isinf(value))
					{
						vertex_count -= 1;
					}
					else
					{
						sum += value;
					}
				}
			}

			return sum / (float)vertex_count;
		}

		size_t min_size(size_t a, size_t b)
		{
			return a < b ? a : b;
		}

		Matrix<float> compress(const Matrix<float>& altitudes, const Options& options)
		{
			size_t size_in_bytes = options.is_max_size_set()
				? min_size(options.max_size(), get_size_in_bytes(altitudes))
				: get_compressed_bytes(altitudes, options.compression_ratio());			

			Size compressed_size = get_compressed_size(altitudes, size_in_bytes);

			if (compressed_size.x < 2 || compressed_size.y < 2)
				throw std::invalid_argument("compression ratio was set high for bathymetry ");

			Matrix<float> out(compressed_size.x, compressed_size.y);
			
			double x_step = (double)(altitudes.size_x() - out.size_x() - 1) / (double)out.size_x() + 1.0;
			double y_step = (double)(altitudes.size_y() - out.size_y() - 1) / (double)out.size_y() + 1.0;
			size_t vertex_count_x = (size_t)std::ceil(x_step);
			size_t vertex_count_y = (size_t)std::ceil(y_step);

			for (size_t y = 0; y < out.size_y(); ++y)
			{
				size_t start_y = (size_t)((double)y * y_step);

				for (size_t x = 0; x < out.size_x(); ++x)
				{
					size_t start_x = (size_t)((double)x * x_step);

					out.at(x, y) = get_average(altitudes, start_x, start_y, vertex_count_x, vertex_count_y);
				}
			}

			return out;
		}
	}
}
