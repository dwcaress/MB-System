# Georeferencing and provenance metadata in glTF and X3D files

**Status: proposal.** This describes how `mbmesh` and `mbgrd2gltf` record where their
glTF (`.glb`) models are on the Earth, and how they were made, and the names they use. glTF
and 3D Tiles have no common vocabulary for this, so the names here are chosen to be
vendor-neutral and are offered for other tools to adopt. Where an existing name has the
meaning needed, it is reused, and the existing sources are listed below.

## The problem

Vertex positions in a glTF file are 32-bit floats. Geocentric (ECEF) coordinates are millions
of meters from the Earth's center, where a float resolves only 0.25 to 0.5 m. A model of a
small area is therefore written as vertices **relative to a center point** that is chosen near
the area, and the center is added back to get the true position. That center is the **GeoOrigin**,
a node of the X3D Geospatial component (`GeoOrigin`, with `geoSystem` and `geoCoords`) that exists for
the same purpose: it is the point that geospatial coordinates are relative to, which keeps
them small enough for single precision. MB-System takes the name from X3D and uses it in the
`--geoorigin` option; the glTF and 3D Tiles formats have no equivalent name, so the
vocabulary below is proposed for them.

If the center is not stored with the model it is easily lost: the file then shows a seafloor
a few hundred meters across and nothing says where it is. The same holds for how the file
was made.

## Where it is stored

| Format | Where | What |
|---|---|---|
| glTF 2.0 | `asset.extras.georeference` | where the model is |
| glTF 2.0 | `asset.extras.provenance` | how it was made |
| glTF 2.0 | `asset.extras["MB-System"]` | details particular to MB-System: what this file holds, counts and settings |
| X3D | `GeoMetadata.summary` | the geospatial facts, with the keywords that X3D defines |
| X3D | `GeoMetadata.metadata` | the same three blocks as a `MetadataSet` tree |
| X3D | `WorldInfo` | the same entries as readable `key: value` lines |

`extras` is the place glTF 2.0 provides for application-specific data, with a JSON object
recommended for portability. The X3D `GeoMetadata` node is defined by X3D 3.3 and 4.0 for
exactly this purpose; its `summary` field takes only the keywords the specification lists
(unknown keywords are ignored by a conforming browser), so everything else is carried by the
`metadata` field.

The glTF `extras` and the X3D `MetadataSet` are written from one tree, so they cannot disagree,
and the tests check that they do not.

## `georeference` (proposed for adoption)

```json
"georeference": {
  "schemaVersion": 1,
  "crs": "EPSG:4978",
  "vertexCoordinates": "relativeToCenter",
  "center": [-2749436.658, -4344156.687, 3760606.005],
  "geodeticCenter": {
    "crs": "EPSG:4979",
    "longitude": -122.399674,
    "latitude": 36.637060,
    "height": -2228.12
  },
  "heightReference": "ellipsoid",
  "extent": { "west": -122.39995, "south": 36.63694, "east": -122.39939, "north": 36.63724 }
}
```

| Name | Meaning |
|---|---|
| `schemaVersion` | Version of this vocabulary. 1 for this document. |
| `crs` | The coordinate reference system of the vertex coordinates, once the center is added back. An EPSG code: `EPSG:4978` is WGS 84 geocentric Cartesian, in meters. |
| `vertexCoordinates` | `"absolute"` if the vertices are already in `crs`, `"relativeToCenter"` if `center` must be added to every vertex to get coordinates in `crs`, or `"localFrame"` if the vertices are in the local east-north-up frame at `center` (see below). |
| `center` | Present when `vertexCoordinates` is `"relativeToCenter"`. A 3-vector in meters in `crs`. Same name and same meaning as `CESIUM_RTC.center` and the 3D Tiles `RTC_CENTER`: **absolute = vertex + center**. |
| `geodeticCenter` | The same point as `center`, as WGS 84 geodetic coordinates in decimal degrees and meters. Present when `center` is. Written with named members because `EPSG:4979` orders its axes latitude, longitude, height, which is often confused. |
| `heightReference` | What `height` is measured from. `"ellipsoid"`: the height above the WGS 84 ellipsoid. |
| `extent` | The bounds of the content as WGS 84 geodetic coordinates in decimal degrees. |

The axes of the vertices are those of `crs`; they are **not** rotated to a local east-north-up
frame. For a model with local axes use a node transform, or the 3D Tiles tile `transform`,
instead.

To get absolute coordinates: `absolute = vertex + center` when `vertexCoordinates` is
`"relativeToCenter"`, and `absolute = vertex` when it is `"absolute"`.

### Content of a 3D Tiles tileset: `localFrame`

