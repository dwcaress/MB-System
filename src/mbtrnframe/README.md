# Source Directory: src/mbtrnframe/

This directory contains the source files for one of several libraries and tools related
to Terrain Relative Navigation (TRN). The primary TRN tool in MB-System is the
program mbtrnpp, whose source is in [mbtrnutils](../mbtrnutils/). The program
mbtrnpp links with libraries in this directory and also the TRN library in the
direction [mbtrnav](../mbtrnav/). The TRN library has been developed by [Steve
Rock](https://profiles.stanford.edu/stephen-rock) and his students at Stanford
in collaboration with Rich Henthorn, Rob McEwen, and Mike Risi at the [Monterey
Bay Aquarium Research Institute](https://mbari.org/) (MBARI). The program
mbtrnpp has been developed by Kent Headley and David Caress of MBARI. The
primary use of mbtrnpp is to localize a submerged platform (e.g. AUV or ROV) by
comparing realtime multibeam bathymetry with a pre-existing map.

These source files are copyrighted and licensed in a variety of ways, as
indicated in the file headers.

