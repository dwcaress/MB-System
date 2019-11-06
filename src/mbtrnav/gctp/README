August 1998

                         ******** Disclaimer ********

The General Cartographic Transformation Package (GCTP) is a system of
software routines designed to permit the transformation of coordinate
pairs from one map projection to another.  The GCTP is the standard
computer software used by the National Mapping Division for map
projection computations.

Because of requirements internal to the EROS Data Center, the GCTP software
was converted from FORTRAN to the C programming language, and several new
projections were added.  During the conversion process, the software was
tested on the following UNIX systems:  Data General running DG/UX Release
5.4R2.01 AViiON, SUN running SunOS Release 4.1.3, and Silicon Graphics
running IRIX Release 5.3 IP7.  In addition, the software has also been
verified against the current National Mapping Division (NMD) GCTP
software.  This new version, GCTPC, is being made available informally
on a 'user-beware' basis.  EDC assumes no responsibility for the accuracy
of the routines in this package.

Because of decreasing budgets and impacts on software development staff, 
EDC will not be responsible for supporting this software package.  The 
user is responsible for using the package correctly.  A set of validation
data is provided so that each user/facility may perform its' own testing
of the programs.  A description of this data set and instructions for its
use are in the test/README file.  EDC will welcome any questions, comments, 
or suggestions, and will respond on a time-available basis based on our 
interests and requirements.  Communications concerning the GCTPC package 
may be directed to this email address:

		gctpc@edcmail.cr.usgs.gov

--------------------------------------------------------------------------------

The GCTPC Directory Structure is as follows:

    gctpc

	- README
	- Announce.txt contains information on retrieving and installing
	  GCTPC

    gctpc/doc:

	- Documentation for GCTPC

    gctpc/source:

	- Code for the GCTPC library.  A Makefile exists that has been
	  tested on Data General, SUN, and Silicon Graphics systems.

    gctpc/test:

	- Test procedures

Note:    README files located in each directory contain additional information.


Periodic updates and corrections will be put in this directory for access
by the science community.  Descriptions of these updates will be added to
this file.

Change Log:

Version	Date	Comments
-------	----- 	----------------------------------------------------------
 c.1.0  11/93 	Initial C version.  Includes all previous GCTP projections 
              	and Space Oblique Mercator (SOM), Robinson, and Alaska
	      	Conformal.  The projections Hammer, Interrupted Goode
	      	Homolosine, Mollweide, Interrupted Mollweide, Wagner IV,
	      	Wagner VII, and Oblated Equal Area have been added.

 c.1.1   1/95 	The argument OUTDATUM was added to the GCTP call to add
		The option to convert from one ellipsoid to another.
	      	The ability for UTM to be processed with any ellipsoid code
		was added.  It previously only supported Clarke 1866.
		Affected source files include:  gctp.c, inv_init.c,
		and for_init.c.

		Modifications have been made to the documentation for
		clarification.  Affected documents include: AppendixA.txt,
		for_init.ps, gctp.ps, and inv_init.ps.  in the document
		gctp.ps, the 12th spheroid code was changed from WALBECK
		to WGS 84.

 c.1.2	 2/95	Changed the perror call in imolwfor.c and wivfor.c to
		p_error.

		Files utmfor.c and utminv.c were developed from tmfor.c
		and tminv.c consecutively.  The UTM portion has been
		taken out of tmfor.c and tminv.c.  This change was due
		to errors that occurred when the utm and tm projection
		were run in the same direction (forward or inverse) in
		the same application. 

		Both changes to this version are internal and have no
		impact on the method of application.  No changes have
		been made to documentation.

 c.1.3	 2/96	A modification was made to the function alberinv.c.
		An error was brought to our attention that parenthesis
		around the argument "1.0 + e3" was missing in the
		assignment of the variable con on line 149.  This
		error was found by comparing the C code to the FORTRAN
		version.  The parenthesis have been added to the source.
		This did not change the results of our tests, therefore,
		the test data files have not been changed.

		A correction was made to the document xxxinv.ps.  The
		input and output names were switched in the parameter
		descriptions.  Also, an explanation was added to xxxfor.ps
		(xxxinv.ps) indicating that for_init (inv_init) must be
		called before calling xxxfor (xxxinv).  Small changes
		were made to gctp.ps, inv_init.ps, and for_init.ps for
		documentation consistency.

		In the file sphdz.c, the name of spheroid code number 12
		was incorrectly listed as Walbeck in the comment section.
		This is actually WGS 84 and has been changed accordingly.

c.1.4	5/97	The email address for GCTPC support has been changed from
		gctpc@edcserver1.cr.usgs.gov to gctpc@edcmail.cr.usgs.gov

c.2.0   3/98	Version 2.0 of GCTPC will be named gctpc20.tar on the
		anonymous ftp site which is different from the previous
		version which was called gctpc.tar.

		Fixed an error with the State Plane projection where no
		initialization was being done if the spheroid changed.
		Also, added an error check if the spheroid code is not
		0 (Clarke 1866) or 8 (GRS 80).  State Plane will only
		support these two spheroid codes to correspond with NAD27
		and NAD83.  An error code number 23 has been added to represent
		this error.

		New spheroids have been added to the list of standard
		spheroids.  This has been done because of the increasing
		demand for datum conversions.

		In the init function in report.c, changed successful return
		value from 0 to OK.  In polyinv.c, Changed the error returned
		to be the same as the flag returned from phi4z instead of
		74.  A list of the error codes and their corresponding messages
		were added to the doc directory in file error.codes.

		The word "datum" has been switched to "spheroid" within the
		code were appropriate.

		Took out unnessary line, ranlib, from the Makefile.

		Source code comments were added in various files.

		Added a file in the doc directory called error_codes.txt.  This
		file lists all the error code numbers and what they
		represent.

		Removed test data from the tm, snsoid, equidc, stereo, and
		som test directories.  This reduces the size of the package
		from 40.5 megabytes to 38.5 megabytes.  As time permits,
		more work will be done in this area.

		Corrected an error in the phi2z functions in the cproj.c file.
		the variable flag was being assigned incorrectly.

		Added outspheroid to the br_gctp call.

		Added function prototypes to the proj.h file.  Added proj.h
		cproj.h.  Added the function types to each function that was
		defaulted to int.  This was done to eliminate warning messages
		when compiling with certain flags.  I was able to compile with
		flags "-xansi -32 -mips2 -O2 -fullwarn" with only a few
		warnings from gctp.c and br_gctp.c.

		Added #include <stdlib.h> and #include <ctype.h> to testproj.c
		and took out #define NULL 0.

		An error occured on the SUN systems with the sincos subroutine.
		I took out the "ifndef" statement in the cproj.c file to allow
		SUN systems to compile and run the sincos subroutine.

