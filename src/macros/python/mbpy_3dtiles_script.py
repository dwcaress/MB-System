#!/usr/bin/env python3
#--------------------------------------------------------------------
#    The MB-system:  mbpy_3dtiles_script.py
#
#    Copyright (c) 2026 by
#    David W. Caress (caress@mbari.org)
#      Monterey Bay Aquarium Research Institute
#      Moss Landing, California, USA
#
#    See README.md file for copying and redistribution conditions.
#--------------------------------------------------------------------
#
# Purpose:
#   Script generator of mbpy_3dtiles: the commands that make the standard
#   3D products of a survey with mbgrd2gltf, mbmesh and mbglb2tiles, and the
#   shellscript that holds them. The survey directory has one subdirectory
#   for each kind of data (multibeam, lidar), each with a grid, ZTopo.grd, and
#   a datalist of the processed swath data, datalistp.mb-1:
#
#     1. mbgrd2gltf: the grid of each subdirectory as a GLB and a page.
#     2. mbmesh: the swath data of each subdirectory as a mesh (mesh.glb,
#        raw_mesh.glb, pointcloud.glb, track.glb) and a page.
#     3. mbglb2tiles: the GLBs of 1 and 2 as OGC 3D Tiles tilesets of levels
#        of detail, for the GeoTileset of X3DOM.
#     4. mbmesh on the survey directory: the survey page that offers all of it.
#
#   Each of these programs also writes a page for CesiumJS (--cesium-html), beside the page for X3DOM.
#
#   All of the programs get the same GeoOrigin, so that their models line up,
#   and the tilesets get the same lift, so that they stay with each other and
#   with the models that are not tilesets.
#
#   This module only builds the commands, with the exception of the optional
#   look at the grids with gmt grdinfo to set the GeoOrigin and the lift;
#   it does not run any of the programs.
#
import os
import re
import shlex
import subprocess
from dataclasses import dataclass, field
from datetime import datetime
from math import ceil
from typing import List, Optional, Sequence, Tuple

# The clearance above the ellipsoid that mbmesh and mbglb2tiles give the lowest point of a mesh with --lift and no value: X3DOM's GeoTileset
# does not refine tiles for a camera below the ellipsoid.
LIFT_CLEARANCE_METERS = 10.0

DEFAULT_SUBDIRS = ("multibeam", "lidar")

# The subdirectory whose grid sets the GeoOrigin and the lift of every product of the survey: there is one GeoOrigin for a survey, so that all of its models line up.
DEFAULT_REFERENCE_SUBDIR = "multibeam"


@dataclass
class GridExtent:
    west: float
    east: float
    south: float
    north: float
    z_min: float
    z_max: float
    z_mean: float


@dataclass
class Command:
    program: str
    args: List[str]
    comment: str = ""

    def line(self) -> str:
        return " ".join(shlex.quote(part) for part in [self.program] + self.args)


@dataclass
class TilesOptions:
    survey_directory: str = "."
    subdirectories: Sequence[str] = DEFAULT_SUBDIRS
    grid_name: str = "ZTopo.grd"
    datalist_name: str = "datalistp.mb-1"
    geoorigin: Optional[Tuple[float, float, float]] = None   # longitude, latitude, height above the ellipsoid; the same for every program
    lift_meters: Optional[float] = None                       # for every tileset
    lod: Optional[float] = 0.1                                # mbmesh -L, meters
    threads: Optional[int] = 8
    label_chunks: bool = True
    x3dom_url: Optional[str] = "./x3dom-full.js"
    extra_mbgrd2gltf: List[str] = field(default_factory=list)
    extra_mbmesh: List[str] = field(default_factory=list)
    extra_mbglb2tiles: List[str] = field(default_factory=list)
    survey_page: bool = True
    cesium_html: bool = True                                  # --cesium-html for every program
    cesium_url: Optional[str] = None                          # --cesium-url for every program; None leaves each program's default


def format_number(value: float, decimals: int) -> str:
    text = f"{value:.{decimals}f}".rstrip("0").rstrip(".")
    return "0" if text in ("", "-0") else text


