/*--------------------------------------------------------------------
 *    The MB-system:	mbr_mgd77tab.c	5/18/99
 *	$Id$
 *
 *    Copyright (c) 2014-2014 by
 *    David W. Caress (caress@mbari.org)
 *      Monterey Bay Aquarium Research Institute
 *      Moss Landing, CA 95039
 *    and Dale N. Chayes (dale@ldeo.columbia.edu)
 *      Lamont-Doherty Earth Observatory
 *      Palisades, NY 10964
 *
 *    See README file for copying and redistribution conditions.
 *--------------------------------------------------------------------*/
/*
 * mbr_mgd77tab.c contains the functions for reading and writing
 * multibeam data in the MGD77TAB format.
 * These functions include:
 *   mbr_alm_mgd77tab	- allocate read/write memory
 *   mbr_dem_mgd77tab	- deallocate read/write memory
 *   mbr_rt_mgd77tab	- read and translate data
 *   mbr_wt_mgd77tab	- translate and write data
 *
 * Author:	D. W. Caress
 * Date:	February 25, 2014
 *
 * $Log: mbr_mgd77tab.c,v $
 *
 */

/* standard include files */
#include <stdio.h>
#include <math.h>
#include <string.h>

/* mbio include files */
#include "mb_status.h"
#include "mb_format.h"
#include "mb_io.h"
#include "mb_define.h"
#include "mbsys_singlebeam.h"

