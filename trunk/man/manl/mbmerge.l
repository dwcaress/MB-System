.TH mbmerge l "20 August 1996" "MB-System 4.4" "MB-System 4.4"
.SH NAME
\fBmbmerge\fP - Merges multibeam data with new navigation.

.SH VERSION
Version 4.4

.SH SYNOPSIS
\fBmbmerge\fP [\fB-F\fIformat \fB-L\fIlonflip\fP 
\fB-I\fIinfile \fB-O\fIoutfile \fB-M\fInavformat\fP \fB-N\fInavfile\fP \fB-V -H\fP]

.SH DESCRIPTION
\fBmbmerge\fP is an utility for merging navigation with multibeam
echosounder data.  The input navigation can be in one of several
formats, including the L-DEO shipboard processing format. 
The default input and output streams are stdin and stdout.

.SH AUTHORSHIP
David W. Caress (caress@lamont.ldeo.columbia.edu)
.br
Dale N. Chayes (dale@lamont.ldeo.columbia.edu)
.br
Lamont-Doherty Earth Observatory
.br
Palisades, NY 10964

.SH OPTIONS
.TP
.B \fB-F\fIformat\fP
Sets the format for the input and output multibeam data using 
\fBMBIO\fP integer format identifiers. 
This program uses the \fBMBIO\fP library and will read or write any multibeam
format supported by \fBMBIO\fP. A list of the multibeam data formats
currently supported by \fBMBIO\fP and their identifier values
is given in the \fBMBIO\fP manual page.
Default: \fIformat\fP = 11.
.TP
.B \fB-H\fP
This "help" flag causes the program to print out a description
of its operation and then exit immediately.
.TP
.B \fB-I\fIinfile\fP
Data file from which the input data will be read. If
no input file is specified, the input will be read
from stdin. Default: \fIinfile\fP = stdin.
.TP
.B \fB-L\fIlonflip\fP
Sets the range of the longitude values used.
If \fIlonflip\fP=-1 then the longitude values will be in
the range from -360 to 0 degrees. If \fIlonflip\fP=0 
then the longitude values will be in
the range from -180 to 180 degrees. If \fIlonflip\fP=1 
then the longitude values will be in
the range from 0 to 360 degrees.
Default: \fIlonflip\fP = 0.
.TP
.B \fB-M\fInavformat\fP
Defines the format of the navigation data to be read.  Currently,
eight formats are allowed:

\fInavformat\fP = 1:  
.br
This format consists of ascii lines of the form:
 	time_d lon lat
.br
where time_d is decimal unix time in seconds since
1/1/1970 00:00:00.000 and lon and lat are in decimal degrees.
This form can be obtained from multibeam data files using \fBmblist\fP
with the \fB-O\fP\fIMXY\fP option. Some representative lines are:
 5440574.017	-129.505875	 46.445255
 5440574.100	-129.506045	 46.445083
 5440574.183	-129.506208	 46.445083
 5440574.267	-129.506542	 46.444913

\fInavformat\fP = 2:  
.br
This format consists of ascii lines of the form:
 	year mon day hour min sec lon lat
.br
where lon and lat are in decimal degrees.
This form can be obtained from multibeam data files using \fBmblist\fP
with the \fB-O\fP\fItXY\fP option. Some representative lines are:
 1981 05 06 04 14 01	-129.505875	 46.445255
 1981 05 06 04 14 06	-129.506045	 46.445083
 1981 05 06 04 14 11	-129.506208	 46.445083
 1981 05 06 04 14 16	-129.506542	 46.444913

\fInavformat\fP = 3:  
.br
This format consists of ascii lines of the form:
 	year jday hour min sec lon lat
.br
where jday is the julian day of the year and 
lon and lat are in decimal degrees.
This form can be obtained from multibeam data files using \fBmblist\fP
with the \fB-O\fP\fIJXY\fP option. Some representative lines are:
 1981 126 04 14 01	-129.505875	 46.445255
 1981 126 04 14 06	-129.506045	 46.445083
 1981 126 04 14 11	-129.506208	 46.445083
 1981 126 04 14 16	-129.506542	 46.444913

\fInavformat\fP = 4:  
.br
This format consists of ascii lines of the form:
 	year jday daymin sec lon lat
.br
where jday is the julian day of the year, daymin is the minute
of the day, and lon and lat are in decimal degrees.
This form can be obtained from multibeam data files using \fBmblist\fP
with the \fB-O\fP\fIjXY\fP option. Some representative lines are:
 1981 126 0254 01	-129.505875	 46.445255
 1981 126 0254 06	-129.506045	 46.445083
 1981 126 0254 11	-129.506208	 46.445083
 1981 126 0254 16	-129.506542	 46.444913

