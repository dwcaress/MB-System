.TH mbio l "27 August 2001" "MB-System 5.0" "MB-System 5.0"

.SH NAME
\fBmbio\fP - Format independent input/output library for swath mapping sonar data.

.SH VERSION
Version 5.0

.SH DESCRIPTION
\fBMBIO\fP (\fBM\fPulti\fBB\fPeam \fBI\fPnput/\fBO\fPutput) is a library
of functions used for reading and writing swath mapping sonar 
data files.  \fBMBIO\fP supports a large number of data
formats associated with different institutions and different sonar systems.
The purpose of \fBMBIO\fP is to allow users to write processing and display
programs which are independent of particular data formats and to provide
a standard approach to swath mapping sonar data i/o.

.SH AUTHORSHIP
David W. Caress (caress@mbari.org)
.br
  Monterey Bay Aquarium Research Institute
.br
Dale N. Chayes (dale@ldeo.columbia.edu)
.br
  Lamont-Doherty Earth Observatory

.SH DATA TERMINOLOGY
\fBMBIO\fP handles three types of swath mapping data:  beam bathymetry,
beam amplitude, and sidescan.  Both amplitude and sidescan represent
measures of backscatter strength. Beam amplitudes are backscatter
values associated with the same preformed beams used to
obtain bathymetry; \fBMBIO\fP assumes that a bathymetry value
exists for each amplitude value and uses the bathymetry beam
location for the amplitude.  Sidescan is generally constructed with
a higher spatial resolution than bathymetry, and carries its
own location parameters.  In the context of \fBMB-System\fP documentation,
the discrete values of bathymetry and amplitude are referred 
to as "beams", and the discrete values of
sidescan are referred to as "pixels". An additional difference
between "beam" and "pixel" data involves data flagging. An array
of "beamflags" is carried by \fBMBIO\fP functions which allows
the bathymetry (and by extension the amplitude) data to be flagged
as bad. The details of the beamflagging scheme are presented below.

.SH VERSION 5 CHANGES
This document describes version 5 releases of \fBMBIO\fP. 
The source code structure used in version 4 \fBMBIO\fP had grown
complex and difficult to support. In particular, the addition
of new i/o modules had become excessively time consuming.
Consequently, we have restructured and rewritten much of the library.
Most of the changes are internal, but two  function calls have
disappeared (\fBmb_put()\fP and \fBmb_write()\fP), and a 
number of new functions have been added.

.SH OVERVIEW
\fBMBIO\fP opens and initializes sonar data files for reading
and writing using the functions \fBmb_read_init\fP and \fBmb_write_init\fP,
respectively.  These functions return a pointer to a data structure
including all relevent information about the opened file, the
control parameters which determine how data is read or written,
and the arrays used for processing the data as it is read or written.
This pointer is then passed to
the functions used for reading or writing.  There is no limit
on the number of files which may be opened for reading or writing
at any given time in a program.  Data files are closed using the
function \fBmb_close\fP.

When it comes to actually reading and writing swath mapping
sonar data, \fBMBIO\fP has three levels of i/o functionality:
 	1:	Simple reading of swath data files. The positions 
 		of individual beams and pixels are returned in 
 		longitude and latitude  (\fBmb_read\fP) or 
 		acrosstrack and alongtrack distances (\fBmb_get\fP). 
 		Only a limited set of navigation information is
 		returned. Comments are also returned. These  
 		functions can be used without any special include  
 		files or any knowledge of the actual data structures 
 		used by the data formats or \fBMBIO\fP. The primary 
 		functions are:
 			\fBmb_read\fP
			\fBmb_get\fP
 	2:	Complete reading and writing of data structures 
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
 			\fBmb_read_ping()\fP
 			\fBmb_write_ping()\fP
 		The read and write routines which also extract
 		or insert information are:
 			\fBmb_get_all()\fP
 			\fBmb_put_all()\fP
 			\fBmb_put_comment()\fP
 		The information extraction and insertion 
 		functions are:
 			\fBmb_insert()\fP
 			\fBmb_extract()\fP
 			\fBmb_extract_nav()\fP
 			\fBmb_insert_nav()\fP
 			\fBmb_extract_altitude()\fP
 			\fBmb_insert_altitude()\fP
 			\fBmb_ttimes()\fP
 			\fBmb_copyrecord()\fP
 	3:	Buffered reading and writing of data structures 
 		containing all of the available information. 
 		This capability is useful for applications 
 		The primary functions are:
 			\fBmb_buffer_init()\fP
 			\fBmb_buffer_close()\fP
 			\fBmb_buffer_load()\fP
 			\fBmb_buffer_dump()\fP
 			\fBmb_buffer_info()\fP
 			\fBmb_buffer_get_next_data()\fP
 			\fBmb_buffer_extract()\fP
 			\fBmb_buffer_insert()\fP
 			\fBmb_buffer_get_next_nav()\fP
 			\fBmb_buffer_extract_nav()\fP
 			\fBmb_buffer_insert_nav()\fP

The level 1 \fBMBIO\fP functions allow users to read 
sonar data independent of format, with the limitation that 
only a limited set of navigation information is passed.  Thus, some of the
information contained in certain data formats (e.g. the "heave" value
in Hydrosweep DS data) is not passed by \fBmb_read()\fP or \fBmb_get()\fP.  
In general, the level 1 functions are useful for applications 
such as graphics which require only the navigation and the depth 
and/or backscatter values. 

The level 2 functions (\fBmb_get_all()\fP and
\fBmb_put_all()\fP) read and write the 
complete data structures, translate the data to internal data structures
associated with each of the supported sonar systems, and 
pass pointers to these internal data structures. Additional
functions allow a variety of information to be 
extracted from or inserted into
the data structures (e.g. mb_extract() and mb_insert()). Additional
information may be accessed using special include files to 
decode the data structures. The great majority of processing
programs use level 2 functions.

The level 3 functions provide buffered reading and 
writing which is useful for applications which generate
output files and need access to multiple pings 
at a time.  In addition to reading
(\fBmb_buffer_load()\fP) and writing (\fBmb_buffer_dump()\fP), functions
exist for extracting information from the buffer (\fBmb_buffer_extract()\fP)
and inserting information into the buffer (\fBmb_buffer_insert()\fP).

\fBMBIO\fP supports swath data in a number of different formats,
each specified by a unique id number. The function \fBmb_format()\fP
determines if a format id is valid. A set of similar functions
returns information about the specified format
(e.g. \fBmb_format_description()\fP, \fBmb_format_system()\fP,
\fBmb_format_description()\fP, \fBmb_format_dimensions()\fP,
\fBmb_format_flags()\fP, \fBmb_format_source()\fP, 
\fBmb_format_beamwidth()\fP).

Some \fBMB-System\fP programs can process multiple data
files specified in "datalist" files. Each line of a datalist
file contains a file path and the corresponding \fBMBIO\fP
format id. Datalist files can be recursive and can contain
comments. The functions used to extract input swath data
file paths from datalist files 
includes \fBmb_datalist_open()\fP, \fBmb_datalist_read()\fP, 
and \fBmb_datalist_close()\fP.

A number of other \fBMBIO\fP functions dealing with default values for
important parameters, error messages, memory management, and time conversions
also exist and are discussed below.

.SH SUPPORTED SWATH SONAR SYSTEMS
Each swath mapping sonar system outputs a data stream which includes some values
or parameters unique to that system.  In general, a number of different data
formats have come into use for data from each of the sonar systems; many of
these formats include only a subset of the original data stream.  Internally,
\fBMBIO\fP recognizes which sonar system each data format is associated
with and uses a data structure including the complete data stream for that
sonar.  Consequently, it is possible to read and write the complete 
data stream when using the level 2 or 3 \fBMBIO\fP functions.
At present, formats associated with the following sonars are
supported:
 	SeaBeam "classic" 16 beam multibeam sonar 
 	Hydrosweep DS 59 beam multibeam sonar 
 	Hydrosweep MD 40 beam mid-depth multibeam 
 		sonar 
 	SeaBeam 2000 multibeam sonar 
 	SeaBeam 2112, 2120, and 2130 multibeam 
 		sonars
 	Simrad EM12, EM121, EM950, and EM1000 
 		multibeam sonars
 	Simrad EM120, EM300, EM1002, and EM3000 
 		multibeam sonars
 	Hawaii MR-1 shallow tow interferometric 
 		sonar
 	ELAC Bottomchart 1180 and 1050 multibeam 
 		sonars
 	ELAC/SeaBeam Bottomchart Mk2 1180 and 
 		1050 multibeam sonars
 	Reson Seabat 9001/9002 multibeam sonars
 	Reson Seabat 8101 multibeam sonars
	Simrad/Mesotech SM2000 multibeam sonars
 	WHOI DSL AMS-120 deep tow interferometric 
 		sonar
	AMS-60 interferometric sonar
	
