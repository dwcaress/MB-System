#ifndef STOQS_MATRIX_H
#define STOQS_MATRIX_H

// standard library
#include <cstddef>
#include <stdexcept>
#include <string>

namespace stoqs
{
	template <typename T>
	class Matrix
	{
	private: // members

		size_t _size_x;
		size_t _size_y;
		T* _data;

	public: // methods

		Matrix() :
		_size_x(0),
		_size_y(0),
		_data(nullptr)
		{}

		Matrix(size_t size_x, size_t size_y) :
		_size_x(size_x),
		_size_y(size_y),
		_data(new T[size_x * size_y])
		{}

		Matrix(Matrix&& other) :
		_size_x(other._size_x),
		_size_y(other._size_y),
		_data(other._data)
		{
			other._data = nullptr;
		}

		Matrix(const Matrix& other) :
		_size_x(other.size_x()),
		_size_y(other.size_y()),
		_data(new T[other.count()])
		{
			size_t len = count();
			const T *other_data = other.data();

			for (size_t i = 0; i < len; ++i)
				_data[i] = other_data[i];
		}

		~Matrix()
		{
			delete[] _data;
		}

		inline T& operator[](size_t i)
		{
			if (i >= count())
				throw std::out_of_range("attempted to access index "
					+ std::to_string(i)
					+ " but size was "
					+ std::to_string(count()));

			return _data[i];
		}

		inline const T& operator[](size_t i) const
		{
			if (i >= count())
				throw std::out_of_range("attempted to access index "
					+ std::to_string(i)
					+ " but size was "
					+ std::to_string(count()));

			return _data[i];
		}

		inline const T& at(size_t x, size_t y) const
		{
			if (x >= _size_x || y >= _size_y)
				throw std::out_of_range("attempted to access element at ("
					+ std::to_string(x)
					+ ", "
					+ std::to_string(y)
					+ ") but size is ("
					+ std::to_string(_size_x)
					+ ", "
					+ std::to_string(_size_y)
					+ ")");

			return _data[index(x, y)];
		}

		inline T& at(size_t x, size_t y)
		{
			if (x >= _size_x || y >= _size_y)
				throw std::out_of_range("attempted to access element at ("
					+ std::to_string(x)
					+ ", "
					+ std::to_string(y)
					+ ") but size is ("
					+ std::to_string(_size_x)
					+ ", "
					+ std::to_string(_size_y)
					+ ")");

			return _data[index(x, y)];
		}

		inline size_t index(size_t x, size_t y) const
		{
			return x + y * _size_x;
		}

		inline size_t size_x() const { return _size_x; }
		inline size_t size_y() const { return _size_y; }
		inline T* data() { return _data; }
		inline const T* data() const { return _data; }
		inline size_t count() const { return _size_x * _size_y; }

		inline Matrix& operator=(Matrix&& other)
		{
			_size_x = other._size_x;
			_size_y = other._size_y;
			_data = other._data;

			other._data = nullptr;

			return *this;
		}

		inline Matrix& operator=(const Matrix& other)
		{
			delete[] _data;

			size_t len = other.count();

			_size_x = other.size_x();
			_size_y = other.size_y();
			_data = new T[len];

			const T* other_data = other.data();

			for (size_t i = 0; i < len; ++i)
				_data[i] = other_data[i];

			return *this;
		}
	};
}

#endif
