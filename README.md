--
### MB-System README File
--
This file contains general information regarding the MB-System open
source software package for the processing and display of swath sonar data.
This file is located at the top of the MB-System source code distribution directory structure and is displayed by the GitHub web interface on the MB-System source code repository main page at [https://github.com/dwcaress/MB-System](https://github.com/dwcaress/MB-System).

The copyright and licensing terms are contained in the file *COPYING.md*.
The GNU GPL version 3 license used for MB-System can be found in the file *GPL.md*.
The software authors are listed in the file *AUTHORS*.
A log of source code changes is in the file *CHANGELOG.md*.
Installation instructions are found in the file *NOTES*.

### Getting Help
--

The primary source of information about MB-System is the project website at [https://www.mbari.org/products/research-software/mb-system/](https://www.mbari.org/products/research-software/mb-system/), which includes sections on:

- [FAQ](https://www.mbari.org/products/research-software/mb-system/mb-system-faq/)
- [Download and installation](https://www.mbari.org/products/research-software/mb-system/how-to-download-and-install-mb-system/)
- [Documentation](https://www.mbari.org/products/research-software/mb-system/mb-system-documentation/)
- [User and developer email discussion lists](https://www.mbari.org/products/research-software/mb-system/mb-system-discussion-lists/)

Although the GitHub interface includes functionality for raising issues and messaging, these are seen only by the small number of developers that are currently contributing and modifying the code base, and should be used only for short term code-specific issues. 

**Questions about how to install or use MB-System should be directed at the broader MB-System community through the discussion lists linked above.** 

These discussion lists are described more completely near the bottom of this document.

Emails sent directly to members of the developer team are frequently never answered because individually we are often overwhelmed with our many responsibilities.  Again, **please use the discussion lists to ask questions, raise issues, make suggestions, or seek help with MB-System.**

### MB-System Version 5 Description
--

MB-System is a software package consisting of programs which manipulate,
process, list, or display swath sonar bathymetry, amplitude, and sidescan data.
This software is distributed freely (and for free) in the form of source code
for Unix platforms. The heart of the system is an input/output library called
MBIO which allows programs to work transparently with any of a number of
supported swath sonar data formats. This approach has allowed the creation
of "generic" utilities which can be applied in a uniform manner to sonar data
from a variety of sources. Most of the programs are command-line tools, but the
package does include graphical tools for editing swath bathymetry, editing
navigation, modeling bathymetry calculation, and adjusting survey navigation.

### MB-System Programs
--

| Program                 | Description
|-------------------------|-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------|
| mb7k2jstar(1)           | Extracts subbottom profiler and/or sidescan sonar data from Reson 7k format data files into Edgetech Jstar format data files                                                                                                                                                                                                                                                                        |
| mb7k2ss(1)              | Extracts sidescan sonar data from Reson 7k format data, bins and lays the sidescan onto the seafloor, and outputs files in the MBF_MBLDEOIH formst (MBIO format id 71)                                                                                                                                                                                                                              |
| mb7kpreprocess(1)       | Performs preprocessing of Reson 7k multibeam data in the 7k format (MBIO format 88) (deprecated, use mbpreprocess instead)                                                                                                                                                                                                                                                                                                                 |
| mbabsorption(1)         | Calculates the absorption of sound in sea water in dB/km as a function of frequency, temperature, salinity, sound speed, and depth                                                                                                                                                                                                                                                                  |
| mbareaclean(1)          | Tool to automatically flag bad beams in swath sonar bathymetry data within a specified area                                                                                                                                                                                                                                                                                                         |
| mbauvloglist(1)         | Adjusts inertial navigation of a submerged platform/vehicle to be consistent with more accurate but less precise surface tracking (USBL) navigation                                                                                                                                                                                                                                                 |
| mbbackangle(1)          | Generates tables of the average amplitude or sidescan values in swath sonar data as a function of the grazing angle with the seafloor                                                                                                                                                                                                                                                               |
| mbclean(1)              | Tool to automatically flag bad beams in swath sonar bathymetry data                                                                                                                                                                                                                                                                                                                                 |
| mbcontour(1)            | GMT compatible utility for color fill or color shaded relief swath plots of swath sonar data using Postscript                                                                                                                                                                                                                                                                                       |
| mbcopy(1)               | Copy swath sonar bathymetry data files                                                                                                                                                                                                                                                                                                                                                              |
| mbctdlist(1)            | List CTD data in swath sonar data files                                                                                                                                                                                                                                                                                                                                                             |
| mbdatalist(1)           | Parses recursive datalist files and outputs the complete list of data files, formats, and file weights                                                                                                                                                                                                                                                                                              |
| mbdefaults(1)           | Set or list default mbio parameters for reading and writing swath sonar data                                                                                                                                                                                                                                                                                                                        |
| mbedit(1)               | Interactive editor used to flag bad bathymetry values in swath sonar bathymetry data                                                                                                                                                                                                                                                                                                                |
| mbeditviz(1)            | 3D visualization-based editing of swath bathymetry soundings                                                                                                                                                                                                                                                                                                                                        |
| mbextractsegy(1)        | Extracts subbottom profiler, center beam reflection, or seismic reflection data from swath data supported by MB-System and rewrites it as a SEGY file                                                                                                                                                                                                                                               |
| mbfilter(1)             | Apply some simple filter functions to sidescan or beam amplitude data from swath sonar data files                                                                                                                                                                                                                                                                                                   |
| mbformat(1)             | List information about swath sonar data formats supported by the MBIO library                                                                                                                                                                                                                                                                                                                       |
| mbgetesf(1)             | Extract swath bathymetry data flags into an edit save file                                                                                                                                                                                                                                                                                                                                          |
| mbgrd2gltf(1)           | 2D/3D grd to gltf conversion program.
																						|
| mbgrdtiff(1)            | generates a geographically located TIFF image from a GMT grid file                                                                                                                                                                                                                                                                                                                                  |
| mbgrdviz(1)             | Simple interactive 2D/3D visualization of GMT grids                                                                                                                                                                                                                                                                                                                                                 |
| mbgrid(1)               | Grid bathymetry, amplitude, or sidescan data from swath sonar data files                                                                                                                                                                                                                                                                                                                            |
| mbhistogram(1)          | Generate a histogram of bathymetry, amplitude, or sidescan values from swath sonar data files                                                                                                                                                                                                                                                                                                       |
| mbinfo(1)               | Output some basic statistics of swath sonar data files                                                                                                                                                                                                                                                                                                                                              |
| mbio(3)                 | Format independent input/output library for swath mapping sonar data                                                                                                                                                                                                                                                                                                                                |
| mblevitus(1)            | Create a water velocity profile which is representative of the mean annual water column for a specified 1 degree by 1 degree region                                                                                                                                                                                                                                                                 |
| mblist(1)               | List data in swath data files                                                                                                                                                                                                                                                                                                                                                                       |
| mbm_arc2grd(1)          | Macro to convert an ArcView ASCII grid to a GMT grid file in the GMT NetCDF grid format                                                                                                                                                                                                                                                                                                             |
| mbm_copy(1)             | Apply mbcopy to all files referenced through a datalist, using the MB-System file suffix convention to name the output files                                                                                                                                                                                                                                                                        |
| mbm_dslnavfix(1)        | Translate WHOI DSL AMS-120 navigation from UTM eastings and northings to longitude and latitude                                                                                                                                                                                                                                                                                                     |
| mbm_fmtvel(1)           | Extract sound velocity info from Hydrosweep data files and create output in tabular format                                                                                                                                                                                                                                                                                                          |
| mbm_grd2arc(1)          | Macro to convert a GMT grid file in the GMT NetCDF grid format to an ArcView ASCII grid                                                                                                                                                                                                                                                                                                             |
| mbm_grd2geovrml(1)      | Create and execute commands which generate a TerraVision tileset and GeoVRML terrain set that can be combined with other geospatial data for viewing in a web browser                                                                                                                                                                                                                               |
| mbm_grd3dplot(1)        | Create an executable shellscript which will generate a GMT 3D perspective plot of gridded data in a GMT grd file                                                                                                                                                                                                                                                                                    |
| mbm_grdcut(1)           | Macro to extract a specified subarea of a GMT GRD grid file as another GRD file                                                                                                                                                                                                                                                                                                                     |
| mbm_grdinfo(1)          | Macro to get information regarding a GMT grd file when the region of interest is a subset of the area covered in the input file                                                                                                                                                                                                                                                                     |
| mbm_grdplot(1)          | Create an executable shellscript which will generate a GMT map of gridded data in a GMT grd file                                                                                                                                                                                                                                                                                                    |
| mbm_grdtiff(1)          | Create an executable shellscript which will generate a TIFF image of gridded data in a GMT grd file                                                                                                                                                                                                                                                                                                 |
| mbm_grid(1)             | Create an executable shellscript which will generate a grid (bathymetry or topography) or mosaic                                                                                                                                                                                                                                                                                                    |
| mbm_histplot(1)         | Create an executable shellscript which will generate a GMT histogram plot of a dataset                                                                                                                                                                                                                                                                                                              |
| mbm_makedatalist(1)     | Macro to generate a datalist of the swath files in a specified directory                                                                                                                                                                                                                                                                                                                            |
| mbm_plot(1)             | Create an executable shellscript which will generate a GMT plot of swath sonar swath data                                                                                                                                                                                                                                                                                                           |
| mbm_route2mission(1)    | Macro to convert an mbgrdviz route file into an MBARI AUV mission script                                                                                                                                                                                                                                                                                                                            |
| mbm_stat(1)             | Extract beam statistics from output generated by mbinfo                                                                                                                                                                                                                                                                                                                                             |
| mbm_utm(1)              | Macro to perform forward and inverse UTM projections of ASCII data triples                                                                                                                                                                                                                                                                                                                          |
| mbm_vrefcheck(1)        | Macro to generate plot of crosstrack seafloor slope from a swath sonar file. The noise in this time series largely reflects noise in the vertical reference used by the sonar                                                                                                                                                                                                                       |
| mbm_xbt(1)              | Calculate sound speed from XBT data                                                                                                                                                                                                                                                                                                                                                                 |
| mbm_xyplot(1)           | Create an executable shellscript which will generate a GMT plot of xy data                                                                                                                                                                                                                                                                                                                          |
| mbmask(1), mbdumpesf(1) | Output text version of beam edits from a binary edit save file                                                                                                                                                                                                                                                                                                                                      |
| mbmosaic(1)             | Mosaic amplitude or sidescan data from swath mapping sonar data files                                                                                                                                                                                                                                                                                                                               |
| mbnavadjust(1)          | Package that solves for optimal navigation by matching bathymetry of overlapping swaths                                                                                                                                                                                                                                                                                                             |
| mbnavedit(1)            | Interactive navigation editor for swath sonar data                                                                                                                                                                                                                                                                                                                                                  |
| mbnavlist(1)            | List navigation data in swath sonar data files                                                                                                                                                                                                                                                                                                                                                      |
| mbneptune2esf(1)        | Tool to import beam flags from Simrad's Neptune system                                                                                                                                                                                                                                                                                                                                              |
| mbotps(1)               | Predicts tides using the OSU Tidal Prediction Software (OTPS) distribution                                                                                                                                                                                                                                                                                                                          |
| mbprocess(1)            | Performs a variety of swath data processing functions in a single step (producing a single output swath data file), including merging navigation, recalculating bathymetry from travel time and angle data by raytracing through a layered water sound velocity model, applying changes to ship draft, roll bias and pitch bias, applying tides, and applying bathymetry edits from edit save files |
| mbps(1)                 | Generates a PostScript perspective plot of a piece of swath sonar data                                                                                                                                                                                                                                                                                                                              |
| mbrollbias(1)           | Assess roll bias of swath sonar sonar systems                                                                                                                                                                                                                                                                                                                                                       |
| mbrolltimelag(1)        | Calculates cross correlation between the apparent bottom slope in swath bathymetry data and the roll time series used by the sonar in order to assess attitude time lag problems                                                                                                                                                                                                                    |
| mbsegygrid(1)           | Generate time vs. trace number grids of seismic data from segy files                                                                                                                                                                                                                                                                                                                                |
| mbsegyinfo(1)           | Output some basic statistics of segy format seismic data files                                                                                                                                                                                                                                                                                                                                      |
| mbsegylist(1)           | List selected header values in segy format seismic data files                                                                                                                                                                                                                                                                                                                                       |
| mbsegypsd(1)            | Calculates the power spectral densisty function (PSD) of each trace in a segy file, outputting the PSD estimates as a GMT grid file with trace number along the x axis and frequency along the y axis                                                                                                                                                                                               |
| mbset(1)                | Sets values in mbprocess parameter files                                                                                                                                                                                                                                                                                                                                                            |
| mbstripNaN(1)           | Filter to remove NaN nodes                                                                                                                                                                                                                                                                                                                                                                          |
| mbsvplist(1)            | List water sound velocity profiles in swath sonar data files                                                                                                                                                                                                                                                                                                                                        |
| mbswath(1)              | GMT compatible utility for color fill or color shaded relief swath plots of swath sonar data using Postscript                                                                                                                                                                                                                                                                                       |
| mbsystem(1)             | A set of utilities for manipulating and processing swath sonar bathymetry, amplitude, and sidescan data                                                                                                                                                                                                                                                                                             |
| mbtime(1)               | Translate between calendar time values and unix time                                                                                                                                                                                                                                                                                                                                                |
| mbvelocitytool(1)       | Interactive water sound velocity profile editor                                                                                                                                                                                                                                                                                                                                                     |

### The Version 5 Data Processing Structure
--

MB-System version 5 features utilities implementing a parallel processing
scheme that simplifies the processing of most swath data. This scheme is
centered around the program mbprocess, which can accomplish the following
processing tasks in a single step:
   - Merge edited navigation generated by mbnavedit.
   - Apply bathymetry edit flags from mbedit
     and mbclean
   - Recalculate bathymetry from raw travel time and
     angle data by raytracing through water sound speed
     models from mbvelocitytool or mbsvplist.
   - Apply changes to roll bias, pitch bias, heading
     bias, and draft values.
   - Recalculate sidescan from raw backscatter samples
     (Simrad multibeam data only).
   - Correct sidescan for amplitude vs grazing angle
     patterns.
   - Apply tides to bathymetry.
The actions of mbprocess are controlled by text parameter files. Each mbprocess
parameter file is named by adding a ".par" suffix to the associated input swath
data file and contains single line commands that set processing modes and
parameters. Tools such as mbedit, mbnavedit, and mbclean all generate and/or
modify parameter files in addition to generating parallel files used by
mbprocess. The program mbset can also be used to create and modify mbprocess
parameter files.

One example of a possible data processing scheme is presented here:

   1) Run mbdatalist to create ancillary
      data files containing statistics
      (".inf"), quickly read bathymetry
      (".fbt"), and quickly read navigation
      (".fnv"). These files are used to
      speed common operations such as swath
      plotting and gridding.
	  Input:  mbari_1998_524.mb57
	  Output: mbari_1998_524.mb57.inf
		  mbari_1998_524.mb57.fbt
		  mbari_1998_524.mb57.fnv

   2) Run mbclean to identify
      the obvious bathymetric artifacts
      and output a list of the edit events.
      The parameter file is created and
      set to apply bathymetry flags from
      the ".esf" file.
	  Input:  mbari_1998_524.mb57
	  Output: mbari_1998_524.mb57.esf
		  mbari_1998_524.mb57.par

   3) Run mbedit to interactively
      identify bathymetric artifacts
      and output a list of the edit events.
      The existing edits from mbclean
      are loaded and applied prior to editing.
      The parameter file is updated and
      set to apply bathymetry flags from
      the ".esf" file.
	  Input:  mbari_1998_524.mb57
		  mbari_1998_524.mb57.esf
		  mbari_1998_524.mb57.par
	  Output: mbari_1998_524.mb57.esf
		  mbari_1998_524.mb57.par

   4) Run mbnavedit to interactively
      clean the navigation. The edited
      navigation is output to the ".nve" file.
      The parameter file is updated and
      set to merge the navigation from
      the ".nve" file.
	  Input:  mbari_1998_524.mb57
		  mbari_1998_524.mb57.par
	  Output: mbari_1998_524.mb57.nve
		  mbari_1998_524.mb57.par

   5) Run mbvelocitytool to generate an
      an appropriate sound velocity profile
      (SVP) for recalculating the bathymetry.
      The SVP is output to the ".svp" file.
      The parameter file is updated and
      set to recalculate the bathymetry by
      raytracing through the SVP model from
      the ".svp" file.
	  Input:  mbari_1998_524.mb57
		  mbari_1998_524.mb57.par
	  Output: mbari_1998_524.mb57.svp
		  mbari_1998_524.mb57.par

   6) Run mbbackangle to generate an
      a set of amplitude vs grazing angle
      tables at regular intervals in the
      data. These tables are placed into
      a single ".sga" file. The parameter
      file is updated and set to correct
      the sidescan by interpolating the
      amplitude vs grazing angle table for
      each ping.
	  Input:  mbari_1998_524.mb57
		  mbari_1998_524.mb57.par
	  Output: mbari_1998_524.mb57.sga
		  mbari_1998_524.mb57.par

   7) Run mbset to set the parameter file
      so that mbprocess will recalculate
      the sidescan (this is for Simrad
      multibeam data only) while ignoring
      sidescan samples from beams now flagged
      as bad.
	  Input:  mbari_1998_524.mb57.par
	  Output: mbari_1998_524.mb57.par

   8) Run mbprocess to apply the bathymetric
      edits, merge the cleaned navigation,
      recalculate the bathymetry, recalculate
      the sidescan, and correct the sidescan.
      The processed swath data is written to
      an output swath data file. The usual
      ancillary data files containing statistics,
      quickly read bathymetry, and quickly
      read navigation are also created.
	  Input:  mbari_1998_524.mb57
		  mbari_1998_524.mb57.esf
		  mbari_1998_524.mb57.nve
		  mbari_1998_524.mb57.svp
		  mbari_1998_524.mb57.aga
		  mbari_1998_524.mb57.par
	  Output: mbari_1998_524p.mb57
	          mbari_1998_524p.mb57.inf
	          mbari_1998_524p.mb57.fbt
	          mbari_1998_524p.mb57.fnv

