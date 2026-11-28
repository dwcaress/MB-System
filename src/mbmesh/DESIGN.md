# mbmesh, mbglb2tiles and the 3D Tiles output: design as implemented

This document describes how `mbmesh`, `mbglb2tiles` and the pieces they share with `mbgrd2gltf` work together to take swath
bathymetry (or lidar) to a surface that a browser can stream. It records what was built and why; the options and the
details of each algorithm are in [README.md](README.md), [../mbglb2tiles/README.md](../mbglb2tiles/README.md) and the man pages. It
was written for issue [#1629](https://github.com/dwcaress/MB-System/issues/1629), a part of [#1559](https://github.com/dwcaress/MB-System/issues/1559).

## Overview

```
swath datalist ──► mbmesh ──► mesh.glb, chunks/, point outputs, track.glb, mbmesh_output.html
                     │
                     ├── --tileset ──► tileset/ (one level: the chunks as tiles)
                     │
mesh.glb ──► mbglb2tiles ──► tileset/ (levels L0 … Ln)  [+ tileset.html]
(or a GLB of mbgrd2gltf)

directory of surveys ──► mbmesh -I <dir> ──► mbmesh_survey.html (every product found)
```

- **mbmesh** reads a datalist, builds oriented points, reconstructs a watertight-where-supported surface (screened Poisson,
  marching cubes, support trimming), and writes it, optionally as a one-level 3D Tiles tileset.
- **mbglb2tiles** reads a mesh GLB of `mbmesh` or `mbgrd2gltf` and makes the tileset of levels of detail. It reconstructs nothing.
- **src/mbdraco** is the Draco compression shared by `mbmesh`, `mbglb2tiles` and `mbgrd2gltf`.
- **The pages** (`--html`, and the survey page) are X3DOM pages that show all of the above.

## Coordinate frame and georeference

- The reconstruction runs in an exact east-north-up (ENU) frame derived from ECEF, with its origin below the data centroid
  (`coordinate_frame`), not a first-ping equirectangular approximation.
- `--geoorigin` is the option of `mbgrd2gltf`, with the same values and meaning: without it GLBs are in absolute ECEF (about 0.5 m resolution in
  float32, and a warning says so); alone it is the center of the data bounds at the mean elevation; `lon,lat,elev` is an origin of the user's.
  GLBs are ECEF, or ECEF offsets from the GeoOrigin, so `mbmesh` and `mbgrd2gltf` products with the same GeoOrigin overlay.
- Heights are ellipsoid heights, as in `mbgrd2gltf` (`-bathymetry` is treated as the height above the ellipsoid).
- Every GLB carries `asset.extras` with `georeference` (the ECEF center, or that it is absolute), `provenance` and `MB-System` blocks
  (see [GEOREFERENCE.md](GEOREFERENCE.md)); `mbglb2tiles` refuses a GLB without them, as it would not know where it is.

## Reconstruction and chunking

- Stages: read and filter soundings, decimate (off by default), PCA normals, screened Poisson on a regular grid (`sor`, a red-black
  successive over-relaxation, by default; `jacobi` is kept), marching cubes, oriented-sample support trimming (with `--void-size` and
  `--island-size`), and output.
- A single Poisson grid over a survey is mostly empty, so reconstruction is **chunked**: square chunks on a lattice anchored at the frame origin,
  every chunk's grid snapped to one global cell lattice, a halo of neighboring points for normals and trimming, LOD, spacing and Poisson
  parameters computed once for the whole survey, and a triangle kept by the chunk that holds its centroid. Chunking turns on above about 8 million cells.
- **Seams.** Neighboring chunks are solved apart, so their fields differ along the shared edge. Each chunk adds a correction that makes its field on
  the west and south planes the neighbor's own (`--seam-blend`, `--seam-method`), so the marching-cubes vertices on a shared plane coincide to
  the last bit, and `mesh.glb` (which merges vertices within a micrometer) has no seams.
- **Threads.** Chunks run in parallel (`--threads`), sized so that at least two tiles per thread hold points; the tiles with the most points start first,
  or, when the seams are matched, in chunk order, as a chunk waits for its west and south neighbors.
- **Memory.** A survey of more than `--spill-points` soundings is binned to per-chunk files on disk, so memory follows the size of a chunk.
- **Measured.** OceanImaging2018 20180923d1 (51.8 million soundings, 73 files) took 2 h 55 min in one pass and 40 min chunked on 4 threads;
  one swath file gave a median vertex distance of 4.9 cm between the single pass and the chunked run.

## Outputs

- **GLB** (`mesh.glb`, `raw_mesh.glb`, `chunks/`, `pointcloud.glb`, `track.glb`, and so on): `mesh.glb` and the raw mesh have one primitive for each square of `--primitive-size` meters (by default
  256 Poisson cells), as `mbgrd2gltf` writes blocks of 256 cells, and each chunk file is one mesh; every primitive has its own vertices, and Draco-compressed by default (`KHR_draco_mesh_compression`, positions quantized over the range
  of each mesh to `--draco-precision`, normals in `--draco-normal-bits`), with `--no-draco` for arrays. Point clouds are colored by height
  (`--pointcloud-colormap`).
- **Draco** is `src/mbdraco`: one encoder, the same options and the same primitive for all three programs. It is built by CMake as the static
  library `mbdraco` and by autotools as `libmbdraco.la`.

## The tileset

`mbmesh --tileset` and `mbglb2tiles` write one layout into one directory, `tileset/` (`mbmesh_output/tileset/` for `mbmesh`):

```
tileset/tileset.json
tileset/L0/tile_I_J.glb      L0 the finest level
tileset/L1/…  Ln/…
```

- **Version and refinement.** 3D Tiles 1.1, `REPLACE` refinement.
- **Content.** Tile GLBs are in the local ENU frame written Y-up (x east, y up, z south) as glTF and 3D Tiles require, one primitive each, Draco-compressed.
  The `transform` of the root is the ENU frame at the origin in ECEF, column-major. Tiles therefore do not depend on `--geoorigin`, and the float32
  coordinates are local, so there is no 0.5 m limit.
- **Bounding volumes and errors.** A box for each tile that holds its content and its children; `geometricError` is the cell size of the level
  (zero at `L0`).
- **Levels.** `mbmesh --tileset` writes `L0`, the chunks. `mbglb2tiles` reads the mesh, welds the vertices of the primitives (twice the Draco step), recomputes
  normals, puts it in an ENU frame at the middle of its bounds, and for each level above `L0` clusters the vertices on a lattice of cubes of
  spacing × 2^level (each cluster its mean, collapsed and repeated triangles dropped), then cuts the whole level, so the tiles of a level meet exactly.
  Tiles are squares of `--tile-size` × 2^level (default 256 × the spacing); four tiles are replaced by one until a single tile holds everything.
- **Replacing.** Either program removes a tileset already in the directory (`remove_previous_tileset`) before writing, and logs it.
- **Lift.** X3DOM's `GeoTileset` takes the height of the camera as at least 1 m above the ellipsoid, so a camera over seafloor never gets near
  enough to refine a tile. `--lift` puts the tileset higher along its vertical, in the root `transform`, and works as `--geoorigin` does: without
  it no lift (and a warning if any point is below the ellipsoid), alone the default (the lowest point 10 m above the ellipsoid, to a meter), or
  `--lift=<meters>`. The lift is written as `liftMeters` in the extras; the pages lift their other models and viewpoints by it, and Copy GeoViewpoint takes it off.
- **Screen-space error.** The page sets the `GeoTileset` `maximumScreenSpaceError` to 1 pixel (`--sse` in `mbglb2tiles`); X3DOM's default of 8 refines
  tiles of centimeter errors only within a few meters.
- **Colors.** Every tile vertex has a `COLOR_0` (floats from 0 to 1 quantized to 10 bits in Draco, since CesiumJS decodes Draco colors of bytes wrongly; bytes without Draco) from the Haxby table over the heights of the whole survey, so all tiles
  and levels agree, with a white base color so the colors show unchanged (`--no-colors` writes tiles as before). The bounds are recorded as
  `heightColors` in the extras.

## Pages

- `mbmesh --html` writes `<outputdir>.html` beside the output directory (for example `mbmesh_output.html`), and a `mbmesh.html` inside it, which show `mesh.glb` or, with `--tileset`, the tileset, with the GeoOrigin as a child of the `GeoTileset`,
  overview viewpoints, user viewpoints (`--geoviewpoint`), switchable products, chunk outlines and labels (`--label-chunks`), and the processing log.
  It prints the command to serve it; it does not start a server.
- `mbglb2tiles --html` writes `<outputdir>.html` beside the output directory.
- `mbmesh -I <directory>` writes `mbmesh_survey.html`, which finds the products of every run below it, including a tileset in a directory of any
  name, in accordions by directory.
- **Scale object.** Every page of the three programs (`mbgrd2gltf`, `mbmesh`, `mbglb2tiles`) has axes East (red), North (green) and Up (blue) at the center of rotation, the technique that STOQS uses for its platforms
  (a `GeoLocation` with an axes triad, resized by a `viewpointChanged` handler and labeled with its length). Each viewpoint has `onviewpointChanged`,
  which calls `updateScaleAxes()`: from the distance of the camera to the center of rotation and the field of view it picks a length of about a sixth of the
  height of the view, rounded down to 1, 2 or 5 times a power of ten, puts the unit triad there with a matrix (X east, Y up, Z south at that point of the Earth), and
  writes the length in the label only when the rounded length changes. The length is true at the axes, not elsewhere in a perspective view. The
  "scale axes" check box switches it off. The code is one header, `src/mbgrd2gltf/scale_axes.h`, that the three page writers include (the hook, the scene nodes, the
  check box row and the script), so the pages agree; `mbgrd2gltf`, which has a vertical exaggeration, sets `scaleAxesExaggeration` and the label then says what the Up
  axis stands for too ("20 m (up 10 m)").
- A tileset with colors gets a "color by height" checkbox, one for each tileset: unchecked, it turns the vertex colors off and sets the gray
  base color that MeshLab shows (192 of 255). The tiles are reached through X3DOM's internal nodes from the `GeoTileset`, and tiles that load later
  are done as they arrive.
- `--cesium-html` of `mbglb2tiles`, `mbmesh` and `mbgrd2gltf` writes a page for CesiumJS with one writer, `io/cesium_writer.cpp` (standard library only, so that `mbgrd2gltf`
  takes the source too; `io/cesium_convert.h` turns the models and the `--geoviewpoint` values of the X3DOM pages into its options). The page lists models, each with a check box
  that loads it the first time it is switched on. A tileset is a `Cesium3DTileset`, with the lift taken off again by its `modelMatrix` (Cesium needs none) and a "color by height"
  check box (a custom shader that sets a gray base color before the lighting; the `REPLACE` blend mode of a style paints the final color and loses the shading). A GLB is a `Model`
  placed by the translation of the GeoOrigin alone, with `upAxis` Z and `forwardAxis` X, which leaves the axes of glTF as they are, since the vertices are ECEF. A GLB without normals
  (the mesh of `mbgrd2gltf`) is unlit in CesiumJS, which ignores a lighting model that a custom shader asks for, so a custom shader shades it from `dFdx` and `dFdy` of the eye-space
  position and a fixed light; the vertex colors of such a GLB (`mbgrd2gltf` colors its mesh by height) are in `material.diffuse` by then, and the shader mixes them with gray by a uniform that the
  "color by height" check box sets. The globe starts hidden, with a check box that shows it translucent (the data are below the ellipsoid, which the globe would hide). The scale object is
  polylines and labels at the point of the data under the middle of the view, from a depth pick, resized before each render, with the origin raised a tenth of the axis length above
  the surface; with `--exaggeration` its label says what the Up axis stands for. The Viewpoint menu has an overview looking down, which the page opens with, fitted to the models that
  are shown (if none is, as in the survey page of `mbmesh`, which starts with only the tracks on, the first tileset is shown), views at 45 and 15 degrees from the south, and the
  viewpoints of the user: `--geoviewpoint`, in the form of the X3DOM pages (six or ten numbers, true heights, so no lift), is converted in the page from the axis and angle in the
  local frame (X east, Y up, Z south) to the direction and up of the camera. The **Copy viewpoint** button writes the ten-number option for the view on the screen, with the point
  under the middle of the view as the center, so that an option copied from either kind of page works in the other. `mbmesh` writes the page beside the output directory and inside it,
  and `mbmesh_survey_cesium.html` for a directory input; it has no chunk outlines or labels. Seen in headless Chrome with CesiumJS 1.114: `mbglb2tiles` on a small tileset, `mbmesh` on a
  small run and on a small survey directory (a tileset and its `mesh.glb` line up), and `mbgrd2gltf` on one grid; not tried on a large survey or with the OpenStreetMap imagery, which
  the headless run did not load.
- The pages need an X3DOM build that has `GeoTileset` ([PR 1364](https://github.com/x3dom/x3dom/pull/1364)), given with `--x3dom-url`.

## Build

CMake builds everything (`mbdraco`, `mbgrd2gltf`, `mbmesh`, `mbglb2tiles`), and its tests (`ctest`) pass. The autotools build has `Makefile.am` files for all four
directories (`src/mbdraco`, `src/mbgrd2gltf`, `src/mbmesh`, `src/mbglb2tiles`) and works with a `configure` and `Makefile.in` files regenerated from them (the
sequence is in `configure.cmd`, with automake 1.18.1); the committed ones are not regenerated for these programs.

## Verification

- **Unit tests** (CMake `ctest`, autotools `make check`): coordinate frame, GeoOrigin, settings, point spill, chunking and seams, normal
  estimation, marching cubes, support trimming, track, provenance, X3DOM writer (lift, colors, survey scan), tileset writer, Draco (positions, colors,
  primitives), `mbglb2tiles` levels; plus `mbmesh_test.py`, which runs the program.
- **3D Tiles validator.** `3d-tiles-validator` 0.6.1 reports no errors and no warnings on a one-tile tileset and on the multi-level lidar (380 tiles)
  and multibeam (301 tiles) tilesets of OceanImaging2018 20180925d1; it cannot look inside Draco, and says so.
- **CesiumJS.** `mbglb2tiles --cesium-html` pages load the tilesets in CesiumJS 1.114; colors, the scale axes, the viewpoints and their copy button were checked in headless Chrome on a small tileset.
- **X3DOM.** The tilesets load in `GeoTileset`; finer levels load as the camera approaches (with `--lift` and a screen-space error of 1); the color
  checkbox was exercised in headless Chrome and by hand on the survey page.
- **Products overlay.** `mbmesh` and `mbgrd2gltf` products with the same GeoOrigin line up.

## Limits and TODOs

None of these blocks use of the tileset; each is small enough for its own issue.

- **CesiumJS.** The tilesets load in CesiumJS 1.114 in headless Chrome, as a small test tileset; large surveys have not been tried.
- **Coarse levels.** Made by clustering the finished mesh, not by re-reconstructing from points decimated at twice the cell size, so thin features and
  sharp edges round off at coarse levels. `mbmesh --tileset` itself writes only `L0`.
- **Memory of `mbglb2tiles`.** The whole mesh is held (about 12 GB at the peak for 38 million vertices, the weld about 5 minutes and the first
  clustering about 2.4); there is no path from the chunk files.
- **One frame.** All tiles share one ENU frame; a survey tens of kilometers across would need a frame for each tile.
- **Colors.** One table, over height; none from the data (backscatter), and `mesh.glb` is not colored. `mbgrd2gltf` colors its mesh by height with the same table (`src/mbdraco/haxby_colors.h`, shared with `mbmesh`), over the range of the grid, unless `--no-colors`. The checkbox relies on X3DOM internals.
- **Reconstruction.** Sparse tiles (under 32 owned points) are skipped; boundary triangles are dropped, not clipped, by support trimming; the point
  outputs of a very large survey are limited to 5 million points; octree or adaptive Poisson is not tried.
- **Build.** The committed `configure` and `Makefile.in` files are not regenerated for the new sources; CMake is the build that is kept up to date.
