.TH mbsystem l "10 August 2001" "MB-System 5.0" "MB-System 5.0"
.SH NAME
\fBmbsystem\fP - A set of utilities for manipulating and 
processing swath sonar bathymetry, amplitude, and sidescan data.

.SH VERSION
Version 5.0

.SH INTRODUCTION
\fBMB-System\fP is a software package consisting of programs which manipulate, process, list, or
display swath sonar bathymetry, amplitude, and sidescan data. This software is distributed freely
(and for free) in the form of source code for Unix platforms. The heart of the system is an
input/output library called \fBMBIO\fP which allows programs to work transparently with any of a
number of supported swath sonar data formats. This approach has allowed the creation of
"generic" utilities which can be applied in a uniform manner to sonar data from a variety of
sources. Most of the programs are command-line tools, but the package does include graphical
tools for editing swath bathymetry, editing navigation, modeling bathymetry calculation, and
adjusting survey navigation.

The acronym MB stands for MultiBeam; this reflects the fact that the early development of
\fBMB-System\fP focused on processing deep sea data from multibeam sonars. However, \fBMB-System\fP
now supports several data formats from sidescan and interferometric sonars and works with data
from all depth ranges. 

The National Science Foundation (NSF) has been the primary supporter of \fBMB-System\fP
development. NSF initially provided grants to the authors at the Lamont-Doherty Earth
Observatory in 1993 (two years), and 1995 (five years). SeaBeam Instruments also contributed
significant effort from 1994 to 1999. This support has been renewed through a new five year
grant (2001-2005) to Caress at the Monterey Bay Aquarium Research Institute (MBARI) and to
Chayes at Lamont. MBARI is matching the NSF support as part of its seafloor mapping program.

The original NSF mandate was to create a set of generic tools which would work with all of the
sonar data used in NSF-supported projects. Not surprisingly, the core of the \fBMB-System\fP user
community consists of NSF-supported researchers at U.S. institutions such as the
Lamont-Doherty Earth Observatory, the Scripps Institution of Oceanography, and the Woods
Hole Oceanographic Institution. However, \fBMB-System\fP has also come to be used widely in the
international marine geological community. 

The \fBMB-System\fP source code is distributed under the GNU General Public License as formulated
by the GNU Project. MB-System installation depends on some other software packages (e.g.
GMT and netCDF), all of which are freely available as source code.

.SH AUTHORSHIP
David W. Caress (caress@mbari.org)
.br
  Monterey Bay Aquarium Research Institute
.br
Dale N. Chayes (dale@ldeo.columbia.edu)
.br
  Lamont-Doherty Earth Observatory

.SH COPYRIGHT AND LICENSING
\fBMB-System\fP Copyright (C) 1993-2001 by
.br
David W. Caress (caress@mbari.org)
    Monterey Bay Aquarium Research Institute
    Moss Landing, CA 95039
.br
Dale N. Chayes (dale@ldeo.columbia.edu)
    Lamont-Doherty Earth Observatory
    Palisades, NY 10964
.br
All Rights Reserved
.br
All Wrongs Remembered

The MB-System source code is distributed under the GNU General Public License as formulated
by the GNU Project. Previous MB-System distributions have been described as "public domain",
which meant there was no restrictions whatsoever on the use of the code. We have now adopted
the more restrictive GNU GPL license in order to insure that anyone who distributes software
based in whole or in part on MB-System also distributes the modified MB-System source code
and any additional source code. 

The GNU GPL also prohibits the distribution of proprietary executables linked with
MB-System libraries unless the source code is also distributed. We waive this restriction on
distributing proprietary compiled programs for specific software products if and only if those
software products meet the following two conditions:
     1) The software product was created, sold, and delivered to customers using source code 
derived from MB-System release 4.6 distributions. 
     2) The software product was sold and delivered to customers prior to January 1, 2001. 

.SH HISTORY
The development of \fBMB-System\fP began in 1990 as part of ongoing research at L-DEO
involving swath bathymetry data collected with SeaBeam multibeam sonars. Development
was accelerated in 1991 as part of the effort to support the STN-Atlas Hydrosweep DS
multibeam sonar on L-DEO's ship, the R/V Maurice Ewing. The National Science
Foundation provided support in 1993 and 1994 to improve and extend \fBMB-System\fP. The
intent of this initial grant was to provide a standard "generic" set of tools for processing
and display of swath sonar data that could be used by the U.S. academic community. The
first generally released version of \fBMB-System\fP (3.0) was made available in the Spring of
1993. This was followed by versions 3.1 and 3.2 in July, 1993, version 3.3 in November,
1993, and version 3.4 in December 1993. All of these early releases supported only
SeaBeam and Hydrosweep data.

SeaBeam Instruments and Antarctic Support Associates provided additional support in
1994 for the development of \fBMB-System\fP, with particular emphasis on capabilities related
to the new SeaBeam 2100 series of sonars. A considerably enhanced \fBMB-System\fP version
4.0 was released on October 22, 1994; this release followed an almost complete rewrite of
the underlying source code. The new capabilities included support for sidescan as well as
bathymetry data and support for data from a number of very different sonars. 

The National Science Foundation funded a five year effort begun in 1995 to maintain and
further develop \fBMB-System\fP. From 1994 to 1997, SeaBeam Instruments (a major
multibeam sonar manufacturer and, at the time, the principal employer of David W.
Caress) provided significant support for \fBMB-System\fP development and maintenance.
Similarly, the Newport, RI office of the Science Applications International Corporation
(SAIC) supported some \fBMB-System\fP development during 1997-1998, when David W.
Caress worked there. Version 4.1 was released in November, 1994, followed by 4.2 in
February 1995, 4.3 on March 12, 1996, 4.4 on August 27, 1996, and 4.5 on September 23,
1997.

David W. Caress joined the Monterey Bay Aquarium Research Institute (MBARI) in
September, 1998. Version 4.6 was released on April 16, 1999. The final update to version
4.6 (4.6.10) was announced on March 8, 2000. The primary innovations during this period
included support for the new generation of Simrad multibeam sonars and tools for
generating data products that could be imported directly into GIS software packages.

The National Science Foundation has now funded a second five year grant to MBARI and
L-DEO which support the \fBMB-System\fP project from 2001-2005. The version 5.0 release,
of which this documentation is a part, incorporates yet another substantial rewrite of the
underlying code as well as providing significant new capabilities.

.SH WHAT'S NEW ABOUT VERSION 5
The version 5.0 release of \fBMB-System\fP includes a number of changes and improvements relative
to the version 4 releases. The most significant changes include:

\fBA new approach to managing data processing.\fP
.br
- Many tools - one output file. In previous versions of \fBMB-System\fP, each processing
program read an input swath data file and produced an output swath data file. This
"serial" processing scheme generally produced a large number of intermediate data
files. \fBMB-System\fP version 5.0 features the integration of the editing and analysis
tools with a single program, mbprocess, that outputs processed data files. The new
"parallel" processing scheme covers bathymetry data processing, but does not yet
incorporate the sidescan processing capabilities. All of the old tools and capabilities
are still part of the distribution. 
.br
- Recursive datalists. The lists of data files used by gridding and plotting programs can
now be recursive, making it simpler to manage data from many different surveys. 
.br
- Automatic format identification. \fBMB-System\fP programs will now attempt to
automatically identify the swath data format based on the filename suffix. 
.br
- Extended inf files. Users can generate inf files by directing the output of mbinfo to a
file named by adding an ".inf" suffix to the swath data file name. Several programs
can parse inf files, if they exist, to quickly obtain data locations or ranges. This
feature speeds operations such as gridding, mosaicing, and automated plotting. 

\fBNew tools.\fP
.br
- \fBmbnavadjust\fP. This new tool allows users to adjust poorly navigated surveys by
matching features in overlapping swathes. It is particularly useful for processing
surveys conducted from submerged platforms. 
.br
- \fBmbprocess\fP. This new tool performs a variety of processing tasks and produces a
single output processed swath data file. The program mbprocess can apply
bathymetry edits from mbedit and mbclean, navigation edits from mbnavedit, sound
velocity profile changes from mbvelocitytool, and a variety of other corrections. 
.br
- \fBmbset\fP. This new tool allows users to create and modify the parameter files used to
control the operation of mbprocess. 

\fBRestructuring the code.\fP
.br
- All of the C code now conforms to the ANSI C standard. 
.br
- The underlying input/output library (\fBMBIO\fP) has been substantially rewritten. The
structure has been streamlined, simplifying both future development and support of
the existing code. The \fBMBIO\fP API has been greatly modified. 

\fBHandling of old Simrad multibeam data. \fP
.br
- Vendor format data from the old Simrad multibeams (pre-1997 sonars) are now
supported by a single format id (51) rather than a separate format id for each sonar
model. The old format id's are automatically aliased to 51, so existing shellscripts
will continue to work. 
.br
- Sidescan data from old Simrad multibeams (pre-1997 sonars) are now handled in the
same manner as data from the newer sonars (e.g. EM3000, EM3000, EM120). The
raw samples in the vendor data format are binned, averaged, and interpolated into a
1024 pixel sidescan swath. This binned sidescan is not saved in the vendor format, so
it is recommended that the data be copied to an extended format (57) that stores both
bathymetry beam flags and processed sidescan. Format 57 is also used for processing
data from the current Simrad multibeam sonars. 

.SH THE NEW VERSION 5 DATA PROCESSING STRUCTURE
Previous versions of \fBMB-System\fP have emphasized
processing utilities that operate serially, that is, that
read in an input swath data file, modify it, and then
output a second swath data file. Serial processing utilities do allow
for great flexibility in processing because one uses only
the programs required and, in general, the order in which
the programs are used does matter. However, one consequence of serial
processing has been that processing data frequently results in a large
number of intermediate data files. For instance, if
an EM300 data file called "mbari_1998_524.mb57" has
spikes in the bathymetry, outliers in the navigation, and 
the bathymetry has been calculated using an incorrect model
for the water sound speed structure, users of \fBMB-System\fP
would previously have processed it with a sequence 
something like this:

     1) Run \fBmbclean\fP to automatically flag
        the obvious bathymetric artifacts.
            Input:  mbari_1998_524.mb57
            Output: mbari_1998_524_c.mb57

     2) Run \fBmbedit\fP to interactively flag
        bathymetric artifacts.
            Input:  mbari_1998_524_c.mb57
            Output: mbari_1998_524_ce.mb57

     3) Run \fBmbnavedit\fP to interactively clean
        the navigation.
            Input:  mbari_1998_524_ce.mb57
            Output: mbari_1998_524_cen.mb57

     4) Run \fBmbvelocitytool\fP to generate an
        appropriate sound velocity profile (SVP) for
        recalculating the bathymetry.
            Input:  mbari_1998_524_cen.mb57
            Output: good.svp

     5) Run \fBmbbath\fP to recalculate the
        bathymetry using the SVP file generated
        using \fBmbvelocitytool\fP.
            Input:  mbari_1998_524_cen.mb57
            Input:  good.svp
            Output: mbari_1998_524_cenb.mb57

The result of this processing includes the input swath
data file, the final swath data file, and three 
intermediate swath data files. The impact of this approach
on data storage requirements is obvious (though some users
have ameliorated the issue by working with scripts that
automatically delete unneeded data files or by piping
data from one non-graphical process to 
another to avoid making intermediate files). 
The serial processing approach
also presents data management problems because data files
frequently have different processing requirements, even
within a single survey. We have found that working with
very large datasets can be awkward when only a minority
of data files require navigation editing or bathymetric
recalculation, or when some data files require multiple
episodes of bathymetry and navigation editing.

\fBMB-System\fP version 5 includes new utilities implementing
a parallel processing scheme that simplifies the processing
of most swath data. This scheme is centered around the program
\fBmbprocess\fP, which can accomplish the following
processing tasks in a single step:
     - Merge edited navigation generated by \fBmbnavedit\fP.
     - Apply bathymetry edit flags from \fBmbedit\fP 
       and \fBmbclean\fP
     - Recalculate bathymetry from raw travel time and
       angle data by raytracing through water sound speed
       models from \fBmbvelocitytool\fP or \fBmbsvplist\fP.
     - Apply changes to roll bias, pitch bias, heading
       bias, and draft values.
     - Recalculate sidescan from raw backscatter samples
       (Simrad multibeam data only).
     - Apply tides to bathymetry.
.br
The actions of \fBmbprocess\fP are controlled by text
parameter files. Each \fBmbprocess\fP parameter file is
named by adding a ".par" suffix to the associated input
swath data file and contains single line commands that set 
processing modes and parameters. Tools such as \fBmbedit\fP,
\fBmbnavedit\fP, and \fBmbclean\fP all generate and/or modify
parameter files in addition to generating parallel files
used by \fBmbprocess\fP. The program \fBmbset\fP 
can also be used to create and modify \fBmbprocess\fP 
parameter files.