.SH SUPPORTED FORMATS
The following swath mapping sonar data formats are supported in this version of 
\fBMBIO\fP:

    MBIO Data Format ID:  11
    Format name:          MBF_SBSIOMRG
    Informal Description: SIO merge Sea Beam
    Attributes:           Sea Beam, bathymetry, 16 beams, 
                          binary, uncentered, SIO.
    
    MBIO Data Format ID:  12
    Format name:          MBF_SBSIOCEN
    Informal Description: SIO centered Sea Beam
    Attributes:           Sea Beam, bathymetry, 19 beams,  
                          binary, centered, SIO.
    
    MBIO Data Format ID:  13
    Format name:          MBF_SBSIOLSI
    Informal Description: SIO LSI Sea Beam
    Attributes:           Sea Beam, bathymetry, 19 beams,  
                          binary, centered, obsolete, SIO.
    
    MBIO Data Format ID:  14
    Format name:          MBF_SBURICEN
    Informal Description: URI Sea Beam
    Attributes:           Sea Beam, bathymetry, 19 beams,  
                          binary, centered, URI.
    
    MBIO Data Format ID:  15
    Format name:          MBF_SBURIVAX
    Informal Description: URI Sea Beam from VAX
    Attributes:           Sea Beam, bathymetry, 19 beams,  
                          binary, centered,  
                          VAX byte order, URI.
    
    MBIO Data Format ID:  16
    Format name:          MBF_SBSIOSWB
    Informal Description: SIO Swath-bathy SeaBeam
    Attributes:           Sea Beam, bathymetry, 19 beams,  
                          binary, centered, SIO.
    
    MBIO Data Format ID:  17
    Format name:          MBF_SBIFREMR
    Informal Description: IFREMER Archive SeaBeam
    Attributes:           Sea Beam, bathymetry, 19 beams,  
                          ascii, centered, IFREMER.
    
    MBIO Data Format ID:  21
    Format name:          MBF_HSATLRAW
    Informal Description: Raw Hydrosweep
    Attributes:           Hydrosweep DS, bathymetry and  
                          amplitude, 59 beams, ascii,  
                          Atlas Electronik.
    
    MBIO Data Format ID:  22
    Format name:          MBF_HSLDEDMB
    Informal Description: EDMB Hydrosweep
    Attributes:           Hydrosweep DS, bathymetry,  
                          59 beams, binary, NRL.
    
    MBIO Data Format ID:  23
    Format name:          MBF_HSURICEN
    Informal Description: URI Hydrosweep
    Attributes:           Hydrosweep DS, 59 beams,  
                          bathymetry, binary, URI.
    
    MBIO Data Format ID:  24
    Format name:          MBF_HSLDEOIH
    Informal Description: L-DEO in-house binary Hydrosweep
    Attributes:           Hydrosweep DS, 59 beams,  
                          bathymetry and amplitude, 
			  binary, centered, L-DEO.
    
    MBIO Data Format ID:  25
    Format name:          MBF_HSURIVAX
    Informal Description: URI Hydrosweep from VAX
    Attributes:           Hydrosweep DS, 59 beams,  
                          bathymetry, binary, VAX byte  
                          order, URI.
    
    MBIO Data Format ID:  32
    Format name:          MBF_SB2000SB
    Informal Description: SIO Swath-bathy SeaBeam 2000 
    Attributes:           SeaBeam 2000, bathymetry, 121  
                          beams, binary,  SIO.
    
    MBIO Data Format ID:  33
    Format name:          MBF_SB2000SS
    Informal Description: SIO Swath-bathy SeaBeam 2000 
    Attributes:           SeaBeam 2000, sidescan, 
                          1000 pixels for 4-bit  
                          sidescan, 2000 pixels for  
                          12+-bit sidescan, binary,  
                          SIO.
    
    MBIO Data Format ID:  41
    Format name:          MBF_SB2100RW
    Informal Description: SeaBeam 2100 series vender format
    Attributes:           SeaBeam 2100, bathymetry,  
                          amplitude and sidescan,  
                          151 beams and 2000 pixels,  
                          ascii with binary sidescan,  
                          SeaBeam Instruments.
    
    MBIO Data Format ID:  42
    Format name:          MBF_SB2100B1
    Informal Description: SeaBeam 2100 series vender format
    Attributes:           SeaBeam 2100, bathymetry,  
                          amplitude and sidescan,  
                          151 beams bathymetry, 
                          2000 pixels sidescan, binary, 
                          SeaBeam Instruments and L-DEO.
    
    MBIO Data Format ID:  43
    Format name:          MBF_SB2100B2
    Informal Description: SeaBeam 2100 series vender format
    Attributes:           SeaBeam 2100, bathymetry and 
                          amplitude, 151 beams bathymetry, 
                          binary, SeaBeam Instruments 
                          and L-DEO.
    
    MBIO Data Format ID:  51
    Format name:          MBF_EMOLDRAW
    Informal Description: Old Simrad vendor multibeam format
    Attributes:           Simrad EM1000, EM12S, EM12D, 
                          and EM121 multibeam sonars, 
                          bathymetry, amplitude, and  
                          sidescan, 60 beams for EM1000,  
                          81 beams for EM12S/D, 121 beams  
                          for EM121, variable pixels, 
                          ascii + binary, Simrad.
    
    MBIO Data Format ID:  53
    Format name:          MBF_EM12IFRM
    Informal Description: IFREMER TRISMUS format for  
                          Simrad EM12
    Attributes:           Simrad EM12S and EM12D, 
                          bathymetry, amplitude, and  
                          sidescan 81 beams, variable  
                          pixels, binary, IFREMER.
    
    MBIO Data Format ID:  54
    Format name:          MBF_EM12DARW
    Informal Description: Simrad EM12S RRS Darwin  
                          processed format
    Attributes:           Simrad EM12S, bathymetry  
                          and amplitude, 81 beams,  
                          binary, Oxford University.
    
    MBIO Data Format ID:  56
    Format name:          MBF_EM300RAW
    Informal Description: Simrad current multibeam  
                          vendor format
    Attributes:           Simrad EM120, EM300, EM1002,  
                          EM3000, bathymetry, amplitude,  
                          and sidescan, up to 254 beams,  
                          variable pixels, ascii + binary,  
                          Simrad.
    
    MBIO Data Format ID:  57
    Format name:          MBF_EM300MBA
    Informal Description: Simrad multibeam processing format
    Attributes:           Old and new Simrad multibeams, 
			  EM12S, EM12D, EM121, EM120,  
                          EM300, EM100, EM1000, EM950,  
                          EM1002, EM3000, bathymetry,  
                          amplitude, and sidescan, 
                          up to 254 beams, variable pixels,  
                          ascii + binary, MBARI.
    
    MBIO Data Format ID:  61
    Format name:          MBF_MR1PRHIG
    Informal Description: SOEST MR1 post processed format
    Attributes:           SOEST MR1, bathymetry and  
                          sidescan, variable beams and  
                          pixels, xdr binary, SOEST,  
                          University of Hawaii.
    
    MBIO Data Format ID:  62
    Format name:          MBF_MR1ALDEO
    Informal Description: L-DEO MR1 post processed  
                          format with travel times
    Attributes:           L-DEO MR1, bathymetry and  
                          sidescan, variable beams  
                          and pixels, xdr binary, L-DEO.
    
    MBIO Data Format ID:  63
    Format name:          MBF_MR1BLDEO
    Informal Description: L-DEO small MR1 post processed  
                          format with travel times
    Attributes:           L-DEO MR1, bathymetry and sidescan, 
                          variable beams and pixels,  
                          xdr binary, L-DEO.
    
    MBIO Data Format ID:  71
    Format name:          MBF_MBLDEOIH
    Informal Description: L-DEO in-house generic multibeam
    Attributes:           Data from all sonar systems,  
                          bathymetry, amplitude and  
                          sidescan, variable beams and  
                          pixels, binary, centered, L-DEO.
    
    MBIO Data Format ID:  81
    Format name:          MBF_CBAT9001
    Informal Description: Reson SeaBat 9001 shallow  
                          water multibeam
    Attributes:           60 beam bathymetry and  
                          amplitude, binary, University  
                          of New Brunswick.
    
    MBIO Data Format ID:  82
    Format name:          MBF_CBAT8101
    Informal Description: Reson SeaBat 8101 shallow  
                          water multibeam
    Attributes:           101 beam bathymetry and  
                          amplitude, binary, SeaBeam  
                          Instruments.
    
    MBIO Data Format ID:  83
    Format name:          MBF_HYPC8101
    Informal Description: Reson SeaBat 8101 shallow  
                          water multibeam
    Attributes:           101 beam bathymetry, 
                          ASCII, read-only, Coastal  
                          Oceanographics.
    
    MBIO Data Format ID:  91
    Format name:          MBF_BCHRTUNB
    Informal Description: Elac BottomChart shallow  
                          water multibeam
    Attributes:           56 beam bathymetry and  
                          amplitude, binary, University  
                          of New Brunswick.
    
    MBIO Data Format ID:  92
    Format name:          MBF_ELMK2UNB
    Informal Description: Elac BottomChart MkII shallow  
                          water multibeam
    Attributes:           126 beam bathymetry and  
                          amplitude, binary, University  
                          of New Brunswick.
    
    MBIO Data Format ID:  93
    Format name:          MBF_BCHRXUNB
    Informal Description: Elac BottomChart shallow  
                          water multibeam
    Attributes:           56 beam bathymetry and  
                          amplitude, binary, University  
                          of New Brunswick.
    
    MBIO Data Format ID:  94
    Format name:          MBF_L3XSERAW
    Informal Description: ELAC/SeaBeam XSE vendor format
    Attributes:           Bottomchart MkII 50 kHz and  
                          180 kHz multibeam, SeaBeam 2120
                          20 KHz multibeam, bathymetry, 
                          amplitude and sidescan, variable  
                          beams and pixels, binary, L3  
                          Communications (Elac Nautik 
                          and SeaBeam Instruments).
    
    MBIO Data Format ID:  101
    Format name:          MBF_HSMDARAW
    Informal Description: Atlas HSMD medium depth  
                          multibeam raw format
    Attributes:           40 beam bathymetry, 160 pixel  
                          sidescan, XDR (binary),  
                          STN Atlas Elektronik.
    
    MBIO Data Format ID:  102
    Format name:          MBF_HSMDLDIH
    Informal Description: Atlas HSMD medium depth  
                          multibeam processed format
    Attributes:           40 beam bathymetry, 160 pixel  
                          sidescan, XDR (binary), L-DEO.
    
    MBIO Data Format ID:  111
    Format name:          MBF_DSL120SF
    Informal Description: WHOI DSL AMS-120 processed format
    Attributes:           2048 beam bathymetry,  
                          8192 pixel sidescan, 
                          binary, single files, WHOI DSL.
    
    MBIO Data Format ID:  112
    Format name:          MBF_DSL120PF
    Informal Description: WHOI DSL AMS-120 processed  
                          format
    Attributes:           2048 beam bathymetry,  
                          8192 pixel sidescan, 
                          binary, parallel bathymetry  
                          and amplitude files, WHOI DSL.
    
    MBIO Data Format ID:  121
    Format name:          MBF_GSFGENMB
    Informal Description: SAIC Generic Sensor Format (GSF)
    Attributes:           variable beams,  bathymetry  
                          and amplitude, binary,  
                          single files, SAIC. 
    
    MBIO Data Format ID:  131
    Format name:          MBF_MSTIFFSS
    Informal Description: MSTIFF sidescan format
    Attributes:           variable pixels,  sidescan, 
                          binary TIFF variant,  
                          single files, Sea Scan. 
    
    MBIO Data Format ID:  141
    Format name:          MBF_OICGEODA
    Informal Description: OIC swath sonar format
    Attributes:           variable beam bathymetry and
			  amplitude, variable pixel  
                          sidescan, binary,
			  Oceanic Imaging Consultants
    
    MBIO Data Format ID:  142
    Format name:          MBF_OICMBARI
    Informal Description: OIC-style extended swath  
                          sonar format
    Attributes:           variable beam bathymetry and 
                          amplitude, variable pixel  
                          sidescan, binary, MBARI
    
    MBIO Data Format ID:  151
    Format name:          MBF_OMGHDCSJ
    Informal Description: UNB OMG HDCS format  
                          (the John Hughes Clarke format)
    Attributes:           variable beam bathymetry and 
                          amplitude, variable pixel  
                          sidescan, binary, UNB
    
    MBIO Data Format ID:  161
    Format name:          MBF_MGD77DAT
    Informal Description: NGDC MGD77 underway geophysics  
                          format
    Attributes:           single beam bathymetry, nav,  
                          magnetics, gravity, ascii,  
                          NOAA NGDC
    
    MBIO Data Format ID:  165
    Format name:          MBF_MBARIROV
    Informal Description: MBARI ROV navigation format
    Attributes:           ROV navigation, MBARI
    
    MBIO Data Format ID:  166
    Format name:          MBF_MBPRONAV
    Informal Description: MB-System simple navigation  
                          format
    Attributes:           navigation, MBARI
    
    MBIO Data Format ID:  171
    Format name:          MBF_HS10JAMS
    Informal Description: Furuno HS-10 multibeam format,
    Attributes:           45 beams bathymetry and amplitude, 
                          ascii, JAMSTEC
    
    MBIO Data Format ID:  182
    Format name:          MBF_HSDS2RAW
    Informal Description: STN Atlas raw multibeam format
    Attributes:           STN Atlas multibeam sonars, 
			  Hydrosweep DS2, Hydrosweep MD, 
			  Fansweep 10, Fansweep 20, 
			  bathymetry, amplitude, and sidescan,
			  up to 1440 beams and 4096 pixels,
			  XDR binary, STN Atlas.
    
    MBIO Data Format ID:  183
    Format name:          MBF_HSDS2LAM
    Informal Description: L-DEO HSDS2 processing format
    Attributes:           STN Atlas multibeam sonars, 
			  Hydrosweep DS2, Hydrosweep MD, 
			  Fansweep 10, Fansweep 20, 
			  bathymetry, amplitude, and sidescan,
			  up to 1440 beams and 4096 pixels,
			  XDR binary, L-DEO.


The institutional acronyms used above have the following meanings:
 	L-DEO	Lamont-Doherty Earth Observatory
 	MBARI	Monterey Bay Aquarium Research Institute
 	SIO	Scripps Institution of Oceanography
 	WHOI	Woods Hole Oceanographic Institution
 	URI	University of Rhode Island
 	NRL	Naval Research Laboratory
 	UNB	University of New Brunswick
 	UH	University of Hawaii
 	NOAA	National Oceans and Atmospheres Agency
 	NGDC	National Geophysical Data Center
 	USGS	United States Geological Survey
 	IFREMER	French government agency responsible
 		for operation of French oceanographic
 		research fleet.

.SH FUNCTION STATUS AND ERROR CODES
All of the \fBMBIO\fP functions return an integer status value with the 
convention that:
 	status = 1:	success
 	status = 0:	failure
