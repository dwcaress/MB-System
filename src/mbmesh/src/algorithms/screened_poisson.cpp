 #include "algorithms/screened_poisson.h"

#include "util/parallel.h"
#include "util/stopwatch.h"

#include <algorithm>
#include <functional>
#include <thread>

const char *poisson_solver_name(PoissonSolver solver) {
	switch (solver) {
	case PoissonSolver::Jacobi:
		return "jacobi";
	case PoissonSolver::Sor:
		return "sor";
	}
	return "unknown";
}

const char *poisson_solver_names() {
	return "jacobi, sor";
}

bool parse_poisson_solver(const std::string &name, PoissonSolver *solver) {
	if (name == "jacobi") {
		*solver = PoissonSolver::Jacobi;
		return true;
	}
	if (name == "sor") {
		*solver = PoissonSolver::Sor;
		return true;
	}
	return false;
}

Vec3 cell_index(Vec3 point, Vec3 origin, double cell_size) {
	return Vec3(static_cast<int>(std::floor((point.x - origin.x) / cell_size)),
	            static_cast<int>(std::floor((point.y - origin.y) / cell_size)),
	            static_cast<int>(std::floor((point.z - origin.z) / cell_size)));
}

double finite_difference_component(const VectorGrid3D &vector_grid, int x, int y, int z, int component) {

	int prev_x = x;
	int prev_y = y;
	int prev_z = z;
	int next_x = x;
	int next_y = y;
	int next_z = z;

	if (component == 0) {
		prev_x = std::max(0, x - 1);
		next_x = std::min(vector_grid.nx - 1, x + 1);
	}
	else if (component == 1) {
		prev_y = std::max(0, y - 1);
		next_y = std::min(vector_grid.ny - 1, y + 1);
	}
	else {
		prev_z = std::max(0, z - 1);
		next_z = std::min(vector_grid.nz - 1, z + 1);
	}

	const int index_delta = (next_x - prev_x) + (next_y - prev_y) + (next_z - prev_z);
	if (index_delta == 0) {
		return 0.0;
	}

	const double prev_value = vector_grid.at(prev_x, prev_y, prev_z)[component];
	const double next_value = vector_grid.at(next_x, next_y, next_z)[component];
	return (next_value - prev_value) / (static_cast<double>(index_delta) * vector_grid.cell_size);
}

/******************************************************************************
 * Extracts an implicit surface from an OrientedPoint set.
 ******************************************************************************/

