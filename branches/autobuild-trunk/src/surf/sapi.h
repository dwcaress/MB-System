/*-----------------------------------------------------------------------
/ H E A D E R K O P F
/ ------------------------------------------------------------------------
/ ------------------------------------------------------------------------
/  DATEINAME        : sapi.h
/  ERSTELLUNGSDATUM : 21.05.96
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
/ ERSTELLER : Peter Block    : SHD2
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
/ BESCHREIBUNG:       Definitions describing the "SURF-Format" 3
/                                               & SURF-API-lib V3.1.4
/ ------------------------------------------------------------------------
/
/ ------------------------------------------------------------------------
/ E N D E   D E S   K O P F E S
/ ------------------------------------------------------------------------
*/
/************************************************************************/




#ifndef _SAPI
#define _SAPI

#ifndef __SAPI__
/**
#ifndef _WIN32
#include <rpc/types.h>
#include <rpc/xdr.h>
#else
#include <xdr_win32.h>
#endif
**/

#include <rpc/types.h>
#include <rpc/xdr.h>

/*
   ALL DATA ARE SCALED IN MKS
   That means scalings are METER,SECONDS,RAD
   or derivates ..[m/sec],[1/sec]=[Hz]...

   -Relative Times are related to the profile starttime
   -Relative Ways are related to the profile startposition
   -All Positions are relative to the positionreference
   -Positions are scaled either in [m] or [rad] according to
    the presentationOfPosition-dataentry
   -Centerpositions are given for a virtual Shipreferencepoint
    (should be the "ship's turning point")
   -Positions in the Shipcoordinatesystem (ahead,astar) are
    also related to this point and scaled in [m]

   time/date-sets are presented in ASCII-Characters
      DDMMYYHHMMSS.NN<0x0>

   name-strings and Label-strings are presented in
   c-string-annotation    ABCDEFG<0x0>

*/



#define LABEL_SIZE          16
#define STRING_SIZE         16
#define TIME_SIZE           16


/***************************************/
/*                                     */
/* SURF-elements describing SIX-files  */
/*                                     */
/***************************************/


/* SURF-dataset "Globaldata" */

  /* Values of 'typeOfSounder' */

#define MANUAL_DATA              'M'
#define DIGITIZED_DATA           'D'
#define VERTICAL_SOUNDER         'V'
#define BOMA_TYPE_SOUNDER        'B'
#define FAN_TYPE_SOUNDER         'F'

  /* Values of 'presentationOfPosition' */

#define EASTING_NORTHING         'E' /* Values are scaled in [rad] */
#define X_Y                      'X' /* Values are scaled in [m]   */

/* correctedParameterFlags */

#define CP_TIDE_CORRECTED          1
#define CP_DRAUGHT_CORRECTED       2
#define CP_COURSE_MANIPULATED      4
#define CP_HEAVE_MANIPULATED       8
#define CP_ROLL_MANIPULATED       16
#define CP_PITCH_MANIPULATED      32
#define CP_CKEEL_MANIPULATED      64
#define CP_CMEAN_MANIPULATED     128
#define CP_SQUAT_CORRECTED      1024


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
              u_long  numberOfMeasuredSoundings              ;
              u_long  actualNumberOfSoundingSets             ;
              char    timeDateOfTideModification  [TIME_SIZE];
              char    timeDateOfDepthModification [TIME_SIZE];
              char    timeDateOfPosiModification  [TIME_SIZE];
              char    timeDateOfParaModification  [TIME_SIZE];
              u_long  correctedParameterFlags                ;
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
              CProfileValues  values              [1]; /*numberOfActualValues times*/
            } SurfCProfileTable;




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
              SurfPolygonValues values            [1]; /*numberOfPolygons times*/
            } SurfPolygons;






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
              SurfFreeTextBlocks blocks[1]; /* nrFreeTextBlocks times*/
            } SurfFreeText;





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
#define SF_FAN_PAT_1             256 /* 0 = full fan; 1 = split fan */
#define SF_FAN_PAT_2             512 /* 0 = port fan; 1 = star  fan */
#define SF_FAN_PAT_3            1024 /* 0 = norm.fan; 1 = ahead fan */



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


  /* positionFlag

     no entries yet*/




/* SURF-dataset "Center-Position" */

typedef struct
            {
              u_short positionFlag                   ;
              float   centerPositionX                ;
              float   centerPositionY                ;
              float   speed                          ;
            } SurfCenterPosition;




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




/* SURF-dataset "Beam-Amplitudes" */

typedef struct
            {
              u_short beamAmplitude                  ;
            } SurfAmplitudes;




/* SURF-dataset "Extended-Beam-Amplitudes" */

typedef struct
            {
              float   mtau                           ;
              u_short nis                            ;
              u_short beamAmplitude                  ;
            } SurfExtendedAmplitudes;




/* SURF-dataset "RxSignalparameter" */

typedef struct
            {
             float time;               /* scale : sec  */
             float gain;               /* scale : dB */
            }TvgRxSets;

typedef struct
            {
             u_short bscatClass                     ;/* neu V2.2 */
             u_short nrActualGainSets               ;
             float   rxGup                          ;
             float   rxGain                         ;
             float   ar                             ;
             TvgRxSets rxSets[1]                    ;
            } SurfSignalParameter;