\fInavformat\fP = 5:  
.br
This is the L-DEO shipboard processing navigation format, which consists of
ascii records.  Some representative lines of the format are:
 91+300:21:00:00.000 S 31 56.5282 E 179 21.3922   gp1  25.7   0.4
 91+300:21:01:00.000 S 31 56.5746 E 179 21.5991   gp1  25.7   0.4
 91+300:21:02:00.000 S 31 56.6215 E 179 21.8051   gp1  25.7   0.4
 91+300:21:03:00.000 S 31 56.6679 E 179 22.0111   gp1  25.7   0.4
 |year               |north or south              |nav source 
   |julian day         |latitude degrees               |quality1
        |hour             |latitude minutes                   |quality2
           |minute                |east or west
              |second               |longitude degrees
                                        |longitude minutes

\fInavformat\fP = 6 or 7:  
.br
These two formats are subsets of the NMEA 0183 marine interface standard. 
The raw data stream includes ASCII "sentences"
beginning with the '$' character and ending with <CR><LF>
characters; the sentences recognized by \fBmbmerge\fP include
time (starting with "$--ZDA" or "$--UNX" where the "--" can
be any two characters) and navigation (starting with 
"$--GLL" or "$--GGA").
The time tags for each navigation point are taken from the last 
ZDA or UNX sentence read prior to the GLL or GGA sentence.
The GLL sentences used when \fInavformat\fP = 6 record 
generic position data, and the GGA sentences used when 
\fInavformat\fP = 7 record Global Positioning System (GPS)
fixes.
The sentence definitions are:
 
ZDA (Time and Date):
    $--ZDA,hhmmss.ss,dd,mm,yyyy<CR><LF>
           |     |   |  |  |
           |     |   |  |  +- Year
           |     |   |  +---- Month
           |     |   +------- Day
           |     +----------- Optional fractional 
           |                  seconds 
           +----------------- Hour, minute, and 
                              second
 
UNX (Nonstandard Time and Date derived from Unix host):
    $--UNX,hhmmss.ss,dd,mm,yyyy<CR><LF>
           |     |   |  |  |
           |     |   |  |  +- Year
           |     |   |  +---- Month
           |     |   +------- Day
           |     +----------- Optional fractional 
           |                  seconds 
           +----------------- Hour, minute, and 
                              second
 
GLL (Generic Position):
    $--GLL,ddmm.mm,N,dddmm.mm,E,hhmmss.ss,AXX<CR><LF>
           | |     | |  |     | |
           | |     | |  |     | +- UTC time as 
           | |     | |  |     |    in ZDA sentence
           | |     | |  |     +- E/W East or West
           | |     | |  +- Longitude decimal minutes
           | |     | +- Longitude degrees
           | |     +- N/S North or South
           | +- Latitude decimal minutes 
           +- Latitude degrees
 
GGA (GPS Position):
    $--GLL,hhmmss.ss,ddmm.mm,N,dddmm.mm,E,.....<CR><LF>
           |         | |     | |  |     |
           |         | |     | |  |     +- E/W East
           |         | |     | |  |        or West
           |         | |     | |  +- Longitude decimal 
           |         | |     | |     minutes
           |         | |     | +- Longitude degrees
           |         | |     +- N/S North or South
           |         | +- Latitude decimal minutes
           |         +- Latitude degrees 
           +- UTC time as in ZDA sentence

\fInavformat\fP = 8:  
.br
This is the Simrad 90 Position datagram format. This
type of navigation typically derives from the *.pos
files created by the Simrad Mermaid or Merlin data
logging packages (when run in the parallel file mode).
 
 Default: \fInavformat\fP = 5.
.TP
.B \fB-N\fInavfile\fP
Data file from which the navigation data will be read.
.TP
.B \fB-O\fIoutfile\fP
Data file to which the output data will be written. If
no output file is specified, the output will be written to
stdout. Default: \fIoutfile\fP = stdout.
.TP
.B \fB-V\fP
Normally, \fBmbmerge\fP works "silently" without outputting
anything to the stderr stream.  If the
\fB-V\fP flag is given, then \fBmbmerge\fP works in a "verbose" mode and
outputs the program version being used and all error status messages.

.SH EXAMPLES
Suppose one wishes to merge the good navigation in file navhs with the
raw Hydrosweep data (format 21) file hsraw, which currently 
contains real-time navigation.
The following will suffice:
 	mbmerge -F21 -Nnavhs -Ihsraw -Ohsgood

.SH SEE ALSO
\fBmbsystem\fP(l), \fBmblist\fP(l)

.SH BUGS
You tell me.
