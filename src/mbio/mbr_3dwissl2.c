/*--------------------------------------------------------------------
 *    The MB-system:  mbr_3dwissl2.c  2/11/93
 *
 *    Copyright (c) 1993-2025 by
 *    David W. Caress (caress@mbari.org)
 *      Monterey Bay Aquarium Research Institute
 *      Moss Landing, California, USA
 *    Dale N. Chayes 
 *      Center for Coastal and Ocean Mapping
 *      University of New Hampshire
 *      Durham, New Hampshire, USA
 *    Christian dos Santos Ferreira
 *      MARUM
 *      University of Bremen
 *      Bremen Germany
 *     
 *    MB-System was created by Caress and Chayes in 1992 at the
 *      Lamont-Doherty Earth Observatory
 *      Columbia University
 *      Palisades, NY 10964
 *
 *    See README.md file for copying and redistribution conditions.
 *--------------------------------------------------------------------*/
/*
 * mbr_3dwissl2.c contains the functions for reading and writing
 * multibeam data in the MBF_3DWISSLP format.
 * These functions include:
 *   mbr_alm_3dwissl2  - allocate read/write memory
 *   mbr_dem_3dwissl2  - deallocate read/write memory
 *   mbr_rt_3dwissl2  - read and translate data
 *   mbr_wt_3dwissl2  - translate and write data
 *
 * Author:  D. W. Caress
 * Date:  December 27, 2013
 */

#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "mb_define.h"
#include "mb_format.h"
#include "mb_io.h"
#include "mb_status.h"
#include "mbsys_3ddwissl2.h"

//#define MBF_3DWISSLP_DEBUG 1 

