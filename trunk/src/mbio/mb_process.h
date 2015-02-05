/*--------------------------------------------------------------------
 *    The MB-system:	mb_process.h	9/11/00
 *    $Id$
 *
 *    Copyright (c) 2000-2015 by
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
 * MBprocess is a tool for processing swath sonar bathymetry data.
 * This program performs a number of functions, including:
 *   - merging navigation
 *   - recalculating bathymetry from travel time and angle data
 *     by raytracing through a layered water sound velocity model.
 *   - applying changes to ship draft, roll bias and pitch bias
 *   - applying bathymetry edits from edit save files.
 * This file defines processing parameters and structures
 * saved in mbprocess parameter files. The reading, writing, and
 * updating of the mbprocess control parameter values are done
 * using functions in mb_process.c
 *
 * The structures holding beam edit event values associated with
 * edit save files are given here.
 *
 * This file also defines the structure and parameters associated
 * with defining the position and angular offsets between the
 * various sensors on a survey platform (e.g. ship, launch, ROV, AUV).
 *
 * The parameters controlling mbprocess are included in an ascii
 * parameter file. The parameter files consist of single line
 * commands and comment lines beginning with '#'. The commands
 * are given by a keyword and a corresponding value (separated by
 * spaces or tabs). A list of the keywords and values follows,
 * with default values in square brackets:
 *
 * GENERAL PARAMETERS:
 *   EXPLICIT			    # causes mbprocess to set modes implicitely
 *                                  # - e.g. the SVPFILE command will also set
 *                                  #   raytracing on even if the RAYTRACE command
 *                                  #   is not given [explicit mode commands required]
 *   FORMAT constant                # sets format id [no default]
 *   INFILE filename                # sets input file path [no default]
 *   OUTFILE filename               # sets output file path [no default]
 *
 * NAVIGATION MERGING:
 *   NAVMODE boolean                # sets navigation merging [0]
 *                                  #   0: navigation merge off
 *                                  #   1: navigation merge on
 *   NAVFILE filename               # sets navigation file path [no default]
 *   NAVFORMAT constant             # sets navigation file format [9]
 *   NAVHEADING boolean             # sets heading to be merged from navigation file
 *                                  # - note: heading merged from navigation before
 *                                  #   heading correction applied
 *                                  #   0: heading not changed
 *                                  #   1: heading merged from navigation file
 *   NAVSPEED boolean               # sets speed to be merged from navigation file
 *                                  #   0: speed not changed
 *                                  #   1: speed merged from navigation file
 *   NAVDRAFT boolean               # sets draft to be merged from navigation file
 *                                  # - note: draft merged from navigation before
 *                                  #   draft correction applied
 *                                  #   0: draft not changed
 *                                  #   1: draft merged from navigation file
 *   NAVATTITUDE boolean            # sets roll, pitch and heave to be merged from navigation file
 *                                  # - note: roll, pitch, and heave merged from navigation before
 *                                  #   roll bias and pitch bias corrections applied
 *                                  #   0: roll, pitch, and heave not changed
 *                                  #   1: roll, pitch, and heave merged from navigation file
 *   NAVINTERP boolean              # sets navigation interpolation algorithm [0]
 *                                  #   0: linear interpolation (recommended)
 *                                  #   1: spline interpolation
 *   NAVTIMESHIFT constant          # sets navigation time shift (seconds) [0.0]
 *                                  # - note: time shift added to timestamps of
 *                                  #   navigation fixes read in from NAVFILE
 *                                  #   prior to merging
 *
 * NAVIGATION OFFSETS AND SHIFTS:
 *   NAVSHIFT boolean               # sets navigation offset [0]
 *                                  # - note: offsets and shifts are applied to navigation
 *                                  #   values from both survey and navigation records, and
 *                                  #   are applied to navigation read in from
 *                                  #   NAVFILE prior to merging
 *                                  # - note: offsets and shifts are NOT applied to adjusted
 *                                  #   navigation values from NAVADJFILE
 *   NAVOFFSETX constant            # sets navigation athwartship offset (meters) [0.0]
 *                                  # - note: the effective navigation shift is
 *                                  #   (NAVOFFSETX - SONAROFFSETX), and the
 *                                  #   navigation is corrected by subtracting
 *                                  #   this effective shift.
 *                                  # - note: athwartship shift is positive to
 *                                  #   starboard.
 *   NAVOFFSETY constant            # sets navigation fore-aft offset (meters) [0.0]
 *                                  # - note: the effective navigation shift is
 *                                  #   (NAVOFFSETY - SONAROFFSETY), and the
 *                                  #   navigation is corrected by subtracting
 *                                  #   this effective shift.
 *                                  # - note: fore-aft shift is positive forward.
 *   NAVOFFSETZ constant            # sets navigation vertical offset (meters) [0.0]
 *                                  # - note: this value is not yet used for
 *                                  #   anything.
 *                                  # - note: vertical shift is positive down.
 *   NAVSHIFTLON constant           # sets navigation longitude shift (degrees) [0.0]
 *   NAVSHIFTLAT constant           # sets navigation latitude shift (degrees) [0.0]
 *
 * ADJUSTED NAVIGATION MERGING:
 *   NAVADJMODE boolean             # sets navigation merging from mbnavadjust [0]
 *                                  # - longitude and latitude only
 *                                  #   0: adjusted navigation merge off
 *                                  #   1: adjusted navigation merge on
 *   NAVADJFILE filename            # sets adjusted navigation file path
 *                                  # - this file supercedes navigation file for
 *                                  #   lon and lat only
 *                                  # - uses mbnavadjust output
 *   NAVADJINTERP boolean           # sets adjusted navigation interpolation algorithm [0]
 *                                  #   0: linear interpolation (recommended)
 *                                  #   1: spline interpolation
 *
 * ATTITUDE MERGING:
 *   ATTITUDEMODE mode              # sets atttitude (roll, pitch, and heave) merging [0]
 *                                  # - note: roll, pitch, and heave merged before
 *                                  #   roll bias and pitch bias corrections applied
 *                                  # - note: attitude merging from a separate file
 *                                  #   supersedes attitude merging from a navigation file
 *                                  #   0: attitude merging off
 *                                  #   1: attitude merging on
 *   ATTITUDEFILE filename              # sets attitude file path
 *   ATTITUDEFORMAT constant            # sets attitude file format [1]
 *                                  # - attitude files can be in one of four ASCII
 *                                  #   table formats
 *                                  #   1: format is <time_d roll pitch heave>
 *                                  #   2: format is <yr mon day hour min sec roll pitch heave>
 *                                  #   3: format is <yr jday hour min sec roll pitch heave>
 *                                  #   4: format is <yr jday daymin sec roll pitch heave>
 *                                  # - time_d = decimal seconds since 1/1/1970
 *                                  # - daymin = decimal minutes start of day
 *                                  # - roll = positive starboard up, degrees
 *                                  # - pitch = positive forward up, degrees
 *                                  # - heave = positive up, meters
 *
 * SONARDEPTH MERGING:
 *   SONARDEPTHMODE mode            # sets sonardepth merging [0]
 *                                  # - note: sonardepth merged before
 *                                  #   draft corrections applied
 *                                  # - note: sonardepth merging from a separate file
 *                                  #   supersedes draft merging from a navigation file
 *                                  #   0: sonardepth merging off
 *                                  #   1: sonardepth merging on
 *   SONARDEPTHFILE filename        # sets sonardepth file path
 *   SONARDEPTHFORMAT constant      # sets sonardepth file format [1]
 *                                  # - sonardepth files can be in one of four ASCII
 *                                  #   table formats
 *                                  #   1: format is <time_d sonardepth>
 *                                  #   2: format is <yr mon day hour min sec sonardepth>
 *                                  #   3: format is <yr jday hour min sec sonardepth>
 *                                  #   4: format is <yr jday daymin sec sonardepth>
 *                                  # - time_d = decimal seconds since 1/1/1970
 *                                  # - daymin = decimal minutes start of day
 *                                  # - sonardepth = depth of sonar, positive down, meters
 *
 * DATA CUTTING:
 *   DATACUTCLEAR                   # clears all data cutting commands
 *   DATACUT kind mode min max      # adds data cutting command that flags
 *                                  # specified bathymetery and amplitude
 *                                  # beams and zeroes specified sidescan
 *                                  # pixels. Note that cutting bathymetry
 *                                  # means flagging beams, which affects
 *                                  # both bathymetry and amplitude data.
 *                                  # In contrast, cutting amplitude data
 *                                  # means zeroing the values without
 *                                  # affecting the bathymetry, and
 *                                  # cutting sidescan also means zeroing
 *                                  # the values.
 *                                  # - kind:
 *                                  #   0: bathymetry
 *                                  #   1: amplitude
 *                                  #   2: sidescan
 *                                  # - mode:
 *                                  #   0: no cut
 *                                  #   1: cut by beam/pixel number
 *                                  #   2: cut by distance
 *                                  #   3: cut by speed
 *                                  # - min: minimum beam/pixel number
 *                                  #   or acrosstrack distance for data
 *                                  #   data cutting zone.
 *                                  # - max: maximum beam/pixel number
 *                                  #   or acrosstrack distance for data
 *                                  #   data cutting zone.
 *   BATHCUTNUMBER min max          # adds data cutting command to cut
 *                                  #   bathymetry by beam number from
 *                                  #   beam min to beam max.
 *   BATHCUTDISTANCE min max        # adds data cutting command to cut
 *                                  #   bathymetry by beam number from
 *                                  #   acrosstrack distance  min to
 *                                  #   distance max.
 *   BATHCUTSPEED min max           # adds data cutting command to cut
 *                                  #   all bathymetry in pings with
 *                                  #   speed less than min or more
 *                                  #   than max
 *   AMPCUTNUMBER min max           # adds data cutting command to cut
 *                                  #   amplitude by beam number from
 *                                  #   beam min to beam max.
 *   AMPCUTDISTANCE min max         # adds data cutting command to cut
 *                                  #   amplitude by beam number from
 *                                  #   acrosstrack distance  min to
 *                                  #   distance max.
 *   AMPCUTSPEED min max            # adds data cutting command to cut
 *                                  #   all amplitude in pings with
 *                                  #   speed less than min or more
 *                                  #   than max
 *   SSCUTNUMBER min max            # adds data cutting command to cut
 *                                  #   sidescan by beam number from
 *                                  #   beam min to beam max.
 *   SSCUTDISTANCE min max          # adds data cutting command to cut
 *                                  #   sidescan by beam number from
 *                                  #   acrosstrack distance  min to
 *                                  #   distance max.
 *   SSCUTSPEED min max             # adds data cutting command to cut
 *                                  #   all sidescan in pings with
 *                                  #   speed less than min or more
 *                                  #   than max
 *
 * BATHYMETRY EDITING:
 *   EDITSAVEMODE boolean           # turns on reading edit save file (from mbedit) [0]
 *   EDITSAVEFILE filename          # sets edit save file path (from mbedit) [none]
 *
 * BATHYMETRY RECALCULATION:
 *   SVPMODE mode                   # sets bathymetry recalculation by raytracing [0]
 *                                  #  0: raytracing off
 *                                  #  1: raytracing on
 *                                  #  2: translate to/from corrected bathymetry according to CORRECTED
 *   RAYTRACE boolean               # sets bathymetry recalculation by raytracing (obsolete) [0]
 *                                  #  0: raytracing off
 *                                  #  1: raytracing on
 *   SVPFILE filename               # sets svp file path [no default]
 *   SSVMODE mode                   # sets ssv mode [0]
 *                                  #  0: use ssv from file
 *                                  #  1: offset ssv from file (set by SSV)
 *                                  #  2: use constant ssv (set by SSV)
 *   SSV constant/offset            # sets ssv value or offset (m/s) [1500.0]
 *   TTMODE mode                    # sets handling of travel times [0]
 *                                  #  0: do not change travel times.
 *                                  #  1: travel time correction by multiply
 *   TTMULTIPLY multiplier          # sets value multiplied by travel times [1.0]
 *   ANGLEMODE mode                 # sets handling of beam angles [0]
 *                                  #  0: do not change beam angles
 *                                  #  1: adjust beams angles by Snell's law
 *                                  #     ignoring sonar array geometry
 *                                  #  2: adjust beams angles by Snell's law
 *                                  #     using array geometry
 *   SOUNDSPEEDREF boolean          # sets raytraced bathymetry to "corrected" values [1]
 *
 * STATIC BEAM BATHYMETRY OFFSETS:
 *   STATICMODE mode                # sets offsetting of bathymetry by per-beam statics [0]
 *                                  #  0: static correction off
 *                                  #  1: static correction on
 *   STATICFILE filename            # sets static per-beam file path [no default]
 *                                  #   - static files are two-column ascii tables
 *                                  #     with the beam # in the first column and
 *                                  #     the depth offset in m in the second column
 *
 * DRAFT CORRECTION:
 *   DRAFTMODE mode                 # sets draft correction [0]
 *                                  # - note: draft merged from navigation before
 *                                  #   draft correction applied
 *                                  #   0: no draft correction
 *                                  #   1: draft correction by offset
 *                                  #   2: draft correction by multiply
 *                                  #   3: draft correction by offset and multiply
 *                                  #   4: draft set to constant
 *   DRAFT constant                 # sets draft value (m) [0.0]
 *   DRAFTOFFSET offset             # sets value added to draft (m) [0.0]
 *   DRAFTMULTIPLY multiplier       # sets value multiplied by draft [1.0]
 *
 * HEAVE CORRECTION:
 *   HEAVEMODE mode                 # sets heave correction [0]
 *                                  #   0: no heave correction
 *                                  #   1: heave correction by offset
 *                                  #   2: heave correction by multiply
 *                                  #   3: heave correction by offset and multiply
 *   HEAVEOFFSET offset             # sets value added to heave (m)
 *   HEAVEMULTIPLY multiplier       # sets value multiplied by heave
 *
 * LEVER CORRECTION:
 *   LEVERMODE mode                 # sets heave correction by lever correction [0]
 *                                  #   0: no heave correction by lever calculation
 *                                  #   1: heave correction by lever calculation
 *				    # - note: lever calculation finds heave at the
 *                                  #   sonar head location by adding motion
 *                                  #   inferred from roll and pitch projected
 *                                  #   from the vru location.
 *   VRUOFFSETX constant            # sets vru athwartship offset (meters) [0.0]
 *                                  # - note: the effective athwartship distance
 *                                  #   between the vru and the sonar head is
 *                                  #   (VRUOFFSETX - SONAROFFSETX), and the
 *                                  #   lever calculation is made using this
 *                                  #   effective distance.
 *                                  # - note: athwartship distance is positive to
 *                                  #   starboard.
 *   VRUOFFSETY constant            # sets vru fore-aft offset (meters) [0.0]
 *                                  # - note: the effective fore-aft distance
 *                                  #   between the vru and the sonar head is
 *                                  #   (VRUOFFSETY - SONAROFFSETY), and the
 *                                  #   lever calculation is made using this
 *                                  #   effective distance.
 *                                  # - note: fore-aft distance is positive forward.
 *   VRUOFFSETZ constant            # sets vru vertical offset (meters) [0.0]
 *                                  # - note: the effective vertical distance
 *                                  #   between the vru and the sonar head is
 *                                  #   (VRUOFFSETZ - SONAROFFSETZ), and the
 *                                  #   lever calculation is made using this
 *                                  #   effective distance.
 *                                  # - note: vertical distance is positive down.
 *   SONAROFFSETX constant          # sets sonar athwartship offset (meters) [0.0]
 *                                  # - note: this value is used for both
 *                                  #   navigation shifts and lever calculations.
 *                                  # - note: athwartship distance is positive to
 *                                  #   starboard.
 *   SONAROFFSETY constant          # sets vru fore-aft offset (meters) [0.0]
 *                                  # - note: this value is used for both
 *                                  #   navigation shifts and lever calculations.
 *                                  # - note: fore-aft distance is positive forward.
 *   SONAROFFSETZ constant          # sets vru vertical offset (meters) [0.0]
 *                                  # - note: this value is used for lever
 *                                  #   calculations.
 *                                  # - note: vertical distance is positive down.
 *
 * ROLL CORRECTION:
 *   ROLLBIASMODE mode              # sets roll correction [0]
 *                                  #   0: no roll correction
 *                                  #   1: roll correction by single roll bias
 *                                  #   2: roll correction by separate port and starboard roll bias
 *   ROLLBIAS offset                # sets roll bias (degrees)
 *   ROLLBIASPORT offset            # sets port roll bias (degrees)
 *   ROLLBIASSTBD offset            # sets starboard roll bias (degrees)
 *
 * PITCH CORRECTION:
 *   PITCHBIASMODE mode             # sets pitch correction [0]
 *                                  #   0: no pitch correction
 *                                  #   1: pitch correction by pitch bias
 *   PITCHBIAS offset               # sets pitch bias (degrees)
 *
 * HEADING CORRECTION:
 *   HEADINGMODE mode               # sets heading correction [no heading correction]
 *                                  # - note: heading merged from navigation before
 *                                  #   heading correction applied
 *                                  #   0: no heading correction
 *                                  #   1: heading correction using course made good
 *                                  #   2: heading correction by offset
 *                                  #   3: heading correction using course made good and offset
 *   HEADINGOFFSET offset           # sets value added to heading (degrees)
 *
 * TIDE CORRECTION:
 *   TIDEMODE mode                  # sets tide correction [0]
 *                                  # - note: tide added to bathymetry after
 *                                  #   all other calculations and corrections
 *                                  #   0: tide correction off
 *                                  #   1: tide correction on
 *   TIDEFILE filename              # sets tide file path
 *   TIDEFORMAT constant            # sets tide file format [1]
 *                                  # - tide files can be in one of four ASCII
 *                                  #   table formats
 *                                  #   1: format is <time_d tide>
 *                                  #   2: format is <yr mon day hour min sec tide>
 *                                  #   3: format is <yr jday hour min sec tide>
 *                                  #   4: format is <yr jday daymin sec tide>
 *                                  # - time_d = decimal seconds since 1/1/1970
 *                                  # - daymin = decimal minutes start of day
 *
 * AMPLITUDE CORRECTION:
 *   AMPCORRMODE  boolean           # sets correction of amplitude by amplitude vs grazing
 *                                  # angle function
 *                                  #   0: amplitude correction off
 *                                  #   1: amplitude correction on
 *   AMPCORRFILE filename           # sets amplitude correction file path [no default]
 *   AMPCORRTYPE mode               # sets amplitude correction mode [0]
 *                                  #   0: correction is by subtraction
 *                                  #   1: correction is by division
 *   AMPCORRSYMMETRY boolean        # sets amplitude vs grazing angle table symmetry mode [0]
 *                                  #   0: amplitude vs grazing table left asymmetric
 *                                  #   1: amplitude vs grazing table forced to be symmetric
 *   AMPCORRANGLE constant          # sets characteristic angle for amplitude correction (degrees) [30.0]
 *   AMPCORRSLOPE boolean           # sets amplitude correction slope mode [0]
 *                                  #   0: local slope ignored in calculating correction
 *                                  #   1: local slope used in calculating correction
 *                                  #   2: topography grid used in calculating correction
 *                                  #      but slope ignored
 *                                  #   3: local slope from topography grid used in
 *                                  #      calculating correction
 *
 * SIDESCAN CORRECTION:
 *   SSCORRMODE  boolean            # sets correction of sidescan by amplitude vs grazing
 *                                  # angle function
 *                                  #   0: sidescan correction off
 *                                  #   1: sidescan correction on
 *   SSCORRFILE filename            # sets sidescan correction file path [no default]
 *   SSCORRTYPE mode                # sets sidescan correction mode [0]
 *                                  #   0: correction is by subtraction
 *                                  #   1: correction is by division
 *   SSCORRSYMMETRY boolean         # sets amplitude vs grazing angle table symmetry mode [0]
 *                                  #   0: amplitude vs grazing table left asymmetric
 *                                  #   1: amplitude vs grazing table forced to be symmetric
 *   SSCORRANGLE constant           # sets characteristic angle for sidescan correction (degrees) [30.0]
 *   SSCORRSLOPE boolean            # sets sidescan correction slope mode [0]
 *                                  #   0: local slope ignored in calculating correction
 *                                  #   1: local slope used in calculating correction
 *                                  #   2: topography grid used in calculating correction
 *                                  #      but slope ignored
 *                                  #   3: local slope from topography grid used in
 *                                  #      calculating correction
 *
 * AMPLITUDE/SIDESCAN TOPOGRAPHY CORRECTION:
 *    AMPSSCORRTOPOFILE filename    # sets amplitude/sidescan correction topography grid file path
 *
 * SIDESCAN RECALCULATION:
 *   SSRECALCMODE  boolean          # sets recalculation of sidescan for Simrad multibeam data
 *                                  #   0: sidescan recalculation off
 *                                  #   1: sidescan recalculation on
 *   SSPIXELSIZE constant           # sets recalculated sidescan pixel size (m) [0.0]
 *                                  # - a zero value causes the pixel size to be recalculated
 *                                  #   for every data record
 *   SSSWATHWIDTH  constant         # sets sidescan swath width (degrees) [0.0]
 *                                  # - a zero value causes the swath width to be recalculated
 *                                  #   for every data record
 *   SSINTERPOLATE  constant        # sets sidescan interpolation distance (number of pixels)
 *
 * METADATA INSERTION:
 *   METAVESSEL string              # sets mbinfo metadata string for vessel
 *   METAINSTITUTION string         # sets mbinfo metadata string for vessel operator institution or company
 *   METAPLATFORM string            # sets mbinfo metadata string for sonar platform (ship or vehicle)
 *   METASONAR string               # sets mbinfo metadata string for sonar model name
 *   METASONARVERSION string        # sets mbinfo metadata string for sonar version (usually software version)
 *   METACRUISEID string            # sets mbinfo metadata string for institutional cruise id
 *   METACRUISENAME string          # sets mbinfo metadata string for descriptive cruise name
 *   METAPI string                  # sets mbinfo metadata string for principal investigator
 *   METAPIINSTITUTION string       # sets mbinfo metadata string for principal investigator
 *   METACLIENT string              # sets mbinfo metadata string for data owner (usually PI institution)
 *   METASVCORRECTED boolean        # sets mbinfo metadata boolean for sound velocity corrected depths
 *   METATIDECORRECTED boolean      # sets mbinfo metadata boolean for tide corrected bathymetry
 *   METABATHEDITMANUAL boolean     # sets mbinfo metadata boolean for manually edited bathymetry
 *   METABATHEDITAUTO boolean       # sets mbinfo metadata boolean for automatically edited bathymetry
 *   METAROLLBIAS constant          # sets mbinfo metadata constant for roll bias (degrees + to starboard)
 *   METAPITCHBIAS constant         # sets mbinfo metadata constant for pitch bias (degrees + forward)
 *   METAHEADINGBIAS constant       # sets mbinfo metadata constant for heading bias (degrees)
 *   METADRAFT constant             # sets mbinfo metadata constant for vessel draft (m)
 *
 * PROCESSING KLUGES:
 *   KLUGE001                       # processing kluge 001
 *                                  #   - Apply corrections to travel time data in Hydrosweep DS2
 *                                  #     data collected on the R/V Maurice Ewing in 2001/2002
 *   KLUGE002                       # processing kluge 002
 *                                  #   - Apply corrections to travel time data in Hydrosweep DS2
 *                                  #     data collected on the R/V Maurice Ewing in 2001/2002
 *                                  #     enables correction of draft values in Simrad data
 *                                  #   - some Simrad multibeam data has had an
 *                                  #     error in which the heave has bee added
 *                                  #     to the sonar depth (draft for hull
 *                                  #     mounted sonars)
 *                                  #   - this correction subtracts the heave
 *                                  #     value from the sonar depth
 *   KLUGE003                       # processing kluge 003
 *				    #   enables correction of beam angles in SeaBeam 2112 data
 *				    #   - a data sample from the SeaBeam 2112 on
 *				    #     the USCG Icebreaker Healy (collected on
 *				    #     23 July 2003) was found to have an error
 *				    #     in which the beam angles had 0.25 times
 *				    #     the roll added
 *				    #   - this correction subtracts 0.25 * roll
 *				    #     from the beam angles before the bathymetry
 *				    #     is recalculated by raytracing through a
 *				    #     water sound velocity profile
 *				    #   - the mbprocess parameter files must be
 *				    #     set to enable bathymetry recalculation
 *				    #     by raytracing in order to apply this
 *				    #     correction
 *   KLUGE004                       # processing kluge 004
 *				    #   deletes survey data associated with duplicate
 *				    #   or reversed time tags
 *				    #   - if survey data records are encountered
 *				    #     with time tags less than or equal to the
 *				    #     last good time tag, an error is set and
 *				    #     the data record is not output to the
 *				    #     processed data file.
 *   KLUGE005                       # processing kluge 005
 *				    #   replaces survey record timestamps with
 *				    #   timestamps of corresponding merged navigation
 *				    #   records
 *				    #   - this feature allows users to fix
 *				    #     timestamp errors using MBnavedit and
 *				    #     then insert the corrected timestamps
 *				    #     into processed data
 *   KLUGE006                       # processing kluge 006
 *                                  #   changes sonar depth / draft values without
 *                                  #   changing bathymetry values
 *   KLUGE007                       # processing kluge 007
 *				    #   zeros alongtrack values greater than half
 *				    #   the altitude
 *				    #   - this targets some SeaBeam 2112 data originally
 *                                  #     processed using MB-System 4.6.10 that had
 *                                  #     a few pings with bad sidescan alongtrack values
 *				    #   - its not clear if this problem originated with
 *				    #     the data or a bug in MB-System 4.6.10
 *   KLUGE008                       # processing kluge 008 (not yet defined)
 *				    #   - occasionaly odd processing problems will
 *				    #     occur that are specific to a particular
 *				    #     survey or sonar version
 *				    #   - mbprocess will allow one-time fixes to
 *				    #     be defined as "kluges" that can be turned
 *				    #     on through the parameter files.
 *   KLUGE009                       # processing kluge 009 (not yet defined)
 *				    #   - occasionaly odd processing problems will
 *				    #     occur that are specific to a particular
 *				    #     survey or sonar version
 *				    #   - mbprocess will allow one-time fixes to
 *				    #     be defined as "kluges" that can be turned
 *				    #     on through the parameter files.
 *   KLUGE010                       # processing kluge 010 (not yet defined)
 *				    #   - occasionaly odd processing problems will
 *				    #     occur that are specific to a particular
 *				    #     survey or sonar version
 *				    #   - mbprocess will allow one-time fixes to
 *				    #     be defined as "kluges" that can be turned
 *				    #     on through the parameter files.
 *
 * MBprocess and its associated functions and programs use
 * the following file naming convention. The unprocessed swath
 * data file should have a name like "fileroot.mbxxx", where
 * fileroot can be anything and xxx corresponds to the MB-System
 * format id number. Given this kind of filename, we then have:
 *	fileroot.mbxxx.par	    : parameter file
 *	fileroot.mbxxx.nve	    : edited navigation from
 *					mbnavedit
 *	fileroot.mbxxx.nvX	    : adjusted navigation from
 *					mbnavadjust, with
 *					X={0,1,2,3,4,5,6,7,8,9}
 *	fileroot.mbxxx.esf	    : bathymetry edit save file
 *					from mbedit
 *
 * MBprocess and its associated programs utilize a simple file locking
 * mechanism to prevent multiple users or processes from working on
 * the same swath file simultaneously. This mechanism is instituted
 * using two functions: mbp_lockswathfile() and mbp_unlockswathfile().
 * The mbp_lockfile() function creates a *.lck file parallel to a
 * raw swathfile listing the program, purpose, and user locking the
 * file. While this file exists, other MB-System processing programs
 * (e.g. mbprocess, mbedit, mbeditviz, mbnavedit) will not open the
 * file (except in browse mode). The mbp_unlockfile() removes the
 * *.lck file, provided the program, purpose, and user match that
 * contained in the file. MBdatalist includes a capability to remove
 * locks from single files or entire datalist structures, allowing
 * the resetting of files left locked by crashed programs.
 *
 *
 * Author:	D. W. Caress
 * Date:	September 11, 2000
 * Updated:	August 4, 2009 (R/V Zephyr, Cleft Segment, Juan de Fuca Ridge)
 *
 * $Log: mb_process.h,v $
 * Revision 5.29  2009/03/02 18:51:52  caress
 * Fixed problems with formats 58 and 59, and also updated copyright dates in several source files.
 *
 * Revision 5.28  2008/09/11 20:11:52  caress
 * Checking in updates made during cruise AT15-36.
 *
 * Revision 5.27  2008/05/26 04:43:15  caress
 * Getting ready for release 5.1.1beta19.
 *
 * Revision 5.26  2007/10/08 15:59:34  caress
 * MBIO changes as of 8 October 2007.
 *
 * Revision 5.25  2007/05/14 06:25:47  caress
 * Added support for vertical as well as lateral adjustments from mbnavadjust.
 *
 * Revision 5.24  2006/06/16 19:30:58  caress
 * Check in after the Santa Monica Basin Mapping AUV Expedition.
 *
 * Revision 5.23  2006/01/06 18:27:19  caress
 * Working towards 5.0.8
 *
 * Revision 5.22  2005/03/25 04:16:41  caress
 * Added sonar depth merging to mbprocess.
 *
 * Revision 5.21  2004/12/02 06:33:31  caress
 * Fixes while supporting Reson 7k data.
 *
 * Revision 5.20  2004/10/06 19:04:24  caress
 * Release 5.0.5 update.
 *
 * Revision 5.19  2004/05/21 23:46:22  caress
 * Progress supporting Reson 7k data, including support for extracing subbottom profiler data.
 *
 * Revision 5.18  2003/07/26 17:59:32  caress
 * Changed beamflag handling code.
 *
 * Revision 5.17  2003/04/17 21:05:23  caress
 * Release 5.0.beta30
 *
 * Revision 5.16  2003/04/16 16:47:41  caress
 * Release 5.0.beta30
 *
 * Revision 5.15  2002/09/18 23:32:59  caress
 * Release 5.0.beta23
 *
 * Revision 5.14  2002/09/07 04:48:34  caress
 * Added slope mode option to mb_process.
 *
 * Revision 5.13  2002/07/25 19:09:04  caress
 * Release 5.0.beta21
 *
 * Revision 5.12  2002/07/20 20:42:40  caress
 * Release 5.0.beta20
 *
 * Revision 5.11  2002/04/06 02:43:39  caress
 * Release 5.0.beta16
 *
 * Revision 5.10  2001/12/18 04:27:45  caress
 * Release 5.0.beta11.
 *
 * Revision 5.9  2001/08/10  22:41:19  dcaress
 * Release 5.0.beta07
 *
 * Revision 5.8  2001-08-03 18:00:02-07  caress
 * Added cut by speed.
 *
 * Revision 5.7  2001/08/02  01:49:25  caress
 * Added mb_pr_ function prototypes.
 *
 * Revision 5.6  2001/07/31  00:40:52  caress
 * Added data cutting capability.
 *
 * Revision 5.5  2001/07/27  19:07:16  caress
 * Added data cutting.
 *
 * Revision 5.4  2001/06/03 06:54:56  caress
 * Improved handling of lever calculation.
 *
 * Revision 5.3  2001/06/01  00:14:06  caress
 * Added support for metadata insertion.
 *
 * Revision 5.2  2001/03/22  20:50:02  caress
 * Trying to make version 5.0.beta0
 *
 * Revision 5.1  2001/01/22  07:43:34  caress
 * Version 5.0.beta01
 *
 * Revision 5.0  2000/12/01  22:48:41  caress
 * First cut at Version 5.0.
 *
 * Revision 4.0  2000/09/30  06:28:42  caress
 * Snapshot for Dale.
 *
 *
 */

