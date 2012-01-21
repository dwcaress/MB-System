/*-----------------------------------------------------------------------
/ H E A D E R K O P F
/ ------------------------------------------------------------------------
/ ------------------------------------------------------------------------
/  DATEINAME        : surf.h     Version 3.0
/  ERSTELLUNGSDATUM : 29.07.93
/ ----------------------------------------------------------------------*/
/*!
/ ------------------------------------------------------------------------
/ COPYRIGHT (C) 1993: ATLAS ELEKTRONIK GMBH, 28305 BREMEN
/ ------------------------------------------------------------------------
/
/  See README file for copying and redistribution conditions.
/
/
/ HIER/SACHN: P: RP ____ _ ___ __
/ BENENNUNG :
/ ERSTELLER : Peter Block    : SAS3
/ FREIGABE  : __.__.__  GS__
/ AEND/STAND: __.__.__  __
/ PRUEFVERM.:
*/

/*
/ SPRACHE          : UNIX-C
/ COMPILER         : Silicon Graphix
/ BETRIEBSSYSTEM   : IRIX
/ HARDWARE-UMGEBUNG: SGI Crimson
/ URSPRUNGSHINWEIS :
/
/ ------------------------------------------------------------------------
/ BESCHREIBUNG:       Definitions describing the "SURF-Format" V2.0
/ ------------------------------------------------------------------------
/
/ ------------------------------------------------------------------------
/ E N D E   D E S   K O P F E S
/ ------------------------------------------------------------------------
*/
/************************************************************************/

/* Changed long values to int and xdr_long() calls to xdr_int() so that
	the code works with existing data on 64 bit compilers
	David W. Caress
	February 2, 2010 */

#ifndef _SURF
#define _SURF

#ifndef SURF20
#define SURF20
#endif

#ifndef SURF30
#define SURF30
#endif

#ifdef _WIN32
#include "xdr_win32.h"
#else
#include <rpc/types.h>
#include <rpc/xdr.h>
#endif
/*
#include "strings_surf.h"
*/
/*
   time/date-sets are presented in ASCII-Characters
      DDMMYYHHMMSS.NN<0x0>
   name-strings and Label-strings are presented in 
   c-string-annotation    ABCDEFG<0x0> 
*/

#define LABEL_SIZE          16
#define STRING_SIZE         16
#define TIME_SIZE           16
#define TEXT_SIZE           80


/* if number of sets or contents are modified , 
   you should update VERSION !!! */

  
#define SURF_VERSION "SURF V3.0"

#define SURF_VERS3_0 "SURF V3.0"
#define SURF_VERS2_0 "SURF V2.0"


/***************************************/
/*                                     */
/* SURF-elements describing SIX-files  */
/*                                     */
/***************************************/



/* datatypes of different SURF-datasets */

   /* marker types */
            
#define      MIN_M          -4
#define      EOD_M          -4
#define      NROF_M         -3       
#define      SDA_M          -2       
#define      SIX_M          -1      
            
   /* six types */
         
#define      DESCRIPTOR      1
#define      GLOBALDATA      2
#define      STATISTICS      3
#define      POSITIONSENSORS 4
#define      TRANSDUCERPARAM 5
#define      BEAMANGLE       6
#define      CPROFILE        7
#define      AREAPOLYGON     8
#define      EVENTS          9
#define      FREETEXT       10
#define      ADDSTATISTICS  11
#define      TPESTATICS     12
#define      FREESIXDESCR   13
#define      FREESNDGDESCR  14
#define      FREEBEAMDESCR  15
#define      SIXATTDATA     16 
#define      VENDORTEXT     17
#define      CPROFTPES      18
#define      MAXSIX         18

   /* sda types */
         
#define      MINSDA          40
#define      SOUNDING        40
#define      CENTERPOSITION  41
#define      SINGLEBEAMDEPTH 42
#define      MULTIBEAMDEPTH  43 
#define      MULTIBEAMTT     44 
#define      MULTIBEAMRECV   45
#define      SIGNALPARMS     46
#define      SIGNALAMPLITUDE 47  /* wird nicht mehr benutzt V2.0 */
#define      BEAMAMPLITUDES  48
#define      EXTBEAMAMPLI    49
#define      SIDESCANDATA    50
#define      TXPARMS         51
#define      POSITIONCEP     52
#define      MULTITPES       53
#define      SINGLETPES      54
#define      SNDGATTDATA     55
#define      BEAMATTDATA     56
#define      MAXSDA          56

   /* nrof types */
         
