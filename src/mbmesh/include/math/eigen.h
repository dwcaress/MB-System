#pragma once

#include "mat3.h"
#include "vec3.h"

#include <algorithm>
#include <cmath>

struct EigenDecomposition {
    Vec3 values;
    Vec3 vectors[3];

    [[nodiscard]] int index_of_smallest_value() const noexcept {
        int index = 0;
        if (values.y < values.x) {
            index = 1;
        }
        if ((index == 0 && values.z < values.x) || (index == 1 && values.z < values.y)) {
            index = 2;
        }
        return index;
    }
};

[[nodiscard]] inline EigenDecomposition eigen_decomposition_symmetric(const Mat3 &matrix) noexcept {
    struct EigenPair {
        double value;
        Vec3 vector;
    };

    Mat3 a = matrix;
    Mat3 eigenvectors = Mat3::identity();

    for (int iteration = 0; iteration < 32; iteration++) {
        int p = 0;
        int q = 1;
        double largest = std::fabs(a.e[0][1]);

        if (std::fabs(a.e[0][2]) > largest) {
            p = 0;
            q = 2;
            largest = std::fabs(a.e[0][2]);
        }
        if (std::fabs(a.e[1][2]) > largest) {
            p = 1;
            q = 2;
            largest = std::fabs(a.e[1][2]);
        }
        if (largest < 1.0e-12) {
            break;
        }

        const double theta = 0.5 * std::atan2(2.0 * a.e[p][q], a.e[q][q] - a.e[p][p]);
        const double c = std::cos(theta);
        const double s = std::sin(theta);

        const double app = c * c * a.e[p][p] - 2.0 * s * c * a.e[p][q] + s * s * a.e[q][q];
        const double aqq = s * s * a.e[p][p] + 2.0 * s * c * a.e[p][q] + c * c * a.e[q][q];
        a.e[p][p] = app;
        a.e[q][q] = aqq;
        a.e[p][q] = 0.0;
        a.e[q][p] = 0.0;

        for (int r = 0; r < 3; r++) {
            if (r != p && r != q) {
                const double arp = c * a.e[r][p] - s * a.e[r][q];
                const double arq = s * a.e[r][p] + c * a.e[r][q];
                a.e[r][p] = arp;
                a.e[p][r] = arp;
                a.e[r][q] = arq;
                a.e[q][r] = arq;
            }

            const double vrp = c * eigenvectors.e[r][p] - s * eigenvectors.e[r][q];
            const double vrq = s * eigenvectors.e[r][p] + c * eigenvectors.e[r][q];
            eigenvectors.e[r][p] = vrp;
            eigenvectors.e[r][q] = vrq;
        }
    }

    EigenPair pairs[3] = {
        {a.e[0][0], Vec3(eigenvectors.e[0][0], eigenvectors.e[1][0], eigenvectors.e[2][0])},
        {a.e[1][1], Vec3(eigenvectors.e[0][1], eigenvectors.e[1][1], eigenvectors.e[2][1])},
        {a.e[2][2], Vec3(eigenvectors.e[0][2], eigenvectors.e[1][2], eigenvectors.e[2][2])},
    };

    std::sort(pairs, pairs + 3, [](const EigenPair &a_pair, const EigenPair &b_pair) {
        return a_pair.value < b_pair.value;
    });

    EigenDecomposition result;
    result.values = Vec3(pairs[0].value, pairs[1].value, pairs[2].value);
    result.vectors[0] = normalize(pairs[0].vector);
    result.vectors[1] = normalize(pairs[1].vector);
    result.vectors[2] = normalize(pairs[2].vector);
    return result;
}