/* start this include */
#ifndef MB_PROCESS_DEF

/* header file flag */
#define	MB_PROCESS_DEF		1

/* include mb_io.h if needed */
#ifndef MB_IO_DEF
#include "mb_io.h"
#endif

/* mbprocess value defines */
#define MBP_FILENAMESIZE	MB_PATH_MAXLINE
#define MBP_METANOVALUE		9999999.
#define MBP_NAV_OFF		0
#define MBP_NAV_ON		1
#define MBP_NAVADJ_OFF		0
#define MBP_NAVADJ_LL		1
#define MBP_NAVADJ_LLZ		2
#define MBP_NAV_LINEAR		0
#define MBP_NAV_SPLINE		1
#define MBP_ATTITUDE_OFF	0
#define MBP_ATTITUDE_ON		1
#define MBP_SONARDEPTH_OFF	0
#define MBP_SONARDEPTH_ON	1
#define	MBP_CUT_DATA_BATH	0
#define	MBP_CUT_DATA_AMP	1
#define	MBP_CUT_DATA_SS		2
#define	MBP_CUT_MODE_NONE	0
#define	MBP_CUT_MODE_NUMBER   	1
#define	MBP_CUT_MODE_DISTANCE 	2
#define	MBP_CUT_MODE_SPEED 	3
#define	MBP_CUT_NUM_MAX		20
#define MBP_EDIT_OFF		0
#define MBP_EDIT_ON		1
#define MBP_EDIT_FLAG		1
#define MBP_EDIT_UNFLAG		2
#define MBP_EDIT_ZERO		3
#define MBP_EDIT_FILTER		4
#define MBP_ESF_NOWRITE		0
#define MBP_ESF_WRITE		1
#define MBP_ESF_APPEND		2
#define MBP_BATHRECALC_OFF	0
#define MBP_BATHRECALC_RAYTRACE	1
#define MBP_BATHRECALC_ROTATE	2
#define MBP_BATHRECALC_OFFSET	3
#define MBP_SVP_OFF		0
#define MBP_SVP_ON		1
#define MBP_SVP_SOUNDSPEEDREF	2
#define MBP_SSV_OFF		0
#define MBP_SSV_OFFSET		1
#define MBP_SSV_SET		2
#define MBP_TT_OFF		0
#define MBP_TT_MULTIPLY		1
#define	MBP_ANGLES_OK		0
#define	MBP_ANGLES_SNELL	1
#define	MBP_ANGLES_SNELLNULL	2
#define MBP_STATIC_OFF		0
#define MBP_STATIC_BEAM_ON	1
#define MBP_STATIC_ANGLE_ON	2
#define MBP_DRAFT_OFF		0
#define MBP_DRAFT_OFFSET	1
#define MBP_DRAFT_MULTIPLY	2
#define MBP_DRAFT_MULTIPLYOFFSET	3
#define MBP_DRAFT_SET		4
#define MBP_HEAVE_OFF		0
#define MBP_HEAVE_OFFSET	1
#define MBP_HEAVE_MULTIPLY	2
#define MBP_HEAVE_MULTIPLYOFFSET	3
#define MBP_LEVER_OFF		0
#define MBP_LEVER_ON		1
#define MBP_ROLLBIAS_OFF	0
#define MBP_ROLLBIAS_SINGLE	1
#define MBP_ROLLBIAS_DOUBLE	2
#define MBP_PITCHBIAS_OFF	0
#define MBP_PITCHBIAS_ON	1
#define MBP_HEADING_OFF         0
#define MBP_HEADING_CALC        1
#define MBP_HEADING_OFFSET      2
#define MBP_HEADING_CALCOFFSET  3
#define MBP_TIDE_OFF		0
#define MBP_TIDE_ON		1
#define MBP_AMPCORR_OFF		0
#define MBP_AMPCORR_ON		1
#define MBP_AMPCORR_UNKNOWN	-1
#define MBP_AMPCORR_SUBTRACTION	0
#define MBP_AMPCORR_DIVISION	1
#define MBP_AMPCORR_ASYMMETRIC	0
#define MBP_AMPCORR_SYMMETRIC	1
#define MBP_AMPCORR_IGNORESLOPE	0
#define MBP_AMPCORR_USESLOPE	1
#define MBP_AMPCORR_USETOPO	2
#define MBP_AMPCORR_USETOPOSLOPE	3
#define MBP_SSCORR_OFF		0
#define MBP_SSCORR_ON		1
#define MBP_SSCORR_UNKNOWN	-1
#define MBP_SSCORR_SUBTRACTION	0
#define MBP_SSCORR_DIVISION	1
#define MBP_SSCORR_ASYMMETRIC	0
#define MBP_SSCORR_SYMMETRIC	1
#define MBP_SSCORR_IGNORESLOPE	0
#define MBP_SSCORR_USESLOPE	1
#define MBP_SSCORR_USETOPO	2
#define MBP_SSCORR_USETOPOSLOPE	3
#define MBP_SSRECALC_OFF	0
#define MBP_SSRECALC_ON		1
#define MBP_CORRECTION_UNKNOWN	-1
#define MBP_CORRECTION_NO	0
#define MBP_CORRECTION_YES	1