#define      MAX_NROF_BEAMS_PER_TABLE          80
#define      MAX_NROF_CPROFILES_PER_TABLE      81
#define      MAX_NROF_POLYGONS_PER_TABLE       82
#define      MAX_NROF_EVENTS                   83
#define      MAX_NROF_FREE_TEXT_BLOCKS         84
#define      MAX_NROF_SIDESCAN_DATA            85
#define      NROF_RX_TVG_SETS                  86
#define      NROF_TX_TVG_SETS                  87
#define      MAX_NR_OF_TYPES                   87




/* SURF-dataset "Descriptor" */

#define SURF_DESCRIPTOR_LABEL SURF_VERSION

typedef short SurfMarkerDescriptor;

typedef struct
            {
              short       typ        ;
              u_int       nr         ;
            } SurfSixDescriptor;
              
typedef struct
            {
              short       typ        ;
              u_int       nr         ;
            } SurfSdaDescriptor;

typedef struct
            {
              short       typ        ;
              u_int       nr         ;
            } SurfNrofDescriptor;

typedef struct 
            {
              char                  label    [LABEL_SIZE]          ;
              SurfMarkerDescriptor  six                            ;
              SurfSixDescriptor     descriptor                     ; 
              SurfSixDescriptor     globalData                     ; 
              SurfSixDescriptor     statistics                     ; 
              SurfSixDescriptor     positionSensor                 ; 
              SurfSixDescriptor     transducer                     ; 
              SurfSixDescriptor     angleTab                       ; 
              SurfSixDescriptor     cProfile                       ; 
              SurfSixDescriptor     polygon                        ; 
              SurfSixDescriptor     events                         ; 
              SurfSixDescriptor     freeText                       ;
              SurfSixDescriptor     addStatistics                  ;   /* new V3.0 */
              SurfSixDescriptor     tpeStatics                     ;   /* new V3.0 */
              SurfSixDescriptor     cprofTpes                      ;   /* new V3.0 */
              SurfSixDescriptor     freeSixDescr                   ;   /* new V3.0 */
              SurfSixDescriptor     freeSndgDescr                  ;   /* new V3.0 */
              SurfSixDescriptor     freeBeamDescr                  ;   /* new V3.0 */
              SurfSixDescriptor     freeSixAttData                 ;   /* new V3.0 */
              SurfSixDescriptor     vendorText                     ;   /* new V3.0 */

              SurfMarkerDescriptor  sda                            ;
              SurfSdaDescriptor     soundings                      ; 
              SurfSdaDescriptor     centerPositions                ; 
              SurfSdaDescriptor     singleBeamDepth                ; 
              SurfSdaDescriptor     multiBeamDepth                 ; 
              SurfSdaDescriptor     multiBeamTT                    ; 
              SurfSdaDescriptor     multiBeamRecv                  ; 
              SurfSdaDescriptor     signalParams                   ; 
              SurfSdaDescriptor     signalAmplitudes               ;/* wird ab V2.0 nicht mehr benutzt*/ 
              SurfSdaDescriptor     beamAmplitudes                 ; 
              SurfSdaDescriptor     extendBeamAmplitudes           ; 
              SurfSdaDescriptor     sidescanData                   ; 
              SurfSdaDescriptor     txParams                       ;
              SurfSdaDescriptor     positionCpes                   ;   /* new V3.0 */
              SurfSdaDescriptor     multiTpeParams                 ;   /* new V3.0 */
              SurfSdaDescriptor     singleTpeParams                ;   /* new V3.0 */
              SurfSdaDescriptor     sndgAttData                    ;   /* new V3.0 */
              SurfSdaDescriptor     beamAttData                    ;   /* new V3.0 */ 

              SurfMarkerDescriptor  nrof                           ;
              SurfNrofDescriptor    maxNrOfBeams                   ; 
              SurfNrofDescriptor    maxNrOfCProfileElements        ; 
              SurfNrofDescriptor    maxNrOfPolygonElements         ; 
              SurfNrofDescriptor    maxNrOfEvents                  ; 
              SurfNrofDescriptor    maxNrOfFreeTextBlocks          ; 
              SurfNrofDescriptor    maxNrOfSidescanData            ; 
              SurfNrofDescriptor    nrOfRxTvgSets                  ; 
              SurfNrofDescriptor    nrOfTxTvgSets                  ; 

              SurfMarkerDescriptor  eod                            ;
            } SurfDescriptor;       





/* SURF-dataset "Globaldata" */

  /* Values of 'typeOfSounder' */
  
