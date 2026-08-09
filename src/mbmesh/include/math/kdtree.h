#pragma once

#include "vec3.h"

#include <algorithm>
#include <cstddef>
#include <limits>
#include <queue>
#include <vector>

class KDTree {
public:
    struct Neighbor {
        std::size_t index;
        double distance_squared;
    };

    KDTree() = default;

    explicit KDTree(const std::vector<Vec3> &points) {
        build(points);
    }

    void build(const std::vector<Vec3> &points) {
        nodes.clear();
        root = invalid_node;

        if (points.empty()) {
            return;
        }

        std::vector<std::size_t> indices(points.size());
        for (std::size_t i = 0; i < points.size(); i++) {
            indices[i] = i;
        }

        nodes.reserve(points.size());
        root = build_recursive(points, indices, 0, indices.size(), 0);
    }

    [[nodiscard]] bool is_empty() const noexcept {
        return nodes.empty();
    }

    [[nodiscard]] std::size_t size() const noexcept {
        return nodes.size();
    }

    [[nodiscard]] std::vector<Neighbor> k_nearest(const Vec3 &query, std::size_t k,
                                                  std::size_t exclude_index = invalid_index()) const {
        std::vector<Neighbor> neighbors;
        if (root == invalid_node || k == 0) {
            return neighbors;
        }

        NeighborQueue queue;
        k_nearest_recursive(root, query, k, exclude_index, queue);

        neighbors.reserve(queue.size());
        while (!queue.empty()) {
            neighbors.push_back(queue.top());
            queue.pop();
        }

        std::sort(neighbors.begin(), neighbors.end(), closer_neighbor);
        return neighbors;
    }

    [[nodiscard]] std::vector<Neighbor> radius_search_squared(const Vec3 &query, double radius_squared,
                                                              std::size_t exclude_index = invalid_index()) const {
        std::vector<Neighbor> neighbors;
        if (root == invalid_node || radius_squared < 0.0) {
            return neighbors;
        }

        radius_search_squared_recursive(root, query, radius_squared, exclude_index, neighbors);
        std::sort(neighbors.begin(), neighbors.end(), closer_neighbor);
        return neighbors;
    }

    [[nodiscard]] std::vector<Neighbor> bounded_radius_search_squared(
        const Vec3 &query,
        double radius_squared,
        std::size_t max_neighbors,
        std::size_t exclude_index = invalid_index()) const {
        std::vector<Neighbor> neighbors;
        if (root == invalid_node || radius_squared < 0.0 || max_neighbors == 0) {
            return neighbors;
        }

        NeighborQueue queue;
        bounded_radius_search_squared_recursive(root, query, radius_squared, max_neighbors, exclude_index, queue);

        neighbors.reserve(queue.size());
        while (!queue.empty()) {
            neighbors.push_back(queue.top());
            queue.pop();
        }

        std::sort(neighbors.begin(), neighbors.end(), closer_neighbor);
        return neighbors;
    }

    [[nodiscard]] static constexpr std::size_t invalid_index() noexcept {
        return std::numeric_limits<std::size_t>::max();
    }

private:
    static constexpr int invalid_node = -1;

    struct Node {
        Vec3 point;
        std::size_t index;
        int left;
        int right;
        int axis;
    };

    struct FartherNeighbor {
        [[nodiscard]] bool operator()(const Neighbor &a, const Neighbor &b) const noexcept {
            return a.distance_squared < b.distance_squared;
        }
    };

    using NeighborQueue = std::priority_queue<Neighbor, std::vector<Neighbor>, FartherNeighbor>;

    [[nodiscard]] static constexpr double coordinate(const Vec3 &point, int axis) noexcept {
        return point[axis];
    }

    [[nodiscard]] static constexpr double distance_squared(const Vec3 &a, const Vec3 &b) noexcept {
        return (a - b).length_squared();
    }

    [[nodiscard]] static bool closer_neighbor(const Neighbor &a, const Neighbor &b) noexcept {
        if (a.distance_squared == b.distance_squared) {
            return a.index < b.index;
        }
        return a.distance_squared < b.distance_squared;
    }

