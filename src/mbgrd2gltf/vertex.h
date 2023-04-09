#ifndef STOQS_POINT_H
#define STOQS_POINT_H

// standard library
#include <cstdint>

namespace stoqs
{
	class Vertex
	{
	private: // members

		float _x;
		float _y;
		float _z;
		uint32_t _id;

	public: // methods

		Vertex() :
		_x(0),
		_y(0),
		_z(0),
		_id(0)
		{}

		Vertex(float x, float y, float z, uint32_t id) :
		_x(x),
		_y(y),
		_z(z),
		_id(id)
		{}

		Vertex(Vertex&&) = default;
		Vertex(const Vertex&) = default;

		inline float x() const { return _x; }
		inline float y() const { return _y; }
		inline float z() const { return _z; }
		inline uint32_t index() const { return _id - 1; }
		inline bool is_valid() const { return _id > 0; }

		Vertex& operator=(Vertex&&) = default;
		Vertex& operator=(const Vertex&) = default;
	};
}

#endif
