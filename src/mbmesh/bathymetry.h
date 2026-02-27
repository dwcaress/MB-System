#ifndef BATHYMETRY_H
#define BATHYMETRY_H

#include <string>
#include <vector>
#include <limits>

class Bathymetry {
  public:
	Bathymetry(const std::string &filepath);

	size_t width() const { return _width; }
	size_t height() const { return _height; }

	double x_min() const { return _x_min; }
	double x_max() const { return _x_max; }
	double y_min() const { return _y_min; }
	double y_max() const { return _y_max; }
	double z_min() const { return _z_min; }
	double z_max() const { return _z_max; }

	double x_spacing() const { return _x_spacing; }
	double y_spacing() const { return _y_spacing; }

	float get_elevation(size_t x, size_t y) const;

  private:
	void read_netcdf_file(const std::string &filepath);

	size_t _width, _height;
	double _x_min, _x_max, _y_min, _y_max, _z_min, _z_max;
	double _x_spacing, _y_spacing;
	std::vector<float> _elevation_data;
};

#endif
