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
#   Each command is run only if it has something to do: if one of its outputs is missing or older than one of its inputs, by modification times, which the
#   shellscript checks when it runs, so an input made again by an earlier command makes what depends on it stale. --force makes everything again. The
#   inputs of mbmesh are the datalist and every swath file it names, and the inputs of the survey pages are the outputs of all of the other commands. A change
#   of an option is not seen by this: use --force.
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
    outputs: List[str] = field(default_factory=list)   # relative to the survey directory; the command is run if one is missing or older than an input
    inputs: List[str] = field(default_factory=list)

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
    force: bool = False                                       # make every product again, whatever the modification times say
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


def datalist_files(datalist: str, seen: Optional[set] = None) -> List[str]:
    """The datalist and every file that it names, recursively, as MB-System reads it: a line is a file, a format and a weight, and a format of -1 says that the file
    is a datalist. A datalist that says $PROCESSED uses the processed version of each swath file, with a p before the extension, where there is one: both are
    listed if they are there, since the processed one is made from the raw one. A file that is not there is left out."""
    seen = seen if seen is not None else set()
    path = os.path.normpath(datalist)
    if path in seen or not os.path.isfile(path):
        return []
    seen.add(path)
    found = [path]
    directory = os.path.dirname(path)
    try:
        with open(path) as f:
            lines = f.read().splitlines()
    except OSError:
        return found
    for line in lines:
        line = line.strip()
        if not line or line.startswith("#") or line.startswith("$"):
            continue
        fields = line.split()
        if len(fields) < 2:
            continue
        name = fields[0][2:] if fields[0].startswith("P:") else fields[0]
        member = name if os.path.isabs(name) else os.path.join(directory, name)
        try:
            fmt = int(float(fields[1]))
        except ValueError:
            continue
        if fmt == -1:
            found += datalist_files(member, seen)
        else:
            stem, extension = os.path.splitext(member)
            for candidate in (member, stem + "p" + extension):
                if os.path.isfile(candidate) and os.path.normpath(candidate) not in seen:
                    seen.add(os.path.normpath(candidate))
                    found.append(os.path.normpath(candidate))
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

    def pages(root: str) -> List[str]:
        """The pages that a program writes beside what it makes at root: the one of X3DOM, and the one of CesiumJS."""
        return [root + ".html"] + ([root + "_cesium.html"] if options.cesium_html else [])

    for d in with_grid:
        commands.append(Command("mbgrd2gltf", ["--input", f"{d}/{options.grid_name}", "--html"] + cesium + [geoorigin, "--output", f"{d}/{grid_stem}"] + options.extra_mbgrd2gltf,
                                f"The grid of {d} as a GLB and a page",
                                outputs=[f"{d}/{grid_stem}.glb"] + pages(f"{d}/{grid_stem}"), inputs=[f"{d}/{options.grid_name}"]))

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
        datalist = os.path.join(options.survey_directory, d, options.datalist_name)
        swath_inputs = [os.path.relpath(f, options.survey_directory) for f in datalist_files(datalist)] or [f"{d}/{options.datalist_name}"]
        commands.append(Command("mbmesh", args, f"The swath data of {d} as a mesh, a point cloud and a track, and a page",
                                outputs=[f"{d}/mbmesh_output/{name}" for name in ("mesh.glb", "raw_mesh.glb", "pointcloud.glb", "track.glb")] + pages(f"{d}/mbmesh_output"),
                                inputs=swath_inputs))

    for d in with_grid:
        commands.append(Command("mbglb2tiles", ["--input", f"{d}/{grid_stem}.glb", "--html"] + cesium + x3dom + lift + ["--output", f"{d}/{grid_stem}_tileset"] + options.extra_mbglb2tiles,
                                f"The grid of {d} as a tileset of levels of detail",
                                outputs=[f"{d}/{grid_stem}_tileset/tileset.json"] + pages(f"{d}/{grid_stem}_tileset"), inputs=[f"{d}/{grid_stem}.glb"]))
    for d in with_datalist:
        commands.append(Command("mbglb2tiles", ["--input", f"{d}/mbmesh_output/mesh.glb", "--html"] + cesium + x3dom + lift + options.extra_mbglb2tiles,
                                f"The mesh of {d} as a tileset of levels of detail",
                                outputs=[f"{d}/mbmesh_output/tileset/tileset.json"] + pages(f"{d}/mbmesh_output/tileset"), inputs=[f"{d}/mbmesh_output/mesh.glb"]))

    if options.survey_page and commands:
        # The survey page offers what the other commands make, so it is stale when any of that is newer than it.
        commands.append(Command("mbmesh", ["--input", ".", "--html"] + cesium + [geoorigin] + x3dom, "The survey page, which offers all of it",
                                outputs=pages("mbmesh_survey"), inputs=[c.outputs[0] for c in commands if c.outputs]))
    return commands


