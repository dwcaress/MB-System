.TH mbio l "16 September 1997" "MB-System 4.5" "MB-System 4.5"
.SH NAME
\fBmbio\fP - Format independent input/output library for swath mapping sonar data.

.SH VERSION
Version 4.5

.SH DESCRIPTION
\fBMBIO\fP (\fBM\fPulti\fBB\fPeam \fBI\fPnput/\fBO\fPutput) is a library
of functions used for reading and writing swath mapping sonar 
data files.  \fBMBIO\fP supports a large number of data
formats associated with different institutions and different sonar systems.
The purpose of \fBMBIO\fP is to allow users to write processing and display
programs which are independent of particular data formats and to provide
a standard approach to swath mapping sonar data i/o.

.SH AUTHORSHIP
David W. Caress (caress@lamont.ldgo.columbia.edu)
.br
Dale N. Chayes (dale@lamont.ldgo.columbia.edu)
.br
Lamont-Doherty Earth Observatory
.br
Palisades, NY  10964

.SH OVERVIEW
\fBMBIO\fP handles three types of swath mapping data:  beam bathymetry,
beam amplitude, and sidescan.  Both amplitude and sidescan represent
measures of backscatter strength. Beam amplitudes are backscatter
values associated with the same preformed beams used to
obtain bathymetry; \fBMBIO\fP assumes that a bathymetry value
exists for each amplitude value and uses the bathymetry beam
location for the amplitude.  Sidescan is generally constructed with
a higher spatial resolution than bathymetry, and carries its
own location parameters.  In the context of \fBMB-System\fP documentation,
the discrete values of bathymetry and
amplitude are referred to as "beams", and the discrete values of
sidescan are referred to as "pixels".

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

\fBMBIO\fP has three levels of i/o functionality:
 	1	Simple reading and writing with the positions of
 		individual beams and pixels given in longitude 
 		and latitude (\fBmb_read\fP and \fBmb_write\fP) 
 		or acrosstrack distances (\fBmb_get\fP and 
 		\fBmb_put\fP). Only a limited set of navigation 
 		information is passed. These functions can be 
 		used without any special include files or any 
 		knowledge of the actual data structures used
 		by the data formats or \fBMBIO\fP. A function 
		for writing comments (\fBmb_put_comment\fP)
		is also included. The primary 
 		functions are:
 			\fBmb_read\fP
 			\fBmb_write\fP
 			\fBmb_get\fP
 			\fBmb_put\fP
 			\fBmb_put_comment\fP
 	2	Complete reading and writing of data structures 
 		containing all of the available information.   
 		Special include files are required to make 
 		sense of the data structures passed by
 		these functions. The primary functions are:
 			\fBmb_get_all\fP
 			\fBmb_put_all\fP
 	3	Buffered reading and writing of data structures 
 		containing all of the available information. 
 		Special include files are required to make 
 		sense of the data structures passed by 
 		these functions. The primary functions are:
 			\fBmb_buffer_init\fP
 			\fBmb_buffer_close\fP
 			\fBmb_buffer_load\fP
 			\fBmb_buffer_dump\fP
 			\fBmb_buffer_info\fP
 			\fBmb_buffer_get_next_data\fP
 			\fBmb_buffer_extract\fP
 			\fBmb_buffer_insert\fP
 			\fBmb_buffer_get_next_nav\fP
 			\fBmb_buffer_extract_nav\fP
 			\fBmb_buffer_insert_nav\fP

The level 1 \fBMBIO\fP functions allow users to read 
and write sonar data independent of format, with the limitation that 
only a limited set of navigation information is passed.  Thus, some of the
information contained in certain data formats (e.g. the "heave" value
in Hydrosweep DS data) is not passed by \fBmb_read\fP or \fBmb_get\fP.  
In general, the level 1 functions are useful for applications 
such as graphics which require only the navigation and the depth 
and/or backscatter values. The level 2 functions (\fBmb_get_all\fP and
\fBmb_put_all\fP) read and write the 
complete data structures, translate the data to internal data structures
associated with each of the supported sonar systems, and 
pass pointers to these internal data structures.  The level 3 functions
provide buffered reading and writing which is useful for applications
which need access to multiple pings at a time.  In addition to reading
(\fBmb_buffer_load\fP) and writing (\fBmb_buffer_dump\fP), functions
exist for extracting information from the buffer (\fBmb_buffer_extract\fP)
and inserting information into the buffer (\fBmb_buffer_insert\fP).
The level 2 and 3 functions require 
include files for the data structures used but allow access to the complete
data structures; these functions are useful for applications which require
special parameters peculiar to some particular data format (e.g. processing
Hydrosweep DS amplitudes by applying system gain settings to the measured
values). A number of other \fBMBIO\fP functions dealing with default values for
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
 	Sea Beam "classic" 16 beam multibeam sonar 
 	Hydrosweep DS 59 beam multibeam sonar 
 	Hydrosweep MD 40 beam mid-depth multibeam sonar 
 	Sea Beam 2000 multibeam sonar 
 	Sea Beam 2100 series multibeam sonars
 	Simrad EM12 and EM1000 multibeam sonars
 	Hawaii MR-1 shallow tow interferometric sonar
 	ELAC Bottomchart shallow water multibeam sonar
 	Reson Seabat 9001/9002 shallow water multibeam sonars
 	WHOI DSL AMS-120 deep tow interferometric sonar
	
