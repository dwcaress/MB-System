.TH mbsystem l "31 October 1994"
.SH NAME
\fBmbsystem\fP - A set of utilities for manipulating and processing multibeam
bathymetry data.

.SH VERSION
Version 4.1

.SH INTRODUCTION
The \fBMB-System\fP is a software package consisting of programs which 
manipulate, process, list, or display multibeam bathymetry, amplitude, 
and sidescan data which has been developed at the Lamont-Doherty 
Earth Observatory (L-DEO). The heart of the system is an input/output library 
called \fBMBIO\fP which allows programs to transparently
work with any of a number of supported multibeam data formats.  This
approach has allowed the creation of "generic" utilities which can
be applied in a uniform manner to multibeam data from a variety
of sources.

The development of \fBMBIO\fP and its associated programs began in 1990
as part of ongoing research at L-DEO involving multibeam bathymetry data. 
Development was accelerated in 1991 as part of the effort to support
the Atlas Hydrosweep DS multibeam sonar on L-DEO's ship, the R/V
Maurice Ewing.  The National Science Foundation provided support in
1993 and 1994 to improve and extend \fBMB-System\fP so as to provide
a standard "generic" set of tools for processing and display of 
multibeam data that could be used by the U.S. academic community.
SeaBeam Instruments and Antarctic Support Associates are also supporting 
development of MB-System, with particular emphasis on capabilities 
related to the new SeaBeam 2100 series of sonars.

The source code for \fBMB-System\fP is written in C and should be portable
to a wide variety of Unix platforms. Graphical utilities are based 
on the Motif widget set.  To date, \fBMB-System\fP has been successfully
installed on Sun workstations running under SunOS4.1, Silicon Graphics (SGI)
workstations running under IRIX 5.2, and 486 PC's running under the Lynx 
operating system.

.SH AUTHORSHIP
David W. Caress (caress@lamont.ldeo.columbia.edu)
.br
Dale N. Chayes (dale@lamont.ldeo.columbia.edu)
.br
Lamont-Doherty Earth Observatory
.br
Palisades, NY 10964

.SH LIST OF MB-SYSTEM PROGRAMS AND MACROS
See the individual manual pages for detailed information about 
specific programs.
See the manual page for \fBMBIO\fP for information about the 
i/o library and the multibeam data formats supported by \fBMBIO\fP.

These are the current MB-system programs:
 \fBhsdump\fP:  Lists contents of the various sorts of 
 		data records in Hydrosweep DS data. 
 \fBmbbath\fP:  Generates bathymetry from travel times 
 		in multibeam data. 
 \fBmbclean\fP:  Identifies and flags bad beams in 
 		multibeam bathymetry data. 
 \fBmbcontour\fP:  Generate GMT compatible Postscript 
 		color swath contour plots.
 \fBmbcopy\fP:  Copy multibeam data files. 
 \fBmbdefaults\fP:  Set or list default mbio parameters 
 		for reading and writing multibeam data 
 \fBmbedit\fP:  Interactive editor used to flag bad 
 		beams in multibeam bathymetry data. 
 \fBmbformat\fP:   List information about multibeam 
 		data formats supported by the MBIO library. 
 \fBmbgetmask\fP:  Extract list of flagged or edited 
 		beams from a multibeam data file.
 \fBmbgrid\fP:  Grid bathymetry, amplitude, and sidescan 
 		data from multibeam data files. 
 \fBmbinfo\fP:  Output some basic statistics of 
 		multibeam data files. 
 \fBmblevitus\fP:  Create a water velocity profile 
 		which is representative of the mean 
 		annual water column for a specified 
 		1 degree by 1 degree region. 
 \fBmblist\fP:  List data in multibeam data files. 
 \fBmbmask\fP:  Apply editing information obtained 
 		from one file with \fBmbgetmask\fP 
 		to another file.
 \fBmbmerge\fP:  Merges multibeam data with new 
 		navigation. 
 \fBmbrollbias\fP:  Evaluate the roll bias of a 
 		multibeam system using two pieces 
 		of coincident bathymetry data 
 		collected with opposing ship headings.
 \fBmbswath\fP:  Generate GMT compatible Postscript 
 		color and color shaded relief swath plots.
 \fBmbunclean\fP:  Unflags edited beams in multibeam 
 		bathymetry data. 
 \fBmbvelocitytool\fP:  Interactive program for 
 		modeling the affect of the water 
 		velocity profile on multibeam 
 		bathymetry calculations.

