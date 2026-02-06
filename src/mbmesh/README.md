# mbmesh - 3D Mesh Generation from Swath Data

## Overview

`mbmesh` is a utility for creating 3D mesh representations directly from swath sonar data. Unlike traditional 2D gridding approaches used by programs like `mbgrid`, `mbmesh` bypasses the gridding operation to enable accurate representation of complex bathymetric features including:

- Cliffs and steep slopes
- Underwater spires and pinnacles
- Overhangs
- Caves and complex structures

## Output Format

The program outputs to **OGC 3D Tiles 1.1** format, whose fundamental tile format is `.glb` (glTF binary). The initial implementation creates a single root 3D Tile that can be visualized in web browsers using X3DOM or other glTF-capable viewers.

## Current Status (February 2026)

### ✅ Implemented

- **Swath data reading**: Program can read datalist files and process multiple swath sonar files
- **MB-System integration**: Full integration with MB-System MBIO library for format support
- **Command-line interface**: Basic option parsing for input/output files and bounds
- **Data collection**: Collects and stores bathymetric points from swath data
- **Statistics**: Computes and reports data bounds and counts

### 🚧 To Be Implemented

- **3D mesh generation**: Convert collected swath points into 3D mesh geometry
- **Tile generation**: Create OGC 3D Tiles 1.1 structure
- **glTF output**: Write Draco-compressed glTF/glb files
- **Spatial organization**: Implement spatial indexing and tiling strategy
- **Level of detail**: Support multiple LOD levels
- **Texture mapping**: Optional texture or color mapping
- **Web integration**: Full X3DOM compatibility

## Usage

```bash
# Basic usage
mbmesh -I datalist.mb-1 -O output_root

# With geographic bounds
mbmesh -I datalist.mb-1 -O output_root -R-122.5/-122.0/36.5/37.0

# Verbose output
mbmesh -I datalist.mb-1 -O output_root -V
```

### Options

- `-I` : Input datalist file containing swath data files
- `-O` : Output filename root (will create .glb or .gltf file)
- `-R` : Geographic bounds: west/east/south/north
- `-V` : Verbose output (use -VV for very verbose)
- `-H` : Display help message

## Development Roadmap

### Milestone 1: Single Root Tile (Current)
- ✅ Read swath data from datalist
- ⬜ Generate basic 3D mesh
- ⬜ Write single glTF tile
- ⬜ Test with X3DOM viewer

### Milestone 2: Basic 3D Tiles
- ⬜ Implement tile hierarchy
- ⬜ Add geometric error calculations
- ⬜ Create tileset.json structure
- ⬜ Support multiple LOD levels

### Milestone 3: Optimization
- ⬜ Add Draco compression
- ⬜ Implement spatial indexing
- ⬜ Optimize mesh decimation
- ⬜ Memory management for large datasets

### Milestone 4: Enhanced Features
- ⬜ Texture/color support
- ⬜ Multiple data types (amplitude, backscatter)
- ⬜ Custom styling options
- ⬜ Advanced filtering

## Resources

- [OGC 3D Tiles 1.1 Specification](https://docs.ogc.org/cs/22-025r4/22-025r4.html)
- [Cesium 3D Tiles](https://github.com/CesiumGS/3d-tiles)
- [X3DOM 3D Tiles Development](https://doc.x3dom.org/)
- [glTF 2.0 Specification](https://registry.khronos.org/glTF/specs/2.0/glTF-2.0.html)
- [Draco Compression](https://google.github.io/draco/)

## Related MB-System Programs

- **mbgrid**: Creates 2D grids from swath data (traditional approach)
- **mbgrd2gltf**: Converts 2D bathymetry grids to glTF format
- **mbmosaic**: Creates backscatter mosaics from swath data

## Building

`mbmesh` is built as part of the standard MB-System build process:

```bash
cd build
cmake ..
make mbmesh
# or
make -j8  # build all
```

## Author

MB-System Development Team
- David W. Caress (caress@mbari.org)
- Dale N. Chayes
- And contributors

## License

See MB-System COPYING.md for redistribution conditions.
