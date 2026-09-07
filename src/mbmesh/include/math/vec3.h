#pragma once

#include <cassert>
#include <cmath>
#include <ostream>

constexpr double vec3_epsilon = 1e-8;

class Vec3 {
public:
    double x;
    double y;
    double z;

    constexpr Vec3() noexcept : x(0.0), y(0.0), z(0.0) {}
    constexpr Vec3(double value) noexcept : x(value), y(value), z(value) {}
    constexpr Vec3(double x_value, double y_value, double z_value) noexcept : x(x_value), y(y_value), z(z_value) {}

    [[nodiscard]] constexpr double &operator[](int i) noexcept {
        assert(i >= 0 && i < 3);
        return i == 0 ? x : (i == 1 ? y : z);
    }

    [[nodiscard]] constexpr const double &operator[](int i) const noexcept {
        assert(i >= 0 && i < 3);
        return i == 0 ? x : (i == 1 ? y : z);
    }

    [[nodiscard]] constexpr Vec3 operator-() const noexcept { return Vec3(-x, -y, -z); }

    constexpr Vec3 &operator+=(const Vec3 &v) noexcept {
        x += v.x;
        y += v.y;
        z += v.z;
        return *this;
    }

    constexpr Vec3 &operator-=(const Vec3 &v) noexcept {
        x -= v.x;
        y -= v.y;
        z -= v.z;
        return *this;
    }

    constexpr Vec3 &operator*=(double s) noexcept {
        x *= s;
        y *= s;
        z *= s;
        return *this;
    }

    constexpr Vec3 &operator/=(double s) noexcept {
        assert(std::fabs(s) > vec3_epsilon);
        x /= s;
        y /= s;
        z /= s;
        return *this;
    }

    [[nodiscard]] constexpr double length_squared() const noexcept { return x * x + y * y + z * z; }
    [[nodiscard]] double length() const noexcept { return std::sqrt(length_squared()); }
};

inline std::ostream &operator<<(std::ostream &out, const Vec3 &v) {
    return out << "(" << v.x << ", " << v.y << ", " << v.z << ")";
}

[[nodiscard]] inline constexpr Vec3 operator+(const Vec3 &v1, const Vec3 &v2) noexcept {
    return Vec3(v1.x + v2.x, v1.y + v2.y, v1.z + v2.z);
}

[[nodiscard]] inline constexpr Vec3 operator-(const Vec3 &v1, const Vec3 &v2) noexcept {
    return Vec3(v1.x - v2.x, v1.y - v2.y, v1.z - v2.z);
}

[[nodiscard]] inline constexpr Vec3 operator*(const Vec3 &v1, const Vec3 &v2) noexcept {
    return Vec3(v1.x * v2.x, v1.y * v2.y, v1.z * v2.z);
}

[[nodiscard]] inline constexpr Vec3 operator*(const Vec3 &v, double s) noexcept {
    return Vec3(v.x * s, v.y * s, v.z * s);
}

[[nodiscard]] inline constexpr Vec3 operator*(double s, const Vec3 &v) noexcept {
    return v * s;
}

[[nodiscard]] inline constexpr Vec3 operator/(const Vec3 &v1, const Vec3 &v2) noexcept {
    return Vec3(v1.x / v2.x, v1.y / v2.y, v1.z / v2.z);
}

[[nodiscard]] inline constexpr Vec3 operator/(const Vec3 &v, double s) noexcept {
    assert(std::fabs(s) > vec3_epsilon);
    return Vec3(v.x / s, v.y / s, v.z / s);
}

[[nodiscard]] inline double inverse_length(const Vec3 &v) noexcept {
    const double len = v.length();
    if (std::fabs(len) <= vec3_epsilon) {
        return 0.0;
    }
    return 1.0 / len;
}

[[nodiscard]] inline Vec3 normalize(const Vec3 &v) noexcept {
    const double inv_len = inverse_length(v);
    if (inv_len == 0.0) {
        return Vec3(0.0, 0.0, 1.0);
    }
    return v * inv_len;
}

[[nodiscard]] inline constexpr double dot(const Vec3 &v1, const Vec3 &v2) noexcept {
    return v1.x * v2.x + v1.y * v2.y + v1.z * v2.z;
}

[[nodiscard]] inline constexpr Vec3 cross(const Vec3 &v1, const Vec3 &v2) noexcept {
    return Vec3(v1.y * v2.z - v1.z * v2.y, v1.z * v2.x - v1.x * v2.z, v1.x * v2.y - v1.y * v2.x);
}

[[nodiscard]] inline constexpr Vec3 project(const Vec3 &v, const Vec3 &n) noexcept {
    return dot(v, n) * n;
}

[[nodiscard]] inline constexpr Vec3 reflect(const Vec3 &v, const Vec3 &n) noexcept {
    return v - (2 * dot(v, n) * n);
}

[[nodiscard]] inline constexpr double distance_squared(const Vec3 &a, const Vec3 &b) noexcept {
    return (a - b).length_squared();
}