The result of this processing is a single output swath data file. Moreover, the
processed output can be easily updated if, for example, additional bathymetry
editing is required. If the mbedit program is used again, it will load the
existing edit events from the ".esf" file and then update the ".esf" file. To
incorporate the updated bathymetry edits, one just runs mbprocess again. One
can similarly change the SVP file without impacting on the bathymetry
editing or navigation editing components of the processing.


### Other Required and Suggested Software and Data
--

MB-System requires a number of other software packages and databases to be fully functional. These are:

- GMT version 5.4 or later: MB-System makes use of the Generic Mapping Tools (GMT) libraries and programs for much of its graphics. GMT also includes a detailed global coastline database. GMT has been developed by Professor Paul Wessel of SOEST (School of Ocean and Earth Science and Technology at the Univeristy of Hawaii) and Dr. Walter H. F. Smith of NOAA. The GMT source code is available from the GMT web page.

- NetCDF version 4.0 or later: Both GMT and MB-System require the NCAR netCDF library.

- PROJ version 4 or later: PROJ is a generic coordinate transformation software that transforms geospatial coordinates from one coordinate reference system (CRS) to another. This includes cartographic projections as well as geodetic transformations.  MB-System depends on PROJ to deal with navigation in projected coordinate systems and to produce data products (e.g. grids) in coordinate systems other than WGS84 geographic. Support in MB-System from PROJ versions prior to 6.1 is deprecated, but still functional for MB-System version 5 releases.