/*
 * Notes on the MBF_MGD77TAB data format:
 *   1. The MGD77T format is is an exchange format for marine
 *	geophysical data (bathymetry, magnetics, and gravity).
 *      The format standard is maintained by the National
 *      Geophysical Data Center of NOAA.
 *   2. The MGD77T format is an update of the MGD77 format that
 *      replaces fixed length records with variable length, tab
 *      delimited records.
 *   3. Blank fields are denoted by having no characters at all
 *      between the leading and following tab characters for
 *      those fields.
 *   4. The MB-System implementation includes the support of
 *      an arbitrary number of comment records at the beginning
 *      of each file. The comment records are 120 bytes each
 *      and begin with the character '#'.
 *   5. The following notes derive from the format specification
 *      document.
 *
 * THE MGD77T HEADER RECORD
 * The purpose of the (survey) Header Record is to document both the content
 * and structure of the geophysical data contained within subsequent data
 * records. In general, documentation that is constant throughout the survey
 * will be in the Header Record, while data that is variable will be in the
 * Data Records.
 *
 * The Header Record contains fields which are numeric and freely formatted
 * text. Each field is followed by a tab character. Unspecified or unused
 * fields are nil (a tab immediately follows the previous field). Tabs are
 * generally omitted for any trailing unspecified fields, including the tab
 * for the last specified field.
 *
 * MGD77T headers for groups of surveys can be contained in a single file or
 * in separate files (one file per survey). Generally the grouping method will
 * be the same as that of the MGD77T data files.
 *
 * The MGD77T survey header(s) file may optionally have a heading record of
 * field identifiers as the first record of the file, followed by the actual
 * survey header(s). The field identifier names offered in this format
 * description below are not to be considered as the “official” field identifier
 * names. Users may offer their own field id names, or not insert a heading
 * record in the survey header(s) file. However, the use of name “FORMAT_77”
 * as the second field identifier of the first record of the file will make
 * it easy for applications to determine that this record is a heading record
 * for a MGD77T survey header(s) file in the same way that “MGD77T” in the
 * second field of a record identifies the record as a MGD77T survey header
 * record.
 *
 * Format Conventions for the MGD77T Header Record:
 *
 * The following is a detailed description of the Header Record for MGD77T.
 * Fields can be of type integer, floating point or character. Fields that always
 * represent whole numbers are designated as integers. Fields that may contain a
 * decimal component are float, and fields that are alphanumeric are character.
 *
 * 	[field #] Type FIELD_ID Description 
 * 	_____________________________________________________ 
 * 	[1]	char 	SURVEY_ID 	SURVEY IDENTIFIER  
 * 					Identifier supplied by the contributing
 * 					organization, else given by NGDC in a
 * 					manner which represents the data.
 * 					Identical to same in data record. 
 * 	[2]	char	FORMAT_77	FORMAT ACRONYM - Set to “MGD77T” 
 * 	[3]	char	CENTER_ID	DATA CENTER FILE NUMBER 
 * 					Survey identifier bestowed by the data
 * 					center.  
 * 	[4]	char	PARAMS_CO	PARAMETERS SURVEYED CODES  
 * 					Status of geophysical parameters for
 * 					this survey. This field must be nil,
 * 					or exactly 5 characters. 
 * 					nil – entire field unspecified 
 * 					COL. PARAMETER SURVEYED
 * 						1 - bathymetry
 * 						2 - magnetics
 * 						3 - gravity
 * 						4 - high-resolution seismics
 * 						5 - deep penetration seismics
 * 					_________________________________
 * 					CODE - (for columns 27-31)
 * 						0 - unspecified
 * 						1 - Parameter NOT surveyed
 * 						3 - Parameter surveyed, not in file
 * 						5 - Parameter surveyed, in file 
 * 	[5]	int	DATE_CREAT	FILE CREATION DATE - YYYYMMDD  
 * 					Date data records were last altered/assimilated 
 * 	[6]	char	INST_SRC	SOURCE INSTITUTION 
 * 					Organization which collected the data or
 * 					contributor if collector not definitive. 
 * 	[7]	char 	COUNTRY 	COUNTRY  
 * 	[8]	char 	PLATFORM 	PLATFORM NAME 
 * 	[9]	int 	PLAT_TYPCO 	PLATFORM TYPE CODE 
 * 						nil – Unspecified
 * 						1 - Surface ship
 * 						2 - Submersible ship
 * 						3 – Aircraft
 * 						4 – Buoy
 * 						5 - Mobile land
 * 						6 - Fixed land
 * 						7 - Deep tow
 * 						8 - Anchored seafloor instrument
 * 						9 - Other, specify in next field 
 * 	[10]	char 	PLAT_TYP 	PLATFORM TYPE  
 * 					e.g., "Ship","Plane", "Submarine", etc. 
 * 	[11]	char 	CHIEF 		CHIEF SCIENTIST(S) 
 * 	[12]	char 	PROJECT 	PROJECT, CRUISE, LEG  
 * 					e.g., "Survops 6-69", "Indopac, Leg3" 
 * 	[13]	char 	FUNDING 	FUNDING  
 * 					i.e. agency or institution 
 * 	[14]	int 	DATE_DEP 	SURVEY DEPARTURE DATE - YYYYMMDD 
 * 	[15]	char 	PORT_DEP 	PORT OF DEPARTURE  
 * 					i.e. city, country 
 * 	[16]	int 	DATE_ARR 	SURVEY ARRIVAL DATE - YYYYMMDD 
 * 	[17]	char 	PORT_ARR 	PORT OF ARRIVAL  
 * 					i.e. city, country 
 * 	[18]	char 	NAV_INSTR 	NAVIGATION INSTRUMENTATION 
 * 					e.g. "Sat/LORAN A/Sextant" 
 * 	[19]	char 	POS_INFO 	POSITION DETERMINATION METHOD / GEODETIC DATUM 
 * 					e.g. "WGS84/Primary-Satellite, Secondary-LORAN A" 
 * 	[20]	char 	BATH_INSTR 	BATHYMETRY INSTRUMENTATION 
 * 					Include information such as frequency,
 * 					beam width, and sweep speed of recorder. 
 * 	[21]	char 	BATH_ADD 	ADDITIONAL FORMS OF BATHYMETRY 
 * 					e.g., "Microfilm","Analog records" 
 * 	[22]	char 	MAG_INSTR 	MAGNETICS INSTRUMENTATION 
 * 					e.g., "Proton Precession Mag-GEOMETRICS G-801"
 * 					or “Airborne AN/ASQ-81 scalar magnetometer,
 * 					Total Field 1 and Total Field 2 record values
 * 					are Uncorrected and Corrected”. 
 * 	[23]	char 	MAG_ADD 	ADDITIONAL FORMS OF MAGNETICS 
 * 					e.g., "punch tape","analog records" 
 * 	[24]	char 	GRAV_INSTR 	GRAVITY INSTRUMENTATION 
 * 					e.g., "L&R S-26" 
 * 	[25]	char 	GRAV_ADD 	ADDITIONAL FORMS OF GRAVITY DATA 
 * 					e.g., "Microfilm","Analog records " 
 * 	[26]	char 	SEIS_INSTR 	SEISMIC INSTRUMENTATION 
 * 					Include the size of the sound source,
 * 					the recording frequency filters, and
 * 					the number of channels e.g.,
 * 					"1700 cu. in., Airgun, 8-62 Hz, 36 Channels" 
 * 	[27]	char 	SEIS_FRMTS 	FORMATS OF SEISMIC DATA 
 * 					e.g., "Digital SEG-Y", "Mylar Sections", etc. 
 * 	[28]	float 	LAT_TOP 	NORTHBOUND LATITUDE OF SURVEY 
 * 	[29]	float 	LAT_BOTTOM 	SOUTHBOUND LATITUDE OF SURVEY 
 * 	[30]	float 	LON_LEFT 	WESTBOUND LONGITUDE OF SURVEY 
 * 					Between -180 and 180 degrees 
 * 	[31]	float 	LON_RIGHT 	EASTBOUND LONGITUDE OF SURVEY 
 * 					Between -180 and 180 degrees 
 * 	[32]	float 	BATH_DRATE 	GENERAL DIGITIZING RATE OF BATHYMETRY 
 * 					In (tenths of minutes)/minutes.  The rate
 * 					which is present within the data records;
 * 					e.g., if values were coded every 30 seconds,
 * 					set to (050)/0.5 
 * 	[33]	char 	BATH_SRATE 	GENERAL SAMPLING RATE OF BATHYMETRY  
 * 					This rate is instrumentation dependent;
 * 					e.g., "1/Second" 
 * 	[34]	float 	SOUND_VEL 	ASSUMED SOUND VELOCITY 
 * 					In meters per second. Historically, in
 * 					the U.S., this speed has been 800 fathoms/sec,
 * 					which equals 1463.3 meters/sec.; current
 * 					recorders generally have a calibration of
 * 					1500 meters/sec; e.g. 1500 
 * 	[35]	int 	VDATUM_CO 	BATHYMETRIC VERTICAL DATUM CODE - 
 * 						00 - No correction applied
 * 						01 - Lowest normal low water
 * 						02 - Mean lower low water
 * 						03 - Lowest low water
 * 						04 - Mean lower low water spring
 * 						05 - Indian spring low water
 * 						06 - Mean low water spring
 * 						07 - Mean sea level
 * 						08 - Mean low water
 * 						09 - Equatorial spring low water
 * 						10 - Tropic lower low water
 * 						11 - Lowest astronomical tide
 * 						88 - Other, specify in Add. Doc. 
 * 	[36]	char 	BATH_INTRP 	INTERPOLATION SCHEME 
 * 					This field allows for a description of
 * 					the interpolation scheme used, should
 * 					some of the data records contain interpolated
 * 					values; e.g., "5-minute intervals and peaks
 * 					and troughs" 
 * 	[37]	float 	MAG_DRATE 	GENERAL DIGITIZING RATE OF MAGNETICS  
 * 					In minutes.  The rate which is present
 * 					within the data records. 
 * 	[38]	float 	MAG_SRATE 	GENERAL SAMPLING RATE OF MAGNETICS 
 * 					In seconds.  This rate is instrumentation
 * 					dependent  e.g., if the pulse rate is
 * 					every 3 sec, set to 3 
 * 	[39]	float 	MAG_TOWDST 	MAGNETIC SENSOR TOW DISTANCE 
 * 					In meters.  The distance from the navigation
 * 					reference to the leading sensor. 
 * 	[40]	float 	MAG_SNSDEP 	SENSOR DEPTH 
 * 					In meters. This is the average depth (positive)
 * 					of the lead magnetic sensor, or flight altitude
 * 					(negative) if data is aeromagnetics. 
 * 	[41]	float 	MAG_SNSSEP 	HORIZONTAL SENSOR SEPARATION 
 * 					In meters.  Where two sensors are used. 
 * 	[42]	int 	M_REFFL_CO 	REFERENCE FIELD CODE 
 * 					This is the reference field used to determine
 * 					the residual magnetics:
 * 						00 - Unused
 * 						01 - AWC 70
 * 						02 - AWC 75
 * 						03 - IGRF-65
 * 						04 - IGRF-75
 * 						05 - GSFC-1266
 * 						06 - GSFC (POGO) 0674
 * 						07 - UK 75
 * 						08 - POGO 0368
 * 						09 - POGO 1068
 * 						10 - POGO 0869
 * 						11 - IGRF-80
 * 						12 - IGRF-85
 * 						13 - IGRF-90
 * 						14 - IGRF-95
 * 						15 - IGRF-00
 * 						16 - IGRF-05
 * 						17 - IGRF-10
 * 						18 - IGRF-11
 * 						88 - Other, specify
 * 						nil – Unspecified reference field code 
 * 	[43]	char 	MAG_REFFLD 	REFERENCE FIELD 
 * 					e.g., "IGRF-85" 
 * 	[44]	char 	MAG_RF_MTH 	METHOD OF APPLYING RESIDUAL FIELD 
 * 					The procedure used in applying this
 * 					reduction to the data; e.g.,
 * 					"Linear Interp. in 60-mile Square" 
 * 	[45]	float 	GRAV_DRATE 	GENERAL DIGITIZING RATE OF GRAVITY 
 * 					In minutes. The rate present within
 * 					the data records 
 * 	[46]	float 	GRAV_SRATE 	GENERAL SAMPLING RATE OF GRAVITY 
 * 					In seconds.  This rate is
 * 					instrumentation dependent. If recording
 * 					is continuous, set to 0 
 * 	[47]	int 	G_FORMU_CO 	THEORETICAL GRAVITY FORMULA CODE 
 * 						1 - Heiskanen 1924
 * 						2 - International 1930
 * 						3 - IAG System 1967
 * 						4 - IAG System 1980
 * 						8 - Other, specify 
 * 	[48]	char 	GRAV_FORMU 	THEORETICAL GRAVITY FORMULA  
 * 					e.g., "International '30",
 * 					"IAG System 1967", etc. 
 * 	[49]	int 	G_RFSYS_CO 	REFERENCE SYSTEM CODE 
 * 					Identifies the reference field:
 * 						1 - Local system, specify
 * 						2 - Potsdam system
 * 						3 - System IGSN 71
 * 						9 - Other, specify 
 * 	[50]	char 	GRAV_RFSYS 	REFERENCE SYSTEM 
 * 					e.g., "Potsdam System",
 * 					"System IGSN 71", etc. 
 * 	[51]	char 	GRAV_CORR 	CORRECTIONS APPLIED 
 * 					Drift, tare and bias corrections applied;
 * 					e.g. "+0.075 mgal per day 
 * 	[52]	float 	G_ST_DEP_G 	DEPARTURE BASE STATION GRAVITY 
 * 					In milligals.  At sea level - Network
 * 					value preferred. 
 * 	[53]	char 	G_ST_DEP 	DEPARTURE BASE STATION DESCRIPTION 
 * 					Indicates name and number of station 
 * 	[54]	float 	G_ST_ARR_G 	ARRIVAL BASE STATION GRAVITY 
 * 					In milligals.  At sea level - Network
 * 					value preferred. 
 * 	[55]	char 	G_ST_ARR 	ARRIVAL BASE STATION DESCRIPTION 
 * 					Indicates name and number of station 
 * 	[56]	int 	IDS_10_NUM 	NUMBER OF 10-DEGREE IDENTIFIERS  
 * 					This is the number of 4-digit 10-degree
 * 					identifiers which will follow this field,
 * 					excluding the "9999" flag. (see APPENDIX A) 
 * 	[57]	char 	IDS_10DEG 	10-DEGREE IDENTIFIERS 
 * 					A series of 4-digit codes, separated by
 * 					commas, which identify the 10-degree squares
 * 					through which the survey collected data
 * 					(see APPENDIX A). Code "9999" after last
 * 					identifier. 
 * 	[58]	 char 	ADD_DOC 	ADDITIONAL DOCUMENTATION 
 * 					Information concerning this survey not
 * 					contained in other header fields.  Embedded
 * 					End-of-Line characters are NOT ALLOWED. 

 * THE MGD77T DATA RECORD
 * The data record presents underway marine geophysical data in a correlative
 * manner. Geophysical data (bathymetry, magnetics, and gravity) and
 * seismic/segment identification (e.g. seismic line and shot-point ids)
 * are presented with a corresponding time and position. Documentation that
 * is variable throughout the survey also is included within each data record.
 * If primary navigation exists at a juncture where no geophysical data are
 * present, this record should be included with the data parameter fields
 * left unspecified (nil).
 * 
 * MGD77T data files for groups of surveys can be contained in a single file,
 * or in separate files (one file per survey). Generally the grouping method
 * will be the same as that of the MGD77T header files.
 * 
 * Format Conventions for the MGD77T Data Record:
 * 	1. For floating pt numbers, all decimal points are explicit
 * 		(e.g. 123.456 signifies a value of +123.456)
 * 	2. Leading zeros and blanks are discouraged in numeric fields.
 * 		Trailing blanks in numeric fields are not allowed.
 * 	3. Where float values are whole numbers, the decimal part/decimal pt
 * 		are not required. 4. In floats, trailing zeros after the last
 * 		significant digit past the decimal are not required.
 * 	5. Unspecified fields are nil (tab immediately follows previous field’s
 * 		delimiting tab).
 * 	6. All character fields should be trimmed of beginning and ending blanks.
 * 	7. Trailing tabs (trailing unspecified values) are generally omitted,
 * 		including the tab for the last specified (used) field.
 * 	8. All "corrections", such as time zone, diurnal magnetics, and Eotvos,
 * 		are understood to be added (e.g., time-zone correction is the
 * 		number of hours which must be added to the recorded time to
 * 		determine GMT).
 * 	9. For field values which differ from the definitions below, use the
 * 		Additional Documentation to describe how the values were arrived at.
 * 	10. A Column Heading record with will generally be the first record of
 * 		each data file. This can be omitted. The suggested Field Ids
 * 		of the data records should be considered, as they work well
 * 		with GEODAS and other software, and as shape file descriptors.
 * The following is a detailed description of the MGD77T Data Record. Fields
 * can be of type integer, floating point, or character. Fields that always
 * represent whole numbers are described as type int; fields that may contain
 * a decimal component are float, and fields that are alphanumeric are char.
 * 	[Field #] 	Type 	Field Id 	Description
 * 	____________________________________________________________
 *
 * 	[1]	char	SURVEY_ID	SURVEY IDENTIFIER
 * 					Identifier supplied by the contributing
 * 					organization, else given by NGDC in a
 * 					manner which represents the data.
 * 					Identical to that in MGD77/MGD77T header
 * 					record. 
 * 	[2]	float	TIMEZONE	TIME-ZONE CORRECTION 
 * 					In hours. Corrects time (in fields 3-4)
 * 					to GMT when added: equals zero when time
 * 					is GMT.  Time-zone normally falls between
 * 					-13 and +12 inclusively. 
 * 	[3]	int	DATE 		DATE (YYYYMMDD) 
 * 					e.g. 19720530 
 * 	[4]	float	TIME 		TIME 
 * 					Hours and decimal minutes
 * 					i.e. 11:59:40pm = 2359.6667 
 * 	[5]	float	LAT 		LATITUDE 
 * 					in decimal degrees + = North;
 * 					- = South Between -90 and 90 degrees 
 * 	[6]	float	LON 		LONGITUDE 
 * 					in decimal degrees + = East;
 * 					- = West Between -180 and 180 degrees 
 * 	[7]	int	POS_TYPE 	POSITION TYPE CODE 
 * 					Indicates how lat/lon was obtained:
 * 						1 = Observed fix
 * 						3 = Interpolated
 * 						nil = Unspecified 
 * 	[8]	int	NAV_QUALCO 	QUALITY CODE FOR NAVIGATION 
 * 						1 – good
 * 						2 – fair
 * 						3 – poor
 * 						4 – bad
 * 						5 – Suspected, by the originating institution
 * 						6 – Suspected, by the Data Center
 * 						nil – Unspecified
 * 					(Note: - Should Institution code the field as 1
 * 					through 5, the data center will not contradict.) 
 * 	[9]	float	BAT_TTIME 	BATHYMETRY, 2- WAY TRAVELTIME 
 * 					In seconds  Corrected for transducer depth and
 * 					other such corrections, especially in shallow water 
 * 	[10]	float	CORR_DEPTH 	BATHYMETRY, CORRECTED DEPTH 
 * 					In (positive) meters. e.g. 1234.56 
 * 	[11]	int	BAT_CPCO 	BATHYMETRIC CORRECTION CODE 
 * 					This code details the procedure used for
 * 					determining the sound velocity correction to depth:
 * 						01-55 – Matthews' Zones with zone
 * 						59 – Matthews' Zones, no zone specified
 * 						60 – S. Kuwahara Formula
 * 						61 – Wilson Formula
 * 						62 – Del Grosso Formula
 * 						63 – Carter's Tables
 * 						88 – Other (see Add. Doc.)
 * 						97 – Computed using 1500 meters/sec
 * 						98 – Unknown if Corrected nil – Unspecified 
 * 	[12]	int	BAT_TYPCO 	BATHYMETRIC TYPE CODE 
 * 					Indicates how the bathymetric value was obtained:
 * 						1 – Observed
 * 						3 – Interpolated
 * 						nil – Unspecified 
 * 	[13]	int	BAT_QUALCO 	QUALITY CODE FOR BATHYMETRY 
 * 						1 – good
 * 						2 – fair
 * 						3 – poor
 * 						4 – bad
 * 						5 – Suspected bad by Contributor
 * 						6 – Suspected bad by Data Center
 * 						nil - Unspecified 
 * 	[14]	float	MAG_TOT 	MAGNETICS TOTAL FIELD, 1ST SENSOR 
 * 					In nanoteslas; for leading sensor. Use
 * 					this field for single sensor, or for
 * 					aeromagnetics this can be the Uncorrected
 * 					Total Field value (Detail in MAGNETICS
 * 					INSTRUMENTATION header field). 
 * 	[15]	float	MAG_TOT2 	MAGNETICS TOTAL FIELD, 2ND SENSOR 
 * 					In nanoteslas; for trailing sensor, or
 * 					for aeromagnetics this can be the
 * 					Corrected Total Field value, (detail in
 * 					MAGNETICS INSTRUMENTATION and Additional
 * 					Documentation header fields). 
 * 	[16]	float	MAG_RES 	MAGNETICS RESIDUAL FIELD 
 * 					In nanoteslas; (reference field used is
 * 					in Header) 
 * 	[17]	int	MAG_RESSEN 	SENSOR FOR RESIDUAL FIELD 
 * 						1 - 1st or leading sensor
 * 						2 - 2nd or trailing sensor
 * 						nil – Unspecified (or single sensor) 
 * 	[18]	float	MAG_DICORR 	MAGNETICS DIURNAL CORRECTION - 
 * 					In nanoteslas (gammas). If nil, total and
 * 					residual fields are assumed to be
 * 					uncorrected; if used, total and residuals
 * 					are assumed to have been already corrected
 * 					with diurnal. 
 * 	[19]	int	MAG_SDEPTH 	DEPTH/ALTITUDE OF MAGNETICS SENSOR 
 * 					In meters.
 * 						+ = Below sea-level,
 * 						- = Above sea-level 
 * 	[20]	int	MAG_QUALCO 	QUALITY CODE FOR MAGNETICS 
 * 						1 – good
 * 						2 – fair
 * 						3 – poor
 * 						4 – bad
 * 						5 – Suspected bad by Contributor
 * 						6 – Suspected bad by Data Center
 * 						nil - Unspecified 
 * 	[21]	float	GRA_OBS 	OBSERVED GRAVITY 
 * 					In milligals  Corrected for Eotvos,
 * 					drift, and tares 
 * 	[22]	float	EOTVOS 		EOTVOS CORRECTION 
 * 					In milligals
 * 						E = 7.5 V cos(phi) * sin(alpha)
 * 							+ 0.0042 V*V 
 * 	[23]	float	FREEAIR 	FREE-AIR ANOMALY 
 * 					In milligals
 * 						Free-air Anomaly = G(observed)
 * 							minus G(theoretical) 
 * 	[24]	int	GRA_QUALCO 	QUALITY CODE FOR GRAVITY 
 * 						1 – good
 * 						2 – fair
 * 						3 – poor
 * 						4 – bad
 * 						5 – Suspected bad by Contributor
 * 						6 – Suspected bad by Data Center
 * 						nil - Unspecified 
 * 	[25]	int	LINEID 		LINE/TRACK/SEGMENT ID 
 * 					Used, for example, to cross reference
 * 					with seismic data. 
 * 	[26]	int	POINTID 	SEISMIC SHOT-POINT NUMBER/POINT ID 
 *
 * ___________________________________________________________
 * 
 * 10-DEGREE-SQUARE IDENTIFIER CODE 
 * A 10-degree-square area can be easily identified by constructing a four-digit
 * number.  The components of this number, in order of their construction are
 * described as follows: 
 * Quadrant - A one-digit number identifies the quadrant of the world with the
 * following significance to each digit: 
 *   1st digit = Quadrant number 
 *   Qc Code	Latitude	Longitude 
 *   _______ 	________	_________ 
 *     1         North		East 
 *     3         South		East 
 *     5         South		West 
 *     7         North		West 
 * 10-Degree Square - The next three digits identify a unique 10-degree square;
 * thus the significant digits consist of: 
 *	2nd digit = Tens digit of degrees latitude 
 *      3rd digit = Hundreds digit of degrees longitude  
 *      4th digit = Tens digit of degrees longitude 
 *  
 * 10-DEGREE SQ IDENT. CODE  
 * ________________________ 
 * 	Example Quad Lat Long Long 
 * 	37 deg 48'S, 	4 deg 13'E 	3 3 0 0 
 * 	21.6 deg S, 	14.3 deg W 	5 2 0 1 
 * 	34 deg 28'N, 	143 deg 27'W 	7 3 1 4 
 * 	75 deg N, 	43 deg E 	1 7 0 4 
 * ___________________________________________________________
 */