/*--------------------------------------------------------------------*/
int mbr_info_3dwissl2
(
  int verbose,
  int *system,
  int *beams_bath_max,
  int *beams_amp_max,
  int *pixels_ss_max,
  char *format_name,
  char *system_name,
  char *format_description,
  int *numfile,
  int *filetype,
  bool *variable_beams,
  bool *traveltime,
  bool *beam_flagging,
  int *platform_source,
  int *nav_source,
  int *sensordepth_source,
  int *heading_source,
  int *attitude_source,
  int *svp_source,
  double *beamwidth_xtrack,
  double *beamwidth_ltrack,
  int *error
)
{
  if (verbose >= 2)
    {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
    }

  /* set format info parameters */
  *error = MB_ERROR_NO_ERROR;
  *system = MB_SYS_3DDWISSL1;
  *beams_bath_max = 0;
  *beams_amp_max = 0;
  *pixels_ss_max = 0;
  strncpy(format_name, "3DWISSL2", MB_NAME_LENGTH);
  strncpy(system_name, "3DDWISSL2", MB_NAME_LENGTH);
  strncpy(format_description,
    "Format name:          MBF_3DWISSL2\nInformal Description: 3D at Depth " "Second Generation Wide Swath Subsea Lidar (WiSSL2) SRIAT format\n" "           Attributes: 3D at Depth lidar, variable pulses, bathymetry and amplitude, \n" "                      binary, MBARI.\n",
    MB_DESCRIPTION_LENGTH);
  *numfile = 1;
  *filetype = MB_FILETYPE_NORMAL;
  *variable_beams = true;
  *traveltime = false;
  *beam_flagging = true;
  *platform_source = MB_DATA_NONE;
  *nav_source = MB_DATA_DATA;
  *sensordepth_source = MB_DATA_DATA;
  *heading_source = MB_DATA_DATA;
  *attitude_source = MB_DATA_DATA;
  *svp_source = MB_DATA_NONE;
  *beamwidth_xtrack = 0.02;
  *beamwidth_ltrack = 0.02;

  const int status = MB_SUCCESS;

  if (verbose >= 2)
    {
    fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       system:             %d\n", *system);
    fprintf(stderr, "dbg2       beams_bath_max:     %d\n", *beams_bath_max);
    fprintf(stderr, "dbg2       beams_amp_max:      %d\n", *beams_amp_max);
    fprintf(stderr, "dbg2       pixels_ss_max:      %d\n", *pixels_ss_max);
    fprintf(stderr, "dbg2       format_name:        %s\n", format_name);
    fprintf(stderr, "dbg2       system_name:        %s\n", system_name);
    fprintf(stderr, "dbg2       format_description: %s\n", format_description);
    fprintf(stderr, "dbg2       numfile:            %d\n", *numfile);
    fprintf(stderr, "dbg2       filetype:           %d\n", *filetype);
    fprintf(stderr, "dbg2       variable_beams:     %d\n", *variable_beams);
    fprintf(stderr, "dbg2       traveltime:         %d\n", *traveltime);
    fprintf(stderr, "dbg2       beam_flagging:      %d\n", *beam_flagging);
    fprintf(stderr, "dbg2       platform_source:    %d\n", *platform_source);
    fprintf(stderr, "dbg2       nav_source:         %d\n", *nav_source);
    fprintf(stderr, "dbg2       sensordepth_source: %d\n", *sensordepth_source);
    fprintf(stderr, "dbg2       heading_source:     %d\n", *heading_source);
    fprintf(stderr, "dbg2       attitude_source:      %d\n", *attitude_source);
    fprintf(stderr, "dbg2       svp_source:         %d\n", *svp_source);
    fprintf(stderr, "dbg2       beamwidth_xtrack:   %f\n", *beamwidth_xtrack);
    fprintf(stderr, "dbg2       beamwidth_ltrack:   %f\n", *beamwidth_ltrack);
    fprintf(stderr, "dbg2       error:              %d\n", *error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:         %d\n", status);
    }

  return status;
} /* mbr_info_3dwissl2 */
/*--------------------------------------------------------------------*/
int mbr_alm_3dwissl2
(
  int verbose,
  void *mbio_ptr,
  int *error
)
{
  if (verbose >= 2)
    {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
    fprintf(stderr, "dbg2       mbio_ptr:   %p\n", (void *)mbio_ptr);
    }

  /* check for non-null pointers */
  assert(mbio_ptr != NULL);

  /* get pointer to mbio descriptor */
  struct mb_io_struct *mb_io_ptr = (struct mb_io_struct *)mbio_ptr;

  /* allocate memory for data structure */
  const int status = mbsys_3ddwissl2_alloc(verbose, mbio_ptr, &mb_io_ptr->store_data, error);

  /* get pointer to mbio descriptor */
  mb_io_ptr = (struct mb_io_struct *)mbio_ptr;

  if (verbose >= 2)
    {
    fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       error:      %d\n", *error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:  %d\n", status);
    }

  return status;
} /* mbr_alm_3dwissl2 */
/*--------------------------------------------------------------------*/
int mbr_dem_3dwissl2
(
  int verbose,
  void *mbio_ptr,
  int *error
)
{
  if (verbose >= 2)
    {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
    fprintf(stderr, "dbg2       mbio_ptr:   %p\n", (void *)mbio_ptr);
    }

  /* check for non-null pointers */
  assert(mbio_ptr != NULL);

  /* get pointer to mbio descriptor */
  struct mb_io_struct *mb_io_ptr = (struct mb_io_struct *)mbio_ptr;

  int status = MB_SUCCESS;

  /* deallocate reading/writing buffer */
  if ((mb_io_ptr->data_structure_size > 0) && (mb_io_ptr->raw_data != NULL))
    {
    status = mb_freed(verbose, __FILE__, __LINE__, (void **)(&mb_io_ptr->raw_data), error);
    mb_io_ptr->raw_data = NULL;
    mb_io_ptr->data_structure_size = 0;
    }

  /* deallocate memory  */
  status = mbsys_3ddwissl2_deall(verbose, mbio_ptr, &mb_io_ptr->store_data, error);

  if (verbose >= 2)
    {
    fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       error:      %d\n", *error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:  %d\n", status);
    }

  return status;
} /* mbr_dem_3dwissl2 */
/*--------------------------------------------------------------------*/
int mbr_3dwissl2_rd_data
(
  int verbose,
  void *mbio_ptr,
  void *store_ptr,
  int *error
)
{
  if (verbose >= 2)
    {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
    fprintf(stderr, "dbg2       mbio_ptr:   %p\n", (void *)mbio_ptr);
    fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
    }

  /* check for non-null pointers */
  assert(mbio_ptr != NULL);
  assert(store_ptr != NULL);
  
  /* get pointer to mbio descriptor */
  struct mb_io_struct *mb_io_ptr = (struct mb_io_struct *)mbio_ptr;
  struct mbsys_3ddwissl2_struct *store = (struct mbsys_3ddwissl2_struct *)store_ptr;

  /* set file position */
  mb_io_ptr->file_pos = ftell(mb_io_ptr->mbfp);

  /* set status */
  int status = MB_SUCCESS;
  *error = MB_ERROR_NO_ERROR;

  /* allocate read buffer to at least handle the file header if needed */
  if (mb_io_ptr->data_structure_size < (size_t)(SRIAT_RECORD_SIZE_FILEHEADER))
		{
		if ((status = mb_reallocd(verbose, __FILE__, __LINE__, 
							(size_t)SRIAT_RECORD_SIZE_FILEHEADER, 
							(void **)(&mb_io_ptr->raw_data), error)) == MB_SUCCESS)
			{
			mb_io_ptr->data_structure_size = (size_t)SRIAT_RECORD_SIZE_FILEHEADER;
			}
		}
  char *buffer = mb_io_ptr->raw_data;
  
  /* read bytes until the beginning of a data record is found using the sync word */
	unsigned int SyncWord = 0;
	unsigned char PacketID = 0;
	unsigned char Version = 0;
	unsigned int SizeBytes = 0;
	size_t read_len = 10;
	int read_index = 0;
	int skip = 0;
	bool done = false;
	while (!done)
	  {
//fprintf(stderr, "read_len:%zu read_index:%d %02hhx:%02hhx:%02hhx:%02hhx:%02hhx:%02hhx:%02hhx:%02hhx:%02hhx:%02hhx \n", 
//read_len, read_index, 
//buffer[0], buffer[1], buffer[2], buffer[3], buffer[4], 
//buffer[5], buffer[6], buffer[7], buffer[8], buffer[9]);
	  status = mb_fileio_get(verbose, mbio_ptr, &buffer[read_index], &read_len, error);
//fprintf(stderr, "status:%d error:%d        %02hhx:%02hhx:%02hhx:%02hhx:%02hhx:%02hhx:%02hhx:%02hhx:%02hhx:%02hhx \n", 
//status, *error, 
//buffer[0], buffer[1], buffer[2], buffer[3], buffer[4], 
//buffer[5], buffer[6], buffer[7], buffer[8], buffer[9]);
	  if (status == MB_SUCCESS) 
	  	{
			int index = 0;
			mb_get_binary_int(true, (void *)&buffer[index], &SyncWord); index += 4;
			PacketID = buffer[index]; index += 1;
			Version = buffer[index]; index += 1;
			mb_get_binary_int(true, (void *)&buffer[index], &SizeBytes); index += 4;
//fprintf(stderr, "SyncWord: %d %x  SRIAT_SYNC_WORD: %d %x\n", SyncWord, SyncWord, SRIAT_SYNC_WORD, SRIAT_SYNC_WORD);
	  	if (SyncWord == SRIAT_SYNC_WORD)
	  		{
	  		done = true;
	  		}
	  	else 
	  		{
	  		read_index = 9;
	  		read_len = 1;
	  		skip++;
	  		for (int i=0; i<9; i++)
	  			buffer[i] = buffer[i+1];
	  		}
	  	}
	  else
	  	{
	  	done = true;
	  	}
	  }
//if (status == MB_SUCCESS)
//fprintf(stderr, "%s:%d:%s:  skip:%d status:%d error:%d    SyncWord:%x PacketID:%d Version:%d SizeBytes:%d\n", 
//__FILE__, __LINE__, __FUNCTION__, skip, status, *error, SyncWord, PacketID, Version, SizeBytes);

	/* if needed allocate read buffer to handle the entire data section
		and also arrays to hole the unpacked data arrays */
	if (status == MB_SUCCESS && mb_io_ptr->data_structure_size < (size_t)(SizeBytes))
		{
		if ((status = mb_reallocd(verbose, __FILE__, __LINE__, 
							(size_t)SizeBytes, 
							(void **)(&mb_io_ptr->raw_data), error)) == MB_SUCCESS)
			{
			mb_io_ptr->data_structure_size = (size_t)SizeBytes;
			buffer = mb_io_ptr->raw_data;
			}
		else 
			{
			done = MB_YES;
			store->kind = MB_DATA_NONE;
			}
		}


	/* If valid record start found, read the record */
	if (status != MB_SUCCESS)
		{
		store->kind = MB_DATA_NONE;
		}
		
	else if (PacketID == SRIAT_RECORD_ID_FILEHEADER)
		{

	  read_index = 10;
	  size_t read_len = (size_t)(SRIAT_RECORD_SIZE_FILEHEADER - read_index);
	  status = mb_fileio_get(verbose, mbio_ptr, &buffer[read_index], &read_len, error);
		
		if (status == MB_SUCCESS) 
			{
	  	struct mbsys_3ddwissl2_sriat_fileheader_struct *fileheader = &store->fileheader;
	  	
			int index = 0;
			mb_get_binary_int(true, (void *)&buffer[index], &fileheader->SyncWord); index += 4;
			fileheader->PacketID = buffer[index]; index += 1;
			fileheader->Version = buffer[index]; index += 1;
			mb_get_binary_int(true, (void *)&buffer[index], &fileheader->SizeBytes); index += 4;
				
			mb_get_binary_int(true, (void *)&buffer[index], &(fileheader->ScanSizeBytes)); index += 4;
			mb_get_binary_int(true, (void *)&buffer[index], &(fileheader->TimeStart_Sec)); index += 4;
			mb_get_binary_int(true, (void *)&buffer[index], &(fileheader->TimeStart_nSec)); index += 4;
			mb_get_binary_int(true, (void *)&buffer[index], &(fileheader->TimeEnd_Sec)); index += 4;
			mb_get_binary_int(true, (void *)&buffer[index], &(fileheader->TimeEnd_nSec)); index += 4;
			fileheader->SL_GEN = buffer[index]; index++;
			fileheader->SL_Letter = buffer[index]; index++;
			fileheader->SL_X = buffer[index]; index++;
			fileheader->nPtsToAverage = buffer[index]; index++;
			memcpy(fileheader->cJobName, &(buffer[index]), (size_t)24); index += 24;
			memcpy(fileheader->cScanPos, &(buffer[index]), (size_t)24); index += 24;
			memcpy(fileheader->cfileTag, &(buffer[index]), (size_t)24); index += 24;
			mb_get_binary_short(true, (void *)&buffer[index], &(fileheader->nScanNum)); index += 2;
	  
	  	mb_get_binary_int(true, (void *)&buffer[index], &(fileheader->AzCmdStart)); index += 4;
			mb_get_binary_int(true, (void *)&buffer[index], &(fileheader->AzCmdEnd)); index += 4;
	
			mb_get_binary_int(true, (void *)&buffer[index], &(fileheader->rawbit1)); index += 4;
			fileheader->nPtsPerScanLine = fileheader->rawbit1 & 0x3FFF;
			fileheader->nScanLinesPerScan = (fileheader->rawbit1 >> 14) & 0xFFF;
			fileheader->Spare1 = fileheader->rawbit1 >> 26;
			
			mb_get_binary_int(true, (void *)&buffer[index], &(fileheader->rawbit2)); index += 4;
			fileheader->nPtsPerLine = fileheader->rawbit2 & 0x3FFF;
			fileheader->Mode = (fileheader->rawbit2 >> 14) & 0x7;
			fileheader->nTPtsPerScanLine = (fileheader->rawbit2 >> 17) & 0x3FFF;
			fileheader->bHaveThermal = fileheader->rawbit2 >> 31;
	  
			mb_get_binary_int(true, (void *)&buffer[index], &(fileheader->ShotCnt)); index += 4;
			mb_get_binary_short(true, (void *)&buffer[index], &(fileheader->WaterSalinity_psu)); index += 2;
			mb_get_binary_short(true, (void *)&buffer[index], &(fileheader->WaterPressure_dbar)); index += 2;
			
			mb_get_binary_int(true, (void *)&buffer[index], &(fileheader->rawbit3)); index += 4;
			fileheader->WaterTemperature_C = fileheader->rawbit3 & 0x1FFF;
			fileheader->PRF_Hz = fileheader->rawbit3 >> 13;
		
			fileheader->DigitizerTemperature_C = buffer[index]; index++;
			mb_get_binary_float(true, (void *)&buffer[index], &(fileheader->RScale_m_per_cnt)); index += 4;
			mb_get_binary_short(true, (void *)&buffer[index], &(fileheader->ThBinStart_cnt)); index += 2;
			mb_get_binary_short(true, (void *)&buffer[index], &(fileheader->ThBinEnd_cnts)); index += 2;
			fileheader->TempAzCnt = buffer[index]; index++;
			fileheader->TempRowCnt = buffer[index]; index++;
		
			mb_get_binary_int(true, (void *)&buffer[index], &(fileheader->rawbit4)); index += 4;
			fileheader->TempRCnt_av2 = fileheader->rawbit4 & 0xFF;
			fileheader->TempRCnt_av4 = (fileheader->rawbit4 >> 8) & 0xFF;
			fileheader->TempRCnt_av8 = (fileheader->rawbit4 >> 16) & 0xFF;
			fileheader->TempRCnt_av16 = fileheader->rawbit4 >> 24;
		
			mb_get_binary_short(true, (void *)&buffer[index], &(fileheader->ScannerShift_mDeg)); index += 2;
			mb_get_binary_float(true, (void *)&buffer[index], &(fileheader->Shift_m[0])); index += 4;
			mb_get_binary_float(true, (void *)&buffer[index], &(fileheader->Shift_m[1])); index += 4;
			mb_get_binary_float(true, (void *)&buffer[index], &(fileheader->Shift_m[2])); index += 4;
			mb_get_binary_float(true, (void *)&buffer[index], &(fileheader->Rotate_deg[0])); index += 4;
			mb_get_binary_float(true, (void *)&buffer[index], &(fileheader->Rotate_deg[1])); index += 4;
			mb_get_binary_float(true, (void *)&buffer[index], &(fileheader->Rotate_deg[2])); index += 4;
			memcpy(fileheader->EC_Version, &(buffer[index]), (size_t)4); index += 4;
			memcpy(fileheader->InstaCloud_Version, &(buffer[index]), (size_t)4); index += 4;
			mb_get_binary_short(true, (void *)&buffer[index], &(fileheader->ElDeg_cnts)); index += 2;
//fprintf(stderr, "%s:%d:%s: index:%d\n", __FILE__, __LINE__, __FUNCTION__, index);

#ifndef MBF_3DWISSLP_DEBUG
			if (verbose >= 5) 
#endif
				{
				fprintf(stderr, "\ndbg5  Values read in MBIO function <%s>\n", __func__);
				fprintf(stderr, "dbg5       Fileheader Record:\n");
				fprintf(stderr, "dbg5       fileheader->SyncWord:                    %u\n", fileheader->SyncWord);
				fprintf(stderr, "dbg5       fileheader->PacketID:                    %u\n", fileheader->PacketID);
				fprintf(stderr, "dbg5       fileheader->Version:                     %u\n", fileheader->Version);
				fprintf(stderr, "dbg5       fileheader->SizeBytes:                   %u\n", fileheader->SizeBytes);
				fprintf(stderr, "dbg5       fileheader->ScanSizeBytes:               %u\n", fileheader->ScanSizeBytes);
				fprintf(stderr, "dbg5       fileheader->TimeStart_Sec:               %d\n", fileheader->TimeStart_Sec);
				fprintf(stderr, "dbg5       fileheader->TimeStart_nSec:              %d\n", fileheader->TimeStart_nSec);
				double time_d = fileheader->TimeStart_Sec + 1.0e-9 * fileheader->TimeStart_nSec;
				int time_i[7];
				mb_get_date(verbose, time_d, time_i);
				fprintf(stderr, "dbg5       TimeStart timestamp:                     %.6f %4.4d/%2.2d/%2.2d-%2.2d:%2.2d:%2.2d.%6.6d\n", time_d, time_i[0], time_i[1], time_i[2], time_i[3], time_i[4], time_i[5], time_i[6]);
				fprintf(stderr, "dbg5       fileheader->TimeEnd_Sec:                 %d\n", fileheader->TimeEnd_Sec);
				fprintf(stderr, "dbg5       fileheader->TimeEnd_nSec:                %d\n", fileheader->TimeEnd_nSec);
				time_d = fileheader->TimeEnd_Sec + 1.0e-9 * fileheader->TimeEnd_nSec;
				mb_get_date(verbose, time_d, time_i);
				fprintf(stderr, "dbg5       TimeEnd timestamp:                       %.6f %4.4d/%2.2d/%2.2d-%2.2d:%2.2d:%2.2d.%6.6d\n", time_d, time_i[0], time_i[1], time_i[2], time_i[3], time_i[4], time_i[5], time_i[6]);
				fprintf(stderr, "dbg5       fileheader->SL_GEN:                      %u\n", fileheader->SL_GEN);
				fprintf(stderr, "dbg5       fileheader->SL_Letter:                   %u\n", fileheader->SL_Letter);
				fprintf(stderr, "dbg5       fileheader->SL_X:                        %u\n", fileheader->SL_X);
				fprintf(stderr, "dbg5       fileheader->nPtsToAverage:               %u\n", fileheader->nPtsToAverage);
				fprintf(stderr, "dbg5       fileheader->cJobName:                    %s\n", fileheader->cJobName);
				fprintf(stderr, "dbg5       fileheader->cScanPos:                    %s\n", fileheader->cScanPos);
				fprintf(stderr, "dbg5       fileheader->cfileTag:                    %s\n", fileheader->cfileTag);
				fprintf(stderr, "dbg5       fileheader->nScanNum:                    %u\n", fileheader->nScanNum);
				fprintf(stderr, "dbg5       fileheader->AzCmdStart:                  %u\n", fileheader->AzCmdStart);
				fprintf(stderr, "dbg5       fileheader->AzCmdEnd:                    %u\n", fileheader->AzCmdEnd);
				fprintf(stderr, "dbg5       fileheader->nScanNum:                    %u\n", fileheader->nScanNum);
				fprintf(stderr, "dbg5       fileheader->rawbit1:                     %u\n", fileheader->rawbit1);
				fprintf(stderr, "dbg5       -fileheader->nPtsPerScanLine:             %u\n", fileheader->nPtsPerScanLine);
				fprintf(stderr, "dbg5       -fileheader->nScanLinesPerScan:           %u\n", fileheader->nScanLinesPerScan);
				fprintf(stderr, "dbg5       -fileheader->Spare1:                      %u\n", fileheader->Spare1);
				fprintf(stderr, "dbg5       fileheader->rawbit2:                     %u\n", fileheader->rawbit2);
				fprintf(stderr, "dbg5       -fileheader->nPtsPerLine:                 %u\n", fileheader->nPtsPerLine);
				fprintf(stderr, "dbg5       -fileheader->Mode:                        %u\n", fileheader->Mode);
				fprintf(stderr, "dbg5       -fileheader->nTPtsPerScanLine:            %u\n", fileheader->nTPtsPerScanLine);
				fprintf(stderr, "dbg5       -fileheader->bHaveThermal:                %u\n", fileheader->bHaveThermal);
				fprintf(stderr, "dbg5       fileheader->ShotCnt:                     %u\n", fileheader->ShotCnt);
				fprintf(stderr, "dbg5       fileheader->WaterSalinity_psu:           %u  %.3f\n", fileheader->WaterSalinity_psu, fileheader->WaterSalinity_psu * 42.0 / 65535.0 - 2.0);
				fprintf(stderr, "dbg5       fileheader->WaterPressure_dbar:          %u\n", fileheader->WaterPressure_dbar);
				fprintf(stderr, "dbg5       fileheader->rawbit3:                     %u\n", fileheader->rawbit3);
				fprintf(stderr, "dbg5       -fileheader->WaterTemperature_C:          %u  %.3f\n", fileheader->WaterTemperature_C, fileheader->WaterTemperature_C * 37.0 / 8191.0 - 2.0);
				fprintf(stderr, "dbg5       -fileheader->PRF_Hz:                      %u\n", fileheader->PRF_Hz);
				fprintf(stderr, "dbg5       fileheader->DigitizerTemperature_C:      %u  %.3f\n", fileheader->DigitizerTemperature_C, fileheader->DigitizerTemperature_C * 100.0 / 255.0);
				fprintf(stderr, "dbg5       fileheader->RScale_m_per_cnt:            %f\n", fileheader->RScale_m_per_cnt);
				fprintf(stderr, "dbg5       fileheader->ThBinStart_cnt:              %u\n", fileheader->ThBinStart_cnt);
				fprintf(stderr, "dbg5       fileheader->ThBinEnd_cnts:               %u\n", fileheader->ThBinEnd_cnts);
				fprintf(stderr, "dbg5       fileheader->TempAzCnt:                   %u\n", fileheader->TempAzCnt);
				fprintf(stderr, "dbg5       fileheader->TempRowCnt:                  %u\n", fileheader->TempRowCnt);
				fprintf(stderr, "dbg5       fileheader->rawbit4:                     %u\n", fileheader->rawbit4);
				fprintf(stderr, "dbg5       -fileheader->TempRCnt_av2:                %u\n", fileheader->TempRCnt_av2);
				fprintf(stderr, "dbg5       -fileheader->TempRCnt_av4:                %u\n", fileheader->TempRCnt_av4);
				fprintf(stderr, "dbg5       -fileheader->TempRCnt_av8:                %u\n", fileheader->TempRCnt_av8);
				fprintf(stderr, "dbg5       -fileheader->TempRCnt_av16:               %u\n", fileheader->TempRCnt_av16);
				fprintf(stderr, "dbg5       fileheader->ScannerShift_mDeg:           %u\n", fileheader->ScannerShift_mDeg);
				fprintf(stderr, "dbg5       fileheader->Shift_m[0]:                  %f\n", fileheader->Shift_m[0]);
				fprintf(stderr, "dbg5       fileheader->Shift_m[1]:                  %f\n", fileheader->Shift_m[1]);
				fprintf(stderr, "dbg5       fileheader->Shift_m[2]:                  %f\n", fileheader->Shift_m[2]);
				fprintf(stderr, "dbg5       fileheader->Rotate_deg[0]:               %f\n", fileheader->Rotate_deg[0]);
				fprintf(stderr, "dbg5       fileheader->Rotate_deg[1]:               %f\n", fileheader->Rotate_deg[1]);
				fprintf(stderr, "dbg5       fileheader->Rotate_deg[2]:               %f\n", fileheader->Rotate_deg[2]);
				fprintf(stderr, "dbg5       fileheader->EC_Version:                  %d.%d.%d.%d\n", 
								fileheader->EC_Version[0], fileheader->EC_Version[1], 
								fileheader->EC_Version[2], fileheader->EC_Version[3]);
				fprintf(stderr, "dbg5       fileheader->InstaCloud_Version:          %d.%d.%d.%d\n", 
								fileheader->InstaCloud_Version[0], fileheader->InstaCloud_Version[1], 
								fileheader->InstaCloud_Version[2], fileheader->InstaCloud_Version[3]);
				fprintf(stderr, "dbg5       fileheader->ElDeg_cnts:                  %d  %.3f\n", fileheader->ElDeg_cnts, fileheader->ElDeg_cnts * 90.0 / 65535.0);
				}
				
			fileheader->SizeBytes = SRIAT_RECORD_SIZE_FILEHEADER;
			store->kind = MB_DATA_PARAMETER;
			done = true;
			}
		}

	else if (PacketID == SRIAT_RECORD_ID_COMMENT)
  	{

	  read_index = 10;
	  size_t read_len = (size_t)(SizeBytes - read_index);
	  status = mb_fileio_get(verbose, mbio_ptr, &buffer[read_index], &read_len, error);
		
		if (status == MB_SUCCESS) 
			{
  		struct mbsys_3ddwissl2_comment_struct *comment = &store->comment;
	  	
			int index = 0;
			
			mb_get_binary_int(true, (void *)&buffer[index], &comment->SyncWord); index += 4;
			comment->PacketID = buffer[index]; index += 1;
			comment->Version = buffer[index]; index += 1;
			mb_get_binary_int(true, (void *)&buffer[index], &comment->SizeBytes); index += 4;
					
			mb_get_binary_short(true, (void *)&buffer[index], &(comment->comment_len)); index += 2;
			memcpy(comment->comment, &(buffer[index]), (size_t)comment->comment_len); index += comment->comment_len;

			/* print out contents of the data record */
#ifndef MBF_3DWISSLP_DEBUG
			if (verbose >= 5) 
#endif
				{
				fprintf(stderr, "\ndbg5  Values read in MBIO function <%s>\n", __func__);
				fprintf(stderr, "dbg5       Comment Record:\n");
				fprintf(stderr, "dbg5       comment->SyncWord:                       %u\n", comment->SyncWord);
				fprintf(stderr, "dbg5       comment->PacketID:                       %u\n", comment->PacketID);
				fprintf(stderr, "dbg5       comment->Version:                        %u\n", comment->Version);
				fprintf(stderr, "dbg5       comment->SizeBytes:                      %u\n", comment->SizeBytes);
				fprintf(stderr, "dbg5       comment->comment_len:                    %u\n", comment->comment_len);
				fprintf(stderr, "dbg5       comment->comment:                        %s\n", comment->comment);
				}
				
			store->kind = MB_DATA_COMMENT;
			done = true;
			}
		}

	else if (PacketID == SRIAT_RECORD_ID_RANGE)
		{

	  read_index = 10;
	  size_t read_len = (size_t)(SizeBytes - read_index);
	  status = mb_fileio_get(verbose, mbio_ptr, &buffer[read_index], &read_len, error);
		
		if (status == MB_SUCCESS) 
			{
	  	struct mbsys_3ddwissl2_mbarirange_struct *mbarirange = &store->mbarirange;
	  	store->bathymetry_calculated = false;
	  	
			int index = 0;
			unsigned int rawbits = 0;
			
			mb_get_binary_int(true, (void *)&buffer[index], &mbarirange->SyncWord); index += 4;
			mbarirange->PacketID = buffer[index]; index += 1;
			mbarirange->Version = buffer[index]; index += 1;
			mb_get_binary_int(true, (void *)&buffer[index], &mbarirange->SizeBytes); index += 4;
					
			mb_get_binary_short(true, (void *)&buffer[index], &(mbarirange->HdrSizeBytes)); index += 2;
			mb_get_binary_int(true, (void *)&buffer[index], &(mbarirange->TimeStart_Sec)); index += 4;
			mb_get_binary_int(true, (void *)&buffer[index], &(mbarirange->TimeStart_nSec)); index += 4;
			mb_get_binary_short(true, (void *)&buffer[index], &(mbarirange->NumPtsRow)); index += 2;
			mb_get_binary_short(true, (void *)&buffer[index], &(mbarirange->NumPtsPkt)); index += 2;
			mb_get_binary_int(true, (void *)&buffer[index], &(mbarirange->LineLaserPower)); index += 4;
		
			mb_get_binary_int(true, (void *)&buffer[index], &(rawbits)); index += 4;
			mbarirange->PRF_Hz = rawbits & 0x7FFFF;
			int Spare1 = (rawbits >> 19) & 0x7F;
			mbarirange->Points_per_LOS = (rawbits >> 26) & 0x3;
			mbarirange->ScannerType = rawbits >> 28;
			
			mb_get_binary_short(true, (void *)&buffer[index], &(mbarirange->lineAccelX)); index += 2;
			mb_get_binary_short(true, (void *)&buffer[index], &(mbarirange->lineAccelY)); index += 2;
			mb_get_binary_short(true, (void *)&buffer[index], &(mbarirange->lineAccelZ)); index += 2;
			mb_get_binary_short(true, (void *)&buffer[index], &(mbarirange->lineIndex)); index += 2;
			mb_get_binary_short(true, (void *)&buffer[index], &(mbarirange->RowNumber)); index += 2;
			mb_get_binary_short(true, (void *)&buffer[index], &(mbarirange->SHGAmplitudeAv)); index += 2;
  
			mb_get_binary_int(true, (void *)&buffer[index], &(rawbits)); index += 4;
			mbarirange->R_Max = rawbits & 0xFFFFF;
			mbarirange->I_Max = rawbits >> 20;
	 
			mb_get_binary_int(true, (void *)&buffer[index], &(rawbits)); index += 4;
			mbarirange->R_Auto = rawbits & 0xFFFFF;
			mbarirange->I_Auto = rawbits >> 20;
	 
			mb_get_binary_int(true, (void *)&buffer[index], &(rawbits)); index += 4;
			mbarirange->R_Mode = rawbits & 0xFFFFF;
			mbarirange->I_Mode = rawbits >> 20;
	
			mbarirange->I_Good = (unsigned char) buffer[index]; index++;
			mbarirange->I_Low = (unsigned char) buffer[index]; index++;
			mbarirange->I_High = (unsigned char) buffer[index]; index++;
			mbarirange->I_Spare = (unsigned char) buffer[index]; index++;
	
			mb_get_binary_int(true, (void *)&buffer[index], &(rawbits)); index += 4;
			mbarirange->R_offset = rawbits & 0xFFFFF;
			mbarirange->I_offset = rawbits >> 20;
			
			mb_get_binary_int(true, (void *)&buffer[index], &(mbarirange->AZ_offset)); index += 4;
			
			mb_get_binary_int(true, (void *)&buffer[index], &(rawbits)); index += 4;
			int R_nbits = rawbits & 0x1F;
			int I_nbits = (rawbits >> 5) & 0xF;
			int AZ_nbits = (rawbits >> 9) & 0x1F;
			int Spare2 = rawbits >> 14;

//fprintf(stderr, "%s:%d:%s: index:%d\n", __FILE__, __LINE__, __FUNCTION__, index);

			/* scanline timestamp */
			double time_d = mbarirange->TimeStart_Sec + 1.0e-9 * mbarirange->TimeStart_nSec;

			/* fix scanline timestamp */
	  	struct mbsys_3ddwissl2_sriat_fileheader_struct *fileheader = &store->fileheader;
			double start_time_d = fileheader->TimeStart_Sec + 1.0e-9 * fileheader->TimeStart_nSec;
			double end_time_d = fileheader->TimeEnd_Sec + 1.0e-9 * fileheader->TimeEnd_nSec;
			double dtime = (end_time_d - start_time_d) / (fileheader->nScanLinesPerScan - 1);
			time_d = start_time_d + dtime * mbarirange->RowNumber;
   		mbarirange->TimeStart_Sec = (int) floor(time_d);
   		mbarirange->TimeStart_nSec = (int)(1.0e9 * (time_d - mbarirange->TimeStart_Sec));

#ifndef MBF_3DWISSLP_DEBUG
			if (verbose >= 5) 
#endif
				{
				fprintf(stderr, "\ndbg5  Values read in MBIO function <%s>\n", __func__);
				fprintf(stderr, "dbg5       SRIAT Range Record:\n");
				fprintf(stderr, "dbg5       mbarirange->SyncWord:                    %u\n", mbarirange->SyncWord);
				fprintf(stderr, "dbg5       mbarirange->PacketID:                    %u\n", mbarirange->PacketID);
				fprintf(stderr, "dbg5       mbarirange->Version:                     %u\n", mbarirange->Version);
				fprintf(stderr, "dbg5       mbarirange->SizeBytes:                   %u\n", mbarirange->SizeBytes);
				fprintf(stderr, "dbg5       mbarirange->HdrSizeBytes:                %u\n", mbarirange->HdrSizeBytes);
				fprintf(stderr, "dbg5       mbarirange->TimeStart_Sec:               %d\n", mbarirange->TimeStart_Sec);
				fprintf(stderr, "dbg5       mbarirange->TimeStart_nSec:              %d\n", mbarirange->TimeStart_nSec);
				int time_i[7];
				mb_get_date(verbose, time_d, time_i);
				fprintf(stderr, "dbg5       timestamp:                               %.6f %4.4d/%2.2d/%2.2d-%2.2d:%2.2d:%2.2d.%6.6d\n", time_d, time_i[0], time_i[1], time_i[2], time_i[3], time_i[4], time_i[5], time_i[6]);
				fprintf(stderr, "dbg5       mbarirange->NumPtsRow:                   %u\n", mbarirange->NumPtsRow);
				fprintf(stderr, "dbg5       mbarirange->NumPtsPkt:                   %d\n", mbarirange->NumPtsPkt);
				fprintf(stderr, "dbg5       mbarirange->LineLaserPower:              %u  %.2f\n", mbarirange->LineLaserPower, 100.0 * mbarirange->LineLaserPower / 1048576.0);
				fprintf(stderr, "dbg5       mbarirange->PRF_Hz:                      %u\n", mbarirange->PRF_Hz);
				fprintf(stderr, "dbg5       Spare1:                      						 %u\n", Spare1);
				fprintf(stderr, "dbg5       mbarirange->Points_per_LOS:              %u\n", mbarirange->Points_per_LOS);
				fprintf(stderr, "dbg5       mbarirange->ScannerType:                 %u\n", mbarirange->ScannerType);
				fprintf(stderr, "dbg5       mbarirange->lineAccelX:                  %d\n", mbarirange->lineAccelX);
				fprintf(stderr, "dbg5       mbarirange->lineAccelY:                  %d\n", mbarirange->lineAccelY);
				fprintf(stderr, "dbg5       mbarirange->lineAccelZ:                  %d\n", mbarirange->lineAccelZ);
				fprintf(stderr, "dbg5       mbarirange->lineIndex:                   %u\n", mbarirange->lineIndex);
				fprintf(stderr, "dbg5       mbarirange->RowNumber:                   %u\n", mbarirange->RowNumber);
				fprintf(stderr, "dbg5       mbarirange->SHGAmplitudeAv:              %u\n", mbarirange->SHGAmplitudeAv);
				fprintf(stderr, "dbg5       mbarirange->R_Max:                       %u\n", mbarirange->R_Max);
				fprintf(stderr, "dbg5       mbarirange->I_Max:                       %u\n", mbarirange->I_Max);
				fprintf(stderr, "dbg5       mbarirange->R_Auto:                      %u\n", mbarirange->R_Auto);
				fprintf(stderr, "dbg5       mbarirange->I_Auto:                      %u\n", mbarirange->I_Auto);
				fprintf(stderr, "dbg5       mbarirange->R_Mode:                      %u\n", mbarirange->R_Mode);
				fprintf(stderr, "dbg5       mbarirange->I_Mode:                      %u\n", mbarirange->I_Mode);
				fprintf(stderr, "dbg5       mbarirange->I_Good:                      %u\n", mbarirange->I_Good);
				fprintf(stderr, "dbg5       mbarirange->I_Low:                       %u\n", mbarirange->I_Low);
				fprintf(stderr, "dbg5       mbarirange->I_High:                      %u\n", mbarirange->I_High);
				fprintf(stderr, "dbg5       mbarirange->I_Spare:                     %u\n", mbarirange->I_Spare);
				fprintf(stderr, "dbg5       mbarirange->R_offset:                    %u\n", mbarirange->R_offset);
				fprintf(stderr, "dbg5       mbarirange->I_offset:                    %u\n", mbarirange->I_offset);
				fprintf(stderr, "dbg5       mbarirange->AZ_offset:                   %d  %.6f\n", mbarirange->AZ_offset, mbarirange->AZ_offset * 360.0 / 262143.0);
				fprintf(stderr, "dbg5       R_nbits:                     						 %u\n", R_nbits);
				fprintf(stderr, "dbg5       I_nbits:                     						 %u\n", I_nbits);
				fprintf(stderr, "dbg5       AZ_nbits:                    						 %u\n", AZ_nbits);
				fprintf(stderr, "dbg5       Spare2:                      						 %u\n", Spare2);
				}
//int time_i[7];
//mb_get_date(verbose, time_d, time_i);
//fprintf(stderr, "%4.4d/%2.2d/%2.2d-%2.2d:%2.2d:%2.2d.%6.6d   %4.4d:%4.4d:%4.4d",
//time_i[0], time_i[1], time_i[2], time_i[3], time_i[4], time_i[5], time_i[6],
//mbarirange->RowNumber,mbarirange->NumPtsRow,mbarirange->NumPtsPkt);

			/* Initialize the processed data section */
			mbarirange->time_d = time_d;
			mbarirange->navlon = 0.0;
			mbarirange->navlat = 0.0;
			mbarirange->sensordepth = 0.0;
			mbarirange->speed = 0.0;
			mbarirange->heading = 0.0;
			mbarirange->roll = 0.0;
			mbarirange->pitch = 0.0;
			mbarirange->num_soundings = 0;

			/* parse the data section of the record */
			
			/* allocate sounding array */
			if (status == MB_SUCCESS && mbarirange->num_soundings_alloc < 2 * mbarirange->NumPtsPkt)
				{
				size_t alloc_size = 2 * mbarirange->NumPtsPkt * sizeof(struct mbsys_3ddwissl2_sounding_struct);
				status = mb_reallocd(verbose, __FILE__, __LINE__, 
									alloc_size, (void **)(&mbarirange->soundings), error);
				if (status == MB_SUCCESS)
					{
					memset(mbarirange->soundings, 0, alloc_size);
					mbarirange->num_soundings_alloc = 2 * mbarirange->NumPtsPkt;
					}
				else
					{
					done = MB_YES;
					store->kind = MB_DATA_NONE;
					int freestatus = mb_freed(verbose, __FILE__, __LINE__, 
									(void **)(&mbarirange->soundings), error);
					}
				}

			/* parse out the lidar pulse values from the buffer, which contains bit packed arrays */
			char *bpbuffer = NULL;
			void *bitpackedarrayptr = NULL;
			unsigned int bpbuffer_size = 0;
			bitpackedarrayptr = mb_bitpack_new();
			mb_bitpack_setbitsize(bitpackedarrayptr, AZ_nbits);
			status = mb_bitpack_resize(bitpackedarrayptr, mbarirange->NumPtsPkt, &bpbuffer, &bpbuffer_size);
			memcpy(bpbuffer, &buffer[index], bpbuffer_size);
			long long *lptr = (long long *) bpbuffer;
	  	fileheader = &store->fileheader;
			double angle_el = fileheader->ElDeg_cnts * 90.0 / 65535.0;
			int num_soundings = 0;
			for (int ipulse = 0; ipulse < mbarirange->NumPtsPkt; ipulse++) 
				{
				unsigned int i_az;
				status = mb_bitpack_readvalue(bitpackedarrayptr, &i_az);

				mbarirange->soundings[num_soundings].pulse_id = ipulse;
				mbarirange->soundings[num_soundings].sounding_id = 0;
				mbarirange->soundings[num_soundings].time_offset = ((double)ipulse) / ((double)mbarirange->PRF_Hz);
				mbarirange->soundings[num_soundings].angle_az = (mbarirange->AZ_offset + (int) i_az) * 360.0 / 262143.0;
				mbarirange->soundings[num_soundings].angle_el = angle_el;
//if (ipulse == 0)
//fprintf(stderr, "%10d %8.3f ", i_az, mbarirange->soundings[num_soundings].angle_az);
//else if (ipulse == 1)
//fprintf(stderr, "%10d %8.3f ", i_az, mbarirange->soundings[num_soundings].angle_az);
//else if (ipulse == mbarirange->NumPtsPkt - 1)
//fprintf(stderr, "%10d %8.3f\n", i_az, mbarirange->soundings[num_soundings].angle_az);
				num_soundings++;
				
				//mbarirange->soundings[num_soundings].pulse_id = ipulse;
				//mbarirange->soundings[num_soundings].sounding_id = 1;
				//mbarirange->soundings[num_soundings].time_offset = ((double)ipulse) / ((double)mbarirange->PRF_Hz);
				//mbarirange->soundings[num_soundings].angle_az = (mbarirange->AZ_offset + (int) i_az) * 360.0 / 262143.0;
				//mbarirange->soundings[num_soundings].angle_el = angle_el;
				//num_soundings++;
				}
			mb_bitpack_delete(&bitpackedarrayptr);
			index += bpbuffer_size;
		
			bitpackedarrayptr = mb_bitpack_new();
			mb_bitpack_setbitsize(bitpackedarrayptr, R_nbits);
			status = mb_bitpack_resize(bitpackedarrayptr, 2 * mbarirange->NumPtsPkt, &bpbuffer, &bpbuffer_size);
			memcpy(bpbuffer, &buffer[index], bpbuffer_size);
			num_soundings = 0;
			for (int ipulse = 0; ipulse < mbarirange->NumPtsPkt; ipulse++) 
				{
				unsigned int i_r;
				status = mb_bitpack_readvalue(bitpackedarrayptr, &i_r);
				mbarirange->soundings[num_soundings].range = fileheader->RScale_m_per_cnt * i_r;
				num_soundings++;

				status = mb_bitpack_readvalue(bitpackedarrayptr, &i_r);
				//mbarirange->soundings[num_soundings].range = fileheader->RScale_m_per_cnt * i_r;
				//num_soundings++;
				}
			mb_bitpack_delete(&bitpackedarrayptr);
	    index += bpbuffer_size;
//fprintf(stderr, "%s:%d:%s: index:%d\n", __FILE__, __LINE__, __FUNCTION__, index);
		
			bitpackedarrayptr = mb_bitpack_new();
			mb_bitpack_setbitsize(bitpackedarrayptr, I_nbits);
			status = mb_bitpack_resize(bitpackedarrayptr, 2 * mbarirange->NumPtsPkt, &bpbuffer, &bpbuffer_size);
			memcpy(bpbuffer, &buffer[index], bpbuffer_size);
			num_soundings = 0;
			for (int ipulse = 0; ipulse < mbarirange->NumPtsPkt; ipulse++) 
				{
				unsigned int i_i;
				status = mb_bitpack_readvalue(bitpackedarrayptr, &i_i);
				mbarirange->soundings[num_soundings].intensity = i_i;
				num_soundings++;

				status = mb_bitpack_readvalue(bitpackedarrayptr, &i_i);
				//mbarirange->soundings[num_soundings].intensity = i_i;
				//num_soundings++;
				}
			mb_bitpack_delete(&bitpackedarrayptr);
	    index += bpbuffer_size;
//fprintf(stderr, "%s:%d:%s: index:%d\n", __FILE__, __LINE__, __FUNCTION__, index);
		
			bitpackedarrayptr = mb_bitpack_new();
			mb_bitpack_setbitsize(bitpackedarrayptr, 4);
			status = mb_bitpack_resize(bitpackedarrayptr, 2 * mbarirange->NumPtsPkt, &bpbuffer, &bpbuffer_size);
			memcpy(bpbuffer, &buffer[index], bpbuffer_size);
			num_soundings = 0;
			for (int ipulse = 0; ipulse < mbarirange->NumPtsPkt; ipulse++) 
				{
				unsigned int i_c;
				status = mb_bitpack_readvalue(bitpackedarrayptr, &i_c);
				mbarirange->soundings[num_soundings].class = i_c;
				num_soundings++;

				status = mb_bitpack_readvalue(bitpackedarrayptr, &i_c);
				//mbarirange->soundings[num_soundings].class = i_c;
				//num_soundings++;
				}
			mb_bitpack_delete(&bitpackedarrayptr);
	    index += bpbuffer_size;
//fprintf(stderr, "%s:%d:%s: index:%d\n", __FILE__, __LINE__, __FUNCTION__, index);

			mbarirange->num_soundings = num_soundings;
  
#ifndef MBF_3DWISSLP_DEBUG
			if (verbose >= 5) 
#endif
				{
				fprintf(stderr, "\ndbg5  Values read in MBIO function <%s>\n", __func__);
				fprintf(stderr, "dbg5       mbarirange->NumPtsPkt:                   %d\n", mbarirange->NumPtsPkt);
				fprintf(stderr, "dbg5       isounding  Angle Range Amp Class  Range Amp Class\n");
				for (int isounding=0; isounding < mbarirange->num_soundings; isounding++) 
					{
					fprintf(stderr, "dbg5       isounding:%5d   %7.3f   %7.4f %4u %3u\n", 
						isounding, mbarirange->soundings[isounding].angle_az,  
						mbarirange->soundings[isounding].range, mbarirange->soundings[isounding].intensity, mbarirange->soundings[isounding].class);
					}
				}
    	}

			store->kind = MB_DATA_DATA;
			done = true;
		}
		
	else if (PacketID == SRIAT_RECORD_ID_MBARI)
		{

	  read_index = 10;
	  size_t read_len = (size_t)(SizeBytes - read_index);
	  status = mb_fileio_get(verbose, mbio_ptr, &buffer[read_index], &read_len, error);
		
		if (status == MB_SUCCESS) 
			{
	  	struct mbsys_3ddwissl2_mbarirange_struct *mbarirange = &store->mbarirange;
	  	store->bathymetry_calculated = true;
	  	
			int index = 0;
			
			mb_get_binary_int(true, (void *)&buffer[index], &mbarirange->SyncWord); index += 4;
			mbarirange->PacketID = buffer[index]; index += 1;
			mbarirange->Version = buffer[index]; index += 1;
			mb_get_binary_int(true, (void *)&buffer[index], &mbarirange->SizeBytes); index += 4;
					
			mb_get_binary_short(true, (void *)&buffer[index], &(mbarirange->HdrSizeBytes)); index += 2;
			mb_get_binary_int(true, (void *)&buffer[index], &(mbarirange->TimeStart_Sec)); index += 4;
			mb_get_binary_int(true, (void *)&buffer[index], &(mbarirange->TimeStart_nSec)); index += 4;
			mb_get_binary_short(true, (void *)&buffer[index], &(mbarirange->NumPtsRow)); index += 2;
			mb_get_binary_short(true, (void *)&buffer[index], &(mbarirange->NumPtsPkt)); index += 2;
			mb_get_binary_int(true, (void *)&buffer[index], &(mbarirange->LineLaserPower)); index += 4;
			mb_get_binary_int(true, (void *)&buffer[index], &(mbarirange->PRF_Hz)); index += 4;
			mb_get_binary_short(true, (void *)&buffer[index], &(mbarirange->Points_per_LOS)); index += 2;
			mb_get_binary_short(true, (void *)&buffer[index], &(mbarirange->ScannerType)); index += 2;
			mb_get_binary_short(true, (void *)&buffer[index], &(mbarirange->lineAccelX)); index += 2;
			mb_get_binary_short(true, (void *)&buffer[index], &(mbarirange->lineAccelY)); index += 2;
			mb_get_binary_short(true, (void *)&buffer[index], &(mbarirange->lineAccelZ)); index += 2;
			mb_get_binary_short(true, (void *)&buffer[index], &(mbarirange->lineIndex)); index += 2;
			mb_get_binary_short(true, (void *)&buffer[index], &(mbarirange->RowNumber)); index += 2;
			mb_get_binary_short(true, (void *)&buffer[index], &(mbarirange->SHGAmplitudeAv)); index += 2;
			mb_get_binary_int(true, (void *)&buffer[index], &(mbarirange->R_Max)); index += 4;
			mb_get_binary_int(true, (void *)&buffer[index], &(mbarirange->I_Max)); index += 4;
			mb_get_binary_int(true, (void *)&buffer[index], &(mbarirange->R_Auto)); index += 4;
			mb_get_binary_int(true, (void *)&buffer[index], &(mbarirange->I_Auto)); index += 4;
			mb_get_binary_int(true, (void *)&buffer[index], &(mbarirange->R_Mode)); index += 4;
			mb_get_binary_int(true, (void *)&buffer[index], &(mbarirange->I_Mode)); index += 4;
			mbarirange->I_Good = (unsigned char) buffer[index]; index++;
			mbarirange->I_Low = (unsigned char) buffer[index]; index++;
			mbarirange->I_High = (unsigned char) buffer[index]; index++;
			mbarirange->I_Spare = (unsigned char) buffer[index]; index++;
			mb_get_binary_int(true, (void *)&buffer[index], &(mbarirange->R_offset)); index += 4;
			mb_get_binary_int(true, (void *)&buffer[index], &(mbarirange->I_offset)); index += 4;			
			mb_get_binary_int(true, (void *)&buffer[index], &(mbarirange->AZ_offset)); index += 4;
			mb_get_binary_double(true, (void *)&buffer[index], &(mbarirange->time_d)); index += 8;
			mb_get_binary_double(true, (void *)&buffer[index], &(mbarirange->navlon)); index += 8;
			mb_get_binary_double(true, (void *)&buffer[index], &(mbarirange->navlat)); index += 8;
			mb_get_binary_double(true, (void *)&buffer[index], &(mbarirange->sensordepth)); index += 8;
			mb_get_binary_double(true, (void *)&buffer[index], &(mbarirange->speed)); index += 8;
			mb_get_binary_double(true, (void *)&buffer[index], &(mbarirange->heading)); index += 8;
			mb_get_binary_double(true, (void *)&buffer[index], &(mbarirange->roll)); index += 8;
			mb_get_binary_double(true, (void *)&buffer[index], &(mbarirange->pitch)); index += 8;
			mb_get_binary_int(true, (void *)&buffer[index], &(mbarirange->num_soundings)); index += 4;

//fprintf(stderr, "%s:%d:%s: index:%d\n", __FILE__, __LINE__, __FUNCTION__, index);

// int time_i[7];
// mb_get_date(verbose, mbarirange->time_d, time_i);
// fprintf(stdout, "%u %u   %u %u %.6f %4.4d/%2.2d/%2.2d-%2.2d:%2.2d:%2.2d.%6.6d    %d %d %d\n",
// mbarirange->lineIndex, mbarirange->RowNumber, 
// mbarirange->TimeStart_Sec, mbarirange->TimeStart_nSec,
// mbarirange->time_d, time_i[0],time_i[1],time_i[2],time_i[3],time_i[4],time_i[5],time_i[6],
// mbarirange->NumPtsRow, mbarirange->NumPtsPkt, mbarirange->num_soundings);

#ifndef MBF_3DWISSLP_DEBUG
			if (verbose >= 5) 
#endif
				{
				fprintf(stderr, "\ndbg5  Values read in MBIO function <%s>\n", __func__);
				fprintf(stderr, "dbg5       MBARI Range Record:\n");
				fprintf(stderr, "dbg5       mbarirange->SyncWord:                    %u\n", mbarirange->SyncWord);
				fprintf(stderr, "dbg5       mbarirange->PacketID:                    %u\n", mbarirange->PacketID);
				fprintf(stderr, "dbg5       mbarirange->Version:                     %u\n", mbarirange->Version);
				fprintf(stderr, "dbg5       mbarirange->SizeBytes:                   %u\n", mbarirange->SizeBytes);
				fprintf(stderr, "dbg5       mbarirange->HdrSizeBytes:                %u\n", mbarirange->HdrSizeBytes);
				fprintf(stderr, "dbg5       mbarirange->TimeStart_Sec:               %d\n", mbarirange->TimeStart_Sec);
				fprintf(stderr, "dbg5       mbarirange->TimeStart_nSec:              %d\n", mbarirange->TimeStart_nSec);
				double time_d = mbarirange->TimeStart_Sec + 1.0e-9 * mbarirange->TimeStart_nSec;
				int time_i[7];
				mb_get_date(verbose, time_d, time_i);
				fprintf(stderr, "dbg5       timestamp:                               %.6f %4.4d/%2.2d/%2.2d-%2.2d:%2.2d:%2.2d.%6.6d\n", time_d, time_i[0], time_i[1], time_i[2], time_i[3], time_i[4], time_i[5], time_i[6]);
				fprintf(stderr, "dbg5       mbarirange->NumPtsRow:                   %u\n", mbarirange->NumPtsRow);
				fprintf(stderr, "dbg5       mbarirange->NumPtsPkt:                   %d\n", mbarirange->NumPtsPkt);
				fprintf(stderr, "dbg5       mbarirange->LineLaserPower:              %u  %.2f\n", mbarirange->LineLaserPower, 100.0 * mbarirange->LineLaserPower / 1048576.0);
				fprintf(stderr, "dbg5       mbarirange->PRF_Hz:                      %u\n", mbarirange->PRF_Hz);
				fprintf(stderr, "dbg5       mbarirange->Points_per_LOS:              %u\n", mbarirange->Points_per_LOS);
				fprintf(stderr, "dbg5       mbarirange->ScannerType:                 %u\n", mbarirange->ScannerType);
				fprintf(stderr, "dbg5       mbarirange->lineAccelX:                  %d\n", mbarirange->lineAccelX);
				fprintf(stderr, "dbg5       mbarirange->lineAccelY:                  %d\n", mbarirange->lineAccelY);
				fprintf(stderr, "dbg5       mbarirange->lineAccelZ:                  %d\n", mbarirange->lineAccelZ);
				fprintf(stderr, "dbg5       mbarirange->lineIndex:                   %u\n", mbarirange->lineIndex);
				fprintf(stderr, "dbg5       mbarirange->RowNumber:                   %u\n", mbarirange->RowNumber);
				fprintf(stderr, "dbg5       mbarirange->SHGAmplitudeAv:              %u\n", mbarirange->SHGAmplitudeAv);
				fprintf(stderr, "dbg5       mbarirange->R_Max:                       %u\n", mbarirange->R_Max);
				fprintf(stderr, "dbg5       mbarirange->I_Max:                       %u\n", mbarirange->I_Max);
				fprintf(stderr, "dbg5       mbarirange->R_Auto:                      %u\n", mbarirange->R_Auto);
				fprintf(stderr, "dbg5       mbarirange->I_Auto:                      %u\n", mbarirange->I_Auto);
				fprintf(stderr, "dbg5       mbarirange->R_Mode:                      %u\n", mbarirange->R_Mode);
				fprintf(stderr, "dbg5       mbarirange->I_Mode:                      %u\n", mbarirange->I_Mode);
				fprintf(stderr, "dbg5       mbarirange->I_Good:                      %u\n", mbarirange->I_Good);
				fprintf(stderr, "dbg5       mbarirange->I_Low:                       %u\n", mbarirange->I_Good);
				fprintf(stderr, "dbg5       mbarirange->I_High:                      %u\n", mbarirange->I_Good);
				fprintf(stderr, "dbg5       mbarirange->I_Spare:                      %u\n", mbarirange->I_Spare);
				fprintf(stderr, "dbg5       mbarirange->R_offset:                    %u\n", mbarirange->R_offset);
				fprintf(stderr, "dbg5       mbarirange->I_offset:                    %u\n", mbarirange->I_offset);
				fprintf(stderr, "dbg5       mbarirange->AZ_offset:                   %d  %.6f\n", mbarirange->AZ_offset, mbarirange->AZ_offset * 360.0 / 262143.0);
				fprintf(stderr, "dbg5       mbarirange->time_d:                      %f\n", mbarirange->time_d);
				fprintf(stderr, "dbg5       mbarirange->navlon:                      %f\n", mbarirange->navlon);
				fprintf(stderr, "dbg5       mbarirange->navlat:                      %f\n", mbarirange->navlat);
				fprintf(stderr, "dbg5       mbarirange->sensordepth:                 %f\n", mbarirange->sensordepth);
				fprintf(stderr, "dbg5       mbarirange->speed:                       %f\n", mbarirange->speed);
				fprintf(stderr, "dbg5       mbarirange->heading:                     %f\n", mbarirange->heading);
				fprintf(stderr, "dbg5       mbarirange->roll:                        %f\n", mbarirange->roll);
				fprintf(stderr, "dbg5       mbarirange->pitch:                       %f\n", mbarirange->pitch);
				fprintf(stderr, "dbg5       mbarirange->num_soundings:               %d\n", mbarirange->num_soundings);
				}
				
			/* parse the data section of the record */
			
			/* allocate sounding array */
			if (status == MB_SUCCESS && mbarirange->num_soundings_alloc < mbarirange->num_soundings)
				{
				size_t alloc_size = mbarirange->num_soundings * sizeof(struct mbsys_3ddwissl2_sounding_struct);
				status = mb_reallocd(verbose, __FILE__, __LINE__, 
									alloc_size, (void **)(&mbarirange->soundings), error);
				if (status == MB_SUCCESS)
					{
					memset(mbarirange->soundings, 0, alloc_size);
					mbarirange->num_soundings_alloc = mbarirange->num_soundings;
					}
				else
					{
					done = MB_YES;
					store->kind = MB_DATA_NONE;
					int freestatus = mb_freed(verbose, __FILE__, __LINE__, 
									(void **)(&mbarirange->soundings), error);
					}
				}
				
			/* read the soundings */
			for (int isounding = 0; isounding < mbarirange->num_soundings; isounding++) 
				{
				struct mbsys_3ddwissl2_sounding_struct *sounding = &(mbarirange->soundings[isounding]);
				
				mb_get_binary_short(true, (void *)&buffer[index], &(sounding->pulse_id)); index += 2;
				mb_get_binary_short(true, (void *)&buffer[index], &(sounding->sounding_id)); index += 2;
				mb_get_binary_float(true, (void *)&buffer[index], &(sounding->time_offset)); index += 4;
				mb_get_binary_float(true, (void *)&buffer[index], &(sounding->acrosstrack_offset)); index += 4;
				mb_get_binary_float(true, (void *)&buffer[index], &(sounding->alongtrack_offset)); index += 4;
				mb_get_binary_float(true, (void *)&buffer[index], &(sounding->sensordepth_offset)); index += 4;
				mb_get_binary_float(true, (void *)&buffer[index], &(sounding->heading_offset)); index += 4;
				mb_get_binary_float(true, (void *)&buffer[index], &(sounding->pitch_offset)); index += 4;
				mb_get_binary_float(true, (void *)&buffer[index], &(sounding->range)); index += 4;
				mb_get_binary_float(true, (void *)&buffer[index], &(sounding->angle_az)); index += 4;
				mb_get_binary_float(true, (void *)&buffer[index], &(sounding->angle_el)); index += 4;
				mb_get_binary_short(true, (void *)&buffer[index], &(sounding->intensity)); index += 2;
				sounding->class = buffer[index]; index++;
				sounding->beamflag = buffer[index]; index++;
				mb_get_binary_float(true, (void *)&buffer[index], &(sounding->acrosstrack)); index += 4;
				mb_get_binary_float(true, (void *)&buffer[index], &(sounding->alongtrack)); index += 4;
				mb_get_binary_float(true, (void *)&buffer[index], &(sounding->depth)); index += 4;
				}

#ifndef MBF_3DWISSLP_DEBUG
			if (verbose >= 5) 
#endif
				{
				for (int isounding=0; isounding < mbarirange->num_soundings; isounding++) 
					{
					struct mbsys_3ddwissl2_sounding_struct *sounding = &(mbarirange->soundings[isounding]);
					fprintf(stderr, "dbg5       sdg:%5.5d:%4.4d:%d | %8.6f %5.3f %5.3f %5.3f %5.3f %5.3f %5.3f | %7.4f %8.3f %8.3f %4u %3u | %8.4f %8.4f %8.4f %u\n", 
						isounding, sounding->pulse_id, sounding->sounding_id, sounding->time_offset,
						sounding->acrosstrack_offset, sounding->alongtrack_offset, sounding->sensordepth_offset,
						sounding->heading_offset, sounding->roll_offset, sounding->pitch_offset,
						sounding->range, sounding->angle_az, sounding->angle_el, sounding->intensity, sounding->class,
						sounding->acrosstrack, sounding->alongtrack, sounding->depth, sounding->beamflag);
					}
				}
    	}

			store->kind = MB_DATA_DATA;
			done = true;
		}
		
	else
		{
			store->kind = MB_DATA_NONE;
			done = true;
		}

  if (verbose >= 2)
    {
    fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       error:      %d\n", *error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:     %d\n", status);
    }

  return status;
} /* mbr_3dwissl2_rd_data */
  
