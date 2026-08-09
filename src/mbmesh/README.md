# mbmesh

## Overview

**mbmesh** reads swath sonar data through MB-System's datalist processing framework and reconstructs a triangle mesh from accepted multibeam echosounder (MBES) bathymetry soundings.

The current implementation is an end-to-end mesh-generation prototype. It reads MB-System swath files, converts valid soundings and sensor origins into a local Cartesian frame, estimates dataset spacing, derives reconstruction parameters from either a requested or automatic level of detail, estimates oriented normals, reconstructs a scalar field with screened Poisson, extracts a raw mesh with marching cubes, trims unsupported regions, and writes a binary glTF mesh.

The default output is:

- **`mesh.glb`** - support-trimmed final triangle mesh in binary glTF format

Optional command-line flags can also write:

- **`raw_mesh.glb`** - untrimmed marching-cubes mesh before support trimming
- **`pointcloud-local.xyz`** - accepted/decimated samples in the local reconstruction frame
- **`pointcloud-ecef.xyz`** - accepted/decimated samples in WGS84 Earth-Centered Earth-Fixed coordinates
- **`oriented-pointcloud-ecef.ply`** - oriented samples and PCA lambda values
- **`pointcloud.glb`** - diagnostic point-cloud GLB
- **`normals.glb`** - diagnostic normal-vector GLB
- **`origins.glb`** - diagnostic sensor-origin ray GLB
- **`mesh.html`** - X3DOM viewer page for `mesh.glb`

## Quick Start

Build `mbmesh` from the MB-System repository root:

```bash
cmake --build build --target mbmesh
```

Run a basic reconstruction:

```bash
./build/src/mbmesh/mbmesh -I datalist.mb-1
```

This writes:

```text
mbmesh_output/mesh.glb
```

Run with a custom output directory, requested level of detail, raw mesh output, and verbose logging:

```bash
./build/src/mbmesh/mbmesh -I datalist.mb-1 -L 0.5 -O ./mbmesh_output --raw-mesh-glb -V
```

Inspect metadata before running the full reconstruction:

```bash
./build/src/mbmesh/mbmesh -I datalist.mb-1 --metadata
```

Generate an HTML viewer:

```bash
./build/src/mbmesh/mbmesh -I datalist.mb-1 -O ./mbmesh_output --html
```

When `--html` is used, `mbmesh` writes `mesh.html`, starts a local Python HTTP server on `127.0.0.1:8000`, and opens the viewer in the default browser. Server logs are written to `/tmp/mbmesh_http.log`.

## Command-Line Interface

| Option | Argument | Description |
| --- | --- | --- |
| `-I`, `--input` | `datalist` | Input MB-System datalist path. Required unless the datalist is supplied as the first non-option argument. |
| `-O`, `--output` | `outputdir` | Output directory. Defaults to `mbmesh_output`. |
| `-R`, `--bounds` | `west/east/south/north` | Geographic subset bounds in degrees. Use `--bounds=-122.5/-121.8/36.5/37.2` when the west value is negative. |
| `-L`, `--lod`, `--level-of-detail` | `meters` | Requested smallest feature size in local meters. If omitted, `mbmesh` derives LOD from estimated point spacing. |
| `--decimate`, `-decimate` | `meters` | Enable voxel-grid point decimation with the specified cell size. |
| `--metadata`, `--info` | none | Read the datalist, print dataset metadata, and exit before mesh reconstruction. |
| `--html`, `-html` | none | Write `mesh.html`, start a local Python HTTP server, and open the X3DOM viewer. |
| `--xyz`, `-xyz` | none | Write both `pointcloud-local.xyz` and `pointcloud-ecef.xyz`. |
| `--local-xyz`, `-local-xyz` | none | Write `pointcloud-local.xyz`. |
| `--ecef-xyz`, `-ecef-xyz` | none | Write `pointcloud-ecef.xyz`. |
| `--oriented-ply`, `-oriented-ply` | none | Write `oriented-pointcloud-ecef.ply`. |
| `--pointcloud-glb`, `-pointcloud-glb` | none | Write `pointcloud.glb`. |
| `--normal-glb`, `-normal-glb` | none | Write `normals.glb`. |
| `--origin-glb`, `-origin-glb` | none | Write `origins.glb`. |
| `--raw-mesh-glb`, `-raw-mesh-glb` | none | Write `raw_mesh.glb`, the untrimmed mesh before support trimming. |
| `--diagnostics`, `-diagnostics` | none | Write `pointcloud.glb`, `normals.glb`, and `origins.glb`. |
| `--all-outputs`, `-all-outputs` | none | Write every optional output currently supported. |
| `-V`, `--verbose` | none | Enable progress and diagnostic logging. |
| `-H`, `-h`, `--help` | none | Print command usage and exit. |