/* header and data record in bytes */
#define MBF_MGD77TAB_HEADER_FIELDS	58
#define MBF_MGD77TAB_DATA_FIELDS	26

struct mbf_mgd77tab_struct
	{
	/* type of data record */
	int	kind;

	/* data record flags */
	int	defined_survey_id;
	int	defined_timezone;
	int	defined_date;
	int	defined_time;
	int	defined_lat;
	int	defined_lon;
	int	defined_pos_type;
	int	defined_nav_qualco;
	int	defined_bat_ttime;
	int	defined_corr_depth;
	int	defined_bat_cpco;
	int	defined_bat_typco;
	int	defined_bat_qualco;
	int	defined_mag_tot;
	int	defined_mag_tot2;
	int	defined_mag_res;
	int	defined_mag_ressen;
	int	defined_mag_dicorr;
	int	defined_mag_sdepth;
	int	defined_mag_qualco;
	int	defined_gra_obs;
	int	defined_eotvos;
	int	defined_freeair;
	int	defined_gra_qualco;
	int	defined_lineid;
	int	defined_pointid;
	int	last_field_defined;

	/* data record values */
	char	survey_id[8]; 	/* survey identifier
					identifier supplied by the contributing
					organization, else given by ngdc in a
					manner which represents the data.
					identical to that in mgd77/mgd77t header
					record. */

	float	timezone;   	/* time-zone correction 
					in hours. corrects time (in fields 3-4)
					to gmt when added: equals zero when time
					is gmt.  time-zone normally falls between
					-13 and +12 inclusively. */
	int	date;		/* date (yyyymmdd) e.g. 19720530 */
	float	time;		/* time hours and decimal minutes
					i.e. 11:59:40pm = 2359.6667 */
	float	lat;		/* latitude 
					in decimal degrees + = north;
					- = south between -90 and 90 degrees  */
	float	lon;		/* longitude 
					in decimal degrees + = east;
					- = west between -180 and 180 degrees  */
	int	pos_type;	/* position type code 
					indicates how lat/lon was obtained:
						1 = observed fix
						3 = interpolated
						nil = unspecified  */
	int	nav_qualco;	/* quality code for navigation 
						1 – good
						2 – fair
						3 – poor
						4 – bad
 						5 – suspected, by the originating institution
 						6 – suspected, by the data center
 						nil – unspecified
					(note: - should institution code the field as 1
					through 5, the data center will not contradict.)  */
	float	bat_ttime; 	/* bathymetry, 2- way traveltime 
					in seconds  corrected for transducer depth and
					other such corrections, especially in shallow water  */
	float	corr_depth; 	/* bathymetry, corrected depth 
 					in (positive) meters. e.g. 1234.56  */
	int	bat_cpco; 	/* bathymetric correction code 
 					this code details the procedure used for
 					determining the sound velocity correction to depth:
 						01-55 – matthews' zones with zone
 						59 – matthews' zones, no zone specified
 						60 – s. kuwahara formula
 						61 – wilson formula
 						62 – del grosso formula
 						63 – carter's tables
 						88 – other (see add. doc.)
 						97 – computed using 1500 meters/sec
 						98 – unknown if corrected nil – unspecified  */
	int	bat_typco; 	/* bathymetric type code 
 					indicates how the bathymetric value was obtained:
 						1 – observed
 						3 – interpolated
 						nil – unspecified  */
	int	bat_qualco; 	/* quality code for bathymetry 
 						1 – good
 						2 – fair
 						3 – poor
 						4 – bad
 						5 – suspected bad by contributor
 						6 – suspected bad by data center
 						nil - unspecified  */
	float	mag_tot; 	/* magnetics total field, 1st sensor 
 					in nanoteslas; for leading sensor. use
 					this field for single sensor, or for
 					aeromagnetics this can be the uncorrected
 					total field value (detail in magnetics
 					instrumentation header field).  */
	float	mag_tot2; 	/* magnetics total field, 2nd sensor 
 					in nanoteslas; for trailing sensor, or
 					for aeromagnetics this can be the
 					corrected total field value, (detail in
 					magnetics instrumentation and additional
 					documentation header fields).  */
	float	mag_res; 	/* magnetics residual field 
 					in nanoteslas; (reference field used is
 					in header)  */
	int	mag_ressen; 	/* sensor for residual field 
 						1 - 1st or leading sensor
 						2 - 2nd or trailing sensor
 						nil – unspecified (or single sensor)  */
	float	mag_dicorr; 	/* magnetics diurnal correction - 
 					in nanoteslas (gammas). if nil, total and
 					residual fields are assumed to be
 					uncorrected; if used, total and residuals
 					are assumed to have been already corrected
 					with diurnal.  */
	int	mag_sdepth; 	/* depth/altitude of magnetics sensor 
 					in meters.
 						+ = below sea-level,
 						- = above sea-level  */
	int	mag_qualco; 	/* quality code for magnetics 
 						1 – good
 						2 – fair
 						3 – poor
 						4 – bad
 						5 – suspected bad by contributor
 						6 – suspected bad by data center
 						nil - unspecified  */
	float	gra_obs; 	/* observed gravity 
 					in milligals  corrected for eotvos,
 					drift, and tares  */
	float	eotvos; 		/* eotvos correction 
 					in milligals
 						e = 7.5 v cos(phi) * sin(alpha)
 							+ 0.0042 v*v  */
	float	freeair; 	/* free-air anomaly 
 					in milligals
 						free-air anomaly = g(observed)
 							minus g(theoretical)  */
 	int	gra_qualco; 	/* quality code for gravity 
 						1 – good
 						2 – fair
 						3 – poor
 						4 – bad
 						5 – suspected bad by contributor
 						6 – suspected bad by data center
 						nil - unspecified  */
	int	lineid; 		/* line/track/segment id 
 					used, for example, to cross reference
 					with seismic data.  */
	int	pointid; 	/* seismic shot-point number/point id */

	/* comment */
	char	comment[MB_COMMENT_MAXLINE];
	};

/* essential function prototypes */
int mbr_register_mgd77tab(int verbose, void *mbio_ptr,
		int *error);
int mbr_info_mgd77tab(int verbose,
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
			int *svp_source,
			double *beamwidth_xtrack,
			double *beamwidth_ltrack,
			int *error);
int mbr_alm_mgd77tab(int verbose, void *mbio_ptr, int *error);
int mbr_dem_mgd77tab(int verbose, void *mbio_ptr, int *error);
int mbr_rt_mgd77tab(int verbose, void *mbio_ptr, void *store_ptr, int *error);
int mbr_wt_mgd77tab(int verbose, void *mbio_ptr, void *store_ptr, int *error);
int mbr_mgd77tab_rd_data(int verbose, void *mbio_ptr, int *error);
int mbr_mgd77tab_wr_data(int verbose, void *mbio_ptr, void *data_ptr, int *error);

static char rcs_id[]="$Id$";