#define MANUAL_DATA              'M'
#define DIGITIZED_DATA           'D'
#define VERTICAL_SOUNDER         'V'
#define BOMA_TYPE_SOUNDER        'B'
#define FAN_TYPE_SOUNDER         'F'

#define DENSITY_TYPE_SOUNDER     'Z'


  /* Values of 'presentationOfPosition' */
  
#define EASTING_NORTHING         'E' /* Values will be scaled in [rad] */
#define X_Y                      'X' /* Values will be scaled in [m]   */

/* correctedParameterFlags */

#define CP_TIDE_CORRECTED          1
#define CP_DRAUGHT_CORRECTED       2
#define CP_COURSE_MANIPULATED      4
#define CP_HEAVE_MANIPULATED       8
#define CP_ROLL_MANIPULATED       16
#define CP_PITCH_MANIPULATED      32
#define CP_CKEEL_MANIPULATED      64
#define CP_CMEAN_MANIPULATED     128
#define CP_SLOPE_KORRECTED       256
#define CP_REDUCED_RAW_DATA      512 /* Data from .P-files , etc. */
#define CP_SQUAT_CORRECTED      1024
#define CP_HEAVE_COMPENSATED    2048


#define SURF_GLOBAL_DATA_LABEL "GLOBALDATA"

typedef struct
            {
              char    label                      [LABEL_SIZE];
              char    shipsName                 [STRING_SIZE];
              char    startTimeOfProfile          [TIME_SIZE];
              char    regionOfProfile           [STRING_SIZE];
              char    numberOfProfile           [STRING_SIZE];
              float   chartZero                              ;  /* rel. NN*/
              float   tideZero                               ;  /* rel. NN*/
              u_int   numberOfMeasuredSoundings              ;  
              u_int   actualNumberOfSoundingSets             ;
              char    timeDateOfTideModification  [TIME_SIZE];
              char    timeDateOfDepthModification [TIME_SIZE];
              char    timeDateOfPosiModification  [TIME_SIZE];
              char    timeDateOfParaModification  [TIME_SIZE];
              u_int   correctedParameterFlags                ;
              float   offsetHeave                            ;
              float   offsetRollPort                         ;
              float   offsetRollStar                         ;
              float   offsetPitchFore                        ;
              float   offsetPitchAft                         ;
              char    nameOfSounder             [STRING_SIZE];
              char    typeOfSounder                          ;
              float   highFrequency                          ;
              float   mediumFrequency                        ;
              float   lowFrequency                           ;
              char    nameOfEllipsoid           [STRING_SIZE];
              double  semiMajorAxis                          ;
              double  flattening                             ;
              char    projection                [STRING_SIZE];
              char    presentationOfPosition                 ;
              double  referenceMeridian                      ;
              double  falseEasting                           ;
              double  falseNorthing                          ;
              double  referenceOfPositionX                   ;
              double  referenceOfPositionY                   ;
              char    presentationOfRelWay                   ;
                /* 'p' = projection , 'l' = lineintegral */
              float   planedTrackStartX                      ;
              float   planedTrackStartY                      ;
              float   planedTrackStopX                       ;
              float   planedTrackStopY                       ;
              float   originalTrackStartX                    ;
              float   originalTrackStartY                    ;
              float   originalTrackStopX                     ;
              float   originalTrackStopY                     ;
              float   originalStartStopDistance              ;
              double  originalStartStopTime                  ;
              char    timeDateOfTrackModification [TIME_SIZE];
              float   modifiedTrackStartX                    ;
              float   modifiedTrackStartY                    ;
              float   modifiedTrackStopX                     ;
              float   modifiedTrackStopY                     ;
              float   modifiedStartStopDistance              ;
            } SurfGlobalData;





/* SURF-dataset "Statistics" */

#define SURF_STATISTICS_LABEL "STATISTICS"

typedef struct
            {
              char    label              [LABEL_SIZE];
              double  minNorthing                    ;
              double  maxNorthing                    ;
              double  minEasting                     ;
              double  maxEasting                     ;
              float   minSpeed                       ;
              float   maxSpeed                       ;
              float   minRoll                        ;
              float   maxRoll                        ;
              float   minPitch                       ;
              float   maxPitch                       ;
              float   minHeave                       ;
              float   maxHeave                       ;
              float   minBeamPositionStar            ;
              float   maxBeamPositionStar            ;
              float   minBeamPositionAhead           ;
              float   maxBeamPositionAhead           ;
              float   minDepth                       ;
              float   maxDepth                       ;
            } SurfStatistics;  