Notes:

- The final `mesh.glb` is always written for successful non-metadata runs.
- `--xyz` overlaps with `--local-xyz` and `--ecef-xyz`.
- `--diagnostics` overlaps with `--pointcloud-glb`, `--normal-glb`, and `--origin-glb`.
- Point decimation is disabled by default and enabled only with `--decimate <meters>`.

## Output Files

### Default Output

1. **`mesh.glb`** - Final support-trimmed triangle mesh
   - Always written on successful reconstruction runs
   - Uses binary glTF format
   - Intended for web and desktop GLB viewers

### Optional Outputs

2. **`raw_mesh.glb`** - Raw marching-cubes mesh
   - Written with `--raw-mesh-glb` or `--all-outputs`
   - Useful for comparing the mesh before and after support trimming

3. **`mesh.html`** - X3DOM viewer page
   - Written with `--html` or `--all-outputs`
   - References `mesh.glb`
   - Served locally and opened in the default browser when generated

4. **`pointcloud-local.xyz`** - Local Cartesian point cloud
   - Written with `--local-xyz`, `--xyz`, or `--all-outputs`
   - Uses the local reconstruction frame in meters

5. **`pointcloud-ecef.xyz`** - ECEF point cloud
   - Written with `--ecef-xyz`, `--xyz`, or `--all-outputs`
   - Uses WGS84 Earth-Centered Earth-Fixed coordinates

6. **`oriented-pointcloud-ecef.ply`** - Oriented point cloud
   - Written with `--oriented-ply` or `--all-outputs`
   - Includes ECEF position, ECEF normal, and PCA lambda values

7. **`pointcloud.glb`** - Diagnostic point-cloud GLB
   - Written with `--pointcloud-glb`, `--diagnostics`, or `--all-outputs`

8. **`normals.glb`** - Diagnostic normal-vector GLB
   - Written with `--normal-glb`, `--diagnostics`, or `--all-outputs`

9. **`origins.glb`** - Diagnostic sensor-origin ray GLB
   - Written with `--origin-glb`, `--diagnostics`, or `--all-outputs`

## Pipeline Summary

The current pipeline is orchestrated by `src/mbmesh/mbmesh.cpp`:

1. Parse CLI options.
2. Read and preprocess the MB-System datalist.
3. Print metadata and exit if requested.
4. Optionally decimate collected points.
5. Estimate oriented normals.
6. Reconstruct a screened-Poisson scalar field.
7. Extract a raw mesh with marching cubes.
8. Trim unsupported mesh regions.
9. Write `mesh.glb` and requested optional outputs.
10. Launch the local HTML viewer if `--html` was requested.

## Pipeline Details

### 1. Datalist Reading

Implemented in:

- `include/io/datalist_reader.h`
- `src/io/datalist_reader.cpp`

An MB-System datalist contains one or more swath sonar files. Each swath file contains pings, and each ping contains many beams. A valid bathymetry beam produces a sounding: a measured seafloor point.

`read_datalist()` uses MB-System's datalist and MBIO APIs to:

- Open the datalist with `mb_datalist_open()`
- Select processed files when the datalist marks them for use
- Read swath records with `mb_get_all()`
- Process only `MB_DATA_DATA` records
- Count files, pings, soundings, accepted soundings, flagged soundings, and out-of-bounds soundings
- Reject flagged beams with `mb_beam_ok()` when configured to do so
- Apply optional geographic bounds from `-R` / `--bounds`
- Convert accepted soundings into local Cartesian coordinates
- Store the sonar sensor origin for each accepted sounding

The current local coordinate frame is initialized from the first accepted ping's sensor position. Longitude and latitude are converted to meters using an equirectangular local approximation based on that origin latitude. Sounding elevation is stored as negative bathymetry, and sensor elevation is stored as negative sensor depth.

Each accepted sample is stored as a `CollectedPoint`:

- `point` - local Cartesian sounding position
- `origin` - local Cartesian sonar sensor position for the ping

The sensor origin is later used to orient PCA normals.

### 2. Metadata and Dataset Analysis

Implemented in:

- `settings.cpp`
- `settings.h`