    int build_recursive(const std::vector<Vec3> &points, std::vector<std::size_t> &indices,
                        std::size_t begin, std::size_t end, int depth) {
        if (begin >= end) {
            return invalid_node;
        }

        const int axis = depth % 3;
        const std::size_t middle = begin + (end - begin) / 2;

        std::nth_element(indices.begin() + begin, indices.begin() + middle, indices.begin() + end,
                         [&points, axis](std::size_t a, std::size_t b) {
                             const double av = coordinate(points[a], axis);
                             const double bv = coordinate(points[b], axis);
                             if (av == bv) {
                                 return a < b;
                             }
                             return av < bv;
                         });

        const int node_index = static_cast<int>(nodes.size());
        nodes.push_back({points[indices[middle]], indices[middle], invalid_node, invalid_node, axis});

        nodes[node_index].left = build_recursive(points, indices, begin, middle, depth + 1);
        nodes[node_index].right = build_recursive(points, indices, middle + 1, end, depth + 1);
        return node_index;
    }

    void k_nearest_recursive(int node_index, const Vec3 &query, std::size_t k,
                             std::size_t exclude_index, NeighborQueue &queue) const {
        if (node_index == invalid_node) {
            return;
        }

        const Node &node = nodes[node_index];
        const double d2 = distance_squared(query, node.point);
        if (node.index != exclude_index) {
            const Neighbor candidate{node.index, d2};
            if (queue.size() < k) {
                queue.push(candidate);
            }
            else if (closer_neighbor(candidate, queue.top())) {
                queue.pop();
                queue.push(candidate);
            }
        }

        const double delta = coordinate(query, node.axis) - coordinate(node.point, node.axis);
        const int near_child = delta < 0.0 ? node.left : node.right;
        const int far_child = delta < 0.0 ? node.right : node.left;

        k_nearest_recursive(near_child, query, k, exclude_index, queue);

        const double worst_distance_squared = queue.size() < k ? std::numeric_limits<double>::max() : queue.top().distance_squared;
        if (delta * delta <= worst_distance_squared) {
            k_nearest_recursive(far_child, query, k, exclude_index, queue);
        }
    }

    void radius_search_squared_recursive(int node_index, const Vec3 &query, double radius_squared,
                                         std::size_t exclude_index, std::vector<Neighbor> &neighbors) const {
        if (node_index == invalid_node) {
            return;
        }

        const Node &node = nodes[node_index];
        const double d2 = distance_squared(query, node.point);
        if (node.index != exclude_index && d2 <= radius_squared) {
            neighbors.push_back({node.index, d2});
        }

        const double delta = coordinate(query, node.axis) - coordinate(node.point, node.axis);
        const int near_child = delta < 0.0 ? node.left : node.right;
        const int far_child = delta < 0.0 ? node.right : node.left;

        radius_search_squared_recursive(near_child, query, radius_squared, exclude_index, neighbors);
        if (delta * delta <= radius_squared) {
            radius_search_squared_recursive(far_child, query, radius_squared, exclude_index, neighbors);
        }
    }

    void bounded_radius_search_squared_recursive(
        int node_index,
        const Vec3 &query,
        double radius_squared,
        std::size_t max_neighbors,
        std::size_t exclude_index,
        NeighborQueue &queue) const {
        if (node_index == invalid_node) {
            return;
        }

        const Node &node = nodes[node_index];
        const double d2 = distance_squared(query, node.point);
        if (node.index != exclude_index && d2 <= radius_squared) {
            const Neighbor candidate{node.index, d2};
            if (queue.size() < max_neighbors) {
                queue.push(candidate);
            }
            else if (closer_neighbor(candidate, queue.top())) {
                queue.pop();
                queue.push(candidate);
            }
        }

        const double delta = coordinate(query, node.axis) - coordinate(node.point, node.axis);
        const int near_child = delta < 0.0 ? node.left : node.right;
        const int far_child = delta < 0.0 ? node.right : node.left;

        bounded_radius_search_squared_recursive(
            near_child,
            query,
            radius_squared,
            max_neighbors,
            exclude_index,
            queue);

        const double search_limit_squared =
            queue.size() < max_neighbors ? radius_squared : std::min(radius_squared, queue.top().distance_squared);
        if (delta * delta <= search_limit_squared) {
            bounded_radius_search_squared_recursive(
                far_child,
                query,
                radius_squared,
                max_neighbors,
                exclude_index,
                queue);
        }
    }

    std::vector<Node> nodes;
    int root = invalid_node;
};