- GDAL 1.11 or later: GDAL (Geospatial Data Abstraction Library) is a translator library for raster and vector geospatial data, and is now a prerequisite for GMT. Source code and documentation are available at the GDAL web page.

- Perl version 5.0 or later: Perl is a fast, well documented scripting language used widely in the Linux/Unix world. MB-System contains a number of perl scripts used both for installation and to ease common tasks. Most of the current Unix operating systems include perl. If you need to obtain the perl source code, first check with your local system administrator – you will probably find perl already available locally. The perl source code can be obtained from the GNU software archives of the Free Software Foundation.

- Perl Parallel-Forkmanager module: this Perl module is required for the parallel processing macros mbm_multicopy, mbm_multidatalist, and mbm_multiprocess to work. An easily installed package is available from from www.cpan.org.

- X11: The interactive graphical utilities in MB-System (MBedit, MBnavedit, MBvelocitytool, MBgrdviz, MBeditviz) use and require the X11 windowing system.

- Motif: The interactive graphical utilities in MB-System (MBedit, MBnavedit, MBvelocitytool, MBgrdviz, MBeditviz) use version 2 of the Motif widget set, and Motif libraries are required for these utilities to be built and to run. Most current Unix/Linux operating systems include the required Motif libraries, either in the original proprietary form (Motif), or the not-quite-open-source form (OpenMotif). If you do not have these libraries, the OpenMotif 2 source is available through Motifzone. An old alternative open source Motif-compatible distribution called LessTif does not work with the current OpenGL based MB-System programs (MBgrdviz, MBeditviz) and should be avoided.

