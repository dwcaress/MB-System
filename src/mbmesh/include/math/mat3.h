#pragma once

#include "vec3.h"

#include <cassert>
#include <cmath>
#include <ostream>

constexpr double mat3_epsilon = 1e-8;

class Mat3 {
public:
    double e[3][3];

    constexpr Mat3() noexcept : e{{0.0, 0.0, 0.0}, {0.0, 0.0, 0.0}, {0.0, 0.0, 0.0}} {}

    constexpr Mat3(double value) noexcept : e{{value, value, value}, {value, value, value}, {value, value, value}} {}

    constexpr Mat3(double e00, double e01, double e02,
                   double e10, double e11, double e12,
                   double e20, double e21, double e22) noexcept :
        e{{e00, e01, e02}, {e10, e11, e12}, {e20, e21, e22}} {}

    [[nodiscard]] constexpr double *operator[](int row) noexcept {
        assert(row >= 0 && row < 3);
        return e[row];
    }

    [[nodiscard]] constexpr const double *operator[](int row) const noexcept {
        assert(row >= 0 && row < 3);
        return e[row];
    }

    [[nodiscard]] constexpr Mat3 operator-() const noexcept {
        return Mat3(-e[0][0], -e[0][1], -e[0][2],
                    -e[1][0], -e[1][1], -e[1][2],
                    -e[2][0], -e[2][1], -e[2][2]);
    }

    constexpr Mat3 &operator+=(const Mat3 &m) noexcept {
        for (int row = 0; row < 3; row++) {
            for (int col = 0; col < 3; col++) {
                e[row][col] += m.e[row][col];
            }
        }
        return *this;
    }

    constexpr Mat3 &operator-=(const Mat3 &m) noexcept {
        for (int row = 0; row < 3; row++) {
            for (int col = 0; col < 3; col++) {
                e[row][col] -= m.e[row][col];
            }
        }
        return *this;
    }

    constexpr Mat3 &operator*=(double s) noexcept {
        for (int row = 0; row < 3; row++) {
            for (int col = 0; col < 3; col++) {
                e[row][col] *= s;
            }
        }
        return *this;
    }

    constexpr Mat3 &operator/=(double s) noexcept {
        assert(std::fabs(s) > mat3_epsilon);
        return *this *= 1.0 / s;
    }

    constexpr void add_outer_product(const Vec3 &v) noexcept {
        e[0][0] += v.x * v.x;
        e[0][1] += v.x * v.y;
        e[0][2] += v.x * v.z;
        e[1][0] += v.y * v.x;
        e[1][1] += v.y * v.y;
        e[1][2] += v.y * v.z;
        e[2][0] += v.z * v.x;
        e[2][1] += v.z * v.y;
        e[2][2] += v.z * v.z;
    }

    [[nodiscard]] static constexpr Mat3 identity() noexcept {
        return Mat3(1.0, 0.0, 0.0,
                    0.0, 1.0, 0.0,
                    0.0, 0.0, 1.0);
    }
};

inline std::ostream &operator<<(std::ostream &out, const Mat3 &m) {
    return out << "((" << m.e[0][0] << ", " << m.e[0][1] << ", " << m.e[0][2] << "), "
               << "(" << m.e[1][0] << ", " << m.e[1][1] << ", " << m.e[1][2] << "), "
               << "(" << m.e[2][0] << ", " << m.e[2][1] << ", " << m.e[2][2] << "))";
}

[[nodiscard]] inline constexpr Mat3 operator+(const Mat3 &m1, const Mat3 &m2) noexcept {
    return Mat3(m1.e[0][0] + m2.e[0][0], m1.e[0][1] + m2.e[0][1], m1.e[0][2] + m2.e[0][2],
                m1.e[1][0] + m2.e[1][0], m1.e[1][1] + m2.e[1][1], m1.e[1][2] + m2.e[1][2],
                m1.e[2][0] + m2.e[2][0], m1.e[2][1] + m2.e[2][1], m1.e[2][2] + m2.e[2][2]);
}

