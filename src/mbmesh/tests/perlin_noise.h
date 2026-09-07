#pragma once

namespace {

[[nodiscard]] double fade(double t) {
    return t * t * t * (t * (t * 6.0 - 15.0) + 10.0);
}

[[nodiscard]] double lerp(double a, double b, double t) {
    return a + t * (b - a);
}

[[nodiscard]] int fast_floor(double value) {
    const int i = static_cast<int>(value);
    return value < static_cast<double>(i) ? i - 1 : i;
}

[[nodiscard]] int hash_lattice(int x, int y, int z) {
    int h = x * 374761393 + y * 668265263 + z * 2147483647;
    h = (h ^ (h >> 13)) * 1274126177;
    return h ^ (h >> 16);
}

[[nodiscard]] double gradient(int hash, double x, double y, double z) {
    switch (hash & 15) {
    case 0:
        return x + y;
    case 1:
        return -x + y;
    case 2:
        return x - y;
    case 3:
        return -x - y;
    case 4:
        return x + z;
    case 5:
        return -x + z;
    case 6:
        return x - z;
    case 7:
        return -x - z;
    case 8:
        return y + z;
    case 9:
        return -y + z;
    case 10:
        return y - z;
    case 11:
        return -y - z;
    case 12:
        return x + y;
    case 13:
        return -x + y;
    case 14:
        return -y + z;
    default:
        return -y - z;
    }
}

} // namespace

double perlin_noise(double x, double y, double z) {
    const int x0 = fast_floor(x);
    const int y0 = fast_floor(y);
    const int z0 = fast_floor(z);

    const double xf = x - static_cast<double>(x0);
    const double yf = y - static_cast<double>(y0);
    const double zf = z - static_cast<double>(z0);

    const double u = fade(xf);
    const double v = fade(yf);
    const double w = fade(zf);

    const double n000 = gradient(hash_lattice(x0, y0, z0), xf, yf, zf);
    const double n100 = gradient(hash_lattice(x0 + 1, y0, z0), xf - 1.0, yf, zf);
    const double n010 = gradient(hash_lattice(x0, y0 + 1, z0), xf, yf - 1.0, zf);
    const double n110 = gradient(hash_lattice(x0 + 1, y0 + 1, z0), xf - 1.0, yf - 1.0, zf);
    const double n001 = gradient(hash_lattice(x0, y0, z0 + 1), xf, yf, zf - 1.0);
    const double n101 = gradient(hash_lattice(x0 + 1, y0, z0 + 1), xf - 1.0, yf, zf - 1.0);
    const double n011 = gradient(hash_lattice(x0, y0 + 1, z0 + 1), xf, yf - 1.0, zf - 1.0);
    const double n111 = gradient(hash_lattice(x0 + 1, y0 + 1, z0 + 1), xf - 1.0, yf - 1.0, zf - 1.0);

    const double x00 = lerp(n000, n100, u);
    const double x10 = lerp(n010, n110, u);
    const double x01 = lerp(n001, n101, u);
    const double x11 = lerp(n011, n111, u);

    const double y0_value = lerp(x00, x10, v);
    const double y1_value = lerp(x01, x11, v);
    return lerp(y0_value, y1_value, w);
}