/* SURF-dataset "Additional Statistics" */           /* new V3.0 */

#define SURF_ADD_STATISTICS_LABEL "ADD_STATISTICS"

typedef struct
            {
              double  variation                      ;
              double  pointDistance                  ;
              double  maxAstar                       ;
              double  dFuture                        ;
              u_short isReduced                      ;
              u_short fromBeam                       ;
              u_short toBeam                         ;
              u_short reduceOuterBeams               ;
            } SurfReductionParameters;


typedef struct
            {
              double  depthMinDepth                  ;
              double  depthMaxDepth                  ;
              double  depthSlopeOver2                ;
              double  depthSlopeOver3                ;
              u_short depthHasParams                 ;
              u_short depthFilterAhead               ;
              u_short depthFilterAcross              ;
              u_short posHasParams                   ;
              double  posFilterRadius                ;
              double  posMaxCourseChange             ;
              double  dFuture1                       ;
              double  dFuture2                       ;
            } SurfLastFilterParameters;


typedef struct
            {
              char    label[LABEL_SIZE]              ;
              u_int   flag;
              u_int   nrNotDeletedDepth              ;
              u_int   nrNotReducedDepth              ;
              u_int   nrNotDeletedSoundings          ;
              SurfReductionParameters   redParm      ;
              SurfLastFilterParameters  filterParm   ;
              char    serverReduction[TEXT_SIZE]     ;
              double  dFuture[10]                    ;
              u_short iFuture[8]                     ;            
            } SurfAddStatistics;  





/* SURF-dataset "Positionsensors" */

#define SURF_POSITION_SENSOR_LABEL "POSITIONSENSORS"
#define UNION_SIZE 200

typedef struct
            {
              char    label                       [LABEL_SIZE];
              char    positionSensorName         [STRING_SIZE];
              char    sensorUnion                 [UNION_SIZE];
            } SurfPositionSensorArray;
              

    /*  Now special Sensors */


    /*  Overlayed Polarfix-Set  */

#define POLARFIX "POLARFIX"

typedef struct
            {
              char    label                       [LABEL_SIZE];
              char    positionSensorName         [STRING_SIZE];
              float   polarfixLocationX                       ;
              float   polarfixLocationY                       ;
              float   polarfixLocationZ                       ;
              float   polarfixReferenceX                      ;
              float   polarfixReferenceY                      ;
              float   polarfixReferenceZ                      ;
              float   polarfixReferenceDistance               ;
              float   polarfixReferenceAngle                  ;
              char    timeOfLastPolarfixEdit       [TIME_SIZE];
              float   polarfixEditLocationX                   ;
              float   polarfixEditLocationY                   ;
              float   polarfixEditLocationZ                   ;
              float   polarfixEditReferenceX                  ;
              float   polarfixEditReferenceY                  ;
              float   polarfixEditReferenceZ                  ;
              float   polarfixEditReferenceDistance           ;
              float   polarfixEditReferenceAngle              ;
              float   polarfixAntennaPositionAhead            ;
              float   polarfixAntennaPositionStar             ;
              float   polarfixAntennaPositionHeight           ;
            } SurfPositionPolarfix;
              
    /*  other overlayed Sets */

#define UNKNOWNPOSSENS   "UNKNOWN"
#define INAV             "INTEGRATED NAV"
#define SYLEDIS          "SYLEDIS"
#define MNS2000          "MNS2000"
#define GPS              "GPS"
#define EPIRB            "EPIRB"

typedef struct
            {
              char    label                       [LABEL_SIZE];
              char    positionSensorName         [STRING_SIZE];
              float   none1                                   ;
              float   none2                                   ;
              float   none3                                   ;
              float   none4                                   ;
              float   none5                                   ;
              float   none6                                   ;
              float   none7                                   ;
              float   none8                                   ;
              char    time9                        [TIME_SIZE];
              float   none10                                  ;
              float   none11                                  ;
              float   none12                                  ;
              float   none13                                  ;
              float   none14                                  ;
              float   none15                                  ;
              float   none16                                  ;
              float   none17                                  ;
              float   sensorAntennaPositionAhead              ;
              float   sensorAntennaPositionStar               ;
              float   sensorAntennaPositionHeight             ;
            } SurfPositionAnySensor;
              





/* SURF-dataset "Multibeam-Angle-Table" */

#define SURF_MULTIBEAM_ANGLE_LABEL "MULTIBEAMANGLES"