/* mbprocess file locking defines */
#define MBP_LOCK_NONE		0
#define MBP_LOCK_PROCESS	1
#define MBP_LOCK_EDITBATHY	2
#define MBP_LOCK_EDITNAV	3
#define MBP_UNLOCK_OVERRIDE	0
#define MBP_UNLOCK_PROCESS	1
#define MBP_UNLOCK_EDITBATHY	2
#define MBP_UNLOCK_EDITNAV	3

/* mbprocess file checking */
#define MB_PR_FILE_UP_TO_DATE		0
#define MB_PR_FILE_NEEDS_PROCESSING	1
#define MB_PR_FILE_NOT_EXIST		2
#define MB_PR_NO_PARAMETER_FILE		3

/* structure holding mbprocess parameters */
struct mb_process_struct
	{
	/* general parameters */
	int	mbp_ifile_specified;
	char	mbp_ifile[MBP_FILENAMESIZE];
	int	mbp_ofile_specified;
	char	mbp_ofile[MBP_FILENAMESIZE];
	int	mbp_format_specified;
	int	mbp_format;

	/* navigation merging */
	int	mbp_nav_mode;
	char	mbp_navfile[MBP_FILENAMESIZE];
	int	mbp_nav_format;
	int	mbp_nav_heading;
	int	mbp_nav_speed;
	int	mbp_nav_draft;
	int	mbp_nav_attitude;
	int	mbp_nav_algorithm;
	double	mbp_nav_timeshift;
	int	mbp_nav_shift;
	double	mbp_nav_offsetx;
	double	mbp_nav_offsety;
	double	mbp_nav_offsetz;
	double	mbp_nav_shiftlon;
	double	mbp_nav_shiftlat;

	/* adjusted navigation merging */
	int	mbp_navadj_mode;
	char	mbp_navadjfile[MBP_FILENAMESIZE];
	int	mbp_navadj_algorithm;

	/* attitude merging */
	int	mbp_attitude_mode;
	char	mbp_attitudefile[MBP_FILENAMESIZE];
	int	mbp_attitude_format;

	/* sonardepth merging */
	int	mbp_sonardepth_mode;
	char	mbp_sonardepthfile[MBP_FILENAMESIZE];
	int	mbp_sonardepth_format;

	/* data cutting */
	int	mbp_cut_num;
	int	mbp_cut_kind[MBP_CUT_NUM_MAX];
	int	mbp_cut_mode[MBP_CUT_NUM_MAX];
	double	mbp_cut_min[MBP_CUT_NUM_MAX];
	double	mbp_cut_max[MBP_CUT_NUM_MAX];

	/* bathymetry editing */
	int	mbp_edit_mode;
	char	mbp_editfile[MBP_FILENAMESIZE];

	/* bathymetry recalculation */
	int	mbp_bathrecalc_mode;
	int	mbp_svp_mode;
	char	mbp_svpfile[MBP_FILENAMESIZE];
	int	mbp_ssv_mode;
	double	mbp_ssv;
	int	mbp_tt_mode;
	double	mbp_tt_mult;
	int	mbp_angle_mode;
	int	mbp_corrected;
	int	mbp_static_mode;
	char	mbp_staticfile[MBP_FILENAMESIZE];

	/* draft correction */
	int	mbp_draft_mode;
	double	mbp_draft;
	double	mbp_draft_offset;
	double	mbp_draft_mult;

	/* heave correction */
	int	mbp_heave_mode;
	double	mbp_heave;
	double	mbp_heave_mult;

	/* lever correction */
	int	mbp_lever_mode;
	double	mbp_vru_offsetx;
	double	mbp_vru_offsety;
	double	mbp_vru_offsetz;
	double	mbp_sonar_offsetx;
	double	mbp_sonar_offsety;
	double	mbp_sonar_offsetz;

	/* roll correction */
	int	mbp_rollbias_mode;
	double	mbp_rollbias;
	double	mbp_rollbias_port;
	double	mbp_rollbias_stbd;

	/* pitch correction */
	int	mbp_pitchbias_mode;
	double	mbp_pitchbias;

	/* heading correction */
	int	mbp_heading_mode;
	double	mbp_headingbias;

	/* tide correction */
	int	mbp_tide_mode;
	char	mbp_tidefile[MBP_FILENAMESIZE];
	int	mbp_tide_format;

	/* amplitude correction */
	int	mbp_ampcorr_mode;
	char	mbp_ampcorrfile[MBP_FILENAMESIZE];
	int	mbp_ampcorr_type;
	int	mbp_ampcorr_symmetry;
	double	mbp_ampcorr_angle;
	int	mbp_ampcorr_slope;

	/* sidescan correction */
	int	mbp_sscorr_mode;
	char	mbp_sscorrfile[MBP_FILENAMESIZE];
	int	mbp_sscorr_type;
	int	mbp_sscorr_symmetry;
	double	mbp_sscorr_angle;
	int	mbp_sscorr_slope;

	/* amplitude and sidescan correction */
	char	mbp_ampsscorr_topofile[MBP_FILENAMESIZE];

	/* sidescan recalculation */
	int	mbp_ssrecalc_mode;
	double	mbp_ssrecalc_pixelsize;
	double	mbp_ssrecalc_swathwidth;
	int	mbp_ssrecalc_interpolate;

	/* metadata strings */
	char	mbp_meta_vessel[MBP_FILENAMESIZE];
	char	mbp_meta_institution[MBP_FILENAMESIZE];
	char	mbp_meta_platform[MBP_FILENAMESIZE];
	char	mbp_meta_sonar[MBP_FILENAMESIZE];
	char	mbp_meta_sonarversion[MBP_FILENAMESIZE];
	char	mbp_meta_cruiseid[MBP_FILENAMESIZE];
	char	mbp_meta_cruisename[MBP_FILENAMESIZE];
	char	mbp_meta_pi[MBP_FILENAMESIZE];
	char	mbp_meta_piinstitution[MBP_FILENAMESIZE];
	char	mbp_meta_client[MBP_FILENAMESIZE];
	int	mbp_meta_svcorrected;
	int	mbp_meta_tidecorrected;
	int	mbp_meta_batheditmanual;
	int	mbp_meta_batheditauto;
	double	mbp_meta_rollbias;
	double	mbp_meta_pitchbias;
	double	mbp_meta_headingbias;
	double	mbp_meta_draft;

	/* processing kluges */
	int	mbp_kluge001;
	int	mbp_kluge002;
	int	mbp_kluge003;
	int	mbp_kluge004;
	int	mbp_kluge005;
	int	mbp_kluge006;
	int	mbp_kluge007;
	int	mbp_kluge008;
	int	mbp_kluge009;
	int	mbp_kluge010;
	};

