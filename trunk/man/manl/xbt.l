.TH xbt l "20 August 1996" "MB-System 4.4" "MB-System 4.4"
.SH NAME
\fBxbt\fP - Overview of \fBxbt\fP proceedures on the R/V Ewing.

.SH VERSION
Version 4.4

.SH DESCRIPTION
On the R/V Ewing, \fIxbt\fP (eXpendable BathyThermograph) probes are used
to measure the temperature as a function of depth. This temperature is
used to estimate the sound velocity as a function of depth in the
upper (0 to 800 meters) of the water column.
.PP
Probes are launched over the stern or from the quarter out of a hand
held launcher which is stored in the space above the wet staging
("high bay") lab.
.PP
A Sparton BT/SV processor card Model 101 is intalled in a JDR-386/20 PC
clone in the main lab at the aft end of the row of graphic recorders.
Software running in this PC acquires and processes the \fIXBT\fP data.
During a drop, the temperature profile is displayed on the screen. At
the end of the drop, a data file is written on to the internal hard
disk.
.PP
After exiting the \fIXBT\fP application program, the new \fIXBT\fP
file is copied to a floppy disk for backup and to an NFS mounted file
system on the logging computer. The file naming convention is:
.IP	
	EWYYLLNN.xtD
.PP
Where:
.IP	
	EW signifies the data came from the Ewing
.br
	YY is the last two digits of the year
.br
	LL is the current Ewing leg number
.br
	NN is the sequential number of the \fIXBT\fP during the 
.br
           current leg
.br
	xt signifies that this is \fIxbt\fP data (vs. xsv data)
.br
	 D is the probe type designator
.PP
We do not use the Sparton software to convert the temperature
measurements into sound velocity. After the raw temperature data has
been moved to the logging computer it is converted to sound velocity
with \fImbm_xbt\fP and then merged with historical data from the
Levitus database produced with \fImblevitus\fP and entered into
Hydrosweep as sound velocity profile.
.SH AUTHORSHIP
Dale N. Chayes (dale@lamont.ldeo.columbia.edu) 
.br
Lamont-Doherty Earth Observatory
.br
Palisades, NY 10964
.br
wrote the man page.

.SH SEE ALSO
\fBmbsystem\fP(l), \fBmbm_xbt\fP(l), 
\fBmbvelocitytool\fP(l), \fBmblevitus\fP(l)

.SH BUGS
This documentation is virtually guaranteed to be out of date, and/or
just plain wrong, but at least we are trying. Suggestions are welcome.