def freshness(options: TilesOptions, commands: Sequence[Command]) -> List[Tuple[bool, str]]:
    """For each command, whether the shellscript would run it now, and why: the same rule that the shellscript applies when it runs, with the modification times as
    they are now, and with what the earlier commands that would run write counted as new. With --force every command runs. This is what -V says; the shellscript
    looks at the files again each time that it is run."""
    survey = options.survey_directory
    results: List[Tuple[bool, str]] = []
    rewritten = set()   # the outputs that a command that would run makes again
    for command in commands:
        if options.force:
            reason = "--force"
        else:
            reason = ""
            if not command.outputs:
                reason = "no outputs to compare"
            for output in command.outputs:
                path = os.path.join(survey, output)
                if not os.path.exists(path):
                    reason = f"{output} is missing"
                    break
                made = os.path.getmtime(path)
                again = [i for i in command.inputs if i in rewritten]
                newer = [i for i in command.inputs if i not in rewritten and os.path.exists(os.path.join(survey, i)) and os.path.getmtime(os.path.join(survey, i)) > made]
                if again or newer:
                    first = again[0] if again else newer[0]
                    more = len(again) + len(newer) - 1
                    reason = (f"{first} is made again by an earlier command" if again else f"{first} is newer than {output}") + (f" (and {more} more inputs are)" if more else "")
                    break
        runs = bool(reason)
        if runs:
            rewritten.update(command.outputs)
        results.append((runs, reason if runs else "outputs are newer than inputs"))
    return results


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
            "",
            "# A command is run only if one of its outputs is missing or older than one of its inputs. --force of the macro, or FORCE=1 in the environment, runs all of them.",
            f"force=${{FORCE:-{1 if options.force else 0}}}",
            "stale() {   # stale OUTPUT... -- INPUT...",
            "  [ \"$force\" = 1 ] && return 0",
            "  local outputs=() output input",
            "  while [ \"$1\" != \"--\" ]; do outputs+=(\"$1\"); shift; done",
            "  shift",
            "  for output in \"${outputs[@]}\"; do",
            "    [ -e \"$output\" ] || return 0",
            "    for input in \"$@\"; do",
            "      [ \"$input\" -nt \"$output\" ] && return 0",
            "    done",
            "  done",
            "  return 1",
            "}",
            ""]
    for command in commands:
        out.append(f"# {command.comment}")
        if command.outputs:
            outputs = " ".join(shlex.quote(o) for o in command.outputs)
            if len(command.inputs) > 3:
                out += ["inputs=("] + [f"  {shlex.quote(i)}" for i in command.inputs] + [")"]
                check = f"stale {outputs} -- \"${{inputs[@]}}\""
            else:
                check = f"stale {outputs} -- " + " ".join(shlex.quote(i) for i in command.inputs)
            out += [f"if {check}; then", f"  {command.line()}", "else", f"  echo {shlex.quote('Up to date, not made again (--force makes it again): ' + command.outputs[0])}", "fi", ""]
        else:
            out += [command.line(), ""]
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