ScalarGrid3D screened_poisson(const OrientedPointCloud &oriented_points, ScreenedPoissonOptions options,
                              PoissonDiagnostics *diagnostics) {

	const mbmesh::Stopwatch splat_timer;

	if (oriented_points.empty() || options.cell_size <= 0.0 || options.normal_splat_radius <= 0.0) {
		return ScalarGrid3D();
	}

	/**************************************************************************
	 * Step 1: Build the reconstruction domain.
	 *
	 * The Poisson solve happens on a regular 3D grid around the samples.
	 * Padding gives the implicit field room to transition away from the
	 * observed surface instead of being clipped at the input bounds.
	 **************************************************************************/

	const ReconstructionDomain domain = options.use_fixed_domain
	                                        ? ReconstructionDomain(options.domain_bounds, options.cell_size)
	                                        : ReconstructionDomain(oriented_points, options.padding, options.cell_size);

	/**************************************************************************
	 * Step 2: Splat oriented samples into a vector field.
	 *
	 * Each oriented point contributes its unit normal to nearby grid nodes.
	 * The accumulated vector field is the discrete version of the normal field
	 * whose divergence will drive the Poisson equation.
	 **************************************************************************/

	VectorGrid3D accumulated_normals = VectorGrid3D(domain);
	ScalarGrid3D accumulated_weights = ScalarGrid3D(domain);

	for (const OrientedPoint &oriented_point : oriented_points) {

		const Vec3 center = cell_index(oriented_point.point, accumulated_normals.origin, accumulated_normals.cell_size);
		const int radius = std::max(1, static_cast<int>(std::ceil(options.normal_splat_radius / accumulated_normals.cell_size)));

		const double radius_squared = options.normal_splat_radius * options.normal_splat_radius;
		const Vec3 unit_normal = normalize(oriented_point.normal);

		for (int z = center.z - radius; z <= center.z + radius; z++) {
			for (int y = center.y - radius; y <= center.y + radius; y++) {
				for (int x = center.x - radius; x <= center.x + radius; x++) {

					if (!accumulated_normals.contains(x, y, z)) {
						continue;
					}

					const Vec3 cell_position = accumulated_normals.position(x, y, z);
					const double distance_squared = (cell_position - oriented_point.point).length_squared();

					double weight = 0.0;
					if (radius_squared <= 0.0 || distance_squared > radius_squared) {
						weight = 0.0;
					}
					else {
						const double normalized_distance_squared = distance_squared / radius_squared;
						const double falloff = 1.0 - normalized_distance_squared;
						weight = falloff * falloff;
					}

					if (weight <= vec3_epsilon) {
						continue;
					}

					accumulated_normals.at(x, y, z) += unit_normal * weight;
					accumulated_weights.at(x, y, z) += weight;
				}
			}
		}
	}

	for (int z = 0; z < accumulated_normals.nz; z++) {
		for (int y = 0; y < accumulated_normals.ny; y++) {
			for (int x = 0; x < accumulated_normals.nx; x++) {
				const double accumulated_weight = accumulated_weights.at(x, y, z);
				if (accumulated_weight <= vec3_epsilon) {
					continue;
				}

				accumulated_normals.at(x, y, z) /= accumulated_weight;
			}
		}
	}

	/**************************************************************************
	 * Step 3: Splat sample constraints for screened Poisson reconstruction.
	 *
	 * The normal field says what gradient the implicit function should have.
	 * Screening adds a direct surface-position constraint: near observed
	 * samples, the scalar field should stay close to zero.
	 **************************************************************************/

	ScalarGrid3D screening_targets = ScalarGrid3D(domain);
	ScalarGrid3D screening_weights = ScalarGrid3D(domain);

	for (const OrientedPoint &oriented_point : oriented_points) {

		const Vec3 center = cell_index(oriented_point.point, screening_weights.origin, screening_weights.cell_size);
		const int radius = std::max(1, static_cast<int>(std::ceil(options.normal_splat_radius / screening_weights.cell_size)));

		const double radius_squared = options.normal_splat_radius * options.normal_splat_radius;
		constexpr double surface_target_value = 0.0;

		for (int z = center.z - radius; z <= center.z + radius; z++) {
			for (int y = center.y - radius; y <= center.y + radius; y++) {
				for (int x = center.x - radius; x <= center.x + radius; x++) {
					if (!screening_weights.contains(x, y, z)) {
						continue;
					}

					const Vec3 cell_position = screening_weights.position(x, y, z);
					const double distance_squared = (cell_position - oriented_point.point).length_squared();

					double weight = 0.0;
					if (radius_squared <= 0.0 || distance_squared > radius_squared) {
						weight = 0.0;
					}
					else {
						const double normalized_distance_squared = distance_squared / radius_squared;
						const double falloff = 1.0 - normalized_distance_squared;
						weight = falloff * falloff;
					}

					if (weight <= 0.0) {
						continue;
					}

					screening_targets.at(x, y, z) += surface_target_value * weight;
					screening_weights.at(x, y, z) += weight;
				}
			}
		}
	}

	for (int z = 0; z < screening_targets.nz; z++) {
		for (int y = 0; y < screening_targets.ny; y++) {
			for (int x = 0; x < screening_targets.nx; x++) {
				const double weight = screening_weights.at(x, y, z);
				if (weight <= 0.0) {
					continue;
				}

				screening_targets.at(x, y, z) /= weight;
			}
		}
	}

	/**************************************************************************
	 * Step 4: Compute the Poisson right-hand side from the normal field
	 * divergence.
	 *
	 * Divergence turns the vector field of normals into a scalar source term.
	 * Intuitively, it marks where the splatted normals behave like they are
	 * flowing out of or into the grid, which is the signal that the scalar
	 * reconstruction solve will try to match.
	 **************************************************************************/

	ScalarGrid3D normal_divergence(domain);

	for (int z = 0; z < accumulated_normals.nz; z++) {
		for (int y = 0; y < accumulated_normals.ny; y++) {
			for (int x = 0; x < accumulated_normals.nx; x++) {
				const double normal_x_change_along_x = finite_difference_component(accumulated_normals, x, y, z, 0);
				const double normal_y_change_along_y = finite_difference_component(accumulated_normals, x, y, z, 1);
				const double normal_z_change_along_z = finite_difference_component(accumulated_normals, x, y, z, 2);
				normal_divergence.at(x, y, z) = normal_x_change_along_x + normal_y_change_along_y + normal_z_change_along_z;
			}
		}
	}

	/**************************************************************************
	 * Step 5: Solve the screened Poisson equation for a scalar indicator
	 * field.
	 *
	 * Without screening, this is laplacian(field) = divergence(normals). With
	 * screening enabled, nearby samples add soft constraints that pull the
	 * scalar field toward zero at the observed surface.
	 **************************************************************************/

	ScalarGrid3D scalar_indicator_field(domain);

	// Jacobi writes each sweep to a second grid; sor updates the field in place.
	ScalarGrid3D next_iteration;
	if (options.solver == PoissonSolver::Jacobi) {
		next_iteration = ScalarGrid3D(domain);
	}
	int iteration_count = std::max(0, options.solver_iterations);
	if (iteration_count == 0) {
		iteration_count = options.solver == PoissonSolver::Sor
		                      ? std::max({50, static_cast<int>(scalar_indicator_field.nx), static_cast<int>(scalar_indicator_field.ny),
		                                static_cast<int>(scalar_indicator_field.nz)})
		                      : 200;
	}
	const double cell_size_squared = scalar_indicator_field.cell_size * scalar_indicator_field.cell_size;

	// Nodes that hold the values of a neighbor and are not solved for.
	std::vector<unsigned char> fixed_nodes;
	std::size_t fixed_count = 0;
	if (options.fix_nodes) {
		fixed_nodes.assign(scalar_indicator_field.values.size(), 0);
		fixed_count = options.fix_nodes(&scalar_indicator_field, &fixed_nodes);
		if (fixed_count == 0) {
			fixed_nodes.clear();
		}
	}
	const bool has_fixed_nodes = !fixed_nodes.empty();

	// The equation at an interior cell is  diagonal * u = (sum of the six neighbors) + right_hand_side.
	const auto cell_equation = [&](int x, int y, int z, double *diagonal, double *right_hand_side) {
		*diagonal = 6.0;
		*right_hand_side = -cell_size_squared * normal_divergence.at(x, y, z);

		if (options.use_screening && options.screening_weight > 0.0) {
			const double sample_weight = screening_weights.at(x, y, z);
			if (sample_weight > 0.0) {
				const double scaled_screening_weight = cell_size_squared * options.screening_weight * sample_weight;
				*diagonal += scaled_screening_weight;
				*right_hand_side += scaled_screening_weight * screening_targets.at(x, y, z);
			}
		}
	};

	// Relative residual of the current field; see PoissonDiagnostics.
	const auto relative_residual = [&]() {
		double residual_sum = 0.0;
		double right_hand_side_sum = 0.0;
		for (int z = 1; z < scalar_indicator_field.nz - 1; z++) {
			for (int y = 1; y < scalar_indicator_field.ny - 1; y++) {
				for (int x = 1; x < scalar_indicator_field.nx - 1; x++) {
					if (has_fixed_nodes && fixed_nodes[scalar_indicator_field.index(x, y, z)] != 0) {
						continue;
					}
					double diagonal = 0.0;
					double right_hand_side = 0.0;
					cell_equation(x, y, z, &diagonal, &right_hand_side);
					const double neighbor_sum = scalar_indicator_field.at(x - 1, y, z) + scalar_indicator_field.at(x + 1, y, z) +
					                            scalar_indicator_field.at(x, y - 1, z) + scalar_indicator_field.at(x, y + 1, z) +
					                            scalar_indicator_field.at(x, y, z - 1) + scalar_indicator_field.at(x, y, z + 1);
					const double residual = neighbor_sum + right_hand_side - diagonal * scalar_indicator_field.at(x, y, z);
					residual_sum += residual * residual;
					right_hand_side_sum += right_hand_side * right_hand_side;
				}
			}
		}
		return right_hand_side_sum > 0.0 ? std::sqrt(residual_sum / right_hand_side_sum) : 0.0;
	};

	// Over-relaxation factor of sor. Without the screening term, the Jacobi iteration
	// matrix of the 7-point Laplacian on this grid has the spectral radius
	// (cos(pi/nx) + cos(pi/ny) + cos(pi/nz)) / 3, and the best factor is
	// 2 / (1 + sqrt(1 - radius^2)). Screening only strengthens the diagonal, so that
	// factor still converges (any factor below 2 does); it is kept below 1.98 so that
	// a very large grid cannot push it to 2.
	double relaxation = 1.0;
	if (options.solver == PoissonSolver::Sor) {
		const double pi = std::acos(-1.0);
		const auto width = [](auto cells) { return std::max(2.0, static_cast<double>(cells) - 1.0); };
		const double radius = (std::cos(pi / width(scalar_indicator_field.nx)) + std::cos(pi / width(scalar_indicator_field.ny)) +
		                       std::cos(pi / width(scalar_indicator_field.nz))) / 3.0;
		relaxation = std::min(1.98, 2.0 / (1.0 + std::sqrt(std::max(0.0, 1.0 - radius * radius))));
	}

	if (diagnostics != nullptr) {
		*diagnostics = PoissonDiagnostics();
		diagnostics->relaxation = relaxation;
		diagnostics->solver = options.solver;
		diagnostics->cells = static_cast<long long>(domain.nx) * domain.ny * domain.nz;
		diagnostics->iterations = iteration_count;
		diagnostics->splat_seconds = splat_timer.seconds();
	}

	// Runs body(first z, one past the last z) over the interior slabs of the grid, split between the threads of the solve. Each color
	// of sor and each sweep of jacobi writes a cell from cells that no thread writes in the same pass, so the result does not
	// depend on the number of threads.
	const int solver_threads = std::max(1, std::min({options.threads, static_cast<int>(scalar_indicator_field.nz) / 8, 64}));
	const auto run_slabs = [&](const std::function<void(int, int)> &body) {
		mbmesh::parallel_for_ranges(1, static_cast<long>(scalar_indicator_field.nz) - 1, solver_threads,
		                            [&](long begin, long end) { body(static_cast<int>(begin), static_cast<int>(end)); });
	};

	const mbmesh::Stopwatch solve_timer;
	double residual_seconds = 0.0;   // measuring the residual is not part of the solve

	for (int iteration = 0; iteration < iteration_count; iteration++) {

		if (options.solver == PoissonSolver::Sor) {

			// Red-black order: the cells whose x + y + z is even, then those where it is
			// odd. A cell's six neighbors all have the other color, so each color can be
			// updated in place and the result does not depend on the order within it.
			for (int color = 0; color < 2; color++) {
				run_slabs([&](int z_begin, int z_end) {
				for (int z = z_begin; z < z_end; z++) {
					for (int y = 1; y < scalar_indicator_field.ny - 1; y++) {
						const int first_x = ((1 + y + z) & 1) == color ? 1 : 2;
						for (int x = first_x; x < scalar_indicator_field.nx - 1; x += 2) {

							if (has_fixed_nodes && fixed_nodes[scalar_indicator_field.index(x, y, z)] != 0) {
								continue;
							}
							const double neighbor_sum = scalar_indicator_field.at(x - 1, y, z) + scalar_indicator_field.at(x + 1, y, z) +
							                            scalar_indicator_field.at(x, y - 1, z) + scalar_indicator_field.at(x, y + 1, z) +
							                            scalar_indicator_field.at(x, y, z - 1) + scalar_indicator_field.at(x, y, z + 1);

							double diagonal = 0.0;
							double right_hand_side = 0.0;
							cell_equation(x, y, z, &diagonal, &right_hand_side);

							double &value = scalar_indicator_field.at(x, y, z);
							value += relaxation * ((neighbor_sum + right_hand_side) / diagonal - value);
						}
					}
				}
				});
			}
		}
		else {

			std::fill(next_iteration.values.begin(), next_iteration.values.end(), 0.0);

			run_slabs([&](int z_begin, int z_end) {
			for (int z = z_begin; z < z_end; z++) {
				for (int y = 1; y < scalar_indicator_field.ny - 1; y++) {
					for (int x = 1; x < scalar_indicator_field.nx - 1; x++) {

						if (has_fixed_nodes && fixed_nodes[scalar_indicator_field.index(x, y, z)] != 0) {
							next_iteration.at(x, y, z) = scalar_indicator_field.at(x, y, z);
							continue;
						}
						const double neighbor_sum = scalar_indicator_field.at(x - 1, y, z) + scalar_indicator_field.at(x + 1, y, z) +
						                            scalar_indicator_field.at(x, y - 1, z) + scalar_indicator_field.at(x, y + 1, z) +
						                            scalar_indicator_field.at(x, y, z - 1) + scalar_indicator_field.at(x, y, z + 1);

						double diagonal = 0.0;
						double right_hand_side = 0.0;
						cell_equation(x, y, z, &diagonal, &right_hand_side);

						next_iteration.at(x, y, z) = (neighbor_sum + right_hand_side) / diagonal;
					}
				}
			}
			});

			scalar_indicator_field.values.swap(next_iteration.values);
		}

		if (diagnostics != nullptr && iteration + 1 == iteration_count / 2 && iteration + 1 < iteration_count) {
			const mbmesh::Stopwatch residual_timer;
			diagnostics->residual_midway = relative_residual();
			residual_seconds += residual_timer.seconds();
		}
	}

	if (diagnostics != nullptr) {
		const mbmesh::Stopwatch residual_timer;
		diagnostics->residual_final = relative_residual();
		residual_seconds += residual_timer.seconds();
		diagnostics->solve_seconds = solve_timer.seconds() - residual_seconds;
	}

	/**************************************************************************
	 * Step 6: Choose a practical iso-surface convention for marching cubes.
	 *
	 * The Poisson solution has an arbitrary additive offset. By subtracting
	 * the average scalar value at the input samples, the surface samples
	 * cluster around iso_value = 0.0.
	 **************************************************************************/

	if (has_fixed_nodes) {
		return scalar_indicator_field;
	}

	double scalar_sum = 0.0;

	for (const OrientedPoint &oriented_point : oriented_points) {

        if (scalar_indicator_field.nx < 2 || scalar_indicator_field.ny < 2 || scalar_indicator_field.nz < 2) {
            continue;
        }

		const Vec3 grid_coordinate = (oriented_point.point - scalar_indicator_field.origin) / scalar_indicator_field.cell_size;

		const int x0 = std::clamp(static_cast<int>(std::floor(grid_coordinate.x)), 0, static_cast<int>(scalar_indicator_field.nx - 2));
		const int y0 = std::clamp(static_cast<int>(std::floor(grid_coordinate.y)), 0, static_cast<int>(scalar_indicator_field.ny - 2));
		const int z0 = std::clamp(static_cast<int>(std::floor(grid_coordinate.z)), 0, static_cast<int>(scalar_indicator_field.nz - 2));

		const int x1 = x0 + 1;
		const int y1 = y0 + 1;
		const int z1 = z0 + 1;

		const double tx = std::clamp(grid_coordinate.x - static_cast<double>(x0), 0.0, 1.0);
		const double ty = std::clamp(grid_coordinate.y - static_cast<double>(y0), 0.0, 1.0);
		const double tz = std::clamp(grid_coordinate.z - static_cast<double>(z0), 0.0, 1.0);

		const double c00 = scalar_indicator_field.at(x0, y0, z0) * (1.0 - tx) + scalar_indicator_field.at(x1, y0, z0) * tx;
		const double c10 = scalar_indicator_field.at(x0, y1, z0) * (1.0 - tx) + scalar_indicator_field.at(x1, y1, z0) * tx;
		const double c01 = scalar_indicator_field.at(x0, y0, z1) * (1.0 - tx) + scalar_indicator_field.at(x1, y0, z1) * tx;
		const double c11 = scalar_indicator_field.at(x0, y1, z1) * (1.0 - tx) + scalar_indicator_field.at(x1, y1, z1) * tx;

		const double c0 = c00 * (1.0 - ty) + c10 * ty;
		const double c1 = c01 * (1.0 - ty) + c11 * ty;

		const double trilinear_sample = c0 * (1.0 - tz) + c1 * tz;

		scalar_sum += trilinear_sample;
	}

	const double sample_iso_value = scalar_sum / static_cast<double>(oriented_points.size());

    for (double &value : scalar_indicator_field.values) {
        value -= sample_iso_value - options.iso_value;
    }

	return scalar_indicator_field;
}
