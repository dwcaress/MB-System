#include "algorithms/normal_estimation.h"

Vec3 direction_to_origin(CollectedPoint collected_point) {
    const Vec3 direction = collected_point.origin - collected_point.point;
    if (direction.length_squared() < vec3_epsilon * vec3_epsilon) {
        return  Vec3(0.0, 0.0, 1.0);
    } else {
        return normalize(direction);
    }
}

OrientedPointCloud normal_estimation(
    const CollectedPointCloud &collected_points,
    NormalEstimationOptions options
) {

    OrientedPointCloud oriented_points;
    oriented_points.reserve(collected_points.size());

    // Extract the points from the CollectedPoint struct for use with the
    // KDTree. Ideally the KDTree data structure would accept an accessor
    // function so that arbitrary point collections can be passed in without
    // copying
    std::vector<Vec3> points;
    points.reserve(collected_points.size());
    for (const CollectedPoint &collected_point : collected_points) {
        points.push_back(collected_point.point);
    }

    // Create a KDTree from the points so that we can find their nearest
    // neighbors quickly
    KDTree tree(points);
    using Neighbors = std::vector<KDTree::Neighbor>;

    for (std::size_t i = 0; i < collected_points.size(); i++) {

        Neighbors neighbors;

        // Prefer a physical-radius neighborhood so normal smoothing follows
        // the requested level of detail instead of raw point density. Keep the
        // query bounded by k so dense surveys do not collect every sample in
        // the physical radius before trimming the neighborhood.
        if (options.search_radius > 0.0) {
            if (options.k > 0) {
                neighbors = tree.bounded_radius_search_squared(
                    points[i],
                    options.search_radius * options.search_radius,
                    options.k,
                    i);
            } else {
                neighbors = tree.radius_search_squared(points[i], options.search_radius * options.search_radius, i);
            }
        }
        if (neighbors.size() < options.minimum_neighbors) {
            neighbors = tree.k_nearest(points[i], options.k, i);
        }

        // CollectedPoint stores the location of the point and the sensor, to
        // get the direction to the sensor we need to calculate and normalize.
        const Vec3 to_origin = direction_to_origin(collected_points[i]);

        // PCA needs at least three points. If there are less than three, we
        // default to using the to_sensor vector as the normal.
        if (neighbors.size() + 1 < 3) {
            OrientedPoint oriented_point(points[i], to_origin);
            oriented_points.push_back(oriented_point);
            continue;
        }

        // Find the centroid of the query point and its neighbors.
        const std::size_t neighborhood_size = neighbors.size() + 1;
        Vec3 local_center = points[i];
        for (std::size_t j = 0; j < neighbors.size(); j++) {
            local_center += points[neighbors[j].index];
        }
        local_center /= static_cast<double>(neighborhood_size);

        // Compute the covariance matrix of the query point and its neighbors.
        Mat3 covariance;
        const Vec3 query_delta = points[i] - local_center;
        covariance.add_outer_product(query_delta);
        for (std::size_t j = 0; j < neighbors.size(); j++) {
            const Vec3 delta = points[neighbors[j].index] - local_center;
            covariance.add_outer_product(delta);
        }
        covariance /= static_cast<double>(neighborhood_size);

        // Find the direction of least variance using Eigen decomposition and
        // use it as the point's normal
        const EigenDecomposition eig = eigen_decomposition_symmetric(covariance);
        const int normal_index = eig.index_of_smallest_value();
        Vec3 normal = normalize(eig.vectors[normal_index]);

        // PCA produces an arbitrarily oriented normal respective to the plane
        // defined by the eigen decomposition, so to determine the proper
        // orientation we use the vector from the point to the sensor origin.
        if (dot(normal, to_origin) < 0.0) {
            normal = -normal;
        }

        // Store the point and normal in the OrientedPoint vector. We also
        // preserve the lambda values of the eigen decomposition as they can be
        // useful for other purposes
        OrientedPoint oriented_point(points[i], normal);
        oriented_point.lambdas = eig.values;
        oriented_points.push_back(oriented_point);
    }

    return oriented_points;
}