.br
All \fBMBIO\fP functions also pass an error value argument which gives
somewhat more information about problems than the status value.  The
full suite of possible error values and the associated error messages are:
  	error = 0:	"No error",
  	error = -1:	"Time gap in data",
  	error = -2:	"Data outside specified location 
 			bounds",
  	error = -3:	"Data outside specified time interval",
  	error = -4:	"Ship speed too small",
  	error = -5:	"Comment record",
  	error = -6:	"Neither a data record nor a comment 
 			record",
  	error = -7:	"Unintelligible data record",
  	error = -8:	"Ignore this data",
  	error = -9:	"No data requested for buffer load",
  	error = -10:	"Data buffer is full",
  	error = -11:	"No data was loaded into the buffer",
  	error = -12:	"Data buffer is empty",
  	error = -13:	"No data was dumped from the buffer"
  	error = -14:	"No more survey data records in buffer"
  	error = -15:	"Data inconsistencies prevented 
 			inserting data into storage structure"
  	error = 1:	"Unable to allocate memory, 
 			initialization failed",
  	error = 2:	"Unable to open file, 
 			initialization failed",
  	error = 3:	"Illegal format identifier, 
 			initialization failed",
  	error = 4:	"Read error, probably end-of-file",
  	error = 5:	"Write error",
  	error = 6:	"No data in specified location bounds",
  	error = 7:	"No data in specified time interval",
  	error = 8:	"Invalid MBIO descriptor",
  	error = 9:	"Inconsistent usage of MBIO descriptor",
  	error = 10:	"No pings binned but no fatal error 
 			- this should not happen!",
  	error = 11:	"Invalid data record type specified 
 			for writing",
  	error = 12:	"Invalid control parameter specified 
 			by user",
  	error = 13:	"Invalid buffer id",
  	error = 14:	"Invalid system id - this should 
 			not happen!"
  	error = 15:	"This data file is not in the specified format!"
.br
In general, programs should treat negative error values as non-fatal
(reading and writing can continue) and positive error values as fatal (the data
files should be closed and the program terminated).
.br
.SH FUNCTION VERBOSITY
All of the \fBMBIO\fP functions are passed a \fIverbose\fP parameter which
controls how much debugging information is output to standard error.  If
\fIverbose\fP is 0 or 1, the \fBMBIO\fP functions will be silent.  If 
\fIverbose\fP is 2, then each function will output information as it is
entered and as it returns, along with the parameter values passed into and
returned out of the function.  Greater values of \fIverbose\fP will cause
additional information to be output, including values at various stages
of data processing during read and write operations.  In general, programs
using \fBMBIO\fP functions should adopt the following verbosity conventions:
.br
 	\fIverbose\fP = 0:	"silent" or near-"silent" execution
 	\fIverbose\fP = 1:	simple output including
 				program name, version
 				and simple progress updates
 	\fIverbose\fP >= 2:	debug mode with copious output
 				including every function call
 				and status listings
.SH INITIALIZATION AND CLOSING FUNCTIONS

int \fBmb_read_init\fP(
 		int \fIverbose\fP, 
 		char \fI*file\fP, 
		int \fIformat\fP, 
		int \fIpings\fP, 
		int \fIlonflip\fP, 
 		double \fIbounds[4]\fP,
		int \fIbtime_i[7]\fP, 
		int \fIetime_i[7]\fP, 
		double \fIspeedmin\fP, 
		double \fItimegap\fP,
		char \fI**mbio_ptr\fP, 
 		double \fI*btime_d\fP, 
		double \fI*etime_d\fP,
		int \fI*beams_bath\fP, 
		int \fI*beams_amp\fP, 
 		int \fI*pixels_ss\fP,
		int \fI*error\fP);

The function \fBmb_read_init\fP initializes the data file to be read 
and the data structures required for reading the data. The \fIverbose\fP
value controls the standard error output verbosity of the function.
  
The input control parameters have the following significance:
 	\fIfile\fP:		input filename
 	\fIformat\fP:		input \fBMBIO\fP data format id
 	\fIpings\fP:		ping averaging
 	\fIlonflip\fP:		longitude flipping
 	\fIbounds\fP:		location bounds of acceptable data
 	\fIbtime_i\fP:		beginning time of acceptable data
 	\fIetime_i\fP:		ending time of acceptable data
 	\fIspeedmin\fP:		minimum ship speed of acceptable data
 	\fItimegap\fP:		maximum time allowed before data gap

The format identifier \fIformat\fP specifies which of the supported
data formats is being read or written; the currently supported formats
are listed in the "SUPPORTED FORMATS" section.

The \fIpings\fP parameter determines whether and how pings are averaged as part
of data input.  This parameter is used only by the functions \fBmb_read\fP
and \fBmb_get\fP; \fBmb_get_all\fP and \fBmb_buffer_load\fP do not average
pings.  If \fIpings\fP = 1, then no ping averaging will be done and
each ping read will be returned unaltered by the reading function.  If
\fIpings\fP > 1, then the navigation and beam data for \fIpings\fP pings
will be read, averaged, and returned as the data for a single ping.  If
\fIpings\fP = 0, then the ping averaging will be varied so that the
along-track distance between averaged pings is as close as possible to the 
across-track distance between beams.

The \fIlonflip\fP paramenter determines the range in which longitude values
are returned:
 	\fIlonflip\fP = -1 : -360 to   0
 	\fIlonflip\fP =  0 : -180 to 180
 	\fIlonflip\fP =  1 :    0 to 360

The \fIbounds\fP array sets the area within which data are desired.
Data which lie outside the area specified by \fIbounds\fP will
be returned with an error by the reading function.  
The functions \fBmb_read\fP, 
\fBmb_get\fP and \fBmb_get_all\fP use the \fIbounds\fP array; the
function \fBmb_buffer_load\fP does no location checking.
 	\fIbounds\fP[0] : minimum longitude
 	\fIbounds\fP[1] : maximum longitude
 	\fIbounds\fP[2] : minimum latitude
 	\fIbounds\fP[3] : maximum latitude

The \fIbtime_i\fP array sets the desired beginning time for the data
and the \fIetime_i\fP array sets the desired ending time.
If the beginning time is earlier than the ending time, then any
data with a time stamp before the beginning time or 
after the ending time will be returned with an
MB_ERROR_OUT_TIME error by the reading function.
If the beginning time is after the ending time, then 
data with time stamps between the ending and beginning
time are returned with an error. This scheme allows time
windowing outside or inside a specified interval.
The functions \fBmb_read\fP, 
\fBmb_get\fP and \fBmb_get_all\fP use the 
\fIbtime_i\fP and \fIbtime_i\fP arrays; the
function \fBmb_buffer_load\fP does no time checking.
 	\fIbtime\fP[0] : year
 	\fIbtime\fP[1] : month
 	\fIbtime\fP[2] : day
 	\fIbtime\fP[3] : hour
 	\fIbtime\fP[4] : minute
 	\fIbtime\fP[5] : second
 	\fIbtime\fP[6] : microsecond
 	\fIetime\fP[0] : year
 	\fIetime\fP[1] : month
 	\fIetime\fP[2] : day
 	\fIetime\fP[3] : hour
 	\fIetime\fP[4] : minute
 	\fIetime\fP[5] : second
 	\fIetime\fP[6] : microsecond

The \fIspeedmin\fP parameter sets the minimum acceptable ship speed for 
the data.  If the ship speed associated with any ping is less than 
\fIspeedmin\fP, then that data will be returned with an error
by the reading function.
This is used to eliminate data collected while a ship is on station is
a simple way. The functions \fBmb_read\fP, 
\fBmb_get\fP and \fBmb_get_all\fP use the \fIspeedmin\fP value; the
function \fBmb_buffer_load\fP does no speed checking.

The \fItimegap\fP parameter sets the minimum time gap allowed before a 
gap in the data is declared.  Ping averaging is not done across
data gaps; an error is returned when time gaps are encountered. 
The functions \fBmb_read\fP and 
\fBmb_get\fP use the \fItimegap\fP value; the
functions \fBmb_get_all\fP and \fBmb_buffer_load\fP do no ping averaging
and thus have no need to check for time gaps.

The returned values are:
 	\fImbio_ptr\fP:	pointer to an \fBMBIO\fP descriptor structure
 	\fIbtime_d\fP:		desired beginning time in seconds since 1/1/70 00:00:0
 	\fIetime_d\fP:		desired ending time in seconds since 1/1/70 00:00:0
 	\fIbeams_bath\fP:	maximum number of bathymetry beams
 	\fIbeams_amp\fP:	maximum number of amplitude beams
 	\fIpixels_ss\fP:	maximum number of sidescan pixels
 	\fIerror\fP:		error value
.br
The structure pointed to by
\fImbio_ptr\fP holds the file descriptor and all of the control parameters
which govern how the data is read; this pointer must be provided to
the functions \fBmb_read\fP, \fBmb_get\fP, \fBmb_get_all\fP, 
or \fBmb_buffer_load\fP to read data. The values \fIbeams_bath\fP,
\fIbeams_amp\fP, and \fIpixels_ss\fP return the maximum number 
of bathymetry and amplitude beams and sidescan pixels, respectively,
that the specified data format may contain.  In general, \fIbeams_amp\fP
will either be zero or equal to \fIbeams_bath\fP.  In order to successfully
read data, the calling program must
provide pointers to arrays large enough to hold \fIbeams_bath\fP bathymetry
values, \fIbeams_amp\fP amplitude values, and \fIpixels_ss\fP sidescan
values. The values \fIbtime_d\fP
and \fIetime_d\fP give the desired beginning and end times of the data
converted to seconds since 00:00:00 on January 1, 1970;
\fBMBIO\fP uses these units to calculate time internally.
A status value indicating success or failure is returned; an error value
argument passes more detailed information about initialization failures. 

int \fBmb_write_init\fP(
 		int \fIverbose\fP,
		char \fI*file\fP,
 		int \fIformat\fP,
 		char \fI**mbio_ptr\fP,
		int \fI*beams_bath\fP,
 		int \fI*beams_amp\fP,
 		int \fI*pixels_ss\fP,
		int \fI*error\fP);

The function \fBmb_write_init\fP initializes the data file to be written 
and the data structures required for writing the data. The \fIverbose\fP
value controls the standard error output verbosity of the function.
  
The input control parameters have the following significance:
 	\fIfile\fP:		output filename
 	\fIformat\fP:		output \fBMBIO\fP data format id

The returned values are:
 	\fImbio_ptr\fP:	pointer to a structure describing the output file
 	\fIbeams_bath\fP:	maximum number of bathymetry beams
 	\fIbeams_back\fP:	maximum number of backscatter beams
 	\fIerror\fP:		error value
.br
The structure pointed to by
\fImbio_ptr\fP holds the output file descriptor; this pointer must be 
provided to
the functions \fBmb_write\fP, \fBmb_put\fP, \fBmb_put_all\fP, 
or \fBmb_buffer_dump\fP to write data. The values \fIbeams_bath\fP,
\fIbeams_amp\fP, and \fIpixels_ss\fP return the maximum number 
of bathymetry and amplitude beams and sidescan pixels, respectively,
that the specified data format may contain.  In general, \fIbeams_amp\fP
will either be zero or equal to \fIbeams_bath\fP.  In order to successfully
write data, the calling program must
provide pointers to arrays large enough to hold \fIbeams_bath\fP bathymetry
values, \fIbeams_amp\fP amplitude values, and \fIpixels_ss\fP sidescan
values. 
A status value indicating success or failure is returned; an error value
argument passes more detailed information about initialization failures. 

int \fBmb_close\fP(
 		int \fIverbose\fP, 
 		char *\fImbio_ptr\fP, 
 		int \fI*error\fP)

Closes the data file listed in the \fBMBIO\fP descriptor pointed to 
by \fImbio_ptr\fP and releases all specially allocated memory.  
The \fIverbose\fP
value controls the standard error output verbosity of the function.
A status value indicating success or failure is returned; an error value
argument passes more detailed information about failures. 
.SH LEVEL 1 FUNCTIONS