.SH SUPPORTED FORMATS
With the introduction of \fBMB-System\fP version 4.00, the data format
id's used by \fBMBIO\fP changed considerably.  The old format id's,
which ranged from 1 to 9, are still supported in the sense that they
are automatically converted to the appropriate new format id's by \fBMBIO\fP.
Thus, old shellscripts using the old format id's will continue to work
without modification.  However, we recommend that the new format id's
be used in the future.  Perpetual support for the old format id's is
not guarenteed.

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
                          binary, centered, VAX byte order, 
                          URI.

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
    Attributes:           Hydrosweep DS, bathymetry, 59 
                          beams, binary, NRL.

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
                          bathymetry, binary, VAX 
                          byte order, URI.

    MBIO Data Format ID:  32
    Format name:          MBF_SB2000SB
    Informal Description: SIO Swath-bathy SeaBeam 2000
    Attributes:           SeaBeam 2000, bathymetry, 
                          121 beams, binary,  SIO.

    MBIO Data Format ID:  33
    Format name:          MBF_SB2000SS
    Informal Description: SIO Swath-bathy SeaBeam 2000
    Attributes:           SeaBeam 2000, sidescan,
                          1000 pixels for 4-bit sidescan,
                          2000 pixels for 12+-bin sidescan, 
                          binary,  SIO.

    MBIO Data Format ID:  41
    Format name:          MBF_SB2100RW
    Informal Description: SeaBeam 2100 series vender format
    Attributes:           SeaBeam 2100, bathymetry, 
                          amplitude and sidescan, 151 
                          beams and 2000 pixels, ascii 
                          with binary sidescan, SeaBeam 
                          Instruments.

    MBIO Data Format ID:  42
    Format name:          MBF_SB2100B1
    Informal Description: SeaBeam 2100 series vender format
    Attributes:           SeaBeam 2100, bathymetry, 
                          amplitude and sidescan, 151 
                          beams bathymetry, 2000 pixels 
                          sidescan, binary, SeaBeam 
                          Instruments and L-DEO.

    MBIO Data Format ID:  43
    Format name:          MBF_SB2100B2
    Informal Description: SeaBeam 2100 series vender format
    Attributes:           SeaBeam 2100, bathymetry and 
                          amplitude, 151 beams bathymetry,
                          binary, SeaBeam Instruments 
                          and L-DEO.

    MBIO Data Format ID:  51
    Format name:          MBF_EM1000RW
    Informal Description: Simrad EM1000 vendor format
    Attributes:           Simrad EM1000, bathymetry,
                          amplitude, and sidescan, 60 beams,
                          variable pixels, ascii + binary,
                          Simrad.

    MBIO Data Format ID:  52
    Format name:          MBF_EM12SRAW
    Informal Description: Simrad EM12S vendor format
    Attributes:           Simrad EM12S, bathymetry, 
                          amplitude, and sidescan,
                          81 beams, variable pixels, 
                          ascii + binary, Simrad.

    MBIO Data Format ID:  54
    Format name:          MBF_EM12DARW
    Informal Description: Simrad EM12S RRS Darwin format
    Attributes:           Simrad EM12S, bathymetry and 
                          amplitude, 81 beams, binary,
                          Oxford University.

    MBIO Data Format ID:  55
    Format name:          MBF_EM121RAW
    Informal Description: Simrad EM121 vendor format
    Attributes:           Simrad EM121, bathymetry, 
                          amplitude, and sidescan,
                          121 beams, variable pixels, 
                          ascii + binary, Simrad.

    MBIO Data Format ID:  61
    Format name:          MBF_MR1PRHIG
    Informal Description: SOEST MR1 post processed format
    Attributes:           SOEST MR1, bathymetry and 
                          sidescan, variable beams and 
                          pixels, xdr binary, SOEST, UH.

    MBIO data format id: 62
    Format name:          MBF_MR1ALDEO
    Informal Description: L-DEO MR1 post processed format 
                          with travel times
    Attributes:           L-DEO MR1, bathymetry and 
                          sidescan, variable beams and 
                          pixels, xdr binary, L-DEO.

    MBIO data format id: 63
    Format name:          MBF_MR1BLDEO
    Informal Description: L-DEO small MR1 post processed 
                          format with travel times
    Attributes:           L-DEO MR1, bathymetry and 
                          sidescan, variable beams  
                          and pixels, xdr binary, L-DEO.

    MBIO Data Format ID:  71
    Format name:          MBF_MBLDEOIH
    Informal Description: L-DEO in-house generic multibeam
    Attributes:           Data from all sonar systems, 
                          bathymetry, amplitude and
                          sidescan, variable beams and 
                          pixels, binary, centered, 
                          L-DEO.

    MBIO Data Format ID:  81
    Format name:          MBF_CBAT9001
    Informal Description: Reson SeaBat 9001 multibeam
    Attributes:           60 beam bathymetry and amplitude,
                          binary, UNB.

    MBIO Data Format ID:  91
    Format name:          MBF_BCHRTUNB
    Informal Description: Elac BottomChart multibeam
    Attributes:           56 beam bathymetry and amplitude,
                          binary, UNB.

    MBIO Data Format ID:  101
    Format name:          MBF_HSMDARAW
    Informal Description: Atlas HSMD medium depth multibeam 
                          raw format
    Attributes:           40 beam bathymetry, 160 pixel 
                          sidescan, XDR (binary), 
                          STN Atlas Elektronik.

    MBIO Data Format ID:  102
    Format name:          MBF_HSMDLDIH
    Informal Description: Atlas HSMD medium depth multibeam 
                          process format
    Attributes:           40 beam bathymetry, 160 pixel 
                          sidescan, XDR (binary), L-DEO.

    MBIO Data Format ID:  111
    Format name:          MBF_DSL120PF
    Informal Description: WHOI DSL AMS-120 processed format
    Attributes:           2048 beam bathymetry, 2048 pixel 
                          sidescan, binary, parallel 
                          bathymetry and amplitude files, 
                          WHOI DSL.

    MBIO Data Format ID:  112
    Format name:          MBF_DSL120SF
    Informal Description: WHOI DSL AMS-120 processed format
    Attributes:           2048 beam bathymetry, 2048 pixel 
                          sidescan, binary, single files,
                          WHOI DSL.


