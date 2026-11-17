# Source Directory: src/mbgrd2gltf/

This directory contains the source files for the program mbgrd2gltf, a 2D/3D grd to gltf conversion program.
The original gltf-generator program was created for STOQS by a CSUMB Capstone team in Fall 2021. This program was contributed to MB-System as part of the CSUMB Capstone in Spring 2023. Additional refinements and additions added as part of CSUMB Capstone in Spring 2024 (draco compression) and Fall 2025 (code refinement).

# Team Fall 2020

* Edward Montoya
* Sean Towne

# Team Fall 2021

* Isaac Hirzel
* Oliver Stringer
* Zachary Abbett

# Team Spring 2023

* Nicolas Porras Falconio
* Kyle Dowling
* Jesse Benavides
* Julian Fortin

# Team Spring 2024

* Ivan Martinez
* Steven Patrick
* Varad Poddar
* Aret Tinoco

# Team Fall 2025

* Miguel Gonzales
* Eric Rios
* Jonathan Ramirez-Fausto
* Tanmay Zende

# Georeference and provenance metadata

The glTF file records where the model is and how it was made in `asset.extras`, in the blocks `georeference` (coordinate reference system `EPSG:4978`, absolute or relative-to-center vertices, the `center` = GeoOrigin as ECEF meters, the same point as `geodeticCenter`, the `heightReference` and the `extent`), `provenance` (linked to W3C PROV-O through a JSON-LD `@context`: software and version, time of execution, command line, working directory, user, host and the input grid) and `MB-System` (grid size, spacing and registration, vertical exaggeration, vertical reference and, when the mesh is Draco-compressed, `content.compression`: the extension, bits and step, as `mbmesh` writes it). The `--html` page holds the same tree in X3D `GeoMetadata` and `WorldInfo` nodes. The code is in `provenance.h` and `provenance.cpp` and is tested by `tests/test_provenance.cpp`. The names are shared with `mbmesh` and are described in [../mbmesh/GEOREFERENCE.md](../mbmesh/GEOREFERENCE.md). The grid value is used as a height above the WGS 84 ellipsoid, with no geoid correction.

# Colors

Every vertex has a color (`COLOR_0`) from the Haxby table that `mbm_grdplot` uses by default: 11 colors from dark blue to white, changing linearly between 11 heights that are chosen from the range of the grid as `mbm_grdplot -G5` does. The height is the value of the grid, not the exaggerated one, so the colors mean the same whatever `--exaggeration` says. The table is the one of `mbmesh` and `mbglb2tiles` (`src/mbdraco/haxby_colors.h`). Colors are written with Draco as floats quantized to 10 bits, and without it as floats; they add about 15 per cent to the file. `--no-colors` leaves them out. Both pages have a "color by height" check box that shows the model in one gray instead.

The material of the model is not the default of glTF, which is fully metallic (`metallicFactor` 1) and rough: a viewer that draws physically based materials with no environment to reflect, as the build of X3DOM of October 2026 does, draws such a surface black. It has `metallicFactor` 0 and `roughnessFactor` 0.65, as the meshes of `mbmesh` do.

# The page for CesiumJS (--cesium-html)