int \fBmb_read\fP(
 		int \fIverbose\fP,
 		char \fI*mbio_ptr\fP,
		int \fI*kind\fP,
 		int \fI*pings\fP,
		int \fItime_i[7]\fP,
 		double \fI*time_d\fP,
		double \fI*navlon\fP,
 		double \fI*navlat\fP,
		double \fI*speed\fP,
 		double \fI*heading\fP,
		double \fI*distance\fP,
 		double \fI*altitude\fP,
 		double \fI*sonardepth\fP,
		int \fI*nbath\fP,
 		int \fI*namp\fP,
 		int \fI*nss\fP,
		char \fI*beamflag\fP,
 		double \fI*bath\fP,
 		double \fI*amp\fP,
		double \fI*bathlon\fP,
 		double \fI*bathlat\fP,
		double \fI*ss\fP,
 		double \fI*sslon\fP,
 		double \fI*sslat\fP,
		char \fI*comment\fP,
 		int \fI*error\fP);

The function \fBmb_read\fP reads, processes, and returns sonar
data according to the \fBMBIO\fP descriptor pointed to by \fImbio_ptr\fP.
The \fIverbose\fP
value controls the standard error output verbosity of the function.
The data is in the form of bathymetry, amplitude, and sidescan values combined
with the longitude and latitude locations of the bathymetry and 
sidescan measurements (amplitudes are coincident with the bathymetry).
The return values are:
 	\fIkind\fP:		kind of data record read
 					1	survey data
 					2	comment
 					>=3	other data that cannot 
 						be passed by \fBmb_read\fP
 	\fIpings\fP:		number of pings averaged to give current data
 	\fItime_i\fP:		time of current ping
 			\fItime_i\fP[0]: year
 			\fItime_i\fP[1]: month
 			\fItime_i\fP[2]: day
 			\fItime_i\fP[3]: hour
 			\fItime_i\fP[4]: minute
 			\fItime_i\fP[5]: second
 			\fItime_i\fP[6]: microsecond
 	\fItime_d\fP:		time of current ping in seconds since 1/1/70 00:00:00
 	\fInavlon\fP:		longitude
 	\fInavlat\fP:		latitude
 	\fIspeed\fP:		ship speed in km/s
 	\fIheading\fP:		ship heading in degrees
 	\fIdistance\fP:		distance along shiptrack since last ping in km
 	\fInbath\fP:		number of bathymetry values
 	\fInamp\fP:		number of amplitude values
 	\fInss\fP:		number of sidescan values
	\fIbeamflag\fP:		array of bathymetry flags
 	\fIbath\fP:		array of bathymetry values in meters
 	\fIamp\fP:		array of amplitude values in unknown units
 	\fIbathlon\fP:		array of of longitude values corresponding 
 			to bathymetry
 	\fIbathlat\fP:		array of of latitude values corresponding 
 			to bathymetry
 	\fIss\fP:		array of sidescan values in unknown units
 	\fIsslon\fP:		array of of longitude values corresponding 
 			to sidescan
 	\fIsslat\fP:		array of of latitude values corresponding 
 			to sidescan
 	\fIcomment\fP:	comment string
 	\fIerror\fP:		error value
.br
A status value indicating success or failure is returned; the error value
argument \fIerror\fP passes more detailed information about 
read failures. 

int \fBmb_get\fP(
 		int \fIverbose\fP,
 		char \fI*mbio_ptr\fP,
 		int \fI*kind\fP,
 		int \fI*pings\fP,
		int \fItime_i[7]\fP,
 		double \fI*time_d\fP,
		double \fI*navlon\fP,
 		double \fI*navlat\fP,
		double \fI*speed\fP,
 		double \fI*heading\fP,
		double \fI*distance\fP,
 		double \fI*altitude\fP,
 		double \fI*sonardepth\fP,
		int \fI*nbath\fP,
 		int \fI*namp\fP,
 		int \fI*nss\fP,
		char \fI*beamflag\fP,
 		double \fI*bath\fP,
 		double \fI*amp\fP,
		double \fI*bathacrosstrack\fP,
 		double \fI*bathalongtrack\fP,
		double \fI*ss\fP,
 		double \fI*ssacrosstrack\fP,
 		double \fI*ssalongtrack\fP,
		char \fI*comment\fP,
 		int \fI*error\fP);

The function \fBmb_get\fP reads, processes, and returns sonar
data according to the \fBMBIO\fP descriptor pointed to by \fImbio_ptr\fP.
The \fIverbose\fP
value controls the standard error output verbosity of the function.
The data is in the form of bathymetry, amplitude, and sidescan values combined
with the acrosstrack and alongtrack distances relative to the navigation
of the bathymetry and sidescan 
measurements (amplitudes are coincident with the bathymetry values).
The return values are:
 	\fIkind\fP:		kind of data record read
 					1	survey data
 					2	comment
 					>=3	other data that cannot 
 						be passed by \fBmb_get\fP
 	\fIpings\fP:		number of pings averaged to give current data
 	\fItime_i\fP:		time of current ping
 			\fItime_i\fP[0]: year
 			\fItime_i\fP[1]: month
 			\fItime_i\fP[2]: day
 			\fItime_i\fP[3]: hour
 			\fItime_i\fP[4]: minute
 			\fItime_i\fP[5]: second
 			\fItime_i\fP[6]: microsecond
 	\fItime_d\fP:		time of current ping in seconds since 1/1/70 00:00:00
 	\fInavlon\fP:		longitude
 	\fInavlat\fP:		latitude
 	\fIspeed\fP:		ship speed in km/s
 	\fIheading\fP:		ship heading in degrees
 	\fIdistance\fP:	distance along shiptrack since last ping in km
 	\fInbath\fP:		number of bathymetry values
 	\fInamp\fP:		number of amplitude values
 	\fInss\fP:		number of sidescan values
	\fIbeamflag\fP:		array of bathymetry flags
 	\fIbath\fP:		array of bathymetry values in meters
 	\fIbath\fP:		array of amplitude values in unknown units
 	\fIbathacrosstrack\fP:	array of of acrosstrack distances in meters
 			corresponding to bathymetry
 	\fIbathalongtrack\fP:	array of of alongtrack distances in meters
 			corresponding to bathymetry
 	\fIss\fP:		array of sidescan values in unknown units
 	\fIssacrosstrack\fP:	array of of acrosstrack distances in meters
 			corresponding to sidescan
 	\fIssacrosstrack\fP:	array of of alongtrack distances in meters
 			corresponding to sidescan
 	\fIcomment\fP:		comment string
 	\fIerror\fP:		error value
.br
A status value indicating success or failure is returned; the error value
argument \fIerror\fP passes more detailed information about 
read failures. 

.SH LEVEL 2 FUNCTIONS

int \fBmb_get_all\fP(
 		int \fIverbose\fP,
 		char \fI*mbio_ptr\fP,
 		char \fI**store_ptr\fP,
 		int \fI*kind\fP,
		int \fItime_i[7]\fP,
 		double \fI*time_d\fP,
		double \fI*navlon\fP,
 		double \fI*navlat\fP,
		double \fI*speed\fP,
 		double \fI*heading\fP,
		double \fI*distance\fP,
 		double \fI*altitude\fP,
 		double \fI*sonardepth\fP,
		int \fI*nbath\fP,
 		int \fI*namp\fP,
 		int \fI*nss\fP,
		char \fI*beamflag\fP,
 		double \fI*bath\fP,
 		double \fI*amp\fP,
		double \fI*bathacrosstrack\fP,
 		double \fI*bathalongtrack\fP,
		double \fI*ss\fP,
 		double \fI*ssacrosstrack\fP,
 		double \fI*ssalongtrack\fP,
		char \fI*comment\fP,
 		int \fI*error\fP);

The function \fBmb_get_all\fP reads and returns sonar
data according to the \fBMBIO\fP descriptor pointed to by \fImbio_ptr\fP.
The \fIverbose\fP
value controls the standard error output verbosity of the function.
The data is returned one record at a time; no averaging is performed.
A pointer to a data structure containing all of the data read is returned
as \fIstore_ptr\fP; the form of the data structure is determined by the
sonar system associated with the format of the data being read.
Data is also returned in the forms of bathymetry, amplitude, and sidescan
survey data
and comment strings when the data record \fIkind\fP is survey data or
comment data, respectively.
The return values are:
 	\fIstore_ptr\fP:	pointer to complete data structure
 	\fIkind\fP:		kind of data record read
 					1	survey data
 					2	comment
 					3	calibrate data
 					4	mean water velocity
 					5	water velocity profile
 					6	standby
 					7	navigation source
 					8	unknown
 	\fIpings\fP:		number of pings averaged to give current data
 	\fItime_i\fP:		time of current ping
 			\fItime_i\fP[0]: year
 			\fItime_i\fP[1]: month
 			\fItime_i\fP[2]: day
 			\fItime_i\fP[3]: hour
 			\fItime_i\fP[4]: minute
 			\fItime_i\fP[5]: second
 			\fItime_i\fP[6]: microsecond
 	\fItime_d\fP:		time of current ping in seconds since 1/1/70 00:00:00
 	\fInavlon\fP:		longitude
 	\fInavlat\fP:		latitude
 	\fIspeed\fP:		ship speed in km/s
 	\fIheading\fP:		ship heading in degrees
 	\fIdistance\fP:		distance along shiptrack since last ping in km
 	\fInbath\fP:		number of bathymetry values
 	\fInamp\fP:		number of amplitude values
 	\fInss\fP:		number of sidescan values
	\fIbeamflag\fP:		array of bathymetry flags
 	\fIbath\fP:		array of bathymetry values in meters
 	\fIamp\fP:		array of amplitude values in unknown units
 	\fIbathacrosstrack\fP:	array of of acrosstrack distances in meters
 			corresponding to bathymetry
 	\fIbathalongtrack\fP:	array of of alongtrack distances in meters
 			corresponding to bathymetry
 	\fIss\fP:		array of sidescan values in unknown units
 	\fIssacrosstrack\fP:	array of of acrosstrack distances in meters
 			corresponding to sidescan
 	\fIssacrosstrack\fP:	array of of alongtrack distances in meters
 			corresponding to sidescan
 	\fIcomment\fP:	comment string
 	\fIerror\fP:		error value
.br
A status value indicating success or failure is returned; the error value
argument \fIerror\fP passes more detailed information about 
read failures. 

int \fBmb_put_all\fP(
 		int \fIverbose\fP,
 		char \fI*mbio_ptr\fP,
 		char \fI*store_ptr\fP,
		int \fIusevalues\fP,
 		int \fIkind\fP,
		int \fItime_i[7]\fP,
 		double \fItime_d\fP,
		double \fInavlon\fP,
 		double \fInavlat\fP,
		double \fIspeed\fP,
 		double \fIheading\fP,
		int \fInbath\fP,
 		int \fInamp\fP,
 		int \fInss\fP,
		char \fI*beamflag\fP,
 		double \fI*bath\fP,
 		double \fI*amp\fP,
		double \fI*bathacrosstrack\fP,
 		double \fI*bathalongtrack\fP,
		double \fI*ss\fP,
 		double \fI*ssacrosstrack\fP,
 		double \fI*ssalongtrack\fP,
		char \fI*comment\fP,
 		int \fI*error\fP);