typedef struct
            {
              char              label    [LABEL_SIZE];
              u_short actualNumberOfBeams            ;
              float   beamAngle                   [1]; /*numberOfBeams times*/
            } SurfMultiBeamAngleTable;

#define SIZE_OF_SURF_MULTIBEAM_ANGLE_TAB(MAX_NR_OF_BEAMS) \
        (MAX_NR_OF_BEAMS == 0)?\
        0\
        :\
        (sizeof(SurfMultiBeamAngleTable) + \
        ((MAX_NR_OF_BEAMS - 1) * sizeof(float)))



/* SURF-dataset "Transducer Parameters" */

#define SURF_TRANSDUCER_TABLE_LABEL "TRANSDUCERTABLE"

typedef struct
            {
              char              label    [LABEL_SIZE];
              float   transducerDepth                ;
              float   transducerPositionAhead        ;
              float   transducerPositionStar         ;
              float   transducerTwoThetaHFreq        ;
              float   transducerTwoThetaMFreq        ;
              float   transducerTwoThetaLFreq        ;
            } SurfTransducerParameterTable;
              





/* SURF-dataset "C-profile-tables" */

#define SURF_C_PROFILE_LABEL "C_PROFILES"

typedef struct
            {
               float  depth                          ;
               float  cValue                         ; 
            } CProfileValues;
              

typedef struct
            {
              char              label    [LABEL_SIZE];
              float             relTime              ;
              u_short           numberOfActualValues ;
              CProfileValues    values            [1];
            } SurfCProfileTable;

#define SIZE_OF_SURF_C_PROFILE_TAB(MAX_NR_OF_PROFILES) \
        (MAX_NR_OF_PROFILES == 0)?\
        0\
        :\
        (sizeof(SurfCProfileTable) + \
        ((MAX_NR_OF_PROFILES - 1) * sizeof(CProfileValues)))

              


/* SURF-dataset "C-profile-TPE-Values" */

#define SURF_C_PROFILE_TPE_LABEL "C_PROFILE_TPES"

typedef struct
            {
              char              label    [LABEL_SIZE];
              float             cpTpes            [1];
            } SurfCProfileTpeTable;

#define SIZE_OF_SURF_C_PROFILE_TPE_TAB(MAX_NR_OF_PROFILES) \
        (MAX_NR_OF_PROFILES == 0)?\
        0\
        :\
        (sizeof(SurfCProfileTpeTable) + \
        ((MAX_NR_OF_PROFILES - 1) * sizeof(float)))

              




/* SURF-dataset "Polygon" */          

#define SURF_POLYGONS_LABEL "POLYGON"

typedef struct
            {
              double  polygonX                       ;
              double  polygonY                       ;
            } SurfPolygonValues;

typedef struct
            {
              char              label    [LABEL_SIZE];
              SurfPolygonValues values            [1];
            } SurfPolygons;

#define SIZE_OF_SURF_POLYGON_ARRAY(NR_OF_POLYGONS) \
        (NR_OF_POLYGONS == 0)?\
        0\
        :\
        (sizeof(SurfPolygons) + \
        ((NR_OF_POLYGONS - 1) * sizeof(SurfPolygonValues)))

              




/* SURF-dataset "Events " */          

#define SURF_EVENT_LABEL "EVENTS"
#define EVENT_SIZE 84

typedef struct
            {
              double  positionX                      ;
              double  positionY                      ;
              float   relTime                        ;
              char    text[EVENT_SIZE]               ;
            } SurfEventValues;

typedef struct
            {
              char              label    [LABEL_SIZE];
              SurfEventValues   values            [1];
            } SurfEvents;   

#define SIZE_OF_SURF_EVENT_ARRAY(NR_OF_EVENTS) \
        (NR_OF_EVENTS == 0)?\
        0\
        :\
        (sizeof(SurfEvents) + \
        ((NR_OF_EVENTS - 1) * sizeof(SurfEventValues)))

              




/* SURF-dataset "Static Values for Tpe-Calculation" */           /* new V3.0 */
/* RAN-Special !! */

#define SURF_TPE_STATICS_LABEL "TPE_STATICS"

typedef enum {
              TPE_NEVERCALCULATED,
              TPE_MUSTRECALCULATE,
              TPE_ISCALCULATED
             }TpeFlag;