Macros are programs or shellscripts which make use of programs 
from the MB-system and other software packages to accomplish common
tasks easily.  These are the current MB-system macros:
 \fBmbmfmtvel\fP:  Scans a Hydrosweep DS data file 
 		and outputs a formatted table of 
 		the mean water velocity and surface 
 		water velocity values used in 
 		processing that data.
 \fBmbm_grdplot\fP:  Reads a GMT GRD grid file and 
 		writes a shellscript which will 
 		generate a GMT plot of the data.
 \fBmbm_plot\fP:  Reads a multibeam data file and 
 		writes a shellscript which will 
 		generate a swath and/or contour 
 		plot of the data.
 \fBmbm_rollerror\fP:  Reads a multibeam data file,
 		calculates the noise in the vertical reference 
 		used by the sonar, and generates a file 
 		containing roll corrections which
 		can be applied to the data.
 \fBmbm_stat\fP:  Runs \fBmbinfo\fP on a multibeam data  
 		file and extracts beam statistics from 
 		the output of mbinfo.
 \fBmbm_vrefcheck\fP:  Generates a plot of high pass
 		filtered apparent crosstrack seafloor slope.
 \fBmbm_xbt\fP:  Processes a Sparton XBT data file 
 		and outputs a sound velocity profile 
 		file which can be used to process 
 		multibeam data.

.SH EXAMPLE APPROACH TO DATA PROCESSING
The following data processing stream is recommended for Hydrosweep DS
data. Similar approachs can be taken with data from other supported
sonars (e.g. SeaBeam 2112), provided that travel time information
is included in the data stream.

Consider a data file "hs_ew9204_134.mb21" containing one day's worth of
Hydrosweep data in the raw "real-time" ascii format (format 21).

 	Step 1: Copy the data from the raw ascii format 
 		(format 21) to a binary format (format 24) 
 		containing the same information.  Data i/o  
 		for format 24 is 15 times faster than 
 		format 21.  For example, give the command:
 		  mbcopy -F21/24 -Ihs_ew9204_134.mb21 
 			-Ohsih_ew9204_134.mb24

 	Step 2: Obtain a reasonable water velocity profile.  
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

 	Step 3: Construct bathymetry data by full raytracing 
 		through the water velocity profile.  Use 
 		the command:
 		  mbbath -F24 -Ihsnav_ew9204_134.mb24 
 			-Wwvel_ew9204 
 			-Ohsbat_ew9204_134_b.mb24

 	Step 4: Apply autoediting to flag the really bad 
 		bathymetric artifacts as bad.  Use the 
 		command:
 		  mbclean -F24 -Ihsbat_ew9204_134_b.mb24 
 			-Ohscln_ew9204_134_bc.mb24

 	Step 5: Interactively edit the data using 
 		\fBMBEDIT\fP. Start up \fBMBEDIT\fP and 
 		load the data file "hscln_ew9204_134_bc.mb24"; 
 		the output file will be
 		"hscln_ew9204_134_bce.mb24".

 	Step 6: Merge the data with good, processed navigation.
 		If nav_ew9204_134 is a file containing a day's 
 		worth of navigation in the L-DEO navigation 
 		format, then give the command:
 		  mbmerge -F24 -Ihsih_ew9204_134_bce.mb24 
 			-Nnav_ew9204_134 
 			-Ohsnav_ew9204_134_bcen.mb24

 	Step 7: Display the data as swath contour plots 
 		or swath color fill/shaded relief plots 
 		using \fBmbcontour\fP and \fBmbswath\fP 
 		in conjunction with GMT utilities. See 
 		the appropriate manual pages for examples.

 	Step 8: Grid and display the data.  Supposing    
 		that the area of interest is in the bounds 
 		 -140 < longitude < -139 and 
 		-10 < latitude < -9, that the desired grid 
 		has dimensions 301 X 301, and that the 
 		output should be a \fBGMT\fP version 2 
 		GRD file, use the command:
 		  cat hsnav_ew9204_134_bcen.mb24 24 > data.list
 		  mbgrid -R-140/-139/-10/-9 -D301/-D301 -L1 
			-C2 -Idata.list -Oew9204_134 -G3
 		This command will produce a shellscript 
 		which will run \fBGMT\fP programs to 
 		display the data.

.SH BUGS
It doesn't do everything we want it to yet, it doesn't work with
every kind of multibeam data ever collected, and sometimes it
breaks.