def geoorigin_option(geoorigin: Tuple[float, float, float]) -> str:
    """The value is attached with an equals sign: a longitude west of Greenwich begins with a minus."""
    lon, lat, height = geoorigin
    return f"--geoorigin={format_number(lon, 6)},{format_number(lat, 6)},{format_number(height, 3)}"


def lift_option(lift_meters: float) -> List[str]:
    return ["--lift", format_number(lift_meters, 3)]


def parse_grdinfo(text: str) -> GridExtent:
    """The output of gmt grdinfo -C -L2: name, x_min, x_max, y_min, y_max, z_min, z_max, dx, dy, nx, ny, mean, stdev, rms."""
    for line in text.splitlines():
        fields = line.split()
        if len(fields) >= 12:
            try:
                numbers = [float(value) for value in fields[1:12]]
            except ValueError:
                continue
            return GridExtent(west=numbers[0], east=numbers[1], south=numbers[2], north=numbers[3],
                              z_min=numbers[4], z_max=numbers[5], z_mean=numbers[10])
    raise ValueError("no grid information in the output of gmt grdinfo")


def read_grid_extent(path: str) -> GridExtent:
    try:
        proc = subprocess.run(["gmt", "grdinfo", "-C", "-L2", path], capture_output=True, text=True, check=True)
    except FileNotFoundError as e:
        raise RuntimeError("gmt is needed to read " + path + " for the GeoOrigin and the lift: give them with --geoorigin and --lift") from e
    except subprocess.CalledProcessError as e:
        raise RuntimeError("gmt grdinfo failed for " + path + ":\n" + e.stderr) from e
    return parse_grdinfo(proc.stdout)


def geoorigin_of(extents: Sequence[GridExtent]) -> Tuple[float, float, float]:
    """The center of the area that the grids cover together and the mean of their mean heights, so that the GeoOrigin is in the middle of
    all of the data (for one grid it is what mbgrd2gltf takes for --geoorigin with no value)."""
    west, east = min(e.west for e in extents), max(e.east for e in extents)
    south, north = min(e.south for e in extents), max(e.north for e in extents)
    return ((west + east) / 2.0, (south + north) / 2.0, sum(e.z_mean for e in extents) / len(extents))


def lift_of(extents: Sequence[GridExtent]) -> float:
    """Enough to put the lowest point of the grids LIFT_CLEARANCE_METERS above the ellipsoid, to a meter, as mbglb2tiles does with --lift."""
    lowest = min(extent.z_min for extent in extents)
    return float(max(0, ceil(LIFT_CLEARANCE_METERS - lowest)))


def existing_subdirectories(options: TilesOptions) -> List[str]:
    """The subdirectories that are there and have something to make a product from."""
    found = []
    for name in options.subdirectories:
        directory = os.path.join(options.survey_directory, name)
        if os.path.isfile(os.path.join(directory, options.grid_name)) or os.path.isfile(os.path.join(directory, options.datalist_name)):
            found.append(name)
    return found


def missing_inputs(options: TilesOptions) -> List[str]:
    """Warnings for the standard inputs that a subdirectory that is there does not have. ZTopo.grd is a standard product of a survey, which this macro does
    not make: without it there is no ZTopo.glb or tileset of the grid, and without the datalist there is no mesh."""
    warnings = []
    for name in existing_subdirectories(options):
        directory = os.path.join(options.survey_directory, name)
        if not os.path.isfile(os.path.join(directory, options.grid_name)):
            warnings.append(f"{os.path.join(directory, options.grid_name)} not found: no {os.path.splitext(options.grid_name)[0]}.glb or tileset of the grid for {name}")
        if not os.path.isfile(os.path.join(directory, options.datalist_name)):
            warnings.append(f"{os.path.join(directory, options.datalist_name)} not found: no mesh, point cloud or track for {name}")
    return warnings