typedef struct
            {
              char    label    [LABEL_SIZE];
              u_int   tpeFlag                        ;
              char    timeDateOfLastTpeCalculation[TIME_SIZE];
              double  ltncyHprMb                     ;
              double  ltncyNavHss                    ;
              double  initRoll                       ;
              double  initPtch                       ;
              double  initHve                        ;
              double  initYaw                        ;
              double  rollRateC                      ;
              double  ptchRateC                      ;
              double  hveRateC                       ;
              double  yawRateC                       ;
              double  lvrml                          ;
              double  lvrmw                          ;
              double  lvrmh                          ;
              double  shpFctr                        ;
              double  bwx                            ;
              double  bwy                            ;
              double  tmtDurn                        ;
              double  dTide                          ;
              double  Ss                             ;
              double  detect                         ;
              double  Ts                             ;
              double  svTrns                         ;
              double  reserve1                       ;
              double  reserve2                       ;
              double  reserve3                       ;
              double  reserve4                       ;
            } SurfTpeStatics;  





/* SURF-dataset "Free Six Data Descriptor"  */         /* new V3.0 */

typedef struct
            {
              char    descr[STRING_SIZE]          ;
            } SurfFreeSixDataDescr;   

#define SIZE_OF_SURF_SIX_ATTACHED_DESCR(NR_OF_SIX_ATTACHED_DATA) \
            (NR_OF_SIX_ATTACHED_DATA == 0)?\
            0\
            :\
            (sizeof(SurfFreeSixDataDescr) + \
            ((NR_OF_SIX_ATTACHED_DATA - 1) * sizeof(SurfFreeSixDataDescr)))
        




/* SURF-dataset "Free Sounding Data Descriptor"  */    /* new V3.0 */

typedef struct
            {
              char    descr[STRING_SIZE]          ;
            } SurfFreeSndgDataDescr;   

#define SIZE_OF_SURF_SNDG_ATTACHED_DESCR(NR_OF_SNDG_ATTACHED_DATA) \
            (NR_OF_SNDG_ATTACHED_DATA == 0)?\
            0\
            :\
            (sizeof(SurfFreeSndgDataDescr) + \
            ((NR_OF_SNDG_ATTACHED_DATA - 1) * sizeof(SurfFreeSndgDataDescr)))
        





/* SURF-dataset "Free Beam Data Descriptor"  */        /* new V3.0 */

typedef struct
            {
              char    descr[STRING_SIZE]          ;
            } SurfFreeBeamDataDescr;   

#define SIZE_OF_SURF_BEAM_ATTACHED_DESCR(NR_OF_BEAM_ATTACHED_DATA) \
            (NR_OF_BEAM_ATTACHED_DATA == 0)?\
            0\
            :\
            (sizeof(SurfFreeBeamDataDescr) + \
            ((NR_OF_BEAM_ATTACHED_DATA - 1) * sizeof(SurfFreeBeamDataDescr)))
        





/* SURF-dataset "Free SixAttached Data"  */           /* new V3.0 */

typedef double SurfFreeSixAttachedData;   

#define SIZE_OF_SURF_SIX_ATTACHED_DATA(NR_OF_SIX_ATTACHED_DATA) \
            (NR_OF_SIX_ATTACHED_DATA == 0)?\
            0\
            :\
            (sizeof(SurfFreeSixAttachedData) + \
            ((NR_OF_SIX_ATTACHED_DATA - 1) * sizeof(SurfFreeSixAttachedData)))
        





/* SURF-dataset "Free Text " */          

#define SURF_FREE_TEXT_LABEL "FREETEXT"
#define FREE_TEXT_BLOCK_SIZE        4

typedef struct
            {
              char    text[FREE_TEXT_BLOCK_SIZE]               ;
            } SurfFreeTextBlocks;

typedef struct
            {
              char              label    [LABEL_SIZE];
              SurfFreeTextBlocks blocks[1];
            } SurfFreeText;   

#define SIZE_OF_FREE_TEXT_ARRAY(NR_OF_FREE_TEXT_BLOCKS) \
        (NR_OF_FREE_TEXT_BLOCKS == 0)?\
        0\
        :\
        (sizeof(SurfFreeText) + \
        ((NR_OF_FREE_TEXT_BLOCKS - 1) * sizeof(SurfFreeTextBlocks)))
        





/* SURF-dataset " Vendor Text " */          

typedef struct
            {
              char    text[TEXT_SIZE]               ;
            } SurfVendorText;





/***************************************/
/*                                     */
/* SURF-elements describing SDA-files  */
/*                                     */
/***************************************/
              



/* SURF-dataset "Sounding-Data" */

  /* soundingFlag */
  