/* edit save file definitions */
#define MB_ESF_MAXTIMEDIFF 0.00011
#define MB_ESF_MULTIPLICITY_FACTOR	1000000
struct mb_edit_struct
	{
	double	time_d;
	int	beam;
	int	action;
	int	use;
	};
struct mb_esf_struct
	{
	char	esffile[MB_PATH_MAXLINE];
	char	esstream[MB_PATH_MAXLINE];
	int	byteswapped;
	int	nedit;
	struct mb_edit_struct	*edit;
	FILE	*esffp;
	FILE	*essfp;
        int     startnextsearch;
	};

int mb_pr_checkstatus(int verbose, char *file,
			int *prstatus, int *error);
int mb_pr_readpar(int verbose, char *file, int lookforfiles,
			struct mb_process_struct *process,
			int *error);
int mb_pr_writepar(int verbose, char *file,
			struct mb_process_struct *process,
			int *error);
int mb_pr_bathmode(int verbose, struct mb_process_struct *process,
			int *error);
int mb_pr_default_output(int verbose, struct mb_process_struct *process,
			int *error);
int mb_pr_get_output(int verbose, int *format,
			char *ifile, char *ofile,
			int *error);
int mb_pr_check(int verbose, char *ifile,
			int *nparproblem,
			int *ndataproblem,
			int *error);