The same processing steps described above as a serial 
processing scheme can be accomplished with the new parallel
processing scheme as follows:

     1) Run \fBmbclean\fP to identify 
        the obvious bathymetric artifacts
        and output a list of the edit events.
        The parameter file is created and
        set to apply bathymetry flags from 
        the ".esf" file.
            Input:  mbari_1998_524.mb57
            Output: mbari_1998_524.mb57.esf
                    mbari_1998_524.mb57.par

     2) Run \fBmbedit\fP to interactively 
        identify bathymetric artifacts
        and output a list of the edit events.
        The existing edits from \fBmbclean\fP
        are loaded and applied prior to editing.
        The parameter file is updated and
        set to apply bathymetry flags from 
        the ".esf" file.
            Input:  mbari_1998_524.mb57
                    mbari_1998_524.mb57.esf
                    mbari_1998_524.mb57.par
            Output: mbari_1998_524.mb57.esf
                    mbari_1998_524.mb57.par

     3) Run \fBmbnavedit\fP to interactively 
        clean the navigation. The edited 
        navigation is output to the ".nve" file.
        The parameter file is updated and
        set to merge the navigation from 
        the ".nve" file.
            Input:  mbari_1998_524.mb57
                    mbari_1998_524.mb57.par
            Output: mbari_1998_524.mb57.nve
                    mbari_1998_524.mb57.par

     4) Run \fBmbvelocitytool\fP to generate an
        an appropriate sound velocity profile  
        (SVP) for recalculating the bathymetry.   
        The SVP is output to the ".svp" file.  
        The parameter file is updated and
        set to recalculate the bathymetry by 
        raytracing throught SVP model from
        the ".svp" file.
            Input:  mbari_1998_524.mb57
                    mbari_1998_524.mb57.par
            Output: mbari_1998_524.mb57.svp
                    mbari_1998_524.mb57.par

     5) Run \fBmbprocess\fP to apply the bathymetric
        edits, merge the cleaned navigation, and
        recalculate the bathymetry. The processed
        swath data is written to an output file.
            Input:  mbari_1998_524.mb57
                    mbari_1998_524.mb57.esf
                    mbari_1998_524.mb57.nve
                    mbari_1998_524.mb57.svp
                    mbari_1998_524.mb57.par
            Output: mbari_1998_524.mb57.esf

The result of this processing is a single output swath
data file. Moreover, the processed output can be easily
updated if, for example, additional bathymetry editing
is required. If the \fBmbedit\fP program is used again,
it will load the existing edit events from the ".esf"
file and then update the ".esf" file. To incorporate the
updated bathymetry edits, one just runs \fBmbprocess\fP
again. One can similarly change the SVP file without
impacting on the bathymetry editing or navigation editing
components of the processing.

All of the old, serial processing utilities are still
distributed with \fBMB-System\fP. However, some of the serial
tools have been replaced by new versions of the same name
(e.g. \fBmbedit\fP, \fBmbclean\fP, and \fBmbnavedit\fP).
In these cases, the old versions are preserved as programs
with "old" added to their names (e.g. \fBmbeditold\fP, 
\fBmbcleanold\fP, and \fBmbnaveditold\fP).
At present, sidescan correction and filtering are not
yet implemented in the new scheme, so programs such as
mbanglecorrect and mbfilter must still be used in a serial fashion.

.SH THE NEW VERSION 5 DATALIST FILES
Previous versions of \fBMB-System\fP have used lists of
data files, or datalists, as input to several programs
(e.g. \fBmbgrid\fP, \fBmbmosaic\fP, \fBmbinfo\fP, 
\fBmblist\fP, and \fBmbm_plot\fP). The basic datalist
entry has consisted of a swath data name (often including
the entire path) followed by a space and then the \fBMB-System\fP format
id for that file. Datalist entry lines starting with
the character '#' are considered comments. Version 5 extends
the definition and usage of datalists in several
significant ways. First, datalists may now be recursive.
A datalist entry may be another datalist file, as indicated
by a format id of -1. Second, datalists entries may contain
a third column which is interpreted as a gridding weight
value by \fBmbgird\fP. This third value may be used to
weight some data higher than other data. For example, one might
weight SeaBeam 2112 data with a value of 1.0 and lower
quality SeaBeam classic data with a value of 0.001. The result
would be that the newer 2112 data effectively overlies the less good
data wherever overlap occurs. The third new feature of datalists 
works with data processed using the new parallel processing scheme.
In the parallel processing scheme the raw data files are often,
but not always,
accompanied by processed files produced by \fBmbprocess\fP. 
It is awkward to maintain datalists that directly refer to the
current best datafiles. If one inserts the text $PROCESSED
into the first line of a datalist containing raw files, 
then programs like \fBmbgrid\fP will read the processed file
if it exists, but otherwise will read the raw file. Similarly,
a first line of $RAW will force the programs to only
read the raw files directly referenced in the datalist. These
options also work recursively. The first instance of
a $PROCESSED or $RAW entry will prevail over all
others encountered through a hierarchy of recursive datalists.

.SH LIST OF MB-SYSTEM PROGRAMS AND MACROS
See the individual manual pages for detailed information about 
specific programs.
See the manual page for \fBMBIO\fP for information about the 
i/o library and the swath sonar data formats supported by \fBMBIO\fP.