The tile files of an OGC 3D Tiles tileset (`mbmesh --tileset`) are not offsets in ECEF axes. 3D Tiles
glTF content is Y-up and is placed by the transform of the tileset, so their vertices are in the
east-north-up frame at the frame origin, written Y-up: X is east, Y is up and Z is south. `vertexCoordinates` is
`"localFrame"` and `center` is the origin of that frame, in ECEF. To get absolute coordinates:
`absolute = center + east * x + north * (-z) + up * y`, where `east`, `north` and `up` are the unit
vectors of the local frame at `geodeticCenter` expressed in ECEF; they are the first three columns of the
`transform` of the root tile in `tileset.json`, a 4x4 matrix in column-major order whose last column is `center`. A
viewer that follows 3D Tiles (turn Y-up to Z-up with a quarter turn about X, then apply the transform) needs none of this.

### Why these names

Each name is either borrowed from an existing source, with its meaning kept, or is new. The
sources were checked when this was written; where a source uses a different spelling, that is
stated.

| Name | Where it comes from |
|---|---|
| `center` | Borrowed. `CESIUM_RTC.center` (a glTF extension, written for glTF 1.0 and still found in glTF 2.0 files) and `RTC_CENTER` (3D Tiles 1.0 tile formats) both mean a center that is added to vertices that are relative to it. 3D Tiles 1.1 moves this to the translation of the glTF root node. |
| `crs` and the EPSG codes | Borrowed. ISO 19111 coordinate reference systems, identified by their EPSG code. |
| `longitude`, `latitude`, `height` | Borrowed, written out in full. The OGC GeoPose standard uses `lon`, `lat` and `h` for WGS 84 decimal degrees and a height in meters, and X3D uses `geoCoords`. Named members, not an array, are used because `EPSG:4979` orders its axes latitude, longitude, height, and arrays in either order are often misread. |
| `extent`, `west`, `south`, `east`, `north` | Borrowed, shortened. ISO 19115 `EX_GeographicBoundingBox` uses `westBoundLongitude`, `eastBoundLongitude`, `southBoundLatitude` and `northBoundLatitude`, in decimal degrees on WGS 84. The X3D `GeoMetadata` `extent` keyword is not used for this, because the specification states its order as east, south, west, north and shows an example in west, south, east, north order. |
| `heightReference` | Borrowed in spirit. CesiumJS has a `HeightReference` whose `NONE` value means a height above the ellipsoid, while its other values are relative to terrain or 3D Tiles. Here the value is the plain word `"ellipsoid"`, so that nothing depends on an enumeration. |
| `startedAtTime`, `software`, `inputs` | Borrowed, by linking. W3C PROV-O `prov:startedAtTime`, `prov:wasAssociatedWith` and `prov:used`, through the JSON-LD `@context`; the names stay readable. |
| `geodeticCenter` | New. There is no convention for the geodetic form of a center. |
| `vertexCoordinates` | New. Files whose vertices are absolute and files whose vertices are relative to a center are otherwise indistinguishable. |
| `georeference`, `provenance` | New, and neutral: they name what the block is, not a product, so that other software can write them. |

## `provenance` (proposed for adoption)

```json
"provenance": {
  "@context": {
    "prov": "http://www.w3.org/ns/prov#",
    "xsd": "http://www.w3.org/2001/XMLSchema#",
    "startedAtTime": { "@id": "prov:startedAtTime", "@type": "xsd:dateTime" },
    "software": { "@id": "prov:wasAssociatedWith" },
    "inputs": { "@id": "prov:used" }
  },
  "@type": "prov:Activity",
  "schemaVersion": 1,
  "software": { "@type": "prov:SoftwareAgent", "name": "mbmesh", "version": "5.8.3beta24" },
  "startedAtTime": "2026-10-22T16:09:29Z",
  "commandLine": "mbmesh -I datalist.mb-1 -L 0.5 -G --track-glb --html",
  "workingDirectory": "/data/OceanImaging2018/20180923d1/multibeam",
  "user": "mccann",
  "host": "menard",
  "inputs": {
    "@type": "prov:Entity",
    "datalist": "/data/.../datalist.mb-1",
    "formats": [89],
    "fileCount": 73,
    "files": ["/data/.../20180923_153141.mb89"],
    "firstTime": "2018-09-23T15:31:41Z",
    "lastTime": "2018-09-23T23:59:12Z"
  }
}
```

All times are UTC, in ISO 8601.

### Linked to W3C PROV-O