#define SF_DELETED                 1
#define SF_COURSE_MANIPULATED      2
#define SF_HEAVE_MANIPULATED       4
#define SF_ROLL_MANIPULATED        8
#define SF_PITCH_MANIPULATED      16
#define SF_CKEEL_MANIPULATED      32
#define SF_CMEAN_MANIPULATED      64
#define SF_SLOPE_KORRECTED       128
#define SF_FAN_PAT_1             256 /* 0 = full fan; 1 = split fan */
#define SF_FAN_PAT_2             512 /* 0 = port fan; 1 = star  fan */
#define SF_FAN_PAT_3            1024 /* 0 = norm.fan; 1 = ahead fan */
#define SF_ALL_BEAMS_DELETED    2048

/* FAN pattern */

#define SF_FAN_PAT_MASK         (SF_FAN_PAT_1+SF_FAN_PAT_2)

#define SF_FULL_FAN             0
#define SF_PORT_FAN             (SF_FAN_PAT_1)
#define SF_STAR_FAN             (SF_FAN_PAT_1+SF_FAN_PAT_2)

  
typedef struct
            {
              u_short soundingFlag                   ;
              u_short indexToAngle                   ;
              u_short indexToTransducer              ;
              u_short indexToCProfile                ;
              float   relTime                        ;
              float   relWay                         ;
              float   tide                           ;
              float   headingWhileTransmitting       ;
              float   heaveWhileTransmitting         ;
              float   rollWhileTransmitting          ;
              float   pitchWhileTransmitting         ;
              float   cKeel                          ;
              float   cMean                          ;
              float   dynChartZero                   ;
            } SurfSoundingData;  



/* SURF-dataset "Free SoundingAttached Data"  */             /* new V3.0 */   

typedef float SurfFreeSoundingAttachedData;                

#define SIZE_OF_SURF_SNDG_ATTACHED_DATA(NR_OF_SNDG_ATTACHED_DATA) \
            (NR_OF_SNDG_ATTACHED_DATA == 0)?\
            0\
            :\
            (sizeof(SurfFreeSoundingAttachedData) + \
            ((NR_OF_SNDG_ATTACHED_DATA - 1) * sizeof(SurfFreeSoundingAttachedData)))
        



             
  /* positionFlag */
  
#define PF_DELETED                 1



/* SURF-dataset "Center-Position" */

typedef struct
            {
              u_short positionFlag                   ;
              float   centerPositionX                ;
              float   centerPositionY                ;
              float   speed                          ;
            } SurfCenterPosition;
              



/* SURF-dataset "Position-Cep"  */                    /* new V3.0 */   

typedef float SurfPositionCepData;                





/* SURF-dataset "Single-Beam-Depth" */

/* depthFlag */
#define SB_DELETED                 1
#define SB_OBJECT                  2
#define SB_FRAC_LINE               4
#define SB_MAN_DATA                8
#define SB_TIDE_CORRECTED         16
#define SB_TIDE_MANIPULATED       32
#define SB_POSI_MANIPULATED       64
#define SB_DEPTH_MANIPULATED     128 /* Multibeam  */
#define SB_H_DEPTH_MANIPULATED   128 /* Singlebeam */
#define SB_M_DEPTH_MANIPULATED   256 /*     "      */
#define SB_L_DEPTH_MANIPULATED   512 /*     "      */
#define SB_DRAUGHT_CORRECTED    1024
#define SB_DEPTH_SUPPRESSED     2048
#define SB_REDUCED_FAN          4096 /* Multibeam  */
#define SB_TRANSDUCER_PLUS1     8192 /* Multibeam  */

typedef struct
            {
              u_short depthFlag                      ;
              float   travelTimeOfRay                ;
              float   depthHFreq                     ;
              float   depthMFreq                     ;
              float   depthLFreq                     ;
            } SurfSingleBeamDepth;
              



/* SURF-dataset "Multi-Beam-Depth" */

typedef struct
            {
              u_short depthFlag                      ;
              float   depth                          ;
              float   beamPositionAhead              ;
              float   beamPositionStar               ;
            } SurfMultiBeamDepth;
              



/* SURF-dataset "Multi-Beam-Travel-Time" */

typedef struct
            {
              float   travelTimeOfRay                ;
            } SurfMultiBeamTT;
              



/* SURF-dataset "Multi-Beam-Receive" */

typedef struct
            {
              float   headingWhileReceiving          ;
              float   heaveWhileReceiving            ;
            } SurfMultiBeamReceive;
              