These are the current MB-system programs:
 \fBhsdump\fP:  Lists contents of the various sorts of 
 		data records in Hydrosweep DS data. 
 \fBmbanglecorrect\fP:  Apply a grazing angle correction 
 		to beam amplitude or sidescan data. 
 \fBmbbackangle\fP:  Generates a table of the average 
 		amplitude or sidescan values as a 
 		function of the grazing angle with 
 		the seafloor.
 \fBmbbath\fP:  Generates bathymetry from travel times 
 		in swath sonar data. 
 \fBmbclean\fP:  Automatically identifies and 
 		flags bad beams in swath sonar
 		bathymetry data. 
 \fBmbcleanold\fP:  Old tool that utomatically 
 		identifies and flags bad beams 
 		in swath sonar bathymetry data. 
 \fBmbcontour\fP:  Generate GMT compatible Postscript 
 		color swath contour plots.
 \fBmbcopy\fP:  Copy swath sonar data files. 
 \fBmbcut\fP:  Removes data from portions of swath
 		as specified by the user. 
 \fBmbdefaults\fP:  Set or list default mbio parameters 
 		for reading and writing swath sonar data 
 \fBmbedit\fP:  Interactive editor used to flag bad 
 		beams in swath sonar bathymetry data. 
 \fBmbeditold\fP:  Old interactive editor used to flag bad 
 		beams in swath sonar bathymetry data. 
 \fBmbfilter\fP:   Apply some simple filter functions 
 		to sidescan, beam amplitude, or 
 		bathymetry data. 
 \fBmbformat\fP:   List information about swath sonar 
 		data formats supported by the \fBMBIO\fP library. 
 \fBmbgetmask\fP:  Extract list of flagged or edited 
 		beams from a swath sonar data file.
 \fBmbgrid\fP:  Grid bathymetry, amplitude, and sidescan 
 		data from swath sonar data files. 
 \fBmbhistogram\fP:  Obtain histogram of bathymetry, 
 		amplitude, or sidescan data from
  		swath sonar data files.
 \fBmbinfo\fP:  Output some basic statistics of 
 		swath sonar data files. 
 \fBmblevitus\fP:  Create a water velocity profile 
 		which is representative of the mean 
 		annual water column for a specified 
 		1 degree by 1 degree region. 
 \fBmblist\fP:  List data in swath sonar data files. 
 \fBmbmask\fP:  Apply editing information obtained 
 		from one file with \fBmbgetmask\fP 
 		to another file.
 \fBmbmerge\fP:  Merge swath sonar data with new 
 		navigation. 
 \fBmbmosaic\fP:  Mosaic sidescan and amplitude data. 
 \fBmbnavadjust\fP:  Interactive navigation adjustment
 		package that adjusts navigation so
 		that swath bathymetry matches where
 		swathes overlap or cross. 
 \fBmbnavedit\fP:  Interactive editor used to fix
 		problems with navigation in
 		swath sonar data files. 
 \fBmbnaveditold\fP:  Old interactive editor used to fix
 		problems with navigation in
 		swath sonar data files. 
 \fBmbnavlist\fP:  List navigation data in swath 
 		sonar data files. 
 \fBmbprocess\fP: Performs a variety of processing
 		tasks in a single step, including
 		merging navigation, applying 
 		bathymetry edits, recalculating
 		bathymetry, and recalculating
 		sidescan.
 \fBmbps\fP:    Simple perspective views of swath
 		bathymetry in Postscript. 
 \fBmbrollbias\fP:  Evaluate the roll bias of a 
 		swath sonar system using two pieces 
 		of coincident bathymetry data 
 		collected with opposing ship headings.
 \fBmbset\fP:  Create and modify \fBmbprocess\fP
 		parameter files. 
 \fBmbsimradmakess\fP:  Regenerate sidescan imagery 
 		from the raw amplitude samples contained 
 		in data from Simrad EM300 and EM3000 sonars. 
 \fBmbsvplist\fP:  List water sound velocity profiles in swath
 		sonar data files.
 \fBmbswath\fP:  Generate GMT compatible Postscript 
 		color and color shaded relief swath plots.
 \fBmbtide\fP:  Corrects swath sonar bathymetry
 		for tide data. 
 \fBmbunclean\fP:  Unflags edited beams in swath sonar 
 		bathymetry data. 
 \fBmbvelocitytool\fP:  Interactive program for 
 		modeling the affect of the water 
 		velocity profile on swath sonar 
 		bathymetry calculations.

Macros are programs or shellscripts which make use of programs 
from the MB-system and other software packages to accomplish common
tasks easily.  These are the current MB-system macros:
 \fBmbmfmtvel\fP:  Scans a Hydrosweep DS data file 
 		and outputs a formatted table of 
 		the mean water velocity and surface 
 		water velocity values used in 
 		processing that data.
 \fBmbm_dslnavfix\fP:  Reads a WHOI DSL AMS-120 processed
 		navigation file containing UTM northings
 		and eastings and outputs a navigation file
 		containing longitude and latitude which is
 		suitable for use with \fBmbmerge\fP.
 \fBmbm_grdplot\fP:  Reads a GMT GRD grid file and 
 		writes a shellscript which will 
 		generate a GMT map of the data.
 \fBmbm_grd3dplot\fP:  Reads a GMT GRD grid file and 
 		writes a shellscript which will 
 		generate a GMT 3D perspective view
 		of the data.
 \fBmbm_plot\fP:  Reads a swath sonar data file and 
 		writes a shellscript which will 
 		generate a swath and/or contour 
 		plot of the data.
 \fBmbm_rollerror\fP:  Reads a swath sonar data file,
 		calculates the noise in the vertical reference 
 		used by the sonar, and generates a file 
 		containing roll corrections which
 		can be applied to the data.
 \fBmbm_stat\fP:  Runs \fBmbinfo\fP on a swath sonar data  
 		file and extracts beam statistics from 
 		the output of mbinfo.
 \fBmbm_vrefcheck\fP:  Generates a plot of high pass
 		filtered apparent crosstrack seafloor slope.
 \fBmbm_xbt\fP:  Processes a Sparton XBT data file 
 		and outputs a sound velocity profile 
 		file which can be used to process 
 		swath sonar data.
 \fBmbm_xyplot\fP:  Reads one or more ASCII "X-Y" 
 		data files and writes a shellscript 
 		which will generate an XY plot of the data.

.SH EXAMPLE PROCESSING APPROACHES
A number of processing streams for different kinds
of swath sonar data are provided below. All of these
examples use the older, serial processing tools.
Examples incorporating the new, parallel processing
scheme will be included in this documentation at a
later date.

.SH PROCESSING SEABEAM 2100 DATA

The following data processing stream is recommended for
data obtained with SeaBeam 2100 series multibeam sonars. 
SeaBeam 2112 sonars are currently in use on the R/V Knorr 
(operated by the Woods Hole Oceanographic Institution),
the R/V Revelle (operated by the Scripps Institution
of Oceanography), 
and the R/V Nathaniel B. Palmer (operated by Antarctic
Support Associates for the NSF), among other vessels.

Consider a data file "sb2112_example.mb41" 
containing one hour's worth of
SeaBeam 2112 data in the vendor format (format 41).
This file contains bathymetry, beam amplitude, and
sidescan data. The following commands
are typical for processing such data and generating
preliminary maps.

\fBStep 1\fP: What's in the data file?

First we run \fBmbinfo\fP to obtain statistics about the contents
of the data file:

 	mbinfo -F41 -I sb2112_example.mb41

\fBStep 2\fP: Edit the data.

We edit the data to check the quality of the bathymetry
and to flag artifacts as necessary by running \fBmbedit\fP.
The edited data are output when the done or quit button
is clicked.

 	mbedit -F41 -I sb2112_example.mb41 -O sb2112_example_e.mb41

\fBStep 3\fP: Generate some first cut plots of the data.

Now, we use the \fBmbm_plot\fP macro to generate the following plots:
     bathcont:  color fill bathymetry with contours
     bathshade: shaded relief color fill bathymetry
     bathamp:   color fill bathymetry overlaid with amplitude
     amp:       grayscale amplitude 
     ss:        grayscale sidescan
.br
We use the -S option to apply histogram equalization to
sidescan and amplitude data; note that for the bathymetry
overlaid with amplitude map we use -S0/1 so that the
bathymetry is not histogram equalized but the amplitude
data used for shading is histogram equalized.
First generate plot shellscripts:

 	mbm_plot -F41 -I sb2112_example_e.mb41 -G1 \\
 		-C    -N -V -Obathcont
 	mbm_plot -F41 -I sb2112_example_e.mb41 -G2 \\
 		      -N -V -Obathshade
 	mbm_plot -F41 -I sb2112_example_e.mb41 -G3 \\
 		-S0/1 -N -V -Obathamp
 	mbm_plot -F41 -I sb2112_example_e.mb41 -G4 \\
 		-S    -N -V -Oamp
 	mbm_plot -F41 -I sb2112_example_e.mb41 -G5 \\
 		-S    -N -V -Oss