`preprocess_datalist()` wraps `read_datalist()` and computes metadata needed for reconstruction defaults:

- Accepted point count
- Local 3D bounds
- Geodetic longitude/latitude bounds
- Nearest-neighbor spacing statistics

Use metadata mode to inspect a dataset before reconstruction:

```bash
./build/src/mbmesh/mbmesh -I datalist.mb-1 --metadata
```

### 3. Level of Detail and Automatic Defaults

Implemented in:

- `settings.cpp`
- `settings.h`

The `-L`, `--lod`, or `--level-of-detail` option is the main user-facing reconstruction control. It represents the requested smallest feature size in local meters. If it is omitted, `mbmesh` derives an automatic LOD from the estimated average point spacing.

`settings.cpp` clamps the resolved level of detail to the supported range and derives:

- Decimation cell size
- Normal-estimation search radius
- Normal-estimation neighbor count
- Poisson grid cell size
- Poisson padding
- Normal splat radius
- Poisson screening weight
- Poisson solver iteration count
- Marching-cubes iso-value
- Support-trimming radius
- Support-trimming normal-offset threshold

Warnings are printed when:

- The resolved LOD is not well supported by the estimated average point spacing.
- The normal-estimation radius is large relative to the requested feature size.

### 4. Point Decimation

Implemented in:

- `include/algorithms/point_decimation.h`
- `src/algorithms/point_decimation.cpp`

Point decimation reduces a `CollectedPointCloud` by voxel-grid averaging. Points are grouped into uniform 3D cells, and each occupied cell emits one averaged point and one averaged sensor origin.

Current behavior:

- The implementation exists and is part of the pipeline.
- The current spacing-driven defaults set `options.decimation.decimate = false`.
- The `--decimate <meters>` option enables decimation and sets the voxel cell size.

This stage is useful for reducing large datasets before normal estimation and reconstruction. Smaller cell sizes preserve more detail; larger values reduce downstream cost more aggressively.

### 5. PCA Normal Estimation

Implemented in:

- `include/algorithms/normal_estimation.h`
- `src/algorithms/normal_estimation.cpp`

`normal_estimation()` converts `CollectedPoint` samples into `OrientedPoint` samples:

- A KD-tree is built over the local point positions.
- The algorithm prefers a physical-radius neighbor search when `search_radius > 0`.
- Radius searches are capped by `k` to avoid unbounded neighborhoods in dense data.
- If the radius search finds fewer than `minimum_neighbors`, the algorithm falls back to k-nearest neighbors.
- PCA is performed on the query point plus its neighbors.
- The eigenvector associated with the smallest eigenvalue is used as the local normal direction.
- The normal is flipped, if needed, to point toward the sonar sensor origin.
- Eigenvalues are retained in `OrientedPoint::lambdas`.

The sensor-origin orientation step is important. PCA produces an unoriented axis: both `n` and `-n` fit the same local plane. Orienting toward the ping's sensor origin is more appropriate for complex bathymetry than a blanket `+Z` assumption.

Known limitation: PCA normals can become noisy near sharp transitions, sparse regions, inconsistent point spacing, or survey edges. Normal quality strongly affects the screened-Poisson reconstruction and support trimming stages.

### 6. Screened Poisson Reconstruction

Implemented in:

- `include/algorithms/screened_poisson.h`
- `src/algorithms/screened_poisson.cpp`

`screened_poisson()` converts oriented samples into a regular 3D scalar grid:

1. Build a padded reconstruction domain around the oriented samples.
2. Splat unit normals into nearby grid nodes with compact quadratic weights.
3. Build screening weights around observed samples.
4. Compute the divergence of the splatted normal field with finite differences.
5. Solve the screened Poisson equation with Jacobi relaxation.
6. Shift the scalar field so sample locations cluster around the configured iso-value.

The output is a `ScalarGrid3D` whose `iso_value` is normally `0.0`.

Screened Poisson was chosen because it can reconstruct surfaces with complex 3D structure, including overhang-like geometry that 2.5D gridding cannot represent. Its main tradeoff is that it tends to produce smooth, closed surfaces. Sparse regions and incorrect normals can create artifacts or closures that need downstream trimming.

The current implementation uses a regular grid, not an adaptive octree. Large high-resolution reconstructions can therefore be memory intensive. Chunked processing or an adaptive octree would be major future improvements.

### 7. Marching Cubes

Implemented in:

- `include/algorithms/marching_cubes.h`
- `src/algorithms/marching_cubes.cpp`