/*--------------------------------------------------------------------*/
int mbr_register_mgd77tab(int verbose, void *mbio_ptr, int *error)
{
	char	*function_name = "mbr_register_mgd77tab";
	int	status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",function_name);
		fprintf(stderr,"dbg2  Revision id: %s\n",rcs_id);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		}

	/* get mb_io_ptr */
	mb_io_ptr = (struct mb_io_struct *) mbio_ptr;

	/* set format info parameters */
	status = mbr_info_mgd77tab(verbose,
			&mb_io_ptr->system,
			&mb_io_ptr->beams_bath_max,
			&mb_io_ptr->beams_amp_max,
			&mb_io_ptr->pixels_ss_max,
			mb_io_ptr->format_name,
			mb_io_ptr->system_name,
			mb_io_ptr->format_description,
			&mb_io_ptr->numfile,
			&mb_io_ptr->filetype,
			&mb_io_ptr->variable_beams,
			&mb_io_ptr->traveltime,
			&mb_io_ptr->beam_flagging,
			&mb_io_ptr->nav_source,
			&mb_io_ptr->heading_source,
			&mb_io_ptr->vru_source,
			&mb_io_ptr->svp_source,
			&mb_io_ptr->beamwidth_xtrack,
			&mb_io_ptr->beamwidth_ltrack,
			error);

	/* set format and system specific function pointers */
	mb_io_ptr->mb_io_format_alloc = &mbr_alm_mgd77tab;
	mb_io_ptr->mb_io_format_free = &mbr_dem_mgd77tab;
	mb_io_ptr->mb_io_store_alloc = &mbsys_singlebeam_alloc;
	mb_io_ptr->mb_io_store_free = &mbsys_singlebeam_deall;
	mb_io_ptr->mb_io_read_ping = &mbr_rt_mgd77tab;
	mb_io_ptr->mb_io_write_ping = &mbr_wt_mgd77tab;
	mb_io_ptr->mb_io_dimensions = &mbsys_singlebeam_dimensions;
	mb_io_ptr->mb_io_extract = &mbsys_singlebeam_extract;
	mb_io_ptr->mb_io_insert = &mbsys_singlebeam_insert;
	mb_io_ptr->mb_io_extract_nav = &mbsys_singlebeam_extract_nav;
	mb_io_ptr->mb_io_insert_nav = &mbsys_singlebeam_insert_nav;
	mb_io_ptr->mb_io_extract_altitude = &mbsys_singlebeam_extract_altitude;
	mb_io_ptr->mb_io_insert_altitude = NULL;
	mb_io_ptr->mb_io_extract_svp = NULL;
	mb_io_ptr->mb_io_insert_svp = NULL;
	mb_io_ptr->mb_io_ttimes = &mbsys_singlebeam_ttimes;
	mb_io_ptr->mb_io_detects = &mbsys_singlebeam_detects;
	mb_io_ptr->mb_io_copyrecord = &mbsys_singlebeam_copy;
	mb_io_ptr->mb_io_extract_rawss = NULL;
	mb_io_ptr->mb_io_insert_rawss = NULL;

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",function_name);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       system:             %d\n",mb_io_ptr->system);
		fprintf(stderr,"dbg2       beams_bath_max:     %d\n",mb_io_ptr->beams_bath_max);
		fprintf(stderr,"dbg2       beams_amp_max:      %d\n",mb_io_ptr->beams_amp_max);
		fprintf(stderr,"dbg2       pixels_ss_max:      %d\n",mb_io_ptr->pixels_ss_max);
		fprintf(stderr,"dbg2       format_name:        %s\n",mb_io_ptr->format_name);
		fprintf(stderr,"dbg2       system_name:        %s\n",mb_io_ptr->system_name);
		fprintf(stderr,"dbg2       format_description: %s\n",mb_io_ptr->format_description);
		fprintf(stderr,"dbg2       numfile:            %d\n",mb_io_ptr->numfile);
		fprintf(stderr,"dbg2       filetype:           %d\n",mb_io_ptr->filetype);
		fprintf(stderr,"dbg2       variable_beams:     %d\n",mb_io_ptr->variable_beams);
		fprintf(stderr,"dbg2       traveltime:         %d\n",mb_io_ptr->traveltime);
		fprintf(stderr,"dbg2       beam_flagging:      %d\n",mb_io_ptr->beam_flagging);
		fprintf(stderr,"dbg2       nav_source:         %d\n",mb_io_ptr->nav_source);
		fprintf(stderr,"dbg2       heading_source:     %d\n",mb_io_ptr->heading_source);
		fprintf(stderr,"dbg2       vru_source:         %d\n",mb_io_ptr->vru_source);
		fprintf(stderr,"dbg2       svp_source:         %d\n",mb_io_ptr->svp_source);
		fprintf(stderr,"dbg2       beamwidth_xtrack:   %f\n",mb_io_ptr->beamwidth_xtrack);
		fprintf(stderr,"dbg2       beamwidth_ltrack:   %f\n",mb_io_ptr->beamwidth_ltrack);
		fprintf(stderr,"dbg2       format_alloc:       %p\n",(void *)mb_io_ptr->mb_io_format_alloc);
		fprintf(stderr,"dbg2       format_free:        %p\n",(void *)mb_io_ptr->mb_io_format_free);
		fprintf(stderr,"dbg2       store_alloc:        %p\n",(void *)mb_io_ptr->mb_io_store_alloc);
		fprintf(stderr,"dbg2       store_free:         %p\n",(void *)mb_io_ptr->mb_io_store_free);
		fprintf(stderr,"dbg2       read_ping:          %p\n",(void *)mb_io_ptr->mb_io_read_ping);
		fprintf(stderr,"dbg2       write_ping:         %p\n",(void *)mb_io_ptr->mb_io_write_ping);
		fprintf(stderr,"dbg2       extract:            %p\n",(void *)mb_io_ptr->mb_io_extract);
		fprintf(stderr,"dbg2       insert:             %p\n",(void *)mb_io_ptr->mb_io_insert);
		fprintf(stderr,"dbg2       extract_nav:        %p\n",(void *)mb_io_ptr->mb_io_extract_nav);
		fprintf(stderr,"dbg2       insert_nav:         %p\n",(void *)mb_io_ptr->mb_io_insert_nav);
		fprintf(stderr,"dbg2       extract_altitude:   %p\n",(void *)mb_io_ptr->mb_io_extract_altitude);
		fprintf(stderr,"dbg2       insert_altitude:    %p\n",(void *)mb_io_ptr->mb_io_insert_altitude);
		fprintf(stderr,"dbg2       extract_svp:        %p\n",(void *)mb_io_ptr->mb_io_extract_svp);
		fprintf(stderr,"dbg2       insert_svp:         %p\n",(void *)mb_io_ptr->mb_io_insert_svp);
		fprintf(stderr,"dbg2       ttimes:             %p\n",(void *)mb_io_ptr->mb_io_ttimes);
		fprintf(stderr,"dbg2       detects:            %p\n",(void *)mb_io_ptr->mb_io_detects);
		fprintf(stderr,"dbg2       extract_rawss:      %p\n",(void *)mb_io_ptr->mb_io_extract_rawss);
		fprintf(stderr,"dbg2       insert_rawss:       %p\n",(void *)mb_io_ptr->mb_io_insert_rawss);
		fprintf(stderr,"dbg2       copyrecord:         %p\n",(void *)mb_io_ptr->mb_io_copyrecord);
		fprintf(stderr,"dbg2       error:              %d\n",*error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:         %d\n",status);
		}

	/* return status */
	return(status);
}