Then run plot shellscripts to generate plots (postscript files)
and put them on the screen:

 	bathcont.cmd
 	bathshade.cmd
 	bathamp.cmd
 	amp.cmd
 	ss.cmd

If we had used the -X option in \fBmbm_plot\fP the plotting 
shellscripts would have been executed automatically,
bringing the plots up on the screen after they were
generated.

\fBStep 4\fP: Destripe the sidescan data.

Hull mounted multibeam sonars are subject to a variety of
noise sources (e.g. bubbles under the hull) which can cause
the sidescan from some pings to be "bad". If parts of some
sidescan pings are much higher or lower in amplitude than
the adjacent pings, a map made with the sidescan will appear
"stripey". The program \fBmbfilter\fP can be used to despike and
destripe data by operating a boxcar median filter with
low and high ration thresholds (each value is changed only 
if the original value divided by the local median is less 
than the low threshold or greater than the high threshold).
The -A2 causes the program to operated on sidescan data.
We use -S3/3/3/1 so that a median smoothing filter with 
dimensions of 3 acrosstrack by 3 alongtrack is applied to
the data with 1 iteration. The -T0.6/1.67 causes the
median filter to work with ratio thresholds of 0.6 and 1.67.

 	mbfilter -F41 -I sb2112_example_e.mb41  \\
 		-O sb2112_example_ed.mb41 \\
 		-A2 -S3/3/3/1 -T0.6/1.67 -V

We run \fBmbm_plot\fP again to see the results of the destriping:

 	mbm_plot -F41 -I sb2112_example_ed.mb41 -G5 \\
 		-S    -N -V -Ossdestripe
 	ssdestripe.cmd

\fBStep 5\fP: Obtain amplitude vs grazing angle functions.

The raw sidescan and  amplitude images are dominated by the 
contrast between the high values at the center of the
swath and the low values at the swath edges. In order
to focus variations in grayscale or color on structure
of interest, we seek to "flatten" the images by 
correcting the sidescan and amplitude for the drop in
backscatter amplitude with increasing seafloor grazing
angle. In order to make such corrections, we first use
\fBmbbackangle\fP to obtain amplitude vs grazing angle
functions for both amplitude and sidescan data:

 	mbbackangle -F41 -I sb2112_example_ed.mb41 \\
 		-A1 -V > avsga_amplitude.dat
 	mbbackangle -F41 -I sb2112_example_ed.mb41 \\
 		-A2 -V > avsga_ss.dat

\fBStep 6\fP: Apply amplitude vs grazing angle corrections.

Use \fBmbanglecorrect\fP to apply the amplitude vs grazing
angle correction to both amplitude and sidescan data
in turn (note the output of first run is input of second).
In the first case, the -G argument forces the program 
to ignore seafloor slope in calculating grazing angles, 
so bathymetric features remain in the amplitude or 
sidescan data:

 	mbanglecorrect -F41 -I sb2112_example_ed.mb41 \\
 		-A1/100 -S avsga_amplitude.dat -G -Z3800 \\
 		-O sb2112_example_eda1.mb41 -V
 	mbanglecorrect -F41 -I sb2112_example_eda1.mb41 \\
 		-A2/1000 -S avsga_ss.dat -G -Z3800 \\
 		-O sb2112_example_eda1a1.mb41 -V

In the second case, the program uses seafloor slope 
in calculating grazing angles, so bathymetric features 
tend to be removed from the amplitude or sidescan data:

 	mbanglecorrect -F41 -I sb2112_example_ed.mb41 \\
 		-A1/100 -S avsga_amplitude.dat -Z3800 \\
 		-O sb2112_example_eda2.mb41 -V
 	mbanglecorrect -F41 -I sb2112_example_eda2.mb41 \\
 		-A2/1000 -S avsga_ss.dat -Z3800 \\
 		-O sb2112_example_eda2a2.mb41 -V

Now generate first cut plots of the corrected amplitude
and sidescan data:

 	mbm_plot -F41 -I sb2112_example_eda1a1.mb41 -G3 \\
 		-S0/1 -N -V -Obathampcorr1
 	mbm_plot -F41 -I sb2112_example_eda1a1.mb41 -G4 \\
 		-S    -N -V -Oampcorr1
 	mbm_plot -F41 -I sb2112_example_eda1a1.mb41 -G5 \\
 		-S    -N -V -Osscorr1
 	mbm_plot -F41 -I sb2112_example_eda2a2.mb41 -G3 \\
 		-S0/1 -N -V -Obathampcorr2
 	mbm_plot -F41 -I sb2112_example_eda2a2.mb41 -G4 \\
 		-S    -N -V -Oampcorr2
 	mbm_plot -F41 -I sb2112_example_eda2a2.mb41 -G5 \\
 		-S    -N -V -Osscorr2

 	bathampcorr1.cmd
 	ampcorr1.cmd
 	sscorr1.cmd
 	bathampcorr2.cmd
 	ampcorr2.cmd
 	sscorr2.cmd

\fBStep 7\fP: Apply filtering to fully corrected sidescan.

Now use \fBmbfilter\fP to try to bring out fine scale features
and differences in seafloor texture from the multibeam
data. In order to avoid biasing the filtered data, we
first use mbcut to remove the nadir region from the 
sidescan swath. This is necessary because the amplitude 
vs angle correction does not entirely clean up the 
specular reflection that dominates the nadir region.
For filtering we apply simple boxcar mean hi-pass and
lo-pass filters to the sidescan. The sizes of the boxcar
filters are specified in terms of across and along-track
numbers of pixels. In this case we use:

     Mean subtraction filter for hipass
       filter acrosstrack dimension: 301
       filter alongtrack dimension:  1
       filter iterations:            1
       filter offset:                10000.000000
     Mean filter for smoothing
       filter acrosstrack dimension: 3
       filter alongtrack dimension:  3
       filter iterations:            1

Since the alongtrack dimension is 1, the sidescan data
in each ping is hipass filtered separately.

 	mbcut -F41 -I sb2112_example_eda2a2.mb41 \\
 		-O sb2112_example_eda2a2c.mb41 \\
 		-A2/1/951/1048 -V
 	mbfilter -F41 -I sb2112_example_eda2a2c.mb41 \\
 		-O sb2112_example_eda2a2cf.mb41 \\
 		-A2 -D1/301/1/1 -S1/3/3/1 -V