`marching_cubes()` extracts a triangle mesh from the scalar field. It classifies each grid cube against the configured iso-value and uses lookup tables to generate triangles for intersected cells.

The result is the raw reconstructed surface. Write it with:

```bash
./build/src/mbmesh/mbmesh -I datalist.mb-1 --raw-mesh-glb
```

The raw mesh is useful for debugging because it shows the screened-Poisson and marching-cubes output before support trimming removes unsupported regions.

Future candidate: dual contouring may represent sharp edges and abrupt orientation changes better than marching cubes.

### 8. Oriented Sample Support Trimming

Implemented in:

- `include/algorithms/support_trimming.h`
- `src/algorithms/support_trimming.cpp`

Screened Poisson and marching cubes can produce closed surfaces and unsupported artifacts. `support_trimming()` removes generated triangles that are not supported by nearby oriented input samples.

Current trimming logic:

- Build a KD-tree over oriented sample positions.
- Estimate point spacing if automatic radius or offset thresholds are needed.
- Compute mesh vertex normals if the raw mesh does not provide them.
- For each mesh vertex, gather nearby oriented samples within the support radius.
- Reject vertices with too few contributing neighbors.
- Reject vertices whose weighted point-to-plane offset is too large.
- Optionally reject vertices whose mesh normal is poorly aligned with nearby sample normals.
- Keep only triangles whose three vertices are supported.
- Compact surviving vertices and recompute vertex normals.

Current defaults:

- Support trimming is enabled.
- Minimum neighbors defaults to `2`.
- Minimum normal alignment defaults to `0.0`, which disables the alignment test.
- Support radius and normal offset are derived from point spacing and LOD-driven settings.

If trimming removes the entire mesh, `mbmesh` exits with an error instead of writing an empty final surface.

Known limitation: boundary triangles with one or two rejected vertices are removed entirely rather than clipped, which can leave ragged edges.

### 9. Output Generation

Implemented in:

- `include/io/glb_writer.h`
- `src/io/glb_writer.cpp`
- `include/io/xyz_writer.h`
- `src/io/xyz_writer.cpp`
- `include/io/x3dom_writer.h`
- `src/io/x3dom_writer.cpp`

`write_outputs()` always writes the final mesh as `mesh.glb`. Other output writers run only when their CLI flags are set.

Local XYZ output writes local reconstruction coordinates. ECEF XYZ and oriented PLY outputs convert local points back through the coordinate frame and then into WGS84 ECEF coordinates. The HTML writer creates a minimal X3DOM page titled `MB-System mbmesh Mesh Viewer`.

When `--html` is used, `launch_html_viewer_server()` starts:

```bash
python3 -m http.server 8000 --bind 127.0.0.1 --directory <outputdir>
```

and opens:

```text
http://127.0.0.1:8000/mesh.html
```

## Legacy Implementation

The previous implementation in `src/mbmesh/legacy/` was created by the Spring 2026 CSUMB Capstone project team. It read MB-System datalists and generated point-cloud outputs:

- `adjustedPointcloud.xyz`
- `ecefPointcloud.xyz`
- `adjustedPointcloud.glb`
- `adjustedPointcloud.html`

The legacy tool used a single large `mbmesh.cpp` file plus a point-cloud GLB writer. It had `-html` support and could launch a local viewer, but mesh generation was future work in that version.

The current implementation keeps the same broad input domain and web-viewer direction, but replaces the point-cloud-first program with modular mesh reconstruction code:

- Algorithm stages live under `include/algorithms/` and `src/algorithms/`.
- Datalist, GLB, XYZ/PLY, and X3DOM output are separate I/O modules.
- Runtime defaults and metadata live in `settings.cpp`.
- Final output is `mesh.glb` by default, with point-cloud files now optional diagnostics.

## Current Status and Limitations

**mbmesh** is a functioning mesh-generation prototype. It is useful for reconstructing and inspecting bathymetry meshes, but several parts are still research/prototype quality.

Important limitations:

- Reconstruction is a single end-to-end run; there is no reusable preprocessing artifact yet.
- Decimation is available through `--decimate`, but remains off by default because it trades reconstruction detail for lower downstream cost.
- Screened Poisson uses a regular grid, so high-detail reconstruction of large datasets can be memory intensive.
- Normal estimation is sensitive to sparse data, sharp edges, and inconsistent spacing.
- Support trimming can over-trim when normals or thresholds are poor.
- Boundary triangles are discarded rather than clipped when only part of a triangle is unsupported.
- The HTML viewer launch depends on `python3` and a desktop/browser environment.

