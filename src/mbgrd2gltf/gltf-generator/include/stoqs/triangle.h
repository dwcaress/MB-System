#ifndef STOQS_TRIANGLE_H
#define STOQS_TRIANGLE_H

// standard library
#include <cstdint>

namespace stoqs
{
	class Triangle
	{
	private: // members

		uint32_t _a;
		uint32_t _b;
		uint32_t _c;

	public: // methods

		Triangle() :
		_a(0),
		_b(0),
		_c(0)
		{}

		Triangle(uint32_t a, uint32_t b, uint32_t c) :
		_a(a),
		_b(b),
		_c(c)
		{}

		inline uint32_t a() const { return _a; }
		inline uint32_t b() const { return _b; }
		inline uint32_t c() const { return _c; }
	};
}

#endif