The institutional acronyms used above have the following meanings:
 	L-DEO	Lamont-Doherty Earth Observatory
 	SIO	Scripps Institution of Oceanography
 	WHOI	Woods Hole Oceanographic Institution
 	URI	University of Rhode Island
 	NRL	Naval Research Laboratory
 	UNB	University of New Brunswick
 	UH	University of Hawaii
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
.RE
.IP
int \fBmb_read_init\fP(\fIverbose\fP, \fIfile\fP, 
\fIformat\fP, \fIpings\fP, \fIlonflip\fP, 
\fIbounds\fP, \fIbtime_i\fP, \fIetime_i\fP, \fIspeedmin\fP, 
\fItimegap\fP,
\fImbio_ptr\fP, \fIbtime_d\fP, \fIetime_d\fP, 
\fIbeams_bath\fP, \fIbeams_amp\fP, \fIpixels_ss\fP, \fIerror\fP)
.br
int \fIverbose\fP
.br
char *\fIfile\fP;
.br
int \fIformat\fP;
.br
int \fIpings\fP;
.br
int \fIlonflip\fP;
.br
double \fIbounds\fP[4];
.br
int \fIbtime_i[6]\fP;
.br
int \fIetime_i[6]\fP;
.br
double \fIspeedmin\fP;
.br
double \fItimegap\fP;
.br
char **\fImbio_ptr\fP;
.br
double *\fIbtime_d\fP;
.br
double *\fIetime_d\fP;
.br
int *\fIbeams_bath\fP;
.br
int *\fIbeams_amp\fP;
.br
int *\fIpixels_ss\fP;
.br
int *\fIerror\fP;
.RS 10
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

The \fIbtime_i\fP array sets the desired beginning time for the data.
Data with an earlier time stamp will be returned with an error by 
the reading function. The functions \fBmb_read\fP, 
\fBmb_get\fP and \fBmb_get_all\fP use the \fIbtime_i\fP array; the
function \fBmb_buffer_load\fP does no time checking.
 	\fIbtime\fP[0] : year
 	\fIbtime\fP[1] : month
 	\fIbtime\fP[2] : day
 	\fIbtime\fP[3] : hour
 	\fIbtime\fP[4] : minute
 	\fIbtime\fP[5] : second

