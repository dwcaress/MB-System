# mbmesh

## Overview

**mbmesh** reads swath sonar data through MB-System's datalist processing framework and reconstructs a triangle mesh from accepted multibeam echosounder (MBES) bathymetry soundings.

The current implementation is an end-to-end mesh-generation prototype. It reads MB-System swath files, converts valid soundings and sensor origins into a local Cartesian frame, estimates dataset spacing, derives reconstruction parameters from either a requested or automatic level of detail, estimates oriented normals, reconstructs a scalar field with screened Poisson, extracts a raw mesh with marching cubes, trims unsupported regions, and writes a binary glTF mesh.

`mesh.glb` vertices are WGS84 Earth-Centered, Earth-Fixed (ECEF) coordinates in meters, using the same conventions as `mbgrd2gltf`:

- **No `--geoorigin`** (default): absolute ECEF coordinates.
- **`--geoorigin` / `-G` with no value**: ECEF offsets from the GeoOrigin at the center of the data bounds and the mean sounding elevation.
- **`--geoorigin=LON,LAT,ELEV` / `-GLON,LAT,ELEV`**: ECEF offsets from the specified GeoOrigin.

Offsets are a pure translation (`vertex = ECEF(point) - ECEF(GeoOrigin)`); the axes stay aligned with ECEF, exactly as in `mbgrd2gltf --geoorigin`. A `mbmesh` and an `mbgrd2gltf` product made with the same GeoOrigin therefore line up directly. GLB positions are 32-bit floats, so absolute ECEF (magnitudes of about 6.4 million meters) resolves only about 0.5 m; use a GeoOrigin to preserve sub-meter detail. `mbmesh` prints a warning when no GeoOrigin is given.

Internally, reconstruction runs in a separate local east-north-up (ENU) frame tangent to the WGS84 ellipsoid at the point below the data centroid. Each sounding and sensor position is converted to ECEF and then rigidly translated and rotated into this frame, so there is no map-projection distortion, and results are converted back to ECEF exactly. This frame is an implementation detail and does not depend on `--geoorigin`. `pointcloud-local.xyz` is written in it.

The default output is:

- **`mesh.glb`** - support-trimmed final triangle mesh in binary glTF format, using the `mbmesh` local Cartesian coordinate frame

Optional command-line flags can also write:

