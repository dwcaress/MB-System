## Overview

**mbmesh** reads swath sonar data files via MB-System's datalist processing framework and extracts multibeam echosounder (MBES) soundings. The extracted data is output in multiple formats suitable for 3D visualization, analysis, and web-based applications:

- **Local projected XYZ point cloud** - Coordinates in meters relative to data centroid
- **ECEF XYZ point cloud** - WGS84 Earth-Centered Earth-Fixed coordinates
- **GLB (glTF) format** - Binary 3D model format for web and desktop viewers
- **HTML viewer** - Interactive web-based visualization (optional)

## Development History

This tool was created as a CSUMB (California State University, Monterey Bay) Capstone project for Spring 2026. As of May 2026, it is capable of processing swath sonar datalists into a point cloud that can be visualized in either CesiumJS or X3DOM. The plan is to have X3DOM as the main way to visualize, but it currently does not have the capability to have parent/child tiles.

## Current Implementation

**mbmesh** implements a complete pipeline for processing swath sonar data into 3D point clouds:

1. **Data Ingestion**: Read datalist files and all referenced swath data files using MB-System API
2. **Beam Extraction**: Extract individual bathymetry soundings from multibeam pings
3. **Quality Filtering**: Automatically filter out flagged/invalid beams
4. **Geographic Filtering**: Optional bounding box filtering via -R option
5. **Output Generation**: Write point clouds in multiple formats (XYZ, ECEF, GLB)
6. **Visualization**: Optional HTML viewer with local web server launch

### Current Features

- Support for all MB-System formats via datalist processing
- Automatic beam quality filtering using MB-System flags
- Geographic bounds filtering (west/east/south/north)
- Local projected coordinate system for improved numerical stability in 3D viewers
- WGS84 ECEF coordinate output with GeoOrigin offset for geodetic precision
- GLB binary format export via TinyGLTF library
- Optional HTML viewer with automatic local web server launch
- Comprehensive statistics and progress reporting
- Verbose mode (-V -V) for detailed debugging and sample data display

## Future Development

Planned enhancements for future versions:

- **Mesh Generation**: Create triangle meshes from point clouds
- **py3dtiles Integration**: Evaluate py3dtiles library and alternatives for OGC 3D Tiles format support
- **Draco Compression**: Implement Draco compression for efficient GLB file transmission
- **X3DOM Visualization**: X3DOM viewer as default visualization option
- **GeoOrigin Refinement**: Improved GeoOrigin handling and direct ECEF coordinate output without offset

## Source Files

### Main Application

- **mbmesh.cpp** - Main application file (currently ~1200 lines) containing:
  - Data structures (Sounding struct for individual beam data)
  - Command-line argument parsing (-I, -O, -R, -html, -V, -H)
  - Datalist and swath file reading using MB-System libmbio API
  - Ping processing and beam extraction with quality flagging
  - Point cloud file writing in local projected coordinates (adjustedPointcloud.xyz)
  - ECEF coordinate conversion and output (ecefPointcloud.xyz)
  - GLB export via pointcloud_glb_writer for 3D model generation
  - HTML viewer generation with X3DOM support
  - Comprehensive statistics and progress reporting

### Point Cloud Export

- **pointcloud_glb_writer.cpp** - GLB format export
  - Converts XYZ point clouds to glTF Binary format
  - Creates valid 3D models readable by standard viewers
  
- **pointcloud_glb_writer.h** - Header file for GLB writer

### Build Configuration

- **CMakeLists.txt** - CMake build configuration
- **Makefile.am** - Autotools build configuration

### Data Examples

- **tiles/** - Example tile data
- **tiles_ex2/** - Additional example tile data

## Building

### CMake (Recommended)

```bash
cd /path/to/MB-System/build
cmake ..
make mbmesh
```

### Autotools

```bash
cd /path/to/MB-System
./configure
make mbmesh
```

## Usage Examples

### Basic Usage

```bash
mbmesh -Idatalist.mb-1
```

### Geographic Filtering

```bash
mbmesh -Idatalist.mb-1 -R-122.5/-121.8/36.5/37.2
```

### Verbose Output

```bash
mbmesh -Idatalist.mb-1 -V -V
```

### With HTML Viewer

```bash
mbmesh -Idatalist.mb-1 -html -O./output_dir
```

### Custom Output Directory

```bash
mbmesh -Idatalist.mb-1 -O./my_output
```

## Output Files

Upon successful execution, mbmesh generates:

1. **adjustedPointcloud.xyz** - XYZ point cloud in local projected meters
   - X and Y relative to data centroid
   - Z (depth) centered around mean
   - Suitable for standard 3D viewers

2. **ecefPointcloud.xyz** - XYZ point cloud in ECEF coordinates
   - WGS84 reference frame
   - GeoOrigin offset applied
   - Preserves geodetic accuracy

3. **adjustedPointcloud.glb** - Binary glTF model
   - Can be viewed in web browsers (Cesium.js, Three.js, BabylonJS)
   - Suitable for web-based visualization platforms
   - Compatible with desktop 3D viewers

4. **adjustedPointcloud.html** (optional, requires -html flag)
   - Interactive web-based viewer
   - Uses X3DOM or similar rendering library
   - Automatically opened in default browser if server launches

## Statistics and Output

**mbmesh** prints detailed statistics after processing:

```
========================================
    Swath Data Reading Statistics
========================================
Files in datalist:       N
Files successfully read: N
Pings processed:         N
Beams total:             N
Beams good:              N
Beams flagged:           N
Soundings stored:        N
Acceptance rate:         X.X%
```

## Command-Line Options

### Required

- `-I<datalist>` - Input datalist file (MB-System format)

### Optional

- `-O<outputdir>` - Output directory (default: ./tileset)
- `-R<w/e/s/n>` - Geographic bounds in degrees
- `-html` - Generate HTML viewer and launch server
- `-V` - Increase verbosity (can repeat: -V -V)
- `-H` - Display help message

## Copyright and License

These source files are copyright by David W. Caress, Dale N. Chayes, and the 
California State University Monterey Bay Capstone team. They are licensed using
GPL3 as part of MB-System.

Initial implementation: Spring 2026 CSUMB Capstone Project