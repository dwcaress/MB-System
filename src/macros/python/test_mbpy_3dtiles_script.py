#!/usr/bin/env python3
"""Check the commands of mbpy_3dtiles_script.py against the example commands of a LASS survey, and the helpers that set
the GeoOrigin and the lift. Needs no program of MB-System; the one check that reads a grid needs gmt and is left out without it."""
import os
import shutil
import time
import subprocess
import sys
import tempfile
from pathlib import Path

sys.path.insert(0, str(Path(__file__).parent))
import mbpy_3dtiles_script as script

ORIGIN = (-122.304, 36.3928, -842.845)
G = "--geoorigin=-122.304,36.3928,-842.845"
X = ["--x3dom-url", "./x3dom-full.js"]


def check(name, ok, detail=""):
    print(f"  [{'PASS' if ok else 'FAIL'}] {name}" + (f"  ({detail})" if detail and not ok else ""))
    return ok


def touch(path: Path):
    path.parent.mkdir(parents=True, exist_ok=True)
    path.touch()


def main():
    ok = True
    with tempfile.TemporaryDirectory() as tmp:
        survey = Path(tmp) / "survey"
        for d in ("multibeam", "lidar"):
            touch(survey / d / "ZTopo.grd")
            touch(survey / d / "datalistp.mb-1")

        print("\n=== both subdirectories ===")
        options = script.TilesOptions(survey_directory=str(survey), geoorigin=ORIGIN, lift_meters=856.0)
        lines = [c.line() for c in script.plan(options)]
        mbmesh = lambda d: f"mbmesh --input {d}/datalistp.mb-1 --html --cesium-html --threads 8 {G} -L 0.1 --raw-mesh-glb --pointcloud-glb --track-glb --label-chunks --output {d}/mbmesh_output"
        expected = [
            f"mbgrd2gltf --input multibeam/ZTopo.grd --html --cesium-html {G} --output multibeam/ZTopo",
            f"mbgrd2gltf --input lidar/ZTopo.grd --html --cesium-html {G} --output lidar/ZTopo",
            mbmesh("multibeam"), mbmesh("lidar"),
            "mbglb2tiles --input multibeam/ZTopo.glb --html --cesium-html --x3dom-url ./x3dom-full.js --lift 856 --output multibeam/ZTopo_tileset",
            "mbglb2tiles --input lidar/ZTopo.glb --html --cesium-html --x3dom-url ./x3dom-full.js --lift 856 --output lidar/ZTopo_tileset",
            "mbglb2tiles --input multibeam/mbmesh_output/mesh.glb --html --cesium-html --x3dom-url ./x3dom-full.js --lift 856",
            "mbglb2tiles --input lidar/mbmesh_output/mesh.glb --html --cesium-html --x3dom-url ./x3dom-full.js --lift 856",
            f"mbmesh --input . --html --cesium-html {G} --x3dom-url ./x3dom-full.js",
        ]
        ok &= check("the commands are those of the example, in its order", lines == expected, "\n".join(lines))

        print("\n=== only the lidar has a datalist, the multibeam has no grid ===")
        (survey / "multibeam" / "ZTopo.grd").unlink()
        (survey / "lidar" / "datalistp.mb-1").unlink()
        lines = [c.line() for c in script.plan(options)]
        ok &= check("a missing file leaves out its commands only",
                    [l.split()[0] + " " + l.split()[2] for l in lines] == ["mbgrd2gltf lidar/ZTopo.grd", "mbmesh multibeam/datalistp.mb-1",
                                                                          "mbglb2tiles lidar/ZTopo.glb", "mbglb2tiles multibeam/mbmesh_output/mesh.glb", "mbmesh ."],
                    "\n".join(lines))
        ok &= check("a subdirectory that is not there is left out", script.existing_subdirectories(
            script.TilesOptions(survey_directory=str(survey), subdirectories=("lidar", "multibeam", "sidescan"))) == ["lidar", "multibeam"])

        print("\n=== missing inputs ===")
        other_survey = Path(tmp) / "other_survey"
        touch(other_survey / "multibeam" / "ZTopo.grd")
        touch(other_survey / "multibeam" / "datalistp.mb-1")
        touch(other_survey / "lidar" / "datalistp.mb-1")
        warn_options = script.TilesOptions(survey_directory=str(other_survey))
        warnings = script.missing_inputs(warn_options)
        ok &= check("a subdirectory with a datalist and no grid is warned about", len(warnings) == 1 and "lidar/ZTopo.grd not found" in warnings[0] and "no ZTopo.glb" in warnings[0], "\n".join(warnings))
        (other_survey / "multibeam" / "datalistp.mb-1").unlink()
        warnings = script.missing_inputs(warn_options)
        ok &= check("a missing datalist is warned about too", any("multibeam/datalistp.mb-1 not found" in w and "no mesh" in w for w in warnings), "\n".join(warnings))
        touch(other_survey / "multibeam" / "datalistp.mb-1")
        touch(other_survey / "lidar" / "ZTopo.grd")
        ok &= check("nothing is warned about when the inputs are there", script.missing_inputs(warn_options) == [], "\n".join(script.missing_inputs(warn_options)))

        print("\n=== options ===")
        touch(survey / "multibeam" / "ZTopo.grd")
        options = script.TilesOptions(survey_directory=str(survey), subdirectories=("multibeam",), geoorigin=ORIGIN, lift_meters=12.5, threads=2, lod=None,
                                      label_chunks=False, x3dom_url=None, extra_mbmesh=["--decimate", "0.05"], survey_page=False, cesium_html=False)
        lines = [c.line() for c in script.plan(options)]
        ok &= check("the options change the commands",
                    lines[1] == f"mbmesh --input multibeam/datalistp.mb-1 --html --threads 2 {G} --raw-mesh-glb --pointcloud-glb --track-glb --output multibeam/mbmesh_output --decimate 0.05" and
                    lines[2] == "mbglb2tiles --input multibeam/ZTopo.glb --html --lift 12.5 --output multibeam/ZTopo_tileset" and len(lines) == 4, "\n".join(lines))
        options.cesium_html, options.cesium_url = True, "./cesium"
        lines = [c.line() for c in script.plan(options)]
        ok &= check("--cesium-html and --cesium-url are in every command that writes a page",
                    all("--cesium-html --cesium-url ./cesium" in l for l in lines) and len(lines) == 4, "\n".join(lines))
        options.cesium_html, options.cesium_url = False, None
        lines = [c.line() for c in script.plan(options)]
        ok &= check("without them no command has an option for CesiumJS", not any("cesium" in l for l in lines), "\n".join(lines))
        text = script.script_text(options, script.plan(options))
        ok &= check("the script stops on an error and starts in the survey directory", "set -euo pipefail" in text and f"cd {os.path.abspath(survey)}" in text)
        try:
            script.plan(script.TilesOptions(survey_directory=str(survey)))
            ok &= check("no GeoOrigin is an error", False)
        except ValueError:
            ok &= check("no GeoOrigin is an error", True)

        print("\n=== freshness: the shellscript with stand-in programs ===")
        fresh = Path(tmp) / "fresh"
        for d in ("multibeam", "lidar"):
            touch(fresh / d / "ZTopo.grd")
            (fresh / d / "datalistp.mb-1").write_text("$PROCESSED\ndatalist.mb-1 -1\n")
            (fresh / d / "datalist.mb-1").write_text("swath1.mb88 88 1.0\n")
            touch(fresh / d / "swath1.mb88")
            touch(fresh / d / "swath1p.mb88")
        bin_dir = Path(tmp) / "bin"
        bin_dir.mkdir()
        # Each stand-in writes what its program writes, appends its command to a log, and does nothing else
        stand_in = """#!/bin/bash
echo "$(basename $0) $*" >> "$STAND_IN_LOG"
args=("$@")
value() { for ((i = 0; i < ${#args[@]}; i++)); do [ "${args[$i]}" = "$1" ] && echo "${args[$((i + 1))]}"; done; }
case "$(basename $0)" in
  mbgrd2gltf) o=$(value --output); touch "$o.glb" "$o.html" "$o_cesium.html" "${o}_cesium.html" ;;
  mbmesh)
    i=$(value --input)
    if [ "$i" = "." ]; then touch mbmesh_survey.html mbmesh_survey_cesium.html
    else o=$(value --output); mkdir -p "$o"; touch "$o/mesh.glb" "$o/raw_mesh.glb" "$o/pointcloud.glb" "$o/track.glb" "$o.html" "${o}_cesium.html"; fi ;;
  mbglb2tiles)
    o=$(value --output); [ -z "$o" ] && o="$(dirname $(value --input))/tileset"
    mkdir -p "$o"; touch "$o/tileset.json" "$o.html" "${o}_cesium.html" ;;
esac
"""
        for name in ("mbgrd2gltf", "mbmesh", "mbglb2tiles"):
            (bin_dir / name).write_text(stand_in)
            (bin_dir / name).chmod(0o755)
        log = Path(tmp) / "stand_in.log"
        env = dict(os.environ, PATH=f"{bin_dir}:{os.environ['PATH']}", STAND_IN_LOG=str(log))
        fresh_options = script.TilesOptions(survey_directory=str(fresh), geoorigin=ORIGIN, lift_meters=856.0)
        script_path = Path(tmp) / "fresh.sh"
        script.generate_and_write(fresh_options, str(script_path))

        def run(extra_env=None):
            log.write_text("")
            result = subprocess.run(["bash", str(script_path)], capture_output=True, text=True, env=dict(env, **(extra_env or {})))
            return result, [line.split()[0] + " " + " ".join(line.split()[1:3]) for line in log.read_text().splitlines()]

        result, ran = run()
        ok &= check("a first run makes everything: 2 grids, 2 meshes, 2 tilesets of the grids, 2 of the meshes, the survey page", result.returncode == 0 and len(ran) == 9, result.stderr + "\n".join(ran))
        result, ran = run()
        ok &= check("a second run makes nothing, and says so", result.returncode == 0 and ran == [] and "Up to date, not made again" in result.stdout, result.stderr + "\n".join(ran))
        # An input made newer: the grid of multibeam. Its GLB, its tileset and the survey page are stale; nothing else is.
        later = time.time() + 100
        os.utime(fresh / "multibeam" / "ZTopo.grd", (later, later))
        result, ran = run()
        ok &= check("a newer grid makes its GLB, then its tileset, then the survey page, and nothing else",
                    ran == ["mbgrd2gltf --input multibeam/ZTopo.grd", "mbglb2tiles --input multibeam/ZTopo.glb", "mbmesh --input ."], "\n".join(ran))
        past = time.time() - 1000   # an input is made newer than its outputs by touching it into the future, and old again once it has been used
        os.utime(fresh / "multibeam" / "ZTopo.grd", (past, past))
        # A swath file made newer: the mesh of lidar is made again, then its tileset, then the survey page
        later += 100
        os.utime(fresh / "lidar" / "swath1p.mb88", (later, later))
        result, ran = run()
        ok &= check("a newer swath file makes the mesh of its subdirectory, its tileset and the survey page",
                    ran == ["mbmesh --input lidar/datalistp.mb-1", "mbglb2tiles --input lidar/mbmesh_output/mesh.glb", "mbmesh --input ."], "\n".join(ran))
        os.utime(fresh / "lidar" / "swath1p.mb88", (past, past))
        # A missing page makes the command that writes it run again
        (fresh / "multibeam" / "ZTopo_cesium.html").unlink()
        result, ran = run()
        ok &= check("a missing output makes its command run again, and what depends on it", ran[:1] == ["mbgrd2gltf --input multibeam/ZTopo.grd"], "\n".join(ran))
        result, ran = run({"FORCE": "1"})
        ok &= check("FORCE=1 makes everything again", len(ran) == 9, "\n".join(ran))
        forced = script.TilesOptions(survey_directory=str(fresh), geoorigin=ORIGIN, lift_meters=856.0, force=True)
        script.generate_and_write(forced, str(script_path))
        result, ran = run()
        ok &= check("a shellscript made with --force makes everything again", len(ran) == 9, "\n".join(ran))
        # What -V says, as freshness(): after a full run all is up to date; an input made newer makes its commands, and what follows from them, run
        fresh_commands = script.plan(fresh_options)
        script_path_run = subprocess.run(["bash", str(script_path)], capture_output=True, text=True, env=env)
        ok &= check("-V: after a run every command is up to date", [r for r, _ in script.freshness(fresh_options, fresh_commands)] == [False] * 9 and
                    all("newer than inputs" in why for _, why in script.freshness(fresh_options, fresh_commands)))
        later = time.time() + 100
        os.utime(fresh / "multibeam" / "ZTopo.grd", (later, later))
        status = script.freshness(fresh_options, fresh_commands)
        ok &= check("-V: a newer grid makes its GLB run, and then what follows from it, with the reasons",
                    [(c.program, c.args[1], r) for c, (r, _) in zip(fresh_commands, status) if r] ==
                    [("mbgrd2gltf", "multibeam/ZTopo.grd", True), ("mbglb2tiles", "multibeam/ZTopo.glb", True), ("mbmesh", ".", True)] and
                    "multibeam/ZTopo.grd is newer than multibeam/ZTopo.glb" in status[0][1] and "made again by an earlier command" in status[4][1], str(status))
        (fresh / "lidar" / "ZTopo_cesium.html").unlink()
        os.utime(fresh / "multibeam" / "ZTopo.grd", (past, past))
        status = script.freshness(fresh_options, fresh_commands)
        ok &= check("-V: a missing output is the reason, by name", status[1] == (True, "lidar/ZTopo_cesium.html is missing"), str(status[1]))
        forced_options = script.TilesOptions(survey_directory=str(fresh), geoorigin=ORIGIN, lift_meters=856.0, force=True)
        ok &= check("-V with --force: every command runs", all(r and why == "--force" for r, why in script.freshness(forced_options, script.plan(forced_options))))
        ok &= check("the inputs of mbmesh are the datalist and the files it names, the processed ones too",
                    script.datalist_files(str(fresh / "lidar" / "datalistp.mb-1")) ==
                    [str(fresh / "lidar" / n) for n in ("datalistp.mb-1", "datalist.mb-1", "swath1.mb88", "swath1p.mb88")])

        print("\n=== GeoOrigin and lift ===")
        info = "x.grd\t-122.31\t-122.3\t36.39\t36.4\t-846.2\t-800\t0.001\t0.001\t11\t11\t-842.845\t10\t842.9\t0\t0\n"
        extent = script.parse_grdinfo(info)
        ok &= check("gmt grdinfo -C -L2 is read", (extent.west, extent.z_min, extent.z_mean) == (-122.31, -846.2, -842.845))
        ok &= check("the GeoOrigin is the center and the mean height", all(abs(a - b) < 1e-9 for a, b in zip(script.geoorigin_of([extent]), (-122.305, 36.395, -842.845))))
        other = script.GridExtent(-122.29, -122.28, 36.40, 36.42, -700, -600, -650.0)
        ok &= check("the GeoOrigin of several grids is the center of the area they cover and the mean of their mean heights",
                    all(abs(a - b) < 1e-9 for a, b in zip(script.geoorigin_of([extent, other]), (-122.295, 36.405, -746.4225))))
        ok &= check("the lift puts the lowest point 10 m above the ellipsoid, to a meter", script.lift_of([extent]) == 857.0 and
                    script.lift_of([extent, script.GridExtent(0, 0, 0, 0, -100, 0, 0)]) == 857.0 and script.lift_of([script.GridExtent(0, 0, 0, 0, 30, 40, 35)]) == 0.0)
        ok &= check("numbers are written without trailing zeros", script.geoorigin_option((-122.3040, 36.39280, -842.8450)) == G)

        if shutil.which("gmt"):
            print("\n=== gmt ===")
            grid = Path(tmp) / "g.grd"
            subprocess.run(["gmt", "grdmath", "-R-122.31/-122.30/36.39/36.40", "-I0.001", "-846", "=", str(grid)], check=True, capture_output=True)
            extent = script.read_grid_extent(str(grid))
            ok &= check("a grid is read with gmt grdinfo", abs(extent.z_mean + 846) < 1e-6 and abs(script.geoorigin_of([extent])[0] + 122.305) < 1e-9)

    print("\n" + ("ALL CHECKS PASSED" if ok else "SOME CHECKS FAILED"))
    return 0 if ok else 1


if __name__ == "__main__":
    raise SystemExit(main())
