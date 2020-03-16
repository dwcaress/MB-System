### MB-System

Source Directory: **src/share/**

This directory contains data resources used by MB-System programs. 

The file LevitusAnnual82.dat contains a global 1 degree by 1 degree resolution model of ocean water properties. The program mblevitus uses this to generate water sound speed profile models at locations specified by users. 

The file Projections.dat contains a large set of geodetic projection definitions in the format used by the PROJ4 library. This file is kept for backward compatibility to allow building MB-System with old versions of Proj. Version 6 of Proj contains it's own, much more comprehensive coordinate system database.

These resource files, as distributed here, are licensed using GPL3 as part of MB-System.