- **`raw_mesh.glb`** - untrimmed marching-cubes mesh before support trimming
- **`pointcloud-local.xyz`** - accepted/decimated samples in the same local reconstruction frame as `mesh.glb`
- **`pointcloud-ecef.xyz`** - accepted/decimated samples in WGS84 Earth-Centered, Earth-Fixed coordinates
- **`oriented-pointcloud-ecef.ply`** - oriented samples and PCA lambda values
- **`pointcloud.glb`** - diagnostic point-cloud GLB, colored by height with the Haxby color table of `mbm_grdplot` (a `COLOR_0` attribute): 11 colors from dark blue to white, changing linearly between 11 heights that are listed in the file's `MB-System.content` description; `--pointcloud-colormap` chooses the heights
- **`normals.glb`** - diagnostic normal-vector GLB
- **`origins.glb`** - diagnostic sensor-origin ray GLB
- **`track.glb`** - the sensor track as a black 3D line
- **`<outputdir>.html`** (for example `mbmesh_output.html`) - X3DOM viewer page for the GLB files, written in the parent of the output directory
- **`<outputdir>/mbmesh.html`** - the same page inside the output directory, showing only the GLBs of this run
- **`mbmesh_survey.html`** - written in the current directory when `-I` names a directory instead of a datalist: one page for every GLB beneath it (see [Survey Directories](#survey-directories))

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
./build/src/mbmesh/mbmesh -I datalist.mb-1 --info
```

Generate an HTML viewer:

```bash
./build/src/mbmesh/mbmesh -I datalist.mb-1 -O ./mbmesh_output --html
```

When `--html` is used, `mbmesh` writes a viewer page and prints, as a log message, the command to view it. The page is written in the **parent** of the output directory and named after the output directory, for example `mbmesh_output.html` next to `mbmesh_output/`, so that the parent is the root of the web server. It does not start a server or a browser, so no process is left running and no port is held. Run the printed command in a terminal (Ctrl-C stops it) and open the printed URL:

```text
mbmesh: [html] to view mbmesh_output.html, run this command in a terminal (Ctrl-C stops it), then open the URL:
mbmesh: [html]   python3 -m http.server 8000 --bind 127.0.0.1 --directory '/path/to/OceanImaging2018/20180923d1/multibeam'
mbmesh: [html]   http://127.0.0.1:8000/mbmesh_output.html
```

The page has to be served over HTTP because browsers will not load the GLB files from a `file://` URL.

The same page is also written inside the output directory as `mbmesh.html`. It is like `mbmesh_output.html` except that it refers to its GLBs by their names in that directory and offers none from beside the directory (no "Other GLBs in this directory" section), so it works wherever the web server is rooted, for example at a survey directory above the `multibeam` and `lidar` directories (`http://127.0.0.1:8000/multibeam/mbmesh_output/mbmesh.html`).

### Survey Directories

A survey directory in the SeafloorMapping layout, for example `20260722d1`, holds subdirectories such as `multibeam` and `lidar`, each of which can hold an `mbmesh_output`. Run `mbmesh` in the survey directory, naming the datalist inside a subdirectory:

```bash
cd 20260722d1
mbmesh -I multibeam/datalistp.mb-1 --html     # writes multibeam/mbmesh_output/, multibeam/mbmesh_output.html and multibeam/mbmesh_output/mbmesh.html
```

When `-I` names a **directory**, `mbmesh` reconstructs nothing. It writes `mbmesh_survey.html` in the current directory, a viewer page for every `.glb` file beneath that directory:

```bash
mbmesh -I .                                   # in the survey directory; serve this directory and open /mbmesh_survey.html
```

The check boxes are in accordions that mirror the directory layout, one inside another for each level, with the directory named by `-I` open and the ones inside it closed. In each directory `mesh.glb` is listed first, then `track.glb`, then the others by name, and a directory with a `tileset.json` in it, such as `tileset` from `mbmesh --tileset` or `mbglb2tiles_output` from `mbglb2tiles`, is offered as one tileset, labeled with its name unless it is `tileset`, in place of its tiles (the `chunks` directories are otherwise skipped, as are directories whose names begin with a dot). The `track.glb` files are switched on when the page opens, so the page shows where the surveys ran; the other GLBs are off and are loaded when first switched on. At most 1000 models are listed. The paths are relative to the current directory, so run `mbmesh` in the survey directory or above it, and serve that directory.

The page gets an initial view as a run's page does, from the georeference (`asset.extras`) of every GLB that has one: the overview (`Viewpoint` in model coordinates, and the `GeoViewpoint`) looks straight down on the middle of the union of their extents, from the height that fits it in a 45 degree view, so it takes in the whole survey. The GeoOrigin is that of the first GLB whose vertices are relative to one. GLBs from runs with another GeoOrigin do not line up with it, so every GLB with a georeference is compared with the GeoOrigin of the page, and each that has another (its center more than 1 cm away), or has absolute vertices, is logged as an `ERROR` that names it and says how far apart the GeoOrigins are; the page is still written. GLBs written before the georeference was recorded give no view, and a page with none opens on the equator.

`--geoviewpoint` works with a directory input, without `--html`, and adds its GeoViewpoints to the survey page's Viewpoint menu (`USER_GEOVIEW_1`, `USER_GEOVIEW_2`, ...), as it does for the page of a run; so do `--x3dom-url` and `--lifted`. With `--cesium-html` a directory input also writes `mbmesh_survey_cesium.html`, which offers the same GLBs and tilesets to CesiumJS (the first tileset, or else the first GLB, is shown when it opens, since the survey page starts with only the `track.glb` files on). The viewpoints of the pages of the runs are not carried over.

## Command-Line Interface

Run with no arguments, or with `-H`, `-h` or `--help`, `mbmesh` prints its usage: a description, the options and the survey page, as `mbgrd2gltf` does.

| Option | Argument | Description |
| --- | --- | --- |
| `-I`, `--input` | `datalist` | Input MB-System datalist path. Required unless the datalist is supplied as the first non-option argument. |
| `-O`, `--output` | `outputdir` | Output directory. Defaults to `mbmesh_output` in the directory of the input datalist, so `-I multibeam/datalistp.mb-1` writes `multibeam/mbmesh_output`. |
| `-R`, `--bounds` | `west/east/south/north` | Geographic subset bounds in degrees. Use `--bounds=-122.5/-121.8/36.5/37.2` when the west value is negative. |
| `-G`, `--geoorigin` | `[lon,lat,elev]` | Write GLB vertices as ECEF offsets from a GeoOrigin, as in `mbgrd2gltf`. With a value, all three of lon, lat, elev (degrees, degrees, meters above the ellipsoid) are required. With no value, uses the data-bounds center and mean elevation. Without the option, vertices are absolute ECEF. The value must be attached to the option, with an equals sign and no spaces, as in `mbgrd2gltf`: `--geoorigin=-122.4,36.6,0` or `-G-122.4,36.6,0`. A value in a separate argument (`--geoorigin -122.4,36.6,0`) is not accepted. |
| `-L`, `--lod`, `--level-of-detail` | `meters` | Requested smallest feature size in local meters; the Poisson cell is a quarter of it, but no finer than half the point spacing. If omitted, `mbmesh` derives LOD from the measured point spacing, coarsened if needed to keep the run within about 500 million cells. |
| `--chunk-size` | `meters` | Reconstruct in square chunks of about this size (rounded up to whole Poisson cells) instead of one pass. Chunking turns on automatically when a single Poisson grid would exceed about 8 million cells. |
| `--chunk-halo` | `meters` | Neighboring points each chunk reads beyond its edge. The default is what the normal estimation, splatting and trimming need (0.275 m at `-L 0.1`). A smaller halo is allowed, with a warning. A larger one costs memory, disk (with `--spill-points`) and time, since every chunk reconstructs its halo again, and it changes the automatic chunk size; on one multibeam survey a halo of 1 m left larger mismatches at the chunk edges than 0.3 m did. Only used when the run is chunked. |
| `--primitive-size` | `m` | Write `mesh.glb` and `raw_mesh.glb` as one mesh of many primitives, one for each square of `m` meters of the local east-north frame (anchored at its origin, so the squares are the chunks when `m` is the chunk size), as `mbgrd2gltf` writes blocks of 256 cells (the default is 256 Poisson cells). Each primitive has the vertices its triangles use, numbered from zero, and its own Draco stream, so a viewer such as X3DOM decodes and draws the surface in parts: a mesh of 38 million vertices as one Draco primitive is more than its WebAssembly heap holds. The vertices on the edges of the squares are in both primitives, and each is quantized over its own extent, so they can differ by up to half the step of the quantization (under a millimeter at the default). `0` writes one primitive. The chunk files and the tiles of a tileset are one primitive each. |
| `--seam-blend` | `meters` | With `--seam-method blend`, over this width on each side of an edge between chunks, the field of a chunk is made to match that of the chunk to its west or south on the edge, so that the meshes meet exactly, with no step and no gap (see "Seams between chunks"). The default is 1 m, or a quarter of the chunk size if that is less; 0 does not match them, and lets the chunks be started in any order. Only used when the run is chunked. |
| `--seam-method` | `blend` or `solve` | How chunks are matched on their edges (default `solve`): `solve` solves each chunk with the final fields of its west and south neighbors fixed on the edges they share, so there is nothing to blend and no edge effect on those sides, which makes the Poisson solves go along a wavefront of chunks and not all at once, with the threads that have no chunk to solve sharing the sweeps of the solver (see "Seams between chunks"); `blend` solves each chunk free and then blends its field to the neighbors' over `--seam-blend`, with all chunks solved at the same time. `--seam-blend 0` turns off the matching, whichever the method. |
| `--no-draco` | none | By default the triangle meshes of the GLB files (`mesh.glb`, `raw_mesh.glb`, the chunks and the tileset tiles) are compressed with Draco (`KHR_draco_mesh_compression`), which makes them about a tenth of the size (see "Draco compression"). This writes them as arrays of 32-bit floats and integers. The point cloud, normals, sensor positions and track are never compressed. |
| `--draco-precision` | `meters` | With Draco, the positions are kept to this many meters (default 0.001): each file gets the number of bits (8 to 30) that makes the step of its quantization no more than this over the range of its extent. |
| `--draco-normal-bits` | `n` | With Draco, the bits of each normal (2 to 30, default 10). |
| `--chunk` | `i,j` | Reconstruct only chunk (i,j), for fast testing. Chunk (i,j) covers x in [i*size, (i+1)*size) and y in [j*size, (j+1)*size) in the local east-north frame; use `--info` and a verbose run to see chunk indices. `i` and `j` can each be a range, `first..last` (both included), as in `--chunk 2..3,-11..-10` for four chunks, and the option can be repeated, `--chunk 2,-10 --chunk 3,-10`, to see the seam between neighbors; the chunks add up. `mesh.glb` and the pages then hold only those chunks. |
| `--max-chunks` | `n` | Stop after n chunks (for testing). |
| `--label-chunks` | none | With `--html`, the page draws the outline of each chunk, at the mean height of its surface, and its file name (`chunk_2_-10.glb`) as text that faces the viewer, raised (by a `GeoTransform`) to one text height above the highest point of the chunk so that it is not in the surface, with a check box (Chunks) to switch them off. For finding which chunk a seam, a void or an overhang belongs to. Needs a chunked run. The survey page of a directory input (`mbmesh -I . --label-chunks`) draws them too, for every `chunk_*.glb` in a `chunks` directory beneath it that says where its chunk is: each chunk file carries the corners of its square, its middle, the height of its name over it and the height of the text, as longitude, latitude and height, in `MB-System.content.chunkLabel`, so the chunks of runs made before that was written are not drawn (the log says how many). |
| `--poisson-solver` | `name` | How the Poisson grid equation is solved: `sor` (the default) or `jacobi`. `sor` is red-black successive over-relaxation, updated in place, with the over-relaxation factor chosen from the grid size (about 1.95 for a grid of a hundred cells across). One iteration of either costs about the same, but `sor` converges in far fewer, so it chooses its own, smaller number of iterations. See "Poisson solvers". |
| `--poisson-iterations` | `n` | Iterations of the solver, from 1 to 100000, for either solver. By default `sor` takes one iteration per cell across the longest edge of each chunk's grid, but at least 50, and `jacobi` takes a count chosen from the level of detail, 200 to 500. The relative residual after the iterations is logged (see "Log Messages"). |
| `--threads` | `n` | Chunks reconstructed at once when chunking. Defaults to the smaller of the core count and 4. Each chunk holds its own Poisson grid, so memory grows with this. |
| `--spill-points` | `n` | A survey with more than `n` accepted soundings (default 50,000,000) is not held in memory: the soundings go to a file in `mbmesh_work` beside the results and are sorted into one file per chunk, so memory follows the size of a chunk and not of the survey. `0` always does. See [Surveys larger than memory](#surveys-larger-than-memory). |
| `--void-size` | `m` | Put back voids of up to `m` meters across (default 2) that support trimming would remove from inside the surface, where the soundings thin out in the depressions of the seafloor and under overhangs. Gaps at the edge of the survey stay trimmed. `0` trims them all. See [Voids](#voids-in-the-surveyed-surface). |
| `--island-size` | `m` | When the survey is reconstructed in chunks, remove from `mesh.glb` the pieces of the merged mesh that are not joined to the rest and have an area of up to that of a disc `m` meters across (default 2), if no sounding is within a support radius of most of their vertices. Small pieces with soundings under them, such as rock cut from the rest by the trimming, stay. `0` keeps them all. See "Islands". |
| `--decimate`, `-decimate` | `meters` | Enable voxel-grid point decimation with the specified cell size. |
| `--info` | none | Read the datalist, print dataset information (counts, bounds, point spacing) and the reconstruction plan (cell size, chunk plan, cell count), and exit before mesh reconstruction. This option was called `--metadata` in earlier versions; `--metadata` is no longer accepted. |
| `--html`, `-html` | none | Write an X3DOM viewer page, with a heads-up display, a check box for each GLB and the processing log, in the parent of the output directory (`mbmesh_output.html`), and print the command to serve and view it. |
| `--xyz`, `-xyz` | none | Write both `pointcloud-local.xyz` and `pointcloud-ecef.xyz`. |
| `--local-xyz`, `-local-xyz` | none | Write `pointcloud-local.xyz`. |
| `--ecef-xyz`, `-ecef-xyz` | none | Write `pointcloud-ecef.xyz`. |
| `--oriented-ply`, `-oriented-ply` | none | Write `oriented-pointcloud-ecef.ply`. |
| `--pointcloud-glb`, `-pointcloud-glb` | none | Write `pointcloud.glb`. |
| `--normal-glb`, `-normal-glb` | none | Write `normals.glb`. |
| `--origin-glb`, `-origin-glb` | none | Write `origins.glb`. |
| `--track-glb`, `-track-glb` | none | Write `track.glb`, the sensor track as a 3D line. |
| `--tileset`, `-tileset` | none | Write `tileset/tileset.json` and `tileset/L0/tile_I_J.glb`, the surface as an OGC 3D Tiles tileset (see "OGC 3D Tiles"). |
| `--pointcloud-colormap` | `name` | How `pointcloud.glb` is colored by height: `linear` (the default), 11 equal steps between rounded heights chosen from the range, as `mbm_grdplot -G5` does; `equalized`, steps with equal numbers of points, as `mbm_grdplot -G5 -S` does (histogram equalization, using the quanchunks of the heights; GMT's `grdhisteq` weights a geographic grid by cell area, which can move the upper steps by a fraction of the range); `none`, one orange. All use the Haxby colors, from dark blue to white. |
| `--no-colors` | | With `--tileset`, do not color the tiles by height. By default every vertex has a `COLOR_0` from the Haxby table (as `mbm_grdplot` uses) over the heights of the whole survey, so that all tiles agree, stored as bytes, or with Draco as floats quantized to 10 bits (CesiumJS decodes a Draco color of bytes wrongly); the base color is white so the colors show unchanged. The tile metadata and `tileset.json` extras record the bounds as `heightColors`, and the `--html` page then has a "color by height" checkbox that switches the vertex colors off, which shows the tiles in gray (192 of 255, MeshLab's). |
| `--lift` | `[=meters]` | With `--tileset`, lift the tileset above the ellipsoid along the local vertical, in the `transform` of `tileset.json`. As `--geoorigin` works: without it the tileset has the true heights, with a warning if any are below the ellipsoid; alone it takes the default; with `--lift=900` it is that many meters, the value attached. X3DOM's `GeoTileset` takes the height of the camera as at least 1 m, so tiles below the ellipsoid, as seafloor is, never get near enough to refine. The default lift puts the lowest point of the mesh 10 m above the ellipsoid, to a meter, if it is lower; it is logged and written as `liftMeters` in the extras of `tileset.json`. The `--html` page lifts its other models, with `GeoTransform` nodes, and the viewpoints the same, and sets the `maximumScreenSpaceError` of the `GeoTileset` to 1 pixel. The Copy GeoViewpoint button takes the lift off, so the option it gives is for the true heights. |
| `--cesium-html`, `-cesium-html` | none | Also write a viewer page for CesiumJS, with a check box for each GLB and tileset the `--html` page has (and the "color by height" check box of a tileset), beside the output directory (`mbmesh_output_cesium.html`) and inside it (`mbmesh_cesium.html`). The globe of Cesium would hide data below the ellipsoid, so the page starts with it hidden and has a check box that shows it, translucent. Tilesets are shown at their true height whatever `--lift` says. It has the Viewpoint menu, `--geoviewpoint` viewpoints, the Copy viewpoint button and the scale axes of the `mbglb2tiles` page, and no chunk outlines or labels. It is written by `io/cesium_writer.cpp`, which `mbgrd2gltf` and `mbglb2tiles` use too. |
| `--cesium-url` | `url` | The directory of the CesiumJS build of the `--cesium-html` page, with `Cesium.js` and `Widgets/widgets.css` in it. Defaults to release 1.114 from Cesium's web site. |
| `--x3dom-url` | `url` | The X3DOM script of the `--html` page, and the stylesheet beside it. Defaults to the build on `www.x3dom.org`, which has no `GeoTileset`; use a build of X3DOM master (October 2026 or later) to see a tileset. |
| `--raw-mesh-glb`, `-raw-mesh-glb` | none | Write `raw_mesh.glb`, the untrimmed mesh before support trimming. |
| `--diagnostics`, `-diagnostics` | none | Write `pointcloud.glb`, `normals.glb`, and `origins.glb`. |
| `--all-outputs`, `-all-outputs` | none | Write every optional output currently supported. |
| `-V`, `--verbose` | none | Enable progress and diagnostic logging. |
| `-H`, `-h`, `--help` | none | Print command usage and exit. |

Notes:

- The final `mesh.glb` is always written for successful runs without `--info`.
- `--xyz` overlaps with `--local-xyz` and `--ecef-xyz`.
- `--diagnostics` overlaps with `--pointcloud-glb`, `--normal-glb`, and `--origin-glb`.
- Point decimation is disabled by default and enabled only with `--decimate <meters>`.

## Output Files

### Default Output

1. **`mesh.glb`** - Final support-trimmed triangle mesh
   - Always written on successful reconstruction runs
   - Uses binary glTF format
   - Vertices are ECEF, or ECEF offsets from the GeoOrigin with `--geoorigin`, matching `mbgrd2gltf`
   - Intended for web and desktop GLB viewers

### Georeference and provenance metadata

Every GLB records where it is on the Earth and how it was made in `asset.extras`, in three blocks:

- `georeference` - the coordinate reference system (`EPSG:4978`), whether the vertices are `absolute` or `relativeToCenter`, the `center` (the GeoOrigin as ECEF meters; absolute = vertex + center), the same point as `geodeticCenter` (longitude, latitude, height), the `heightReference` (`ellipsoid`) and the `extent` of the data
- `provenance` - the software and its version, the time of execution (`startedAtTime`, UTC; linked to W3C PROV-O through a JSON-LD `@context`), the command line, working directory, user and host, and the inputs (datalist, MBIO formats, swath files, first and last ping time)
- `MB-System` - what this file holds (`content.role`: `mesh`, `chunk`, `raw_mesh`, `pointcloud`, `normals`, `origin_rays` or `track`), the sounding counts, the reconstruction settings, and the vertical reference

The viewer page (`--html`) carries the same tree in a `GeoMetadata` node (the `summary` keywords, and the full tree as a `MetadataSet` named `extras`) and a `WorldInfo` node. All are written from one tree, so they cannot disagree. The names are a proposal for other software to adopt, described in [GEOREFERENCE.md](GEOREFERENCE.md), which also lists where each name comes from and what the files do not say (the vertical reference has no geoid correction). `mbgrd2gltf` writes the same blocks.

### OGC 3D Tiles

`--tileset` writes the reconstruction as an OGC 3D Tiles 1.1 tileset in `<outputdir>/tileset/`: one `L0/tile_I_J.glb` for each reconstruction chunk (one tile, `L0/tile_0_0.glb`, for a single-pass run) and `tileset.json`. The tileset does not depend on `--geoorigin`. `mbglb2tiles` writes the same layout into the same directory by default, with coarser levels `L1`, `L2` and so on as well, made from `mesh.glb`: running it replaces this one-level tileset with the one of levels of detail, and either is found by the survey page.

- **Content.** Each tile's vertices are in the local east-north-up frame of the reconstruction, written Y-up as glTF requires and as 3D Tiles expects: X is east, Y is up and Z is south. Loaders turn Y-up into Z-up with a quarter turn about X.
- **Placement.** The `transform` of the root tile is that east-north-up frame at the frame origin in WGS 84 ECEF (a 4x4 matrix in column-major order whose columns are east, north, up and the origin).
- **Structure.** The root tile has no content and the tiles are its leaf children, each with a box bounding volume (in the east-north-up frame) and a geometric error of 0. The geometric error of the root is the diagonal of its box, so a viewer always refines to the tiles and loads those in view. There is only one level of detail for now.
- **Metadata.** `asset.extras` of each tile and `extras` of `tileset.json` hold the same `georeference`, `provenance` and `MB-System` blocks as the other GLBs (see [GEOREFERENCE.md](GEOREFERENCE.md)), with `vertexCoordinates` `localFrame`, and the frame origin as the `center`.
- **Viewing.** With `--html` the page shows the tileset with a `GeoTileset` node, with the GeoOrigin as its child node if there is one, in place of `mesh.glb`. X3DOM has a `GeoTileset` node only since its master of October 2026 ([PR 1364](https://github.com/x3dom/x3dom/pull/1364)); the build on `www.x3dom.org` does not, so point the page at a newer build with `--x3dom-url`.

### Chunked Output

When reconstruction is chunked (see "Chunked Reconstruction"), `mesh.glb` is still written as the merged mesh, and each chunk is also written as `chunks/chunk_I_J.glb`, with vertices placed exactly as in `mesh.glb`.

### Draco compression

The triangle meshes are written with Draco compression (the `KHR_draco_mesh_compression` extension of glTF, `extensionsRequired` in the file), with the code that `mbmesh` and `mbgrd2gltf` share, in `src/mbdraco` (the Draco encoder and the quantization, the options `--no-draco`, `--draco-precision` and `--draco-normal-bits`, the defaults, and the glTF primitive that is written, which `mbgrd2gltf` takes from here, so the two programs compress alike); CMake and autotools build it as the library `mbdraco` (`libmbdraco.la`), and a build without it writes the arrays and says so in the log. The quantization of the positions is over the largest range of the three axes of a file, so the bits are chosen for each file from its range to give the step that `--draco-precision` asks for, which is the thing that the fixed 16 bits that `mbgrd2gltf` used to have could not do for a survey of kilometers: a chunk of 3 m takes 12 bits for a millimeter, a block of 120 m takes 17, and 16 bits over 1.5 km would be 2 cm. The normals go in an octahedral encoding with `--draco-normal-bits` bits. A file says what it was compressed with in `MB-System.content.compression` (`positionBits`, `positionStep` in meters and `normalBits`). Each mesh is decoded again right after it is encoded, to check it and to count what it holds, since Draco can make a vertex that touches the surface twice into two. On one chunk of 143,477 vertices at `-L 0.1` with a GeoOrigin, `mesh.glb` went from 6.8 MB to 0.66 MB (13 bits over 6.7 m, a step of 0.8 mm). A viewer needs a Draco decoder: the X3DOM builds that the viewer page loads have one (`x3dom-full.js` has `KHR_draco_mesh_compression` support and a WASM decoder), and CesiumJS, three.js and Babylon.js do. The extension is for triangles, so the point cloud (`pointcloud.glb`), the normal lines, the sensor positions and the track are written as arrays.

### Optional Outputs

2. **`tileset/tileset.json`** and **`tileset/L0/tile_I_J.glb`** - OGC 3D Tiles tileset
   - Written with `--tileset` or `--all-outputs`; see "OGC 3D Tiles" below
   - A part of the surface is loaded at a time, so a survey too large for one GLB can be viewed (a browser will not load a GLB of more than 2 GiB)

2. **`raw_mesh.glb`** - Raw marching-cubes mesh
   - Written with `--raw-mesh-glb` or `--all-outputs`
   - Useful for comparing the mesh before and after support trimming

3. **`<outputdir>.html`** - X3DOM viewer page
   - Written with `--html` or `--all-outputs`, in the parent of the output directory and named after it (`mbmesh_output.html` for `mbmesh_output`). The parent is the web root, so the page refers to its models by paths relative to it (`mbmesh_output/mesh.glb`)
   - Titled with the last three directories of the web root, which identify the survey, for example `OceanImaging2018/20180923d1/multibeam`
   - Laid out like the page `mbgrd2gltf` writes: a heads-up display with the navigation mode, the Stats and Log debug displays, and the Show Everything and Reset View buttons, and a **Processing Log** panel, opened with the button at the bottom left, that shows the log of the run. The log is not written into the page, which would make it large and hard to edit by hand: it is the text file `mbmesh_output/mbmesh.log`, one message a line, and the page loads it (with `fetch`) the first time the panel is opened, so the page has to be served over HTTP, as it does for its GLBs
   - Also written inside the output directory as `mbmesh.html`, with paths relative to that directory and without the GLBs of the web root
   - Has a check box, grouped under headings, for every GLB the page can show: first the `.glb` files in the output directory (`mesh.glb`, then `track.glb`, then the others such as `pointcloud.glb`, `normals.glb` or `raw_mesh.glb`), then the `.glb` files in the web root beside the output directory, such as the product of `mbgrd2gltf`. `mesh.glb` and `track.glb` are on when the page opens; the others are off and are not loaded until they are first switched on, so a large file costs nothing until it is wanted. The chunk files in `chunks/` are not listed, since they repeat `mesh.glb`. At most 200 models are listed
   - GLB files from the web root line up with the mesh only if they use the same coordinates: the same GeoOrigin, for `mbgrd2gltf` the same `--geoorigin` value as `mbmesh`. The page says so under that heading
   - Always has a `GeoViewpoint` over the center of the data; with `--geoorigin` it also declares the `GeoOrigin` that the viewpoint and the vertices are relative to, as `mbgrd2gltf` does
   - The processing log holds every message of the run, up to `DEBUG` level, whether or not `-V` was used: the command line, the settings (level of detail, Poisson cell size, point spacing, chunking), the timing of each stage and of the whole run, and, per chunk when chunking, each stage of each chunk. It is capped at 20,000 messages, and the page says how many were left out. The page is written last so that the log is complete
   - The command to serve the web root is printed when the page is generated; mbmesh does not start a server or browser

4. **`pointcloud-local.xyz`** - Local Cartesian point cloud
   - Written with `--local-xyz`, `--xyz`, or `--all-outputs`
   - Uses the same local reconstruction frame as `mesh.glb`

5. **`pointcloud-ecef.xyz`** - ECEF point cloud
   - Written with `--ecef-xyz`, `--xyz`, or `--all-outputs`
   - Uses WGS84 Earth-Centered, Earth-Fixed coordinates

6. **`oriented-pointcloud-ecef.ply`** - Oriented point cloud
   - Written with `--oriented-ply` or `--all-outputs`
   - Includes ECEF position, ECEF normal, and PCA lambda values

7. **`pointcloud.glb`** - Diagnostic point-cloud GLB
   - Written with `--pointcloud-glb`, `--diagnostics`, or `--all-outputs`

8. **`normals.glb`** - Diagnostic normal-vector GLB
   - Written with `--normal-glb`, `--diagnostics`, or `--all-outputs`

9. **`origins.glb`** - Diagnostic sensor-origin ray GLB
   - Written with `--origin-glb`, `--diagnostics`, or `--all-outputs`

10. **`track.glb`** - The sensor track as a 3D line
    - Written with `--track-glb` or `--all-outputs`
    - One position per ping, taken from the navigation and sensor depth that MBIO supplies for the datalist (so processed files and alternate navigation files are used as the datalist specifies), drawn as line segments in the same coordinates as `mesh.glb`, including any `--geoorigin` offset, so the two overlay directly
    - Only pings that contribute at least one accepted sounding are drawn, so `-R` bounds trim the track to the area of the mesh
    - The track is broken wherever the swath file changes, the time runs backwards, or the time between two pings is more than ten times the median ping interval of that file, so the lines do not run straight across the turns between survey lines or across gaps in the data
    - Checked against `mbnavlist` for 1,372 pings of one file: the position and sensor depth of every ping agree to better than 0.1 mm
    - Checked against `mesh.glb` with an explicit GeoOrigin, the automatic GeoOrigin and none: the track keeps the same position relative to the mesh in all three (identical to 0.1 mm with a GeoOrigin, within 32-bit float resolution, about 0.3 m, without one), and adding each run's GeoOrigin back gives the same absolute ECEF coordinates

## Pipeline Summary

The current pipeline is orchestrated by `src/mbmesh/mbmesh.cpp`:

1. Parse CLI options.
2. Read and preprocess the MB-System datalist.
3. Print metadata and exit if requested.
4. Optionally decimate collected points.
5. Reconstruct the surface, either in one pass or chunk by chunk (steps 5a-5d are the same either way, and run once per chunk when chunked):
   - 5a. Estimate oriented normals.
   - 5b. Reconstruct a screened-Poisson scalar field.
   - 5c. Extract a raw mesh with marching cubes.
   - 5d. Trim unsupported mesh regions.
6. Write `mesh.glb`, per-chunk GLBs when chunked, and requested optional outputs.
7. Write the viewer page and print the command to view it if `--html` was requested.

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

Soundings and sensor positions are converted to WGS84 ECEF while reading and held in absolute ECEF until all files have been read. The reconstruction frame origin is then chosen (the point on the ellipsoid below the centroid, rounded to 1e-6 degrees) and every point is converted in place to the local east-north-up frame described in the Overview. Sounding elevation is stored as negative bathymetry and sensor elevation as negative sensor depth, both treated as heights above the WGS84 ellipsoid. The conversions live in `include/io/coordinate_frame.h` and `src/io/coordinate_frame.cpp`.

On output, mesh and diagnostic GLB vertices are converted from the local frame back to ECEF and offset by the `--geoorigin` position when one is used (normals rotate but do not translate).

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
- Nearest-neighbor spacing statistics. Consecutive soundings are spatially adjacent (consecutive beams of consecutive pings), so spacing is measured inside blocks of consecutive points, 16 blocks of 16,384 points spread through the dataset (the whole dataset if it has fewer than 262,144 points). This is the spacing along a survey line, and it does not depend on how many points there are. An earlier version searched a subsample of 8,192 points spread over the whole dataset, which overstated the spacing by about the square root of the number of points divided by 8,192, so dense surveys looked sparse and were reconstructed far too coarsely.

Use metadata mode to inspect a dataset before reconstruction:

```bash
./build/src/mbmesh/mbmesh -I datalist.mb-1 --info
```

### 3. Level of Detail and Automatic Defaults

Implemented in:

- `settings.cpp`
- `settings.h`

The `-L`, `--lod`, or `--level-of-detail` option is the main user-facing reconstruction control. It represents the requested smallest feature size in local meters. If it is omitted, `mbmesh` derives an automatic LOD from the estimated average point spacing.

The Poisson cell size is a quarter of the level of detail, but no finer than half the point spacing, since the data cannot support more: `cell = max(L / 4, spacing / 2)`. It is not capped at the spacing, so `-L` is honored even when the points are much denser than the level of detail asked for. To match a 5 cm grid, ask for `-L 0.2`.

The automatic level of detail is the finest the data supports, twice the point spacing, unless that would need more than about 500 million Poisson cells across the survey's bounding box, in which case it is raised to fit and an `INFO` message says so. Work grows as the cube of the resolution, so a dense survey would otherwise ask for a centimeter-scale cell and run for days. A level of detail given with `-L` is never limited, but a `WARN` message is printed when it needs more than about 2 billion cells, since that may take hours. `--info` shows the resulting cell size, chunk plan and cell count before you commit to a run.

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

#### Voids in the surveyed surface

Sound returns less from the floor of a depression and from under an overhang than from its rim, so the soundings there are sparse. A vertex of the screened Poisson surface bridging the gap fails the neighbor test (too few samples within the support radius, which is a few cells), and trimming removes the surface across the whole void, leaving holes inside a surface that is otherwise there.

`--void-size` (default 2 m) puts these back. Before reconstruction, a raster of the soundings of the whole survey is made (cells of 1/32 of the void size, 0.06 m for the default), keeping the lowest and highest height of the soundings in each cell. The raster is of the whole survey, so a void that crosses the border of two chunks is seen whole from both. After the normal trimming, a vertex that was rejected only for too few neighbors is put back if a sounding is within three support radii of it: the soundings are sparse there, as in the lanes between lines and in the pinholes that the support radius, chosen from the median spacing of the densest parts of the survey, does not reach. A vertex that is farther from the soundings is put back only if it passes three tests:

- Its nearest sounding is no farther than half the void size.
- Soundings are all around it, in the tangent plane of the mesh. The vertex is probed in eight directions, at twice the distance of its nearest sounding, and a sounding must be within half that distance of the probe, in three dimensions (the raster knows the heights of the soundings of the whole survey and not only those of the chunk). Up to two of the eight may fail. This tells a void, which the soundings surround, from the extrapolation of a surface beyond the edge of its soundings, which has none on the far side.
- Its nearest few soundings are beside it, along the surface, and not above it along its normal. This keeps out the sheet that screened Poisson closes under an overhang or beyond the edge of the data, which hangs below the surveyed surface, so that a raster seen from above cannot tell it from a void.

A vertex rejected because the samples disagree with the surface (the point-to-plane offset) stays out, as does a vertex with no soundings on its far side, so the edge of the survey is trimmed as before.

Finally, small holes that remain inside the trimmed surface are put back, if they pass the second and third test: a patch of rejected triangles that the open boundary of the raw mesh does not touch, so that kept triangles surround it, with an area of up to that of a disk the size of the void (3.1 square meters for the default). The vertices of such a hole were often rejected only because the surface is a few cells off the samples on a rough bottom. The edge of the survey touches the open boundary of the mesh, so it is not filled.

Last, the pieces of the trimmed surface that are not joined to the rest, are smaller than a quarter of that area (0.8 square meters for the default), and do not touch the open boundary of the raw mesh are taken out: the specks that sound reflected from the water column, a fish or a bubble leave, and the end of a sheet of extrapolated surface that the other steps left.

The raster is in the horizontal plane of the local frame, with z up. It takes a few hundred thousand cells for a survey of a few hundred meters, 8 bytes each, and at most 16 million (the cells grow if the survey is larger). The debug log (`-V`) shows its size, and the support-trimming line of each chunk shows how many vertices were put back and how far from a sample they are.

The surface in a void is the interpolation of the screened Poisson solution from the rim. It is smooth and plausible, not measured: it will not show a feature that the sonar did not see.

The tests of the vertices, and the putting back of the vertices short of neighbors, of the vertices of voids and of the vertices of the holes, are split between `--threads` threads (a test of a vertex does not depend on the others, and the results are put together in vertex order, so the mesh does not depend on the number of threads). The connected pieces and the open boundary are found once, by sorting the edges of the raw mesh, and shared by the holes and the islands. On one chunk of 264,030 raw vertices the step took 5.7 s before, 4.8 s with one thread now (the open boundary is found by sorting the edges and not with a hash table), and 1.5 s with eight; on 12 chunks run together, the time summed over the chunks went from 3 min 28 s to 1 min 37 s and the run from 1 min 58 s to 1 min 49 s, with a `mesh.glb` that is the same, vertex for vertex.

### 9. Output Generation

Implemented in:

- `include/io/glb_writer.h`
- `src/io/glb_writer.cpp`
- `include/io/xyz_writer.h`
- `src/io/xyz_writer.cpp`
- `include/io/x3dom_writer.h`
- `src/io/x3dom_writer.cpp`

`write_outputs()` always writes the final mesh as `mesh.glb`. Other output writers run only when their CLI flags are set.

Local XYZ output writes local reconstruction coordinates. ECEF XYZ and oriented PLY outputs convert local points back through the coordinate frame and then into WGS84 ECEF coordinates. The HTML writer creates an X3DOM page in the style of the one `mbgrd2gltf` writes, with the processing log captured from the logger (see "Log Messages"). The page has two overview viewpoints for the same view, looking straight down from above the middle of the data with north at the top: `OVERVIEW_LOCAL`, a plain `Viewpoint` in the coordinates of the GLBs (ECEF, minus the GeoOrigin's ECEF position when there is one), which is bound when the page opens, and `OVERVIEW_GEO`, a `GeoViewpoint` that refers to the GeoOrigin. Choose between them in the Viewpoint menu, or step through them with Page Down. The GeoOrigin is written as a child node of the `GeoViewpoint`, because `geoOrigin` is an SFNode field and X3DOM takes it from a child element: it ignores an attribute naming a DEF, and then places the `GeoViewpoint` in absolute ECEF, millions of meters from models that are relative to the GeoOrigin. Both viewpoints are written so that each can be checked against the other. A GeoViewpoint of your own can be added with `--geoviewpoint=lon,lat,elev,clon,clat,celev` (the camera position and the point it looks at) or `--geoviewpoint=lon,lat,elev,ax,ay,az,angle,clon,clat,celev` (with the orientation as an axis and an angle, as a `GeoViewpoint` takes it, so that no rotation looks level to the north and `1 0 0 -1.5708` looks straight down), which can be repeated and gives `USER_GEOVIEW_1`, `USER_GEOVIEW_2` and so on, written as readable `GeoViewpoint` nodes with the position as latitude, longitude and elevation and the GeoOrigin used again (`<GeoOrigin USE='GRID_ORIGIN'>`). The values are geodetic, in the order of `--geoorigin` (longitude first; the node has latitude first, as X3D does), so they do not depend on the GeoOrigin or the data. The **Copy GeoViewpoint** button of the page puts the option for the view on the screen on the clipboard (the ten-number form) and in the X3DOM log; paste it into the command line of the next run.

When `--html` is used, `print_html_viewer_command()` logs the command to serve the web root, the parent of the output directory (`python3 -m http.server 8000 --bind 127.0.0.1 --directory <absolute web root>`), and the URL to open (`http://127.0.0.1:8000/<outputdir>.html`). It does not start the server. The pages have a scale object: axes East (red), North (green) and Up (blue) at the center of rotation, which the page resizes whenever the viewpoint changes so that they stay about a sixth of the height of the view, with their length in the label, rounded down to 1, 2 or 5 times a power of ten ("5 m", "20 cm", "2 km"). The length is true at the axes, and the perspective makes other distances differ; the "scale axes" check box switches the object off.

## Chunked Reconstruction

Implemented in:

- `include/algorithms/reconstruction.h`
- `src/algorithms/reconstruction.cpp`

A single screened-Poisson grid covers the bounding box of its input. A seafloor survey is a thin sheet through a large 3D box, so the grid is mostly empty, and its memory and run time grow quickly with survey size and level of detail. Chunking splits the survey into square horizontal chunks and reconstructs each one separately.

How it works:

- **Chunk lattice.** Chunk (i, j) owns x in [i*T, (i+1)*T) and y in [j*T, (j+1)*T) in the local east-north frame, where T is the chunk size, a whole number of Poisson cells. The lattice is anchored at the local-frame origin, so chunk indices do not depend on the order of the data.
- **Shared grid lattice.** Every chunk's Poisson grid is snapped to the same global lattice of cell-size steps, so neighboring chunks share grid nodes and their surfaces nearly coincide where they overlap.
- **Halo.** A chunk reads its own points plus a ring of neighboring points, so that points near the chunk edge see the neighborhoods a single run would give them. The halo covers the reach of normal splatting or support trimming plus the normal-estimation radius plus a few cells (0.275 m at `-L 0.1`). `--chunk-halo <meters>` sets it (rounded up to whole Poisson cells; a value under that reach is used, with a warning). A larger halo is not better: it makes the chunk size, which is chosen with the halo in mind (the shrinking of the chunk to keep the threads busy stops at four halos across), and the points each chunk finds its iso-value from different, and on one multibeam survey a 1 m halo left larger mismatches at the chunk edges than 0.3 m did.
- **Seams.** The Poisson fields of neighboring chunks are solved apart, so they differ where the chunks meet, by an additive offset (each field is shifted so that its own samples sit on the iso-value) and by the shape of the solution, and the surfaces have a step between them and triangles that do not share vertices. A chunk therefore waits for the chunks to its west and south, which are finished before it, takes their final fields on the edges it shares with them, and adds to its own field a correction that is the difference there and dies away smoothly (as cos²) over `--seam-blend` meters into the chunk and out of it; then it publishes its own final field on its east and north edges. A chunk's edges are on the shared lattice, so marching cubes makes the same vertices along an edge from the same field values: the meshes meet exactly. See "Seams between chunks".
- **Ownership.** After trimming, a chunk keeps only triangles whose centroid lies inside its half-open square, so each triangle belongs to exactly one chunk and the merged mesh has no overlap.
- **Automatic sizing.** Without `--chunk-size`, mbmesh turns chunking on when the whole-survey grid would exceed about 8 million cells, and chooses the chunk size so each chunk's grid, including halo and padding, is about that size. A chunk's grid is as tall as the relief inside it, so the vertical extent is measured from a sample of the points as the tallest relief in any one chunk of that size, not the survey's full depth range, which would make chunks needlessly small on a sloping seafloor. The memory budget alone can give only a few huge chunks, which leaves most threads idle, so the chunk is then made smaller until at least two chunks per thread contain points, down to a floor of four halos across (so that the halo and padding, which every chunk reconstructs again, stay a modest cost). Asking for more threads therefore gives smaller chunks. An explicit `--chunk-size` is used as given. `--info` prints the resulting chunk size, halo, grid size, number of chunks with points and thread count.
- **Threads.** Chunks are independent, so several are reconstructed at once (`--threads`). The chunks with the most points are started first, so the last chunks to start are the small ones and the threads finish together. Results are delivered in chunk order, so the output does not depend on the thread count or the start order.

**`--seam-method solve`.** The blend takes a free solution, which has the edge effect of its own boundary (the field is held at zero on the faces of the grid, at the edge of the halo), and bends it toward the neighbor over `--seam-blend`. With `solve`, a chunk waits, before its Poisson iterations, for the chunks to its west and south, and the nodes of the grid on the edges that it shares with them are held at the values of the neighbors' final fields and are not solved for, as in the domain decompositions of Bolitho et al. for parallel Poisson reconstruction, solved in an order with Dirichlet values from the neighbor solved before. The chunk is then in the gauge of its neighbors, so its field is not shifted to put its samples on the iso-value (a chunk without those neighbors is shifted as usual), and the halo is only needed on the east and north sides. On the 12-chunk region of a multibeam survey at `-L 0.1`, the vertices within 0.15 m of a chunk edge that were more than 0.3 m from a sounding went from 4 to 9% to under 0.1%, vertices more than 0.3 m from a sounding went from 12,687 to 6,730 of the whole mesh and the farthest from 0.999 to 0.659 m, and the pieces of the merged mesh from 45 to 36; because a chunk cannot start its solve before its two neighbors have finished theirs, only as many chunks as the shorter side of the block of chunks are solved at the same time, so the threads that have no chunk to solve share the sweeps of the solver (the red-black order of `sor`, and each sweep of `jacobi`, are split into slabs of z, and the solution does not depend on the number of threads): the run took 1 min 59 s on 8 threads, against 6 min 16 s with the solver on one thread per chunk and 2 min 19 s for `--seam-method blend`, and the raw mesh is the same in the two `solve` runs, vertex for vertex. A chunk alone, with 8 threads in its solve, took 11 s for the solve instead of 80 s. It has been checked on that one region.

#### Islands

The mesh of a survey in chunks, put together, can have pieces that are not joined to the rest: slivers and blobs that the trimming of a chunk left when it took away their connection, or that sound reflected in the water column made, in part, in the chunks around a seam. On a region of 12 chunks of a multibeam survey at `-L 0.1` the merged mesh had 45 pieces; 95% of the triangles were in the largest. The small ones were of two kinds: pieces of 0.1 to 0.4 square meters with a median distance of 0.7 to 0.9 m to the nearest sounding (nothing is there), and pieces of 1 to 4 square meters within 0.05 to 0.15 m of the soundings (real rock or ledges, cut from the rest). After the chunks are put together, `--island-size` (default 2 m, the area of a disc that wide) removes the pieces of at most that area for which fewer than half the vertices have a sounding within one support radius, in three dimensions, as the raster of the soundings used for `--void-size` knows them (cells of 1/32 of the void size). The pieces with soundings under them stay. Only `mesh.glb` is cleaned; `raw_mesh.glb` is not. The raster keeps only the lowest and highest height of the soundings of a cell, and where lines overlap and scatter that range is wide (median 0.18 m in a cell of 0.06 m), so a piece floating over such a cell can still count as supported: on the region above, 14 of the 45 pieces were removed and two floating ones of 2,061 and 423 triangles were not.

#### Seams between chunks

Two chunks that were solved apart do not meet: on a multibeam survey at `-L 0.1` the two surfaces on either side of an edge of two chunks differed in height by a median of 19 mm (up to 94 mm), all the same way along the 3 m edge, and a tenth of the vertices on the edge had no partner within 6 cm. `--seam-blend` (on by default) fixes that at the source rather than moving vertices afterward:

- The grids of all chunks have their nodes on one lattice, and the edges of a chunk are lattice planes, so two neighbors have nodes at the same places on the plane they share.
- A chunk, after its Poisson solve and before marching cubes, takes the final field of its west and its south neighbor on that plane (a plane of nodes, kept in memory until the chunk that needs it has used it), and adds the correction `w_west(d_x) * (neighbor - own)` on the west plane and the same for the south plane, where `w` is 1 on the plane and falls smoothly to 0 at the blend width. In the corner where both are not zero, the south term has the value that the west term has at their meeting subtracted, so that each plane has exactly its own values. Where the neighbor's grid is not as tall as the chunk's, the correction of its highest or lowest node is used.
- The field on the plane is then the neighbor's own, to the last bit, so the marching-cubes vertices on the plane are the same points, in both meshes. The triangles on the two sides are assigned by centroid as before, so together they are exactly the surface that marching cubes would make over the matched field.
- Chunks are started in chunk order (not largest first), because a chunk waits for its west and south neighbors, which come before it. The threads still run the normal estimation and the Poisson solves of the chunks that wait; only the field matching waits. On a run with the soundings on disk this adds the memory of the waiting fields, at most one for each thread. The result does not depend on the thread count.
- The vertices that the chunk meshes have in common are separate in each chunk file and are merged in `mesh.glb` and `raw_mesh.glb` (vertices within a micrometer), so the merged mesh has no seams, as an edge or as a gap. The files of the chunks, and the tiles of a tileset, are as they were, but meet exactly.

Measured on that survey (six chunks of 3.025 m at `-L 0.1`, a 3 by 2 block): the vertices of all seven edges between the chunks coincide to under a micrometer, on a plane to 0.001 mm, and of about 1,400 seam vertices only two (at corners) had no partner in the neighboring chunk. One thread and four threads gave identical meshes, in `mesh.glb` and in every chunk file, and four threads took 1 min 41 s against 3 min 9 s for one. The two chunks that were solved apart, which had two separate pieces in `mesh.glb`, became one.

What it does to the surface: within the blend width of an edge the surface moves toward the neighbor's by the offset between the two, a centimeter or two on the survey above, which is a slope of about a degree over the width. Farther from the edges it is the chunk's own. The trimming of the two chunks is decided, for the vertices on the edge, from the same field and the same soundings (the halo reaches farther than the support radius), and on the survey above both chunks kept and removed the same vertices along the whole edge; the voids that trimming leaves at a seam are another matter (see the cleanup of islands and voids).

Measured on one 175 MB swath file (993,245 accepted soundings, about 50 m by 37 m) at `-L 2`, a single run took 207 s and 1.4 GB. Chunked with 10 m chunks, it took 254 s on one thread and 69 s on four threads at a peak of 0.76 GB, with 151,057 triangles against 151,148 for the single run. The median distance from a chunked vertex to the nearest single-run vertex was 4.9 cm, about the vertex spacing. At what was then the default level of detail (a 0.098 m cell from a spacing estimate that was too large, about 102 million cells for the whole survey, which a single run refuses to attempt) automatic chunking produced 20 chunks of about 8.3 m and 528,208 triangles in 232 s at a peak of 1.8 GB on four threads. Three survey-edge chunks with fewer than 32 owned points were skipped.

On the same file at `-L 2` with automatic chunking, 4 threads took 66 s for 8 chunks with points, 8 threads 45 s for 16 chunks, and 12 threads 48 s for 20 chunks, so beyond about 8 threads the gain stops.

### Surveys larger than memory

Chunking alone bounds the Poisson grids, not the soundings. A `CollectedPoint` is 48 bytes (the sounding and the sensor position that measured it, three doubles each), and growing the vector that holds them as they are read can need about two and a half times that at its peak. One 600 MB swath file with 650,000 accepted soundings is nothing, but a survey of 515 million soundings is 25 GB of points and well over 60 GB at the peak, and the operating system ends a run that asks for that. On 28 files with 18 million soundings the peak after reading was 2.3 GB, which is 127 bytes per sounding.

So a survey with more accepted soundings than `--spill-points` (50 million unless it is changed; 0 always does) does not keep them in memory:

1. **Reading.** The soundings read so far are written to `mbmesh_work/soundings.bin`, as absolute ECEF, and so is each one after them. The log gives the free disk space there when the spilling starts. The reading itself is as slow as ever, but its memory no longer grows. (The first 50 million soundings are held and written when the 50 millionth is passed; the vector that holds them grows to about 2.4 GB first.)
2. **Measuring.** One pass over the file, in pieces of 4 million points, puts the points in the local frame and finds what an in-memory run finds from its points: the bounds, the geodetic bounds and mean elevation, the point spacing (from the same blocks of consecutive points, picked out of the pieces as they go by, so the result is the same), and a sample of about a million points from which the chunk size is chosen. From these the level of detail, the Poisson cell and the chunk size are chosen as always. A survey that turns out to need no chunking is read back into memory, since it is small enough.
3. **Binning.** A second pass sorts the points into one file per chunk in `mbmesh_work/chunks`, each holding the points the chunk owns and those in its halo, so that points near a chunk edge are in more than one file (about 1.3 times the soundings in all for a 3 m chunk with a 0.3 m halo, and about 3.5 times for a 1 m halo). If `--decimate` is on, the points are averaged into voxels first, in the same way and with the same voxels as `point_decimation` in memory, so memory then follows the number of occupied voxels. The spill file is deleted when the pass is done.
4. **Reconstruction.** Each chunk reads its own file, so only the chunks being worked on are in memory, and gives exactly the mesh it gives from memory (the tests compare them).
5. **Cleaning up.** `mbmesh_work` is deleted when the run ends, however it ends.

The disk needs 28 bytes for each sounding for the spill file (the position as three doubles and the number of its ping, since the sensor position is the same for all the soundings of a ping and is kept once, in the track), so 14 GB for 515 million soundings. The chunk files hold 48 bytes for each point, about 1.3 times as many as there are points because of the halos, and they exist while the spill file does, so with `--decimate` they are small and without it they are larger than the spill file. If the disk fills, the run stops with an error that says so; use `-O` to put the results, and with them `mbmesh_work`, on a disk with room. The work directory is beside the results so that it is on a disk with room for them.

The outputs that hold points in one file are limited, since they are written from memory. `pointcloud-local.xyz` and `pointcloud-ecef.xyz` are written in full, a chunk at a time, but `pointcloud.glb`, `origins.glb`, `normals.glb` and `oriented-pointcloud-ecef.ply` hold one point in n, chosen so that there are at most 5 million, and the log says which n. `--all-outputs` therefore works on a survey of any size, but writes text files that are as large as the data.

Each stage logs the peak memory of the process so far (`peak memory 2.3 GB`, from `getrusage`), at the end of reading, after measuring, after binning, with each progress line of the chunking, at the end of the reconstruction and at the end of the run, so that the stage that needs the memory is the one the line follows.

Current limitations:

- A survey of up to `--spill-points` soundings (default 50 million) is read into memory before chunking, and only the Poisson grids are chunked. A larger one is spilled to disk and binned into chunk files; see [Surveys larger than memory](#surveys-larger-than-memory).
- With `--seam-blend 0` each chunk computes its own iso-value shift from its own samples and the chunks are not matched, so there are steps and cracks at chunk edges. By default the fields are matched along the edges (see "Seams between chunks"); this has been checked on one pair of chunks of one multibeam survey, and on a synthetic field. Where a chunk has no neighbor to its west or south (the neighbor was skipped as sparse or has no points) the edge is not matched.
- Per-chunk run time is not reduced, only memory per run, so small surveys can run slower chunked on one thread than in one pass because the halo and padding are reconstructed more than once.
- Boundary triangles are assigned by centroid and are not clipped at chunk edges.

## Legacy Implementation

The previous implementation in `src/mbmesh/legacy/` was created by the Spring 2026 CSUMB Capstone project team. (The mesh reconstruction pipeline that replaced it, and the layout of the source in `include/`, `src/algorithms/`, `src/io/` and `tests/`, were constructed by the Summer 2026 CSUMB Capstone team of Daniel Karnofel and Benjamin Shimek.) It read MB-System datalists and generated point-cloud outputs:

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

**mbmesh** reconstructs bathymetry meshes from swath datalists, in one pass or in chunks, writes them as GLB (Draco-compressed by default) and as an OGC 3D Tiles tileset, and writes X3DOM pages to view them. **mbglb2tiles** builds levels of detail from a mesh GLB. Both are in use on multibeam and lidar surveys. Some parts are still research or prototype quality.

What works:

- An exact ECEF-derived east-north-up frame, and `--geoorigin` as `mbgrd2gltf` has it; products of the two overlay.
- Chunked, threaded reconstruction with fields matched on shared chunk edges (`--seam-blend`), so merged meshes meet; surveys larger than `--spill-points` soundings are kept on disk, so memory follows the size of a chunk.
- A single-pass and a chunked run of one swath file agree to a median vertex distance of 4.9 cm; OceanImaging2018 20180923d1 (51.8 million soundings) went from 2 h 55 min in one pass to 40 min chunked on 4 threads, and the `sor` solver is faster still.
- `--tileset` writes a tileset of the chunks with a root ENU-to-ECEF `transform`; `mbglb2tiles` adds the coarser levels with `REPLACE` refinement. Both write the same `tileset/` layout, find each other's output in `mbmesh_survey.html`, and refine in X3DOM's `GeoTileset` (`--lift` is needed over seafloor, where the camera is below the ellipsoid).
- The tiles are colored by height (`COLOR_0`, the Haxby table over the heights of the whole survey) unless `--no-colors`, and the pages have a "color by height" checkbox.

Known limitations:

- The tilesets are validated with `3d-tiles-validator` 0.6.1 (no errors or warnings, though it cannot look inside Draco), load in X3DOM, and load in CesiumJS 1.114 through `mbglb2tiles --cesium-html`; that was tried on a small test tileset in headless Chrome, not on a large survey.
- The coarser levels of a tileset come from clustering the finished mesh (`mbglb2tiles`), not from reconstructing again from decimated points; `mbmesh --tileset` writes only the finest level. `mbglb2tiles` holds the whole mesh in memory (about 12 GB at its peak for a mesh of 38 million vertices, with the weld taking about 5 minutes and the first clustering about 2.4), so there is no out-of-core path from the chunk files.
- A tileset has one frame, at its center, for all its tiles; a survey large enough that the Earth's curvature matters across it would need a frame for each tile.
- The height colors are one table with the heights of the survey; there is no other colormap for a tileset, and no color from the data (backscatter, for example). The page's checkbox reaches the tiles through X3DOM's internal nodes, so a new X3DOM could break it.
- The CMake build and its tests (`ctest`) work, and CMake is the build that is kept up to date. The autotools build works with a `configure` and `Makefile.in` files regenerated from the `Makefile.am` files (the sequence is in `configure.cmd`, with automake 1.18.1); the committed ones are not regenerated for these programs. A build without Draco writes uncompressed meshes, and `mbglb2tiles` cannot read a Draco-compressed input then.
- Reconstruction is a single end-to-end run; there is no reusable preprocessing artifact yet.
- Decimation is available through `--decimate`, but remains off by default because it trades reconstruction detail for lower downstream cost.
- Screened Poisson uses a regular grid, so memory per chunk grows with grid size; a survey of fewer soundings than `--spill-points` is held in memory.
- Normal estimation is sensitive to sparse data, sharp edges, and inconsistent spacing.
- Support trimming can over-trim when normals or thresholds are poor, and boundary triangles are discarded rather than clipped when only part of a triangle is unsupported.
- Tiles with fewer than 32 owned points are skipped, which drops a few points at the edge of a survey.
- Viewing a page needs `python3` (or any local web server) and a browser; mbmesh only prints the command.

Future work:

- Validate the tilesets with `3d-tiles-validator` and CesiumJS.
- Make the coarser levels from decimated points, and build them from the chunk files without holding the whole mesh.
- Decimate while reading, and write the point outputs of a survey on disk in full, which are limited to 5 million points now.
- Expose selected reconstruction parameters as documented CLI options.
- Investigate octree/adaptive Poisson reconstruction.
- Improve normal estimation near boundaries and sharp orientation changes.
- Clip support-trimmed boundary triangles instead of dropping them.
- Add full CLI regression tests using small real or synthetic datalists.
- Regenerate the committed `Makefile.in` and `configure` for the autotools build.

## Source Layout

### Main Application

- **mbmesh.cpp** - CLI parsing, pipeline orchestration, output selection, help text, and the viewer command message
- **settings.cpp** - Runtime defaults, metadata computation, spacing estimation, bounds handling, and metadata output
- **settings.h** - CLI option state and metadata structures

### Algorithm Interfaces

- **include/algorithms/point_decimation.h** - Point decimation options and interface
- **include/algorithms/normal_estimation.h** - Normal estimation options and interface
- **include/algorithms/screened_poisson.h** - Screened-Poisson options and scalar-grid interface
- **include/algorithms/marching_cubes.h** - Marching-cubes options and mesh extraction interface
- **include/algorithms/support_trimming.h** - Support-trimming options, diagnostics, and interface
- **include/algorithms/seam_blending.h** - Matching the fields of neighboring chunks on their shared edges, and merging the coincident vertices of the chunk meshes

### Algorithm Implementations

- **src/algorithms/point_decimation.cpp** - Voxel-grid point decimation
- **src/algorithms/normal_estimation.cpp** - PCA normal estimation with sensor-origin orientation
- **src/algorithms/screened_poisson.cpp** - Regular-grid screened-Poisson scalar-field reconstruction
- **src/algorithms/marching_cubes.cpp** - Triangle extraction from scalar grids
- **src/algorithms/support_trimming.cpp** - Support-based mesh filtering
- **src/algorithms/seam_blending.cpp** - Field planes, the correction that makes a chunk match its neighbors, and the weld of coincident vertices

### Data Structures and Math Helpers

- **include/data_types/geometry.h** - Point, mesh, bounds, reconstruction-domain, and grid data types
- **include/data_types/swathfile.h** - Swath-file data types
- **include/util/stopwatch.h** - Wall-clock timer used for the timing messages
- **include/util/parallel.h** - `parallel_for_ranges`, which splits a range between threads (the solver sweeps and the support trimming)
- **include/util/logger.h** - Header-only logger producing the same line format as `mbgrd2gltf`
- **include/math/vec2.h**, **vec3.h**, **mat3.h** - Basic vector and matrix helpers
- **include/math/eigen.h** - Symmetric 3x3 eigen decomposition
- **include/math/kdtree.h** - KD-tree nearest-neighbor search

- **include/algorithms/reconstruction.h** and **src/algorithms/reconstruction.cpp** - Per-chunk reconstruction pipeline and the chunked, threaded chunk driver

### I/O

- **include/io/coordinate_frame.h** and **src/io/coordinate_frame.cpp** - Geodetic, ECEF, and local east-north-up conversions
- **include/io/datalist_reader.h** and **src/io/datalist_reader.cpp** - MB-System datalist and swath reading
- **include/io/glb_writer.h** and **src/io/glb_writer.cpp** - Mesh, point, normal, and origin-ray GLB writers
- **../mbdraco/draco_mesh.h** and **draco_mesh.cpp** - Draco encoding of a triangle mesh and of a glTF primitive, shared with `mbgrd2gltf` (a stub without Draco), and a decoder for tests
- **include/io/tileset_writer.h** and **src/io/tileset_writer.cpp** - OGC 3D Tiles output: tile content in the local frame, Y-up, and `tileset.json`
- **include/io/provenance.h** and **src/io/provenance.cpp** - The georeference and provenance metadata tree written to GLB extras and to the page's GeoMetadata and WorldInfo nodes
- **include/io/track.h** and **src/io/track.cpp** - Splits the sensor track into lines, breaking it at file changes and gaps
- **include/io/xyz_writer.h** and **src/io/xyz_writer.cpp** - Local XYZ, ECEF XYZ, and oriented PLY writers
- **include/io/x3dom_writer.h** and **src/io/x3dom_writer.cpp** - HTML/X3DOM viewer writer, in the style of the `mbgrd2gltf` page, with the processing log

### Tests

- **tests/test_marching_cubes.cpp** - Marching-cubes test program
- **tests/test_normal_estimation.cpp** - Normal-estimation test program
- **tests/test_support_trimming.cpp** - Support-trimming test program
- **include/algorithms/coverage_footprint.h** - Raster of where the soundings are and how high, for the support trimming
- **tests/test_coverage_footprint.cpp** - Footprint and void-restoration test program
- **tests/test_seam_blending.cpp** - Seam matching tests: the field on the plane equals the neighbor's, nothing changes beyond the width, a corner keeps both planes, marching cubes gives the same vertices on the plane, and the weld
- **tests/test_chunking.cpp** - Chunked reconstruction test (accuracy, coverage, ownership, thread-count independence) on a synthetic surface
- **tests/test_x3dom_writer.cpp** - Viewer page writer tests (controls, GeoOrigin, track checkbox, log escaping)
- **tests/test_tileset.cpp** - Tileset tests: Y-up content and the transform give back the ECEF position of each vertex, and the structure of `tileset.json`
- **tests/test_provenance.cpp** - Metadata tests: GLB extras equal the metadata tree, absolute = vertex + center, the page says the same
- **tests/test_track.cpp** - Track splitting and track GLB writer tests
- **tests/test_coordinate_frame.cpp** - Geodetic/ECEF/local frame conversion tests
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
make -C src/mbdraco && make -C src/mbmesh && make -C src/mbglb2tiles
```

`mbmesh` and `mbglb2tiles` link the Draco library of `src/mbdraco`, which is built first (a plain `make` does it in order). `make check` in each directory runs the unit tests.

## Poisson solvers

The Poisson step solves one linear equation per cell of a regular grid: the cell's value times its diagonal equals the sum of its six neighbors plus a right-hand side (the divergence of the splatted normals, plus the screening term near samples). `--poisson-solver` chooses how:

- **`jacobi`** (the previous default, kept for comparison) computes every cell from the previous sweep's neighbors. It needs a second grid and, because information crosses the grid by one cell a sweep, a number of sweeps that grows with the square of the grid width to converge. At the fine grids of a detailed survey (a hundred cells or more across) the 200 to 500 iterations that are used by default stop long before that.
- **`sor`** (the default) updates the cells in place in a red-black order (first the cells whose x + y + z is even, then the odd ones, so the neighbors of a cell all have the other color and the result does not depend on the order within a color) and over-relaxes each update by a factor between 1 and 2. The factor is the best one for the plain Laplacian on the grid, 2 / (1 + sqrt(1 - r^2)) with r the average of cos(pi / n) over the three directions, kept below 1.98; the screening term only strengthens the diagonal, so it still converges. The number of sweeps then grows with the grid width instead of its square. It uses one grid less memory than `jacobi`.

The relative residual that is logged (see "Log Messages") shows how far either got. On three chunks of a test survey at a 0.025 m cell (grids of 2.6 to 3.5 million cells), with the time of the solver iterations on one chunk:

| Solver and iterations | Residual at the end | Solve time |
|---|---|---|
| `jacobi`, 473 (its default) | 1.5e-2 to 2.5e-2 | 66 s |
| `sor`, 40 | 5e-1 to 7e-1 | 5 s |
| `sor`, 100 | 3e-2 to 5e-2 | 13 s |
| `sor`, 473 | 1.4e-9 to 6.4e-9 | 60 s |

so `sor` reaches the residual that `jacobi` reaches in 473 iterations in about 100 to 120, and converges completely if it is given more. Without `--poisson-iterations` it therefore takes one iteration per cell across the longest edge of the grid, at least 50 (145, 186 and 196 for those three chunks), which took the residual to 9e-4 to 4e-3, 6 to 25 times lower than `jacobi` reached, in 86 s of Poisson time in all against 241 s, 2.8 times less. The count follows the grid because the iterations that a relaxation needs grow with the width of the grid. A chunk of a different size, or a different cell size, gets its own count, and `--poisson-iterations` sets one count for all. The meshes differ a little because `jacobi` has not converged: the vertex counts of the converged `sor` meshes (473 iterations) are 1834, 9072 and 27152, against 1835, 9072 and 27154 from `sor` with its own count, and 1846, 9096 and 27452 from `jacobi`.

## Log Messages

`mbmesh` logs in the same line format as `mbgrd2gltf`: the level, the local date and time to the millisecond, the source file, function and line of the call, then the message:

```text
INFO 2026-10-19 14:35:15.419 mbmesh.cpp main():151 [startup] input=datalist.mb-1 output=mbmesh_output
DEBUG 2026-10-19 14:35:15.886 reconstruction.cpp reconstruct_chunked():290 [chunking] chunk_size=6.05 m, halo=0.72 m, 6 chunks contain points
```

A normal run logs how long reading the input and the whole reconstruction took, and the total (`[input] completed ... in 1.4 s, peak memory 0.5 GB`, `[reconstruction] completed in 1 min 7.5 s on 4 threads, peak memory 0.8 GB`, `[complete] ..., total 1 min 9.0 s, peak memory 0.8 GB`). The peak memory is the largest amount of physical memory the process has had in use at one time.

The long wait in between is reported at `INFO` level, so that a run without `-V` shows its progress, but sparingly. When chunking, at most one short line a minute says how many chunks are finished, the percentage, the elapsed time and an estimate of the time left, and a last line gives the time spent in each stage added up over all the chunks and threads, which shows which stage the run waits for. The chunks run on several threads and the largest start first, so the estimate is pessimistic early in the run:

```text
INFO ... [chunking]  40 of 624 chunks,  6%,   3.2 min elapsed, about  46.6 min left, peak memory 2.5 GB
INFO ... [chunking]  52 of 624 chunks,  8%,   4.4 min elapsed, about  48.1 min left, peak memory 2.5 GB
INFO ... [chunking] time summed over all chunks and threads: normals 7 min 2.0 s, poisson 112 min 1.0 s, marching cubes 2 min 4.1 s, trimming 6 min 51.9 s
```

Each Poisson solve also logs how its time divides and how far the solver got, with `-V` for each chunk and at `INFO` in a single pass:

```text
[poisson] chunk 3,-4: solver jacobi, 473 iterations on 2603620 cells: splat 0.5 s, solve 1 min 5.8 s; relative residual 1 -> 3.05e-02 (after 236) -> 1.47e-02 (after 473)
```

The splat is the time to spread the normals and samples onto the grid and form the right-hand side, and the solve is the iterations. The relative residual is the norm of what is left over in the grid equation divided by the norm of the right-hand side, so it is 1 at the start and falls toward 0 as the solver converges; the run only measures it twice, after half the iterations and at the end, which costs two extra sweeps. A residual that is still falling steeply at the end means that the iterations did not converge.

A single-pass run has no chunks, so it logs the completion of each stage instead (`[normal-estimation]`, `[poisson]`, `[marching-cubes]`, `[support-trimming]`). With `-V` each chunk also logs a line of its own, with its number, its time and the time in each of its stages, and the start of each stage is logged.

By default `INFO`, `WARN` and `ERROR` lines are shown. `-V` adds `DEBUG` lines, which carry the progress messages for each stage and, when chunking, each chunk (the chunk name follows the stage). Unlike `mbgrd2gltf`, every level is written to standard error, so standard output carries only data such as the `--info` report. Output is serialized, so lines from chunks reconstructed on different threads do not interleave. The logger is `include/util/logger.h`; it is a separate, header-only implementation of the `mbgrd2gltf` format rather than shared code.

## Development Notes

- If `mbmesh` on the shell resolves to `/usr/local/bin/mbmesh`, it may be an older installed binary. Use `./build/src/mbmesh/mbmesh` to run the current build-tree executable, or reinstall after building.
- The CMake target is `mbmesh`.
- Only the final mesh output is enabled by default. Optional diagnostic outputs must be requested explicitly.
- Metadata mode still reads the datalist and computes spacing, but it exits before normal estimation, screened Poisson, marching cubes, trimming, and output writing.

## Credits

- Initial implementation (PR #1584: reading swath datalists, projected and ECEF XYZ and GLB point clouds, the X3DOM viewer page, build, documentation and the `mbm_tmap` tiling macro): CSUMB Capstone, Spring 2026: Jorge Barrera-Simental, Kevin Crapo, Joceline Cortez-Arellano, Noah Mckegney, Lyn Larson, Metztli Herrera, Andre Gutierrez and David Martinez.
- Construction and architectural layout of the mesh reconstruction pipeline (datalist reader, normal estimation, screened Poisson reconstruction, marching cubes, support trimming and the output writers): CSUMB Capstone, Summer 2026, Daniel Karnofel and Benjamin Shimek.
- Continued refinement, Mike McCann (supervising Claude Sonnet 5.5, an AI coding agent from Anthropic), MBARI (issues #1559 and #1629): the ECEF frame and `--geoorigin`, chunked threaded reconstruction, Draco compression, OGC 3D Tiles tilesets with levels of detail for X3DOM (`mbglb2tiles`), the viewer and survey pages, and height colors.

## Copyright and License

These source files are copyright by David W. Caress, Dale N. Chayes, and the California State University Monterey Bay Capstone team. They are licensed using GPL3 as part of MB-System.
