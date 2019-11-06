Test GCTPC: 

The test directory structure and contents for GCTPC are as follows:

   gctpc/test/test_proj

	files included:
	   testproj.c	--  "C" code to make files of projected values
	   Makefile	--  file used to compile code

   gctpc/test/proj

	files include:
	   Proj.name	-- shell script to run a test on each projection
	   <proj name>	-- 30 directories, one for each projection

   gctpc/test/proj/<proj name>

	files include:
	   data*	-- files containing projection data
	   parm*	-- parameter files to be used with "testproj"
	   <proj name>_test -- shell script to run tests on <proj name>
	   README	-- comments pertaining to testing--not included
			   with all projections
	
Throughout the test procedures, global variables are required.  They are
listed below along with their contents:

	LIBGCTP -- directory containing geolib.a 
	SRCGCTP -- directory containing proj.h and cproj.h
	CMPGCTP -- directory containing the TESTPROJ executable

The user has several options to test the projection values.  They are
listed below from most manual to most automated.  The information provided
in each option may apply to any option following it. 

Option 1--Use data file to compare individual points

    The data files (in the form--data<proj #><letter>) in the individual
    projection directories contain a table of projected values that may 
    be used to compare individual points.  The beginning of this file
    contains projection information.  The first two columns of the table
    are the Longitude and Latitude entered as input.  The third and
    fourth columns contain the x and y projected values using columns one
    and two as input.  The fifth and sixth columns contain the Longitude
    and Latitude results using columns three and four as input.

Option 2--Run "testproj" to make a grid for individual projections

    To compile testproj.c the global variable LIBGCTP must be set to the
    directory containing geolib.a and the global variable SRCGCTP must be
    set to the directory containing cproj.h and proj.h.  Type "make" in
    the directory gctpc/test/test_proj.

    To run "testproj" without a parameter file.  You will
    be prompted for the input.  Input includes:

	- Projection number
	- Zone code (for UTM and State Plane only)
	- Projection parameters (See Appendix A)
	- Range of Longitude
	- Range of Latitude
	- Increment of Longitude
	- Increment of Latitude
	- Name of data file containing new projected points
	- 'Yes' or 'No' if values are to be compared
	- If 'Yes':
		* Name of existing data file for comparison
		* Tolerance value representing the exceptable difference
		  between the new and existing data.
		* Name of comparison file which includes the long/lat
		  of any points whose difference is more than the tolerance
		  value and if an error occurs reading the input data.

 Option 3--Run "testproj" with a parameter file 

    Parameter files are located in each separate projection directory
    (in the form--parm<proj #><letter>).  These files may be
    redirected as input for "testproj".  All projection files are
    set up to compare the results with a data file called
    data<proj #><letter>.  The new data file will be called
    temp<proj #><letter> and the comparison file will be called
    cmp<proj #><letter>.

    Example:  testproj < parm10a

 Option 4--Run all parameters for a projection

    Each separate projection directory has a file called <proj name>_test
    that is a UNIX shell script for running "testproj" with all parameter
    files in that directory.  The global variable CMPGCTP must be set
    to the directory containing the executable "testproj".  See option
    3 for results.

    Example:  In directory gctpc/test/proj/stereo type

	"stereo_test"

 Option 5--Run all projection parameters at one time

    Proj.name located in gctpc/test/proj is a UNIX shell script that
    will run <proj name>_test in each projection directory.  This script
    changes to each directory and runs <proj name>_test.  The global
    variable CMPGCTP must be set as in Option 4 and the result will be
    the same as in Option 3.

 

			******** Disclaimer *********

This test procedure was set up to handle testing of GCTPC at EDC.  This
was not intended to be used for production.  Any part of this test procedure
may or may not be used.  This program has not been set up to handle errors
with input.  Therefore, unpredictable results may occur.  The user may want
to adapt testproj.c for their purposes.  For example, error messages are
currently sent to "error_file.txt".  This may be changed to the output file
so that the Lat/Long may be identified with the error.

Due to precision errors of systems, forward conversions may not be accurate
beyond the units digit for certain projections, however, most projections
should have more accuracy.  Be aware that sign differences may occur at
longitude 180 (-180) degrees and differences may occur at latitude 90 (-90)
degrees.  Input longitude and latitude values that lie outside of the
reasonable area of coverage for non-global projections will return
unpredictable results, as will input easting and northing values that
are outside of the data area.
