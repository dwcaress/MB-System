/********************************************************************
 *
 * Module Name : GSF_ENC
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
 * hem  08/20/96  Added gsfEncodeSinglebeam.
 * jsb  09/28/98  Added gsfEncodeHVNavigationError.
 *
 * Classification : Unclassified
 *
 * References :
 *
 *
 * Copyright (C) Science Applications International Corp.
 ********************************************************************/
#ifndef _GSF_ENC_H_
   #define _GSF_ENC_H_

   #include "gsf.h"
   #include "gsf_ft.h"

   int OPTLK gsfEncodeHeader(unsigned char *sptr, gsfHeader *header);
   int OPTLK gsfEncodeSwathBathySummary(unsigned char *sptr, gsfSwathBathySummary *sum);
   int OPTLK gsfEncodeSwathBathymetryPing(unsigned char *sptr, gsfSwathBathyPing *ping, GSF_FILE_TABLE *ft, int handle);
   int OPTLK gsfEncodeSoundVelocityProfile(unsigned char *sptr, gsfSVP *svp);
   int OPTLK gsfEncodeProcessingParameters(unsigned char *sptr, gsfProcessingParameters *pparam);
   int OPTLK gsfEncodeSensorParameters(unsigned char *sptr, gsfSensorParameters *sparam);
   int OPTLK gsfEncodeComment(unsigned char *sptr, gsfComment *comment);
   int OPTLK gsfEncodeHistory(unsigned char *sptr, gsfHistory *history);
   int OPTLK gsfEncodeNavigationError(unsigned char *sptr, gsfNavigationError *nav_error);
   int OPTLK gsfEncodeHVNavigationError(unsigned char *sptr, gsfHVNavigationError *hv_nav_error);
   int OPTLK gsfEncodeSinglebeam (unsigned char *sptr, gsfSingleBeamPing * ping);

#endif