The function \fBmb_put_all\fP writes sonar data to the file listed
in the \fBMBIO\fP descriptor pointed to by \fIMBIO_ptr\fP.
The \fIverbose\fP
value controls the standard error output verbosity of the function.
A pointer to a data structure containing all of the data read is passed
as \fIstore_ptr\fP; the form of the data structure is determined by the
sonar system associated with the format of the data being written.
Data is also passed in the forms of bathymetry and backscatter survey data
and comment strings when the data record \fIkind\fP is survey data or
comment data, respectively.  If the \fIusevalues\fP flag is set to 1,
then the passed values will be inserted in the data structure 
pointed to by \fIstore_ptr\fP before
the data is written.  If the \fIusevalues\fP flag is set to 0, the data
structure pointed to by \fIstore_ptr\fP will be written without modification.
The values to be output are:
 	\fIstore_ptr\fP:	pointer to complete data structure
 	\fIusevalues\fP:	flag controlling use of data passed by value
 					0	do not insert into data 
 						structure before writing 
 						the data
 					1	insert into data structure
 						before writing the data
 	\fIkind\fP:		kind of data record to be written
 					1	survey data
 					2	comment
 					3	calibrate data
 					4	mean water velocity
 					5	water velocity profile
 					6	standby
 					7	navigation source
 					8	unknown
 	\fItime_i\fP:		time of current ping (used if \fItime_i\fP[0] != 0)
 			\fItime_i\fP[0]: year
 			\fItime_i\fP[1]: month
 			\fItime_i\fP[2]: day
 			\fItime_i\fP[3]: hour
 			\fItime_i\fP[4]: minute
 			\fItime_i\fP[5]: second
 			\fItime_i\fP[6]: microsecond
 	\fItime_d\fP:		time of current ping in seconds since 
 				1/1/70 00:00:00 (used if \fItime_i\fP[0] = 0)
 	\fInavlon\fP:		longitude
 	\fInavlat\fP:		latitude
 	\fIspeed\fP:		ship speed in km/s
 	\fIheading\fP:		ship heading in degrees
 	\fIdistance\fP:		distance along shiptrack since last ping in km
 	\fInbath\fP:		number of bathymetry values
 	\fInamp\fP:		number of amplitude values
 	\fInss\fP:		number of sidescan values
	\fIbeamflag\fP:		array of bathymetry flags
 	\fIbath\fP:		array of bathymetry values in meters
 	\fIamp\fP:		array of amplitude values in unknown units
 	\fIbathacrosstrack\fP:	array of of acrosstrack distances in meters
 			corresponding to bathymetry
 	\fIbathalongtrack\fP:	array of of alongtrack distances in meters
 			corresponding to bathymetry
 	\fIss\fP:		array of sidescan values in unknown units
 	\fIssacrosstrack\fP:	array of of acrosstrack distances in meters
 			corresponding to sidescan
 	\fIssacrosstrack\fP:	array of of alongtrack distances in meters
 			corresponding to sidescan
 	\fIcomment\fP:	comment string
 	\fIerror\fP:		error value
.br
A status value indicating success or failure is returned; the error value
argument \fIerror\fP passes more detailed information about 
write failures. 

int \fBmb_put_comment\fP(
 		int \fIverbose\fP,
 		char \fI*mbio_ptr\fP,
 		char \fI*comment\fP,
		int \fI*error\fP);

The function \fBmb_put_comment\fP writes a comment to the file listed
in the \fBMBIO\fP descriptor pointed to by \fIMBIO_ptr\fP.
The \fIverbose\fP
value controls the standard error output verbosity of the function.
The data is in the form of a null terminated string. The maximum
length of comments varies with different data formats. In
general individual comments should be less than 80 characters
long to insure compatibility with all formats.
The values to be output are:
 	\fIcomment\fP:		comment string
 	\fIerror\fP:		error value
.br
A status value indicating success or failure is returned; the error value
argument \fIerror\fP passes more detailed information about 
write failures. 

int \fBmb_read_ping\fP(
 		int \fIverbose\fP,
 		char \fI*mbio_ptr\fP,
 		char \fI*store_ptr\fP,
		int \fI*kind\fP,
 		int \fI*error\fP);

int \fBmb_write_ping\fP(
 		int \fIverbose\fP,
 		char \fI*mbio_ptr\fP,
 		char \fI*store_ptr\fP,
		int \fI*error\fP);

int \fBmb_extract\fP(
 		int \fIverbose\fP,
 		char \fI*mbio_ptr\fP,
 		char \fI*store_ptr\fP,
		int \fI*kind\fP,
 		int \fItime_i[7]\fP,
 		double \fI*time_d\fP,
		double \fI*navlon\fP,
 		double \fI*navlat\fP,
		double \fI*speed\fP,
 		double \fI*heading\fP,
		int \fI*nbath\fP,
 		int \fI*namp\fP,
 		int \fI*nss\fP,
		char \fI*beamflag\fP,
 		double \fI*bath\fP,
 		double \fI*amp\fP,
		double \fI*bathacrosstrack\fP,
 		double \fI*bathalongtrack\fP,
		double \fI*ss\fP,
 		double \fI*ssacrosstrack\fP,
 		double \fI*ssalongtrack\fP,
		char \fI*comment\fP,
 		int \fI*error\fP);
int \fBmb_insert\fP(
 		int \fIverbose\fP,
 		char \fI*mbio_ptr\fP,
 		char \fI*store_ptr\fP,
		int \fIkind\fP,
 		int \fItime_i[7]\fP,
 		double \fItime_d\fP,
		double \fInavlon\fP,
 		double \fInavlat\fP,
		double \fIspeed\fP,
 		double \fIheading\fP,
		int \fInbath\fP,
 		int \fInamp\fP,
 		int \fInss\fP,
		char \fI*beamflag\fP,
 		double \fI*bath\fP,
 		double \fI*amp\fP,
		double \fI*bathacrosstrack\fP,
 		double \fI*bathalongtrack\fP,
		double \fI*ss\fP,
 		double \fI*ssacrosstrack\fP,
 		double \fI*ssalongtrack\fP,
		char \fI*comment\fP,
 		int \fI*error\fP);
int \fBmb_extract_nav\fP(
 		int \fIverbose\fP,
 		char \fI*mbio_ptr\fP,
 		char \fI*store_ptr\fP,
 		int \fI*kind\fP,
		int \fItime_i[7]\fP,
 		double \fI*time_d\fP,
		double \fI*navlon\fP,
 		double \fI*navlat\fP,
		double \fI*speed\fP,
 		double \fI*heading\fP,
 		double \fI*draft\fP,
		double \fI*roll\fP,
 		double \fI*pitch\fP,
 		double \fI*heave\fP,
		int \fI*error\fP);
int \fBmb_insert_nav\fP(
 		int \fIverbose\fP,
 		char \fI*mbio_ptr\fP,
 		char \fI*store_ptr\fP,
		int \fItime_i[7]\fP,
 		double \fItime_d\fP,
		double \fInavlon\fP,
 		double \fInavlat\fP,
		double \fIspeed\fP,
 		double \fIheading\fP,
 		double \fIdraft\fP,
		double \fIroll\fP,
 		double \fIpitch\fP,
 		double \fIheave\fP,
		int \fI*error\fP);
int \fBmb_extract_altitude\fP(
 		int \fIverbose\fP,
 		char \fI*mbio_ptr\fP,
 		char \fI*store_ptr\fP,
		int \fI*kind\fP,
		double \fI*transducer_depth\fP,
 		double \fI*altitude\fP,
		int \fI*error\fP);

The function \fBmb_extract_altitude\fP extracts the sonar transducer 
depth (\fBtransducer_depth\fP) below the sea surface
and the the sonar transducer \fBaltitude\fP above the seafloor
from a stored survey data ping. These values are useful for
sidescan processing applications. Both transducer depths and
altitudes are reported in meters.
The return values are:
 	\fIkind\fP:		kind of data record read (error
 				if not survey data):
 					1	survey data
 	\fItransducer_depth\fP:	depth of sonar in meters
	\fIaltitude\fP:		altitude of sonar above seafloor
 			in meters.
 	\fIerror\fP:		error value
.br
A status value indicating success or failure is returned; the error value
argument \fIerror\fP passes more detailed information about 
data extraction failures. 
int \fBmb_insert_altitude\fP(
 		int \fIverbose\fP,
 		char \fI*mbio_ptr\fP,
 		char \fI*store_ptr\fP,
		double \fItransducer_depth\fP,
 		double \fIaltitude\fP,
		int \fI*error\fP);
int \fBmb_extract_svp\fP(
 		int \fIverbose\fP,
 		char \fI*mbio_ptr\fP,
 		,
 		char \fI*store_ptr\fP,
		int \fI*kind\fP,
		int \fI*nsvp\fP,
		double \fI*depth\fP,
 		double \fI*velocity\fP,
		int \fI*error\fP);
int \fBmb_insert_svp\fP(
 		int \fIverbose\fP,
 		char \fI*mbio_ptr\fP,
 		char \fI*store_ptr\fP,
		int \fInsvp\fP,
		double \fI*depth\fP,
 		double \fI*velocity\fP,
		int \fI*error\fP);
int \fBmb_ttimes\fP(
 		int \fIverbose\fP,
 		char \fI*mbio_ptr\fP,
 		char \fI*store_ptr\fP,
		int \fI*kind\fP,
 		int \fI*nbeams\fP,
		double \fI*ttimes\fP,
 		double	*angles\fP,
		double \fI*angles_forward\fP,
 		double \fI*angles_null\fP,
		double \fI*heave\fP,
 		double \fI*alongtrack_offset\fP,
		double \fI*draft\fP,
 		double \fI*ssv\fP,
 		int \fI*error\fP);

The function \fBmb_ttimes\fP extracts travel times and beam
angles from a sonar-specific data structure pointed to by 
\fIstore_ptr\fP. These values are used for calculating 
swath bathymetry. The \fIverbose\fP value controls 
the standard error output verbosity of the function.
The coordinates of the beam angles can be a bit confusing.
The angles are returned in "takeoff angle coordinates"
appropriate for raytracing. The array angles contains the
angle from vertical and the array angles_forward
contains the angle from acrosstrack. This 
coordinate system is distinct from the roll-pitch coordinates
appropriate for correcting roll and pitch values. A description 
of these relevent coordinate systems is given below. 
The \fBangles_null\fP array
contains the effective sonar array orientation for each beam. The
\fBangles_null\fP array may be used to correct 
beam angles using Snell's law if the \fBssv\fP is changed. 
The \fBangles_null\fP values reflect the sonar configuration.
For example, some multibeam sonars have a flat transducer array, and
so the \fBangles_null\fP array consists of \fBnbeams\fP zero values.
Other multibeams have circular arrays so that the \fBangles_null\fP
values equal the \fBangles\fP values. The \fBalongtrack_offset\fP array
accomodates sonars which report multiple pings in a single
survey record; each ping occurs at a different position along the
shiptrack, producing alongtrack offsets relative to the navigation
for some beam values. The sum of the \fBdraft\fP value 
and the \fBheave\fP array values gives the depth
of the sonar for each beam. For hull mounted installations the
\fBdraft\fP value is generally static but the \fBheave\fP values
vary with time. For towed sonars the \fBdraft\fP varies with time
and the \fBheave\fP values are typically zero. The \fBssv\fP value
gives the water sound velocity at the sonar array.
The return values are:
 	\fIkind\fP:		kind of data record read (error
 				if not survey data):
 					1	survey data
 	\fInbeams\fP:		number of beams
	\fIttimes\fP:		array of two-way travel times in seconds
 	\fIangles\fP:		array of angles from vertical in degrees
 	\fIangles_forward\fP:	array of angles from acrosstrack in degrees
 	\fIangles_null\fP:	array of sonar array orientation in degrees
 	\fIheave\fP:		array of heave values for each beam in meters
 	\fIalongtrack_offset\fP:array of alongtrack distance offsets for 
 			each beam in meters
 	\fIssacrosstrack\fP:	draft of sonar in meters
 	\fIssacrosstrack\fP:	water sound velocity at sonar in m/seconds
 	\fIerror\fP:		error value
.br
A status value indicating success or failure is returned; the error value
argument \fIerror\fP passes more detailed information about 
data extraction failures. 
int \fBmb_extract_rawss\fP(
 		int \fIverbose\fP,
 		char \fI*mbio_ptr\fP,
 		char \fI*store_ptr\fP,
		int \fI*kind\fP,
		int \fI*nrawss\fP,
		double \fI*rawss\fP,
		double \fI*rawssacrosstrack\fP,
		double \fI*rawssalongtrack\fP,
		int \fI*error\fP);