The \fIetime_i\fP array sets the desired ending time for the data.
Data with a later time stamp will be returned with an error by 
the reading function. The functions \fBmb_read\fP, 
\fBmb_get\fP and \fBmb_get_all\fP use the \fIetime_i\fP array; the
function \fBmb_buffer_load\fP does no time checking.
 	\fIetime\fP[0] : year
 	\fIetime\fP[1] : month
 	\fIetime\fP[2] : day
 	\fIetime\fP[3] : hour
 	\fIetime\fP[4] : minute
 	\fIetime\fP[5] : second

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
.RE
.IP
int \fBmb_write_init\fP(\fIverbose\fP, \fIfile\fP, \fIformat\fP, 
\fImbio_ptr\fP, \fIbeams_bath\fP, \fIbeams_amp\fP, \fIpixels_ss\fP, \fIerror\fP)
.br
int \fIverbose\fP
.br
char *\fIfile\fP;
.br
int \fIformat\fP;
.br
char **\fImbio_ptr\fP;
.br
int *\fIbeams_bath\fP;
.br
int *\fIbeams_amp\fP;
.br
int *\fIpixels_ss\fP;
.br
int *\fIerror\fP;
.RS 10
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
.RE
.IP
int \fBmb_close\fP(\fIverbose\fP, \fImbio_ptr\fP, \fIerror\fP)
.br
int \fIverbose\fP
.br
char *\fImbio_ptr\fP;
.br
int *\fIerror\fP;
.RS 10
Closes the data file listed in the \fBMBIO\fP descriptor pointed to 
by \fImbio_ptr\fP and releases all specially allocated memory.  
The \fIverbose\fP
value controls the standard error output verbosity of the function.
A status value indicating success or failure is returned; an error value
argument passes more detailed information about failures. 
.SH LEVEL 1 FUNCTIONS
.RE
.IP
int \fBmb_read\fP(\fIverbose\fP, \fImbio_ptr\fP, \fIkind\fP, 
\fIpings\fP, \fItime_i\fP, \fItime_d\fP, 
\fInavlon\fP, \fInavlat\fP, \fIspeed\fP, \fIheading\fP, \fIdistance\fP, 
\fInbath\fP, \fInamp\fP, \fInss\fP, 
\fIbath\fP, \fIamp\fP, \fIbathlon\fP, \fIbathlat\fP, 
\fIss\fP, \fIsslon\fP, \fIsslat\fP, 
\fIcomment\fP, \fIerror\fP)
.br
int \fIverbose\fP
.br
char *\fImbio_ptr\fP;
.br
int *\fIkind\fP;
.br
int *\fIpings\fP;
.br
int \fItime_i\fP[7];
.br
double *\fItime_d\fP;
.br
double *\fInavlon\fP;
.br
double *\fInavlat\fP;
.br
double *\fIspeed\fP;
.br
double *\fIheading\fP;
.br
double *\fIdistance\fP;
.br
int *\fInbath\fP;
.br
int *\fInamp\fP;
.br
int *\fInss\fP;
.br
double *\fIbath\fP;
.br
double *\fIamp\fP;
.br
double *\fIbathlon\fP;
.br
double *\fIbathlat\fP;
.br
double *\fIss\fP;
.br
double *\fIsslon\fP;
.br
double *\fIsslat\fP;
.br
char \fIcomment\fP[256];
.br
int *\fIerror\fP;
.RS 10
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
.RE
.IP
int \fBmb_write\fP(\fIverbose\fP, \fImbio_ptr\fP, 
\fIkind\fP, \fItime_i\fP, \fItime_d\fP, 
\fInavlon\fP, \fInavlat\fP, \fIspeed\fP, \fIheading\fP, 
\fInbath\fP, \fInamp\fP, \fInss\fP, 
\fIbath\fP, \fIamp\fP, \fIbathlon\fP, \fIbathlat\fP, 
\fIss\fP, \fIsslon\fP, \fIsslat\fP, 
\fIcomment\fP, \fIerror\fP)
.br
int \fIverbose\fP
.br
char *\fImbio_ptr\fP;
.br
int \fIkind\fP
.br
int \fItime_i\fP[7];
.br
double \fItime_d\fP;
.br
double \fInavlon\fP;
.br
double \fInavlat\fP;
.br
double \fIspeed\fP;
.br
double \fIheading\fP;
.br
int \fInbath\fP;
.br
int \fInamp\fP;
.br
int \fInss\fP;
.br
double *\fIbath\fP;
.br
double *\fIamp\fP;
.br
double *\fIbathlon\fP;
.br
double *\fIbathlat\fP;
.br
double *\fIss\fP;
.br
double *\fIsslon\fP;
.br
double *\fIsslat\fP;
.br
char \fIcomment\fP[256];
.br
int \fIerror\fP;
.RS 10
The function \fBmb_write\fP writes sonar data to the file listed
in the \fBMBIO\fP descriptor pointed to by \fImbio_ptr\fP.
The \fIverbose\fP
value controls the standard error output verbosity of the function.
The data is in the form of bathymetry and backscatter values combined
with the longitude and latitude locations of the bathymetry and 
backscatter measurements.
The values to be output are:
 	\fIkind\fP:		kind of data record written
 					1	survey data
 					2	comment
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
 	\fInbath\fP:		number of bathymetry values
 	\fInamp\fP:		number of amplitude values
 	\fInss\fP:		number of sidescan values
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
 	\fIcomment\fP:		comment string