High-value future work:

- Expose selected reconstruction parameters as documented CLI options.
- Add chunked or tiled reconstruction for large surveys.
- Investigate octree/adaptive Poisson reconstruction.
- Improve normal estimation near boundaries and sharp orientation changes.
- Clip support-trimmed boundary triangles instead of dropping them.
- Add full CLI regression tests using small real or synthetic datalists.
- Improve the generated HTML viewer with metadata and controls.
- Evaluate 3D Tiles and compression outputs for large web visualization workflows.

## Source Layout

### Main Application

- **mbmesh.cpp** - CLI parsing, pipeline orchestration, output selection, help text, and local HTML viewer launch
- **settings.cpp** - Runtime defaults, metadata computation, spacing estimation, bounds handling, and metadata output
- **settings.h** - CLI option state and metadata structures

### Algorithm Interfaces

- **include/algorithms/point_decimation.h** - Point decimation options and interface
- **include/algorithms/normal_estimation.h** - Normal estimation options and interface
- **include/algorithms/screened_poisson.h** - Screened-Poisson options and scalar-grid interface
- **include/algorithms/marching_cubes.h** - Marching-cubes options and mesh extraction interface
- **include/algorithms/support_trimming.h** - Support-trimming options, diagnostics, and interface

### Algorithm Implementations

- **src/algorithms/point_decimation.cpp** - Voxel-grid point decimation
- **src/algorithms/normal_estimation.cpp** - PCA normal estimation with sensor-origin orientation
- **src/algorithms/screened_poisson.cpp** - Regular-grid screened-Poisson scalar-field reconstruction
- **src/algorithms/marching_cubes.cpp** - Triangle extraction from scalar grids
- **src/algorithms/support_trimming.cpp** - Support-based mesh filtering

### Data Structures and Math Helpers

- **include/data_types/geometry.h** - Point, mesh, bounds, reconstruction-domain, and grid data types
- **include/data_types/swathfile.h** - Swath-file data types
- **include/math/vec2.h**, **vec3.h**, **mat3.h** - Basic vector and matrix helpers
- **include/math/eigen.h** - Symmetric 3x3 eigen decomposition
- **include/math/kdtree.h** - KD-tree nearest-neighbor search

### I/O

- **include/io/datalist_reader.h** and **src/io/datalist_reader.cpp** - MB-System datalist and swath reading
- **include/io/glb_writer.h** and **src/io/glb_writer.cpp** - Mesh, point, normal, and origin-ray GLB writers
- **include/io/xyz_writer.h** and **src/io/xyz_writer.cpp** - Local XYZ, ECEF XYZ, and oriented PLY writers
- **include/io/x3dom_writer.h** and **src/io/x3dom_writer.cpp** - HTML/X3DOM viewer writer

### Tests

- **tests/test_marching_cubes.cpp** - Marching-cubes test program
- **tests/test_normal_estimation.cpp** - Normal-estimation test program
- **tests/test_support_trimming.cpp** - Support-trimming test program
- **tests/synthetic_data.h** and **tests/perlin_noise.h** - Synthetic test data helpers

### Build Configuration

- **CMakeLists.txt** - CMake build configuration for the `mbmesh` executable and tests
- **Makefile.am** - Autotools build configuration

## Building

### CMake

From the MB-System repository root:

```bash
cmake --build build --target mbmesh
```

If the build directory has not been configured yet:

```bash
cmake -S . -B build
cmake --build build --target mbmesh
```

The resulting executable is:

```bash
./build/src/mbmesh/mbmesh
```

### Autotools

From the MB-System repository root:

```bash
./configure
make mbmesh
```

## Development Notes

- If `mbmesh` on the shell resolves to `/usr/local/bin/mbmesh`, it may be an older installed binary. Use `./build/src/mbmesh/mbmesh` to run the current build-tree executable, or reinstall after building.
- The CMake target is `mbmesh`.
- Only the final mesh output is enabled by default. Optional diagnostic outputs must be requested explicitly.
- Metadata mode still reads the datalist and computes spacing, but it exits before normal estimation, screened Poisson, marching cubes, trimming, and output writing.

## Copyright and License

These source files are copyright by David W. Caress, Dale N. Chayes, and the California State University Monterey Bay Capstone team. They are licensed using GPL3 as part of MB-System.