`--cesium-html` writes `<output>_cesium.html` beside the model, a page for CesiumJS (release 1.114 from Cesium's web site, or the build that `--cesium-url` names). The page has a check box for the model and a link to the log, the Viewpoint menu (an overview looking down, views at 45 and 15 degrees from the south, and the `--geoviewpoint` viewpoints, with a Copy viewpoint button that gives the option for the view on the screen), and the scale axes. The globe of Cesium would hide data below the ellipsoid, so the page starts with it hidden and has a check box that shows it, translucent. The model is placed by the translation of the GeoOrigin, and the axes of glTF are left as they are, because the vertices are ECEF. The mesh has no normals, which CesiumJS draws unlit, so the page shades it from the relief of what it sees, with the colors by height (a "color by height" check box shows it in gray). With `--exaggeration` the label of the scale axes says what the Up axis stands for. The page is written by `src/mbmesh/src/io/cesium_writer.cpp`, which `mbmesh` and `mbglb2tiles` use too. Like the `--html` page it has to be served over HTTP.

# Viewpoints in the --html page

The page has two overview viewpoints for the same view, looking straight down from above the middle of the grid with north at the top: `OVERVIEW_LOCAL`, a plain `Viewpoint` in the coordinates of the model (ECEF minus the GeoOrigin's ECEF position), which is bound when the page opens, and `OVERVIEW_GEO`, a `GeoViewpoint` that refers to the GeoOrigin. Choose between them in the Viewpoint menu, or step through them with Page Down. The GeoOrigin is written as a child node of the `GeoViewpoint`, because `geoOrigin` is an SFNode field and X3DOM takes it from a child element: it ignores an attribute naming a DEF, and then places the `GeoViewpoint` in absolute ECEF, millions of meters from a model that is relative to the GeoOrigin. Both viewpoints are written so that each can be checked against the other. A GeoViewpoint of your own can be added with `--geoviewpoint=lon,lat,elev,clon,clat,celev` (the camera position and the point it looks at) or `--geoviewpoint=lon,lat,elev,ax,ay,az,angle,clon,clat,celev` (with the orientation as an axis and an angle, as a `GeoViewpoint` takes it, so that no rotation looks level to the north and `1 0 0 -1.5708` looks straight down), which can be repeated and gives `USER_GEOVIEW_1`, `USER_GEOVIEW_2` and so on, written as readable `GeoViewpoint` nodes with the position as latitude, longitude and elevation and the GeoOrigin used again (`<GeoOrigin USE='GRID_ORIGIN'>`). The values are geodetic, in the order of `--geoorigin` (longitude first; the node has latitude first, as X3D does), so they do not depend on the GeoOrigin or the data. The **Copy GeoViewpoint** button of the page puts the option for the view on the screen on the clipboard (the ten-number form) and in the X3DOM log; paste it into the command line of the next run.

The page also has a scale object: axes East (red), North (green) and Up (blue) at the center of rotation, which the page resizes whenever the viewpoint changes so that they stay about a sixth of the height of the view, with their length in the label, rounded down to 1, 2 or 5 times a power of ten ("5 m", "20 cm", "2 km"). The length is true at the axes, and the perspective makes other distances differ. With `--exaggeration` the Up axis is that many times as long as the meters it stands for, and the label says both ("20 m (up 10 m)"). The "scale axes" check box switches the object off. The code is `scale_axes.h`, which `mbmesh` and `mbglb2tiles` use for their pages as well.

# Dependencies

* netCDF
* TinyGLTF
	* This is in the project files as a single-header library. No futher action is needed for this dependency.

# GitHub Issues Addressed

## Fall 2020 Team

* [STOQS Issue #1093](https://github.com/stoqs/stoqs/issues/1093) - Initial exploration of automating the 3D geometry generation pipeline

## Fall 2021 Team
* [STOQS Issue #1093](https://github.com/stoqs/stoqs/issues/1093) - Initial gltf-generator implementation to replace 3D geometry generation pipeline

## Spring 2023 Team  
* [MB-System Issue #1257](https://github.com/dwcaress/MB-System/issues/1257) - Initial MB-System integration

## Spring 2024 Team
* [MB-System Issue #1456](https://github.com/dwcaress/MB-System/issues/1456) - Draco mesh compression support

## Fall 2025 Team
* [MB-System Issue #1521](https://github.com/dwcaress/MB-System/issues/1521) - General mbgrd2gltf C++ code refinement

## 2026: continued refinement
Mike McCann (supervising Claude Sonnet 5.5, an AI coding agent from Anthropic), MBARI:
* [MB-System Issue #1629](https://github.com/dwcaress/MB-System/issues/1629) - The GeoOrigin that keeps the vertices precise, the scale axes and the colors by height of the pages, the page for CesiumJS, and the material of the model (a sub-issue of [#1559](https://github.com/dwcaress/MB-System/issues/1559))

# Code Formatting

* This directory uses its own .clang-format file to maintain a consistent coding style for the mbgrd2gltf program.
* To apply the formatting rules to this code, run the following script from within this directory:
	* ./clang-format.sh
* It is recommended to run before committing changes to keep the codebase consistent for future contributors.