- FFTW: The “Fastest Fourier Transform in the West” package is used by the sonagram calculation program MBbsegypsd to calculate, well, Fast Fourier Transforms. FFTW is commonly used and thus available on or for most current Unix operating systems.

- OTPS: The OSU Ocean Tide Prediction Software (OTPS) package is required for the tidal modeling program MBotps (which in fact is just a convenient front end for the old-style-batch interface of OTPS). The OTPS package can be obtained from the tidal modeling group at Oregon State University at https://www.tpxo.net. The global TPXO tidal models are made available for free to registered academic users, and can be licensed for a fee by non-academic groups.

MB-System produces Postscript based graphics. Most current Unix operating systems provide a program which serves as a screen-based Postscript viewer. If you do not have a Postscript viewer, one option is to obtain one of several Ghostscript-based packages from the Free Software Foundation. The best is gv:

- gv: Open source screen Postscript viewer. This package is a bit more sophisticated than ghostview and can handle poster-sized plots. The source code can be obtained from the GNU software archives of the Free Software Foundation.

### Documentation
--

The MB-System web site has a variety of documentation, and is located at [https://www.mbari.org/products/research-software/mb-system/
](https://www.mbari.org/products/research-software/mb-system/
).

Some documentation in the form of a number of html documents and images 
are included in the source distribution in the directory mbsystem/share/doc/mbsystem/html. 
These web pages include some general information about MB-System (e.g. who
wrote, how to get it, how to install it) and a complete set of old fashioned unix-style manual pages.

