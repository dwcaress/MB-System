# MB-Mesh: 3D Mesh Generation from Bathymetry Data

## Overview

`mb-mesh` is a utility program that generates 3D GLTF mesh files directly from bathymetry data, complementing the MB-System suite. While `mbgrid` creates 2D gridded maps from swath sonar data, `mb-mesh` produces optimized 3D mesh representations suitable for visualization, analysis, and web-based applications.

## Purpose

This tool bridges the gap between raw bathymetry data and 3D visualization by:
- Generating 3D meshes directly from bathymetry point data
- Creating GLTF 2.0 files for modern 3D visualization pipelines
- Providing mesh optimization through decimation and compression
- Supporting vertical exaggeration for better feature visualization
- Enabling web-based seafloor visualization workflows

## Relationship to MB-System Tools

```
┌─────────────────┐
│ Swath Sonar Data│
└────────┬────────┘
         │
    ┌────┴────────────────────┐
    │                         │
┌───▼──────┐          ┌──────▼─────┐
│ mbgrid   │          │  mb-mesh   │
│ (2D Map) │          │ (3D Mesh)  │
└───┬──────┘          └──────┬─────┘
    │                        │
┌───▼──────────┐      ┌──────▼──────┐
│   .grd file  │      │ .gltf file  │
│ (NetCDF Grid)│      │ (3D Model)  │
└───┬──────────┘      └──────┬──────┘
    │                        │
┌───▼──────────┐      ┌──────▼──────┐
│ mbgrd2gltf   │      │   Direct    │
│ (Conversion) │      │ Rendering   │
└───┬──────────┘      └─────────────┘
    │
┌───▼──────────┐
│ .gltf file   │
└──────────────┘
```

## Features

- **Direct Mesh Generation**: Creates 3D meshes without intermediate grid files
- **GLTF 2.0 Format**: Industry-standard 3D format supported by modern viewers
- **Mesh Optimization**: Configurable decimation and triangle limits
- **Vertical Exaggeration**: Adjustable scaling for better feature visibility
- **Regular Grid Generation**: Creates evenly-spaced mesh vertices
- **Normal Computation**: Automatic calculation of vertex normals for smooth shading
- **Inverse Distance Weighting**: Intelligent interpolation between bathymetry points

## Dependencies

- C++17 compiler
- CMake 3.10 or higher
- Standard C++ libraries

Optional:
- Draco (for mesh compression) - to be integrated in future versions

## Building

### With CMake (Recommended)

```bash
cd src/mb-mesh
mkdir build
cd build
cmake ..
make
```

### As Part of MB-System

The module integrates with the main MB-System build system:

```bash
cd /path/to/MB-System-Proto-Test/build
cmake ..
make mb-mesh
```

## Usage

### Basic Usage

```bash
mb-mesh -i input_bathymetry.txt -o output_mesh.gltf
```

### With Options

```bash
mb-mesh -i bathymetry.txt -o mesh.gltf -s 2.0 -e 3.0 -d 2 -v
```

### Command Line Options

#### Required

- `-i, --input <file>` - Input bathymetry data file (lon, lat, depth format)
- `-o, --output <file>` - Output GLTF file path

#### Optional

- `-h, --help` - Display help message
- `-v, --verbose` - Enable verbose output
- `-s, --spacing <value>` - Grid spacing in meters (default: 1.0)
- `-e, --exaggeration <value>` - Vertical exaggeration factor (default: 1.0)
- `-d, --decimation <level>` - Mesh decimation level 0-10 (default: 0)
- `-c, --draco` - Enable Draco compression (future feature)
- `-m, --max-triangles <n>` - Maximum number of triangles (default: 1000000)
- `-t, --edge-threshold <val>` - Edge collapse threshold (default: 0.001)

## Input Data Format

Input files should contain bathymetry data in space/tab-separated format:

```
lon lat depth
-122.345 36.789 -1234.5
-122.344 36.790 -1235.2
-122.343 36.791 -1233.8
...
```

- **lon**: Longitude in decimal degrees
- **lat**: Latitude in decimal degrees  
- **depth**: Depth in meters (negative values for below sea level)

Optional header lines starting with column names are automatically skipped.

## Output Format

The tool generates GLTF 2.0 files containing:
- Vertex positions (lon/lat/depth coordinates)
- Vertex normals (for smooth shading)
- Triangle indices (mesh connectivity)
- Metadata (bounds, count information)

Output files can be:
- Viewed in 3D viewers (Blender, MeshLab, online GLTF viewers)
- Used in web applications (Three.js, Babylon.js)
- Imported into GIS software
- Processed with 3D analysis tools

## Examples

### Generate Basic Mesh

```bash
mb-mesh -i seafloor_data.txt -o seafloor.gltf
```

### High-Resolution Mesh with Vertical Exaggeration

```bash
mb-mesh -i canyon.txt -o canyon.gltf -s 0.5 -e 5.0 -v
```

### Optimized Mesh for Web Viewing

```bash
mb-mesh -i survey.txt -o survey_web.gltf -s 3.0 -d 3 -m 50000
```

## Testing

The module includes comprehensive tests with simulated bathymetry data:

```bash
cd test/mb-mesh
python3 test_data_generator.py
python3 mb-mesh_test.py
```

See the test directory README for detailed testing information.

## Algorithm Details

### Mesh Generation Pipeline

1. **Data Loading**: Read bathymetry points from input file
2. **Regular Grid Creation**: Generate evenly-spaced vertices
3. **Depth Interpolation**: Use inverse distance weighting for vertex depths
4. **Triangulation**: Create triangle mesh from regular grid
5. **Decimation** (optional): Reduce triangle count while preserving shape
6. **Normal Computation**: Calculate vertex normals for smooth shading
7. **GLTF Export**: Write optimized GLTF 2.0 file

### Interpolation Method

Uses inverse distance weighting with power=2:

```
depth = Σ(w_i * depth_i) / Σ(w_i)
where w_i = 1 / distance_i²
```

## Future Enhancements

- [ ] Draco mesh compression support
- [ ] Support for MB-System native data formats
- [ ] Adaptive mesh refinement in high-detail areas
- [ ] Texture mapping support
- [ ] Multiple level-of-detail (LOD) generation
- [ ] Binary GLTF (.glb) output
- [ ] Integration with mbgrid for grid-based input

## Contributing

Contributions are welcome! Please follow the MB-System coding standards:
- Use `.clang-format` for code formatting
- Add tests for new features
- Update documentation

## Authors

Created: February 6, 2026

Part of the MB-System project:
- David W. Caress (caress@mbari.org) - Monterey Bay Aquarium Research Institute
- Dale N. Chayes - University of New Hampshire
- Christian dos Santos Ferreira - University of Bremen

## License

See MB-System [COPYING.md](../../COPYING.md) for license information.

## References

- [MB-System](https://www.mbari.org/products/research-software/mb-system/)
- [GLTF 2.0 Specification](https://registry.khronos.org/glTF/specs/2.0/glTF-2.0.html)
- [mbgrid documentation](../utilities/mbgrid.cc)
- [mbgrd2gltf](../mbgrd2gltf/README.md)

## Support

For questions or issues:
- Check MB-System documentation
- File issues on GitHub
- Contact MB-System developers