.br
A status value indicating success or failure is returned; the error value
argument \fIerror\fP passes more detailed information about 
write failures. 
.RE
.IP
int \fBmb_get\fP(\fIverbose\fP, \fImbio_ptr\fP, \fIkind\fP, \fIpings\fP, 
\fItime_i\fP, \fItime_d\fP, 
\fInavlon\fP, \fInavlat\fP, \fIspeed\fP, \fIheading\fP, \fIdistance\fP, 
\fInbath\fP, \fInamp\fP, \fInss\fP, 
\fIbath\fP, \fIamp\fP, \fIbathacrosstrack\fP, \fIbathalongtrack\fP, 
\fIss\fP, \fIssacrosstrack\fP, \fIssalongtrack\fP, 
\fIcomment\fP, \fIerror\fP)
.br
int \fIverbose\fP;
.br
char *\fImbio_ptr\fP;
.br
int *\fIkind\fP;
.br
int *\fIpings\fP;
.br
int \fItime_i\fP[7];
.br
double *\fItime_d\fP;
.br
double *\fInavlon\fP;
.br
double *\fInavlat\fP;
.br
double *\fIspeed\fP;
.br
double *\fIheading\fP;
.br
double *\fIdistance\fP;
.br
int *\fInbath\fP;
.br
int *\fInamp\fP;
.br
int *\fInss\fP;
.br
double *\fIbath\fP;
.br
double *\fIamp\fP;
.br
double *\fIbathacrosstrack\fP;
.br
double *\fIbathalongtrack\fP;
.br
double *\fIss\fP;
.br
double *\fIssacrosstrack\fP;
.br
double *\fIssalongtrack\fP;
.br
char \fIcomment\fP[256];
.br
int *\fIerror\fP;
.RS 10
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
.RE
.IP
int \fBmb_put\fP(\fIverbose\fP, \fImbio_ptr\fP, \fIkind\fP, 
\fItime_i\fP, \fItime_d\fP, 
\fInavlon\fP, \fInavlat\fP, \fIspeed\fP, \fIheading\fP,  
\fInbath\fP, \fInamp\fP, \fInss\fP, 
\fIbath\fP, \fIamp\fP, \fIbathacrosstrack\fP, \fIbathalongtrack\fP, 
\fIss\fP, \fIssacrosstrack\fP, \fIssalongtrack\fP, 
\fIcomment\fP, \fIerror\fP)
.br
int \fIverbose\fP;
.br
char *\fImbio_ptr\fP;
.br
int \fIkind\fP
.br
int \fItime_i\fP[7];
.br
double \fItime_d\fP;
.br
double \fInavlon\fP;
.br
double \fInavlat\fP;
.br
double \fIspeed\fP;
.br
double \fIheading\fP;
.br
int \fInbath\fP;
.br
int \fInamp\fP;
.br
int \fInss\fP;
.br
double *\fIbath\fP;
.br
double *\fIamp\fP;
.br
double *\fIbathacrosstrack\fP;
.br
double *\fIbathalongtrack\fP;
.br
double *\fIss\fP;
.br
double *\fIssacrosstrack\fP;
.br
double *\fIssalongtrack\fP;
.br
char \fIcomment\fP[256];
.br
int *\fIerror\fP;
.RS 10
The function \fBmb_put\fP writes sonar data to the file listed
in the \fBMBIO\fP descriptor pointed to by \fIMBIO_ptr\fP.
The \fIverbose\fP
value controls the standard error output verbosity of the function.
The data is in the form of bathymetry, amplitude, and sidescan values combined
with the acrosstrack and alongtrack distances relative to the navigation
of the bathymetry and sidescan 
measurements (amplitudes are coincident with the bathymetry values).
The values to be output are:
 	\fIkind\fP:		kind of data record to be written
 					1	survey data
 					2	comment
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
write failures. 
.RE
.IP
int \fBmb_put_comment\fP(\fIverbose\fP, \fImbio_ptr\fP, 
\fIcomment\fP, \fIerror\fP)
.br
int \fIverbose\fP;
.br
char *\fImbio_ptr\fP;
.br
char \fIcomment\fP[256];
.br
int *\fIerror\fP;
.RS 10
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
.SH LEVEL 2 FUNCTIONS
.RE
.IP
int \fBmb_get_all\fP(\fIverbose\fP, \fImbio_ptr\fP, \fIstore_ptr\fP, 
\fIkind\fP, \fItime_i\fP, \fItime_d\fP, 
\fInavlon\fP, \fInavlat\fP, \fIspeed\fP, \fIheading\fP, \fIdistance\fP, 
\fInbath\fP, \fInamp\fP, \fInss\fP, 
\fIbath\fP, \fIamp\fP, \fIbathacrosstrack\fP, \fIbathalongtrack\fP, 
\fIss\fP, \fIssacrosstrack\fP, \fIssalongtrack\fP, 
\fIcomment\fP, \fIerror\fP)
.br
int \fIverbose\fP;
.br
char *\fImbio_ptr\fP;
.br
char **\fIstore_ptr\fP;
.br
int *\fIkind\fP;
.br
int \fItime_i\fP[7];
.br
double *\fItime_d\fP;
.br
double *\fInavlon\fP;
.br
double *\fInavlat\fP;
.br
double *\fIspeed\fP;
.br
double *\fIheading\fP;
.br
double *\fIdistance\fP;
.br
int *\fInbath\fP;
.br
int *\fInamp\fP;
.br
int *\fInss\fP;
.br
double *\fIbath\fP;
.br
double *\fIamp\fP;
.br
double *\fIbathacrosstrack\fP;
.br
double *\fIbathalongtrack\fP;
.br
double *\fIss\fP;
.br
double *\fIssacrosstrack\fP;
.br
double *\fIssalongtrack\fP;
.br
char \fIcomment\fP[256];
.br
int *\fIerror\fP;
.RS 10
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
 	\fIcomment\fP:	comment string
 	\fIerror\fP:		error value