/* SURF-dataset "TxSignalparameter" */

typedef struct
            {
             u_long txBeamIndex   ;  /* Code of external Beamshapetab */
             float  txLevel       ;  /* scale : dB rel 1 uPa */
             float  txBeamAngle   ;  /* scale : rad */
             float  pulseLength   ;  /* scale : sec */
            }TxSets;

typedef struct
            {
             TxSets  txSets[1]                      ;
            } SurfTxParameter;




/* SURF-dataset "Sidescandata"  */

typedef struct
            {
              u_long  sidescanFlag                   ;
              u_short actualNrOfSsDataPort           ;
              u_short actualNrOfSsDataStb            ;
              float   minSsTimePort                  ;
              float   minSsTimeStb                   ;
              float   maxSsTimePort                  ;
              float   maxSsTimeStb                   ;
              mb_u_char  ssData[1]                      ;/* nr of actual Data times
                                                         first Port then Stb */
            } SurfSidescanData;


#endif /* ifndef __SAPI__ */



/* FILE-handling functions */

extern void  SAPI_printAPIandSURFversion(void);

extern long  SAPI_open(char* surfDir,char* surfFile,long errorprint);
extern long  SAPI_nextSounding(long errorprint);
extern long  SAPI_rewind(long errorprint);
extern void  SAPI_close(void);

extern long SAPI_openIntoMemory(char* surfDir,char* surfFile,long errorprint);
extern long SAPI_writeBackFromMemory(char* surfDir,char* surfFile,long errorprint);

extern long SAPI_createSurfBody(long nrSoundings,long nrBeams,
                         long maxNrSidescanSamplesPerSounding,long errorprint);


/* Data from SIX(Index-)-File */

extern char* SAPI_getNameOfShip(void);
extern char* SAPI_getTypeOfSounder(void);
extern char* SAPI_getNameOfSounder(void);
extern long  SAPI_getNrSoundings(void);
extern long  SAPI_getNrBeams(void);
extern long  SAPI_posPresentationIsRad(void);
extern long  SAPI_getNrPositionsensors(void);
extern long  SAPI_getNrSoundvelocityProfiles(void);
extern long  SAPI_getNrEvents(void);
extern long  SAPI_getNrPolygonElements(void);

extern double SAPI_getAbsoluteStartTimeOfProfile(void);

extern long  SAPI_dataHaveHighFrequencyLayer(void);
extern long  SAPI_dataHaveMediumFrequencyLayer(void);
extern long  SAPI_dataHaveLowFrequencyLayer(void);

extern SurfGlobalData*               SAPI_getGlobalData(void);
extern SurfStatistics*               SAPI_getStatistics(void);
extern SurfPositionAnySensor*        SAPI_getPositionSensor(long nrSensor);
extern SurfEventValues*              SAPI_getEvent(long nrEvent);
extern SurfPolygons*                 SAPI_getPolygons(void);


/* Data from SDA(Massdata-)-File */

extern SurfSoundingData*             SAPI_getSoundingData(void);
extern SurfTransducerParameterTable* SAPI_getActualTransducerTable(void);
extern SurfMultiBeamAngleTable*      SAPI_getActualAngleTable(void);
extern SurfCProfileTable*            SAPI_getActualCProfileTable(void);

extern SurfCenterPosition*           SAPI_getCenterPosition(long nrPositionSensor);
extern SurfSingleBeamDepth*          SAPI_getSingleBeamDepth(void);
extern SurfMultiBeamDepth*           SAPI_getMultiBeamDepth(long beam);
extern SurfMultiBeamTT*              SAPI_getMultiBeamTraveltime(long beam);
extern SurfMultiBeamReceive*         SAPI_getMultiBeamReceiveParams(long beam);


/* Sidescan- & Backscatter-related Data from SDA(Massdata-)-File */

SurfAmplitudes*                      SAPI_getMultibeamBeamAmplitudes(long beam);
SurfExtendedAmplitudes*              SAPI_getMultibeamExtendedBeamAmplitudes(long beam);
SurfSignalParameter*                 SAPI_getMultibeamSignalParameters(void);
SurfTxParameter*                     SAPI_getMultibeamTransmitterParameters(int *nTxParams);
SurfSidescanData*                    SAPI_getSidescanData(void);



/* some simple routines (simple,but maybe slower than plain code !) */

  /* depthOverChartZero =  0 --> depth over normal 0
     depthOverChartZero =  1 --> depth over in SURF defined Chartzero */

extern long SAPI_getXYZfromMultibeamSounding(long nrBeam,long depthOverChartZero,
                                             double* north,double* east,double* depth);

  /* different Frequencylayers may be stored in one profile of vertical sounders */
                    /* LF < 15kHz < MF < 70 kHz < HF */

extern long SAPI_getXYZfromSinglebeamSoundingHF(long depthOverChartZero,
                                             double* north,double* east,double* depth);
extern long SAPI_getXYZfromSinglebeamSoundingMF(long depthOverChartZero,
                                             double* north,double* east,double* depth);
extern long SAPI_getXYZfromSinglebeamSoundingLF(long depthOverChartZero,
                                             double* north,double* east,double* depth);


#endif /* ifndef _SAPI */




