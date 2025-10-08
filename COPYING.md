-------------------------------------------------------------------------------
**MB-System Copyright, Licensing and Caveat:**

-------------------------------------------------------------------------------

This file contains the copyright and licensing terms for the MB-System open
source software package for the processing and display of swath sonar data.
This file is located at the top of the MB-System source code distribution
directory structure.

Last Updated June 16, 2025

-------------------------------------------------------------------------------

**MB-System Copyright (C) 1993-2025 by**

**David W. Caress (caress@mbari.org)**  
Monterey Bay Aquarium Research Institute  
Moss Landing, California, USA

**Dale N. Chayes**  
Center for Coastal and Ocean Mapping  
University of New Hampshire  
Durham, New Hampshire, USA  

**Christian dos Santos Ferreira**  
Zentrum für Marine Umweltwissenschaften (MARUM)  
Universität Bremen  
Center of Marine Environmental Science (MARUM)  
University of Bremen  
Bremen Germany  

MB-System was created by Caress and Chayes in 1992 at the  
Lamont-Doherty Earth Observatory of  
Columbia University  
Palisades, NY 10964

All Rights Reserved

-------------------------------------------------------------------------------

**License**

The MB-System source code is distributed under the GNU General Public License,
version 3, as formulated by the GNU Project. Early MB-System distributions
were described as "public domain", which meant there was no restriction
whatsoever on the use of the code. For over a decade we have adopted the
more restrictive GNU GPL license in order to insure that anyone who distributes
software based in whole or in part on MB-System also distributes their
modified MB-System source code and any additional source code.

The file "GPL.md", also located at the top of the MB-System source code
distribution directory structure, contains the text of the GNU GPL version 3.
This text and associated explanations are available at the GNU Project website
https://www.gnu.org/licenses/gpl-3.0.en.html

The MB-System distribution includes some source code derived from packages that
are distributed separately and have been authored by programmers other than the
MB-System team. This includes:

* The Generic Sensor Format (GSF) library source located in mbsystem/src/gsf   
  The GSF package is maintained and distributed by Leidos Corporation under contract to the United States Naval Oceanographic Office. The current source code can downloaded from https://www.leidos.com/products/ocean-marine and is licensed using the LGPLv2.1 open source license.
* The SURF API (SAPI) library source located in mbsystem/src/surf  
  The SAPI package was released under the GNU General Public License by Atlas Hydrographic 
  in 2001; Atlas Hydrographic has since been acquired by Teledyne Marine Technologies Inc.
* The MR1PR library source located in mbsystem/src/mr1pr  
  This package was primarily developed by Roger Davis of the Hawaii Mapping Research Group 
  (HMRG) at the University of Hawaii. The MR1PR source code was released without restriction 
  by HMRG in 2000. This code is licensed as part of MB-System under the GNU General Public 
  License (version 3).
* The mb\_beaudoin() function found in mbsystem/src/mbio/mb_angle.c  
  This important function that calculates beam angles for multibeam sonars was contributed 
  by Jonathan Beaudoin of CCOM/JHC University of New Hampshire and John Hughes Clarke of 
  the Ocean Mapping Group, University of New Brunswick. This code is licensed as part of 
  MB-System under the GNU General Public License (version 3).
* The mb\_mergesort() function found in mbsystem/src/mbio/mb_esf.c  
  This implementation of the merge sorting algorithm derives from the GNU-Darwin Distribution. 
  This code is released under both the Apple Public Source License Version 1.1 and the BSD 
  license, with original copyright by the Regents of the University of California.
* The Terrain Relative Navigation (TRN) algorithm and related software  
  The TRN software included in MB-System derives from software developed by Steve
  Rock of Stanford University, many of Steve Rock's graduate students from 2000
  through 2020, and Rob McEwen, Rich Henthorn, and Kent Headley of MBARI. The 
  integration of TRN into MB-System was doen by Kent Headley and David Caress, and
  the relevant source files are found in mbsystem/src/mbtrn, mbsystem/src/mbtrnav,
  mbsystem/src/mbtrnframe, and mbsystem/src/mbtrnutils.
  This code is licensed as part of MB-System under the GNU General Public License 
  (version 3).

-------------------------------------------------------------------------------

**Caveat**

The MB-System source code and any associated documentation does not come with 
any warranties, nor is it guaranteed to work 
on your computer or to do anything useful. The user assumes full responsibility
for the use of this system. In particular, David W. Caress, the Monterey Bay 
Aquarium Research Institute, the Lamont-Doherty Earth Observatory of Columbia 
University, or any other individuals or organizations involved in the design 
and maintenance of the MB-System software package are NOT responsible for any 
damage that may follow from correct or incorrect use of these programs.

-------------------------------------------------------------------------------