.br
A status value indicating success or failure is returned; the error value
argument \fIerror\fP passes more detailed information about 
read failures. 
.RE
.IP
int \fBmb_put_all\fP(\fIverbose\fP, \fImbio_ptr\fP, 
\fIstore_ptr\fP, \fIusevalues\fP, \fIkind\fP, 
\fItime_i\fP, \fItime_d\fP, 
\fInavlon\fP, \fInavlat\fP, \fIspeed\fP, \fIheading\fP,  
\fInbath\fP, \fInamp\fP, \fInss\fP, 
\fIbath\fP, \fIamp\fP, \fIbathacrosstrack\fP, \fIbathalongtrack\fP, 
\fIss\fP, \fIssacrosstrack\fP, \fIssalongtrack\fP, 
\fIcomment\fP, \fIerror\fP)
.br
int \fIverbose\fP;
.br
char *\fImbio_ptr\fP;
.br
char *\fIstore_ptr\fP;
.br
int \fIusevalues\fP;
.br
int \fIkind\fP
.br
int \fItime_i\fP[7];
.br
double \fItime_d\fP;
.br
double \fInavlon\fP;
.br
double \fInavlat\fP;
.br
double \fIspeed\fP;
.br
double \fIheading\fP;
.br
int \fInbath\fP;
.br
int \fInamp\fP;
.br
int \fInss\fP;
.br
double *\fIbath\fP;
.br
double *\fIamp\fP;
.br
double *\fIbathacrosstrack\fP;
.br
double *\fIbathalongtrack\fP;
.br
double *\fIss\fP;
.br
double *\fIssacrosstrack\fP;
.br
double *\fIssalongtrack\fP;
.br
char \fIcomment\fP[256];
.br
int *\fIerror\fP;
.RS 10
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
 	\fIcomment\fP:	comment string
 	\fIerror\fP:		error value
.br
A status value indicating success or failure is returned; the error value
argument \fIerror\fP passes more detailed information about 
write failures. 
.SH LEVEL 3 FUNCTIONS
.RE
.IP
int \fBmb_buffer_init\fP(\fIverbose\fP, \fIbuff_ptr\fP, \fIerror\fP)
.br
int \fIverbose\fP;
.br
char **\fIbuff_ptr\fP;
.br
int *\fIerror\fP;
.RS 10
The function \fBmb_buffer_init\fP initializes the data structures
required for buffered i/0. A pointer to the buffer data structure
is returned as *\fIbuff_ptr\fP.
The \fIverbose\fP
value controls the standard error output verbosity of the function.
.br
A status value indicating success or failure is returned; the error value
argument \fIerror\fP passes more detailed information about 
buffer initialization failures. 
.RE
.IP
int \fBmb_buffer_close\fP(\fIverbose\fP, \fIbuff_ptr\fP, \fIerror\fP)
.br
int \fIverbose\fP;
.br
char **\fIbuff_ptr\fP;
.br
int *\fIerror\fP;
.RS 10
The function \fBmb_buffer_close\fP releases all memory allocated
for buffered i/0, including the structure pointed to by *\fIbuff_ptr\fP.
The \fIverbose\fP
value controls the standard error output verbosity of the function.
.br
A status value indicating success or failure is returned; the error value
argument \fIerror\fP passes more detailed information about 
buffer deallocation failures. 
.RE
.IP
int \fBmb_buffer_load\fP(\fIverbose\fP, \fIbuff_ptr\fP, 
\fImbio_ptr\fP, \fInwant\fP, \fInload\fP, \fInbuff\fP, \fIerror\fP)
.br
int \fIverbose\fP;
.br
char *\fIbuff_ptr\fP;
.br
char *\fImbio_ptr\fP;
.br
int \fInwant\fP;
.br
int *\fInload\fP;
.br
int *\fInbuff\fP;
.br
int *\fIerror\fP;
.RS 10
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
.RE
.IP
int \fBmb_buffer_dump\fP(\fIverbose\fP, \fIbuff_ptr\fP, 
\fImbio_ptr\fP, \fInhold\fP, \fIndump\fP, \fInbuff\fP, \fIerror\fP)
.br
int \fIverbose\fP;
.br
char *\fIbuff_ptr\fP;
.br
char *\fImbio_ptr\fP;
.br
int \fInhold\fP;
.br
int *\fIndump\fP;
.br
int *\fInbuff\fP;
.br
int *\fIerror\fP;
.RS 10
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
.RE
.IP
int \fBmb_buffer_clear\fP(\fIverbose\fP, \fIbuff_ptr\fP, 
\fImbio_ptr\fP, \fInhold\fP, \fIndump\fP, \fInbuff\fP, \fIerror\fP)
.br
int \fIverbose\fP;
.br
char *\fIbuff_ptr\fP;
.br
char *\fImbio_ptr\fP;
.br
int \fInhold\fP;
.br
int *\fIndump\fP;
.br
int *\fInbuff\fP;
.br
int *\fIerror\fP;
.RS 10
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
.RE
.IP
int \fBmb_buffer_get_next_data\fP(\fIverbose\fP, \fIbuff_ptr\fP, 
\fImbio_ptr\fP, \fIstart\fP, \fIid\fP, 
\fItime_i\fP, \fItime_d\fP, 
\fInavlon\fP, \fInavlat\fP, \fIspeed\fP, \fIheading\fP, 
\fInbath\fP, \fInamp\fP, \fInss\fP, 
\fIbath\fP, \fIamp\fP, \fIbathacrosstrack\fP, \fIbathalongtrack\fP, 
\fIss\fP, \fIssacrosstrack\fP, \fIssalongtrack\fP, 
\fIerror\fP)
.br
int \fIverbose\fP;
.br
char *\fIbuff_ptr\fP;
.br
char *\fImbio_ptr\fP;
.br
int \fIstart\fP;
.br
int *\fIid\fP;
.br
int \fItime_i\fP[7];
.br
double *\fItime_d\fP;
.br
double *\fInavlon\fP;
.br
double *\fInavlat\fP;
.br
double *\fIspeed\fP;
.br
double *\fIheading\fP;
.br
int *\fInbath\fP;
.br
int *\fInamp\fP;
.br
int *\fInss\fP;
.br
double *\fIbath\fP;
.br
double *\fIamp\fP;
.br
double *\fIbathacrosstrack\fP;
.br
double *\fIbathalongtrack\fP;
.br
double *\fIss\fP;
.br
double *\fIssacrosstrack\fP;
.br
double *\fIssalongtrack\fP;
.br
int *\fIerror\fP;
.RS 10
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
 