int \fBmb_insert_rawss\fP(
 		int \fIverbose\fP,
 		char \fI*mbio_ptr\fP,
 		char \fI*store_ptr\fP,
		int \fInrawss\fP,
		double \fI*rawss\fP,
		double \fI*rawssacrosstrack\fP,
		double \fI*rawssalongtrack\fP,
		int \fI*error\fP);
int \fBmb_copyrecord\fP(
 		int \fIverbose\fP,
 		char \fI*mbio_ptr\fP,
		char \fI*store_ptr\fP,
 		char \fI*copy_ptr\fP,
 		int \fI*error\fP);
.SH LEVEL 3 FUNCTIONS

int \fBmb_buffer_init\fP(
 		int \fIverbose\fP,
 		char \fI**buff_ptr\fP,
 		int \fI*error\fP);

The function \fBmb_buffer_init\fP initializes the data structures
required for buffered i/0. A pointer to the buffer data structure
is returned as *\fIbuff_ptr\fP.
The \fIverbose\fP
value controls the standard error output verbosity of the function.
.br
A status value indicating success or failure is returned; the error value
argument \fIerror\fP passes more detailed information about 
buffer initialization failures. 

int \fBmb_buffer_close\fP(
 		int \fIverbose\fP,
 		char \fI**buff_ptr\fP,
 		char \fI*mbio_ptr\fP,
		int \fI*error\fP);

The function \fBmb_buffer_close\fP releases all memory allocated
for buffered i/0, including the structure pointed to by *\fIbuff_ptr\fP.
The \fIverbose\fP
value controls the standard error output verbosity of the function.
.br
A status value indicating success or failure is returned; the error value
argument \fIerror\fP passes more detailed information about 
buffer deallocation failures. 

int \fBmb_buffer_load\fP(
 		int \fIverbose\fP,
 		char \fI*buff_ptr\fP,char \fI*mbio_ptr\fP,
		int \fInwant\fP,
 		int \fI*nload\fP,
 		int \fI*nbuff\fP,
 		int \fI*error\fP);

The function \fBmb_buffer_load\fP loads data into the buffer pointed
to by \fIbuff_ptr\fP from the input file initialized in the 
\fBMBIO\fP descriptor pointed to by \fImbio_ptr\fP.
The \fIverbose\fP
value controls the standard error output verbosity of the function.
  
The input control parameters have the following significance:
 	\fInwant\fP:		The number of data records desired
 				in the buffer.

The returned values are:
 	\fInload\fP:		The number of data records loaded into the buffer.
 	\fInbuff\fP:		The total number of data records in the buffer after loading.
 
The buffer may already contain data records when the \fBmb_buffer_load\fP
call is made; if the number of previously loaded records is less than
\fInwant\fP, the function will attempt to read and load records
until a total of \fInwant\fP records are loaded. The \fInload\fP
value is the number of data records loaded during the current
function call, and the \fInbuff\fP value is the number of data
records in the buffer at the completion of the \fBmb_buffer_load\fP
call.
A status value indicating success or failure is returned; the error value
argument \fIerror\fP passes more detailed information about 
buffer deallocation failures. 

int \fBmb_buffer_dump\fP(
 		int \fIverbose\fP,
 		char \fI*buff_ptr\fP,
 		char \fI*mbio_ptr\fP,
		int \fInhold\fP,
 		int \fI*ndump\fP,
 		int \fI*nbuff\fP,
 		int \fI*error\fP);

The function \fBmb_buffer_dump\fP dumps data from the buffer pointed
to by *\fIbuff_ptr\fP into the output file initialized in the 
\fBMBIO\fP descriptor pointed to by \fImbio_ptr\fP.
The \fIverbose\fP
value controls the standard error output verbosity of the function.
  
The input control parameters have the following significance:
 	\fInwant\fP:		The number of data records desired to be held
 				in the buffer.

The returned values are:
 	\fInload\fP:		The number of data records dumped from the buffer.
 	\fInbuff\fP:		The total number of data records in the buffer after dumping.
 
If the number of loaded records is more than
\fInhold\fP, the function will attempt to write out records
from the beginning of the buffer until \fInhold\fP records 
are left in the buffer. The \fIndump\fP
value is the number of data records dumped during the current
function call, and the \fInbuff\fP value is the number of data
records in the buffer at the completion of the \fBmb_buffer_dump\fP
call.
A status value indicating success or failure is returned; the error value
argument \fIerror\fP passes more detailed information about 
buffer deallocation failures. 

int \fBmb_buffer_clear\fP(
 		int \fIverbose\fP,
 		char \fI*buff_ptr\fP,
 		char \fI*mbio_ptr\fP,
		int \fInhold\fP,
 		int \fI*ndump\fP,
 		int \fI*nbuff\fP,
 		int \fI*error\fP);

The function \fBmb_buffer_clear\fP removes data from the buffer pointed
to by *\fIbuff_ptr\fP without writing those data records to an
output file. An \fBMBIO\fP descriptor pointed to by \fImbio_ptr\fP
is still required, and generally represents the \fBMBIO\fP descriptor
used to read and load the data originally.
The \fIverbose\fP
value controls the standard error output verbosity of the function.
  
The input control parameters have the following significance:
 	\fInwant\fP:		The number of data records desired to be held
 				in the buffer.

The returned values are:
 	\fInload\fP:		The number of data records cleared from the buffer.
 	\fInbuff\fP:		The total number of data records in the buffer after dumping.
 
If the number of loaded records is more than
\fInhold\fP, the function will attempt to clear out records
from the beginning of the buffer until \fInhold\fP records 
are left in the buffer. The \fIndump\fP
value is the number of data records cleared during the current
function call, and the \fInbuff\fP value is the number of data
records in the buffer at the completion of the \fBmb_buffer_dump\fP
call.
A status value indicating success or failure is returned; the error value
argument \fIerror\fP passes more detailed information about 
buffer deallocation failures. 

int \fBmb_buffer_info\fP(
 		int \fIverbose\fP,
 		char \fI*buff_ptr\fP,
 		char \fI*mbio_ptr\fP,
		int \fIid\fP,
 		int \fI*system\fP,
 		int \fI*kind\fP,
 		int \fI*error\fP);

int \fBmb_buffer_get_next_data\fP(
 		int \fIverbose\fP,
 		char \fI*buff_ptr\fP,
 		char \fI*mbio_ptr\fP,
		int \fIstart\fP,
 		int \fI*id\fP,
		int \fItime_i[7]\fP,
 		double \fI*time_d\fP,
		double \fI*navlon\fP,
 		double \fI*navlat\fP,
		double \fI*speed\fP,
 		double \fI*heading\fP,
		int \fI*nbath\fP,
 		int \fI*namp\fP,
 		int \fI*nss\fP,
		char \fI*beamflag\fP,
 		double \fI*bath\fP,
 		double \fI*amp\fP,
		double \fI*bathacrosstrack\fP,
 		double \fI*bathalongtrack\fP,
		double \fI*ss\fP,
 		double \fI*ssacrosstrack\fP,
 		double \fI*ssalongtrack\fP,
		int \fI*error\fP);

The function \fBmb_buffer_get_next_data\fP searches for the
next survey data record in the buffer, beginning at buffer
index \fIstart\fP. Since buffer indexes begin at 0, the first
call to \fBmb_buffer_get_next_data\fP should have \fIstart\fP = 0.
If a survey data record is found at or beyond \fIstart\fP,
\fBmb_buffer_get_next_data\fP returns the buffer index of that 
record in \fIid\fP. Data is also returned in the forms of bathymetry, 
amplitude, and sidescan survey data. No comments or other
non-survey data records are returned.
The \fIverbose\fP
value controls the standard error output verbosity of the function.
  
The input control parameters have the following significance:
 	\fIstart\fP:		The buffer index at which to start searching for a survey data record.

The returned values are:
 	\fIid\fP:		The buffer index of the first survey data record at or after \fIstart\fP.
 	\fItime_i\fP:		time of current ping
 			\fItime_i\fP[0]: year
 			\fItime_i\fP[1]: month
 			\fItime_i\fP[2]: day
 			\fItime_i\fP[3]: hour
 			\fItime_i\fP[4]: minute
 			\fItime_i\fP[5]: second
 			\fItime_i\fP[6]: microsecond
 	\fItime_d\fP:		time of current ping in seconds since 1/1/70 00:00:00
 	\fInavlon\fP:		longitude
 	\fInavlat\fP:		latitude
 	\fIspeed\fP:		ship speed in km/s
 	\fIheading\fP:		ship heading in degrees
 	\fInbath\fP:		number of bathymetry values
 	\fInamp\fP:		number of amplitude values
 	\fInss\fP:		number of sidescan values
	\fIbeamflag\fP:		array of bathymetry flags
 	\fIbath\fP:		array of bathymetry values in meters
 	\fIamp\fP:		array of amplitude values in unknown units
 	\fIbathacrosstrack\fP:	array of of acrosstrack distances in meters
 			corresponding to bathymetry
 	\fIbathalongtrack\fP:	array of of alongtrack distances in meters
 			corresponding to bathymetry
 	\fIss\fP:		array of sidescan values in unknown units
 	\fIssacrosstrack\fP:	array of of acrosstrack distances in meters
 			corresponding to sidescan
 	\fIssacrosstrack\fP:	array of of alongtrack distances in meters
 			corresponding to sidescan
 
A status value indicating success or failure is returned; the error value
argument \fIerror\fP passes more detailed information about 
failures. The most common error occurs when no more survey data
records remain to be found in the buffer; in this case,
\fIerror\fP = -14.

int \fBmb_buffer_get_next_nav\fP(
 		int \fIverbose\fP,
 		char \fI*buff_ptr\fP,
 		char \fI*mbio_ptr\fP,
		int \fIstart\fP,
 		int \fI*id\fP,
		int \fItime_i[7]\fP,
 		double \fI*time_d\fP,
		double \fI*navlon\fP,
 		double \fI*navlat\fP,
		double \fI*speed\fP,
 		double \fI*heading\fP,
 		double \fI*draft\fP,
		double \fI*roll\fP,
 		double \fI*pitch\fP,
 		double \fI*heave\fP,
		int \fI*error\fP);

int \fBmb_buffer_extract\fP(
 		int \fIverbose\fP,
 		char \fI*buff_ptr\fP,
 		char \fI*mbio_ptr\fP,
		int \fIid\fP,
 		int \fI*kind\fP,
		int \fItime_i[7]\fP,
 		double \fI*time_d\fP,
		double \fI*navlon\fP,
 		double \fI*navlat\fP,
		double \fI*speed\fP,
 		double \fI*heading\fP,
		int \fI*nbath\fP,
 		int \fI*namp\fP,
 		int \fI*nss\fP,
		char \fI*beamflag\fP,
 		double \fI*bath\fP,
 		double \fI*amp\fP,
		double \fI*bathacrosstrack\fP,
 		double \fI*bathalongtrack\fP,
		double \fI*ss\fP,
 		double \fI*ssacrosstrack\fP,
 		double \fI*ssalongtrack\fP,
		char \fI*comment\fP,
		int \fI*error\fP);

The function \fBmb_buffer_extract\fP extracts and returns
a subset of the data in a buffer record.
The \fIverbose\fP
value controls the standard error output verbosity of the function.
The buffer record is specified with the 
buffer index \fIid\fP. The data is either
in the form of bathymetry, 
amplitude, and sidescan survey data or a comment string.
  
The input control parameters have the following significance:
 	\fIid\fP:		The buffer index of the data record to extract.