Now generate first cut plots of the corrected and 
filtered sidescan:

 	mbm_plot -F41 -I sb2112_example_eda2a2cf.mb41 \\
 		-G5 -S -N -V -Ossfilter

 	ssfilter.cmd

\fBStep 8\fP: Grid the bathymetry.

Now use \fBmbgrid\fP to grid the bathymetry. The greatest
depth in the file is 4502 meters (from the mbinfo output).
The 120 degree swath is 3.4 times the water depth wide, or
15.3 km wide. This translates to an average acrosstrack
spacing of 15300 m / 120 = 127.5 m. If a region of a grid
has more than one data point in each grid cell or bin, we
say that this region is "oversampled". If some bins in a
region have no data points, we say that this region is
"undersampled". We choose a grid cell spacing of 150 m,
which will cause the grid to be oversampled towards
the center of the swath, but undersampled towards the
edges of the swath.

The program \fBmbgrid\fP takes a datalist as input, so we first 
construct that file:

 	echo sb2112_example_eda2a2.mb41 41 > datalist_grid

Now run \fBmbgrid\fP using the gaussian weighted mean algorithm
(-F1) and longitude and latitude grid cell spacings of
150 m (-E150/150/m) to grid bathymetry (-A1). We use 
spline interpolation to fill in small gaps in the data
no larger than two grid cells (-C2), and we set regions
with no data to Nan values for compatibility with
GMT programs (-N). We also specify -M so that grids of
data density and data standard deviation will be
generated:

 	mbgrid -Idatalist_grid -E150/150/m \\
 		-R114.2208/114.4209/-31.9001/-31.6377 \\
 		-Osb2112_example_bath -A1 -N -C2 -M -V

The program \fBmbgrid\fP creates a shellscript which, when
executed, will generate a color fill plot overlaid with
contours of the gridded bathymetry. Now run that shellscript:

 	grd_sb2112_example_bath.cmd

Shellscripts have also been created to generate plots of
the data density and standard deviation grids:

 	grd_sb2112_example_bath_num.cmd
 	grd_sb2112_example_bath_sd.cmd

\fBStep 9\fP: Mosaic the corrected sidescan data.

The program \fBmbmosaic\fP operates in a fashion similar to
\fBmbgrid\fP, but has special capabilities allowing users
to prioritize which parts of the swath are used in the
mosaic that are useful for sidescan data. In particular,
it is possible to create mosaics which do not use the
nadir region of the swath except where no other data is
available, or to create mosaics of sidescan data with
particular look azimuths. The sidescan data has a higher
resolution than the bathymetry so a smaller grid size
can be appropriate. However, we wish to overlay the 
sidescan on the bathymetry, so we use the same grid
cell spacing of 150 m.

In order to give the nadir region a lower priority than
the outer parts of the sidescan swath, create a file
of data priorities (priorities range from 0 to 1) as
a function of apparent grazing angle (negative angles
are on the port side of the swath, zero is at nadir,
and positive angles are on the starboard side of the
swath):

 	-60.0 0.2
 	-45.0 1.0
 	-15.0 0.8
 	-14.9 0.1
 	14.9 0.1
 	15.0 0.8
 	45.0 1.0
 	60.0 0.2

Here the nadir region has been set to a low priority and
the highest priority has been given to data from an
apparent grazing angle of 45 degrees.

Now run \fBmbmosaic\fP to mosaic the bathymetry corrected sidescan
data (-A4):

 	mbmosaic -Idatalist_grid -E150/150/m \\
 		-R114.2208/114.4209/-31.9001/-31.6377 \\
 		-Wangle_priority.dat -F0.10 \\
 		-Osb2112_example_ss -A4 -N -C2 -M -V

Now run the plot shellscripts to view the gridded sidescan
data in grayscale and the ancillary data density and
standard deviation plots:

 	grd_sb2112_example_ss.cmd
 	grd_sb2112_example_ss_num.cmd
 	grd_sb2112_example_ss_sd.cmd

\fBStep 10\fP: Generate additional maps of the gridded data.

First, we use \fBmbm_grdplot\fP to generate a color shaded
relief view of the bathymetry. We choose to illuminate
the bathymetry from the northeast (azimuth of 45 degrees)
and to use a shading magnitude of 0.4 (-A0.4/45).
Because the data has been gridded as bathymetry (positive
down) rather than as topography (positive up), the default
plot will have "hot" colors for deep regions and "cold"
colors for shallow regions; this is the opposite of the
convention we usually use. In order to fix the colors, we have
to either rescale the data by multiplying the bathymetry
by -1 (accomplished with -MGS-1), or flip the color
pallette (accomplished with -D). We use the latter approach.
Finally, because this grid is so small, the default shaded
relief image is likely to be grainy. To fix this problem, we
specify a dots per inch resolution of 72 (-MGQ72); this
will take longer and generate a larger plotfile, but the
plot will look better. We also use the -L option to specify
the title and color scale label for the plot. Here is the 
command:

 	mbm_grdplot -Igrd_sb2112_example_bath \\
 		-G2 -A0.4/45 -D -MGQ72 -V \\
 		-L"Shaded Relief Bathymetry":"Depth (meters)" \\
 		-Osb2112_example_bathshade

 	sb2112_example_bathshade.cmd

Second, we use \fBmbm_grdplot\fP to generate a color fill
view of the bathymetry overlaid with the gridded sidescan.
The sidescan overlay is specified using the -K option.
We want the colors for the bathymetry to be chosen without
histogram equalization, but we also want histogram
equalization to be applied to the sidescan data used for
shading. To do this, we use -S0/1, where the first number
(0) specifies no histogram equalization of the color
scale and the second number (1) causes histogram 
equalization of the shading sidescan data to be
implemented. In order to maintain the convention that
high sidescan amplitudes are black, we flip both the
color pallette (as in the previous example) and the
shading scale with -D1/1. We could also flip the shading
by specifing a negative shading magnitude (-A-0.4).
In this case, we forgo specifying the image resolution,
resulting in a grainy plot:

 	mbm_grdplot -Igrd_sb2112_example_bath \\
 		-G3 -Kgrd_sb2112_example_ss \\
 		-S0/1 -D1/1 -A0.4 -V \\
 		-L"Bathymetry Overlaid With Sidescan":"Depth (meters)" \\
 		-Osb2112_example_bathss

 	sb2112_example_bathss.cmd

\fBStep 11\fP: Generate 3D perspective views of the gridded data.

Now, generate a 3D perspective view of the gridded
bathymetry with shading through synthetic illumination (-G2)
using the macro \fBmbm_grd3dplot\fP. The grid file is in
bathymetry (positive down) rather in topography (positive
up), so the bathymetry needs to be rescaled by multiplying
by -1 (-MGS-1). We choose an illumination magnitude of 0.4
and an illumination azimuth of 45 degrees (-A0.4/45). We 
also choose a perspective azimuth of 250 degrees and an 
elevation of 30 degrees (-E240/30):

 	mbm_grd3dplot -Igrd_sb2112_example_bath \\
 		-G2 -A0.4/45 -E250/30 -MGS-1 -V \\
 		-Osb2112_example_bath3d

 	sb2112_example_bath3d.cmd