A status value indicating success or failure is returned; the error value
argument \fIerror\fP passes more detailed information about 
failures. The most common error occurs when no more survey data
records remain to be found in the buffer; in this case,
\fIerror\fP = -14.
.RE
.IP
int \fBmb_buffer_extract\fP(\fIverbose\fP, \fIbuff_ptr\fP, 
\fImbio_ptr\fP, \fIid\fP, \fIkind\fP, 
\fItime_i\fP, \fItime_d\fP, 
\fInavlon\fP, \fInavlat\fP, \fIspeed\fP, \fIheading\fP, 
\fInbath\fP, \fInamp\fP, \fInss\fP, 
\fIbath\fP, \fIamp\fP, \fIbathacrosstrack\fP, \fIbathalongtrack\fP, 
\fIss\fP, \fIssacrosstrack\fP, \fIssalongtrack\fP, 
\fIcomment\fP, \fIerror\fP)
.br
int \fIverbose\fP;
.br
char *\fIbuff_ptr\fP;
.br
char *\fImbio_ptr\fP;
.br
int \fIid\fP;
.br
int *\fIkind\fP;
.br
int \fItime_i\fP[7];
.br
double *\fItime_d\fP;
.br
double *\fInavlon\fP;
.br
double *\fInavlat\fP;
.br
double *\fIspeed\fP;
.br
double *\fIheading\fP;
.br
int *\fInbath\fP;
.br
int *\fInamp\fP;
.br
int *\fInss\fP;
.br
double *\fIbath\fP;
.br
double *\fIamp\fP;
.br
double *\fIbathacrosstrack\fP;
.br
double *\fIbathalongtrack\fP;
.br
double *\fIss\fP;
.br
double *\fIssacrosstrack\fP;
.br
double *\fIssalongtrack\fP;
.br
char \fIcomment\fP[256];
.br
int *\fIerror\fP;
.RS 10
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
 
A status value indicating success or failure is returned; the error value
argument \fIerror\fP passes more detailed information about 
extract failures.
.RE
.IP
int \fBmb_buffer_insert\fP(\fIverbose\fP, \fIbuff_ptr\fP, 
\fImbio_ptr\fP, \fIid\fP, \fIkind\fP, 
\fItime_i\fP, \fItime_d\fP, 
\fInavlon\fP, \fInavlat\fP, \fIspeed\fP, \fIheading\fP, 
\fInbath\fP, \fInamp\fP, \fInss\fP, 
\fIbath\fP, \fIamp\fP, \fIbathacrosstrack\fP, \fIbathalongtrack\fP, 
\fIss\fP, \fIssacrosstrack\fP, \fIssalongtrack\fP, 
\fIcomment\fP, \fIerror\fP)
.br
int \fIverbose\fP;
.br
char *\fIbuff_ptr\fP;
.br
char *\fImbio_ptr\fP;
.br
int \fIid\fP;
.br
int *\fIkind\fP;
.br
int \fItime_i\fP[7];
.br
double \fItime_d\fP;
.br
double \fInavlon\fP;
.br
double \fInavlat\fP;
.br
double \fIspeed\fP;
.br
double \fIheading\fP;
.br
int *\fInbath\fP;
.br
int *\fInamp\fP;
.br
int *\fInss\fP;
.br
double *\fIbath\fP;
.br
double *\fIamp\fP;
.br
double *\fIbathacrosstrack\fP;
.br
double *\fIbathalongtrack\fP;
.br
double *\fIss\fP;
.br
double *\fIssacrosstrack\fP;
.br
double *\fIssalongtrack\fP;
.br
char \fIcomment\fP[256];
.br
int *\fIerror\fP;
.RS 10
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
 