The returned values are:
 	\fIkind\fP:		kind of data record extracted
 					1	survey data
 					2	comment
 					>=3	other data that cannot 
 						be passed by \fBmb_buffer_extract\fP
 	\fItime_i\fP:		time of current ping
 			\fItime_i\fP[0]: year
 			\fItime_i\fP[1]: month
 			\fItime_i\fP[2]: day
 			\fItime_i\fP[3]: hour
 			\fItime_i\fP[4]: minute
 			\fItime_i\fP[5]: second
 			\fItime_i\fP[6]: microsecond
 	\fItime_d\fP:		time of current ping in seconds since 1/1/70 00:00:00
 	\fInavlon\fP:		longitude
 	\fInavlat\fP:		latitude
 	\fIspeed\fP:		ship speed in km/s
 	\fIheading\fP:		ship heading in degrees
 	\fInbath\fP:		number of bathymetry values
 	\fInamp\fP:		number of amplitude values
 	\fInss\fP:		number of sidescan values
	\fIbeamflag\fP:		array of bathymetry flags
 	\fIbath\fP:		array of bathymetry values in meters
 	\fIamp\fP:		array of amplitude values in unknown units
 	\fIbathacrosstrack\fP:	array of of acrosstrack distances in meters
 			corresponding to bathymetry
 	\fIbathalongtrack\fP:	array of of alongtrack distances in meters
 			corresponding to bathymetry
 	\fIss\fP:		array of sidescan values in unknown units
 	\fIssacrosstrack\fP:	array of of acrosstrack distances in meters
 			corresponding to sidescan
 	\fIssacrosstrack\fP:	array of of alongtrack distances in meters
 			corresponding to sidescan
 	\fIcomment\fP:		comment string
 
A status value indicating success or failure is returned; the error value
argument \fIerror\fP passes more detailed information about 
extract failures.

int \fBmb_buffer_insert\fP(
 		int \fIverbose\fP,
 		char \fI*buff_ptr\fP,
 		char \fI*mbio_ptr\fP,
		int \fIid\fP,
 		int \fItime_i[7]\fP,
 		double \fItime_d\fP,
		double \fInavlon\fP,
 		double \fInavlat\fP,
		double \fIspeed\fP,
 		double \fIheading\fP,
		int \fInbath\fP,
 		int \fInamp\fP,
 		int \fInss\fP,
		char \fI*beamflag\fP,
 		double \fI*bath\fP,
 		double \fI*amp\fP,
		double \fI*bathacrosstrack\fP,
 		double \fI*bathalongtrack\fP,
		double \fI*ss\fP,
 		double \fI*ssacrosstrack\fP,
 		double \fI*ssalongtrack\fP,
		char \fI*comment\fP,
		int \fI*error\fP);

The function \fBmb_buffer_insert\fP inserts data into a
buffer record, replacing a subset of the original values.
The \fIverbose\fP
value controls the standard error output verbosity of the function.
The buffer record is specified with the 
buffer index \fIid\fP. The data is either
in the form of bathymetry, 
amplitude, and sidescan survey data or a comment string.
  
The input control parameters have the following significance:
 	\fIid\fP:		The buffer index of the data record to insert.

The returned values are:
 	\fIkind\fP:		kind of data record inserted
 					1	survey data
 					2	comment
 					>=3	other data that cannot 
 						be passed by \fBmb_buffer_extract\fP
 	\fItime_i\fP:		time of current ping
 			\fItime_i\fP[0]: year
 			\fItime_i\fP[1]: month
 			\fItime_i\fP[2]: day
 			\fItime_i\fP[3]: hour
 			\fItime_i\fP[4]: minute
 			\fItime_i\fP[5]: second
 			\fItime_i\fP[6]: microsecond
 	\fItime_d\fP:		time of current ping in seconds since 1/1/70 00:00:00
 	\fInavlon\fP:		longitude
 	\fInavlat\fP:		latitude
 	\fIspeed\fP:		ship speed in km/s
 	\fIheading\fP:		ship heading in degrees
 	\fInbath\fP:		number of bathymetry values
 	\fInamp\fP:		number of amplitude values
 	\fInss\fP:		number of sidescan values
	\fIbeamflag\fP:		array of bathymetry flags
 	\fIbath\fP:		array of bathymetry values in meters
 	\fIamp\fP:		array of amplitude values in unknown units
 	\fIbathacrosstrack\fP:	array of of acrosstrack distances in meters
 			corresponding to bathymetry
 	\fIbathalongtrack\fP:	array of of alongtrack distances in meters
 			corresponding to bathymetry
 	\fIss\fP:		array of sidescan values in unknown units
 	\fIssacrosstrack\fP:	array of of acrosstrack distances in meters
 			corresponding to sidescan
 	\fIssacrosstrack\fP:	array of of alongtrack distances in meters
 			corresponding to sidescan
 	\fIcomment\fP:		comment string
 
A status value indicating success or failure is returned; the error value
argument \fIerror\fP passes more detailed information about 
insert failures.

int \fBmb_buffer_get_next_nav\fP(
 		int \fIverbose\fP,
 		char \fI*buff_ptr\fP,
 		char \fI*mbio_ptr\fP,
		int \fIstart\fP,
 		int \fI*id\fP,
		int \fItime_i[7]\fP,
 		double \fI*time_d\fP,
		double \fI*navlon\fP,
 		double \fI*navlat\fP,
		double \fI*speed\fP,
 		double \fI*heading\fP,
 		double \fI*draft\fP,
		double \fI*roll\fP,
 		double \fI*pitch\fP,
 		double \fI*heave\fP,
		int \fI*error\fP);

The function \fBmb_buffer_get_next_nav\fP searches for the
next survey data record in the buffer, beginning at buffer
index \fIstart\fP. Since buffer indexes begin at 0, the first
call to \fBmb_buffer_get_next_nav\fP should have \fIstart\fP = 0.
If a survey data record is found at or beyond \fIstart\fP,
\fBmb_buffer_get_next_nav\fP returns the buffer index of that 
record in \fIid\fP. Navigation and vertical reference sensor
data is also returned. No comments or other
non-survey data records are returned.
The \fIverbose\fP
value controls the standard error output verbosity of the function.
  
The input control parameters have the following significance:
 	\fIstart\fP:		The buffer index at which to start searching for a survey data record.

The returned values are:
 	\fIid\fP:		The buffer index of the first survey data record at or after \fIstart\fP.
 	\fItime_i\fP:		time of current ping
 			\fItime_i\fP[0]: year
 			\fItime_i\fP[1]: month
 			\fItime_i\fP[2]: day
 			\fItime_i\fP[3]: hour
 			\fItime_i\fP[4]: minute
 			\fItime_i\fP[5]: second
 			\fItime_i\fP[6]: microsecond
 	\fItime_d\fP:		time of current ping in seconds since 1/1/70 00:00:00
 	\fInavlon\fP:		longitude
 	\fInavlat\fP:		latitude
 	\fIspeed\fP:		ship speed in km/s
 	\fIheading\fP:		ship heading in degrees
 	\fIroll\fP:		ship roll in degrees
 	\fIpitch\fP:		ship pitch in degrees
 	\fIheave\fP:		ship heave in meters
 
A status value indicating success or failure is returned; the error value
argument \fIerror\fP passes more detailed information about 
failures. The most common error occurs when no more survey data
records remain to be found in the buffer; in this case,
\fIerror\fP = -14.

int \fBmb_buffer_extract_nav\fP(
 		int \fIverbose\fP,
 		char \fI*buff_ptr\fP,
 		char \fI*mbio_ptr\fP,
		int \fIid\fP,
 		int \fI*kind\fP,
		int \fItime_i[7]\fP,
 		double \fI*time_d\fP,
		double \fI*navlon\fP,
 		double \fI*navlat\fP,
		double \fI*speed\fP,
 		double \fI*heading\fP,
 		double \fI*draft\fP,
		double \fI*roll\fP,
 		double \fI*pitch\fP,
 		double \fI*heave\fP,
		int \fI*error\fP);

The function \fBmb_buffer_extract_nav\fP extracts and returns
a subset of the data in a buffer record.
The \fIverbose\fP
value controls the standard error output verbosity of the function.
The buffer record is specified with the 
buffer index \fIid\fP. The data returned consists of navigation
and vertical reference sensor data.
  
The input control parameters have the following significance:
 	\fIid\fP:		The buffer index of the data record to extract.

The returned values are:
 	\fIkind\fP:		kind of data record extracted
 					1	survey data
 					2	comment
 					>=3	other data that cannot 
 						be passed by \fBmb_buffer_extract\fP
 	\fItime_i\fP:		time of current ping
 			\fItime_i\fP[0]: year
 			\fItime_i\fP[1]: month
 			\fItime_i\fP[2]: day
 			\fItime_i\fP[3]: hour
 			\fItime_i\fP[4]: minute
 			\fItime_i\fP[5]: second
 			\fItime_i\fP[6]: microsecond
 	\fItime_d\fP:		time of current ping in seconds since 1/1/70 00:00:00
 	\fInavlon\fP:		longitude
 	\fInavlat\fP:		latitude
 	\fIspeed\fP:		ship speed in km/s
 	\fIheading\fP:		ship heading in degrees
 	\fIroll\fP:		ship roll in degrees
 	\fIpitch\fP:		ship pitch in degrees
 	\fIheave\fP:		ship heave in meters
 
A status value indicating success or failure is returned; the error value
argument \fIerror\fP passes more detailed information about 
extract failures.

int \fBmb_buffer_insert_nav\fP(
 		int \fIverbose\fP,
 		char \fI*buff_ptr\fP,
 		char \fI*mbio_ptr\fP,
		int \fIid\fP,
 		int \fItime_i[7]\fP,
 		double \fItime_d\fP,
		double \fInavlon\fP,
 		double \fInavlat\fP,
		double \fIspeed\fP,
 		double \fIheading\fP,
 		double \fIdraft\fP,
		double \fIroll\fP,
 		double \fIpitch\fP,
 		double \fIheave\fP,
		int \fI*error\fP);

The function \fBmb_buffer_insert_nav\fP inserts navigation
and vertical reference sensor data into a
buffer record, replacing a subset of the original values.
The \fIverbose\fP
value controls the standard error output verbosity of the function.
The buffer record is specified with the 
buffer index \fIid\fP. 
  
The input control parameters have the following significance:
 	\fIid\fP:		The buffer index of the data record to insert.

The returned values are:
 	\fIkind\fP:		kind of data record inserted
 					1	survey data
 					2	comment
 					>=3	other data that cannot 
 						be passed by \fBmb_buffer_extract\fP
 	\fItime_i\fP:		time of current ping
 			\fItime_i\fP[0]: year
 			\fItime_i\fP[1]: month
 			\fItime_i\fP[2]: day
 			\fItime_i\fP[3]: hour
 			\fItime_i\fP[4]: minute
 			\fItime_i\fP[5]: second
 			\fItime_i\fP[6]: microsecond
 	\fItime_d\fP:		time of current ping in seconds since 1/1/70 00:00:00
 	\fInavlon\fP:		longitude
 	\fInavlat\fP:		latitude
 	\fIspeed\fP:		ship speed in km/s
 	\fIheading\fP:		ship heading in degrees
 	\fIroll\fP:		ship roll in degrees
 	\fIpitch\fP:		ship pitch in degrees
 	\fIheave\fP:		ship heave in meters
 
A status value indicating success or failure is returned; the error value
argument \fIerror\fP passes more detailed information about 
insert failures.

int \fBmb_buffer_get_ptr\fP(
 		int \fIverbose\fP,
 		char \fI*buff_ptr\fP,
 		char \fI*mbio_ptr\fP,
		int \fIid\fP,
 		char \fI**store_ptr\fP,
		int \fI*error\fP);
.SH MISCELLANEOUS FUNCTIONS

int \fBmb_defaults\fP(
 		int \fIverbose\fP,
 		int \fI*format\fP,
 		int \fI*pings\fP,
		int \fI*lonflip\fP,
 		double \fIbounds[4]\fP,
		int \fI*btime_i\fP,
 		int \fI*etime_i\fP,
		double \fI*speedmin\fP,
 		double \fI*timegap\fP);