/* SURF-dataset "Beam-Amplitudes" */                    /* new V2.0 */

typedef struct
            { 
              u_short beamAmplitude                  ;  /* new V2.0 */
            } SurfAmplitudes;
              



/* SURF-dataset "Extended-Beam-Amplitudes" */           /* new V2.0 */

typedef struct
            {
              float   mtau                           ;  /* new V2.0 */
              u_short nis                            ;
              u_short beamAmplitude                  ;
            } SurfExtendedAmplitudes;
              



/* SURF-dataset "Free Beamattached Data"  */            /* new V3.0 */

typedef float SurfFreeBeamAttachedData;               

#define SIZE_OF_SURF_BEAM_ATTACHED_DATA(NR_OF_BEAM_ATTACHED_DATA) \
            (NR_OF_BEAM_ATTACHED_DATA == 0)?\
            0\
            :\
            (sizeof(SurfFreeBeamAttachedData) + \
            ((NR_OF_BEAM_ATTACHED_DATA - 1) * sizeof(SurfFreeBeamAttachedData)))
        


/* SURF-dataset "RxSignalparameter" */                 /* new V2.0 */

typedef struct
            {
             float time;               /* scale : sec  */
             float gain;               /* scale : dB */
            }TvgRxSets;

typedef struct
            { 
             u_short bscatClass                     ;/* new V2.2 */
             u_short nrActualGainSets               ;
             float   rxGup                          ;
             float   rxGain                         ;
             float   ar                             ;
             TvgRxSets rxSets[1]                    ;
            } SurfSignalParameter;

#define SIZE_OF_SURF_SIGNAL_PARAMETER(NR_OF_SETS) \
        (NR_OF_SETS == 0)?\
        0\
        :\
        (sizeof(SurfSignalParameter) + \
        ((NR_OF_SETS - 1) * sizeof(TvgRxSets)))




/* SURF-dataset "TxSignalparameter" */              /* new V2.2 */

typedef struct
            {
             u_int  txBeamIndex   ;  /* Code of external Beamshapetab */
             float  txLevel       ;  /* scale : dB rel 1 uPa */
             float  txBeamAngle   ;  /* scale : rad */
             float  pulseLength   ;  /* scale : sec */
            }TxSets;

typedef struct
            { 
             TxSets  txSets[1]                      ;  /* new V2.0 */
            } SurfTxParameter;

#define SIZE_OF_SURF_TX_PARAMETER(NR_OF_SETS) \
        (NR_OF_SETS == 0)?\
        0\
        :\
        ((NR_OF_SETS) * sizeof(TxSets))




/* SURF-dataset "Signalamplitudes"  */
        /* wird nicht mehr benutzt ab V2.0 */

typedef struct
            {
              u_short amplitudesFlag                 ;
              u_short actualNrOfAmplitudes           ;
              float   maxAmplPosAstar                ;
              u_char  amplitudes[1]                  ;
            } SurfSignalAmplitudes;


#define SIZE_OF_SURF_SIGNAL_AMPLITUDES_ARRAY(NR_OF_AMPLITUDES) \
        (NR_OF_AMPLITUDES == 0)?\
        0\
        :\
        (sizeof(SurfSignalAmplitudes) + \
        ((NR_OF_AMPLITUDES - 1) * sizeof(u_char)))
        

/* SURF-dataset "Sidescandata"  */

typedef struct
            {
              u_int   sidescanFlag                   ;  /* new V2.0 */
              u_short actualNrOfSsDataPort           ;
              u_short actualNrOfSsDataStb            ;
              float   minSsPosPort                   ;
              float   minSsPosStb                    ;
              float   maxSsPosPort                   ;
              float   maxSsPosStb                    ;
              u_char  ssData[1]                      ;
            } SurfSidescanData;


#define SIZE_OF_SURF_SIDESCAN_DATA_ARRAY(NR_OF_AMPLITUDES) \
        (NR_OF_AMPLITUDES == 0)?\
        0\
        :\
        (sizeof(SurfSidescanData) + \
        ((NR_OF_AMPLITUDES - 1) * sizeof(u_char)))


        

/* SURF-dataset "TPE-values"  */                         /* new V3.0 */

typedef struct
            {
              float   depthTpe                       ;
              float   posTpe                         ;
              float   minDetectionVolumeTpe          ;
            } SurfTpeValues;

typedef SurfTpeValues SurfMultiBeamTpeValues;               

typedef SurfTpeValues SurfSingleBeamTpeValues;   

              



#endif /* ifndef _SURF */