def plan(options: TilesOptions) -> List[Command]:
    """The commands, in the order that they run, with paths relative to the survey directory."""
    if options.geoorigin is None:
        raise ValueError("the GeoOrigin is not set")
    geoorigin = geoorigin_option(options.geoorigin)
    grid_stem = os.path.splitext(options.grid_name)[0]
    subdirectories = existing_subdirectories(options)
    with_grid = [d for d in subdirectories if os.path.isfile(os.path.join(options.survey_directory, d, options.grid_name))]
    with_datalist = [d for d in subdirectories if os.path.isfile(os.path.join(options.survey_directory, d, options.datalist_name))]
    x3dom = ["--x3dom-url", options.x3dom_url] if options.x3dom_url else []
    lift = lift_option(options.lift_meters) if options.lift_meters is not None else ["--lift"]
    cesium = (["--cesium-html"] + (["--cesium-url", options.cesium_url] if options.cesium_url else [])) if options.cesium_html else []
    commands: List[Command] = []

    for d in with_grid:
        commands.append(Command("mbgrd2gltf", ["--input", f"{d}/{options.grid_name}", "--html"] + cesium + [geoorigin, "--output", f"{d}/{grid_stem}"] + options.extra_mbgrd2gltf,
                                f"The grid of {d} as a GLB and a page"))

    for d in with_datalist:
        args = ["--input", f"{d}/{options.datalist_name}", "--html"] + cesium
        if options.threads:
            args += ["--threads", str(options.threads)]
        args.append(geoorigin)
        if options.lod is not None:
            args += ["-L", format_number(options.lod, 6)]
        args += ["--raw-mesh-glb", "--pointcloud-glb", "--track-glb"]
        if options.label_chunks:
            args.append("--label-chunks")
        args += ["--output", f"{d}/mbmesh_output"] + options.extra_mbmesh
        commands.append(Command("mbmesh", args, f"The swath data of {d} as a mesh, a point cloud and a track, and a page"))

    for d in with_grid:
        commands.append(Command("mbglb2tiles", ["--input", f"{d}/{grid_stem}.glb", "--html"] + cesium + x3dom + lift + ["--output", f"{d}/{grid_stem}_tileset"] + options.extra_mbglb2tiles,
                                f"The grid of {d} as a tileset of levels of detail"))
    for d in with_datalist:
        commands.append(Command("mbglb2tiles", ["--input", f"{d}/mbmesh_output/mesh.glb", "--html"] + cesium + x3dom + lift + options.extra_mbglb2tiles,
                                f"The mesh of {d} as a tileset of levels of detail"))

    if options.survey_page and commands:
        commands.append(Command("mbmesh", ["--input", ".", "--html"] + cesium + [geoorigin] + x3dom, "The survey page, which offers all of it"))
    return commands


def script_text(options: TilesOptions, commands: Sequence[Command], command_line: str = "") -> str:
    out = ["#!/usr/bin/env bash",
           "#",
           "# Shellscript that makes the 3D products of a survey: GLBs, OGC 3D Tiles tilesets and X3DOM pages",
           "#",
           "# Created by macro mbpy_3dtiles",
           f"# Run on {datetime.now().astimezone().strftime('%a %b %d %H:%M:%S %Z %Y')}"]
    if command_line:
        out.append(f"# Command line: {command_line}")
    out += ["#",
            "set -euo pipefail",
            "",
            f"cd {shlex.quote(os.path.abspath(options.survey_directory))}",
            ""]
    for command in commands:
        out += [f"# {command.comment}", command.line(), ""]
    out += ["# All done! The pages and their logs need a web server: from this directory, run",
            "#   python3 -m http.server 8000",
            "# and open http://localhost:8000/mbmesh_survey.html (mbmesh_survey_cesium.html for CesiumJS)",
            "echo All done!",
            ""]
    return "\n".join(out)


def generate_and_write(options: TilesOptions, script_path: str, command_line: str = "") -> List[Command]:
    commands = plan(options)
    if not commands:
        raise ValueError("nothing to make: there is no " + options.grid_name + " or " + options.datalist_name + " in " +
                         ", ".join(os.path.join(options.survey_directory, d) for d in options.subdirectories))
    with open(script_path, "w") as f:
        f.write(script_text(options, commands, command_line))
    os.chmod(script_path, 0o775)
    return commands