The function \fBmb_defaults\fP provides default values of control
parameters used by some of the \fBMBIO\fP functions. The \fIverbose\fP
value controls the standard error output verbosity of the function.  
The other parameters
are set by the function; the meaning of these parameters is discussed
in the listings of the functions  
\fBmb_read_init\fP and \fBmb_write_init\fP.
If an .mbio_defaults file exists in the user's home directory, 
the defaults are read from this file. A status value is returned to 
indicate success or failure.

int \fBmb_env\fP(
 		int \fIverbose\fP,
 		char \fI*psdisplay\fP,
 		char \fI*imgdisplay\fP,
 		char \fI*mbproject\fP);

int \fBmb_format\fP(
 		int \fIverbose\fP,
 		int \fI*format\fP,
 		int \fI*error\fP);

Given the format identifier \fIformat\fP, \fBmb_format\fP checks if the
format is valid.  If the format id corresponds to a value used in
previous (<4.00) versions of \fBMB-System\fP, then the format value
will be aliased to the current corresponding value.  If the format id
is valid, then the location of that format in the various format
tables used by \fBMBIO\fP is returned in \fIformat_num\fP.  The return
status value signals success if \fIformat\fP is valid and failure otherwise.

int \fBmb_format_register\fP(
 		int \fIverbose\fP,
 		int \fI*format\fP,
 		char \fI*mbio_ptr\fP,
		int \fI*error\fP);

int \fBmb_format_info\fP(
 		int \fIverbose\fP,
 		int \fI*format\fP,
 		int \fI*system\fP,
		int \fI*beams_bath_max\fP,
 		int \fI*beams_amp_max\fP,
 		int \fI*pixels_ss_max\fP,
		char \fI*format_name\fP,
 		char \fI*system_name\fP,
 		char \fI*format_description\fP,
		int \fI*numfile\fP,
 		int \fI*filetype\fP,
 		int \fI*variable_beams\fP,
		int \fI*traveltime\fP,
 		int \fI*beam_flagging\fP,
		int \fI*nav_source\fP,
 		int \fI*heading_source\fP,
 		int \fI*vru_source\fP,
		double \fI*beamwidth_xtrack\fP,
 		double \fI*beamwidth_ltrack\fP,
		int \fI*error\fP);

int \fBmb_format_system\fP(
 		int \fIverbose\fP,
 		int \fI*format\fP,
 		int \fI*system\fP,
 		int \fI*error\fP);

int \fBmb_format_description\fP(
 		int \fIverbose\fP,
 		int \fI*format\fP,
		char \fI*description\fP,
 		int \fI*error\fP);

Given the format identifier \fIformat\fP, \fBmb_format_description\fP returns a
short description of the format in the string \fI*description\fP. The \fIverbose\fP
value controls the standard error output verbosity of the function.  The return
status value signals success if \fIformat\fP is valid and failure otherwise.

int \fBmb_format_dimensions\fP(
 		int \fIverbose\fP,
 		int \fI*format\fP,
		int \fI*beams_bath_max\fP,
 		int \fI*beams_amp_max\fP,
 		int \fI*pixels_ss_max\fP,
		int \fI*error\fP);

int \fBmb_format_flags\fP(
 		int \fIverbose\fP,
 		int \fI*format\fP,
		int \fI*variable_beams\fP,
 		int \fI*traveltime\fP,
 		int \fI*beam_flagging\fP,
		int \fI*error\fP);

int \fBmb_format_source\fP(
 		int \fIverbose\fP,
 		int \fI*format\fP,
		int \fI*nav_source\fP,
 		int \fI*heading_source\fP,
 		int \fI*vru_source\fP,
		int \fI*error\fP);

int \fBmb_format_beamwidth\fP(
 		int \fIverbose\fP,
 		int \fI*format\fP,
		double \fI*beamwidth_xtrack\fP,
 		double \fI*beamwidth_ltrack\fP,
		int \fI*error\fP);

int \fBmb_alloc\fP(
 		int \fIverbose\fP,
 		char \fI*mbio_ptr\fP,
		char \fI**store_ptr\fP,
 		int \fI*error\fP);

int \fBmb_deall\fP(
 		int \fIverbose\fP,
 		char \fI*mbio_ptr\fP,
		char \fI**store_ptr\fP,
 		int \fI*error\fP);

int \fBmb_datalist_open\fP(
 		int \fIverbose\fP,
		char \fI**datalist\fP,
		char \fI*path\fP,
		int \fIlook_processed\fP,
		int \fI*error\fP);

int \fBmb_datalist_read\fP(
 		int \fIverbose\fP,
		char \fI \fI*datalist\fP,
		char \fI*path\fP,
 		int \fI*format\fP,
 		double \fI*weight\fP,
		int \fI*error\fP);

int \fBmb_datalist_close\fP(
 		int \fIverbose\fP,
		char \fI**datalist\fP,
 		int \fI*error\fP);

int \fBmb_error\fP(
 		int \fIerror\fP,
 		int \fIerror\fP,
 		char \fI**message\fP);

Given the error value \fIerror\fP, \fBmb_format_inf\fP returns a
short error message in the string **\fImessage\fP. The \fIverbose\fP
value controls the standard error output verbosity of the function.  The return
status value signals success if \fIformat\fP is valid and failure otherwise.

int \fBmb_navint_add\fP(
 		int \fIverbose\fP,
 		char \fI*mbio_ptr\fP,
		double \fItime_d\fP,
 		double \fIlon\fP,
 		double \fIlat\fP,
 		int \fI*error\fP);

int \fBmb_navint_interp\fP(
 		int \fIverbose\fP,
 		char \fI*mbio_ptr\fP,
		double \fItime_d\fP,
 		double \fIheading\fP,
 		double \fIrawspeed\fP,
		double \fI*lon\fP,
 		double \fI*lat\fP,
 		double \fI*speed\fP,
		int \fI*error\fP);

int \fBmb_get_double\fP(
 		double \fI*value\fP,
 		char \fI*str\fP,
 		int \fInchar\fP);

int \fBmb_get_int\fP(
 		int \fI*value\fP,
 		char \fI*str\fP,
 		int \fInchar\fP);

int \fBmb_get_binary_short\fP(
 		int \fIswapped\fP,
 		void \fI*buffer\fP,
 		short *value\fP);

int \fBmb_get_binary_int\fP(
 		int \fIswapped\fP,
 		void \fI*buffer\fP,
 		int \fI*value\fP);

int \fBmb_get_binary_float\fP(
 		int \fIswapped\fP,
 		void \fI*buffer\fP,
 		float *value\fP);

int \fBmb_get_binary_double\fP(
 		int \fIswapped\fP,
 		void \fI*buffer\fP,
 		double \fI*value\fP);

int \fBmb_get_bounds\fP(
 		char \fI*text\fP,
 		double \fI*bounds\fP);

int \fBmb_put_binary_short\fP(
 		int \fIswapped\fP,
 		short value\fP,
 		void \fI*buffer\fP);

int \fBmb_put_binary_int\fP(
 		int \fIswapped\fP,
 		int \fIvalue\fP,
 		void \fI*buffer\fP);

int \fBmb_put_binary_float\fP(
 		int \fIswapped\fP,
 		float value\fP,
 		void \fI*buffer\fP);

int \fBmb_put_binary_double\fP(
 		int \fIswapped\fP,
 		double \fIvalue\fP,
 		void \fI*buffer\fP);
double \fBmb_ddmmss_to_degree\fP(
 		char \fI*text\fP);

int \fBmb_takeoff_to_rollpitch\fP(
 		int\fP,
 		double\fP,
 		double\fP,
 		double \fI*\fP,
 		double \fI*\fP,
 		int \fI*\fP);

int \fBmb_rollpitch_to_takeoff\fP(
 		int\fP,
 		double\fP,
 		double\fP,
 		double \fI*\fP,
 		double \fI*\fP,
 		int \fI*\fP);

int \fBmb_double_compare\fP(double \fI*a\fP,
 		double \fI*b\fP);

int \fBmb_int_compare\fP(
 		int \fI*a\fP,
 		int \fI*b\fP);

.SH COORDINATE SYSTEMS USED IN MB-SYSTEM
I. Introduction
.br
The coordinate systems described below are used
within \fBMB-System\fP for calculations involving
the location in space of depth, amplitude, or
sidescan data. In all cases the origin of the
coordinate system is at the center of the sonar 
transducers.

II. Cartesian Coordinates
.br
The cartesian coordinate system used in \fBMB-System\fP
is a bit odd because it is left-handed, as opposed
to the right-handed x-y-z space conventionally
used in most circumstances. With respect to the
sonar (or the ship on which the sonar is mounted),
the x-axis is athwartships with positive to starboard
(to the right if facing forward), the y-axis is
fore-aft with positive forward, and the z-axis is
positive down.

III. Spherical Coordinates
.br
There are two non-traditional spherical coordinate 
systems used in \fBMB-System\fP. The first, referred to here 
as takeoff angle coordinates, is useful for raytracing.
The second, referred to here as roll-pitch 
coordinates, is useful for taking account of 
corrections to roll and pitch angles.

III.1. Takeoff Angle Coordinates

The three parameters are r, theta, and phi, where
r is the distance from the origin, theta is the
angle from vertical down (that is, from the 
positive z-axis), and phi is the angle from 
acrosstrack (the positive x-axis) in the x-y plane.
Note that theta is always positive; the direction
in the x-y plane is given by phi.
Raytracing is simple in these coordinates because
the ray takeoff angle is just theta. However,
applying roll or pitch corrections is complicated because
roll and pitch have components in both theta and phi.

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

III.2. Roll-Pitch Coordinates

The three parameters are r, alpha, and beta, where
r is the distance from the origin, alpha is the angle 
forward (effectively pitch angle), and beta is the
angle from horizontal in the x-z plane (effectively
roll angle). Applying a roll or pitch correction is 
simple in these coordinates because pitch is just alpha 
and roll is just beta. However, raytracing is complicated 
because deflection from vertical has components in both 
alpha and beta.

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

IV. SeaBeam Coordinates

The per-beam parameters in the SB2100 data format include
angle-from-vertical and angle-forward. Angle-from-vertical
is the same as theta except that it is signed based on
the acrosstrack direction (positive to starboard, negative 
to port). The angle-forward values are also defined 
slightly differently from phi, in that angle-forward is 
signed differently on the port and starboard sides. The 
SeaBeam 2100 External Interface Specifications document 
includes both discussion and figures illustrating the 
angle-forward value. To summarize:

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

V. Usage of Coordinate Systems in \fBMB-System\fP

Some sonar data formats provide angle values along with
travel times. The angles are converted to takoff-angle 
coordinates regardless of the  storage form of the 
particular data format. Currently, most data formats
do not contain an alongtrack component to the position
values; in these cases the conversion is trivial since
phi = beta = 0 and theta = alpha. The angle and travel time 
values can be accessed using the \fBMBIO\fP function \fBmb_ttimes\fP.
All angle values passed by \fBMB-System\fP functions are in
degrees rather than radians.

The programs \fBmbbath\fP and \fBmbvelocitytool\fP use angles in
take-off angle coordinates to do the raytracing. If roll
and/or pitch corrections are to be made, the angles are
converted to roll-pitch coordinates, corrected, and then
converted back prior to raytracing.


.SH BEAM FLAGS USED IN MB-SYSTEM
\fBMB-System\fP uses arrays of 1-byte "beamflag" values to indicate
beam data quality. Each beamflag value is actually an eight bit mask
allowing fairly complicated information to be stored regarding
each bathymetry value. In particular, beams may be flagged as
bad, they may be selected as being of special interest, and
one or more reasons for flagging or selection may be indicated. 
This scheme is very similar to the convention used in the HMPS 
hydrographic data processing package and the SAIC Hydrobat 
package. The beam selection mechanism is not currently used by
any \fBMB-System\fP programs.

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

.SH SEE ALSO
\fBmbsystem\fP(l), \fBmbformat\fP(l)

.SH BUGS
What could go wrong in a mere 171,000 lines of code?
.br
Let us know...