/*--------------------------------------------------------------------*/
int mbr_rt_3dwissl2
(
  int verbose,
  void *mbio_ptr,
  void *store_ptr,
  int *error
)
{
  if (verbose >= 2)
    {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
    fprintf(stderr, "dbg2       mbio_ptr:   %p\n", (void *)mbio_ptr);
    fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
    }

  /* check for non-null pointers */
  assert(mbio_ptr != NULL);
  assert(store_ptr != NULL);

  /* get pointers to mbio descriptor and data structure */
  struct mb_io_struct *mb_io_ptr = (struct mb_io_struct *)mbio_ptr;
  struct mbsys_3ddwissl2_struct *store = (struct mbsys_3ddwissl2_struct *)store_ptr;

  /* read next data from file */
  const int status = mbr_3dwissl2_rd_data(verbose, mbio_ptr, store_ptr, error);

  /* if needed calculate bathymetry */
  if (( status == MB_SUCCESS) && ( store->kind == MB_DATA_DATA) &&
      (!store->bathymetry_calculated) )
    mbsys_3ddwissl2_calculatebathymetry(verbose,
      mbio_ptr,
      store_ptr,
      MBSYS_3DDWISSL2_DEFAULT_AMPLITUDE_THRESHOLD,
      MBSYS_3DDWISSL2_DEFAULT_TARGET_ALTITUDE,
      error);

  /* set error and kind in mb_io_ptr */
  mb_io_ptr->new_error = *error;
  mb_io_ptr->new_kind = store->kind;

  if (verbose >= 2)
    {
    fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       error:      %d\n", *error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:  %d\n", status);
    }

  return status;
} /* mbr_rt_3dwissl2 */
/*--------------------------------------------------------------------*/
int mbr_3dwissl2_wr_data
(
  int verbose,
  void *mbio_ptr,
  void *store_ptr,
  int *error
)
{
  if (verbose >= 2)
    {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
    fprintf(stderr, "dbg2       mbio_ptr:   %p\n", (void *)mbio_ptr);
    fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
    }

  /* check for non-null pointers */
  assert(mbio_ptr != NULL);
  assert(store_ptr != NULL);
  
  /* get pointer to mbio descriptor */
  struct mb_io_struct *mb_io_ptr = (struct mb_io_struct *)mbio_ptr;
  struct mbsys_3ddwissl2_struct *store = (struct mbsys_3ddwissl2_struct *)store_ptr;

  /* set file position */
  mb_io_ptr->file_pos = ftell(mb_io_ptr->mbfp);

  if (verbose >= 4)
    {
    fprintf(stderr, "\ndbg4  Data record kind in MBIO function <%s>\n", __func__);
    fprintf(stderr, "dbg4       kind:       %d\n", store->kind);
    }

  /* set status */
  int status = MB_SUCCESS;
  *error = MB_ERROR_NO_ERROR;
  
  /* write records in the order they arrive */
  if (store->kind == MB_DATA_PARAMETER) 
  	{
		/* allocate read buffer as needed */
		if (mb_io_ptr->data_structure_size < (size_t)(SRIAT_RECORD_SIZE_FILEHEADER))
			{
			if ((status = mb_reallocd(verbose, __FILE__, __LINE__, 
								(size_t)SRIAT_RECORD_SIZE_FILEHEADER, 
								(void **)(&mb_io_ptr->raw_data), error)) == MB_SUCCESS)
				{
				mb_io_ptr->data_structure_size = (size_t)SRIAT_RECORD_SIZE_FILEHEADER;
				}
			else 
				{
				mb_io_ptr->data_structure_size = (size_t)0;
				}
			}
			
		if (status == MB_SUCCESS) 
			{
			struct mbsys_3ddwissl2_sriat_fileheader_struct *fileheader = &store->fileheader;
			char *buffer = mb_io_ptr->raw_data;
	
#ifndef MBF_3DWISSLP_DEBUG
			if (verbose >= 5) 
#endif
				{
				fprintf(stderr, "\ndbg5  Values read in MBIO function <%s>\n", __func__);
				fprintf(stderr, "dbg5       Fileheader Record:\n");
				fprintf(stderr, "dbg5       fileheader->SyncWord:                    %u\n", fileheader->SyncWord);
				fprintf(stderr, "dbg5       fileheader->PacketID:                    %u\n", fileheader->PacketID);
				fprintf(stderr, "dbg5       fileheader->Version:                     %u\n", fileheader->Version);
				fprintf(stderr, "dbg5       fileheader->SizeBytes:                   %u\n", fileheader->SizeBytes);
				fprintf(stderr, "dbg5       fileheader->ScanSizeBytes:               %u\n", fileheader->ScanSizeBytes);
				fprintf(stderr, "dbg5       fileheader->TimeStart_Sec:               %d\n", fileheader->TimeStart_Sec);
				fprintf(stderr, "dbg5       fileheader->TimeStart_nSec:              %d\n", fileheader->TimeStart_nSec);
				fprintf(stderr, "dbg5       fileheader->TimeEnd_Sec:                 %d\n", fileheader->TimeEnd_Sec);
				fprintf(stderr, "dbg5       fileheader->TimeEnd_nSec:                %d\n", fileheader->TimeEnd_nSec);
				fprintf(stderr, "dbg5       fileheader->SL_GEN:                      %u\n", fileheader->SL_GEN);
				fprintf(stderr, "dbg5       fileheader->SL_Letter:                   %u\n", fileheader->SL_Letter);
				fprintf(stderr, "dbg5       fileheader->SL_X:                        %u\n", fileheader->SL_X);
				fprintf(stderr, "dbg5       fileheader->nPtsToAverage:               %u\n", fileheader->nPtsToAverage);
				fprintf(stderr, "dbg5       fileheader->cJobName:                    %s\n", fileheader->cJobName);
				fprintf(stderr, "dbg5       fileheader->cScanPos:                    %s\n", fileheader->cScanPos);
				fprintf(stderr, "dbg5       fileheader->cfileTag:                    %s\n", fileheader->cfileTag);
				fprintf(stderr, "dbg5       fileheader->nScanNum:                    %u\n", fileheader->nScanNum);
				fprintf(stderr, "dbg5       fileheader->AzCmdStart:                  %u\n", fileheader->AzCmdStart);
				fprintf(stderr, "dbg5       fileheader->AzCmdEnd:                    %u\n", fileheader->AzCmdEnd);
				fprintf(stderr, "dbg5       fileheader->nScanNum:                    %u\n", fileheader->nScanNum);
				fprintf(stderr, "dbg5       fileheader->rawbit1:                     %u\n", fileheader->rawbit1);
				fprintf(stderr, "dbg5       -fileheader->nPtsPerScanLine:             %u\n", fileheader->nPtsPerScanLine);
				fprintf(stderr, "dbg5       -fileheader->nScanLinesPerScan:           %u\n", fileheader->nScanLinesPerScan);
				fprintf(stderr, "dbg5       -fileheader->Spare1:                      %u\n", fileheader->Spare1);
				fprintf(stderr, "dbg5       fileheader->rawbit2:                     %u\n", fileheader->rawbit2);
				fprintf(stderr, "dbg5       -fileheader->nPtsPerLine:                 %u\n", fileheader->nPtsPerLine);
				fprintf(stderr, "dbg5       -fileheader->Mode:                        %u\n", fileheader->Mode);
				fprintf(stderr, "dbg5       -fileheader->nTPtsPerScanLine:            %u\n", fileheader->nTPtsPerScanLine);
				fprintf(stderr, "dbg5       -fileheader->bHaveThermal:                %u\n", fileheader->bHaveThermal);
				fprintf(stderr, "dbg5       fileheader->ShotCnt:                     %u\n", fileheader->ShotCnt);
				fprintf(stderr, "dbg5       fileheader->WaterSalinity_psu:           %u  %.3f\n", fileheader->WaterSalinity_psu, fileheader->WaterSalinity_psu * 42.0 / 65535.0 - 2.0);
				fprintf(stderr, "dbg5       fileheader->WaterPressure_dbar:          %u\n", fileheader->WaterPressure_dbar);
				fprintf(stderr, "dbg5       fileheader->rawbit3:                     %u\n", fileheader->rawbit3);
				fprintf(stderr, "dbg5       -fileheader->WaterTemperature_C:          %u  %.3f\n", fileheader->WaterTemperature_C, fileheader->WaterTemperature_C * 37.0 / 8191.0 - 2.0);
				fprintf(stderr, "dbg5       -fileheader->PRF_Hz:                      %u\n", fileheader->PRF_Hz);
				fprintf(stderr, "dbg5       fileheader->DigitizerTemperature_C:      %u  %.3f\n", fileheader->DigitizerTemperature_C, fileheader->DigitizerTemperature_C * 100.0 / 255.0);
				fprintf(stderr, "dbg5       fileheader->RScale_m_per_cnt:            %f\n", fileheader->RScale_m_per_cnt);
				fprintf(stderr, "dbg5       fileheader->ThBinStart_cnt:              %u\n", fileheader->ThBinStart_cnt);
				fprintf(stderr, "dbg5       fileheader->ThBinEnd_cnts:               %u\n", fileheader->ThBinEnd_cnts);
				fprintf(stderr, "dbg5       fileheader->TempAzCnt:                   %u\n", fileheader->TempAzCnt);
				fprintf(stderr, "dbg5       fileheader->TempRowCnt:                  %u\n", fileheader->TempRowCnt);
				fprintf(stderr, "dbg5       fileheader->rawbit4:                     %u\n", fileheader->rawbit4);
				fprintf(stderr, "dbg5       -fileheader->TempRCnt_av2:                %u\n", fileheader->TempRCnt_av2);
				fprintf(stderr, "dbg5       -fileheader->TempRCnt_av4:                %u\n", fileheader->TempRCnt_av4);
				fprintf(stderr, "dbg5       -fileheader->TempRCnt_av8:                %u\n", fileheader->TempRCnt_av8);
				fprintf(stderr, "dbg5       -fileheader->TempRCnt_av16:               %u\n", fileheader->TempRCnt_av16);
				fprintf(stderr, "dbg5       fileheader->ScannerShift_mDeg:           %u\n", fileheader->ScannerShift_mDeg);
				fprintf(stderr, "dbg5       fileheader->Shift_m[0]:                  %f\n", fileheader->Shift_m[0]);
				fprintf(stderr, "dbg5       fileheader->Shift_m[1]:                  %f\n", fileheader->Shift_m[1]);
				fprintf(stderr, "dbg5       fileheader->Shift_m[2]:                  %f\n", fileheader->Shift_m[2]);
				fprintf(stderr, "dbg5       fileheader->Rotate_deg[0]:               %f\n", fileheader->Rotate_deg[0]);
				fprintf(stderr, "dbg5       fileheader->Rotate_deg[1]:               %f\n", fileheader->Rotate_deg[1]);
				fprintf(stderr, "dbg5       fileheader->Rotate_deg[2]:               %f\n", fileheader->Rotate_deg[2]);
				fprintf(stderr, "dbg5       fileheader->EC_Version:                  %d.%d.%d.%d\n", 
								fileheader->EC_Version[0], fileheader->EC_Version[1], 
								fileheader->EC_Version[2], fileheader->EC_Version[3]);
				fprintf(stderr, "dbg5       fileheader->InstaCloud_Version:          %d.%d.%d.%d\n", 
								fileheader->InstaCloud_Version[0], fileheader->InstaCloud_Version[1], 
								fileheader->InstaCloud_Version[2], fileheader->InstaCloud_Version[3]);
				fprintf(stderr, "dbg5       fileheader->ElDeg_cnts:                  %d  %.3f\n", fileheader->ElDeg_cnts, fileheader->ElDeg_cnts * 90.0 / 65535.0);
				}

			int index = 0;
			mb_put_binary_int(true, fileheader->SyncWord, (void **)&buffer[index]); index += 4;
			buffer[index] = fileheader->PacketID; index += 1;
			buffer[index] = fileheader->Version; index += 1;
			mb_put_binary_int(true, fileheader->SizeBytes, (void **)&buffer[index]); index += 4;
				
			mb_put_binary_int(true, (fileheader->ScanSizeBytes), (void **)&buffer[index]); index += 4;
			mb_put_binary_int(true, (fileheader->TimeStart_Sec), (void **)&buffer[index]); index += 4;
			mb_put_binary_int(true, (fileheader->TimeStart_nSec), (void **)&buffer[index]); index += 4;
			mb_put_binary_int(true, (fileheader->TimeEnd_Sec), (void **)&buffer[index]); index += 4;
			mb_put_binary_int(true, (fileheader->TimeEnd_nSec), (void **)&buffer[index]); index += 4;
			buffer[index] = fileheader->SL_GEN; index++;
			buffer[index] = fileheader->SL_Letter; index++;
			buffer[index] = fileheader->SL_X; index++;
			buffer[index] = fileheader->nPtsToAverage; index++;
			memcpy(&(buffer[index]), fileheader->cJobName, (size_t)24); index += 24;
			memcpy(&(buffer[index]), fileheader->cScanPos, (size_t)24); index += 24;
			memcpy(&(buffer[index]), fileheader->cfileTag, (size_t)24); index += 24;
			mb_put_binary_short(true, (fileheader->nScanNum), (void **)&buffer[index]); index += 2;
	  
	  	mb_put_binary_int(true, (fileheader->AzCmdStart), (void **)&buffer[index]); index += 4;
			mb_put_binary_int(true, (fileheader->AzCmdEnd), (void **)&buffer[index]); index += 4;
	
			mb_put_binary_int(true, (fileheader->rawbit1), (void **)&buffer[index]); index += 4;
			//fileheader->nPtsPerScanLine = fileheader->rawbit1 & 0x3FFF;
			//fileheader->nScanLinesPerScan = (fileheader->rawbit1 >> 14) & 0xFFF;
			//fileheader->Spare1 = fileheader->rawbit1 >> 26;
			
			mb_put_binary_int(true, (fileheader->rawbit2), (void **)&buffer[index]); index += 4;
			//fileheader->nPtsPerLine = fileheader->rawbit2 & 0x3FFF;
			//fileheader->Mode = (fileheader->rawbit2 >> 14) & 0x7;
			//fileheader->nTPtsPerScanLine = (fileheader->rawbit2 >> 17) & 0x3FFF;
			//fileheader->bHaveThermal = fileheader->rawbit2 >> 31;
	  
			mb_put_binary_int(true, (fileheader->ShotCnt), (void **)&buffer[index]); index += 4;
			mb_put_binary_short(true, (fileheader->WaterSalinity_psu), (void **)&buffer[index]); index += 2;
			mb_put_binary_short(true, (fileheader->WaterPressure_dbar), (void **)&buffer[index]); index += 2;
			
			mb_put_binary_int(true, (fileheader->rawbit3), (void **)&buffer[index]); index += 4;
			//fileheader->WaterTemperature_C = fileheader->rawbit3 & 0x1FFF;
			//fileheader->PRF_Hz = fileheader->rawbit3 >> 13;
		
			fileheader->DigitizerTemperature_C = buffer[index]; index++;
			mb_put_binary_float(true, (fileheader->RScale_m_per_cnt), (void **)&buffer[index]); index += 4;
			mb_put_binary_short(true, (fileheader->ThBinStart_cnt), (void **)&buffer[index]); index += 2;
			mb_put_binary_short(true, (fileheader->ThBinEnd_cnts), (void **)&buffer[index]); index += 2;
			fileheader->TempAzCnt = buffer[index]; index++;
			fileheader->TempRowCnt = buffer[index]; index++;
		
			mb_put_binary_int(true, (fileheader->rawbit4), (void **)&buffer[index]); index += 4;
			//fileheader->TempRCnt_av2 = fileheader->rawbit4 & 0xFF;
			//fileheader->TempRCnt_av4 = (fileheader->rawbit4 >> 8) & 0xFF;
			//fileheader->TempRCnt_av8 = (fileheader->rawbit4 >> 16) & 0xFF;
			//fileheader->TempRCnt_av16 = fileheader->rawbit4 >> 24;
		
			mb_put_binary_short(true, (fileheader->ScannerShift_mDeg), (void **)&buffer[index]); index += 2;
			mb_put_binary_float(true, (fileheader->Shift_m[0]), (void **)&buffer[index]); index += 4;
			mb_put_binary_float(true, (fileheader->Shift_m[1]), (void **)&buffer[index]); index += 4;
			mb_put_binary_float(true, (fileheader->Shift_m[2]), (void **)&buffer[index]); index += 4;
			mb_put_binary_float(true, (fileheader->Rotate_deg[0]), (void **)&buffer[index]); index += 4;
			mb_put_binary_float(true, (fileheader->Rotate_deg[1]), (void **)&buffer[index]); index += 4;
			mb_put_binary_float(true, (fileheader->Rotate_deg[2]), (void **)&buffer[index]); index += 4;
			memcpy(&(buffer[index]), fileheader->EC_Version, (size_t)4); index += 4;
			memcpy(&(buffer[index]), fileheader->InstaCloud_Version, (size_t)4); index += 4;
			mb_put_binary_short(true, (fileheader->ElDeg_cnts), (void **)&buffer[index]); index += 2;
//fprintf(stderr, "%s:%d:%s: index:%d\n", __FILE__, __LINE__, __FUNCTION__, index);

      /* write data record from buffer */
      size_t write_len = fileheader->SizeBytes;
      status = mb_fileio_put(verbose, mbio_ptr, (void *)buffer, &write_len, error);
			}

  	}

  else if (store->kind == MB_DATA_COMMENT) 
  	{
  	struct mbsys_3ddwissl2_comment_struct *comment = &store->comment;

  	/* print out contents of the data record */
#ifndef MBF_3DWISSLP_DEBUG
		if (verbose >= 5) 
#endif
			{
			fprintf(stderr, "\ndbg5  Values read in MBIO function <%s>\n", __func__);
			fprintf(stderr, "dbg5       Comment Record:\n");
			fprintf(stderr, "dbg5       comment->SyncWord:                       %u\n", comment->SyncWord);
			fprintf(stderr, "dbg5       comment->PacketID:                       %u\n", comment->PacketID);
			fprintf(stderr, "dbg5       comment->Version:                        %u\n", comment->Version);
			fprintf(stderr, "dbg5       comment->SizeBytes:                      %u\n", comment->SizeBytes);
			fprintf(stderr, "dbg5       comment->comment_len:                    %u\n", comment->comment_len);
			fprintf(stderr, "dbg5       comment->comment:                        %s\n", comment->comment);
			}

		/* calculate the record size */
		comment->SizeBytes = SRIAT_RECORD_SIZE_COMMENT_HEADER + comment->comment_len;

		/* allocate write buffer as needed */
		if (mb_io_ptr->data_structure_size < (size_t)(comment->SizeBytes))
			{
			if ((status = mb_reallocd(verbose, __FILE__, __LINE__, 
								(size_t)comment->SizeBytes, 
								(void **)(&mb_io_ptr->raw_data), error)) == MB_SUCCESS)
				{
				mb_io_ptr->data_structure_size = (size_t)comment->SizeBytes;
				}
			else 
				{
				mb_io_ptr->data_structure_size = (size_t)0;
				}
			}
			
		if (status == MB_SUCCESS) 
			{
			struct mbsys_3ddwissl2_comment_struct *comment = &store->comment;
			char *buffer = mb_io_ptr->raw_data;

			int index = 0;
			mb_put_binary_int(true, comment->SyncWord, (void **)&buffer[index]); index += 4;
			buffer[index] = comment->PacketID; index += 1;
			buffer[index] = comment->Version; index += 1;
			mb_put_binary_int(true, comment->SizeBytes, (void **)&buffer[index]); index += 4;
					
			mb_put_binary_short(true, comment->comment_len, (void **)&buffer[index]); index += 2;
 			memcpy(&(buffer[index]), comment->comment, (size_t)comment->comment_len); index += 24;
 	
    	/* write data record from buffer */
    	size_t write_len = comment->SizeBytes;
    	status = mb_fileio_put(verbose, mbio_ptr, (void *)buffer, &write_len, error);
    	}
  	}

  else if (store->kind == MB_DATA_DATA) 
  	{
  	struct mbsys_3ddwissl2_mbarirange_struct *mbarirange = &store->mbarirange;

		/* calculate the record size */
  	mbarirange->PacketID = SRIAT_RECORD_ID_MBARI;
		mbarirange->HdrSizeBytes = SRIAT_RECORD_SIZE_MBARI_HEADER;
		mbarirange->SizeBytes = mbarirange->HdrSizeBytes + mbarirange->num_soundings * SRIAT_RECORD_SIZE_MBARI_SOUNDING;

  	/* print out contents of the data record */
#ifndef MBF_3DWISSLP_DEBUG
		if (verbose >= 5) 
#endif
			{
			fprintf(stderr, "\ndbg5  Values read in MBIO function <%s>\n", __func__);
			fprintf(stderr, "dbg5       MBARI Range Record:\n");
			fprintf(stderr, "dbg5       mbarirange->SyncWord:                    %u\n", mbarirange->SyncWord);
			fprintf(stderr, "dbg5       mbarirange->PacketID:                    %u\n", mbarirange->PacketID);
			fprintf(stderr, "dbg5       mbarirange->Version:                     %u\n", mbarirange->Version);
			fprintf(stderr, "dbg5       mbarirange->SizeBytes:                   %u\n", mbarirange->SizeBytes);
			fprintf(stderr, "dbg5       mbarirange->HdrSizeBytes:                %u\n", mbarirange->HdrSizeBytes);
			fprintf(stderr, "dbg5       mbarirange->TimeStart_Sec:               %d\n", mbarirange->TimeStart_Sec);
			fprintf(stderr, "dbg5       mbarirange->TimeStart_nSec:              %d\n", mbarirange->TimeStart_nSec);
			fprintf(stderr, "dbg5       mbarirange->NumPtsRow:                   %u\n", mbarirange->NumPtsRow);
			fprintf(stderr, "dbg5       mbarirange->NumPtsPkt:                   %d\n", mbarirange->NumPtsPkt);
			fprintf(stderr, "dbg5       mbarirange->LineLaserPower:              %u  %.2f\n", mbarirange->LineLaserPower, 100.0 * mbarirange->LineLaserPower / 1048576.0);
			fprintf(stderr, "dbg5       mbarirange->PRF_Hz:                      %u\n", mbarirange->PRF_Hz);
			fprintf(stderr, "dbg5       mbarirange->Points_per_LOS:              %u\n", mbarirange->Points_per_LOS);
			fprintf(stderr, "dbg5       mbarirange->ScannerType:                 %u\n", mbarirange->ScannerType);
			fprintf(stderr, "dbg5       mbarirange->lineAccelX:                  %d\n", mbarirange->lineAccelX);
			fprintf(stderr, "dbg5       mbarirange->lineAccelY:                  %d\n", mbarirange->lineAccelY);
			fprintf(stderr, "dbg5       mbarirange->lineAccelZ:                  %d\n", mbarirange->lineAccelZ);
			fprintf(stderr, "dbg5       mbarirange->lineIndex:                   %u\n", mbarirange->lineIndex);
			fprintf(stderr, "dbg5       mbarirange->RowNumber:                   %u\n", mbarirange->RowNumber);
			fprintf(stderr, "dbg5       mbarirange->SHGAmplitudeAv:              %u\n", mbarirange->SHGAmplitudeAv);
			fprintf(stderr, "dbg5       mbarirange->R_Max:                       %u\n", mbarirange->R_Max);
			fprintf(stderr, "dbg5       mbarirange->I_Max:                       %u\n", mbarirange->I_Max);
			fprintf(stderr, "dbg5       mbarirange->R_Auto:                      %u\n", mbarirange->R_Auto);
			fprintf(stderr, "dbg5       mbarirange->I_Auto:                      %u\n", mbarirange->I_Auto);
			fprintf(stderr, "dbg5       mbarirange->R_Mode:                      %u\n", mbarirange->R_Mode);
			fprintf(stderr, "dbg5       mbarirange->I_Mode:                      %u\n", mbarirange->I_Mode);
			fprintf(stderr, "dbg5       mbarirange->I_Good:                      %u\n", mbarirange->I_Good);
			fprintf(stderr, "dbg5       mbarirange->I_Low:                       %u\n", mbarirange->I_Good);
			fprintf(stderr, "dbg5       mbarirange->I_High:                      %u\n", mbarirange->I_Good);
			fprintf(stderr, "dbg5       mbarirange->I_Spare:                      %u\n", mbarirange->I_Spare);
			fprintf(stderr, "dbg5       mbarirange->R_offset:                    %u\n", mbarirange->R_offset);
			fprintf(stderr, "dbg5       mbarirange->I_offset:                    %u\n", mbarirange->I_offset);
			fprintf(stderr, "dbg5       mbarirange->AZ_offset:                   %d  %.6f\n", mbarirange->AZ_offset, mbarirange->AZ_offset * 360.0 / 262143.0);
			fprintf(stderr, "dbg5       mbarirange->time_d:                      %f\n", mbarirange->time_d);
			fprintf(stderr, "dbg5       mbarirange->navlon:                      %f\n", mbarirange->navlon);
			fprintf(stderr, "dbg5       mbarirange->navlat:                      %f\n", mbarirange->navlat);
			fprintf(stderr, "dbg5       mbarirange->sensordepth:                 %f\n", mbarirange->sensordepth);
			fprintf(stderr, "dbg5       mbarirange->speed:                       %f\n", mbarirange->speed);
			fprintf(stderr, "dbg5       mbarirange->heading:                     %f\n", mbarirange->heading);
			fprintf(stderr, "dbg5       mbarirange->roll:                        %f\n", mbarirange->roll);
			fprintf(stderr, "dbg5       mbarirange->pitch:                       %f\n", mbarirange->pitch);
			fprintf(stderr, "dbg5       mbarirange->num_soundings:               %d\n", mbarirange->num_soundings);
			fprintf(stderr, "dbg5       ipulse  Angle Range Amp Class  Range Amp Class\n");
			for (int isounding=0; isounding < mbarirange->num_soundings; isounding++) 
				{
				struct mbsys_3ddwissl2_sounding_struct *sounding = &(mbarirange->soundings[isounding]);
				fprintf(stderr, "dbg5       sdg:%5.5d:%4.4d:%d | %8.6f %5.3f %5.3f %5.3f %5.3f %5.3f %5.3f | %7.4f %8.3f %8.3f %4u %3u | %8.4f %8.4f %8.4f %u\n", 
					isounding, sounding->pulse_id, sounding->sounding_id, sounding->time_offset,
					sounding->acrosstrack_offset, sounding->alongtrack_offset, sounding->sensordepth_offset,
					sounding->heading_offset, sounding->roll_offset, sounding->pitch_offset,
					sounding->range, sounding->angle_az, sounding->angle_el, sounding->intensity, sounding->class,
					sounding->acrosstrack, sounding->alongtrack, sounding->depth, sounding->beamflag);
				}
			}
  	
		/* allocate write buffer as needed */
		if (mb_io_ptr->data_structure_size < (size_t)(mbarirange->SizeBytes))
			{
			if ((status = mb_reallocd(verbose, __FILE__, __LINE__, 
								(size_t)mbarirange->SizeBytes, 
								(void **)(&mb_io_ptr->raw_data), error)) == MB_SUCCESS)
				{
				mb_io_ptr->data_structure_size = (size_t)mbarirange->SizeBytes;
				}
			else 
				{
				mb_io_ptr->data_structure_size = (size_t)0;
				}
			}
			
		if (status == MB_SUCCESS) 
			{
			struct mbsys_3ddwissl2_mbarirange_struct *mbarirange = &store->mbarirange;
			char *buffer = mb_io_ptr->raw_data;

			int index = 0;
			mb_put_binary_int(true, mbarirange->SyncWord, (void **)&buffer[index]); index += 4;
			buffer[index] = mbarirange->PacketID; index += 1;
			buffer[index] = mbarirange->Version; index += 1;
			mb_put_binary_int(true, mbarirange->SizeBytes, (void **)&buffer[index]); index += 4;
					
			mb_put_binary_short(true, mbarirange->HdrSizeBytes, (void **)&buffer[index]); index += 2;
			mb_put_binary_int(true, mbarirange->TimeStart_Sec, (void **)&buffer[index]); index += 4;
			mb_put_binary_int(true, mbarirange->TimeStart_nSec, (void **)&buffer[index]); index += 4;
			mb_put_binary_short(true, mbarirange->NumPtsRow, (void **)&buffer[index]); index += 2;
			mb_put_binary_short(true, mbarirange->NumPtsPkt, (void **)&buffer[index]); index += 2;
			mb_put_binary_int(true, mbarirange->LineLaserPower, (void **)&buffer[index]); index += 4;
			mb_put_binary_int(true, mbarirange->PRF_Hz, (void **)&buffer[index]); index += 4;
			mb_put_binary_short(true, mbarirange->Points_per_LOS, (void **)&buffer[index]); index += 2;
			mb_put_binary_short(true, mbarirange->ScannerType, (void **)&buffer[index]); index += 2;
			mb_put_binary_short(true, mbarirange->lineAccelX, (void **)&buffer[index]); index += 2;
			mb_put_binary_short(true, mbarirange->lineAccelY, (void **)&buffer[index]); index += 2;
			mb_put_binary_short(true, mbarirange->lineAccelZ, (void **)&buffer[index]); index += 2;
			mb_put_binary_short(true, mbarirange->lineIndex, (void **)&buffer[index]); index += 2;
			mb_put_binary_short(true, mbarirange->RowNumber, (void **)&buffer[index]); index += 2;
			mb_put_binary_short(true, mbarirange->SHGAmplitudeAv, (void **)&buffer[index]); index += 2;
			mb_put_binary_int(true, mbarirange->R_Max, (void **)&buffer[index]); index += 4;
			mb_put_binary_int(true, mbarirange->I_Max, (void **)&buffer[index]); index += 4;
			mb_put_binary_int(true, mbarirange->R_Auto, (void **)&buffer[index]); index += 4;
			mb_put_binary_int(true, mbarirange->I_Auto, (void **)&buffer[index]); index += 4;
			mb_put_binary_int(true, mbarirange->R_Mode, (void **)&buffer[index]); index += 4;
			mb_put_binary_int(true, mbarirange->I_Mode, (void **)&buffer[index]); index += 4;
			buffer[index] = mbarirange->I_Good; index++;
			buffer[index] = mbarirange->I_Low; index++;
			buffer[index] = mbarirange->I_High; index++;
			buffer[index] = mbarirange->I_Spare; index++;
			mb_put_binary_int(true, mbarirange->R_offset, (void **)&buffer[index]); index += 4;
			mb_put_binary_int(true, mbarirange->I_offset, (void **)&buffer[index]); index += 4;			
			mb_put_binary_int(true, mbarirange->AZ_offset, (void **)&buffer[index]); index += 4;
			mb_put_binary_double(true, mbarirange->time_d, (void **)&buffer[index]); index += 8;
			mb_put_binary_double(true, mbarirange->navlon, (void **)&buffer[index]); index += 8;
			mb_put_binary_double(true, mbarirange->navlat, (void **)&buffer[index]); index += 8;
			mb_put_binary_double(true, mbarirange->sensordepth, (void **)&buffer[index]); index += 8;
			mb_put_binary_double(true, mbarirange->speed, (void **)&buffer[index]); index += 8;
			mb_put_binary_double(true, mbarirange->heading, (void **)&buffer[index]); index += 8;
			mb_put_binary_double(true, mbarirange->roll, (void **)&buffer[index]); index += 8;
			mb_put_binary_double(true, mbarirange->pitch, (void **)&buffer[index]); index += 8;
			mb_put_binary_int(true, mbarirange->num_soundings, (void **)&buffer[index]); index += 4;
//fprintf(stderr, "%s:%d:%s: index:%d\n", __FILE__, __LINE__, __FUNCTION__, index);
  
			for (int isounding = 0; isounding < mbarirange->num_soundings; isounding++) 
				{
				struct mbsys_3ddwissl2_sounding_struct *sounding = &(mbarirange->soundings[isounding]);
				
				mb_put_binary_short(true, sounding->pulse_id, (void **)&buffer[index]); index += 2;
				mb_put_binary_short(true, sounding->sounding_id, (void **)&buffer[index]); index += 2;
				mb_put_binary_float(true, sounding->time_offset, (void **)&buffer[index]); index += 4;
				mb_put_binary_float(true, sounding->acrosstrack_offset, (void **)&buffer[index]); index += 4;
				mb_put_binary_float(true, sounding->alongtrack_offset, (void **)&buffer[index]); index += 4;
				mb_put_binary_float(true, sounding->sensordepth_offset, (void **)&buffer[index]); index += 4;
				mb_put_binary_float(true, sounding->heading_offset, (void **)&buffer[index]); index += 4;
				mb_put_binary_float(true, sounding->pitch_offset, (void **)&buffer[index]); index += 4;
				mb_put_binary_float(true, sounding->range, (void **)&buffer[index]); index += 4;
				mb_put_binary_float(true, sounding->angle_az, (void **)&buffer[index]); index += 4;
				mb_put_binary_float(true, sounding->angle_el, (void **)&buffer[index]); index += 4;
				mb_put_binary_short(true, sounding->intensity, (void **)&buffer[index]); index += 2;
				buffer[index] = sounding->class; index++;
				buffer[index] = sounding->beamflag; index++;
				mb_put_binary_float(true, sounding->acrosstrack, (void **)&buffer[index]); index += 4;
				mb_put_binary_float(true, sounding->alongtrack, (void **)&buffer[index]); index += 4;
				mb_put_binary_float(true, sounding->depth, (void **)&buffer[index]); index += 4;
				}

      /* write data record from buffer */
      size_t write_len = mbarirange->SizeBytes;
      status = mb_fileio_put(verbose, mbio_ptr, (void *)buffer, &write_len, error);
			}
  	}


  if (verbose >= 2)
    {
    fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       error:      %d\n", *error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:  %d\n", status);
    }

  return status;
} /* mbr_3dwissl2_wr_data */
/*--------------------------------------------------------------------*/
int mbr_wt_3dwissl2
(
  int verbose,
  void *mbio_ptr,
  void *store_ptr,
  int *error
)
{
  if (verbose >= 2)
    {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
    fprintf(stderr, "dbg2       mbio_ptr:   %p\n", (void *)mbio_ptr);
    fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
    }

  /* check for non-null pointers */
  assert(mbio_ptr != NULL);
  assert(store_ptr != NULL);

  /* write next data to file */
  const int status = mbr_3dwissl2_wr_data(verbose, mbio_ptr, store_ptr, error);

  if (verbose >= 2)
    {
    fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       error:      %d\n", *error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:  %d\n", status);
    }

  return status;
} /* mbr_wt_3dwissl2 */
/*--------------------------------------------------------------------*/
int mbr_register_3dwissl2
(
  int verbose,
  void *mbio_ptr,
  int *error
)
{
  if (verbose >= 2)
    {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
    }

  /* check for non-null structure pointers */
  assert(mbio_ptr != NULL);

  /* get mb_io_ptr */
  struct mb_io_struct *mb_io_ptr = (struct mb_io_struct *)mbio_ptr;

  /* set format info parameters */
  const int status = mbr_info_3dwissl2(verbose,
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
    &mb_io_ptr->platform_source,
    &mb_io_ptr->nav_source,
    &mb_io_ptr->sensordepth_source,
    &mb_io_ptr->heading_source,
    &mb_io_ptr->attitude_source,
    &mb_io_ptr->svp_source,
    &mb_io_ptr->beamwidth_xtrack,
    &mb_io_ptr->beamwidth_ltrack,
    error);

  /* set format and system specific function pointers */
  mb_io_ptr->mb_io_format_alloc = &mbr_alm_3dwissl2;
  mb_io_ptr->mb_io_format_free = &mbr_dem_3dwissl2;
  mb_io_ptr->mb_io_store_alloc = &mbsys_3ddwissl2_alloc;
  mb_io_ptr->mb_io_store_free = &mbsys_3ddwissl2_deall;
  mb_io_ptr->mb_io_read_ping = &mbr_rt_3dwissl2;
  mb_io_ptr->mb_io_write_ping = &mbr_wt_3dwissl2;
  mb_io_ptr->mb_io_dimensions = &mbsys_3ddwissl2_dimensions;
  mb_io_ptr->mb_io_preprocess = &mbsys_3ddwissl2_preprocess;
  mb_io_ptr->mb_io_sensorhead = &mbsys_3ddwissl2_sensorhead;
  mb_io_ptr->mb_io_extract = &mbsys_3ddwissl2_extract;
  mb_io_ptr->mb_io_insert = &mbsys_3ddwissl2_insert;
  mb_io_ptr->mb_io_extract_nav = &mbsys_3ddwissl2_extract_nav;
  mb_io_ptr->mb_io_insert_nav = &mbsys_3ddwissl2_insert_nav;
  mb_io_ptr->mb_io_extract_altitude = &mbsys_3ddwissl2_extract_altitude;
  mb_io_ptr->mb_io_insert_altitude = NULL;
  mb_io_ptr->mb_io_extract_svp = &mbsys_3ddwissl2_extract_svp;
  mb_io_ptr->mb_io_insert_svp = &mbsys_3ddwissl2_insert_svp;
  mb_io_ptr->mb_io_ttimes = &mbsys_3ddwissl2_ttimes;
  mb_io_ptr->mb_io_detects = &mbsys_3ddwissl2_detects;
  mb_io_ptr->mb_io_copyrecord = &mbsys_3ddwissl2_copy;
  mb_io_ptr->mb_io_extract_rawss = NULL;
  mb_io_ptr->mb_io_insert_rawss = NULL;

  if (verbose >= 2)
    {
    fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       system:             %d\n", mb_io_ptr->system);
    fprintf(stderr, "dbg2       beams_bath_max:     %d\n", mb_io_ptr->beams_bath_max);
    fprintf(stderr, "dbg2       beams_amp_max:      %d\n", mb_io_ptr->beams_amp_max);
    fprintf(stderr, "dbg2       pixels_ss_max:      %d\n", mb_io_ptr->pixels_ss_max);
    fprintf(stderr, "dbg2       format_name:        %s\n", mb_io_ptr->format_name);
    fprintf(stderr, "dbg2       system_name:        %s\n", mb_io_ptr->system_name);
    fprintf(stderr, "dbg2       format_description: %s\n", mb_io_ptr->format_description);
    fprintf(stderr, "dbg2       numfile:            %d\n", mb_io_ptr->numfile);
    fprintf(stderr, "dbg2       filetype:           %d\n", mb_io_ptr->filetype);
    fprintf(stderr, "dbg2       variable_beams:     %d\n", mb_io_ptr->variable_beams);
    fprintf(stderr, "dbg2       traveltime:         %d\n", mb_io_ptr->traveltime);
    fprintf(stderr, "dbg2       beam_flagging:      %d\n", mb_io_ptr->beam_flagging);
    fprintf(stderr, "dbg2       platform_source:    %d\n", mb_io_ptr->platform_source);
    fprintf(stderr, "dbg2       nav_source:         %d\n", mb_io_ptr->nav_source);
    fprintf(stderr, "dbg2       sensordepth_source: %d\n", mb_io_ptr->nav_source);
    fprintf(stderr, "dbg2       heading_source:     %d\n", mb_io_ptr->heading_source);
    fprintf(stderr, "dbg2       attitude_source:    %d\n", mb_io_ptr->attitude_source);
    fprintf(stderr, "dbg2       svp_source:         %d\n", mb_io_ptr->svp_source);
    fprintf(stderr, "dbg2       beamwidth_xtrack:   %f\n", mb_io_ptr->beamwidth_xtrack);
    fprintf(stderr, "dbg2       beamwidth_ltrack:   %f\n", mb_io_ptr->beamwidth_ltrack);
    fprintf(stderr, "dbg2       format_alloc:       %p\n",
      (void *)mb_io_ptr->mb_io_format_alloc);
    fprintf(stderr, "dbg2       format_free:        %p\n",
      (void *)mb_io_ptr->mb_io_format_free);
    fprintf(stderr, "dbg2       store_alloc:        %p\n",
      (void *)mb_io_ptr->mb_io_store_alloc);
    fprintf(stderr, "dbg2       store_free:         %p\n", (void *)mb_io_ptr->mb_io_store_free);
    fprintf(stderr, "dbg2       read_ping:          %p\n", (void *)mb_io_ptr->mb_io_read_ping);
    fprintf(stderr, "dbg2       write_ping:         %p\n", (void *)mb_io_ptr->mb_io_write_ping);
    fprintf(stderr, "dbg2       preprocess:         %p\n", (void *)mb_io_ptr->mb_io_preprocess);
    fprintf(stderr, "dbg2       extract:            %p\n", (void *)mb_io_ptr->mb_io_extract);
    fprintf(stderr, "dbg2       insert:             %p\n", (void *)mb_io_ptr->mb_io_insert);
    fprintf(stderr, "dbg2       extract_nav:        %p\n",
      (void *)mb_io_ptr->mb_io_extract_nav);
    fprintf(stderr, "dbg2       insert_nav:         %p\n", (void *)mb_io_ptr->mb_io_insert_nav);
    fprintf(stderr,
      "dbg2       extract_altitude:   %p\n",
      (void *)mb_io_ptr->mb_io_extract_altitude);
    fprintf(stderr,
      "dbg2       insert_altitude:    %p\n",
      (void *)mb_io_ptr->mb_io_insert_altitude);
    fprintf(stderr, "dbg2       extract_svp:        %p\n",
      (void *)mb_io_ptr->mb_io_extract_svp);
    fprintf(stderr, "dbg2       insert_svp:         %p\n", (void *)mb_io_ptr->mb_io_insert_svp);
    fprintf(stderr, "dbg2       ttimes:             %p\n", (void *)mb_io_ptr->mb_io_ttimes);
    fprintf(stderr, "dbg2       detects:            %p\n", (void *)mb_io_ptr->mb_io_detects);
    fprintf(stderr,
      "dbg2       extract_rawss:      %p\n",
      (void *)mb_io_ptr->mb_io_extract_rawss);
    fprintf(stderr, "dbg2       insert_rawss:       %p\n",
      (void *)mb_io_ptr->mb_io_insert_rawss);
    fprintf(stderr, "dbg2       copyrecord:         %p\n", (void *)mb_io_ptr->mb_io_copyrecord);
    fprintf(stderr, "dbg2       error:              %d\n", *error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:         %d\n", status);
    }

  return status;
} /* mbr_register_3dwissl2 */
/*--------------------------------------------------------------------*/