A status value indicating success or failure is returned; the error value
argument \fIerror\fP passes more detailed information about 
insert failures.
.RE
.IP
int \fBmb_buffer_get_next_nav\fP(\fIverbose\fP, \fIbuff_ptr\fP, 
\fImbio_ptr\fP, \fIstart\fP, \fIid\fP, 
\fItime_i\fP, \fItime_d\fP, 
\fInavlon\fP, \fInavlat\fP, \fIspeed\fP, \fIheading\fP, 
\fIroll\fP, \fIpitch\fP, \fIheave\fP, 
\fIerror\fP)
.br
int \fIverbose\fP;
.br
char *\fIbuff_ptr\fP;
.br
char *\fImbio_ptr\fP;
.br
int \fIstart\fP;
.br
int *\fIid\fP;
.br
int \fItime_i\fP[7];
.br
double *\fItime_d\fP;
.br
double *\fInavlon\fP;
.br
double *\fInavlat\fP;
.br
double *\fIspeed\fP;
.br
double *\fIheading\fP;
.br
double *\fIroll\fP;
.br
double *\fIpitch\fP;
.br
double *\fIheave\fP;
.br
int *\fIerror\fP;
.RS 10
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
.RE
.IP
int \fBmb_buffer_extract_nav\fP(\fIverbose\fP, \fIbuff_ptr\fP, 
\fImbio_ptr\fP, \fIid\fP, \fIkind\fP, 
\fItime_i\fP, \fItime_d\fP, 
\fInavlon\fP, \fInavlat\fP, \fIspeed\fP, \fIheading\fP, 
\fIroll\fP, \fIpitch\fP, \fIheave\fP, 
\fIerror\fP)
.br
int \fIverbose\fP;
.br
char *\fIbuff_ptr\fP;
.br
char *\fImbio_ptr\fP;
.br
int \fIid\fP;
.br
int *\fIkind\fP;
.br
int \fItime_i\fP[7];
.br
double *\fItime_d\fP;
.br
double *\fInavlon\fP;
.br
double *\fInavlat\fP;
.br
double *\fIspeed\fP;
.br
double *\fIheading\fP;
.br
double *\fIroll\fP;
.br
double *\fIpitch\fP;
.br
double *\fIheave\fP;
.br
int *\fIerror\fP;
.RS 10
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
.RE
.IP
int \fBmb_buffer_insert_nav\fP(\fIverbose\fP, \fIbuff_ptr\fP, 
\fImbio_ptr\fP, \fIid\fP, \fIkind\fP, 
\fItime_i\fP, \fItime_d\fP, 
\fInavlon\fP, \fInavlat\fP, \fIspeed\fP, \fIheading\fP, 
\fIroll\fP, \fIpitch\fP, \fIheave\fP, 
\fIerror\fP)
.br
int \fIverbose\fP;
.br
char *\fIbuff_ptr\fP;
.br
char *\fImbio_ptr\fP;
.br
int \fIid\fP;
.br
int *\fIkind\fP;
.br
int \fItime_i\fP[7];
.br
double \fItime_d\fP;
.br
double \fInavlon\fP;
.br
double \fInavlat\fP;
.br
double \fIspeed\fP;
.br
double \fIheading\fP;
.br
double \fIroll\fP;
.br
double \fIpitch\fP;
.br
double \fIheave\fP;
.br
int *\fIerror\fP;
.RS 10
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
.SH MISCELLANEOUS FUNCTIONS
.RE
.IP
int \fBmb_defaults\fP(\fIverbose\fP, \fIformat\fP, \fIpings, \fIlonflip, \fIbounds\fP, 
\fIbtime_i\fP, \fIetime_i\fP, \fIspeedmin\fP, \fItimegap\fP)
.br
int \fIverbose\fP;
.br
int *\fIformat\fP;
.br
int *\fIpings\fP;
.br
int *\fIlonflip\fP;
.br
double \fIbounds\fP[4];
.br
int \fIbtime_i[6]\fP;
.br
int \fIetime_i[6]\fP;
.br
double *\fIspeedmin\fP;
.br
double *\fItimegap\fP;
.RS 10
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
.RE
.IP
int \fBmb_format\fP(\fIverbose\fP, \fIformat\fP, \fIformat_num\fP, \fIerror\fP)
.br
int \fIverbose\fP;
.br
int *\fIformat\fP;
.br
int *\fIformat_num\fP;
.br
int *\fIerror\fP;
.RS 10
Given the format identifier \fIformat\fP, \fBmb_format\fP checks if the
format is valid.  If the format id corresponds to a value used in
previous (<4.00) versions of \fBMB-System\fP, then the format value
will be aliased to the current corresponding value.  If the format id
is valid, then the location of that format in the various format
tables used by \fBMBIO\fP is returned in \fIformat_num\fP.  The return
status value signals success if \fIformat\fP is valid and failure otherwise.
.RE
.IP
int \fBmb_format_inf\fP(\fIverbose\fP, \fIformat\fP, \fImessage\fP)
.br
int \fIverbose\fP;
.br
int \fIformat\fP;
.br
char **\fImessage\fP;
.RS 10
Given the format identifier \fIformat\fP, \fBmb_format_inf\fP returns a
short description of the format in the string **\fImessage\fP. The \fIverbose\fP
value controls the standard error output verbosity of the function.  The return
status value signals success if \fIformat\fP is valid and failure otherwise.
.RE
.IP
int \fBmb_error\fP(\fIverbose\fP, \fIerror\fP, \fImessage\fP)
.br
int \fIverbose\fP;
.br
int \fIerror\fP;
.br
char **\fImessage\fP;
.RS 10
Given the error value \fIerror\fP, \fBmb_format_inf\fP returns a
short error message in the string **\fImessage\fP. The \fIverbose\fP
value controls the standard error output verbosity of the function.  The return
status value signals success if \fIformat\fP is valid and failure otherwise.

.SH SEE ALSO
\fBmbsystem\fP(l), \fBmbformat\fP(l)

.SH BUGS
What could go wrong in a mere 90,000 lines of code?
.br
Let us know...