int mb_pr_update_ofile(int verbose, char *file,
			int	mbp_ofile_specified,
			char	*mbp_ofile,
			int	*error);
int mb_pr_update_format(int verbose, char *file,
			int mbp_format_specified,
			int mbp_format,
			int *error);
int mb_pr_update_rollbias(int verbose, char *file,
			int	mbp_rollbias_mode,
			double	mbp_rollbias,
			double	mbp_rollbias_port,
			double	mbp_rollbias_stbd,
			int *error);
int mb_pr_update_pitchbias(int verbose, char *file,
			int	mbp_pitchbias_mode,
			double	mbp_pitchbias,
			int *error);
int mb_pr_update_draft(int verbose, char *file,
			int	mbp_draft_mode,
			double	mbp_draft,
			double	mbp_draft_offset,
			double	mbp_draft_mult,
			int *error);
int mb_pr_update_heave(int verbose, char *file,
			int	mbp_heave_mode,
			double	mbp_heave,
			double	mbp_heave_mult,
			int *error);
int mb_pr_update_lever(int verbose, char *file,
			int	mbp_lever_mode,
			double	mbp_vru_offsetx,
			double	mbp_vru_offsety,
			double	mbp_vru_offsetz,
			double	mbp_sonar_offsetx,
			double	mbp_sonar_offsety,
			double	mbp_sonar_offsetz,
			int *error);
