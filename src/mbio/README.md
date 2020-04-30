## MB-System Source Directory: src/mbio/

This directory contains the C source files for the libmbio library that MB-System uses to read and write seafloor mapping data in the many supported formats.

This document first covers the basic use of the libmbio API and then discusses structure of the source files included in this directory.

A document describing libmbio with a focus on how to write a new i/o module can be found at:
[https://www.mbari.org/wp-content/uploads/2016/03/codinganmb-systemiomodulev4.pdf](https://www.mbari.org/wp-content/uploads/2016/03/codinganmb-systemiomodulev4.pdf)

These source files are copyright by David Caress and Dale Chayes and licensed
using GPL3 as part of MB-System.

-
### Table of Contents
-

1. MBIO Description
	1. Data Terminology
	1. API Overview
		1. Level 1: Simple Reading
		1. Level 2: Complete Reading and Writing 
		1. Level 3: Buffered Reading and Writing
		1. Use of the Different I/O Levels
	1. Header Files
	2. I/O Modules
	2. Function Status And Error Codes
	1. Function Verbosity
1. Initialization And Closing Functions
	1. mb\_read\_init()
	1. mb\_write\_init()
	1. mb\_register\_array()
	1. mb\_close()
	1. Level 1 Functions
	1. mb\_read()
	1. mb\_get()
1. Level 2 Functions
	1. mb\_read\_ping()
	1. mb\_write\_ping()
	1. mb\_get\_store()
	1. mb\_get\_all()
	1. mb\_put\_all()
	1. mb\_put\_comment()
	1. mb\_extract()
	1. mb\_insert()
	1. mb\_extract\_nav()
	1. mb\_insert\_nav()
	1. mb\_extract\_altitude()
	1. mb\_insert\_altitude()
	1. mb\_extract\_svp()
	1. mb\_insert\_svp()
	1. mb\_ttimes()
	1. mb\_detects()
	1. mb\_gains()
	1. mb\_extract\_rawss()
	1. mb\_insert\_rawss()
	1. mb\_copyrecord()
1. Level 3 Functions
	1. mb\_buffer\_init()
	1. mb\_buffer\_close()
	1. mb\_buffer\_load()
	1. mb\_buffer\_dump()
	1. mb\_buffer\_clear()
	1. mb\_buffer\_info()
	1. mb\_buffer\_get\_next\_data()
	1. mb\_buffer\_extract()
	1. mb\_buffer\_insert()
	1. mb\_buffer\_get\_next\_nav()
	1. mb\_buffer\_extract\_nav()
	1. mb\_buffer\_insert\_nav()
	1. mb\_buffer\_get\_ptr()
1. Miscellaneous Functions
	1. mb\_defaults()
	1. mb\_env()
	1. mb\_format()
	1. mb\_format\_register()
	1. mb\_format\_info()
	1. mb\_format\_system()
	1. mb\_format\_description()
	1. mb\_format\_dimensions()
	1. mb\_format\_flags()
	1. mb\_format\_source()
	1. mb\_format\_beamwidth()
	1. mb\_datalist\_open()
	1. mb\_datalist\_read()
	1. mb\_datalist\_close()
	1. mb\_alloc()
	1. mb\_deall()
	1. mb\_error()
	1. mb\_navint\_add()
	1. mb\_navint\_interp()
	1. mb\_attint\_add()
	1. mb\_attint\_interp()
	1. mb\_hedint\_add()
	1. mb\_hedint\_interp()
	1. mb\_get\_double()
	1. mb\_get\_int()
	1. mb\_get\_binary\_short()
	1. mb\_get\_binary\_int()
	1. mb\_get\_binary\_float()
	1. mb\_get\_binary\_double()
	1. mb\_put\_binary\_short()
	1. mb\_put\_binary\_int()
	1. mb\_put\_binary\_float()
	1. mb\_put\_binary\_double()
	1. mb\_get\_bounds()
	1. mb\_ddmmss\_to\_degree()
	1. mb\_takeoff\_to\_rollpitch()
	1. mb\_rollpitch\_to\_takeoff()
	1. mb\_double\_compare()
	1. mb\_int\_compare()
1. Coordinate Systems Used In MB-System
	1. Introduction
	1. Cartesian Coordinates
	1. Spherical Coordinates
		1. Takeoff Angle Coordinates
		1. Roll-Pitch Coordinates
	1. SeaBeam Coordinates
	1. Usage of Coordinate Systems in MB-System
1. Beam Flags Used In MB-System


--
### MBIO Description
--
MBIO (**M**ulti**B**eam **I**nput/**O**utput) is a library
of functions used for reading and writing swath seafloor mapping
data files. This library enables processing and display
programs that are independent of particular data formats and provides
a standard approach to seafloor mapping data i/o.MBIO supports a large number of data
formats associated with different institutions and different mapping systems, including multibean sonars, sidescan sonars, interferometric sonars, lidar, and seafloor photography.
MB-System supports these  heterogenous data types by layering a common application programming interface (API) on top of the I/O modules that read and write sonar data in the existing data formats. The MBIO API consists of high level functions that allow applications to open data streams for reading or writing, read and write data records sequentially, to straighforwardly extract and insert the commonly used values, and to expose the complete data representation to access. Although MB-System as a whole includes C, C++, Python, and perl source code, the MBIO library is entirely written in C. 

### Data Terminology

MBIO handles three types of swath mapping data:  beam bathymetry,
beam amplitude, and sidescan.  Both amplitude and sidescan represent
measures of backscatter strength. Beam amplitudes are backscatter
values associated with the same preformed beams used to
obtain bathymetry; MBIO assumes that a bathymetry value
exists for each amplitude value and uses the bathymetry beam
location for the amplitude.  Sidescan is generally constructed with
a higher spatial resolution than bathymetry, and carries its
own location parameters.  In the context of MB-System documentation,
the discrete values of bathymetry and amplitude are referred
to as "beams", and the discrete values of sidescan are referred to
as "pixels". An additional difference between "beam" and "pixel"
data involves data flagging. An array of "beamflags" is carried by
MBIO functions which allows the bathymetry (and by extension
the amplitude) data to be flagged as bad. The details of the
beamflagging scheme are presented below.
 
### API Overview

MBIO opens and initializes sonar data files for reading
and writing using the functions mb\_read\_init() and mb\_write\_init(),
respectively.  These functions return a pointer to a data structure
including all relevant information about the opened file, the
control parameters which determine how data is read or written,
and the arrays used for processing the data as it is read or written.
This pointer is then passed to the functions used for reading or
writing.  There is no limit on the number of files which may be
opened for reading or writing at any given time in a program.


The mb\_read\_init() and mb\_write\_init() functions also return
initial maximum numbers of bathymetry beams, amplitude beams,
and sidescan pixels that can be used to allocate data storage
arrays of the appropriate sizes. However, for some data formats
there are no specified maximum numbers of beams and pixels, and
so in general the required dimensions may increase as data are
read. Applications must pass appropriately dimensioned arrays
into data extraction routines such as mb\_read(), mb\_get(),
and mb\_get\_all(). In order to enable dynamic memory management
of thse application arrays, the application must first register
each array by passing the array pointer location to the function
mb\_register\_array().

Data files are closed using the function mb\_close(). All
internal and registered arrays are deallocated as part of closing
the file.

When it comes to actually reading and writing swath mapping
sonar data, MBIO has three levels of i/o functionality:

#### Level 1: Simple Reading

The primary functions used to read and return the most
basic and generic information from swath data files are:

* mb\_read()
* mb\_get()
	
The positions of individual beams and pixels are
returned in longitude and latitude by
mb\_read() and in acrosstrack and alongtrack
distances by mb\_get(). Only a limited set
of navigation information is returned. Comments
are also returned. These functions can be used
without any special include  files or any
knowledge of the actual data structures used
by the data formats or MBIO.


#### Level 2: Complete Reading and Writing 

These functions allow for both reading and writing of data structures
containing all of the available information.
Data records may be read or written without
extracting any of the information, or the
swath data may be passed with the data structure.
Several functions exist to extract information
from or insert information into the data
structures; otherwise, special include files
are required to make sense of the sonar-specific
data structures passed by level 2 i/o functions.
The basic read and write functions that only pass
pointers to internal data structures are:

* mb\_read\_ping()
* mb\_write\_ping()

The read and write routines which also extract
or insert information in form of passed variables are:

* mb\_get\_all()
* mb\_put\_all()
* mb\_put\_comment()

The information extraction and insertion
functions are:

* mb\_insert()
* mb\_extract()
* mb\_extract\_nav()
* mb\_insert\_nav()
* mb\_extract\_altitude()
* mb\_insert\_altitude()
* mb\_ttimes()
* mb\_copyrecord()


#### Level 3: Buffered Reading and Writing

These functions do buffered reading and writing of data structures
containing all of the available information. This allows programs
to read and write large numbers of data records in groups, allowing
for processing operations requiring that many data records be 
available in memory simultaneously. 

The primary functions are:

* mb\_buffer\_init()
* mb\_buffer\_close()
* mb\_buffer\_load()
* mb\_buffer\_dump()
* mb\_buffer\_info()
* mb\_buffer\_get\_next\_data()
* mb\_buffer\_extract()
* mb\_buffer\_insert()
* mb\_buffer\_get\_next\_nav()
* mb\_buffer\_extract\_nav()
* mb\_buffer\_insert\_nav()

#### Use of the Different I/O Levels

The level 1 MBIO functions allow users to read
sonar data independent of format, with the limitation that
only a limited set of navigation information is passed.  Thus, some of the
information contained in certain data formats (e.g. the "heave" value
in Hydrosweep DS data) is not passed by mb\_read() or mb\_get().
In general, the level 1 functions are useful for applications
such as graphics which require only the navigation and the depth
and/or backscatter values.

The level 2 functions (mb\_get\_all() and
mb\_put\_all()) read and write the complete data
structures, translate the data to internal data structures
associated with each of the supported sonar systems, and
pass pointers to these internal data structures. Additional
functions allow a variety of information to be extracted
from or inserted into the data structures (e.g. mb\_extract()
and mb\_insert()). Additional information may be accessed
using special include files to  decode the data structures.
The great majority of processing programs use level 2 functions.

The level 3 functions provide buffered reading and
writing which is useful for applications that generate
output files and need access to multiple pings at a time.  In addition to reading
(mb\_buffer\_load()) and writing (mb\_buffer\_dump()), functions
exist for extracting information from the buffer (mb\_buffer\_extract())
and inserting information into the buffer (mb\_buffer\_insert()).

MBIO supports swath data in a number of different formats,
each specified by a unique id number. The function mb\_format()
determines if a format id is valid. A set of similar functions
returns information about the specified format
(e.g. mb\_format\_description(), mb\_format\_system(),
mb\_format\_description(), mb\_format\_dimensions(),
mb\_format\_flags(), mb\_format\_source(),
mb\_format\_beamwidth()).

Some MB-System programs can process multiple data
files specified in "datalist" files. Each line of a datalist
file contains a file path and the corresponding MBIO
format id. Datalist files can be recursive and can contain
comments. The functions used to extract input swath data
file paths from datalist files
includes mb\_datalist\_open(), mb\_datalist\_read(),
and mb\_datalist\_close().

A number of other MBIO functions dealing with default values for
important parameters, error messages, memory management, and time conversions
also exist and are discussed below.

### Header Files
There are several header files that are included by C or C++ programs calling libmbio. The header files made publicly available in an MB-System installation include:

* mb\_config.h - Configuration file generated by the MB-System Autotools based build system. This is included by mb\_define.h
* mb\_define.h - Defines many preprocessor macros used throughout MB-System source files, including key numeric parameters, and also includes the prototypes for all public libmbio functions. This is included by mb\_io.h.
* mb\_status.h - Defines status values, error values, and error messages used or returned by libmbio functions. This is included by mb\_io.h.
* mb\_process.h - Defines the mb\_preprocess\_struct and mb\_process\_struct structures that hold information defining the actions taken by programs mb\_preprocess and mb\_process, respectively, when processing a swath data file.
* mb\_io.h - Defines the mb_io_struct structure that holds all information involved in reading from or writing to a swath data file using libmbio. This structure includes storage of function pointers for the standard i/o functions associated with the data format being read or written. This header file includes mb\_define.h, mb\_status.h, and mb\_process.h.
* mb\_swap.h - Defines macros used to apply byte swapping when reading or writing files that use a different byte order than the current system.
* mb\_format.h - Defines the lists and id numbers of supported data formats and format families, and includes prototypes for functions that register format modules and return basic information about the registered modules. This header file is included by mb\_io.h.
* mb\_info.h - Defines structures and function prototypes used to store geographic bounds information from metadata (*.inf) files associated with swath data files. This header file is used by programs like mbgrid, mbmosaic, mbeditviz, mbareaclean, and mbvoxelclean.
* mb\_segy.h - Defines the structures used to store time series data (usually seismic reflection) from files in the SEGY data format.

There are also header files associated with the many supported data formats and the i/o modules used to read and write those formats. For most purposes, programs using libmbio access swath data through the functions described above, and do not directly access data through the structures used to store data in memory by i/o modules. The structure of the i/o modules is described below.

* mb\_absorption.c
* mb\_access.c
* mb\_angle.c
* mb\_buffer.c
* mb\_check\_info.c
* mb\_close.c
* mb\_compare.c
* mb\_coor\_scale.c
* mb\_defaults.c
* mb\_error.c
* mb\_esf.c
* mb\_fileio.c
* mb\_format.c
* mb\_get.c
* mb\_get\_all.c
* mb\_get\_value.c
* mb\_mem.c
* mb\_navint.c
* mb\_platform.c
* mb\_platform\_math.c
* mb\_process.c
* mb\_proj.c
* mb\_put\_all.c
* mb\_put\_comment.c
* mb\_read.c
* mb\_read\_init.c
* mb\_read\_ping.c
* mb\_rt.c
* mb\_segy.c
* mb\_spline.c
* mb\_swap.c
* mb\_time.c
* mb\_write\_init.c
* mb\_write\_ping.c

mb\_config.h
mb\_define.h
mb\_format.h
mb\_info.h
mb\_io.h
mb\_process.h
mb\_segy.h
mb\_status.h
mb\_swap.h


mbr\_3ddepthp.c

mbr\_3dwisslp.c

mbr\_3dwisslr.c

mbr\_asciixyz.c

mbr\_bchrtunb.c
mbf\_bchrtunb.h

mbr\_bchrxunb.c
mbf\_bchrxunb.h

mbr\_cbat8101.c
mbf\_cbat8101.h

mbr\_cbat9001.c
mbf\_cbat9001.h

mbr\_dsl120pf.c
mbf\_dsl120pf.h

mbr\_dsl120sf.c
mbf\_dsl120sf.h

mbr\_edgjstar.c

mbr\_elmk2unb.c
mbf\_elmk2unb.h

mbr\_em12darw.c
mbf\_em12darw.h

mbr\_em12ifrm.c
mbf\_em12ifrm.h

mbr\_em300mba.c

mbr\_em300raw.c

mbr\_em710mba.c

mbr\_em710raw.c

mbr\_emoldraw.c

mbr\_gsfgenmb.c
mbf\_gsfgenmb.h

mbr\_hir2rnav.c

mbr\_hs10jams.c

mbr\_hsatlraw.c
mbf\_hsatlraw.h

mbr\_hsds2lam.c

mbr\_hsds2raw.c

mbr\_hsldedmb.c
mbf\_hsldedmb.h

mbr\_hsldeoih.c
mbf\_hsldeoih.h

mbr\_hsmdaraw.c
mbf\_hsmdaraw.h

mbr\_hsmdldih.c
mbf\_hsmdldih.h

mbr\_hsunknwn.c

mbr\_hsuricen.c
mbf\_hsuricen.h

mbr\_hsurivax.c

mbr\_hydrob93.c

mbr\_hypc8101.c
mbf\_hypc8101.h

mbr\_hysweep1.c

mbr\_image83p.c

mbr\_imagemba.c

mbr\_kemkmall.c

mbr\_l3xseraw.c

mbr\_mbarimb1.c

mbr\_mbarirov.c
mbf\_mbarirov.h

mbr\_mbarrov2.c
mbf\_mbarrov2.h

mbr\_mbldeoih.c

mbr\_mbnetcdf.c

mbr\_mbpronav.c
mbf\_mbpronav.h

mbr\_mgd77dat.c
mbf_mgd77dat.h

mbr_mgd77tab.c

mbr_mgd77txt.c

mbr_mr1aldeo.c
mbf_mr1aldeo.h

mbr_mr1bldeo.c
mbf_mr1bldeo.h

mbr_mr1prhig.c
mbf_mr1prhig.h

mbr_mr1prvr2.c

mbr_mstiffss.c
mbf_mstiffss.h

mbr_nvnetcdf.c

mbr_oicgeoda.c
mbf_oicgeoda.h

mbr_oicmbari.c
mbf_oicmbari.h

mbr_omghdcsj.c
mbf_omghdcsj.h

mbr_photgram.c

mbr_reson7k3.c

mbr_reson7kr.c

mbr_samesurf.c

mbr_segysegy.c

mbr_swplssxi.c

mbr_swplssxp.c

mbr_tempform.c

mbr_wasspenl.c

mbr_xtfb1624.c

mbr_xtfr8101.c
mbf_xtfr8101.h




mbsys_3datdepthlidar.c
mbsys_3datdepthlidar.h

mbsys_3ddwissl.c
mbsys_3ddwissl.h

mbsys_atlas.c
mbsys_atlas.h

mbsys_benthos.c
mbsys_benthos.h

mbsys_dsl.c
mbsys_dsl.h

mbsys_elac.c
mbsys_elac.h

mbsys_elacmk2.c
mbsys_elacmk2.h

mbsys_gsf.c
mbsys_gsf.h

mbsys_hdcs.c
mbsys_hdcs.h

mbsys_hs10.c
mbsys_hs10.h

mbsys_hsds.c
mbsys_hsds.h

mbsys_hsmd.c
mbsys_hsmd.h

mbsys_hysweep.c
mbsys_hysweep.h

mbsys_image83p.c
mbsys_image83p.h

mbsys_jstar.c
mbsys_jstar.h
mbsys_jstar2.h

mbsys_kmbes.c
mbsys_kmbes.h

mbsys_ldeoih.c
mbsys_ldeoih.h

mbsys_mr1.c
mbsys_mr1.h

mbsys_mr1b.c
mbsys_mr1b.h

mbsys_mr1v2001.c
mbsys_mr1v2001.h

mbsys_mstiff.c
mbsys_mstiff.h

mbsys_navnetcdf.c
mbsys_navnetcdf.h

mbsys_netcdf.c
mbsys_netcdf.h

mbsys_oic.c
mbsys_oic.h

mbsys_reson.c
mbsys_reson.h

mbsys_reson7k.c
mbsys_reson7k.h

mbsys_reson7k3.c
mbsys_reson7k3.h

mbsys_reson8k.c
mbsys_reson8k.h


* **Data System "SB"**
	* SeaBeam Classic multibeam
		* mbsys_sb.c
		* mbsys_sb.h
	* Format SBSIOMRG 11
		* mbr_sbsiomrg.c
		* mbf_sbsiomrg.h
	* Format SBSIOCEN 12
		* mbr_sbsiocen.c
		* mbf_sbsiocen.h
	* Format SBSIOLSI 13
		* mbr_sbsiolsi.c
		* mbf_sbsiolsi.h
	* Format SBURICEN 14
		* mbr_sburicen.c
		* mbf_sburicen.h
	* Format SBURIVAX 15
		* mbr_sburivax.c
	* Format SBSIOSWB 16
		* mbr_sbsioswb.c
		* mbf_sbsioswb.h
	* Format SBIFREMR 17
		* mbr_sbifremr.c
		* mbf_sbifremr.h
		
* **Data System "HSDS"**
	* Atlas Hydrosweep DS multibeam
		* mbsys_hsds.c
		* mbsys_hsds.h

mbr_sb2000sb.c

mbr_sb2000ss.c

mbr_sb2100bi.c

mbr_sb2100rw.c
mbf_sb2100rw.h

mbf_sb2120xs.h








mbsys_sb2000.c
mbsys_sb2000.h

mbsys_sb2100.c
mbsys_sb2100.h

mbsys_simrad.c
mbsys_simrad.h

mbsys_simrad2.c
mbsys_simrad2.h

mbsys_simrad3.c
mbsys_simrad3.h

mbsys_singlebeam.c
mbsys_singlebeam.h

mbsys_stereopair.c
mbsys_stereopair.h

mbsys_surf.c
mbsys_surf.h

mbsys_swathplus.c
mbsys_swathplus.h

mbsys_templatesystem.c
mbsys_templatesystem.h

mbsys_wassp.c
mbsys_wassp.h

mbsys_xse.c
mbsys_xse.h

projections.h

### I/O Modules
In order to support as broad a range of mapping data as possible without losing information, and to allow for advances in the remote sensing technology we use, we have architected MB-System to read and write data in the existing formats and to store those data internally with all information preserved. The consequence is that the MB-System input and output capability is isolated to this modular library (MBIO) supporting dozens of different seafloor mapping data formats. Each unique format is associated with an integer identifier number and with functions that read the data into an internal representation, or data system, and write from that representation. A second level of modularity includes the many different data systems that are supported; each data system is defined by a structure used to store the data and a set of functions that extract commonly used values from, or insert values into, that structure.  

In the terminology of this document, each MBIO I/O module consists of a single data system and at least one data format. Each data system includes both a structure to store the data and functions that map commonly used values to and from that structure. The data format includes functions that read and write the data to and from the data structure of the associated data system. The data systems are defined by two source files each: a header file with a name of the form mbsys\_XXX.h that defines the structure used to store the data and a C file with a name of the form mbsys\_XXX.c that includes all of the functions used to extract information from or insert information into the storage structure. Here "XXX" is the data system name, which may be any number of characters. The data formats are defined in C files with names of the form mbr_YYYYYYYY.c that include the functions to read and write data files in that format, translating the data into and out of the representation structure in mbsys\_XXX.h. Here "YYYYYYYY" is the data format name, which is always eight characters long.

As an example, the Kongsberg third generation multibeam data format is supported with two data formats, the raw or vendor format logged by the sonars, and an extended format defined for MB-System processing. The data system name is "simrad3", and the format names and numeric id's are "em710raw" and 58 for the vendor format and "em710mba" and 59 for the extended format. The source files associated with these two formats are:

* mbsys_simrad3.h
* mbsys_simrad3.c
* mbr_em710raw.c
* mbr_em710mba.c

In some cases, data formats include structure definitions used while translating data from the files to and from the data system representation; these structures are found in files with names of the form mbf_YYYYYYYY.h.

Several data formats depend on lower level libraries such as XDR or NetCDF for i/o. Others depend on high level libraries such Generic Sensor Format (GSF) or BSIO that are included in separate source directories within the MB-System package. 

### Function Status And Error Codes
All of the MBIO functions return an integer status value with the  convention that:

* status = 1:    success
* status = 0:    failure

All  MBIO functions also pass an error value argument which gives some-
what more information about problems than the status value.   The  full
suite of possible error values and the associated error messages are:

* error = 0:     "No error",
* error = -1:    "Time gap in data",
* error = -2:    "Data outside specified location bounds",
* error = -3:    "Data outside specified time interval",
* error = -4:    "Ship speed too small",
* error = -5:    "Comment record",
* error = -6:    "Neither a data record nor a comment record",
* error = -7:    "Unintelligible data record",
* error = -8:    "Ignore this data",
* error = -9:    "No data requested for buffer load",
* error = -10:   "Data buffer is full",
* error = -11:   "No data was loaded into the buffer",
* error = -12:   "Data buffer is empty",
* error = -13:   "No data was dumped from the buffer"
* error = -14:   "No more survey data records in buffer"
* error = -15:   "Data inconsistencies prevented inserting data into storage structure"
* error = 1:     "Unable to allocate memory, initialization failed",
* error = 2:     "Unable to open file, initialization failed",
* error = 3:     "Illegal format identifier, initialization failed",
* error = 4:     "Read error, probably end-of-file",
* error = 5:     "Write error",
* error = 6:     "No data in specified location bounds",
* error = 7:     "No data in specified time interval",
* error = 8:     "Invalid MBIO descriptor",
* error = 9:     "Inconsistent usage of MBIO descriptor",
* error = 10:    "No pings binned but no fatal error - this should not happen!",
* error = 11:    "Invalid data record type specified for writing",
* error = 12:    "Invalid control parameter specified by user",
* error = 13:    "Invalid buffer id",
* error = 14:    "Invalid system id - this should not happen!"
* error = 15:    "This data file is not in the specified format!"

In general, programs should treat negative error values as non-fatal
(reading and writing can continue) and positive error values as fatal
(the data files should be closed and the program terminated).

### Function Verbosity
All of the MBIO functions are passed a verbose parameter which controls
how much debugging information is output to standard error. If verbose
is 0 or 1, the MBIO functions will be silent. If verbose is 2, then
each function will output information as it is entered and as it
returns,  along  with the parameter values passed into and returned out
of the function.  Greater values of verbose will cause additional
information to be output, including values at various stages of data
processing during read and write operations.  In general, programs
using MBIO functions should adopt the following verbosity conventions:

* verbose = 0: "silent" or near-"silent" execution
* verbose = 1: simple output including program name, version and simple progress updates
* verbose >= 2:  debug mode with copious output including every function call and status listings

### Initialization And Closing Functions

#### mb\_read\_init()

	int mb_read_init(
                 int verbose,
                 char *file,
                 int format,
                 int pings,
                 int lonflip,
                 double bounds[4],
                 int btime_i[7],
                 int etime_i[7],
                 double speedmin,
                 double timegap,
                 char **mbio_ptr,
                 double *btime_d,
                 double *etime_d,
                 int *beams_bath,
                 int *beams_amp,
                 int *pixels_ss,
                 int *error);

The  function mb\_read\_init initializes the data file to be read and the
data structures required for reading the data. The verbose value controls the standard error output verbosity of the function.

The input control parameters have the following significance:

* file:          input filename
* format:        input MBIO data format id
* pings:         ping averaging
* lonflip:       longitude flipping
* bounds:        location bounds of acceptable data
* btime\_i:       beginning time of acceptable data
* etime\_i:       ending time of acceptable data
* speedmin:      minimum ship speed of acceptable data
* timegap:       maximum time allowed before data gap

The format identifier format specifies which of the supported data for-
mats is being read or written;  the  currently  supported  formats  are
listed in the "SUPPORTED FORMATS" section.

The pings parameter determines whether and how pings are averaged as
part of data input.  This parameter is used only by the functions
mb\_read and mb\_get; mb\_get\_all and mb\_buffer\_load do not average pings.
If pings = 1, then no ping averaging will be done and  each  ping  read
will be returned unaltered by the reading function.  If pings > 1, then
the navigation and beam data for pings pings will  be  read,  averaged,
and  returned  as  the  data for a single ping.  If pings = 0, then the
ping averaging will be varied so that the along-track distance  between
averaged  pings is as close as possible to the across-track distance
between beams.

The lonflip paramenter determines the range in which  longitude  values
are returned:

* lonflip = -1 : -360 to   0
* lonflip =  0 : -180 to 180
* lonflip =  1 :    0 to 360

The  bounds  array  sets  the area within which data are desired.  Data
which lie outside the area specified by bounds will be returned with an
error  by  the  reading  function.   The  functions mb\_read, mb\_get and
mb\_get\_all use the bounds array; the function  mb\_buffer\_load  does  no
location checking.

* bounds[0] : minimum longitude
* bounds[1] : maximum longitude
* bounds[2] : minimum latitude
* bounds[3] : maximum latitude

The  btime\_i array sets the desired beginning time for the data and the
etime\_i array sets the desired ending time.  If the beginning  time  is
earlier  than  the  ending time, then any data with a time stamp before
the beginning time or after the ending time will be  returned  with  an
MB\_ERROR\_OUT\_TIME error by the reading function.  If the beginning time
is after the ending time, then data with time stamps between the ending
and  beginning time are returned with an error. This scheme allows time
windowing outside  or  inside  a  specified  interval.   The  functions
mb\_read,  mb\_get and mb\_get\_all use the btime\_i and btime\_i arrays; the
function mb\_buffer\_load does no time checking.

* btime[0] : year
* btime[1] : month
* btime[2] : day
* btime[3] : hour
* btime[4] : minute
* btime[5] : second
* btime[6] : microsecond
* etime[0] : year
* etime[1] : month
* etime[2] : day
* etime[3] : hour
* etime[4] : minute
* etime[5] : second
* etime[6] : microsecond

The speedmin parameter sets the minimum acceptable ship speed  for  the
data.   If  the ship speed associated with any ping is less than speed-
min, then that data will be returned with an error by the reading func-
tion.  This is used to eliminate data collected while a ship is on sta-
tion is a simple way. The functions mb\_read, mb\_get and mb\_get\_all  use
the speedmin value; the function mb\_buffer\_load does no speed checking.

The timegap parameter sets the minimum time gap allowed before a gap in
the  data is declared.  Ping averaging is not done across data gaps; an
error is returned  when  time  gaps  are  encountered.   The  functions
mb\_read  and mb\_get use the timegap value; the functions mb\_get\_all and
mb\_buffer\_load do no ping averaging and thus have no need to check  for
time gaps.

The returned values are:

* mbio\_ptr: pointer to an MBIO descriptor structure
* btime\_d:       desired beginning time in seconds
*                     since 1/1/70 00:00:0
* etime\_d:       desired ending time in seconds
*                     since 1/1/70 00:00:0
* beams\_bath:    maximum number of bathymetry beams
* beams\_amp:     maximum number of amplitude beams
* pixels\_ss:     maximum number of sidescan pixels
* error:         error value

The  structure pointed to by mbio\_ptr holds the file descriptor and all
of the control parameters which govern  how  the  data  is  read;  this
pointer  must be provided to the functions mb\_read, mb\_get, mb\_get\_all,
or mb\_buffer\_load to read data. The values beams\_bath,  beams\_amp,  and
pixels\_ss  return initial estimates of the maximum number of bathymetry
and amplitude beams and sidescan pixels, respectively, that the  speci-
fied data format may contain. In general, beams\_amp will either be zero
or equal to beams\_bath. The values btime\_d and etime\_d give the desired
beginning and end times of the data converted to seconds since 00:00:00
on January 1, 1970; MBIO uses these units to calculate time internally.

For  most  data  formats, the initial maximum beam and pixel dimensions
will not change.  However, a few  formats  support  both  variable  and
arbitrarily  large  numbers of beams and/or pixels, and so applications
must be capable of handling dynamic changes in the numbers of beams and
pixels.  The  arrays allocated internally in the mbio\_ptr structure are
automatically increased when necessary. However, in order  to  success-
fully   extract  swath  data  using  mb\_get,  mb\_get\_all,  mb\_read,  or
mb\_extract, an application must also provide pointers to  arrays  large
enough  to hold the current maximum numbers of bathymetry beams, ampli-
tude beams,  and  sidescan  pixels.  The  function  mb\_register\_ioarray
allows applications to register array pointers so that these arrays are
also dynamically allocated by MBIO. Registered arrays will  be  managed
as data are read and then freed when mb\_close is called.

A  status  value  indicating  success  or failure is returned; an error
value argument passes more detailed  information  about  initialization
failures.

#### mb\_write\_init()

	int mb_write_init(
         int verbose,
         char *file,
         int format,
         char **mbio_ptr,
         int *beams_bath,
         int *beams_amp,
         int *pixels_ss,
         int *error);

The  function mb\_write\_init initializes the data file to be written and
the data structures required for writing the data.  The  verbose  value
controls the standard error output verbosity of the function.

The input control parameters have the following significance:

* file:   output filename
* format: output MBIO data format id

The returned values are:

* mbio\_ptr: pointer to a structure describing the output file
* beams\_bath:    maximum number of bathymetry beams
* beams\_back:    maximum number of backscatter beams
* error:         error value

The  structure pointed to by mbio\_ptr holds the output file descriptor;
this pointer must  be  provided  to  the  functions  mb\_write,  mb\_put,
mb\_put\_all,  or  mb\_buffer\_dump  to  write data. The values beams\_bath,
beams\_amp, and pixels\_ss return the maximum number  of  bathymetry  and
amplitude  beams  and sidescan pixels, respectively, that the specified
data format may contain.  In general, beams\_amp will either be zero  or
equal  to beams\_bath.  In order to successfully write data, the calling
program must provide pointers to arrays large enough to hold beams\_bath
bathymetry  values,  beams\_amp amplitude values, and pixels\_ss sidescan
values.

For most data formats, the initial maximum beam  and  pixel  dimensions
will  not  change.   However,  a  few formats support both variable and
arbitrarily large numbers of beams and/or pixels, and  so  applications
must be capable of handling dynamic changes in the numbers of beams and
pixels. The arrays allocated internally in the mbio\_ptr  structure  are
automatically  increased  when necessary. However, in order to successfully 
insert  modified  swath  data  using  mb\_put,   mb\_put\_all,   or
mb\_insert,  an  application  must also provide pointers to arrays large
enough to hold the current maximum numbers of bathymetry beams, amplitude 
beams,  and  sidescan  pixels.  The  function mb\_register\_ioarray
allows applications to register array pointers so that these arrays are
also  dynamically  allocated by MBIO. Registered arrays will be managed
as data are read and written and then freed when mb\_close is called.

A status value indicating success or failure is returned; an error
value argument passes more detailed information about initialization
failures.

#### mb\_register\_array()

	int mb_register_array(
         int verbose,
         void *mbio_ptr,
         int type,
         int size,
         void **handle,
         int *error)

Registers an array pointer \*handle so that the size  of  the  allocated
array  can  be  managed  dynamically  by  MBIO.  Note that the location
\*\*handle of the array pointer must be supplied, not the  pointer  value
\*handle.  The pointer value *handle should initially be NULL.  The type
value indicates whether this array is to be  dimensioned  according  to
the  maximum  number  of  bathymetry  beams (type = 1), amplitude beams
(type = 2), or sidescan pixels (type = 3). The size value indicates the
size  of each element array in bytes (e.g. a char array has size = 1, a
short array has size = 2, an int array or a float array have size =  4,
and a double array has size = 8). The array is associated with the MBIO
descriptor mbio\_ptr, and is freed when mb\_close is called for this par-
ticular mbio\_ptr.

#### mb\_close()

	int mb\_close(
         int verbose,
         char *mbio\_ptr,
         int *error)

Closes  the  data  file  listed  in  the  MBIO descriptor pointed to by
mbio\_ptr and releases all specially  allocated  memory,  including  all
application  arrays  registered  using mb\_register\_array.  The verbose
value controls the standard error output verbosity of the  function.  A
status  value indicating success or failure is returned; an error value
argument passes more detailed information about failures.

### Level 1 Functions

#### mb\_read()

	int mb_read(
         int verbose,
         char *mbio_ptr,
         int *kind,
         int *pings,
         int time_i[7],
         double *time\_d,
         double *navlon,
         double *navlat,
         double *speed,
         double *heading,
         double *distance,
         double *altitude,
         double *sonardepth,
         int *nbath,
         int *namp,
         int *nss,
         char *beamflag,
         double *bath,
         double *amp,
         double *bathlon,
         double *bathlat,
         double *ss,
         double *sslon,
         double *sslat,
         char *comment,
         int *error);

The function mb\_read reads, processes, and returns sonar data according
to  the MBIO descriptor pointed to by mbio\_ptr.  The verbose value con-
trols the standard error output verbosity of the function. A number  of
different  data record types are recognized by MB-System, but mb\_read()
only returns survey and comment data records. The kind value  indicates
which type of record has been read.  The data is in the form of bathym-
etry, amplitude, and sidescan values combined with  the  longitude  and
latitude  locations of the bathymetry and sidescan measurements (ampli-
tudes are coincident with the bathymetry).

The return values are:

* kind: kind of data record read
	* 1    survey data
	* 2    comment
	* >=3  other data that cannot be passed by mb\_read
* pings: number of pings averaged to give current data
* time\_i: time of current ping
	* time\_i[0]: year
	* time\_i[1]: month
	* time\_i[2]: day
	* time\_i[3]: hour
	* time\_i[4]: minute
	* time\_i[5]: second
	* time\_i[6]: microsecond
* time\_d: time of current ping in seconds since 1/1/70 00:00:00
* navlon:			longitude
* navlat:        latitude
* speed:         ship speed in km/s
* heading:       ship heading in degrees
* distance:      distance along shiptrack since last ping in km
* altitude:      altitude of sonar above seafloor in m
* sonardepth:    depth of sonar in m
* nbath:         number of bathymetry values
* namp:          number of amplitude values
* nss:      		number of sidescan values
* beamflag:      array of bathymetry flags
* bath:          array of bathymetry values in meters
* amp:      		array of amplitude values in unknown units
* bathlon:       array of of longitude values corresponding to bathymetry
* bathlat:       array of of latitude values corresponding to bathymetry
* ss:       		array of sidescan values in unknown units
* sslon:         array of of longitude values corresponding to sidescan
* sslat:         array of of latitude values corresponding to sidescan
* comment:  		comment string
* error:         error value

A status value indicating success or failure  is  returned;  the  error
value  argument error passes more detailed information about read failures.


#### mb\_get()

	int mb_get(
         int verbose,
         char *mbio_ptr,
         int *kind,
         int *pings,
         int time_i[7],
         double *time\_d,
         double *navlon,
         double *navlat,
         double *speed,
         double *heading,
         double *distance,
         double *altitude,
         double *sonardepth,
         int *nbath,
         int *namp,
         int *nss,
         char *beamflag,
         double *bath,
         double *amp,
         double *bathacrosstrack,
         double *bathalongtrack,
         double *ss,
         double *ssacrosstrack,
         double *ssalongtrack,
         char *comment,
         int *error);

The function mb\_get reads, processes, and returns sonar data  according
to  the MBIO descriptor pointed to by mbio\_ptr.  The verbose value con-
trols the standard error output verbosity of the function. A number  of
different  data  record types are recognized by MB-System, but mb\_get()
only returns survey and comment data records. The kind value  indicates
which type of record has been read.  The data is in the form of bathym-
etry, amplitude, and sidescan values combined with the acrosstrack  and
alongtrack  distances  relative to the navigation of the bathymetry and
sidescan measurements (amplitudes are coincident  with  the  bathymetry
values).

The return values are:

* kind:          kind of data record read
	* 1    survey data
	* 2    comment
	* >=3  other data that cannot be passed by mb\_get
* pings:         number of pings averaged to give current data
* time\_i:        time of current ping
	* time\_i[0]: year
	* time\_i[1]: month
	* time\_i[2]: day
	* time\_i[3]: hour
	* time\_i[4]: minute
	* time\_i[5]: second
	* time\_i[6]: microsecond
* time\_d:        time of current ping in seconds since 1/1/70 00:00:00
* navlon:        longitude
* navlat:        latitude
* speed:         ship speed in km/s
* heading:       ship heading in degrees
* distance: distance along shiptrack since last ping in km
* altitude:      altitude of sonar above seafloor in m
* sonardepth:    depth of sonar in m
* nbath:         number of bathymetry values
* namp:          number of amplitude values
* nss:      number of sidescan values
* beamflag:      array of bathymetry flags
* bath:          array of bathymetry values in meters
* amp:      array of amplitude values in unknown units
* bathacrosstrack:    array of of acrosstrack distances in meters corresponding to bathymetry
* bathalongtrack:     array of of alongtrack distances in meters corresponding to bathymetry
* ss:       array of sidescan values in unknown units
* ssacrosstrack: array of of acrosstrack distances in meters corresponding to sidescan
* ssacrosstrack: array of of alongtrack distances in meters corresponding to sidescan
* comment:       comment string
* error:         error value

A  status  value  indicating  success or failure is returned; the error
value argument error passes more detailed information about read  fail-
ures.


### Level 2 Functions

#### mb\_read\_ping()

	int mb_read_ping(
         int verbose,
         char *mbio_ptr,
         char *store_ptr,
         int *kind,
         int *error);

The function mb\_read\_ping reads and returns sonar data according to the
MBIO descriptor pointed to by mbio\_ptr.  The verbose value controls the
standard  error output verbosity of the function.  The data is returned
one record at a time; no averaging is performed.  A pointer to  a  data
structure containing all of the data read is returned as store\_ptr; the
form of the data structure is determined by the sonar system associated
with  the  format  of  the  data being read. A number of different data
record types are recognized by  MB-System;  the  kind  value  indicates
which type of record has been read.

The return values are:

* store\_ptr:     pointer to complete data structure
* kind:          kind of data record read
	* 1    survey data
	* 2    comment
	* 3    header
	* 4    calibrate
	* 5    mean sound speed
	* 6    SVP
	* 7    standby
	* 8    nav source
	* 9    parameter
	* 10   start
	* 11   stop
	* 12   nav
	* 13   run parameter
	* 14   clock
	* 15   tide
	* 16   height
	* 17   heading
	* 18   attitude
	* 19   ssv
	* 20   angle
	* 21   event
	* 22   history
	* 23   summary
	* 24   processing parameters
	* 25   sensor parameters
	* 26   navigation error
	* 27   uninterpretable line
* error:         error value

A  status  value  indicating  success or failure is returned; the error
value argument error passes more detailed information about read  fail-
ures.

#### mb\_write\_ping()

	int mb_write_ping(
         int verbose,
         char *mbio_ptr,
         char *store_ptr,
         int *error);

The  function mb\_write\_ping writes sonar data to the file listed in the
MBIO descriptor pointed to by MBIO\_ptr.  The verbose value controls the
standard  error  output verbosity of the function.  A pointer to a data
structure containing all of the data read is passed as  store\_ptr;  the
form of the data structure is determined by the sonar system associated
with the format of the data being written.  The  values  to  be  output
are:

* store\_ptr:     pointer to complete data structure

The return values are:

* error:         error value

A  status  value  indicating  success or failure is returned; the error
value argument error passes more detailed information about write failures.

#### mb\_get\_store()

	int mb_get_store(
         int verbose,
         char *mbio_ptr,
         char **store_ptr,
         int *error);

The  function  mb\_get\_store()  returns a pointer *store\_ptr to the data
storage  structure  associated  with  a  particular   MBIO   descriptor
mbio\_ptr.  The  mb\_read\_init() and mb\_write\_init() functions both 
allocate one of these internal storage structures. The  form  of  the  data
structure  is determined by the sonar system associated with the format
of the data being written.  Storage structure pointers must  be  passed
to  level  two  MBIO functions such as mb\_write\_ping() and mb\_insert().
The verbose value controls the standard error output verbosity  of  the
function.

The return values are:

* store\_ptr:     pointer to complete data structure
* error:         error value

A  status  value  indicating  success or failure is returned; the error
value argument error passes more detailed information about failures.

#### mb\_get\_all()

	int mb_get_all(
         int verbose,
         char *mbio_ptr,
         char **store_ptr,
         int *kind,
         int time_i[7],
         double *time_d,
         double *navlon,
         double *navlat,
         double *speed,
         double *heading,
         double *distance,
         double *altitude,
         double *sonardepth,
         int *nbath,
         int *namp,
         int *nss,
         char *beamflag,
         double *bath,
         double *amp,
         double *bathacrosstrack,
         double *bathalongtrack,
         double *ss,
         double *ssacrosstrack,
         double *ssalongtrack,
         char *comment,
         int *error);

The function mb\_get\_all reads and returns sonar data according  to  the
MBIO descriptor pointed to by mbio\_ptr.  The verbose value controls the
standard error output verbosity of the function.  The data is  returned
one  record  at a time; no averaging is performed.  A pointer to a data
structure containing all of the data read is returned as store\_ptr; the
form of the data structure is determined by the sonar system associated
with the format of the data being read.  A  number  of  different  data
record  types  are  recognized  by  MB-System; the kind value indicates
which type of record has been read. Additional data is returned if  the
data  record  is  survey  data  (navigation, bathymetry, amplitude, and
sidescan), navigation data (navigation only), or comment data  (comment
only).

The return values are:




* store\_ptr:     pointer to complete data structure
* kind:          kind of data record read
	* 1    survey data
	* 2    comment
	* 3    header
	* 4    calibrate
	* 5    mean sound speed
	* 6    SVP
	* 7    standby
	* 8    nav source
	* 9    parameter
	* 10   start
	* 11   stop
	* 12   nav
	* 13   run parameter
	* 14   clock
	* 15   tide
	* 16   height
	* 17   heading
	* 18   attitude
	* 19   ssv
	* 20   angle
	* 21   event
	* 22   history
	* 23   summary
	* 24   processing parameters
	* 25   sensor parameters
	* 26   navigation error
	* 27   uninterpretable line
* time\_i:        time of current ping
	* time\_i[0]: year
	* time\_i[1]: month
	* time\_i[2]: day
	* time\_i[3]: hour
	* time\_i[4]: minute
	* time\_i[5]: second
	* time\_i[6]: microsecond
* time\_d:        time of current ping in seconds
* since 1/1/70 00:00:00
* navlon:        longitude
* navlat:        latitude
* speed:         ship speed in km/s
* heading:       ship heading in degrees
* distance: distance along shiptrack since last ping in km
* altitude:      altitude of sonar above seafloor in m
* sonardepth:    depth of sonar in m
* nbath:         number of bathymetry values
* namp:          number of amplitude values
* nss:      number of sidescan values
* beamflag:      array of bathymetry flags
* bath:          array of bathymetry values in meters
* amp:      array of amplitude values in unknown units
* bathacrosstrack:    array of of acrosstrack distances in meters corresponding to bathymetry
* bathalongtrack:     array of of alongtrack distances in meters corresponding to bathymetry
* ss:       array of sidescan values in unknown units
* ssacrosstrack: array of of acrosstrack distances in meters corresponding to sidescan
* ssacrosstrack: array of of alongtrack distances in meters corresponding to sidescan
* comment:  comment string
* error:         error value

A  status  value  indicating  success or failure is returned; the error
value argument error passes more detailed information about read  fail-
ures.

#### mb\_put\_all()

	int mb_put_all(
         int verbose,
         char *mbio_ptr,
         char *store_ptr,
         int usevalues,
         int kind,
         int time_i[7],
         double time_d,
         double navlon,
         double navlat,
         double speed,
         double heading,
         int nbath,
         int namp,
         int nss,
         char *beamflag,
         double *bath,
         double *amp,
         double *bathacrosstrack,
         double *bathalongtrack,
         double *ss,
         double *ssacrosstrack,
         double *ssalongtrack,
         char *comment,
         int *error);

The  function  mb\_put\_all  writes  sonar data to the file listed in the
MBIO descriptor pointed to by MBIO\_ptr.  The verbose value controls the
standard  error  output verbosity of the function.  A pointer to a data
structure containing all of the data read is passed as  store\_ptr;  the
form of the data structure is determined by the sonar system associated
with the format of the data being written.  Additional data  is  passed
if  the  data record is survey data (navigation, bathymetry, amplitude,
and sidescan), navigation data (navigation only), or comment data (com-
ment  only).  If the usevalues flag is set to 1, then the passed values
will be inserted in the data structure pointed to by  store\_ptr  before
the  data  is  written.   If  the  usevalues flag is set to 0, the data
structure pointed to by store\_ptr will be written without modification.

The values to be output are:

* store\_ptr:     pointer to complete data structure
* usevalues:     flag controlling use of data passed by value
	* 0    do not insert into data structure before writing the data
	* 1    insert into data structure before writing the data
* kind:          kind of data record to be written
	* 1    survey data
	* 2    comment
	* 3    header
	* 4    calibrate
	* 5    mean sound speed
	* 6    SVP
	* 7    standby
	* 8    nav source
	* 9    parameter
	* 10   start
	* 11   stop
	* 12   nav
	* 13   run parameter
	* 14   clock
	* 15   tide
	* 16   height
	* 17   heading
	* 18   attitude
	* 19   ssv
	* 20   angle
	* 21   event
	* 22   history
	* 23   summary
	* 24   processing parameters
	* 25   sensor parameters
	* 26   navigation error
	* 27   uninterpretable line
* time\_i:        time of current ping (used if time\_i[0] != 0)
* time\_i[0]: year
* time\_i[1]: month
* time\_i[2]: day
* time\_i[3]: hour
* time\_i[4]: minute
* time\_i[5]: second
* time\_i[6]: microsecond
* time\_d:        time of current ping in seconds since 1/1/70 00:00:00 (used if time\_i[0] = 0)
* navlon:        longitude
* navlat:        latitude
* speed:         ship speed in km/s
* heading:       ship heading in degrees
* nbath:         number of bathymetry values
* namp:          number of amplitude values
* nss:      number of sidescan values
* beamflag:      array of bathymetry flags
* bath:          array of bathymetry values in meters
* amp:      array of amplitude values in unknown units
* bathacrosstrack:    array of of acrosstrack distances in meters corresponding to bathymetry
* bathalongtrack:     array of of alongtrack distances in meters corresponding to bathymetry
* ss:       array of sidescan values in unknown units
* ssacrosstrack: array of of acrosstrack distances in meters corresponding to sidescan
* ssacrosstrack: array of of alongtrack distances in meters corresponding to sidescan
* comment:  comment string

The return values are:

* error:         error value

A  status  value  indicating  success or failure is returned; the error
value argument error passes more detailed information about write failures.

#### mb\_put\_comment()

	int mb_put_comment(
         int verbose,
         char *mbio_ptr,
         char *comment,
         int *error);

The  function mb\_put\_comment writes a comment to the file listed in the
MBIO descriptor pointed to by MBIO\_ptr.  The verbose value controls the
standard  error  output  verbosity of the function.  The data is in the
form of a null terminated string. The maximum length of comments varies
with  different  data formats. In general individual comments should be
less than 80 characters long to insure compatibility with all  formats.
The values to be output are:

* comment:       comment string

The return values are:

* error:         error value

A  status  value  indicating  success or failure is returned; the error
value argument error passes more detailed information about write failures.

#### mb\_extract()

	int mb_extract(
         int verbose,
         char *mbio_ptr,
         char *store_ptr,
         int *kind,
         int time_i[7],
         double *time_d,
         double *navlon,
         double *navlat,
         double *speed,
         double *heading,
         int *nbath,
         int *namp,
         int *nss,
         char *beamflag,
         double *bath,
         double *amp,
         double *bathacrosstrack,
         double *bathalongtrack,
         double *ss,
         double *ssacrosstrack,
         double *ssalongtrack,
         char *comment,
         int *error);

The  function mb\_extract extracts sonar data from the structure pointed
to by \*store\_ptr  according  to  the  MBIO  descriptor  pointed  to  by
mbio\_ptr.   The  verbose  value controls the standard error output ver-
bosity of the function.  The form of the data structure  is  determined
by  the sonar system associated with the format of the data being read.
A number of different data record types are  recognized  by  MB-System;
the  kind value indicates which type of record is stored in \*store\_ptr.
Additional data is returned if the data record is survey data  (naviga-
tion, bathymetry, amplitude, and sidescan), navigation data (navigation
only), or comment data (comment only).

The return values are:

* kind:          kind of data record read
	* 1    survey data
	* 2    comment
	* 12   navigation
* time\_i:        time of current ping
	* time\_i[0]: year
	* time\_i[1]: month
	* time\_i[2]: day
	* time\_i[3]: hour
	* time\_i[4]: minute
	* time\_i[5]: second
	* time\_i[6]: microsecond
* time\_d:        time of current ping in seconds since 1/1/70 00:00:00
* navlon:        longitude
* navlat:        latitude
* speed:         ship speed in km/s
* heading:       ship heading in degrees
* distance:      distance along shiptrack since last ping in km
* altitude:      altitude of sonar above seafloor in m
* sonardepth:    depth of sonar in m
* nbath:         number of bathymetry values
* namp:          number of amplitude values
* nss:      number of sidescan values
* beamflag:      array of bathymetry flags
* bath:          array of bathymetry values in meters
* amp:      array of amplitude values in unknown units
* bathacrosstrack:    array of of acrosstrack distances in meters corresponding to bathymetry
* bathalongtrack:     array of of alongtrack distances in meters corresponding to bathymetry
* ss:       array of sidescan values in unknown units
* ssacrosstrack: array of of acrosstrack distances in meters corresponding to sidescan
* ssacrosstrack: array of of alongtrack distances in meters corresponding to sidescan
* comment:  comment string
* error:         error value

A status value indicating success or failure  is  returned;  the  error
value  argument  error  passes  more detailed information about extract
failures.

#### mb\_insert()

	int mb_insert(
         int verbose,
         char *mbio_ptr,
         char *store_ptr,
         int kind,
         int time_i[7],
         double time_d,
         double navlon,
         double navlat,
         double speed,
         double heading,
         int nbath,
         int namp,
         int nss,
         char *beamflag,
         double *bath,
         double *amp,
         double *bathacrosstrack,
         double *bathalongtrack,
         double *ss,
         double *ssacrosstrack,
         double *ssalongtrack,
         char *comment,
         int *error);

The function mb\_insert inserts sonar data into the structure pointed to
by  \*store\_ptr according to the MBIO descriptor pointed to by mbio\_ptr.
The verbose value controls the standard error output verbosity  of  the
function.   The  form  of the data structure is determined by the sonar
system associated with the format of the data being read. A  number  of
different data record types are recognized by MB-System; the kind value
indicates which type of record is to be  stored  in  \*store\_ptr.   Data
will  be  inserted  only if the data record is survey data (navigation,
bathymetry,  amplitude,  and  sidescan),  navigation  data  (navigation
only), or comment data (comment only).  

The values to be inserted are:

* kind:          kind of data record inserted
	* 1    survey data
	* 2    comment
	* 12   navigation
* time\_i:        time of current ping
	* time\_i[0]: year
	* time\_i[1]: month
	* time\_i[2]: day
	* time\_i[3]: hour
	* time\_i[4]: minute
	* time\_i[5]: second
	* time\_i[6]: microsecond
* time\_d:        time of current ping in seconds since 1/1/70 00:00:00
* navlon:        longitude
* navlat:        latitude
* speed:         ship speed in km/s
* heading:       ship heading in degrees
* distance:      distance along shiptrack since last ping in km
* altitude:      altitude of sonar above seafloor in m
* sonardepth:    depth of sonar in m
* nbath:         number of bathymetry values
* namp:          number of amplitude values
* nss:      number of sidescan values
* beamflag:      array of bathymetry flags
* bath:          array of bathymetry values in meters
* amp:      array of amplitude values in unknown units
* bathacrosstrack:    array of of acrosstrack distances in meters corresponding to bathymetry
* bathalongtrack:     array of of alongtrack distances in meters corresponding to bathymetry
* ss:       array of sidescan values in unknown units
* ssacrosstrack: array of of acrosstrack distances in meters corresponding to sidescan
* ssacrosstrack: array of of alongtrack distances in meters corresponding to sidescan
* comment:  comment string

The return values are:

* error:         error value

A  status  value  indicating  success or failure is returned; the error
value argument error passes  more  detailed  information  about  insert
failures.

#### mb\_extract\_nav()

	int mb_extract_nav(
         int verbose,
         char *mbio_ptr,
         char *store_ptr,
         int *kind,
         int time_i[7],
         double *time\_d,
         double *navlon,
         double *navlat,
         double *speed,
         double *heading,
         double *draft,
         double *roll,
         double *pitch,
         double *heave,
         int *error);

The function mb\_extract\_nav extracts navigation data from the structure
pointed to by \*store\_ptr according to the MBIO descriptor pointed to by
mbio\_ptr.   The  verbose  value controls the standard error output ver-
bosity of the function.  The form of the data structure  is  determined
by  the sonar system associated with the format of the data being read.
A number of different data record types are  recognized  by  MB-System;
the  kind value indicates which type of record is stored in *store\_ptr.
Navigation data is returned if the data record is survey data  (naviga-
tion,  bathymetry, amplitude, and sidescan) or navigation data (naviga-
tion only).

The return values are:

* kind: kind of data record read
	* 1    survey data
	* 12   navigation
* time\_i: time of current ping
	* time\_i[0]: year
	* time\_i[1]: month
	* time\_i[2]: day
	* time\_i[3]: hour
	* time\_i[4]: minute
	* time\_i[5]: second
	* time\_i[6]: microsecond
* time\_d: time of current ping in seconds since 1/1/70 00:00:00
* navlon: longitude
* navlat: latitude
* speed: ship speed in km/s
* heading: ship heading in degrees
* draft: sonar depth in meters
* roll: sonar roll in degrees
* pitch: sonar pitch in degrees
* heave: sonar heave in meters

A status value indicating success or failure  is  returned;  the  error
value  argument  error  passes  more detailed information about extract
failures.

#### mb\_insert\_nav()

	int mb_insert_nav(
         int verbose,
         char *mbio_ptr,
         char *store_ptr,
         int time_i[7],
         double time_d,
         double navlon,
         double navlat,
         double speed,
         double heading,
         double draft,
         double roll,
         double pitch,
         double heave,
         int *error);

The function mb\_insert\_nav inserts navigation data into  the  structure
pointed to by \*store\_ptr according to the MBIO descriptor pointed to by
mbio\_ptr.  The verbose value controls the standard  error  output  ver-
bosity  of  the function.  The form of the data structure is determined
by the sonar system associated with the format of the data being  read.
A  number  of  different data record types are recognized by MB-System;
the kind value indicates which type  of  record  is  to  be  stored  in
*store\_ptr.   Data  will  be inserted only if the data record is survey
data (navigation, bathymetry, amplitude, and sidescan),  or  navigation
data (navigation only).  The values to be inserted are:

* time\_i: time of current ping
	* time\_i[0]: year
	* time\_i[1]: month
	* time\_i[2]: day
	* time\_i[3]: hour
	* time\_i[4]: minute
	* time\_i[5]: second
	* time\_i[6]: microsecond
* time\_d: time of current ping in seconds since 1/1/70 00:00:00
* navlon: longitude
* navlat: latitude
* speed: ship speed in km/s
* heading: ship heading in degrees
* draft: sonar depth in meters
* roll: sonar roll in degrees
* pitch: sonar pitch in degrees
* heave: sonar heave in meters

The return values are:

* error: error value

A  status  value  indicating  success or failure is returned; the error
value argument error passes  more  detailed  information  about  insert
failures.

#### mb\_extract\_altitude()

	int mb_extract_altitude(
         int verbose,
         char *mbio_ptr,
         char *store_ptr,
         int *kind,
         double *transducer_depth,
         double *altitude,
         int *error);

The  function  mb\_extract\_altitude  extracts the sonar transducer depth
(transducer\_depth) below the sea surface and the the  sonar  transducer
altitude above the seafloor according to the MBIO descriptor pointed to
by mbio\_ptr.  This function is not defined for all data  formats.   The
verbose value controls the standard error output verbosity of the func-
tion.  The form of the data structure is determined by the sonar system
associated  with the format of the data being read. A number of differ-
ent data record types are recognized by MB-System; the kind value indi-
cates  which  type  of  record is stored in \*store\_ptr.  These data are
returned only if the data record is survey data.  These values are use-
ful  for  sidescan  processing applications. Both transducer depths and
altitudes are reported in meters.

The return values are:

* kind:          kind of data record read (error if not survey data):
	* 1    survey data
* transducer\_depth:   depth of sonar in meters
* altitude:      altitude of sonar above seafloor in meters.
* error:         error value

A status value indicating success or failure  is  returned;  the  error
value  argument  error  passes  more  detailed  information  about data
extraction failures.

#### mb\_insert\_altitude()

	int mb_insert_altitude(
         int verbose,
         char *mbio_ptr,
         char *store_ptr,
         double transducer_depth,
         double altitude,
         int *error);

The function mb\_insert\_altitude inserts sonar depth and  altitude  data
into  the  structure  pointed  to  by  \*store\_ptr according to the MBIO
descriptor pointed to by mbio\_ptr.  This function is  not  defined  for
all data formats.  The verbose value controls the standard error output
verbosity of the function.  The form of the data  structure  is  deter-
mined  by the sonar system associated with the format of the data being
read. A number of different data record types are recognized by MB-Sys-
tem.   Data  will  be  inserted  only if the data record is survey data
(navigation, bathymetry, amplitude, and sidescan).  The  values  to  be
inserted are:

* transducer\_depth:   depth of sonar in meters
* altitude:      altitude of sonar in meters

The return values are:

* error:         error value

A  status  value  indicating  success or failure is returned; the error
value argument error passes  more  detailed  information  about  insert
failures.

#### mb\_extract\_svp()

	int mb_extract_svp(
         int verbose,
         char *mbio_ptr,
         char *store_ptr,
         int *kind,
         int *nsvp,
         double *depth,
         double *velocity,
         int *error);

The  function  mb\_extract\_svp  extracts  a water sound velocity profile
according to the MBIO descriptor pointed to by mbio\_ptr.  This function
is  not  defined  for all data formats.  The verbose value controls the
standard error output verbosity of the function.  The form of the  data
structure  is determined by the sonar system associated with the format
of the data being read. A number of different  data  record  types  are
recognized  by MB-System; the kind value indicates which type of record
is stored in \*store\_ptr.  These data are  returned  only  if  the  data
record  is a sound velocity profile record. These values are useful for
calculating bathymetry from travel times and beam angles.

The return values are:

* kind:          kind of data record read (error if not SVP data):
	* 6    SVP data
* nsvp:          number of depth and sound speed data in the profile
* depth:         array of depths in meters
* velocity:      array of sound speeds in m/sec
* error:         error value

A status value indicating success or failure  is  returned;  the  error
value  argument  error  passes  more  detailed  information  about data
extraction failures.

#### mb\_insert\_svp()

	int mb_insert_svp(
         int verbose,
         char *mbio_ptr,
         char *store_ptr,
         int nsvp,
         double *depth,
         double *velocity,
         int *error);

The function mb\_insert\_svp  inserts  a  water  sound  velocity  profile
according to the MBIO descriptor pointed to by mbio\_ptr.  This function
is not defined for all data formats.  The verbose  value  controls  the
standard  error output verbosity of the function.  The form of the data
structure is determined by the sonar system associated with the  format
of  the  data  being  read. A number of different data record types are
recognized by MB-System.  These data are  inserted  only  if  the  data
record  is a sound velocity profile record. These values are useful for
calculating bathymetry from travel times and beam angles.  

The inserted values are:

* nsvp:          number of depth and sound speed data in the profile
* depth:         array of depths in meters
* velocity:      array of sound speeds in m/sec

The return values are:

* error:         error value

A  status  value  indicating  success or failure is returned; the error
value argument error passes more detailed information about data insertion failures.

#### mb\_ttimes()

	int mb\_ttimes(
         int verbose,
         char *mbio_ptr,
         char *store_ptr,
         int *kind,
         int *nbeams,
         double *ttimes,
         double    *angles,
         double *angles_forward,
         double *angles_null,
         double *heave,
         double *alongtrack_offset,
         double *draft,
         double *ssv,
         int *error);

The  function  mb\_ttimes  extracts  travel times and beam angles from a
sonar-specific data structure pointed to by store\_ptr. These values are
used  for  calculating swath bathymetry. The verbose value controls the
standard error output verbosity of the function.   The  coordinates  of
the  beam  angles  can  be a bit confusing.  The angles are returned in
"takeoff angle  coordinates"  appropriate  for  raytracing.  The  array
angles  contains  the  angle from vertical and the array angles\_forward
contains the angle from acrosstrack. This coordinate system is distinct
from  the  roll-pitch  coordinates  appropriate for correcting roll and
pitch values. A description of these  relevant  coordinate  systems  is
given  below.  The angles\_null array contains the effective sonar array
orientation for each beam. The angles\_null array may be used to correct
beam  angles  using Snell's law if the ssv is changed.  The angles\_null
values reflect the sonar configuration.  For  example,  some  multibeam
sonars  have a flat transducer array, and so the angles\_null array con-
sists of nbeams zero values.  Other multibeams have circular arrays  so
that  the  angles\_null  values  equal  the  angles  values.  The along-
track\_offset array accommodates sonars which report multiple pings in a
single  survey  record;  each ping occurs at a different position along
the shiptrack, producing alongtrack offsets relative to the  navigation
for  some  beam  values. The sum of the draft value and the heave array
values gives the depth of the sonar for each  beam.  For  hull  mounted
installations  the draft value is generally static but the heave values
vary with time. For towed sonars the draft varies  with  time  and  the
heave  values  are  typically zero. The ssv value gives the water sound
velocity at the sonar array.

The return values are:

* kind:          kind of data record read (error if not survey data):
	* 1    survey data
* nbeams:        number of beams
* ttimes:        array of two-way travel times in seconds
* angles:        array of angles from vertical in degrees
* angles\_forward:     array of angles from acrosstrack in degrees
* angles\_null:   array of sonar array orientation in degrees
* heave:         array of heave values for each beam in meters
* alongtrack\_offset:array of alongtrack distance offsets for each beam in meters
* draft:         draft of sonar in meters
* ssv:      water sound velocity at sonar in m/seconds
* error:         error value

A status value indicating success or failure  is  returned;  the  error
value  argument  error  passes  more  detailed  information  about data
extraction failures.

#### mb\_detects()

	int mb_detects(
         int verbose,
         void *mbio_ptr,
         void *store_ptr,
         int *kind,
         int *nbeams,
         int *detects,
         int *error);

The function mb\_detects extracts beam bottom detect types from a sonar-
specific  data structure pointed to by store\_ptr. These values indicate
whether the depth value associated with a  particular  beam  i  derived
from  an  amplitude  detect (e.g. detects[i] = 1), a phase detect (e.g.
detects[i] = 2), or the algorithm is unknown (e.g. detects[i] = 0). The
verbose value controls the standard error output verbosity of the func-
tion.

The return values are:

* kind:          kind of data record read (error if not survey data):
	* 1    survey data
* nbeams:        number of beams
* detects:       array of nbeams bottom detect algorithm flags
	* 0  =  unknown                            
	* 1   = amplitude detect                          
	* 2 = phase detect
* error:         error value

A  status  value  indicating  success or failure is returned; the error
value argument  error  passes  more  detailed  information  about  data
extraction  failures. This functionality is available for only a subset
of the supported sonars. If the  corresponding  low  level  routine  is
undefined, \*error will be set to MB\_ERROR\_BAD\_SYSTEM (14).

#### mb\_gains()

	int mb_gains(
         int verbose,
         void *mbio_ptr,
         void *store_ptr,
         int *kind,
         double *transmit_gain,
         double *pulse_length,
         double *receive_gain,
         int *error);

The  function  mb\_gains  extracts  the  most basic gain settings from a
sonar-specific data structure pointed to by store\_ptr. In  many  cases,
sonars  have more complicated gain functions, particularly with respect
to the receiver TVG function. In those cases, the receive gain returned
here  refers  to the constant gain setting and does not include any TVG
parameters. The verbose value controls the standard error  output  ver-
bosity of the function.

The return values are:

* kind:          kind of data record read (error if not survey data):
	* 1    survey data
* transmit\_gain: transmit gain (dB)
* pulse\_length:  transmit pulse length (sec)
* receive\_gain:  receive gain (dB)
* error:         error value

A  status  value  indicating  success or failure is returned; the error
value argument  error  passes  more  detailed  information  about  data
extraction failures.  This functionality is available for only a subset
of the supported sonars. If the  corresponding  low  level  routine  is
undefined, *error will be set to MB\_ERROR\_BAD\_SYSTEM (14).

#### mb\_extract\_rawss()

	int mb_extract_rawss(
         int verbose,
         char *mbio_ptr,
         char *store_ptr,
         int *kind,
         int *nrawss,
         double *rawss,
         double *rawssacrosstrack,
         double *rawssalongtrack,
         int *error);

This  function  has  not yet been implemented for any data format.  The
notion is that since some formats  carry  both  "raw"  and  "processed"
sidescan  imagery,  there should be functions to extract and insert the
"raw" sidescan. Given that the meaning of "raw" sidescan varies greatly
among sonars, the processing one might apply to the data will depend on
the sonar source. The definition of mb\_extract\_rawss  may  well  change
when we actually implement it.

#### mb\_insert\_rawss()

	int mb\_insert_rawss(
         int verbose,
         char *mbio_ptr,
         char *store_ptr,
         int nrawss,
         double *rawss,
         double *rawssacrosstrack,
         double *rawssalongtrack,
         int *error);

This  function  has  not yet been implemented for any data format.  The
notion is that since some formats  carry  both  "raw"  and  "processed"
sidescan  imagery,  there should be functions to extract and insert the
"raw" sidescan. Given that the meaning of "raw" sidescan varies greatly
among sonars, the processing one might apply to the data will depend on
the sonar source. The definition of  mb\_insert\_rawss  may  well  change
when we actually implement it.

#### mb\_copyrecord()

	int mb_copyrecord(
         int verbose,
         char *mbio_ptr,
         char *store_ptr,
         char *copy_ptr,
         int *error);

The  function  mb\_copyrecord  copies  the sonar-specific data structure
pointed  to  by  store\_ptr  into  the  data  structure  pointed  to  by
\*copy\_ptr.  The data structures must already have been allocated.

The return values are:

* error:         error value

A  status  value  indicating  success or failure is returned; the error
value argument error passes more detailed information about  data  copy
failures.

### Level 3 Functions

#### mb\_buffer\_init()

	int mb_buffer_init(
         int verbose,
         char **buff_ptr,
         int *error);

The  function  mb\_buffer\_init  initializes the data structures required
for buffered i/0. A pointer to the buffer data structure is returned as
\*buff\_ptr.   The  verbose value controls the standard error output ver-
bosity of the function.

The return values are:

* \*buff\_ptr:     pointer to buffer structure
* error:         error value

A status value indicating success or failure is returned;  the  error
value argument error passes more detailed information about buffer initialization failures.

#### mb\_buffer\_close()

	int mb_buffer_close(
         int verbose,
         char **buff_ptr,
         char *mbio_ptr,
         int *error);

The function mb\_buffer\_close releases all memory allocated for buffered
i/0,  including  the  structure  pointed  to by *buff\_ptr.  The verbose
value controls the standard error output verbosity of the function.

The return values are:

* error:         error value

A status value indicating success or failure  is  returned;  the  error
value  argument  error  passes  more  detailed information about buffer
deallocation failures.

#### mb\_buffer\_load()

	int mb_buffer_load(
         int verbose,
         char *buff_ptr,char *mbio_ptr,
         int nwant,
         int *nload,
         int *nbuff,
         int *error);

The function mb\_buffer\_load loads data into the buffer  pointed  to  by
buff\_ptr from the input file initialized in the MBIO descriptor pointed
to by mbio\_ptr.  The verbose value controls the standard  error  output
verbosity of the function.

The input control parameters have the following significance:

* nwant: The number of data records desired in the buffer.

The returned values are:

* nload: The  number of data records loaded into the buffer.
* nbuff: The total number of  data  records  in  the  buffer after loading.
* error: error value

The  buffer  may  already  contain data records when the mb\_buffer\_load
call is made; if the number of previously loaded records is  less  than
nwant, the function will attempt to read and load records until a total
of nwant records are loaded. The nload value  is  the  number  of  data
records loaded during the current function call, and the nbuff value is
the number of data records in the  buffer  at  the  completion  of  the
mb\_buffer\_load  call.   A status value indicating success or failure is
returned; the error value argument error passes more detailed  information 
about buffer deallocation failures.

#### mb\_buffer\_dump()

	int mb\_buffer_dump(
         int verbose,
         char *buff_ptr,
         char *mbio_ptr,
         char *ombio_ptr,
         int nhold,
         int *ndump,
         int *nbuff,
         int *error);

The  function  mb\_buffer\_dump  dumps data from the buffer pointed to by
*buff\_ptr into the output  file  initialized  in  the  MBIO  descriptor
pointed  to  by  ombio\_ptr.  The  data in the buffer were read from the
input file initialized in the MBIO descriptor pointed to  by  mbio\_ptr.
The  verbose  value controls the standard error output verbosity of the
function.

The input control parameters have the following significance:

* nhold:         The number of data records desired to be held in the buffer.

The returned values are:

* nload:         The number of data records dumped from the  buffer.
* nbuff:         The  total  number  of  data  records in the buffer after dumping.
* error:         error value

If the number of loaded records is more than nhold, the  function  will
attempt  to  write  out  records from the beginning of the buffer until
nhold records are left in the buffer. The ndump value is the number  of
data  records  dumped  during  the current function call, and the nbuff
value is the number of data records in the buffer at the completion  of
the  mb\_buffer\_dump call.  A status value indicating success or failure
is returned; the error value argument error passes more detailed information 
about buffer deallocation failures.

#### mb\_buffer\_clear()

	int mb_buffer_clear(
         int verbose,
         char *buff_ptr,
         char *mbio_ptr,
         int nhold,
         int *ndump,
         int *nbuff,
         int *error);

The function mb\_buffer\_clear removes data from the buffer pointed to by
*buff\_ptr without writing those data records to an output file. An MBIO
descriptor pointed to by mbio\_ptr is still required, and generally rep-
resents the MBIO descriptor used to read and load the data  originally.
The  verbose  value controls the standard error output verbosity of the
function.

The input control parameters have the following significance:

* nwant: The number of data records desired to be held in the buffer.

The returned values are:

* nload: The number of data records cleared from the buffer.
* nbuff: The  total  number  of  data  records in the buffer after dumping.
* error: error value

If the number of loaded records is more than nhold, the  function  will
attempt  to  clear  out  records from the beginning of the buffer until
nhold records are left in the buffer. The ndump value is the number  of
data  records  cleared  during the current function call, and the nbuff
value is the number of data records in the buffer at the completion  of
the  mb\_buffer\_dump call.  A status value indicating success or failure
is returned; the error value argument error passes more detailed information 
about buffer deallocation failures.

#### mb\_buffer\_info()

	int mb_buffer_info(
         int verbose,
         char *buff_ptr,
         char *mbio_ptr,
         int id,
         int *system,
         int *kind,
         int *error);

The function mb\_buffer\_clear removes data from the buffer pointed to by
*buff\_ptr without writing those data records to an output file. An MBIO
descriptor pointed to by mbio\_ptr is still required, and generally rep-
resents the MBIO descriptor used to read and load the data  originally.
The  verbose  value controls the standard error output verbosity of the
function.

The input control parameters have the following significance:

* nwant: The number of data records desired to be held in the buffer.

The returned values are:

* nload: The number of data records cleared from the buffer.
* nbuff: The  total  number  of  data  records in the buffer after dumping.
* error: error value

If the number of loaded records is more than nhold, the  function  will
attempt  to  clear  out  records from the beginning of the buffer until
nhold records are left in the buffer. The ndump value is the number  of
data  records  cleared  during the current function call, and the nbuff
value is the number of data records in the buffer at the completion  of
the  mb\_buffer\_dump call.  A status value indicating success or failure
is returned; the error value argument error passes more detailed information 
about buffer deallocation failures.

#### mb\_buffer\_get\_next\_data()

	int mb_buffer_get_next_data(
         int verbose,
         char *buff_ptr,
         char *mbio_ptr,
         int start,
         int *id,
         int time_i[7],
         double *time_d,
         double *navlon,
         double *navlat,
         double *speed,
         double *heading,
         int *nbath,
         int *namp,
         int *nss,
         char *beamflag,
         double *bath,
         double *amp,
         double *bathacrosstrack,
         double *bathalongtrack,
         double *ss,
         double *ssacrosstrack,
         double *ssalongtrack,
         int *error);

The  function mb\_buffer\_get\_next\_data searches for the next survey data
record in the buffer, beginning at buffer  index  start.  Since  buffer
indexes  begin  at  0, the first call to mb\_buffer\_get\_next\_data should
have start = 0.  If a survey data record is found at or  beyond  start,
mb\_buffer\_get\_next\_data  returns the buffer index of that record in id.
Data is also returned in the forms of bathymetry, amplitude, and sides-
can  survey  data.  No  comments  or  other non-survey data records are
returned.  The verbose value controls the standard  error  output  ver-
bosity of the function.

The input control parameters have the following significance:

* start: The buffer index at which to start searching for a survey data record.

The returned values are:

* id: The buffer index of the first survey data record at or after start.
* time\_i: time of current ping
	* time\_i[0]: year
	* time\_i[1]: month
	* time\_i[2]: day
	* time\_i[3]: hour
	* time\_i[4]: minute
	* time\_i[5]: second
	* time\_i[6]: microsecond
* time\_d: time of current ping in seconds since 1/1/70 00:00:00
* navlon: longitude
* navlat: latitude
* speed: ship speed in km/s
* heading: ship heading in degrees
* nbath: number of bathymetry values
* namp: number of amplitude values
* nss: number  of  sidescan values      beamflag:      array of bathymetry flags
* bath: array of bathymetry values in meters
* amp: array of amplitude values in unknown units
* bathacrosstrack: array of of acrosstrack distances in meters corresponding to bathymetry
* bathalongtrack: array of of alongtrack distances in meters corresponding to bathymetry
* ss: array of sidescan values in unknown units
* ssacrosstrack: array of of acrosstrack distances in meters corresponding to sidescan
* ssacrosstrack: array of of alongtrack distances in meters corresponding to sidescan
* error: error value

A status value indicating success or failure  is  returned;  the  error
value  argument  error passes more detailed information about failures.
The most common error occurs when no more survey data records remain to
be found in the buffer; in this case, error = -14.

#### mb\_buffer\_extract()

	int mb_buffer_extract(
         int verbose,
         char *buff_ptr,
         char *mbio_ptr,
         int id,
         int *kind,
         int time_i[7],
         double *time_d,
         double *navlon,
         double *navlat,
         double *speed,
         double *heading,
         int *nbath,
         int *namp,
         int *nss,
         char *beamflag,
         double *bath,
         double *amp,
         double *bathacrosstrack,
         double *bathalongtrack,
         double *ss,
         double *ssacrosstrack,
         double *ssalongtrack,
         char *comment,
         int *error);

The  function  mb\_buffer\_extract  extracts  and returns a subset of the
data in a buffer record.  The verbose value controls the standard error
output  verbosity of the function.  The buffer record is specified with
the buffer index id. The data is either  in  the  form  of  bathymetry,
amplitude, and sidescan survey data or a comment string.

The input control parameters have the following significance:

* id: The buffer index of the data record to extract.

The returned values are:

* kind:          kind of data record extracted
	* 1    survey data
	* 2    comment
	* >=3  other data that cannot be passed by mb\_buffer\_extract
* time\_i:        time of current ping
* time\_i[0]: year
* time\_i[1]: month
* time\_i[2]: day
* time\_i[3]: hour
* time\_i[4]: minute
* time\_i[5]: second
* time\_i[6]: microsecond
* time\_d: time of current ping in seconds since 1/1/70 00:00:00
* navlon: longitude
* navlat: latitude
* speed: ship speed in km/s
* heading: ship heading in degrees
* nbath: number of bathymetry values
* namp: number of amplitude values
* nss: number of sidescan values
* beamflag: array of bathymetry flags
* bath: array of bathymetry values in meters
* amp: array of amplitude values in unknown units
* bathacrosstrack: array of of acrosstrack distances in meters corresponding to bathymetry
* bathalongtrack: array of of alongtrack distances in meters corresponding to bathymetry
* ss: array of sidescan values in unknown units
* ssacrosstrack: array of of acrosstrack distances in meters corresponding to sidescan
* ssacrosstrack: array of of alongtrack distances in meters corresponding to sidescan
* comment: comment string
* error: error value

A  status  value  indicating  success or failure is returned; the error
value argument error passes more  detailed  information  about  extract
failures.

#### mb\_buffer\_insert()

	int mb_buffer_insert(
         int verbose,
         char *buff_ptr,
         char *mbio_ptr,
         int id,
         int time_i[7],
         double time_d,
         double navlon,
         double navlat,
         double speed,
         double heading,
         int nbath,
         int namp,
         int nss,
         char *beamflag,
         double *bath,
         double *amp,
         double *bathacrosstrack,
         double *bathalongtrack,
         double *ss,
         double *ssacrosstrack,
         double *ssalongtrack,
         char *comment,
         int *error);

The  function  mb\_buffer\_insert  inserts  data  into  a  buffer record,
replacing a subset of the original values.  The verbose value  controls
the standard error output verbosity of the function.  The buffer record
is specified with the buffer index id. The data is either in  the  form
of bathymetry, amplitude, and sidescan survey data or a comment string.

The input control parameters have the following significance:

* id: The buffer index of the data record to insert.

The returned values are:

* kind: kind of data record inserted
	* 1 survey data
	* 2 comment
	* >=3  other data that cannot be passed by mb\_buffer\_insert
* time\_i:        time of current ping
	* time\_i[0]: year
	* time\_i[1]: month
	* time\_i[2]: day
	* time\_i[3]: hour
	* time\_i[4]: minute
	* time\_i[5]: second
	* time\_i[6]: microsecond
* time\_d: time of current ping in seconds since 1/1/70 00:00:00
* navlon: longitude
* navlat: latitude
* speed: ship speed in km/s
* heading: ship heading in degrees
* nbath: number of bathymetry values
* namp: number of amplitude values
* nss: number of sidescan values
* beamflag: array of bathymetry flags
* bath: array of bathymetry values in meters
* amp: array of amplitude values in unknown units
* bathacrosstrack: array of of acrosstrack distances in meters corresponding to bathymetry
* bathalongtrack: array of of alongtrack distances in meters corresponding to bathymetry
* ss: array of sidescan values in unknown units
* ssacrosstrack: array of of acrosstrack distances in meters corresponding to sidescan
* ssacrosstrack: array of of alongtrack distances in meters corresponding to sidescan
* comment: comment string
* error: error value

A status value indicating success or failure  is  returned;  the  error
value  argument  error  passes  more  detailed information about insert
failures.

#### mb\_buffer\_get\_next\_nav()

	int mb_buffer_get_next_nav(
         int verbose,
         char *buff_ptr,
         char *mbio_ptr,
         int start,
         int *id,
         int time_i[7],
         double *time_d,
         double *navlon,
         double *navlat,
         double *speed,
         double *heading,
         double *draft,
         double *roll,
         double *pitch,
         double *heave,
         int *error);

The function mb\_buffer\_get\_next\_nav searches for the next  survey  data
record  in  the  buffer,  beginning at buffer index start. Since buffer
indexes begin at 0, the first  call  to  mb\_buffer\_get\_next\_nav  should
have  start  = 0.  If a survey data record is found at or beyond start,
mb\_buffer\_get\_next\_nav returns the buffer index of that record  in  id.
Navigation and vertical reference sensor data is also returned. No com-
ments or other non-survey data records are returned.  The verbose value
controls the standard error output verbosity of the function.

The input control parameters have the following significance:

* start: The buffer index at which to start searching for a survey data record.

The returned values are:

* id: The  buffer  index of the first survey data record at or after start.
* time\_i: time of current ping
* time\_i[0]: year
* time\_i[1]: month
* time\_i[2]: day
* time\_i[3]: hour
* time\_i[4]: minute
* time\_i[5]: second
* time\_i[6]: microsecond
* time\_d: time of current ping in seconds since 1/1/70 00:00:00
* navlon: longitude
* navlat: latitude
* speed: ship speed in km/s
* heading: ship heading in degrees
* roll: ship roll in degrees
* pitch: ship pitch in degrees
* heave: ship heave in meters

A status value indicating success or failure  is  returned;  the  error
value  argument  error passes more detailed information about failures.
The most common error occurs when no more survey data records remain to
be found in the buffer; in this case, error = -14.

#### mb\_buffer\_extract\_nav()

	int mb_buffer_extract_nav(
         int verbose,
         char *buff_ptr,
         char *mbio_ptr,
         int id,
         int *kind,
         int time_i[7],
         double *time_d,
         double *navlon,
         double *navlat,
         double *speed,
         double *heading,
         double *draft,
         double *roll,
         double *pitch,
         double *heave,
         int *error);

The function mb\_buffer\_extract\_nav extracts and returns a subset of the
data in a buffer record.  The verbose value controls the standard error
output  verbosity of the function.  The buffer record is specified with
the buffer index id. The data returned consists of navigation and  ver-
tical reference sensor data.

The input control parameters have the following significance:

* id:       The buffer index of the data record to extract.

The returned values are:

* kind:          kind of data record extracted
* 1    survey data
* 2    comment
* >=3  other data that cannot be passed by mb\_buffer\_extract\_nav
* time\_i:        time of current ping
	* time\_i[0]: year
	* time\_i[1]: month
	* time\_i[2]: day
	* time\_i[3]: hour
	* time\_i[4]: minute
	* time\_i[5]: second
	* time\_i[6]: microsecond
* time\_d:        time of current ping in seconds since 1/1/70 00:00:00
* navlon:        longitude
* navlat:        latitude
* speed:         ship speed in km/s
* heading:       ship heading in degrees
* roll:          ship roll in degrees
* pitch:         ship pitch in degrees
* heave:         ship heave in meters

A  status  value  indicating  success or failure is returned; the error
value argument error passes more  detailed  information  about  extract
failures.

#### mb\_buffer\_insert\_nav()

	int mb_buffer_insert_nav(
         int verbose,
         char *buff_ptr,
         char *mbio_ptr,
         int id,
         int time_i[7],
         double time_d,
         double navlon,
         double navlat,
         double speed,
         double heading,
         double draft,
         double roll,
         double pitch,
         double heave,
         int *error);

The  function mb\_buffer\_insert\_nav inserts navigation and vertical ref-
erence sensor data into a buffer record,  replacing  a  subset  of  the
original  values.  The verbose value controls the standard error output
verbosity of the function.  The buffer record  is  specified  with  the
buffer index id.

The input control parameters have the following significance:

* id: The buffer index of the data record to insert.

The returned values are:

* kind: kind of data record inserted
* 1 survey data
* 2 comment
* >=3  other data that cannot be passed by mb\_buffer\_insert\_nav
* time\_i: time of current ping
	* time\_i[0]: year
	* time\_i[1]: month
	* time\_i[2]: day
	* time\_i[3]: hour
	* time\_i[4]: minute
	* time\_i[5]: second
	* time\_i[6]: microsecond
* time\_d: time of current ping in seconds since 1/1/70 00:00:00
* navlon: longitude
* navlat: latitude
* speed: ship speed in km/s
* heading: ship heading in degrees
* roll: ship roll in degrees
* pitch: ship pitch in degrees
* heave: ship heave in meters

A  status  value  indicating  success or failure is returned; the error
value argument error passes  more  detailed  information  about  insert
failures.

#### mb\_buffer\_get\_ptr()

	int mb_buffer_get_ptr(
         int verbose,
         char *buff_ptr,
         char *mbio_ptr,
         int id,
         char **store_ptr,
         int *error);

The  function mb\_buffer\_get\_ptr returns a pointer to the data structure
in a buffer record.  The verbose value controls the standard error out-
put verbosity of the function.  The buffer record is specified with the
buffer index id. The data returned consists of a pointer  to  the  data
structure stored in the specified buffer record.

The input control parameters have the following significance:

* id: The buffer index of the data record to locate.

The return values are:

* \*store\_ptr:    pointer to data in specified buffer record
* error:         error value

A  status  value  indicating  success or failure is returned; the error
value argument error passes  more  detailed  information  about  buffer
failures.

### Miscellaneous Functions

#### mb\_defaults()

	int mb_defaults(
         int verbose,
         int *format,
         int *pings,
         int *lonflip,
         double bounds[4],
         int *btime_i,
         int *etime_i,
         double *speedmin,
         double *timegap);

The  function mb\_defaults provides default values of control parameters
used by some of the MBIO functions.  The  verbose  value  controls  the
standard  error output verbosity of the function.  The other parameters
are set by the function; the meaning of these parameters  is  discussed
in the listings of the functions mb\_read\_init and mb\_write\_init.  If an
.mbio\_defaults file exists in the user's home  directory,  the  lonflip
and  timegap  defaults  are read from this file.  Otherwise, the values
are set as:

* \*lonflip = 0
* \*timegap = 1
    
The other values are simply set as:

* \*format = 0
* \*pings = 1
* bounds[0] = -360.
* bounds[1] = 360.
* bounds[2] = -90.
* bounds[3] = 90.
* btime\_i[0] = 1962;
* btime\_i[1] = 2;
* btime\_i[2] = 21;
* btime\_i[3] = 10;
* btime\_i[4] = 30;
* btime\_i[5] = 0;
* btime\_i[6] = 0;
* etime\_i[0] = 2062;
* etime\_i[1] = 2;
* etime\_i[2] = 21;
* etime\_i[3] = 10;
* etime\_i[4] = 30;
* etime\_i[5] = 0;
* etime\_i[6] = 0;
* \*speedmin = 0.0

A status value is returned to indicate success or failure.

#### mb\_env()

	int mb_env(
         int verbose,
         char *psdisplay,
         char *imgdisplay,
         char *mbproject);

The function mb\_env provides default values  of  Postscript  and  image
display  programs  invoked by some MB-System programs and macros, and a
default value for a working project name that will be  used  by  future
applications. The verbose value controls the standard error output ver-
bosity of the function.  If an .mbio\_defaults file exists in the user's
home  directory,  the  \*psdisplay, \*imgdisplay, \*mbproject defaults are
read from this file.  Otherwise, the values are set as:

* psdisplay =  "xpsview" (IRIX OS)
	* "pageview" (Solaris OS)
	* "gv" (other OS)
	* "ghostview" (other OS)
* imgdisplay = "gimp" (Linux OS)
	* "xv" (other than Linux OS)
* mbproject =  "none"

#### mb\_format()

	int mb_format(
         int verbose,
         int *format,
         int *error);

Given the format identifier format, mb\_format checks if the  format  is
valid.   If  the  format  id  corresponds  to  a value used in previous
(<4.00) versions of MB-System, then the format value will be aliased to
the current corresponding value.

The return values are:

* format:        MBIO format id
* error:         error value

A  status  value  indicating  success or failure is returned; the error
value argument error passes more detailed information about failures.

#### mb\_format\_register()

	int mb_format_register(
         int verbose,
         int *format,
         char *mbio_ptr,
         int *error);

The  function  mb\_format\_register  is  called   by   mb\_read\_init   and
mb\_write\_init  and  serves to load format specific parameters and func-
tion parameters into the MBIO control structure pointed to  by  \*error.
The  format  id  \*format  is first checked for validity. In some cases,
formerly valid but now obsolete format id values are mapped to  current
values.

The input values are:

* \*format: MBIO format id
* \*mbio\_ptr: pointer to data in specified buffer record

The return values are:

* error: error value

A  status  value  indicating  success or failure is returned; the error
value argument error passes more detailed information about failures.

#### mb\_format\_info()

	int mb_format_info(
         int verbose,
         int *format,
         int *system,
         int *beams_bath_max,
         int *beams_amp_max,
         int *pixels_ss_max,
         char *format_name,
         char *system_name,
         char *format_description,
         int *numfile,
         int *filetype,
         int *variable_beams,
         int *traveltime,
         int *beam_flagging,
         int *nav_source,
         int *heading_source,
         int *vru_source,
         double *beamwidth_xtrack,
         double *beamwidth_ltrack,
         int *error);

The function mb\_format\_info returns a variety of data  format  specific
parameters.  The  format  id  *format is first checked for validity. In
some cases, formerly valid but now obsolete format id values are mapped
to current values.

The input values are:
    *format:       MBIO format id

The return values are:

* \*format:       MBIO format id
* \*system:       MBIO sonar system id
* \*beams\_bath\_max:    maximum number of bathymetry beams
* \*beams\_amp\_max:     maximum number of amplitude beams
* \*pixels\_ss\_max:     maximum number of sidescan pixels
* \*format\_name:  MBIO format name
* \*system\_name:  MBIO sonar system name
* \*format\_description:     MBIO format description
* \*numfile:      number of parallel data files used in format
* \*filetype:     type of data files
* \*variable\_beams:    number of beams can vary [boolean]
* \*traveltime:   travel time data available [boolean]
* \*beam\_flagging:     beam flagging supported [boolean]
* \*nav\_source:   kind of data records containing navigation
* \*heading\_source:    kind of data records containing heading
* \*vru\_source:   kind of data records containing attitude
* \*beamwidth\_xtrack:  typical athwartships beam width [degrees]
* \*beamwidth\_ltrack:  typical alongtrack beam width [degrees]
* \*error:         error value

A  status  value  indicating  success or failure is returned; the error
value argument error passes more detailed information about failures.

#### mb\_format\_system()

	int mb_format_system(
         int verbose,
         int *format,
         int *system,
         int *error);

The function mb\_format\_system returns the MBIO  sonar  system  id.  The
format  id  \*format  is first checked for validity. In some cases, for-
merly valid but now oattintbsolete format id values are mapped to  cur-
rent values. The input values are:

* \*format:       MBIO format id

The return values are:

* \*format:       MBIO format id
* \*system:       MBIO sonar system id
* \*error:         error value
    
A  status  value  indicating  success or failure is returned; the error
value argument error passes more detailed information about failures.

#### mb\_format\_description()

	int mb_format_description(
         int verbose,
         int *format,
         char *description,
         int *error);

The function mb\_format\_description returns a short description  of  the
format  in  the  string  \*description.  The  format id \*format is first
checked for validity. In some cases, formerly valid  but  now  obsolete
format id values are mapped to current values. The input values are:

* \*format: MBIO format id

The return values are:

* \*format: MBIO format id
* \*format\_description: MBIO format description
* \*error: error value

A  status  value  indicating  success or failure is returned; the error
value argument error passes more detailed information about failures.

#### mb\_format\_dimensions()

	int mb_format_dimensions(
         int verbose,
         int *format,
         int *beams_bath_max,
         int *beams_amp_max,
         int *pixels_ss_max,
         int *error);

The function mb\_format\_dimensions returns the maximum numbers of  beams
and  pixels  associated  with  a  particular data format. The format id
\*format is first checked for validity. In some  cases,  formerly  valid
but  now  obsolete  format  id values are mapped to current values. 

The input values are:

* \*format:       MBIO format id
    
The return values are:

* \*format:       MBIO format id
* \*beams\_bath\_max:    maximum number of bathymetry beams
* \*beams\_amp\_max:     maximum number of amplitude beams
* \*pixels\_ss\_max:     maximum number of sidescan pixels
* error:         error value
    
A status value indicating success or failure  is  returned;  the  error
value argument error passes more detailed information about failures.

#### mb\_format\_flags()

	int mb_format_flags(
         int verbose,
         int *format,
         int *variable_beams,
         int *traveltime,
         int *beam_flagging,
         int *error);

The  function  mb\_format\_flags returns flags indicating certain charac-
teristics of the specified data format. The format id \*format is  first
checked  for  validity.  In some cases, formerly valid but now obsolete
format id values are mapped to current values. 

The input values are:

* *format:       MBIO format id
    
The return values are:

* *format:       MBIO format id
* *variable\_beams:    number of beams can vary [boolean]
* *traveltime:   travel time data available [boolean]
* *beam\_flagging:     beam flagging supported [boolean]
* error:         error value
    
A status value indicating success or failure  is  returned;  the  error
value argument error passes more detailed information about failures.

#### mb\_format\_source()

	int mb_format_source(
         int verbose,
         int *format,
         int *nav_source,
         int *heading_source,
         int *vru_source,
         int *error);

The  function  mb\_format\_source  returns flags indicating what kinds of
data records contain navigation, heading, and attitude  values  in  the
specified  data  format.  The  format  id  \*format is first checked for
validity. In some cases, formerly valid but now obsolete format id val-
ues are mapped to current values. 

The input values are:

* *format:       MBIO format id
    
The return values are:

* *format:       MBIO format id
* *nav\_source:   kind of data records containing navigation
* *heading\_source:    kind of data records containing heading
* *vru\_source:   kind of data records containing attitude
* error:         error value
    
A  status  value  indicating  success or failure is returned; the error
value argument error passes more detailed information about failures.

#### mb\_format\_beamwidth()

	int mb_format_beamwidth(
         int verbose,
         int *format,
         double *beamwidth_xtrack,
         double *beamwidth_ltrack,
         int *error);

The function mb\_format\_beamwidth returns typical,  upper  bound  values
for  athwartships  and alongtrack beam widths. The format id \*format is
first checked for validity. In some cases, formerly valid but now obso-
lete  format  id  values are mapped to current values. 

The input values are:

* *format:       MBIO format id
    
The return values are:

* *format:       MBIO format id
* *beamwidth\_xtrack:  typical athwartships beam width [degrees]
* *beamwidth\_ltrack:  typical alongtrack beam width [degrees]
* error:         error value
    
A status value indicating success or failure  is  returned;  the  error
value argument error passes more detailed information about failures.

#### mb\_datalist\_open()

	int mb_datalist_open(
         int verbose,
         char **datalist,
         char *path,
         int look_processed,
         int *error);

The function mb\_datalist\_open initializes reading from a datalist tree.
The string \*path is the path to the  top  level  datalist  file  to  be
opened.   The value look\_processed indicates whether the datalist pars-
ing should look for or ignore processed data files (see  the  mbprocess
and mbdatalist manual pages).  

The input values are:

* *path:         datalist file to be opened
* look\_processed:     processed file behavior
	* 0 : unset
	* 1 : ignore processed files
	* 2 : return processed files
                        
The return values are:

* **datalist:    pointer to datalist structure
* error:         error value
    
A  status  value  indicating  success or failure is returned; the error
value argument error passes more detailed information about failures.

#### mb\_datalist\_read()

	int mb_datalist_read(
         int verbose,
         char *datalist,
         char *path,
         int *format,
         double *weight,
         int *error);

The function mb\_datalist\_read reads from a datalist tree, attempting to
return  the  path  to the next valid swath data file, the corresponding
data format id, and a gridding weight (see the mbprocess and mbdatalist
manual  pages).   Information  about the datalist tree is embedded in a
data structure pointed to by \*datalist. 

The input values are:

* *datalist:     pointer to datalist structure

The return values are:

* *path:         swath data file
* *format:       MBIO format id
* *weight:       mbgrid gridding weight
* error:         error value

A status value indicating success or failure  is  returned;  the  error
value argument error passes more detailed information about failures.

#### mb\_datalist\_close()

	int mb_datalist_close(
         int verbose,
         char **datalist,
         int *error);

The  function mb\_datalist\_close closes an open datalist tree, and deal-
locates the data structure pointed to by \*datalist.  

The input values are:

* *datalist:     pointer to datalist structure

The return values are:

* error:         error value

A  status  value  indicating  success or failure is returned; the error
value argument error passes more detailed information about failures.

#### mb\_alloc()

	int mb_alloc(
         int verbose,
         char *mbio_ptr,
         char **store_ptr,
         int *error);

The function mb\_alloc allocates a data structure for  internal  storage
of  swath  sonar  data  and  returns  a  pointer  to  this structure in
\*store\_ptr.  The data structure is specific to the data format  identi-
fied  in  the  MBIO  data structure pointed to by \*mbio\_ptr.  

The input values are:

* *mbio\_ptr:     pointer to MBIO structure
    
The return values are:

* **store\_ptr:   pointer to storage data structure
* error:         error value
    
A status value indicating success or failure  is  returned;  the  error
value argument error passes more detailed information about failures.

#### mb\_deall()

	int mb_deall(
         int verbose,
         char *mbio_ptr,
         char **store_ptr,
         int *error);

The  function  mb\_deall  deallocates a format specific swath sonar data
structure pointed to by \*store\_ptr.  

The input values are:

* *mbio\_ptr:     pointer to MBIO structure
* *store\_ptr:    pointer to storage data structure
                   
The return values are:

* error:         error value
    
A status value indicating success or failure  is  returned;  the  error
value argument error passes more detailed information about failures.

#### mb\_error()

	int mb_error(
         int error,
         int error,
         char **message);

Given  the  error value error, mb\_format\_inf returns a short error mes-
sage in the string **message. The verbose value controls  the  standard
error  output  verbosity of the function.  The return status value sig-
nals success if format is valid and failure otherwise.

#### mb\_navint\_add()

	int mb_navint_add(
         int verbose,
         char *mbio_ptr,
         double time_d,
         double lon,
         double lat,
         int *error);

The function mb\_navint\_add adds a navigation fix to a  circular  buffer
of  navigation  values maintained in the MBIO data structure pointed to
by \*mbio\_ptr.  This buffer is used to interpolate navigation  for  data
formats where the navigation is asynchronous (where navigation and sur-
vey pings come in different data records).  

The input values are:

* *mbio\_ptr:     pointer to MBIO structure
* time\_d:        time of navigation fix in seconds since 1/1/70 00:00:00
* lon:      longitude (degrees)
* lat:      latitude (degrees)

The return values are:

 * error:         error value
    
A status value indicating success or failure  is  returned;  the  error
value argument error passes more detailed information about failures.

#### mb\_navint\_interp()

	int mb_navint_interp(
         int verbose,
         char *mbio_ptr,
         double time_d,
         double heading,
         double rawspeed,
         double *lon,
         double *lat,
         double *speed,
         int *error);

The  function  mb\_navint\_interp  interpolates  navigation  to  the time
time\_d using a circular buffer of navigation values maintained  in  the
MBIO  data  structure  pointed to by \*mbio\_ptr.  This buffer is used to
interpolate navigation for data formats where the navigation  is  asyn-
chronous  (where  navigation  and  survey  pings come in different data
records).  

The input values are:

* *mbio\_ptr:     pointer to MBIO structure
* time\_d:        time of current ping in seconds since 1/1/70 00:00:00
* heading:       heading in degrees
* rawspeed:      speed in km/hr (zero if not known)
                   
The return values are:

* *lon:          longitude (degrees)
* *lat:          latitude (degrees)
* *speed:        speed made good in km/hr
* error:         error value
    
A status value indicating success or failure  is  returned;  the  error
value argument error passes more detailed information about failures.

#### mb\_attint\_add()

	int mb_attint_add(
         int verbose,
         char *mbio_ptr,
         double time_d,
         double heave,
         double roll,
         double pitch,
         int *error);

The  function  mb\_attint\_add adds an attitude (heave, roll, pitch) data
point to a circular buffer of attitude values maintained  in  the  MBIO
data  structure pointed to by \*mbio\_ptr.  This buffer is used to inter-
polate attitude for data formats where  the  attitude  is  asynchronous
(where  attitude and survey pings come in different data records).  

The input values are:

* *mbio\_ptr:     pointer to MBIO structure
* time\_d:        time of attitude in seconds since 1/1/70 00:00:00
* heave:         heave (meters, up +)
* roll:          roll (degrees, starboard up +)
* pitch:         pitch (degrees, forward up +)
    
The return values are:

* error:         error value
    
A status value indicating success or failure  is  returned;  the  error
value argument error passes more detailed information about failures.

#### mb\_attint\_interp()

	int mb_attint_interp(
         int verbose,
         char *mbio_ptr,
         double time_d,
         double *heave,
         double *roll,
         double *pitch,
         int *error);

The  function  mb\_attint\_interp  interpolates  attitude  (heave,  roll,
pitch) data to the time time\_d using a circular buffer of attitude val-
ues  maintained  in  the  MBIO  data structure pointed to by \*mbio\_ptr.
This buffer is used to interpolate attitude for data formats where  the
attitude  is asynchronous (where attitude and survey pings come in dif-
ferent data records).  

The input values are:

* *mbio\_ptr:     pointer to MBIO structure
* time\_d:        time of current ping in seconds since 1/1/70 00:00:00
    
The return values are:

* *heave:        heave (meters, up +)
* *roll:         roll (degrees, starboard up +)
* *pitch:        pitch (degrees, forward up +)
* error:         error value
    
A status value indicating success or failure  is  returned;  the  error
value argument error passes more detailed information about failures.

#### mb\_hedint\_add()

	int mb_hedint_add(
         int verbose,
         char *mbio_ptr,
         double time_d,
         double heading,
         int *error);

The function mb\_hedint\_add adds a heading point to a circular buffer of
heading values maintained in the MBIO  data  structure  pointed  to  by
\*mbio\_ptr.  This buffer is used to interpolate heading for data formats
where the heading is asynchronous (where heading and survey pings  come
in different data records).  

The input values are:

* *mbio\_ptr:     pointer to MBIO structure
* time\_d:        time of heading value in seconds since 1/1/70 00:00:00
* heading:       heading (degrees)
    
The return values are:

* error:         error value
    
A  status  value  indicating  success or failure is returned; the error
value argument error passes more detailed information about failures.

#### mb\_hedint\_interp()

	int mb_hedint_interp(
         int verbose,
         char *mbio_ptr,
         double time_d,
         double *heading,
         int *error);

The function mb\_hedint\_interp interpolates heading to the  time  time\_d
using  a  circular buffer of heading values maintained in the MBIO data
structure pointed to by \*mbio\_ptr.  This buffer is used to  interpolate
heading for data formats where the heading is asynchronous (where head-
ing and survey pings come in different data records).  

The input values are:

* *mbio\_ptr:     pointer to MBIO structure
* time\_d:        time of current ping in seconds since 1/1/70 00:00:00
    
The return values are:

* *heading:      heading in degrees
* error:         error value
    
A  status  value  indicating  success or failure is returned; the error
value argument error passes more detailed information about failures.

#### mb\_get\_double()

	int mb_get_double(
         double *value,
         char *str,
         int nchar);

The function mb\_get\_double parses the first  nchar  characters  of  the
string  \*str for a floating point value, storing this value as a double
in *value.

#### mb\_get\_int()

	int mb_get_int(
         int *value,
         char *str,
         int nchar);

The function mb\_get\_int parses the first nchar characters of the string
\*str for an integer value, storing this value as a int in *value.

#### mb\_get\_binary\_short()

	int mb_get_binary_short(
         int swapped,
         void *buffer,
         short *value);

The  function  mb\_get\_binary\_short  extracts a short int value from the
first two bytes pointed to by \*buffer. If the boolean swapped is  true,
the byte order of *value is swapped.

#### mb\_get\_binary\_int()

	int mb_get_binary_int(
         int swapped,
         void *buffer,
         int *value);

The  function  mb\_get\_binary\_int  extracts  an int value from the first
four bytes pointed to by \*buffer. If the boolean swapped is  true,  the
byte order of *value is swapped.

#### mb\_get\_binary\_float()

	int mb_get_binary_float(
         int swapped,
         void *buffer,
         float *value);

The  function mb\_get\_binary\_float extracts a float value from the first
four bytes pointed to by \*buffer. If the boolean swapped is  true,  the
byte order of *value is swapped.

#### mb\_get\_binary\_double()

	int mb_get_binary_double(
         int swapped,
         void *buffer,
         double *value);

The  function  mb\_get\_binary\_double  extracts  a  double value from the
first eight bytes pointed to by \*buffer.  If  the  boolean  swapped  is
true, the byte order of *value is swapped.

#### mb\_put\_binary\_short()

	int mb_put_binary_short(
         int swapped,
         short value,
         void *buffer);

The  function  mb\_put\_binary\_short  inserts  a short int value into the
first two bytes pointed to by *buffer. If the boolean swapped is  true,
the byte order of value is swapped.

#### mb\_put\_binary\_int()

	int mb_put_binary_int(
         int swapped,
         int value,
         void *buffer);

The function mb\_put\_binary\_int inserts an int value into the first four
bytes pointed to by *buffer. If the boolean swapped is true,  the  byte
order of value is swapped.

#### mb\_put\_binary\_float()

	int mb_put_binary_float(
         int swapped,
         float value,
         void *buffer);

The  function  mb\_put\_binary\_float inserts a float value into the first
four bytes pointed to by *buffer. If the boolean swapped is  true,  the
byte order of value is swapped.

#### mb\_put\_binary\_double()

	int mb_put_binary_double(
         int swapped,
         double value,
         void *buffer);

The function mb\_put\_binary\_double inserts a double value into the first
eight bytes pointed to by \*buffer. If the boolean swapped is true,  the
byte order of value is swapped.

#### mb\_get\_bounds()

	int mb_get_bounds(
         char *text,
         double *bounds);

The  function  mb\_get\_bounds  parses the string \*text and extracts geo-
graphic bounds of a rectangular region in the form:

* bounds[0]: minimum longitude
* bounds[1]: maximum longitude
* bounds[2]: minimum latitude
* bounds[3]: maximum latitude
         
where \*text is in the standard GMT bounds form.  The longitude and lat-
itude values in *text should separated by a '/' character, and individ-
ual values may be represented in decimal degrees or in "dd:mm:ss"  form
(dd=degrees, mm=minutes, ss=seconds).

#### mb\_ddmmss\_to\_degree()

	double mb_ddmmss_to_degree(
         char *text);

The function mb\_ddmmss\_to\_degree parses the string *text and extracts a
decimal longitude or latitude  value  from  a  "dd:mm:ss"  (dd=degrees,
mm=minutes, ss=seconds) value.

#### mb\_takeoff\_to\_rollpitch()

	int mb_takeoff_to_rollpitch(
         int verbose,
         double theta,
         double phi,
         double *alpha,
         double *beta,
         int *error);

The  function mb\_takeoff\_to\_rollpitch translates angles from the "take-
off" coordinate reference frame to the "rollpitch"  coordinate  system.
See the discussion of coordinate systems below.

#### mb\_rollpitch\_to\_takeoff()

	int mb_rollpitch_to_takeoff(
         int verbose,
         double alpha,
         double beta,
         double *theta,
         double *phi,
         int *error);

The  function mb\_rollpitch\_to\_takeoff translates angles from the "roll-
pitch" coordinate reference frame to the "takeoff"  coordinate  system.
See the discussion of coordinate systems below.

#### mb\_double\_compare()

	int mb_double_compare(
         double *a,
         double *b);

The  function  mb\_double\_compare is used with the qsort function.  This
function returns 1 if a > b and -1 if a <= b.

#### mb\_int\_compare()

	int mb_int_compare(
         int *a,
         int *b);

The function mb\_int\_compare is used  with  the  qsort  function.   This
function returns 1 if a > b and -1 if a <= b.

### Coordinate Systems Used In MB-System

#### Introduction
The  coordinate  systems  described below are used within MB-System for
calculations involving the location in space of  depth,  amplitude,  or
sidescan  data.  In all cases the origin of the coordinate system is at
the center of the sonar transducers.

#### Cartesian Coordinates
The cartesian coordinate system used in MB-System is a bit odd  because
it  is  left-handed, as opposed to the right-handed x-y-z space conven-
tionally used in most circumstances. With respect to the sonar (or  the
ship  on  which  the sonar is mounted), the x-axis is athwartships with
positive to starboard (to the right if facing forward), the  y-axis  is
fore-aft with positive forward, and the z-axis is positive down.

#### Spherical Coordinates
There  are two non-traditional spherical coordinate systems used in MB-
System. The first, referred to here as takeoff  angle  coordinates,  is
useful  for  raytracing.   The  second,  referred to here as roll-pitch
coordinates, is useful for taking account of corrections  to  roll  and
pitch angles.

##### Takeoff Angle Coordinates

The  three  parameters  are  r, theta, and phi, where r is the distance
from the origin, theta is the angle from vertical down (that  is,  from
the  positive z-axis), and phi is the angle from acrosstrack (the posi-
tive x-axis) in the x-y plane.  Note that theta is always positive; the
direction  in  the  x-y plane is given by phi.  Raytracing is simple in
these coordinates because the ray takeoff angle is just theta. However,
applying  roll  or  pitch  corrections  is complicated because roll and
pitch have components in both theta and phi.

    0 <= theta <= PI/2
    -PI/2 <= phi <= 3*PI/2

    x = rSIN(theta)COS(phi)
    y = rSIN(theta)SIN(phi)
    z = rCOS(theta)

    theta = 0    ---> vertical, along positive z-axis
    theta = PI/2 ---> horizontal, in x-y plane
    phi = -PI/2  ---> aft, in y-z plane with y negative
    phi = 0      ---> port, in x-z plane with x positive
    phi = PI/2   ---> forward, in y-z plane with y positive
    phi = PI     ---> starboard, in x-z plane with x negative
    phi = 3*PI/2 ---> aft, in y-z plane with y negative

##### Roll-Pitch Coordinates

The three parameters are r, alpha, and beta, where r  is  the  distance
from  the origin, alpha is the angle forward (effectively pitch angle),
and beta is the angle from horizontal in  the  x-z  plane  (effectively
roll  angle).  Applying  a  roll or pitch correction is simple in these
coordinates because pitch is just alpha and roll is just beta. However,
raytracing  is  complicated because deflection from vertical has compo-
nents in both alpha and beta.

    -PI/2 <= alpha <= PI/2
    0 <= beta <= PI

    x = rCOS(alpha)COS(beta)
    y = rSIN(alpha)
    z = rCOS(alpha)SIN(beta)

    alpha = -PI/2 ---> horizontal, in x-y plane with y negative
    alpha = 0     ---> ship level, zero pitch, in x-z plane
    alpha = PI/2  ---> horizontal, in x-y plane with y positive
    beta = 0      ---> starboard, along positive x-axis
    beta = PI/2   ---> in y-z plane rotated by alpha
    beta = PI     ---> port, along negative x-axis

#### SeaBeam Coordinates

The per-beam parameters in the SB2100 data format  include  angle-from-
vertical  and  angle-forward.  Angle-from-vertical is the same as theta
except that it is signed based on the acrosstrack  direction  (positive
to  starboard,  negative  to  port).  The angle-forward values are also
defined slightly differently from phi, in that angle-forward is  signed
differently  on the port and starboard sides. The SeaBeam 2100 External
Interface Specifications document includes both discussion and  figures 
illustrating the angle-forward value. To summarize:
	
	Port:
	theta = absolute value of angle-from-vertical
	
	-PI/2 <= phi <= PI/2
	is equivalent to
	-PI/2 <= angle-forward <= PI/2
	
	phi = -PI/2 ---> angle-forward = -PI/2 (aft)
	phi = 0     ---> angle-forward = 0     (starboard)
	phi = PI/2  ---> angle-forward = PI/2  (forward)
	
	Starboard:
	theta = angle-from-vertical
	
	PI/2 <= phi <= 3*PI/2
	is equivalent to
	-PI/2 <= angle-forward <= PI/2
	
	phi = PI/2   ---> angle-forward = -PI/2 (forward)
	phi = PI     ---> angle-forward = 0     (port)
	phi = 3*PI/2 ---> angle-forward = PI/2  (aft)

#### Usage of Coordinate Systems in MB-System

Some  sonar  data formats provide angle values along with travel times.
The angles are converted to takoff-angle coordinates regardless of  the
storage  form  of the particular data format. Currently, most data for-
mats do not contain an alongtrack component to the position values;  in
these  cases the conversion is trivial since phi = beta = 0 and theta =
alpha. The angle and travel time values can be accessed using the  MBIO
function mb\_ttimes.  All angle values passed by MB-System functions are
in degrees rather than radians.

The programs mbbath and mbvelocitytool use  angles  in  take-off  angle
coordinates  to do the raytracing. If roll and/or pitch corrections are
to be made, the angles are converted to  roll-pitch  coordinates,  cor-
rected, and then converted back prior to raytracing.

### Beam Flags Used In MB-System

MB-System uses arrays of 1-byte "beamflag" values to indicate beam data
quality. Each beamflag value is actually an  eight  bit  mask  allowing
fairly  complicated  information to be stored regarding each bathymetry
value. In particular, beams may be flagged as bad, they may be selected
as  being  of special interest, and one or more reasons for flagging or
selection may be indicated.  This scheme is very similar to the conven-
tion used in the HMPS hydrographic data processing package and the SAIC
Hydrobat package. The beam selection mechanism is not currently used by
any MB-System programs.

The flag and select bits:
	
	xxxxxx00 => This beam is neither flagged nor selected.
	xxxxxx01 => This beam is flagged as bad and should be ignored.
	xxxxxx10 => This beam has been selected.

Flagging modes:
	
	00000001 => Flagged because no detection was made by the sonar.
	xxxxx101 => Flagged by manual editing.
	xxxx1x01 => Flagged by automatic filter.
	xxx1xx01 => Flagged because uncertainty exceeds 1 X IHO standard.
	xx1xxx01 => Flagged because uncertainty exceeds 2 X IHO standard.
	x1xxxx01 => Flagged because footprint is too large
	1xxxxx01 => Flagged by sonar as unreliable.

Selection modes:
	
	00000010 => Selected, no reason specified.
	xxxxx110 => Selected as least depth.
	xxxx1x10 => Selected as average depth.
	xxx1xx10 => Selected as maximum depth.
	xx1xxx10 => Selected as location of sidescan contact.
	x1xxxx10 => Selected, spare.
	1xxxxx10 => Selected, spare.