Now, generate a 3D perspective view of the gridded
bathymetry shaded using the gridded sidescan data
(-Kgrd_sb2112_example_ss). We want the sidescan data to
be histogram equalized, so we use -S0/1. We also want
the shading to be more prominent than the default
shading magnitude of 0.2 would produce, so we use -A0.5:

 	mbm_grd3dplot -Igrd_sb2112_example_bath \\
 		-Kgrd_sb2112_example_ss \\
 		-G3 -A0.5 -E250/30 -D0/1 -S0/1 -MGS-1 -V \\
 		-Osb2112_example_bathss3d

 	sb2112_example_bathss3d.cmd

.SH PROCESSING HYDROSWEEP DS DATA

The following data processing stream is recommended for Hydrosweep DS
data. Hydrosweep DS sonars are currently in use on the R/V Maurice
Ewing (operated by the Lamont-Doherty Earth Observatory 
and the R/V Thomas Thompson (operated by the University of
Washington), among other vessels.

Consider a data file "hs_ew9204_134.mb21" containing one day's worth of
Hydrosweep data in the raw "real-time" ascii format (format 21).

\fBStep 1\fP: What's in the data file?

Run \fBmbinfo\fP to obtain statistics about the contents
of the data file:

  	mbinfo -F21 -Ihs_ew9204_134.mb21

\fBStep 2\fP: Copy the data from the raw ascii format 
(format 21) to a binary format (format 24) 
containing the same information.  Data i/o  
for format 24 is 15 times faster than 
format 21.  For example, give the command:

  	mbcopy -F21/24 -Ihs_ew9204_134.mb21 \\
 		-Ohsih_ew9204_134.mb24

\fBStep 3\fP: Obtain a reasonable water velocity profile.  
Use the command:

  	mblevitus -R-140/-9 -Owvel_ew9204
 
to obtain an initial model.  Then use  
\fBmbvelocitytool\fP to interactively model 
the affect of changes in the water velocity 
profile on the bathymetry; the best model
may be output for use with \fBmbbath\fP.  
Velocity profiles from XBT or CTD casts 
may also be viewed in \fBmbvelocitytool\fP  
to help define the profile to be 
used in processing.

\fBStep 4\fP: Construct bathymetry data by full raytracing 
through the water velocity profile. This step
is NECESSARY because the sonar calculates
bathymetry assuming a homogeneous water
sound velocity. Use the command:

 	mbbath -F24 -Ihsnav_ew9204_134.mb24 \\
 		-Wwvel_ew9204 \\
 		-Ohsbat_ew9204_134_b.mb24

\fBStep 5\fP: Apply autoediting to flag the really bad 
bathymetric artifacts as bad. This step
is optional - sometimes it makes the 
interactive editing easier and sometimes
it just makes things worse.  Use the command:

 	mbclean -F24 -Ihsbat_ew9204_134_b.mb24 \\
 		-Ohscln_ew9204_134_bc.mb24 \\
 		-D0.01/0.25 -G0.97/1.03 -C2.5 -V

\fBStep 6\fP: Interactively edit the data using 
\fBmbedit\fP. Start up \fBmbedit\fP and 
load the data file "hscln_ew9204_134_bc.mb24"; 
the output file will be "hscln_ew9204_134_bce.mb24".

\fBStep 7a\fP: Merge the data with good, processed navigation.
If nav_ew9204_134 is a file containing a day's 
worth of navigation in the L-DEO navigation 
format, then give the command:

 	mbmerge -F24 -Ihsih_ew9204_134_bce.mb24 \\
 		-Nnav_ew9204_134 \\
 		-Ohsnav_ew9204_134_bcen.mb24

\fBStep 7b\fP: If separately processed navigation is not
available, edit the existing navigation using
the program \fBmbnavedit\fP. This allows the
user to remove spikes and other problems from
the navigation. Start up \fBmbedit\fP and load the
data file "hscln_ew9204_134_bce.mb24"; the
output file will be "hscln_ew9204_134_bcen.mb24".

\fBStep 8\fP: Display the data as swath contour plots 
or swath color fill/shaded relief plots 
using the macro \fBmbm_plot\fP. See 
the manual page for examples.

\fBStep 9\fP: Grid and display the data.  Supposing    
that the area of interest is in the bounds 
 -140 < longitude < -139 and 
-10 < latitude < -9, that the desired grid 
has a 500 m node spacing, that the gridded data
should be in the form of topography (positive upward),
and that the 
grid should be spline interpolated to fill
in small gaps (2 grid cells wide), try:

 	cat hsnav_ew9204_134_bcen.mb24 24 > data.list
 	mbgrid -R-140/-139/-10/-9 -E500/500/meters \\
 		-A1 -C2 -Idata.list -Oew9204_bath 

Here the first command creates a data list file used
by \fBmbgrid\fP, and the second command runs \fBmbgrid\fP,
generating a file of gridded topography.
This command will produce a shellscript 
grd_ew9204_bath.cmd which will run 
\fBGMT\fP programs to display the data.

\fBStep 10\fP: Display the gridded data in 2D maps using the
macro \fBmbm_grdplot\fP or in 3D perspective
views using the macro \fBmbm_grd3dplot\fP. See 
the appropriate manual pages for examples.

.SH PROCESSING HYDROSWEEP MD DATA

Processing Hydrosweep MD data is similar to processing
Hydrosweep DS data. In particular, recalculating 
bathymetry from the travel times is necessary because
the sonar uses a homogeneous water velocity model.
The difference is simply in the data formats used.
The raw Hydrosweep MD data files (typically named
with a ".R" suffix) contain only the travel times; the
sonar calculated bathymetry is contained in parallel
files (typically named with a ".P" suffix). The 
".R" files are supported as format 101. Format 102
data files contain bathymetry in addition to the travel
times. 

Thus, the first step in processing Hydrosweep MD
data is to copy the data from format 101 to  format 102. 
For example, give the command:

  	mbcopy -F101/102 -Iys9409040607.R \\
 		-Oys9409040607.mb102

Bathymetry equivalent to that generated by the sonar
will automatically be calculated in the copy process
(the data stream includes the mean water velocity used
by the sonar).
Following this step, process the Hydrosweep MD data in
the same fashion as that recommended above for Hydrosweep
DS data.  

Should one wish to export data from format 102 files
back to data files usable by Atlas data processing
software, be sure to run mbcopy with the \fB-N\fP so
that comments are stripped from the data. The \fBMB-System\fP
implementation of format 101 includes comment records
that are not supported by the Atlas software. For example: 

  	mbcopy -F102/101 -Iys9409040607.mb102 \\
 		-Oys9409040607.R -N

.SH PROCESSING OLD WHOI DSL AMS-120 DATA

The WHOI DSL group used to provide processed AMS-120
data as a set of parallel files in a DSL specific format. 
An example of the filenames used is:
.br
 	DSL120.940630_1100.amp.dat 	(sidescan)
 	DSL120.940630_1100.bat.dat 	(bathymetry)
 	DSL120.940630_1100.nav 		(navigation)
.br
The sidescan and bathymetry files contain navigation, but it
is generally poor. The processed navigation resides in the
*.nav file, but it is sampled less frequently than the sonar
pings, and is reported in UTM easting and northing meters.
The \fBmbm_dslnavfix\fP macro is used to translate the
eastings and northings into the geographic coordinates
(longitude and latitude) used by \fBMB-System\fP programs. The
program \fBmbmerge\fP can be used to merge the translated
navigation in with the bathymetry and sidescan.

The parallel files listed above are supported as format 111
by \fBMB-System\fP; a similar single-file format is supported
as format 112. We recommend converting the data to format 112
before proceeding with processing to simplify data management.
However, all relevent \fBMB-System\fP programs will work with format
111 files. In either case, the data is voluminous (~75 MB/hour)
and processing will be slow relative to data from multibeam
sonars.

We recommend the following initial procedure when
processing old WHOI DSL AMS-120 data with \fBMB-System\fP:

\fBStep 1\fP: Concatenate all of the processed navigation
files into a single file.

 	cat *.nav > cruise.rnav

\fBStep 2\fP: Translate the DSL UTM navigation into geographic
coordinates. The input navigation is in the "old" navigation
format, as specified by \fB-F\fP\fI1\fP. 
Note that you must know the UTM zone
used - here we assume the zone is +27.

 	mbm_dslnavfix -Icruise.rnav \\
 		-Ocruise.mrg2 -F1 -J27 -V

\fBStep 3\fP: Copy the bathymetry and sidescan data in each
file from format 111 to format 112. Note that it does not
matter which of the two parallel files you specify for
format 111, as long as they end in ".bat.dat" and ".amp.dat".

 	mbcopy -F111/112 \\
 		-IDSL120.940630_1100.bat.dat \\
 		-ODSL120.940630_1100.mb112

\fBStep 4\fP: Merge the translated navigation with each of the 
format 112 data files. Note that \fBmbm_dslnavfix\fP
produces navigation in \fBmbmerge\fP format 2
so that the \fB-M\fP\fI2\fP option is used.

 	mbmerge -F112 \\
 		-IDSL120.940630_1100.mb112 \\
 		-ODSL120.940630_1100_n.mb112 \\
 		-M2 -Ncruise.mrg2 -V

At present, there is no capability for recalculating 
bathymetry using different water sound velocity models
than that used originally. Adding such a capability would
require defining a new data format and seems 
unnecessary for any of the data we have examined.

The large number of bathymetry values (2048) per ping and
the high (> 1/second) ping rate make interactive editing
of the bathymetry extremely time consuming. It is 
probably more realistic to use auto-editing (e.g. \fBmbclean\fP)
to clean up the worst artifacts. One may also find it
necessary to use median filtering rather than weighted
mean filtering in the gridding process (\fBmbgrid\fP).
Other than the above, the generation of grids and maps 
can proceed in the same
fashion as outlined above for SeaBeam 2112 data.

.SH PROCESSING NEW WHOI DSL AMS-120 DATA

The WHOI DSL group now provides processed AMS-120
data in a data format defined by Ocean Imaging Consultants. 
\fBMB-System\fP supports the OIC data as \fBMBIO\fP format 141.
An example of a typical filename is:
.br
 	DSL120.98.227.2020.tmp.oicp
.br
The OIC format files generally contain no navigation (the 
position values are zero).
The processed navigation resides in a separate file,
but it is sampled less frequently than the sonar
pings, and is reported in UTM easting and northing meters.
The \fBmbm_dslnavfix\fP macro is used to translate the
eastings and northings into the geographic coordinates
(longitude and latitude) used by \fBMB-System\fP programs. The
program \fBmbmerge\fP can be used to merge the translated
navigation in with the bathymetry and sidescan.

\fBMB-System\fP also supports an "enhanced" data format (142) 
similar to the OIC format. Format 142 adds a beam flag 
array to the data, allowing bathymetry editing.
Since interferometry derived bathymetry tends to be noisy and
to require editing, we recommend converting the AMS-120 
data to format 142 before proceeding with processing.
In either case, the data is voluminous (~75 MB/hour)
and processing will be slow relative to data from multibeam
sonars.

We recommend the following initial procedure when
processing new WHOI DSL AMS-120 data with \fBMB-System\fP:

\fBStep 1\fP: Concatenate all of the processed navigation
files into a single file.

 	cat *.nav > cruise.rnav

\fBStep 2\fP: Translate the DSL UTM navigation into geographic
coordinates. The input navigation is in the "new" navigation
format, as specified using \fB-F\fP\fI2\fP. 
Note that you must know the UTM zone
used - here we assume the zone is +9. 

 	mbm_dslnavfix -F2 -Icruise.rnav \\
 		-Ocruise.mrg2 -J9 -V

\fBStep 3\fP: Copy the bathymetry and sidescan data in each
file from format 141 to format 142.

 	mbcopy -F141/142 \\
 		-IDSL120.98.227.2020.tmp.oicp \\
 		-ODSL120.98.227.2020.mb142

\fBStep 4\fP: Merge the translated navigation with each of the 
format 112 data files. Note that \fBmbm_dslnavfix\fP
produces navigation in \fBmbmerge\fP format 2
so that the \fB-M\fP\fI2\fP option is used.

 	mbmerge -F112 \\
 		-IDSL120.98.227.2020.mb112 \\
 		-ODSL120.98.227.2020_n.mb112 \\
 		-M2 -Ncruise.mrg2 -V

The large number of bathymetry values (2048) per ping and
the high (> 1/second) ping rate make interactive editing
of the bathymetry extremely time consuming. It is 
probably more realistic to use auto-editing (e.g. \fBmbclean\fP)
to clean up the worst artifacts. One may also find it
necessary to use median filtering rather than weighted
mean filtering in the gridding process (\fBmbgrid\fP).
Other than the above, the generation of grids and maps 
can proceed in the same
fashion as outlined above for SeaBeam 2112 data.

.SH SEE ALSO
\fBmbio\fP(l)

.SH BUGS
It doesn't do everything we want it to yet, it doesn't work with
every kind of swath data ever collected, and sometimes it
breaks.