int mb_pr_update_tide(int verbose, char *file,
			int	mbp_tide_mode,
			char *mbp_tidefile,
			int	mbp_tide_format,
			int *error);
int mb_pr_update_tt(int verbose, char *file,
			int	mbp_tt_mode,
			double	mbp_tt_mult,
			int *error);
int mb_pr_update_ssv(int verbose, char *file,
			int	mbp_ssv_mode,
			double	mbp_ssv,
			int *error);
int mb_pr_update_svp(int verbose, char *file,
			int	mbp_svp_mode,
			char	*mbp_svpfile,
			int	mbp_angle_mode,
			int	mbp_corrected,
			int *error);
int mb_pr_update_static(int verbose, char *file,
			int	mbp_static_mode,
			char	*mbp_staticfile,
			int *error);
int mb_pr_update_navadj(int verbose, char *file,
			int	mbp_navadj_mode,
			char	*mbp_navadjfile,
			int	mbp_navadj_algorithm,
			int *error);
int mb_pr_update_nav(int verbose, char *file,
			int	mbp_nav_mode,
			char	*mbp_navfile,
			int	mbp_nav_format,
			int	mbp_nav_heading,
			int	mbp_nav_speed,
			int	mbp_nav_draft,
			int	mbp_nav_attitude,
			int	mbp_nav_algorithm,
			double mbp_nav_timeshift,
			int *error);