/*--------------------------------------------------------------------*/
int mbr_info_mgd77tab(int verbose,
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
			int *svp_source,
			double *beamwidth_xtrack,
			double *beamwidth_ltrack,
			int *error)
{
	char	*function_name = "mbr_info_mgd77tab";
	int	status = MB_SUCCESS;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",function_name);
		fprintf(stderr,"dbg2  Revision id: %s\n",rcs_id);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		}

	/* set format info parameters */
	status = MB_SUCCESS;
	*error = MB_ERROR_NO_ERROR;
	*system = MB_SYS_SINGLEBEAM;
	*beams_bath_max = 1;
	*beams_amp_max = 0;
	*pixels_ss_max = 0;
	strncpy(format_name, "MGD77TAB", MB_NAME_LENGTH);
	strncpy(system_name, "SINGLEBEAM", MB_NAME_LENGTH);
	strncpy(format_description, "Format name:          MBF_MGD77TAB\nInformal Description: NGDC MGD77 underway geophysics format\nAttributes:           single beam bathymetry, nav, magnetics, gravity,\n                      122 byte ascii records with CRLF line breaks, NOAA NGDC\n", MB_DESCRIPTION_LENGTH);
	*numfile = 1;
	*filetype = MB_FILETYPE_NORMAL;
	*variable_beams = MB_NO;
	*traveltime = MB_YES;
	*beam_flagging = MB_NO;
	*nav_source = MB_DATA_DATA;
	*heading_source = MB_DATA_DATA;
	*vru_source = MB_DATA_DATA;
	*svp_source = MB_DATA_NONE;
	*beamwidth_xtrack = 0.0;
	*beamwidth_ltrack = 0.0;

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",function_name);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       system:             %d\n",*system);
		fprintf(stderr,"dbg2       beams_bath_max:     %d\n",*beams_bath_max);
		fprintf(stderr,"dbg2       beams_amp_max:      %d\n",*beams_amp_max);
		fprintf(stderr,"dbg2       pixels_ss_max:      %d\n",*pixels_ss_max);
		fprintf(stderr,"dbg2       format_name:        %s\n",format_name);
		fprintf(stderr,"dbg2       system_name:        %s\n",system_name);
		fprintf(stderr,"dbg2       format_description: %s\n",format_description);
		fprintf(stderr,"dbg2       numfile:            %d\n",*numfile);
		fprintf(stderr,"dbg2       filetype:           %d\n",*filetype);
		fprintf(stderr,"dbg2       variable_beams:     %d\n",*variable_beams);
		fprintf(stderr,"dbg2       traveltime:         %d\n",*traveltime);
		fprintf(stderr,"dbg2       beam_flagging:      %d\n",*beam_flagging);
		fprintf(stderr,"dbg2       nav_source:         %d\n",*nav_source);
		fprintf(stderr,"dbg2       heading_source:     %d\n",*heading_source);
		fprintf(stderr,"dbg2       vru_source:         %d\n",*vru_source);
		fprintf(stderr,"dbg2       svp_source:         %d\n",*svp_source);
		fprintf(stderr,"dbg2       beamwidth_xtrack:   %f\n",*beamwidth_xtrack);
		fprintf(stderr,"dbg2       beamwidth_ltrack:   %f\n",*beamwidth_ltrack);
		fprintf(stderr,"dbg2       error:              %d\n",*error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:         %d\n",status);
		}

	/* return status */
	return(status);
}
/*--------------------------------------------------------------------*/
int mbr_alm_mgd77tab(int verbose, void *mbio_ptr, int *error)
{
	char	*function_name = "mbr_alm_mgd77tab";
	int	status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	struct mbf_mgd77tab_struct *data;
	char	*data_ptr;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",function_name);
		fprintf(stderr,"dbg2  Revision id: %s\n",rcs_id);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       mbio_ptr:   %p\n",(void *)mbio_ptr);
		}

	/* get pointer to mbio descriptor */
	mb_io_ptr = (struct mb_io_struct *) mbio_ptr;

	/* set initial status */
	status = MB_SUCCESS;

	/* allocate memory for data structure */
	mb_io_ptr->structure_size = sizeof(struct mbf_mgd77tab_struct);
	mb_io_ptr->data_structure_size = 0;
	status = mb_malloc(verbose,mb_io_ptr->structure_size,
				&mb_io_ptr->raw_data,error);
	status = mb_malloc(verbose,sizeof(struct mbsys_singlebeam_struct),
				&mb_io_ptr->store_data,error);

	/* get pointer to mbio descriptor */
	mb_io_ptr = (struct mb_io_struct *) mbio_ptr;
	data = (struct mbf_mgd77tab_struct *) mb_io_ptr->raw_data;
	data_ptr = (char *) data;

	/* set number of header records read to zero */
	mb_io_ptr->save1 = 0;

	/* initialize everything to zeros */
	memset(data_ptr, 0, sizeof(struct mbf_mgd77tab_struct));

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",function_name);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       error:      %d\n",*error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:  %d\n",status);
		}

	/* return status */
	return(status);
}
/*--------------------------------------------------------------------*/
int mbr_dem_mgd77tab(int verbose, void *mbio_ptr, int *error)
{
	char	*function_name = "mbr_dem_mgd77tab";
	int	status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",function_name);
		fprintf(stderr,"dbg2  Revision id: %s\n",rcs_id);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       mbio_ptr:   %p\n",(void *)mbio_ptr);
		}

	/* get pointer to mbio descriptor */
	mb_io_ptr = (struct mb_io_struct *) mbio_ptr;

	/* deallocate memory for data descriptor */
	status = mb_free(verbose,&mb_io_ptr->raw_data,error);
	status = mb_free(verbose,&mb_io_ptr->store_data,error);

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",function_name);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       error:      %d\n",*error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:  %d\n",status);
		}

	/* return status */
	return(status);
}
/*--------------------------------------------------------------------*/
int mbr_rt_mgd77tab(int verbose, void *mbio_ptr, void *store_ptr, int *error)
{
	char	*function_name = "mbr_rt_mgd77tab";
	int	status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	struct mbf_mgd77tab_struct *data;
	struct mbsys_singlebeam_struct *store;
	double	minutes;
	double	seconds;
	int	i;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",function_name);
		fprintf(stderr,"dbg2  Revision id: %s\n",rcs_id);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       mbio_ptr:   %p\n",(void *)mbio_ptr);
		fprintf(stderr,"dbg2       store_ptr:  %p\n",(void *)store_ptr);
		}

	/* get pointers to mbio descriptor and data structures */
	mb_io_ptr = (struct mb_io_struct *) mbio_ptr;
	data = (struct mbf_mgd77tab_struct *) mb_io_ptr->raw_data;
	store = (struct mbsys_singlebeam_struct *) store_ptr;

	/* read next data from file */
	status = mbr_mgd77tab_rd_data(verbose,mbio_ptr,error);

	/* set error and kind in mb_io_ptr */
	mb_io_ptr->new_error = *error;
	mb_io_ptr->new_kind = data->kind;

	/* translate values to data storage structure */
	if (status == MB_SUCCESS
		&& store != NULL)
		{
		/* record kind */
		store->kind = data->kind;
		
		/* survey id */
		for (i=0;i<8;i++)
		    store->survey_id[i] = data->survey_id[i];
		
		/* get MB-System time values from the MGD77T date, time, and timezone values */
		store->time_i[0] = (int)(data->date / 10000);
		store->time_i[1] = (int)((data->date - 10000 * store->time_i[0]) / 100);
		store->time_i[2] = (int)(data->date - 10000 * store->time_i[0]
							- 100 * store->time_i[1]);
		store->time_i[3] = (int)floor(data->time / 100.0);
		minutes = data->time - 100.0 * store->time_i[3];
		store->time_i[4] = (int)floor(minutes);
		seconds = (minutes - store->time_i[4]) * 60.0;
		store->time_i[5] = (int)floor(seconds);
		store->time_i[6] = (int)((seconds - store->time_i[5]) * 1000000);
		mb_get_time(verbose, store->time_i, &store->time_d);
		store->timezone = (int)round(data->timezone);
		store->time_d += 3600.0 * store->timezone;
		mb_get_date(verbose, store->time_d, store->time_i);
		
		/* position data */
		store->longitude = data->lon;
		store->latitude = data->lat;
		store->nav_type = data->pos_type;
		store->nav_quality = data->nav_qualco;
		
		/* bathymetry data */
		if (data->corr_depth == 0.0)
			store->flag = MB_FLAG_NULL;
		else if (data->bat_qualco <= 1)
			store->flag = MB_FLAG_NONE;
		else
			store->flag = MB_FLAG_FLAG + MB_FLAG_MANUAL;
		store->tt = data->bat_ttime;
		store->bath = data->corr_depth;
		store->bath_corr = data->bat_cpco;
		store->bath_type = data->bat_typco;
		
		/* magnetic data */
		store->mag_tot_1 = data->mag_tot;
		store->mag_tot_2 = data->mag_tot2;
		store->mag_res = data->mag_res;
		store->mag_res_sensor = data->mag_ressen;
		store->mag_diurnal = data->mag_dicorr;
		store->mag_altitude = data->mag_sdepth;
		store->mag_qualco = data->mag_qualco;
		
		/* gravity data */
		store->gravity = data->gra_obs;
		store->eotvos = data->eotvos;
		store->free_air = data->freeair;
		store->gra_qualco = data->gra_qualco;
		
		store->seismic_line = data->lineid;
		store->seismic_shot = data->pointid;
		
		for (i=0;i<MB_COMMENT_MAXLINE;i++)
		    store->comment[i] = data->comment[i];
		}

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",function_name);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       error:      %d\n",*error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:  %d\n",status);
		}

	/* return status */
	return(status);
}
/*--------------------------------------------------------------------*/
int mbr_wt_mgd77tab(int verbose, void *mbio_ptr, void *store_ptr, int *error)
{
	char	*function_name = "mbr_wt_mgd77tab";
	int	status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	struct mbf_mgd77tab_struct *data;
	struct mbsys_singlebeam_struct *store;
	int	i;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",function_name);
		fprintf(stderr,"dbg2  Revision id: %s\n",rcs_id);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       mbio_ptr:   %p\n",(void *)mbio_ptr);
		fprintf(stderr,"dbg2       store_ptr:  %p\n",(void *)store_ptr);
		}

	/* get pointer to mbio descriptor */
	mb_io_ptr = (struct mb_io_struct *) mbio_ptr;

	/* get pointer to raw data structure */
	data = (struct mbf_mgd77tab_struct *) mb_io_ptr->raw_data;
	store = (struct mbsys_singlebeam_struct *) store_ptr;

	/* first translate values from data storage structure */
	if (store != NULL)
		{
		/* record kind */
		data->kind = store->kind;
		
		/* survey id */
		for (i=0;i<8;i++)
		    data->survey_id[i] = store->survey_id[i];
		
		/* get MB-System time values from the MGD77T date, time, and timezone values */
		data->date = store->time_i[0] * 10000 + store->time_i[1] * 100 + store->time_i[2];
		data->time = store->time_i[3] * 100.0 + store->time_i[4]
				+ store->time_i[5] / 3600.0
				+ store->time_i[6] / 3600.0 / 1000000.0;
		
		/* position data */
		data->lon = store->longitude;
		data->lat = store->latitude;
		data->pos_type = store->nav_type;
		data->nav_qualco = store->nav_quality;
		
		/* bathymetry data */
		if (mb_beam_check_flag_null(store->flag))
			{
			data->bat_qualco = 0;
			data->corr_depth = 0.0;
			}
		else if (mb_beam_ok(store->flag))
			data->bat_qualco = 1;
		else
			data->bat_qualco = 4;
		store->tt = data->bat_ttime;
		data->corr_depth = store->bath;
		data->bat_cpco = store->bath_corr;
		data->bat_typco = store->bath_type;
		
		/* magnetic data */
		data->mag_tot = store->mag_tot_1;
		data->mag_tot2 = store->mag_tot_2;
		data->mag_res = store->mag_res;
		data->mag_ressen = store->mag_res_sensor;
		data->mag_dicorr = store->mag_diurnal;
		data->mag_sdepth = store->mag_altitude;
		data->mag_qualco = store->mag_qualco;
		
		/* gravity data */
		data->gra_obs = store->gravity;
		data->eotvos = store->eotvos;
		data->freeair = store->free_air;
		data->gra_qualco = store->gra_qualco;
		
		data->lineid = store->seismic_line;
		data->pointid = store->seismic_shot;
		
		for (i=0;i<MB_COMMENT_MAXLINE;i++)
		    data->comment[i] = store->comment[i];
		    
		/* check for valid fields */
		data->last_field_defined = 0;
		if (strlen(data->survey_id) > 0)
			{
			data->defined_survey_id = MB_YES;
			data->last_field_defined = 0;
			}
		if (data->timezone != 0.0)
			{
			data->defined_timezone = MB_YES;
			data->last_field_defined = 1;
			}
		if (data->date != 0)
			{
			data->defined_date = MB_YES;
			data->last_field_defined = 2;
			}
		if (data->time != 0.0)
			{
			data->defined_time = MB_YES;
			data->last_field_defined = 3;
			}
		if (data->lat != 0.0)
			{
			data->defined_lat = MB_YES;
			data->last_field_defined = 4;
			}
		if (data->lon != 0.0)
			{
			data->defined_lon = MB_YES;
			data->last_field_defined = 5;
			}
		if (data->pos_type != 0)
			{
			data->defined_pos_type = MB_YES;
			data->last_field_defined = 6;
			}
		if (data->nav_qualco != 0)
			{
			data->defined_nav_qualco = MB_YES;
			data->last_field_defined = 7;
			}
		if (data->bat_ttime != 0.0)
			{
			data->defined_bat_ttime = MB_YES;
			data->last_field_defined = 8;
			}
		if (data->corr_depth != 0.0)
			{
			data->defined_corr_depth = MB_YES;
			data->last_field_defined = 9;
			}
		if (data->bat_cpco != 0)
			{
			data->defined_bat_cpco = MB_YES;
			data->last_field_defined = 10;
			}
		if (data->bat_typco != 0)
			{
			data->defined_bat_typco = MB_YES;
			data->last_field_defined = 11;
			}
		if (data->bat_qualco != 0)
			{
			data->defined_bat_qualco = MB_YES;
			data->last_field_defined = 12;
			}
		if (data->mag_tot != 0.0)
			{
			data->defined_mag_tot = MB_YES;
			data->last_field_defined = 13;
			}
		if (data->mag_tot2 != 0.0)
			{
			data->defined_mag_tot2 = MB_YES;
			data->last_field_defined = 14;
			}
		if (data->mag_res != 0.0)
			{
			data->defined_mag_res = MB_YES;
			data->last_field_defined = 15;
			}
		if (data->mag_ressen != 0)
			{
			data->defined_mag_ressen = MB_YES;
			data->last_field_defined = 16;
			}
		if (data->mag_dicorr != 0.0)
			{
			data->defined_mag_dicorr = MB_YES;
			data->last_field_defined = 17;
			}
		if (data->mag_sdepth != 0)
			{
			data->defined_mag_sdepth = MB_YES;
			data->last_field_defined = 18;
			}
		if (data->mag_qualco != 0)
			{
			data->defined_mag_qualco = MB_YES;
			data->last_field_defined = 19;
			}
		if (data->gra_obs != 0.0)
			{
			data->defined_gra_obs = MB_YES;
			data->last_field_defined = 20;
			}
		if (data->eotvos != 0.0)
			{
			data->defined_eotvos = MB_YES;
			data->last_field_defined = 21;
			}
		if (data->freeair != 0.0)
			{
			data->defined_freeair = MB_YES;
			data->last_field_defined = 22;
			}
		if (data->gra_qualco != 0)
			{
			data->defined_gra_qualco = MB_YES;
			data->last_field_defined = 23;
			}
		if (data->lineid != 0)
			{
			data->defined_lineid = MB_YES;
			data->last_field_defined = 24;
			}
		if (data->pointid != 0)
			{
			data->defined_pointid = MB_YES;
			data->last_field_defined = 25;
			}
		}

	/* write next data to file */
	status = mbr_mgd77tab_wr_data(verbose,mbio_ptr,(void *)data,error);

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",function_name);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       error:      %d\n",*error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:  %d\n",status);
		}

	/* return status */
	return(status);
}
/*--------------------------------------------------------------------*/
int mbr_mgd77tab_rd_data(int verbose, void *mbio_ptr, int *error)
{
	char	*function_name = "mbr_mgd77tab_rd_data";
	int	status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	struct mbf_mgd77tab_struct *data;
	int	*header_read;
	char	line[MB_COMMENT_MAXLINE];
	int	line_len;
	char	*read_ptr;
	int	ntabs;
	int	nfields, ifield;
	char	*fields[MBF_MGD77TAB_HEADER_FIELDS];
	int	nscan;
	int	i;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",function_name);
		fprintf(stderr,"dbg2  Revision id: %s\n",rcs_id);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       mbio_ptr:   %p\n",(void *)mbio_ptr);
		}

	/* get pointer to mbio descriptor */
	mb_io_ptr = (struct mb_io_struct *) mbio_ptr;

	/* get pointer to raw data structure */
	data = (struct mbf_mgd77tab_struct *) mb_io_ptr->raw_data;
	header_read = (int *) &mb_io_ptr->save1;

	/* set file position */
	mb_io_ptr->file_bytes = ftell(mb_io_ptr->mbfp);
	mb_io_ptr->file_pos = mb_io_ptr->file_bytes;

	/* read next record */
	if ((read_ptr = fgets(line, MB_PATH_MAXLINE, mb_io_ptr->mbfp)) != NULL)
		{
		mb_io_ptr->file_bytes += strlen(line);
		status = MB_SUCCESS;
		*error = MB_ERROR_NO_ERROR;
		}
	else
		{
		status = MB_FAILURE;
		*error = MB_ERROR_EOF;
		}
	mb_io_ptr->file_bytes = ftell(mb_io_ptr->mbfp);
	
	/* parse the record */
	if (status == MB_SUCCESS && strlen(line) > 0)
		{
		/* count the number of tabs in the line */
		line_len = strlen(line);
		ntabs = 0;
		for (i=0;i<line_len;i++)
			{
			if (line[i] == '\t')
				ntabs++;
			}
		
		/* first check for comment */
		if (line[0] == '#')
			{
			data->kind = MB_DATA_COMMENT;
			strncpy(data->comment, &line[1], line_len-3);
			data->comment[line_len-3] = '\0';
			}
		
		/* else check for header lines */
		else if (strncmp(line, "SURVEY_ID", 9) == 0 || ntabs > 26)
			{
			data->kind = MB_DATA_HEADER;
			strncpy(data->comment, line, line_len-2);
			data->comment[line_len-2] = '\0';
			}
			
		/* else parse data record */
		else if (ntabs > 0)
			{
			data->kind = MB_DATA_DATA;
			
			/* break line up into null-terminated fields
				- keep array of pointers to the start of each field */
			nfields = 0;
			fields[nfields] = &line[0];
			nfields++;
			for (i=0;i<line_len-2;i++)
				{
				if (line[i] == '\t')
					{
					line[i] = '\0';
					fields[nfields] = &line[i+1];
					nfields++;
					}
				}
			
			/* now parse field 0 of the 26 expected fields */
			data->last_field_defined = 0;
			ifield = 0;
			if (nfields > ifield && strlen(fields[ifield]) > 0)
				{
				data->defined_survey_id = MB_NO;
				nscan = sscanf(fields[ifield], "%s", data->survey_id);
				if (nscan == 1)
					{
					data->defined_survey_id = MB_YES;
					data->last_field_defined = ifield;
					}
				}

			/* now parse field 1 of the 26 expected fields */
			ifield = 1;
			if (nfields > ifield && strlen(fields[ifield]) > 0)
				{
				data->defined_timezone = MB_NO;
				nscan = sscanf(fields[ifield], "%f", &data->timezone);
				if (nscan == 1)
					{
					data->defined_timezone = MB_YES;
					data->last_field_defined = ifield;
					}
				}

			/* now parse field 2 of the 26 expected fields */
			ifield = 2;
			if (nfields > ifield && strlen(fields[ifield]) > 0)
				{
				data->defined_date = MB_NO;
				nscan = sscanf(fields[ifield], "%d", &data->date);
				if (nscan == 1)
					{
					data->defined_date = MB_YES;
					data->last_field_defined = ifield;
					}
				}

			/* now parse field 3 of the 26 expected fields */
			ifield = 3;
			if (nfields > ifield && strlen(fields[ifield]) > 0)
				{
				data->defined_time = MB_NO;
				nscan = sscanf(fields[ifield], "%f", &data->time);
				if (nscan == 1)
					{
					data->defined_time = MB_YES;
					data->last_field_defined = ifield;
					}
				}

			/* now parse field 4 of the 26 expected fields */
			ifield = 4;
			if (nfields > ifield && strlen(fields[ifield]) > 0)
				{
				data->defined_lat = MB_NO;
				nscan = sscanf(fields[ifield], "%f", &data->lat);
				if (nscan == 1)
					{
					data->defined_lat = MB_YES;
					data->last_field_defined = ifield;
					}
				}

			/* now parse field 5 of the 26 expected fields */
			ifield = 5;
			if (nfields > ifield && strlen(fields[ifield]) > 0)
				{
				data->defined_lon = MB_NO;
				nscan = sscanf(fields[ifield], "%f", &data->lon);
				if (nscan == 1)
					{
					data->defined_lon = MB_YES;
					data->last_field_defined = ifield;
					}
				}

			/* now parse field 6 of the 26 expected fields */
			ifield = 6;
			if (nfields > ifield && strlen(fields[ifield]) > 0)
				{
				data->defined_pos_type = MB_NO;
				nscan = sscanf(fields[ifield], "%d", &data->pos_type);
				if (nscan == 1)
					{
					data->defined_pos_type = MB_YES;
					data->last_field_defined = ifield;
					}
				}

			/* now parse field 7 of the 26 expected fields */
			ifield = 7;
			if (nfields > ifield && strlen(fields[ifield]) > 0)
				{
				data->defined_nav_qualco = MB_NO;
				nscan = sscanf(fields[ifield], "%d", &data->nav_qualco);
				if (nscan == 1)
					{
					data->defined_nav_qualco = MB_YES;
					data->last_field_defined = ifield;
					}
				}

			/* now parse field 8 of the 26 expected fields */
			ifield = 8;
			if (nfields > ifield && strlen(fields[ifield]) > 0)
				{
				data->defined_bat_ttime = MB_NO;
				nscan = sscanf(fields[ifield], "%f", &data->bat_ttime);
				if (nscan == 1)
					{
					data->defined_bat_ttime = MB_YES;
					data->last_field_defined = ifield;
					}
				}

			/* now parse field 9 of the 26 expected fields */
			ifield = 9;
			if (nfields > ifield && strlen(fields[ifield]) > 0)
				{
				data->defined_corr_depth = MB_NO;
				nscan = sscanf(fields[ifield], "%f", &data->corr_depth);
				if (nscan == 1)
					{
					data->defined_corr_depth = MB_YES;
					data->last_field_defined = ifield;
					}
				}

			/* now parse field 10 of the 26 expected fields */
			ifield = 10;
			if (nfields > ifield && strlen(fields[ifield]) > 0)
				{
				data->defined_bat_cpco = MB_NO;
				nscan = sscanf(fields[ifield], "%d", &data->bat_cpco);
				if (nscan == 1)
					{
					data->defined_bat_cpco = MB_YES;
					data->last_field_defined = ifield;
					}
				}

			/* now parse field 11 of the 26 expected fields */
			ifield = 11;
			if (nfields > ifield && strlen(fields[ifield]) > 0)
				{
				data->defined_bat_typco = MB_NO;
				nscan = sscanf(fields[ifield], "%d", &data->bat_typco);
				if (nscan == 1)
					{
					data->defined_bat_typco = MB_YES;
					data->last_field_defined = ifield;
					}
				}

			/* now parse field 12 of the 26 expected fields */
			ifield = 12;
			if (nfields > ifield && strlen(fields[ifield]) > 0)
				{
				data->defined_bat_qualco = MB_NO;
				nscan = sscanf(fields[ifield], "%d", &data->bat_qualco);
				if (nscan == 1)
					{
					data->defined_bat_qualco = MB_YES;
					data->last_field_defined = ifield;
					}
				}

			/* now parse field 13 of the 26 expected fields */
			ifield = 13;
			if (nfields > ifield && strlen(fields[ifield]) > 0)
				{
				data->defined_mag_tot = MB_NO;
				nscan = sscanf(fields[ifield], "%f", &data->mag_tot);
				if (nscan == 1)
					{
					data->defined_mag_tot = MB_YES;
					data->last_field_defined = ifield;
					}
				}

			/* now parse field 14 of the 26 expected fields */
			ifield = 14;
			if (nfields > ifield && strlen(fields[ifield]) > 0)
				{
				data->defined_mag_tot2 = MB_NO;
				nscan = sscanf(fields[ifield], "%f", &data->mag_tot2);
				if (nscan == 1)
					{
					data->defined_mag_tot2 = MB_YES;
					data->last_field_defined = ifield;
					}
				}

			/* now parse field 15 of the 26 expected fields */
			ifield = 15;
			if (nfields > ifield && strlen(fields[ifield]) > 0)
				{
				data->defined_mag_res = MB_NO;
				nscan = sscanf(fields[ifield], "%f", &data->mag_res);
				if (nscan == 1)
					{
					data->defined_mag_res = MB_YES;
					data->last_field_defined = ifield;
					}
				}

			/* now parse field 16 of the 26 expected fields */
			ifield = 16;
			if (nfields > ifield && strlen(fields[ifield]) > 0)
				{
				data->defined_mag_ressen = MB_NO;
				nscan = sscanf(fields[ifield], "%d", &data->mag_ressen);
				if (nscan == 1)
					{
					data->defined_mag_ressen = MB_YES;
					data->last_field_defined = ifield;
					}
				}

			/* now parse field 17 of the 26 expected fields */
			ifield = 17;
			if (nfields > ifield && strlen(fields[ifield]) > 0)
				{
				data->defined_mag_dicorr = MB_NO;
				nscan = sscanf(fields[ifield], "%f", &data->mag_dicorr);
				if (nscan == 1)
					{
					data->defined_mag_dicorr = MB_YES;
					data->last_field_defined = ifield;
					}
				}

			/* now parse field 18 of the 26 expected fields */
			ifield = 18;
			if (nfields > ifield && strlen(fields[ifield]) > 0)
				{
				data->defined_mag_sdepth = MB_NO;
				nscan = sscanf(fields[ifield], "%d", &data->mag_sdepth);
				if (nscan == 1)
					{
					data->defined_mag_sdepth = MB_YES;
					data->last_field_defined = ifield;
					}
				}

			/* now parse field 19 of the 26 expected fields */
			ifield = 19;
			if (nfields > ifield && strlen(fields[ifield]) > 0)
				{
				data->defined_mag_qualco = MB_NO;
				nscan = sscanf(fields[ifield], "%d", &data->mag_qualco);
				if (nscan == 1)
					{
					data->defined_mag_qualco = MB_YES;
					data->last_field_defined = ifield;
					}
				}

			/* now parse field 20 of the 26 expected fields */
			ifield = 20;
			if (nfields > ifield && strlen(fields[ifield]) > 0)
				{
				data->defined_gra_obs = MB_NO;
				nscan = sscanf(fields[ifield], "%f", &data->gra_obs);
				if (nscan == 1)
					{
					data->defined_gra_obs = MB_YES;
					data->last_field_defined = ifield;
					}
				}

			/* now parse field 21 of the 26 expected fields */
			ifield = 21;
			if (nfields > ifield && strlen(fields[ifield]) > 0)
				{
				data->defined_eotvos = MB_NO;
				nscan = sscanf(fields[ifield], "%f", &data->eotvos);
				if (nscan == 1)
					{
					data->defined_eotvos = MB_YES;
					data->last_field_defined = ifield;
					}
				}

			/* now parse field 22 of the 26 expected fields */
			ifield = 22;
			if (nfields > ifield && strlen(fields[ifield]) > 0)
				{
				data->defined_freeair = MB_NO;
				nscan = sscanf(fields[ifield], "%f", &data->freeair);
				if (nscan == 1)
					{
					data->defined_freeair = MB_YES;
					data->last_field_defined = ifield;
					}
				}

			/* now parse field 23 of the 26 expected fields */
			ifield = 23;
			if (nfields > ifield && strlen(fields[ifield]) > 0)
				{
				data->defined_gra_qualco = MB_NO;
				nscan = sscanf(fields[ifield], "%d", &data->gra_qualco);
				if (nscan == 1)
					{
					data->defined_gra_qualco = MB_YES;
					data->last_field_defined = ifield;
					}
				}

			/* now parse field 24 of the 26 expected fields */
			ifield = 24;
			if (nfields > ifield && strlen(fields[ifield]) > 0)
				{
				data->defined_lineid = MB_NO;
				nscan = sscanf(fields[ifield], "%d", &data->lineid);
				if (nscan == 1)
					{
					data->defined_lineid = MB_YES;
					data->last_field_defined = ifield;
					}
				}

			/* now parse field 25 of the 26 expected fields */
			ifield = 25;
			if (nfields > ifield && strlen(fields[ifield]) > 0)
				{
				data->defined_pointid = MB_NO;
				nscan = sscanf(fields[ifield], "%d", &data->pointid);
				if (nscan == 1)
					{
					data->defined_pointid = MB_YES;
					data->last_field_defined = ifield;
					}
				}
			}
		}

	/* print input debug statements */
	if (verbose >= 5)
		{
		fprintf(stderr,"\ndbg5  Data read in function <%s>\n",function_name);
		fprintf(stderr,"dbg5       data->kind:                 %d\n", data->kind);
		fprintf(stderr,"dbg5       data->defined_survey_id:    %d\n", data->defined_survey_id);
		fprintf(stderr,"dbg5       data->defined_timezone:     %d\n", data->defined_timezone);
		fprintf(stderr,"dbg5       data->defined_date:         %d\n", data->defined_date);
		fprintf(stderr,"dbg5       data->defined_time:         %d\n", data->defined_time);
		fprintf(stderr,"dbg5       data->defined_lat:          %d\n", data->defined_lat);
		fprintf(stderr,"dbg5       data->defined_lon:          %d\n", data->defined_lon);
		fprintf(stderr,"dbg5       data->defined_pos_type:     %d\n", data->defined_pos_type);
		fprintf(stderr,"dbg5       data->defined_nav_qualco:   %d\n", data->defined_nav_qualco);
		fprintf(stderr,"dbg5       data->defined_bat_ttime:    %d\n", data->defined_bat_ttime);
		fprintf(stderr,"dbg5       data->defined_corr_depth:   %d\n", data->defined_corr_depth);
		fprintf(stderr,"dbg5       data->defined_bat_cpco:     %d\n", data->defined_bat_cpco);
		fprintf(stderr,"dbg5       data->defined_bat_typco:    %d\n", data->defined_bat_typco);
		fprintf(stderr,"dbg5       data->defined_bat_qualco:   %d\n", data->defined_bat_qualco);
		fprintf(stderr,"dbg5       data->defined_mag_tot:      %d\n", data->defined_mag_tot);
		fprintf(stderr,"dbg5       data->defined_mag_tot2:     %d\n", data->defined_mag_tot2);
		fprintf(stderr,"dbg5       data->defined_mag_res:      %d\n", data->defined_mag_res);
		fprintf(stderr,"dbg5       data->defined_mag_ressen:   %d\n", data->defined_mag_ressen);
		fprintf(stderr,"dbg5       data->defined_mag_dicorr:   %d\n", data->defined_mag_dicorr);
		fprintf(stderr,"dbg5       data->defined_mag_sdepth:   %d\n", data->defined_mag_sdepth);
		fprintf(stderr,"dbg5       data->defined_mag_qualco:   %d\n", data->defined_mag_qualco);
		fprintf(stderr,"dbg5       data->defined_gra_obs:      %d\n", data->defined_gra_obs);
		fprintf(stderr,"dbg5       data->defined_eotvos:       %d\n", data->defined_eotvos);
		fprintf(stderr,"dbg5       data->defined_freeair:      %d\n", data->defined_freeair);
		fprintf(stderr,"dbg5       data->defined_gra_qualco:   %d\n", data->defined_gra_qualco);
		fprintf(stderr,"dbg5       data->defined_lineid:       %d\n", data->defined_lineid);
		fprintf(stderr,"dbg5       data->defined_pointid:      %d\n", data->defined_pointid);
		fprintf(stderr,"dbg5       data->last_field_defined:   %d\n", data->last_field_defined);
		fprintf(stderr,"dbg5       data->survey_id:            %s\n", data->survey_id);
		fprintf(stderr,"dbg5       data->timezone:             %f\n", data->timezone);
		fprintf(stderr,"dbg5       data->date:                 %d\n", data->date);
		fprintf(stderr,"dbg5       data->time:                 %f\n", data->time);
		fprintf(stderr,"dbg5       data->lat:                  %f\n", data->lat);
		fprintf(stderr,"dbg5       data->lon:                  %f\n", data->lon);
		fprintf(stderr,"dbg5       data->pos_type:             %d\n", data->pos_type);
		fprintf(stderr,"dbg5       data->nav_qualco:           %d\n", data->nav_qualco);
		fprintf(stderr,"dbg5       data->bat_ttime:            %f\n", data->bat_ttime);
		fprintf(stderr,"dbg5       data->corr_depth:           %f\n", data->corr_depth);
		fprintf(stderr,"dbg5       data->bat_cpco:             %d\n", data->bat_cpco);
		fprintf(stderr,"dbg5       data->bat_typco:            %d\n", data->bat_typco);
		fprintf(stderr,"dbg5       data->bat_qualco:           %d\n", data->bat_qualco);
		fprintf(stderr,"dbg5       data->mag_tot:              %f\n", data->mag_tot);
		fprintf(stderr,"dbg5       data->mag_tot2:             %f\n", data->mag_tot2);
		fprintf(stderr,"dbg5       data->mag_res:              %f\n", data->mag_res);
		fprintf(stderr,"dbg5       data->mag_ressen:           %d\n", data->mag_ressen);
		fprintf(stderr,"dbg5       data->mag_dicorr:           %f\n", data->mag_dicorr);
		fprintf(stderr,"dbg5       data->mag_sdepth:           %d\n", data->mag_sdepth);
		fprintf(stderr,"dbg5       data->mag_qualco:           %d\n", data->mag_qualco);
		fprintf(stderr,"dbg5       data->gra_obs:              %f\n", data->gra_obs);
		fprintf(stderr,"dbg5       data->eotvos:               %f\n", data->eotvos);
		fprintf(stderr,"dbg5       data->freeair:              %f\n", data->freeair);
		fprintf(stderr,"dbg5       data->gra_qualco:           %d\n", data->gra_qualco);
		fprintf(stderr,"dbg5       data->lineid:               %d\n", data->lineid);
		fprintf(stderr,"dbg5       data->pointid:              %d\n", data->pointid);
		fprintf(stderr,"dbg5       data->comment:              %s\n", data->comment);
		}

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",function_name);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       error:      %d\n",*error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:  %d\n",status);
		}

	/* return status */
	return(status);
}
/*--------------------------------------------------------------------*/
int mbr_mgd77tab_wr_data(int verbose, void *mbio_ptr, void *data_ptr, int *error)
{
	char	*function_name = "mbr_mgd77tab_wr_data";
	int	status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	struct mbf_mgd77tab_struct *data;
	char	line[MB_COMMENT_MAXLINE];
	int	write_status;
	int	shift;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",function_name);
		fprintf(stderr,"dbg2  Revision id: %s\n",rcs_id);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       mbio_ptr:   %p\n",(void *)mbio_ptr);
		fprintf(stderr,"dbg2       data_ptr:   %p\n",(void *)data_ptr);
		}

	/* get pointer to mbio descriptor */
	mb_io_ptr = (struct mb_io_struct *) mbio_ptr;

	/* get pointer to raw data structure */
	data = (struct mbf_mgd77tab_struct *) data_ptr;

	/* print input debug statements */
	if (verbose >= 5)
		{
		fprintf(stderr,"\ndbg5  Data to be written in function <%s>\n",function_name);
		fprintf(stderr,"dbg5       data->kind:                 %d\n", data->kind);
		fprintf(stderr,"dbg5       data->defined_survey_id:    %d\n", data->defined_survey_id);
		fprintf(stderr,"dbg5       data->defined_timezone:     %d\n", data->defined_timezone);
		fprintf(stderr,"dbg5       data->defined_date:         %d\n", data->defined_date);
		fprintf(stderr,"dbg5       data->defined_time:         %d\n", data->defined_time);
		fprintf(stderr,"dbg5       data->defined_lat:          %d\n", data->defined_lat);
		fprintf(stderr,"dbg5       data->defined_lon:          %d\n", data->defined_lon);
		fprintf(stderr,"dbg5       data->defined_pos_type:     %d\n", data->defined_pos_type);
		fprintf(stderr,"dbg5       data->defined_nav_qualco:   %d\n", data->defined_nav_qualco);
		fprintf(stderr,"dbg5       data->defined_bat_ttime:    %d\n", data->defined_bat_ttime);
		fprintf(stderr,"dbg5       data->defined_corr_depth:   %d\n", data->defined_corr_depth);
		fprintf(stderr,"dbg5       data->defined_bat_cpco:     %d\n", data->defined_bat_cpco);
		fprintf(stderr,"dbg5       data->defined_bat_typco:    %d\n", data->defined_bat_typco);
		fprintf(stderr,"dbg5       data->defined_bat_qualco:   %d\n", data->defined_bat_qualco);
		fprintf(stderr,"dbg5       data->defined_mag_tot:      %d\n", data->defined_mag_tot);
		fprintf(stderr,"dbg5       data->defined_mag_tot2:     %d\n", data->defined_mag_tot2);
		fprintf(stderr,"dbg5       data->defined_mag_res:      %d\n", data->defined_mag_res);
		fprintf(stderr,"dbg5       data->defined_mag_ressen:   %d\n", data->defined_mag_ressen);
		fprintf(stderr,"dbg5       data->defined_mag_dicorr:   %d\n", data->defined_mag_dicorr);
		fprintf(stderr,"dbg5       data->defined_mag_sdepth:   %d\n", data->defined_mag_sdepth);
		fprintf(stderr,"dbg5       data->defined_mag_qualco:   %d\n", data->defined_mag_qualco);
		fprintf(stderr,"dbg5       data->defined_gra_obs:      %d\n", data->defined_gra_obs);
		fprintf(stderr,"dbg5       data->defined_eotvos:       %d\n", data->defined_eotvos);
		fprintf(stderr,"dbg5       data->defined_freeair:      %d\n", data->defined_freeair);
		fprintf(stderr,"dbg5       data->defined_gra_qualco:   %d\n", data->defined_gra_qualco);
		fprintf(stderr,"dbg5       data->defined_lineid:       %d\n", data->defined_lineid);
		fprintf(stderr,"dbg5       data->defined_pointid:      %d\n", data->defined_pointid);
		fprintf(stderr,"dbg5       data->last_field_defined:   %d\n", data->last_field_defined);
		fprintf(stderr,"dbg5       data->survey_id:            %s\n", data->survey_id);
		fprintf(stderr,"dbg5       data->timezone:             %f\n", data->timezone);
		fprintf(stderr,"dbg5       data->date:                 %d\n", data->date);
		fprintf(stderr,"dbg5       data->time:                 %f\n", data->time);
		fprintf(stderr,"dbg5       data->lat:                  %f\n", data->lat);
		fprintf(stderr,"dbg5       data->lon:                  %f\n", data->lon);
		fprintf(stderr,"dbg5       data->pos_type:             %d\n", data->pos_type);
		fprintf(stderr,"dbg5       data->nav_qualco:           %d\n", data->nav_qualco);
		fprintf(stderr,"dbg5       data->bat_ttime:            %f\n", data->bat_ttime);
		fprintf(stderr,"dbg5       data->corr_depth:           %f\n", data->corr_depth);
		fprintf(stderr,"dbg5       data->bat_cpco:             %d\n", data->bat_cpco);
		fprintf(stderr,"dbg5       data->bat_typco:            %d\n", data->bat_typco);
		fprintf(stderr,"dbg5       data->bat_qualco:           %d\n", data->bat_qualco);
		fprintf(stderr,"dbg5       data->mag_tot:              %f\n", data->mag_tot);
		fprintf(stderr,"dbg5       data->mag_tot2:             %f\n", data->mag_tot2);
		fprintf(stderr,"dbg5       data->mag_res:              %f\n", data->mag_res);
		fprintf(stderr,"dbg5       data->mag_ressen:           %d\n", data->mag_ressen);
		fprintf(stderr,"dbg5       data->mag_dicorr:           %f\n", data->mag_dicorr);
		fprintf(stderr,"dbg5       data->mag_sdepth:           %d\n", data->mag_sdepth);
		fprintf(stderr,"dbg5       data->mag_qualco:           %d\n", data->mag_qualco);
		fprintf(stderr,"dbg5       data->gra_obs:              %f\n", data->gra_obs);
		fprintf(stderr,"dbg5       data->eotvos:               %f\n", data->eotvos);
		fprintf(stderr,"dbg5       data->freeair:              %f\n", data->freeair);
		fprintf(stderr,"dbg5       data->gra_qualco:           %d\n", data->gra_qualco);
		fprintf(stderr,"dbg5       data->lineid:               %d\n", data->lineid);
		fprintf(stderr,"dbg5       data->pointid:              %d\n", data->pointid);
		fprintf(stderr,"dbg5       data->comment:              %s\n", data->comment);
		}

	/* handle the data */
	if (data->kind == MB_DATA_HEADER)
		{
		sprintf(line,"%s\r\n",data->comment);
		}
	else if (data->kind == MB_DATA_COMMENT)
		{
		 sprintf(line, "#%s\r\n", data->comment);
		}
	else if (data->kind == MB_DATA_DATA)
		{
		/* figure out which is the last valid field - no tabs to be
		 * written past that field */
		
		
		/* write out each field */
		shift = 0;
		if (data->defined_survey_id == MB_YES)
			{
			sprintf(&line[shift], "%s", data->survey_id); shift = strlen(line);
			}
		if (data->defined_timezone == MB_YES)
			{
			sprintf(&line[shift], "\t%f", data->timezone); shift = strlen(line);
			}
		else if (data->last_field_defined > 1)
			{
			sprintf(&line[shift], "\t"); shift = strlen(line);
			}
		if (data->defined_date == MB_YES)
			{
			sprintf(&line[shift], "\t%d", data->date); shift = strlen(line);
			}
		else if (data->last_field_defined > 2)
			{
			sprintf(&line[shift], "\t"); shift = strlen(line);
			}
		if (data->defined_time == MB_YES)
			{
			sprintf(&line[shift], "\t%f", data->time); shift = strlen(line);
			}
		else if (data->last_field_defined > 3)
			{
			sprintf(&line[shift], "\t"); shift = strlen(line);
			}
		if (data->defined_lat == MB_YES)
			{
			sprintf(&line[shift], "\t%f", data->lat); shift = strlen(line);
			}
		else if (data->last_field_defined > 4)
			{
			sprintf(&line[shift], "\t"); shift = strlen(line);
			}
		if (data->defined_lon == MB_YES)
			{
			sprintf(&line[shift], "\t%f", data->lon); shift = strlen(line);
			}
		else if (data->last_field_defined > 5)
			{
			sprintf(&line[shift], "\t"); shift = strlen(line);
			}
		if (data->defined_pos_type == MB_YES)
			{
			sprintf(&line[shift], "\t%d", data->pos_type); shift = strlen(line);
			}
		else if (data->last_field_defined > 6)
			{
			sprintf(&line[shift], "\t"); shift = strlen(line);
			}
		if (data->defined_nav_qualco == MB_YES)
			{
			sprintf(&line[shift], "\t%d", data->nav_qualco); shift = strlen(line);
			}
		else if (data->last_field_defined > 7)
			{
			sprintf(&line[shift], "\t"); shift = strlen(line);
			}
		if (data->defined_bat_ttime == MB_YES)
			{
			sprintf(&line[shift], "\t%f", data->bat_ttime); shift = strlen(line);
			}
		else if (data->last_field_defined > 8)
			{
			sprintf(&line[shift], "\t"); shift = strlen(line);
			}
		if (data->defined_corr_depth == MB_YES)
			{
			sprintf(&line[shift], "\t%f", data->corr_depth); shift = strlen(line);
			}
		else if (data->last_field_defined > 9)
			{
			sprintf(&line[shift], "\t"); shift = strlen(line);
			}
		if (data->defined_bat_cpco == MB_YES)
			{
			sprintf(&line[shift], "\t%d", data->bat_cpco); shift = strlen(line);
			}
		else if (data->last_field_defined > 10)
			{
			sprintf(&line[shift], "\t"); shift = strlen(line);
			}
		if (data->defined_bat_typco == MB_YES)
			{
			sprintf(&line[shift], "\t%d", data->bat_typco); shift = strlen(line);
			}
		else if (data->last_field_defined > 11)
			{
			sprintf(&line[shift], "\t"); shift = strlen(line);
			}
		if (data->defined_bat_qualco == MB_YES)
			{
			sprintf(&line[shift], "\t%d", data->bat_qualco); shift = strlen(line);
			}
		else if (data->last_field_defined > 12)
			{
			sprintf(&line[shift], "\t"); shift = strlen(line);
			}
		if (data->defined_mag_tot == MB_YES)
			{
			sprintf(&line[shift], "\t%f", data->mag_tot); shift = strlen(line);
			}
		else if (data->last_field_defined > 13)
			{
			sprintf(&line[shift], "\t"); shift = strlen(line);
			}
		if (data->defined_mag_tot2 == MB_YES)
			{
			sprintf(&line[shift], "\t%f", data->mag_tot2); shift = strlen(line);
			}
		else if (data->last_field_defined > 14)
			{
			sprintf(&line[shift], "\t"); shift = strlen(line);
			}
		if (data->defined_mag_res == MB_YES)
			{
			sprintf(&line[shift], "\t%f", data->mag_res); shift = strlen(line);
			}
		else if (data->last_field_defined > 15)
			{
			sprintf(&line[shift], "\t"); shift = strlen(line);
			}
		if (data->defined_mag_ressen == MB_YES)
			{
			sprintf(&line[shift], "\t%d", data->mag_ressen); shift = strlen(line);
			}
		else if (data->last_field_defined > 16)
			{
			sprintf(&line[shift], "\t"); shift = strlen(line);
			}
		if (data->defined_mag_dicorr == MB_YES)
			{
			sprintf(&line[shift], "\t%f", data->mag_dicorr); shift = strlen(line);
			}
		else if (data->last_field_defined > 17)
			{
			sprintf(&line[shift], "\t"); shift = strlen(line);
			}
		if (data->defined_mag_sdepth == MB_YES)
			{
			sprintf(&line[shift], "\t%d", data->mag_sdepth); shift = strlen(line);
			}
		else if (data->last_field_defined > 18)
			{
			sprintf(&line[shift], "\t"); shift = strlen(line);
			}
		if (data->defined_mag_qualco == MB_YES)
			{
			sprintf(&line[shift], "\t%d", data->mag_qualco); shift = strlen(line);
			}
		else if (data->last_field_defined > 19)
			{
			sprintf(&line[shift], "\t"); shift = strlen(line);
			}
		if (data->defined_gra_obs == MB_YES)
			{
			sprintf(&line[shift], "\t%f", data->gra_obs); shift = strlen(line);
			}
		else if (data->last_field_defined > 20)
			{
			sprintf(&line[shift], "\t"); shift = strlen(line);
			}
		if (data->defined_eotvos == MB_YES)
			{
			sprintf(&line[shift], "\t%f", data->eotvos); shift = strlen(line);
			}
		else if (data->last_field_defined > 21)
			{
			sprintf(&line[shift], "\t"); shift = strlen(line);
			}
		if (data->defined_freeair == MB_YES)
			{
			sprintf(&line[shift], "\t%f", data->freeair); shift = strlen(line);
			}
		else if (data->last_field_defined > 22)
			{
			sprintf(&line[shift], "\t"); shift = strlen(line);
			}
		if (data->defined_gra_qualco == MB_YES)
			{
			sprintf(&line[shift], "\t%d", data->gra_qualco); shift = strlen(line);
			}
		else if (data->last_field_defined > 23)
			{
			sprintf(&line[shift], "\t"); shift = strlen(line);
			}
		if (data->defined_lineid == MB_YES)
			{
			sprintf(&line[shift], "\t%d", data->lineid); shift = strlen(line);
			}
		else if (data->last_field_defined > 24)
			{
			sprintf(&line[shift], "\t"); shift = strlen(line);
			}
		if (data->defined_pointid == MB_YES)
			{
			sprintf(&line[shift], "\t%d", data->pointid); shift = strlen(line);
			}
		line[shift] = '\r'; shift++;
		line[shift] = '\n'; shift++;
		line[shift] = '\0'; shift++;
		}

	if ((write_status = fputs(line, mb_io_ptr->mbfp)) > 0)
		{
		*error = MB_ERROR_NO_ERROR;
		status = MB_SUCCESS;
		}
	else
		{
		*error = MB_ERROR_WRITE_FAIL;
		status = MB_FAILURE;
		}


	/* print output debug statements */
	if (verbose >= 5)
		{
		fprintf(stderr,"\ndbg5  Data record kind in MBIO function <%s>\n",function_name);
		fprintf(stderr,"dbg5       kind:       %d\n",data->kind);
		}

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",function_name);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       error:      %d\n",*error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:  %d\n",status);
		}

	/* return status */
	return(status);
}
/*--------------------------------------------------------------------*/