### MB-System Discussion Lists
--
We maintain two MB-System email discussion lists to facilitate communication among MB-System users and developers. The list server archives are publicly viewable and searchable over a web interface.

The **MB-System User’s Discussion List** is intended for questions about how to use MB-System, for discussions of bugs and other problems, and for suggestions about improving the software. We encourage users with questions and/or problems to use this list rather than emailing the developers directly. One must subscribe to the list in order to post or respond to messages, but the message archive is publicly viewable. The maximum message size in this list is 100K.

Please include with your carefully worded question details such as:

- computer and operating system you are using
mb-system version you are using
- FTP or equivalent link to the data file you are having trouble with
a description of the data file (what ship, what sonar, what acquisition system, where you found the file, etc.)
- a copy (cut and paste and/or attach as a text file) of the command script you are trying to execute
the output/error message you are getting

The web interface to the MB-System User’s Discussion List is at:
[http://listserver.mbari.org/sympa/info/mbsystem](http://listserver.mbari.org/sympa/info/mbsystem)

To subscribe to the MB-System User’s Discussion List go to:
[http://listserver.mbari.org/sympa/subscribe/mbsystem](http://listserver.mbari.org/sympa/subscribe/mbsystem)

To read the MB-System User’s Discussion List archives go to:
[http://listserver.mbari.org/sympa/arc/mbsystem](http://listserver.mbari.org/sympa/arc/mbsystem)

The **MB-System Developer’s Discussion List** is intended for detailed discussions amongst the active MB-System developers. All are welcome to join the developer’s list, but be warned that topics may not be of general interest. One must subscribe to the list in order to post messages, but the message archive is publicly viewable. The maximum message size in this list is 100K.

The web interface to the MB-System Developer’s Discussion List is at:
[http://listserver.mbari.org/sympa/info/mbsystem-dev](http://listserver.mbari.org/sympa/info/mbsystem-dev)

To subscribe to the MB-System Developer’s Discussion List go to:
[http://listserver.mbari.org/sympa/subscribe/mbsystem-dev](http://listserver.mbari.org/sympa/subscribe/mbsystem-dev)

To read the MB-System Developer’s Discussion List archives go to:
[http://listserver.mbari.org/sympa/arc/mbsystem-dev](http://listserver.mbari.org/sympa/arc/mbsystem-dev)


### Suggestions
--

We are interested in your suggestions. Please post in the discussion list
rather than emailing the authors directly.

### Bugs
--

There are undoubtably bugs in this software. Although we make no promises about
how rapidly problems will be fixed, we strongly encourage users to notify us of
bugs (and fixes!!). We will continue to support this software for the foreseeable
future.