[[nodiscard]] inline constexpr Mat3 operator-(const Mat3 &m1, const Mat3 &m2) noexcept {
    return Mat3(m1.e[0][0] - m2.e[0][0], m1.e[0][1] - m2.e[0][1], m1.e[0][2] - m2.e[0][2],
                m1.e[1][0] - m2.e[1][0], m1.e[1][1] - m2.e[1][1], m1.e[1][2] - m2.e[1][2],
                m1.e[2][0] - m2.e[2][0], m1.e[2][1] - m2.e[2][1], m1.e[2][2] - m2.e[2][2]);
}

[[nodiscard]] inline constexpr Mat3 operator*(const Mat3 &m, double s) noexcept {
    return Mat3(m.e[0][0] * s, m.e[0][1] * s, m.e[0][2] * s,
                m.e[1][0] * s, m.e[1][1] * s, m.e[1][2] * s,
                m.e[2][0] * s, m.e[2][1] * s, m.e[2][2] * s);
}

[[nodiscard]] inline constexpr Mat3 operator*(double s, const Mat3 &m) noexcept {
    return m * s;
}

[[nodiscard]] inline constexpr Mat3 operator/(const Mat3 &m, double s) noexcept {
    assert(std::fabs(s) > mat3_epsilon);
    return m * (1.0 / s);
}

[[nodiscard]] inline constexpr Vec3 operator*(const Mat3 &m, const Vec3 &v) noexcept {
    return Vec3(m.e[0][0] * v.x + m.e[0][1] * v.y + m.e[0][2] * v.z,
                m.e[1][0] * v.x + m.e[1][1] * v.y + m.e[1][2] * v.z,
                m.e[2][0] * v.x + m.e[2][1] * v.y + m.e[2][2] * v.z);
}

[[nodiscard]] inline constexpr Mat3 operator*(const Mat3 &m1, const Mat3 &m2) noexcept {
    return Mat3(
        m1.e[0][0] * m2.e[0][0] + m1.e[0][1] * m2.e[1][0] + m1.e[0][2] * m2.e[2][0],
        m1.e[0][0] * m2.e[0][1] + m1.e[0][1] * m2.e[1][1] + m1.e[0][2] * m2.e[2][1],
        m1.e[0][0] * m2.e[0][2] + m1.e[0][1] * m2.e[1][2] + m1.e[0][2] * m2.e[2][2],
        m1.e[1][0] * m2.e[0][0] + m1.e[1][1] * m2.e[1][0] + m1.e[1][2] * m2.e[2][0],
        m1.e[1][0] * m2.e[0][1] + m1.e[1][1] * m2.e[1][1] + m1.e[1][2] * m2.e[2][1],
        m1.e[1][0] * m2.e[0][2] + m1.e[1][1] * m2.e[1][2] + m1.e[1][2] * m2.e[2][2],
        m1.e[2][0] * m2.e[0][0] + m1.e[2][1] * m2.e[1][0] + m1.e[2][2] * m2.e[2][0],
        m1.e[2][0] * m2.e[0][1] + m1.e[2][1] * m2.e[1][1] + m1.e[2][2] * m2.e[2][1],
        m1.e[2][0] * m2.e[0][2] + m1.e[2][1] * m2.e[1][2] + m1.e[2][2] * m2.e[2][2]);
}

[[nodiscard]] inline constexpr Mat3 transpose(const Mat3 &m) noexcept {
    return Mat3(m.e[0][0], m.e[1][0], m.e[2][0],
                m.e[0][1], m.e[1][1], m.e[2][1],
                m.e[0][2], m.e[1][2], m.e[2][2]);
}

[[nodiscard]] inline constexpr Mat3 outer_product(const Vec3 &v1, const Vec3 &v2) noexcept {
    return Mat3(v1.x * v2.x, v1.x * v2.y, v1.x * v2.z,
                v1.y * v2.x, v1.y * v2.y, v1.y * v2.z,
                v1.z * v2.x, v1.z * v2.y, v1.z * v2.z);
}
