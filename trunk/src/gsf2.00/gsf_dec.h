/********************************************************************
 *
 * Module Name : GSF_DEC
 *
 * Author/Date : J. S. Byrne / 26 May 1994
 *
 * Description :
 *
 * Restrictions/Limitations :
 *
 * Change Descriptions :
 * who	when	  what
 * ---	----	  ----
 * hem  08/20/96  Added gsfDecodeSinglebeam.
 *
 * Classification : Unclassified
 *
 * References :
 *
 *
 * Copyright (C) Science Applications International Corp.
 ********************************************************************/

#ifndef _GSF_DEC_H_
   #define _GSF_DEC_H_

   #include "gsf.h"
   #include "gsf_ft.h"

   int OPTLK gsfDecodeHeader(gsfHeader *header, unsigned char *sptr);
   int OPTLK gsfDecodeSwathBathySummary(gsfSwathBathySummary *summ, unsigned char *sptr);
   int OPTLK gsfDecodeSwathBathymetryPing(gsfSwathBathyPing *ping, unsigned char *sptr, GSF_FILE_TABLE *ft, int handle, int record_size);
   int OPTLK gsfDecodeSoundVelocityProfile(gsfSVP *svp, GSF_FILE_TABLE *ft, unsigned char *sptr);
   int OPTLK gsfDecodeProcessingParameters(gsfProcessingParameters *param, GSF_FILE_TABLE *ft, unsigned char *sptr);
   int OPTLK gsfDecodeSensorParameters(gsfSensorParameters *param, GSF_FILE_TABLE *ft, unsigned char *sptr);
   int OPTLK gsfDecodeComment(gsfComment *comment, GSF_FILE_TABLE *ft, unsigned char *sptr);
   int OPTLK gsfDecodeHistory(gsfHistory *history, GSF_FILE_TABLE *ft, unsigned char *sptr);
   int OPTLK gsfDecodeNavigationError(gsfNavigationError *nav_error, unsigned char *sptr);
   int OPTLK gsfDecodeSinglebeam(gsfSingleBeamPing * ping, unsigned char *sptr, GSF_FILE_TABLE *ft, int handle, int record_size);

#endif