int mb_pr_update_attitude(int verbose, char *file,
			int	mbp_attitude_mode,
			char	*mbp_attitudefile,
			int	mbp_attitude_format,
			int *error);
int mb_pr_update_sonardepth(int verbose, char *file,
			int	mbp_sonardepth_mode,
			char	*mbp_sonardepthfile,
			int	mbp_sonardepth_format,
			int *error);
int mb_pr_update_navshift(int verbose, char *file,
			int	mbp_nav_shift,
			double	mbp_nav_offsetx,
			double	mbp_nav_offsety,
			double	mbp_nav_offsetz,
			double	mbp_nav_shiftlon,
			double	mbp_nav_shiftlat,
			int *error);
int mb_pr_update_heading(int verbose, char *file,
			int	mbp_heading_mode,
			double	mbp_headingbias,
			int *error);
int mb_pr_update_datacut(int verbose, char *file,
			int	mbp_cut_num,
			int	*mbp_cut_kind,
			int	*mbp_cut_mode,
			double	*mbp_cut_min,
			double	*mbp_cut_max,
			int *error);
int mb_pr_update_edit(int verbose, char *file,
			int	mbp_edit_mode,
			char	*mbp_editfile,
			int *error);
int mb_pr_update_ampcorr(int verbose, char *file,
			int	mbp_ampcorr_mode,
			char	*mbp_ampcorrfile,
			int	mbp_ampcorr_type,
			int	mbp_ampcorr_symmetry,
			double	mbp_ampcorr_angle,
			int	mbp_ampcorr_slope,
			char	*mbp_ampcorr_topofile,
			int *error);
int mb_pr_update_sscorr(int verbose, char *file,
			int	mbp_sscorr_mode,
			char	*mbp_sscorrfile,
			int	mbp_sscorr_type,
			int	mbp_sscorr_symmetry,
			double	mbp_sscorr_angle,
			int	mbp_sscorr_slope,
			char	*mbp_sscorr_topofile,
			int *error);
int mb_pr_update_ssrecalc(int verbose, char *file,
			int	mbp_ssrecalc_mode,
			double	mbp_ssrecalc_pixelsize,
			double	mbp_ssrecalc_swathwidth,
			int	mbp_ssrecalc_interpolate,
			int *error);
int mb_pr_update_metadata(int verbose, char *file,
			char	*mbp_meta_vessel,
			char	*mbp_meta_institution,
			char	*mbp_meta_platform,
			char	*mbp_meta_sonar,
			char	*mbp_meta_sonarversion,
			char	*mbp_meta_cruiseid,
			char	*mbp_meta_cruisename,
			char	*mbp_meta_pi,
			char	*mbp_meta_piinstitution,
			char	*mbp_meta_client,
			int	mbp_meta_svcorrected,
			int	mbp_meta_tidecorrected,
			int	mbp_meta_batheditmanual,
			int	mbp_meta_batheditauto,
			double	mbp_meta_rollbias,
			double	mbp_meta_pitchbias,
			double	mbp_meta_headingbias,
			double	mbp_meta_draft,
			int *error);
int mb_pr_update_kluges(int verbose, char *file,
			int	mbp_kluge001,
			int	mbp_kluge002,
			int	mbp_kluge003,
			int	mbp_kluge004,
			int	mbp_kluge005,
			int	mbp_kluge006,
			int	mbp_kluge007,
			int	mbp_kluge008,
			int	mbp_kluge009,
			int	mbp_kluge010,
			int *error);
int mb_pr_get_ofile(int verbose, char *file,
			int	*mbp_ofile_specified,
			char	*mbp_ofile,
			int	*error);
int mb_pr_get_format(int verbose, char *file,
			int *mbp_format_specified,
			int *mbp_format,
			int *error);
int mb_pr_get_rollbias(int verbose, char *file,
			int	*mbp_rollbias_mode,
			double	*mbp_rollbias,
			double	*mbp_rollbias_port,
			double	*mbp_rollbias_stbd,
			int *error);
int mb_pr_get_pitchbias(int verbose, char *file,
			int	*mbp_pitchbias_mode,
			double	*mbp_pitchbias,
			int *error);
int mb_pr_get_draft(int verbose, char *file,
			int	*mbp_draft_mode,
			double	*mbp_draft,
			double	*mbp_draft_offset,
			double	*mbp_draft_mult,
			int *error);
