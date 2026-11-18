# mbdraco

Draco compression of triangle meshes (the glTF extension `KHR_draco_mesh_compression`), shared by `mbmesh` and `mbgrd2gltf` so that the two programs compress alike and take the same options.

| Option | Meaning |
|---|---|
| `--no-draco` | write the arrays; the default is to compress |
| `--draco-precision METERS` | the coarsest step of the positions, default 0.001 |
| `--draco-normal-bits N` | the bits of each normal, 2 to 30, default 10 (`mbgrd2gltf` has no normals) |

The positions of a mesh are quantized over the largest of the ranges of its three axes, with the number of bits (8 to 30) that makes the step no coarser than the precision: a small mesh gets few bits and a large one more. Where the range over the precision is more than 2^30 the step is coarser than asked, and the step that was used is what the file records, in `MB-System.content.compression` (`positionBits`, `positionStep`, `normalBits`). A file of many meshes, as the tiles of `mbgrd2gltf` are, records the most bits and the coarsest step.

## Code

- `draco_mesh.h`, `draco_mesh.cpp` - namespace `mbdraco`: `Settings` and the defaults; `position_bits()`; `parse_precision()` and `parse_normal_bits()` for the options; `encode()` of a mesh (positions, optional normals, triangles), which decodes the result again to check it and to count what it holds, and `decode()` for tests; `add_primitive()`, which encodes a mesh into a buffer shared by the meshes of a file and adds the primitive, its accessors (which have no buffer views) and the extension, listed as used and required, to a `tinygltf::Model`.
- `haxby_colors.h` - namespace `mbdraco`, header only: the Haxby color table of `mbm_grdplot`, the bounds that `mbm_grdplot -G5` chooses from a range of heights, and the color of a height, for the colors by height of `mbmesh`, `mbglb2tiles` and `mbgrd2gltf`.
- `draco/` - the Draco source (encoder and decoder), moved here from `src/mbgrd2gltf/draco`.
- `test_mbdraco.cpp` - the test, built by CMake as `mbdraco_test_draco_mesh`.

CMake builds the static library `mbdraco`, and the autotools build (`Makefile.am` here) the convenience library `libmbdraco.la`, from the same sources; `mbmesh`, `mbglb2tiles` and `mbgrd2gltf` link it, and the tinygltf headers are those that `mbgrd2gltf` holds. A program that is built without the library (`draco_mesh.cpp` compiled without `MBDRACO_WITH_DRACO`) has `available()` false and writes the arrays.
