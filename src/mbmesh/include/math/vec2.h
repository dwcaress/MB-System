#pragma once

#include <cassert>
#include <cmath>
#include <ostream>

constexpr double vec2_epsilon = 1e-8;

class Vec2 {
public:
    double x;
    double y;

    constexpr Vec2() noexcept : x(0.0), y(0.0) {}
    constexpr Vec2(double value) noexcept : x(value), y(value) {}
    constexpr Vec2(double x_value, double y_value) noexcept : x(x_value), y(y_value) {}

    [[nodiscard]] constexpr double &operator[](int i) noexcept { return i == 0 ? x : y; }
    [[nodiscard]] constexpr const double &operator[](int i) const noexcept { return i == 0 ? x : y; }

    [[nodiscard]] constexpr Vec2 operator-() const noexcept { return Vec2(-x, -y); }

    constexpr Vec2 &operator+=(const Vec2 &v) noexcept {
        x += v.x;
        y += v.y;
        return *this;
    }

    constexpr Vec2 &operator-=(const Vec2 &v) noexcept {
        x -= v.x;
        y -= v.y;
        return *this;
    }

    constexpr Vec2 &operator*=(double s) noexcept {
        x *= s;
        y *= s;
        return *this;
    }

    constexpr Vec2 &operator/=(double s) noexcept {
        assert(std::fabs(s) > vec2_epsilon);
        x /= s;
        y /= s;
        return *this;
    }

    [[nodiscard]] constexpr double length_squared() const noexcept { return x * x + y * y; }
    [[nodiscard]] double length() const noexcept { return std::sqrt(length_squared()); }
};

inline std::ostream &operator<<(std::ostream &out, const Vec2 &v) {
    return out << "(" << v.x << ", " << v.y << ")";
}

[[nodiscard]] inline constexpr Vec2 operator+(const Vec2 &v1, const Vec2 &v2) noexcept {
    return Vec2(v1.x + v2.x, v1.y + v2.y);
}

[[nodiscard]] inline constexpr Vec2 operator-(const Vec2 &v1, const Vec2 &v2) noexcept {
    return Vec2(v1.x - v2.x, v1.y - v2.y);
}

[[nodiscard]] inline constexpr Vec2 operator*(const Vec2 &v1, const Vec2 &v2) noexcept {
    return Vec2(v1.x * v2.x, v1.y * v2.y);
}

[[nodiscard]] inline constexpr Vec2 operator*(const Vec2 &v, double s) noexcept {
    return Vec2(v.x * s, v.y * s);
}

[[nodiscard]] inline constexpr Vec2 operator*(double s, const Vec2 &v) noexcept {
    return v * s;
}

[[nodiscard]] inline constexpr Vec2 operator/(const Vec2 &v1, const Vec2 &v2) noexcept {
    return Vec2(v1.x / v2.x, v1.y / v2.y);
}

[[nodiscard]] inline constexpr Vec2 operator/(const Vec2 &v, double s) noexcept {
    assert(std::fabs(s) > vec2_epsilon);
    return Vec2(v.x / s, v.y / s);
}

[[nodiscard]] inline double inverse_length(const Vec2 &v) noexcept {
    const double len = v.length();
    assert(std::fabs(len) > vec2_epsilon);
    return 1.0 / len;
}

[[nodiscard]] inline Vec2 normalize(const Vec2 &v) noexcept {
    return v * inverse_length(v);
}

[[nodiscard]] inline constexpr double dot(const Vec2 &v1, const Vec2 &v2) noexcept {
    return v1.x * v2.x + v1.y * v2.y;
}

[[nodiscard]] inline constexpr Vec2 project(const Vec2 &v, const Vec2 &n) noexcept {
    return dot(v, n) * n;
}

[[nodiscard]] inline constexpr Vec2 reflect(const Vec2 &v, const Vec2 &n) noexcept {
    return v - (2 * dot(v, n) * n);
}