int mb_pr_get_heave(int verbose, char *file,
			int	*mbp_heave_mode,
			double	*mbp_heave,
			double	*mbp_heave_mult,
			int *error);
int mb_pr_get_lever(int verbose, char *file,
			int	*mbp_lever_mode,
			double	*mbp_vru_offsetx,
			double	*mbp_vru_offsety,
			double	*mbp_vru_offsetz,
			double	*mbp_sonar_offsetx,
			double	*mbp_sonar_offsety,
			double	*mbp_sonar_offsetz,
			int *error);
int mb_pr_get_tide(int verbose, char *file,
			int	*mbp_tide_mode,
			char *mbp_tidefile,
			int	*mbp_tide_format,
			int *error);
int mb_pr_get_tt(int verbose, char *file,
			int	*mbp_tt_mode,
			double	*mbp_tt_mult,
			int *error);
int mb_pr_get_ssv(int verbose, char *file,
			int	*mbp_ssv_mode,
			double	*mbp_ssv,
			int *error);
int mb_pr_get_svp(int verbose, char *file,
			int	*mbp_svp_mode,
			char	*mbp_svpfile,
			int	*mbp_angle_mode,
			int	*mbp_corrected,
			int *error);
int mb_pr_get_static(int verbose, char *file,
			int	*mbp_static_mode,
			char	*mbp_staticfile,
			int *error);
int mb_pr_get_navadj(int verbose, char *file,
			int	*mbp_navadj_mode,
			char	*mbp_navadjfile,
			int	*mbp_navadj_algorithm,
			int *error);
int mb_pr_get_nav(int verbose, char *file,
			int	*mbp_nav_mode,
			char	*mbp_navfile,
			int	*mbp_nav_format,
			int	*mbp_nav_heading,
			int	*mbp_nav_speed,
			int	*mbp_nav_draft,
			int	*mbp_nav_attitude,
			int	*mbp_nav_algorithm,
			double *mbp_nav_timeshift,
			int *error);
int mb_pr_get_attitude(int verbose, char *file,
			int	*mbp_attitude_mode,
			char	*mbp_attitudefile,
			int	*mbp_attitude_format,
			int *error);
int mb_pr_get_sonardepth(int verbose, char *file,
			int	*mbp_sonardepth_mode,
			char	*mbp_sonardepthfile,
			int	*mbp_sonardepth_format,
			int *error);
int mb_pr_get_navshift(int verbose, char *file,
			int	*mbp_nav_shift,
			double	*mbp_nav_offsetx,
			double	*mbp_nav_offsety,
			double	*mbp_nav_offsetz,
			double	*mbp_nav_shiftlon,
			double	*mbp_nav_shiftlat,
			int *error);
int mb_pr_get_heading(int verbose, char *file,
			int	*mbp_heading_mode,
			double	*mbp_headingbias,
			int *error);
int mb_pr_get_datacut(int verbose, char *file,
			int	*mbp_cut_num,
			int	*mbp_cut_kind,
			int	*mbp_cut_mode,
			double	*mbp_cut_min,
			double	*mbp_cut_max,
			int *error);
int mb_pr_get_edit(int verbose, char *file,
			int	*mbp_edit_mode,
			char	*mbp_editfile,
			int *error);
int mb_pr_get_ampcorr(int verbose, char *file,
			int	*mbp_ampcorr_mode,
			char	*mbp_ampcorrfile,
			int	*mbp_ampcorr_type,
			int	*mbp_ampcorr_symmetry,
			double	*mbp_ampcorr_angle,
			int	*mbp_ampcorr_slope,
			char	*mbp_ampcorr_topofile,
			int *error);
int mb_pr_get_sscorr(int verbose, char *file,
			int	*mbp_sscorr_mode,
			char	*mbp_sscorrfile,
			int	*mbp_sscorr_type,
			int	*mbp_sscorr_symmetry,
			double	*mbp_sscorr_angle,
			int	*mbp_sscorr_slope,
			char	*mbp_sscorr_topofile,
			int *error);
int mb_pr_get_ssrecalc(int verbose, char *file,
			int	*mbp_ssrecalc_mode,
			double	*mbp_ssrecalc_pixelsize,
			double	*mbp_ssrecalc_swathwidth,
			int	*mbp_ssrecalc_interpolate,
			int *error);
int mb_pr_get_metadata(int verbose, char *file,
			char	*mbp_meta_vessel,
			char	*mbp_meta_institution,
			char	*mbp_meta_platform,
			char	*mbp_meta_sonar,
			char	*mbp_meta_sonarversion,
			char	*mbp_meta_cruiseid,
			char	*mbp_meta_cruisename,
			char	*mbp_meta_pi,
			char	*mbp_meta_piinstitution,
			char	*mbp_meta_client,
			int	*mbp_meta_svcorrected,
			int	*mbp_meta_tidecorrected,
			int	*mbp_meta_batheditmanual,
			int	*mbp_meta_batheditauto,
			double	*mbp_meta_rollbias,
			double	*mbp_meta_pitchbias,
			double	*mbp_meta_headingbias,
			double	*mbp_meta_draft,
			int *error);
int mb_pr_get_kluges(int verbose, char *file,
			int	*mbp_kluge001,
			int	*mbp_kluge002,
			int	*mbp_kluge003,
			int	*mbp_kluge004,
			int	*mbp_kluge005,
			int	*mbp_kluge006,
			int	*mbp_kluge007,
			int	*mbp_kluge008,
			int	*mbp_kluge009,
			int	*mbp_kluge010,
			int *error);
int mb_pr_set_bathyslope(int verbose,
			int nsmooth,
			int nbath, char *beamflag, double *bath, double *bathacrosstrack,
			int *ndepths, double *depths, double *depthacrosstrack,
			int *nslopes, double *slopes, double *slopeacrosstrack,
			double *depthsmooth,
			int *error);
int mb_pr_get_bathyslope(int verbose,
			int ndepths, double *depths, double *depthacrosstrack,
			int nslopes, double *slopes, double *slopeacrosstrack,
			double acrosstrack, double *depth, double *slope,
			int *error);
int mb_pr_point_in_quad(int verbose, double px, double py,
			double *x, double *y, int *error);
int mb_esf_check(int verbose, char *swathfile, char *esffile,
			int *found, int *error);
int mb_esf_load(int verbose, char *swathfile,
			int load, int output,
			char *esffile,
			struct mb_esf_struct *esf,
			int *error);
int mb_esf_open(int verbose, char *esffile,
			int load, int output,
			struct mb_esf_struct *esf,
			int *error);
int mb_esf_fixtimestamps(int verbose, struct mb_esf_struct *esf,
			double time_d, double tolerance, int *error);
int mb_esf_apply(int verbose, struct mb_esf_struct *esf,
			double time_d, int pingmultiplicity, int nbath, char *beamflag,
			int *error);
int mb_esf_save(int verbose, struct mb_esf_struct *esf,
			double time_d, int beam, int action, int *error);
int mb_ess_save(int verbose, struct mb_esf_struct *esf,
		double time_d, int beam, int action, int *error);
int mb_esf_close(int verbose, struct mb_esf_struct *esf, int *error);

int mb_pr_lockswathfile(int verbose, char *file, int purpose,
			char *program, int *error);
int mb_pr_unlockswathfile(int verbose, char *file, int purpose,
			char *program, int *error);
int mb_pr_lockinfo(int verbose, char *file, int *locked,
			int *purpose, char *program, char *user, char *cpu,
			char *date, int *error);

/* end this include */
#endif