The block is also JSON-LD. Its `@context` links the names to [W3C PROV-O](https://www.w3.org/TR/prov-o/)
(`http://www.w3.org/ns/prov#`), so that JSON-LD and RDF software reads it as PROV statements: the run is a
`prov:Activity`; `startedAtTime` is `prov:startedAtTime`; `software` (a `prov:SoftwareAgent`) is
`prov:wasAssociatedWith`; and `inputs` (a `prov:Entity`) is `prov:used`. Software that does not
know JSON-LD sees ordinary members, and the names stay readable.

What PROV-O does not cover has no link and is plain description: `commandLine`, `workingDirectory`,
`host`, `user` and the details inside `software` and `inputs`. The GLB, which `prov:wasGeneratedBy` the
run, is the document the block is in, so it is not repeated. No `endedAtTime` is given because the
extras are fixed before the run has finished (chunks are written while it is still reconstructing).

The `@`-keywords are not data: the X3D `MetadataSet` and the `WorldInfo` lines leave them out and show
only the values. The block has not been run through a JSON-LD processor; the mapping follows the
PROV-O terms and the JSON-LD 1.1 syntax.

## `MB-System` (specific to this software)

What this particular file holds, and the settings of the program that made it. Other
software is free to ignore it. Every file has:

| Name | Meaning |
|---|---|
| `schemaVersion` | Version of this block. |
| `verticalReference` | What the height of a vertex is measured from, in words (see "What these files do not say"). |
| `vertexConvention` | In words, how to get from vertices to `crs` coordinates. The same statement as `vertexCoordinates` and `center`. |
| `geoOrigin.mode` | `"none"`, `"automatic"` (the center of the data) or `"specified"` (`--geoorigin=lon,lat,elev`). |
| `content` | `role` (what this file holds, for example `"mesh"`, `"chunk"`, `"tileset_tile"`, `"track"`) and a `description`; a chunk also has `chunk`, and a tile of a tileset `tile`, its `[i, j]` index, a Draco-compressed file has `compression` (`extension`, `positionBits`, `positionStep` in meters and `normalBits`; `mbgrd2gltf` writes it as well, with the most bits and the coarsest step of its tiles and no `normalBits`), and a chunk has `chunkLabel`, where it is, for drawing its outline and name in a survey page: `corners` (the four corners of the square the chunk owns, at the mean height of its surface, in order around it) and `center`, each `[longitude, latitude, height above the ellipsoid]`, `rise` (meters above the center, along the local vertical, where the name is) and `textSize` (meters). |

`mbmesh` adds `soundings` (pings, read, accepted, flagged, out of bounds), `area`
(whether `-R` was applied, and its bounds) and `reconstruction` (level of detail, Poisson
cell size, normal search radius, point spacing, single-pass or chunked, and for chunked
runs the chunk size, halo and thread count). `mbgrd2gltf` adds `grid` (columns, rows,
longitude and latitude spacing, and the `"gridline"` or `"pixel"` registration) and
`model` (vertical exaggeration, mean altitude the exaggeration is applied about, whether
and whether the output is binary). The
`provenance.inputs` of `mbmesh` list the datalist, the MBIO formats, the swath files (at
most 1000 are named, the count is always given) and the time of the first and last ping;
those of `mbgrd2gltf` give the `gridFile` and its `format`.

Both programs write the blocks `georeference` and `provenance` with the same names, so
that a viewer can read the position of a model without knowing which program made it.
The two programs share no code for this; `tests/test_provenance.cpp` of each checks the
names against this document.

## How to read them

```python
import json, struct
data = open("mesh.glb", "rb").read()
length = struct.unpack("<I", data[12:16])[0]          # first chunk is the JSON
extras = json.loads(data[20:20 + length])["asset"]["extras"]
center = extras["georeference"].get("center")          # None if the vertices are absolute
```

In the browser, X3DOM shows the `WorldInfo` lines in its log (the Log check box), and the
page's `GeoMetadata` node holds the same tree.

## X3D

```xml
<WorldInfo title='OceanImaging2018/20180923d1/multibeam' info='"georeference.crs: EPSG:4978" ...'></WorldInfo>
<GeoMetadata summary='"title" "..." "coordinateSystem" "GC" "horizontalDatum" "W84" "ellipsoid" "WE" ...'>
  <MetadataSet containerField='metadata' name='extras' reference='.../GEOREFERENCE.md'> ... </MetadataSet>
</GeoMetadata>
```

The `summary` keywords used are those of Table 25.5 of the X3D geospatial component:
`title`, `description`, `coordinateSystem` (`GC`, geocentric), `horizontalDatum` (`W84`),
`ellipsoid` (`WE`), `verticalDatum`, `resolution`, `originator`, `date` (the time range of the
data), `dataUrl` and `dataFormat`. The page's `GeoOrigin` node, which X3D 4.0 discourages but
which X3DOM needs to place the model, holds the same point as `center`.

## What these files do **not** say

* The height of a sounding is the MBIO depth below the sea surface, negated and used as a
  height above the ellipsoid. No geoid correction is applied, so heights differ from true
  ellipsoidal heights by the geoid undulation (about 30 m near Monterey Bay). This is stated
  in the metadata as `verticalReference`.
* The glTF `extras` of a file are not changed if the file is edited by other software.
