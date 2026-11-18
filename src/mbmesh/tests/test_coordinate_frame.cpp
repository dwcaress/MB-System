#include "io/coordinate_frame.h"

#include <cmath>
#include <cstdio>

namespace {

bool check(bool condition, const char *message) {
    if (!condition) {
        std::fprintf(stderr, "FAIL: %s\n", message);
    }
    return condition;
}

bool near(double a, double b, double tolerance) {
    return std::fabs(a - b) <= tolerance;
}

} // namespace

int main() {
    bool ok = true;

    // Known value: (0 deg, 0 deg, 0 m) is on the ECEF X axis at the semi-major axis.
    const Vec3 equator = geodetic_to_ecef({0.0, 0.0, 0.0});
    ok &= check(near(equator.x, 6378137.0, 1e-6) && near(equator.y, 0.0, 1e-6) && near(equator.z, 0.0, 1e-6),
                "geodetic_to_ecef at lon=0 lat=0");

    // Geodetic <-> ECEF round trip, including deep points, high latitude and the poles.
    const GeodeticPosition samples[] = {
        {-122.0, 36.8, -1500.0}, {-122.0, 36.8, 0.0},  {170.0, -45.0, -4000.0},
        {-179.9, 0.5, -10.0},    {10.0, 89.9999, -50.0}, {0.0, -90.0, -100.0},
    };
    for (const GeodeticPosition &sample : samples) {
        const GeodeticPosition back = ecef_to_geodetic(geodetic_to_ecef(sample));
        const double lon_error = std::fabs(std::remainder(back.longitude - sample.longitude, 360.0));
        const bool pole = std::fabs(sample.latitude) > 89.99999;
        ok &= check((pole || lon_error < 1e-9) && near(back.latitude, sample.latitude, 1e-9) &&
                        near(back.elevation, sample.elevation, 1e-4),
                    "geodetic/ECEF round trip");
    }

    // Frame axes are orthonormal and right-handed (east x north = up).
    const CoordinateFrame frame = coordinate_frame_from_origin({-122.0, 36.8, 0.0});
    ok &= check(is_valid_frame(frame), "frame is valid");
    ok &= check(near(dot(frame.east, frame.east), 1.0, 1e-12) && near(dot(frame.north, frame.north), 1.0, 1e-12) &&
                    near(dot(frame.up, frame.up), 1.0, 1e-12) && near(dot(frame.east, frame.north), 0.0, 1e-12) &&
                    near(dot(frame.east, frame.up), 0.0, 1e-12) && near(dot(frame.north, frame.up), 0.0, 1e-12),
                "frame axes orthonormal");
    const Vec3 east_cross_north = cross(frame.east, frame.north);
    ok &= check(near(east_cross_north.x, frame.up.x, 1e-12) && near(east_cross_north.y, frame.up.y, 1e-12) &&
                    near(east_cross_north.z, frame.up.z, 1e-12),
                "frame is right-handed");

    // The origin maps to (0,0,0); local <-> ECEF and local <-> geodetic round trip.
    const Vec3 origin_local = geodetic_to_local(frame.origin, frame);
    ok &= check(near(origin_local.x, 0.0, 1e-6) && near(origin_local.y, 0.0, 1e-6) && near(origin_local.z, 0.0, 1e-6),
                "origin maps to local zero");

    const Vec3 local(12345.0, -6789.0, -1500.0);
    const Vec3 round_trip = ecef_to_local(local_to_ecef(local, frame), frame);
    ok &= check(near(round_trip.x, local.x, 1e-6) && near(round_trip.y, local.y, 1e-6) && near(round_trip.z, local.z, 1e-6),
                "local/ECEF round trip");

    // 1 km due north: no equirectangular error in y, and the tangent plane
    // leaves the surface by d^2 / (2 R) ~= 0.078 m, so the surface point is below the plane.
    const GeodeticPosition north_1km = {frame.origin.longitude, frame.origin.latitude + 1000.0 / 111000.0, 0.0};
    const Vec3 north_local = geodetic_to_local(north_1km, frame);
    ok &= check(near(north_local.x, 0.0, 0.01) && near(north_local.y, 1000.0, 5.0) && north_local.z < 0.0 &&
                    near(north_local.z, -0.078, 0.01),
                "1 km north shows tangent-plane curvature drop");

    // Directions rotate without translation.
    const Vec3 up_in_ecef = local_vector_to_ecef(Vec3(0.0, 0.0, 1.0), frame);
    ok &= check(near(up_in_ecef.x, frame.up.x, 1e-12) && near(up_in_ecef.z, frame.up.z, 1e-12), "vector rotation");

    std::printf(ok ? "coordinate frame tests passed\n" : "coordinate frame tests FAILED\n");
    return ok ? 0 : 1;
}
