# Source Directory: src/mbgrd2gltf/

This directory contains the source files for the program mbgrd2gltf, a 2D/3D grd to gltf conversion program.
The original gltf-generator program was created for STOQS by a CSUMB Capstone team in Fall 2021. This program was contributed to MB-System as part of the CSUMB Capstone in Spring 2023. Additional refinements and additions added as part of CSUMB Capstone in Spring 2024 (draco compression) and Fall 2025 (code refinement).

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

# Dependencies

* netCDF
* TinyGLTF
	* This is in the project files as a single-header library. No futher action is needed for this dependency.

# Previous Work

* mbgrd2gltf (originally gltf-generator) was made to replace the current 3d geometry generation pipeline used in STOQS. 
* This project was done for CSUMB capstone in fall 2021 to solve the following issue: https://github.com/stoqs/stoqs/issues/1093
* https://github.com/stoqs/stoqs/blob/master/stoqs/contrib/gltf-generator/README.md
* Refinements were completed for CSUMB Capstone in fall 2025 for the following issue: https://github.com/dwcaress/MB-System/issues/1521

# Code Formatting

* This directory uses its own .clang-format file to maintain a consistent coding style for the mbgrd2gltf program.
* To apply the formatting rules to this code, run the following script from within this directory:
	* ./clang-format.sh
* It is recommended to run before committing changes to keep the codebase consistent for future contributors.

