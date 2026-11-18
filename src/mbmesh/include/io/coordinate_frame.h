#pragma once

#include "../data_types/swathfile.h"

/*
 * Conversions between WGS84 geodetic coordinates, WGS84 Earth-Centered
 * Earth-Fixed (ECEF) coordinates, and the local east-north-up (ENU) frame
 * that mbmesh uses for reconstruction.
 *
 * The local frame is a rigid transform of ECEF (a translation to the frame
 * origin and a rotation onto the origin's tangent plane). It introduces no
 * map-projection distortion, so local meshes can be placed back on the Earth
 * exactly with CoordinateFrame::origin_ecef and the east/north/up axes. That
 * is the 4x4 matrix a 3D Tiles tile "transform" needs.
 */

[[nodiscard]] CoordinateFrame coordinate_frame_from_origin(
    const GeodeticPosition &origin);

[[nodiscard]] Vec3 geodetic_to_ecef(
    const GeodeticPosition &position);

[[nodiscard]] GeodeticPosition ecef_to_geodetic(
    const Vec3 &ecef);

[[nodiscard]] Vec3 ecef_to_local(
    const Vec3 &ecef,
    const CoordinateFrame &frame);

[[nodiscard]] Vec3 local_to_ecef(
    const Vec3 &local,
    const CoordinateFrame &frame);

[[nodiscard]] Vec3 geodetic_to_local(
    const GeodeticPosition &position,
    const CoordinateFrame &frame);

[[nodiscard]] GeodeticPosition local_to_geodetic(
    const Vec3 &local,
    const CoordinateFrame &frame);

// Rotates a direction (for example a normal) from the local frame into ECEF.
[[nodiscard]] Vec3 local_vector_to_ecef(
    const Vec3 &vector,
    const CoordinateFrame &frame);

[[nodiscard]] bool is_valid_frame(
    const CoordinateFrame &frame);
