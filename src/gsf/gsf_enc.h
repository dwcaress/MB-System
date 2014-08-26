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
 * who  when      what
 * ---  ----      ----
 * hem  08/20/96  Added gsfEncodeSinglebeam.
 * jsb  09/28/98  Added gsfEncodeHVNavigationError.
 *
 * Classification : Unclassified
 *
 * References :
 *
 *
 * copyright 2014 Leidos, Inc.
 * There is no charge to use the library, and it may be accessed at:
 * https://www.leidos.com/maritime/gsf.
 * This library may be redistributed and/or modified under the terms of
 * the GNU Lesser General Public License version 2.1, as published by the
 * Free Software Foundation.  A copy of the LGPL 2.1 license is included with
 * the GSF distribution and is avaialbe at: http://opensource.org/licenses/LGPL-2.1.
 *
 * Leidos, Inc. configuration manages GSF, and provides GSF releases. Users are
 * strongly encouraged to communicate change requests and change proposals to Leidos, Inc.
 *
 * This library is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE.
 *
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
   int OPTLK gsfEncodeAttitude(unsigned char *sptr, gsfAttitude * attitude);
   int OPTLK gsfSetDefaultScaleFactor(gsfSwathBathyPing *mb_ping);

#endif
