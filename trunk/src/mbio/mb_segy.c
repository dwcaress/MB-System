/*--------------------------------------------------------------------
 *    The MB-system:	mb_segy.c	5/25/2004
 *    $Id$
 *
 *    Copyright (c) 2004-2011 by
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
 * mb_segy.c includes the functions used to read, write, and use
 * edit save files.
 *
 * Author:	D. W. Caress
 * Date:	May 25, 2004
 *
 * $Log: mb_segy.c,v $
 * Revision 5.7  2008/09/13 06:08:09  caress
 * Updates to apply suggested patches to segy handling. Also fixes to remove compiler warnings.
 *
 * Revision 5.6  2008/07/10 06:43:40  caress
 * Preparing for 5.1.1beta20
 *
 * Revision 5.5  2006/11/10 22:36:04  caress
 * Working towards release 5.1.0
 *
 * Revision 5.4  2006/01/24 19:11:17  caress
 * Version 5.0.8 beta.
 *
 * Revision 5.3  2004/11/06 03:55:17  caress
 * Working to support the Reson 7k format.
 *
 * Revision 5.2  2004/09/16 19:02:34  caress
 * Changes to better support segy data.
 *
 * Revision 5.1  2004/07/27 19:44:38  caress
 * Working on handling subbottom data.
 *
 * Revision 5.0  2004/06/18 03:03:04  caress
 * Adding support for segy i/o.
 *
 *
 *
 *
 */

/* standard include files */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

/* mbio include files */
#include "../../include/mb_status.h"
#include "../../include/mb_define.h"
#include "../../include/mb_segy.h"
#include "../../include/mb_swap.h"

static char rcs_id[]="$Id$";

/*--------------------------------------------------------------------*/
/* 	function mb_segy_read_init opens an existing segy file for 
	reading. The file headers are returned */
int mb_segy_read_init(int verbose, char *segyfile, 
		void **mbsegyio_ptr,
		struct mb_segyasciiheader_struct *segyasciiheader,
		struct mb_segyfileheader_struct *segyfileheader,
		int *error)
{
  	char	*function_name = "mb_segy_read_init";
	int	status = MB_SUCCESS;
	struct mb_segyio_struct *mb_segyio_ptr;
	struct mb_segyasciiheader_struct *asciiheader;
	struct mb_segyfileheader_struct *fileheader;
	char	*buffer;
	int	index;
	int	i, j;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",function_name);
		fprintf(stderr,"dbg2  Revision id: %s\n",rcs_id);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:             %d\n",verbose);
		fprintf(stderr,"dbg2       segyfile:            %s\n",segyfile);
		fprintf(stderr,"dbg2       mbsegyio_ptr:        %lu\n",(size_t)mbsegyio_ptr);
		fprintf(stderr,"dbg2       asciiheader:         %lu\n",(size_t)segyasciiheader);
		fprintf(stderr,"dbg2       fileheader:          %lu\n",(size_t)segyfileheader);
		}

	/* allocate memory for mbsegyio descriptor */
	if ((status = mb_mallocd(verbose,__FILE__, __LINE__, sizeof(struct mb_segyio_struct),
				(void **)mbsegyio_ptr, error)) == MB_SUCCESS)
		{
		/* get structure */
		mb_segyio_ptr = (struct mb_segyio_struct *) *mbsegyio_ptr;

		/* zero the structure */
		memset(mb_segyio_ptr, 0, sizeof(struct mb_segyio_struct));
		
		/* allocate buffer memory */
		status = mb_mallocd(verbose,__FILE__, __LINE__, MB_SEGY_FILEHEADER_LENGTH,
				(void **)&(mb_segyio_ptr->buffer),error);
		if (status == MB_SUCCESS)
			mb_segyio_ptr->bufferalloc = MB_SEGY_FILEHEADER_LENGTH;
		else
			mb_segyio_ptr->bufferalloc = 0;
			
		/* set local pointers */
		asciiheader = &(mb_segyio_ptr->asciiheader);
		fileheader = &(mb_segyio_ptr->fileheader);
		}

	/* if ok then open the segy file */
	if (status == MB_SUCCESS)
		{
		/* open the segy file */
		strcpy(mb_segyio_ptr->segyfile, segyfile);
		if ((mb_segyio_ptr->fp = fopen(mb_segyio_ptr->segyfile,"r")) == NULL)
			{
			*error = MB_ERROR_OPEN_FAIL;
			status = MB_FAILURE;
			fprintf(stderr, "\nUnable to open segy file %s\n", mb_segyio_ptr->segyfile);
			}
			
		/* set asciiheader and fileheader flags */
		mb_segyio_ptr->asciiheader_set = MB_NO;
		mb_segyio_ptr->fileheader_set = MB_NO;
		}

	/* if ok then read file headers */
	if (status == MB_SUCCESS)
		{
		/* read ascii header */
		if (fread((char *) asciiheader,1,MB_SEGY_ASCIIHEADER_LENGTH,mb_segyio_ptr->fp) 
					!= MB_SEGY_ASCIIHEADER_LENGTH)
			{
			status = MB_FAILURE;
			*error = MB_ERROR_EOF;
			}
		else
			mb_segyio_ptr->asciiheader_set = MB_YES;

		/* read file header */
		if (fread(mb_segyio_ptr->buffer,1,MB_SEGY_FILEHEADER_LENGTH,mb_segyio_ptr->fp) 
					!= MB_SEGY_FILEHEADER_LENGTH)
			{
			status = MB_FAILURE;
			*error = MB_ERROR_EOF;
			}
		else
			mb_segyio_ptr->fileheader_set = MB_YES;

		/* extract file header data */
		if (status == MB_SUCCESS)
			{
			index = 0;
			buffer = mb_segyio_ptr->buffer;
			mb_get_binary_int(MB_NO, (void *) &(buffer[index]), &(fileheader->jobid)); index += 4;
			mb_get_binary_int(MB_NO, (void *) &(buffer[index]), &(fileheader->line)); index += 4;
			mb_get_binary_int(MB_NO, (void *) &(buffer[index]), &(fileheader->reel)); index += 4;
			mb_get_binary_short(MB_NO, (void *) &(buffer[index]), &(fileheader->channels)); index += 2;
			mb_get_binary_short(MB_NO, (void *) &(buffer[index]), &(fileheader->aux_channels)); index += 2;
			mb_get_binary_short(MB_NO, (void *) &(buffer[index]), &(fileheader->sample_interval)); index += 2;
			mb_get_binary_short(MB_NO, (void *) &(buffer[index]), &(fileheader->sample_interval_org)); index += 2;
			mb_get_binary_short(MB_NO, (void *) &(buffer[index]), &(fileheader->number_samples)); index += 2;
			mb_get_binary_short(MB_NO, (void *) &(buffer[index]), &(fileheader->number_samples_org)); index += 2;
			mb_get_binary_short(MB_NO, (void *) &(buffer[index]), &(fileheader->format)); index += 2;
			mb_get_binary_short(MB_NO, (void *) &(buffer[index]), &(fileheader->cdp_fold)); index += 2;
			mb_get_binary_short(MB_NO, (void *) &(buffer[index]), &(fileheader->trace_sort)); index += 2;
			mb_get_binary_short(MB_NO, (void *) &(buffer[index]), &(fileheader->vertical_sum)); index += 2;
			mb_get_binary_short(MB_NO, (void *) &(buffer[index]), &(fileheader->sweep_start)); index += 2;
			mb_get_binary_short(MB_NO, (void *) &(buffer[index]), &(fileheader->sweep_end)); index += 2;
			mb_get_binary_short(MB_NO, (void *) &(buffer[index]), &(fileheader->sweep_length)); index += 2;
			mb_get_binary_short(MB_NO, (void *) &(buffer[index]), &(fileheader->sweep_type)); index += 2;
			mb_get_binary_short(MB_NO, (void *) &(buffer[index]), &(fileheader->sweep_trace)); index += 2;
			mb_get_binary_short(MB_NO, (void *) &(buffer[index]), &(fileheader->sweep_taper_start)); index += 2;
			mb_get_binary_short(MB_NO, (void *) &(buffer[index]), &(fileheader->sweep_taper_end)); index += 2;
			mb_get_binary_short(MB_NO, (void *) &(buffer[index]), &(fileheader->sweep_taper)); index += 2;
			mb_get_binary_short(MB_NO, (void *) &(buffer[index]), &(fileheader->correlated)); index += 2;
			mb_get_binary_short(MB_NO, (void *) &(buffer[index]), &(fileheader->binary_gain)); index += 2;
			mb_get_binary_short(MB_NO, (void *) &(buffer[index]), &(fileheader->amplitude)); index += 2;
			mb_get_binary_short(MB_NO, (void *) &(buffer[index]), &(fileheader->units)); index += 2;
			mb_get_binary_short(MB_NO, (void *) &(buffer[index]), &(fileheader->impulse_polarity)); index += 2;
			mb_get_binary_short(MB_NO, (void *) &(buffer[index]), &(fileheader->vibrate_polarity)); index += 2;
			mb_get_binary_short(MB_NO, (void *) &(buffer[index]), &(fileheader->domain)); index += 2;
			for (i=0;i<238;i++)
				{
				fileheader->extra[i] = buffer[index]; index++;
				}
			mb_get_binary_short(MB_NO, (void *) &(buffer[index]), &(fileheader->rev)); index += 2;
			mb_get_binary_short(MB_NO, (void *) &(buffer[index]), &(fileheader->fixed_length)); index += 2;
			mb_get_binary_short(MB_NO, (void *) &(buffer[index]), &(fileheader->num_ext_headers)); index += 2;
			for (i=0;i<94;i++)
				{
				fileheader->extra[i] = buffer[index]; index++;
				}
			}

		/* copy the output structures if pointers provided */
		if (segyasciiheader != NULL)
			*segyasciiheader = *asciiheader;
		if (segyfileheader != NULL)
			*segyfileheader = *fileheader;
		}

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",function_name);
		fprintf(stderr,"dbg2  Revision id: %s\n",rcs_id);
		fprintf(stderr,"dbg2  Return value:\n");
		fprintf(stderr,"dbg2       asciiheader:         %lu\n",(size_t)asciiheader);
		fprintf(stderr,"dbg2       fileheader:          %lu\n",(size_t)fileheader);
		for (j=0;j<40;j++)
			{
			fprintf(stderr,"dbg2       asciiheader[%d]:",j);
			for (i=0;i<80;j++)
				fprintf(stderr,"%c",asciiheader->line[j][i]);
			fprintf(stderr,"\n");
			}
		fprintf(stderr,"dbg2       jobid:               %d\n",fileheader->jobid);
		fprintf(stderr,"dbg2       line:                %d\n",fileheader->line);
		fprintf(stderr,"dbg2       reel:                %d\n",fileheader->reel);
		fprintf(stderr,"dbg2       channels:            %d\n",fileheader->channels);
		fprintf(stderr,"dbg2       aux_channels:        %d\n",fileheader->aux_channels);
		fprintf(stderr,"dbg2       sample_interval:     %d\n",fileheader->sample_interval);
		fprintf(stderr,"dbg2       sample_interval_org: %d\n",fileheader->sample_interval_org);
		fprintf(stderr,"dbg2       number_samples:      %d\n",fileheader->number_samples);
		fprintf(stderr,"dbg2       number_samples_org:  %d\n",fileheader->number_samples_org);
		fprintf(stderr,"dbg2       format:              %d\n",fileheader->format);
		fprintf(stderr,"dbg2       cdp_fold:            %d\n",fileheader->cdp_fold);
		fprintf(stderr,"dbg2       trace_sort:          %d\n",fileheader->trace_sort);
		fprintf(stderr,"dbg2       vertical_sum:        %d\n",fileheader->vertical_sum);
		fprintf(stderr,"dbg2       sweep_start:         %d\n",fileheader->sweep_start);
		fprintf(stderr,"dbg2       sweep_end:           %d\n",fileheader->sweep_end);
		fprintf(stderr,"dbg2       sweep_length:        %d\n",fileheader->sweep_length);
		fprintf(stderr,"dbg2       sweep_type:          %d\n",fileheader->sweep_type);
		fprintf(stderr,"dbg2       sweep_trace:         %d\n",fileheader->sweep_trace);
		fprintf(stderr,"dbg2       sweep_taper_start:   %d\n",fileheader->sweep_taper_start);
		fprintf(stderr,"dbg2       sweep_taper_end:     %d\n",fileheader->sweep_taper_end);
		fprintf(stderr,"dbg2       sweep_taper:         %d\n",fileheader->sweep_taper);
		fprintf(stderr,"dbg2       correlated:          %d\n",fileheader->correlated);
		fprintf(stderr,"dbg2       binary_gain:         %d\n",fileheader->binary_gain);
		fprintf(stderr,"dbg2       amplitude:           %d\n",fileheader->amplitude);
		fprintf(stderr,"dbg2       units:               %d\n",fileheader->units);
		fprintf(stderr,"dbg2       impulse_polarity:    %d\n",fileheader->impulse_polarity);
		fprintf(stderr,"dbg2       vibrate_polarity:    %d\n",fileheader->vibrate_polarity);
		fprintf(stderr,"dbg2       domain:              %d\n",fileheader->domain);
		for (i=0;i<238;i++)
			fprintf(stderr,"dbg2       extra[%d]:          %d\n",i,fileheader->extra[i]);
		fprintf(stderr,"dbg2       SEG Y format rev:     %d\n",fileheader->rev);
		fprintf(stderr,"dbg2       fixed_length flag:    %d\n",fileheader->fixed_length);
		fprintf(stderr,"dbg2       num extended headers: %d\n",fileheader->num_ext_headers);
		for (i=238;i<238+94;i++)
			fprintf(stderr,"dbg2       extra[%d]:          %d\n",i,fileheader->extra[i]);
		fprintf(stderr,"dbg2       fp:            %lu\n",(size_t)mb_segyio_ptr->fp);
		fprintf(stderr,"dbg2       error:         %d\n",*error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:       %d\n",status);
		}

	/* return success */
	return(status);
}

/*--------------------------------------------------------------------*/
/* 	function mb_segy_write_init opens a new segy file for 
	writing. The file headers are inserted. */
int mb_segy_write_init(int verbose, char *segyfile, 
		struct mb_segyasciiheader_struct *asciiheader,
		struct mb_segyfileheader_struct *fileheader,
		void **mbsegyio_ptr,
		int *error)
{
  	char	*function_name = "mb_segy_write_init";
	int	status = MB_SUCCESS;
	struct mb_segyio_struct *mb_segyio_ptr;
	char	*buffer;
	int	index;
	int	i, j;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",function_name);
		fprintf(stderr,"dbg2  Revision id: %s\n",rcs_id);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:             %d\n",verbose);
		fprintf(stderr,"dbg2       segyfile:            %s\n",segyfile);
		fprintf(stderr,"dbg2       asciiheader:         %lu\n",(size_t)asciiheader);
		fprintf(stderr,"dbg2       fileheader:          %lu\n",(size_t)fileheader);
		if (asciiheader != NULL)
		for (j=0;j<40;j++)
			{
			fprintf(stderr,"dbg2       asciiheader[%d]:",j);
			for (i=0;i<80;j++)
				fprintf(stderr,"%c",asciiheader->line[j][i]);
			fprintf(stderr,"\n");
			}
		if (fileheader != NULL)
			{
			fprintf(stderr,"dbg2       jobid:               %d\n",fileheader->jobid);
			fprintf(stderr,"dbg2       line:                %d\n",fileheader->line);
			fprintf(stderr,"dbg2       reel:                %d\n",fileheader->reel);
			fprintf(stderr,"dbg2       channels:            %d\n",fileheader->channels);
			fprintf(stderr,"dbg2       aux_channels:        %d\n",fileheader->aux_channels);
			fprintf(stderr,"dbg2       sample_interval:     %d\n",fileheader->sample_interval);
			fprintf(stderr,"dbg2       sample_interval_org: %d\n",fileheader->sample_interval_org);
			fprintf(stderr,"dbg2       number_samples:      %d\n",fileheader->number_samples);
			fprintf(stderr,"dbg2       number_samples_org:  %d\n",fileheader->number_samples_org);
			fprintf(stderr,"dbg2       format:              %d\n",fileheader->format);
			fprintf(stderr,"dbg2       cdp_fold:            %d\n",fileheader->cdp_fold);
			fprintf(stderr,"dbg2       trace_sort:          %d\n",fileheader->trace_sort);
			fprintf(stderr,"dbg2       vertical_sum:        %d\n",fileheader->vertical_sum);
			fprintf(stderr,"dbg2       sweep_start:         %d\n",fileheader->sweep_start);
			fprintf(stderr,"dbg2       sweep_end:           %d\n",fileheader->sweep_end);
			fprintf(stderr,"dbg2       sweep_length:        %d\n",fileheader->sweep_length);
			fprintf(stderr,"dbg2       sweep_type:          %d\n",fileheader->sweep_type);
			fprintf(stderr,"dbg2       sweep_trace:         %d\n",fileheader->sweep_trace);
			fprintf(stderr,"dbg2       sweep_taper_start:   %d\n",fileheader->sweep_taper_start);
			fprintf(stderr,"dbg2       sweep_taper_end:     %d\n",fileheader->sweep_taper_end);
			fprintf(stderr,"dbg2       sweep_taper:         %d\n",fileheader->sweep_taper);
			fprintf(stderr,"dbg2       correlated:          %d\n",fileheader->correlated);
			fprintf(stderr,"dbg2       binary_gain:         %d\n",fileheader->binary_gain);
			fprintf(stderr,"dbg2       amplitude:           %d\n",fileheader->amplitude);
			fprintf(stderr,"dbg2       units:               %d\n",fileheader->units);
			fprintf(stderr,"dbg2       impulse_polarity:    %d\n",fileheader->impulse_polarity);
			fprintf(stderr,"dbg2       vibrate_polarity:    %d\n",fileheader->vibrate_polarity);
			fprintf(stderr,"dbg2       domain:              %d\n",fileheader->domain);
			for (i=0;i<338;i++)
				fprintf(stderr,"dbg2       extra[%d]::          %d",i,fileheader->extra[i]);
			}
		fprintf(stderr,"dbg2       mbsegyio_ptr:        %lu\n",(size_t)mbsegyio_ptr);
		}

	/* allocate memory for mbsegyio descriptor */
	if ((status = mb_mallocd(verbose,__FILE__, __LINE__, sizeof(struct mb_segyio_struct),
				(void **)mbsegyio_ptr, error)) == MB_SUCCESS)
		{
		/* get structure */
		mb_segyio_ptr = (struct mb_segyio_struct *) *mbsegyio_ptr;

		/* zero the structure */
		memset(mb_segyio_ptr, 0, sizeof(struct mb_segyio_struct));
		
		/* allocate buffer memory */
		status = mb_mallocd(verbose,__FILE__, __LINE__, MB_SEGY_FILEHEADER_LENGTH,
				(void **)&(mb_segyio_ptr->buffer),error);
		if (status == MB_SUCCESS)
			mb_segyio_ptr->bufferalloc = MB_SEGY_FILEHEADER_LENGTH;
		else
			mb_segyio_ptr->bufferalloc = 0;
		}

	/* if ok then open the segy file */
	if (status == MB_SUCCESS)
		{
		/* open the segy file */
		strcpy(mb_segyio_ptr->segyfile, segyfile);
		if ((mb_segyio_ptr->fp = fopen(mb_segyio_ptr->segyfile,"w")) == NULL)
			{
			*error = MB_ERROR_OPEN_FAIL;
			status = MB_FAILURE;
			fprintf(stderr, "\nUnable to open segy file %s\n", mb_segyio_ptr->segyfile);
			}
			
		/* set asciiheader and fileheader flags */
		mb_segyio_ptr->asciiheader_set = MB_NO;
		mb_segyio_ptr->fileheader_set = MB_NO;
		}

	/* handle the asciiheader structure */
	if (status == MB_SUCCESS && asciiheader != NULL)
		{
		/* copy the asciiheader structure */
		mb_segyio_ptr->asciiheader = *asciiheader;

		/* if ok then write ascii header */
		if (fwrite(asciiheader,1,MB_SEGY_ASCIIHEADER_LENGTH,mb_segyio_ptr->fp) 
					!= MB_SEGY_ASCIIHEADER_LENGTH)
			{
			status = MB_FAILURE;
			*error = MB_ERROR_WRITE_FAIL;
			}
		else
			mb_segyio_ptr->asciiheader_set = MB_YES;
		}

	/* handle the fileheader structure */
	if (status == MB_SUCCESS && fileheader != NULL)
		{
		/* copy the fileheader structure */
		mb_segyio_ptr->fileheader = *fileheader;

		/* insert file header */
		index = 0;
		buffer = mb_segyio_ptr->buffer;
		mb_put_binary_int(MB_NO, fileheader->jobid, (void *) &(buffer[index])); index += 4;
		mb_put_binary_int(MB_NO, fileheader->line, (void *) &(buffer[index])); index += 4;
		mb_put_binary_int(MB_NO, fileheader->reel, (void *) &(buffer[index])); index += 4;
		mb_put_binary_short(MB_NO, fileheader->channels, (void *) &(buffer[index])); index += 2;
		mb_put_binary_short(MB_NO, fileheader->aux_channels, (void *) &(buffer[index])); index += 2;
		mb_put_binary_short(MB_NO, fileheader->sample_interval, (void *) &(buffer[index])); index += 2;
		mb_put_binary_short(MB_NO, fileheader->sample_interval_org, (void *) &(buffer[index])); index += 2;
		mb_put_binary_short(MB_NO, fileheader->number_samples, (void *) &(buffer[index])); index += 2;
		mb_put_binary_short(MB_NO, fileheader->number_samples_org, (void *) &(buffer[index])); index += 2;
		mb_put_binary_short(MB_NO, fileheader->format, (void *) &(buffer[index])); index += 2;
		mb_put_binary_short(MB_NO, fileheader->cdp_fold, (void *) &(buffer[index])); index += 2;
		mb_put_binary_short(MB_NO, fileheader->trace_sort, (void *) &(buffer[index])); index += 2;
		mb_put_binary_short(MB_NO, fileheader->vertical_sum, (void *) &(buffer[index])); index += 2;
		mb_put_binary_short(MB_NO, fileheader->sweep_start, (void *) &(buffer[index])); index += 2;
		mb_put_binary_short(MB_NO, fileheader->sweep_end, (void *) &(buffer[index])); index += 2;
		mb_put_binary_short(MB_NO, fileheader->sweep_length, (void *) &(buffer[index])); index += 2;
		mb_put_binary_short(MB_NO, fileheader->sweep_type, (void *) &(buffer[index])); index += 2;
		mb_put_binary_short(MB_NO, fileheader->sweep_trace, (void *) &(buffer[index])); index += 2;
		mb_put_binary_short(MB_NO, fileheader->sweep_taper_start, (void *) &(buffer[index])); index += 2;
		mb_put_binary_short(MB_NO, fileheader->sweep_taper_end, (void *) &(buffer[index])); index += 2;
		mb_put_binary_short(MB_NO, fileheader->sweep_taper, (void *) &(buffer[index])); index += 2;
		mb_put_binary_short(MB_NO, fileheader->correlated, (void *) &(buffer[index])); index += 2;
		mb_put_binary_short(MB_NO, fileheader->binary_gain, (void *) &(buffer[index])); index += 2;
		mb_put_binary_short(MB_NO, fileheader->amplitude, (void *) &(buffer[index])); index += 2;
		mb_put_binary_short(MB_NO, fileheader->units, (void *) &(buffer[index])); index += 2;
		mb_put_binary_short(MB_NO, fileheader->impulse_polarity, (void *) &(buffer[index])); index += 2;
		mb_put_binary_short(MB_NO, fileheader->vibrate_polarity, (void *) &(buffer[index])); index += 2;
		mb_put_binary_short(MB_NO, fileheader->domain, (void *) &(buffer[index])); index += 2;
		for (i=0;i<338;i++)
			{
			buffer[index] = fileheader->extra[i]; index++;
			}

		/* write file header */
		if (fwrite(buffer,1,MB_SEGY_FILEHEADER_LENGTH,mb_segyio_ptr->fp) 
					!= MB_SEGY_FILEHEADER_LENGTH)
			{
			status = MB_FAILURE;
			*error = MB_ERROR_WRITE_FAIL;
			}
		else
			mb_segyio_ptr->fileheader_set = MB_YES;
		}

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",function_name);
		fprintf(stderr,"dbg2  Revision id: %s\n",rcs_id);
		fprintf(stderr,"dbg2  Return value:\n");
		fprintf(stderr,"dbg2       error:         %d\n",*error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:       %d\n",status);
		}

	/* return success */
	return(status);
}

/*--------------------------------------------------------------------*/
/* 	function mb_segy_close closes a segy file that was opened 
	for either reading or writing. */
int mb_segy_close(int verbose,void **mbsegyio_ptr, int *error)
{
  	char	*function_name = "mb_segy_close";
	int	status = MB_SUCCESS;
	struct mb_segyio_struct *mb_segyio_ptr;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",function_name);
		fprintf(stderr,"dbg2  Revision id: %s\n",rcs_id);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:     %d\n",verbose);
		fprintf(stderr,"dbg2       fp:          %lu\n",(size_t)mb_segyio_ptr->fp);
		}

	/* get pointer to segyio structure */
	mb_segyio_ptr = (struct mb_segyio_struct *) *mbsegyio_ptr;
	
	/* deallocate memory */
	if (mb_segyio_ptr->bufferalloc > 0)
		mb_freed(verbose,__FILE__, __LINE__,(void **)&(mb_segyio_ptr->buffer), error);
	if (mb_segyio_ptr->tracealloc > 0)
		mb_freed(verbose,__FILE__, __LINE__,(void **)&(mb_segyio_ptr->trace), error);

	/* close the segy file */
	fclose(mb_segyio_ptr->fp);
	mb_segyio_ptr->fp = NULL;

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",function_name);
		fprintf(stderr,"dbg2  Revision id: %s\n",rcs_id);
		fprintf(stderr,"dbg2  Return value:\n");
		fprintf(stderr,"dbg2       fp:            %lu\n",(size_t)mb_segyio_ptr->fp);
		fprintf(stderr,"dbg2       error:         %d\n",*error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:       %d\n",status);
		}

	/* return success */
	return(status);
}

/*--------------------------------------------------------------------*/
/* 	function mb_segy_read_trace reads a trace header and the trace
	data from an open segy file. The trace data array is passed
	in as a handle along with the allocated memory so that 
	additional memory can be allocated if necessary */
int mb_segy_read_trace(int verbose, void *mbsegyio_ptr, 
		struct mb_segytraceheader_struct *traceheaderptr,
		float **traceptr,
		int *error)
{
  	char	*function_name = "mb_segy_read_trace";
	int	status = MB_SUCCESS;
	struct mb_segyio_struct *mb_segyio_ptr;
	struct mb_segyfileheader_struct *fileheader;
	struct mb_segytraceheader_struct *traceheader;
	float	*trace;
	char	*buffer;
	int	index;
	int	bytes_per_sample;
	int	intval;
	short	shortval;
	int	i;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",function_name);
		fprintf(stderr,"dbg2  Revision id: %s\n",rcs_id);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:          %d\n",verbose);
		fprintf(stderr,"dbg2       mbsegyio_ptr:     %lu\n",(size_t)mbsegyio_ptr);
		fprintf(stderr,"dbg2       traceheaderptr:   %lu\n",(size_t)traceheaderptr);
		fprintf(stderr,"dbg2       traceptr:         %lu\n",(size_t)traceptr);
		fprintf(stderr,"dbg2       *traceptr:        %lu\n",(size_t)*traceptr);
		}

	/* get segyio pointer */
	mb_segyio_ptr = (struct mb_segyio_struct *) mbsegyio_ptr;
	fileheader = (struct mb_segyfileheader_struct *) &(mb_segyio_ptr->fileheader);
	traceheader = (struct mb_segytraceheader_struct *) &(mb_segyio_ptr->traceheader);
	
	/* make sure there is adequate memory in the buffer */
	if (mb_segyio_ptr->bufferalloc < MB_SEGY_TRACEHEADER_LENGTH)
		{
		/* allocate buffer memory */
		status = mb_reallocd(verbose, __FILE__, __LINE__, MB_SEGY_TRACEHEADER_LENGTH,
				(void **)(void **)&(mb_segyio_ptr->buffer),error);
		if (status == MB_SUCCESS)
			mb_segyio_ptr->bufferalloc = MB_SEGY_TRACEHEADER_LENGTH;
		else
			mb_segyio_ptr->bufferalloc = 0;
		}
	
	/* read trace header */
	if (status == MB_SUCCESS)
		{
		buffer = (char *) mb_segyio_ptr->buffer;
		if (fread(buffer,1,MB_SEGY_TRACEHEADER_LENGTH,mb_segyio_ptr->fp) 
					!= MB_SEGY_TRACEHEADER_LENGTH)
			{
			status = MB_FAILURE;
			*error = MB_ERROR_EOF;
			}
		}

	/* extract trace header data */
	if (status == MB_SUCCESS)
		{
		index = 0;
		mb_get_binary_int(MB_NO, (void *) &(buffer[index]), &(traceheader->seq_num)); index += 4;
		mb_get_binary_int(MB_NO, (void *) &(buffer[index]), &(traceheader->seq_reel)); index += 4;
		mb_get_binary_int(MB_NO, (void *) &(buffer[index]), &(traceheader->shot_num)); index += 4;
		mb_get_binary_int(MB_NO, (void *) &(buffer[index]), &(traceheader->shot_tr)); index += 4;
		mb_get_binary_int(MB_NO, (void *) &(buffer[index]), &(traceheader->espn)); index += 4;
		mb_get_binary_int(MB_NO, (void *) &(buffer[index]), &(traceheader->rp_num)); index += 4;
		mb_get_binary_int(MB_NO, (void *) &(buffer[index]), &(traceheader->rp_tr)); index += 4;
		mb_get_binary_short(MB_NO, (void *) &(buffer[index]), &(traceheader->trc_id)); index += 2;
		mb_get_binary_short(MB_NO, (void *) &(buffer[index]), &(traceheader->num_vstk)); index += 2;
		mb_get_binary_short(MB_NO, (void *) &(buffer[index]), &(traceheader->cdp_fold)); index += 2;
		mb_get_binary_short(MB_NO, (void *) &(buffer[index]), &(traceheader->use)); index += 2;
		mb_get_binary_int(MB_NO, (void *) &(buffer[index]), &(traceheader->range)); index += 4;
		mb_get_binary_int(MB_NO, (void *) &(buffer[index]), &(traceheader->grp_elev)); index += 4;
		mb_get_binary_int(MB_NO, (void *) &(buffer[index]), &(traceheader->src_elev)); index += 4;
		mb_get_binary_int(MB_NO, (void *) &(buffer[index]), &(traceheader->src_depth)); index += 4;
		mb_get_binary_int(MB_NO, (void *) &(buffer[index]), &(traceheader->grp_datum)); index += 4;
		mb_get_binary_int(MB_NO, (void *) &(buffer[index]), &(traceheader->src_datum)); index += 4;
		mb_get_binary_int(MB_NO, (void *) &(buffer[index]), &(traceheader->src_wbd)); index += 4;
		mb_get_binary_int(MB_NO, (void *) &(buffer[index]), &(traceheader->grp_wbd)); index += 4;
        	mb_get_binary_short(MB_NO, (void *) &(buffer[index]), &(traceheader->elev_scalar)); index += 2;
        	mb_get_binary_short(MB_NO, (void *) &(buffer[index]), &(traceheader->coord_scalar)); index += 2;
		mb_get_binary_int(MB_NO, (void *) &(buffer[index]), &(traceheader->src_long)); index += 4;
		mb_get_binary_int(MB_NO, (void *) &(buffer[index]), &(traceheader->src_lat)); index += 4;
		mb_get_binary_int(MB_NO, (void *) &(buffer[index]), &(traceheader->grp_long)); index += 4;
		mb_get_binary_int(MB_NO, (void *) &(buffer[index]), &(traceheader->grp_lat)); index += 4;
        	mb_get_binary_short(MB_NO, (void *) &(buffer[index]), &(traceheader->coord_units)); index += 2;
        	mb_get_binary_short(MB_NO, (void *) &(buffer[index]), &(traceheader->wvel)); index += 2;
        	mb_get_binary_short(MB_NO, (void *) &(buffer[index]), &(traceheader->sbvel)); index += 2;
        	mb_get_binary_short(MB_NO, (void *) &(buffer[index]), &(traceheader->src_up_vel)); index += 2;
        	mb_get_binary_short(MB_NO, (void *) &(buffer[index]), &(traceheader->grp_up_vel)); index += 2;
        	mb_get_binary_short(MB_NO, (void *) &(buffer[index]), &(traceheader->src_static)); index += 2;
        	mb_get_binary_short(MB_NO, (void *) &(buffer[index]), &(traceheader->grp_static)); index += 2;
        	mb_get_binary_short(MB_NO, (void *) &(buffer[index]), &(traceheader->tot_static)); index += 2;
        	mb_get_binary_short(MB_NO, (void *) &(buffer[index]), &(traceheader->laga)); index += 2;
		mb_get_binary_int(MB_NO, (void *) &(buffer[index]), &(traceheader->delay_mils)); index += 4;
        	mb_get_binary_short(MB_NO, (void *) &(buffer[index]), &(traceheader->smute_mils)); index += 2;
        	mb_get_binary_short(MB_NO, (void *) &(buffer[index]), &(traceheader->emute_mils)); index += 2;
        	mb_get_binary_short(MB_NO, (void *) &(buffer[index]), &(traceheader->nsamps)); index += 2;
        	mb_get_binary_short(MB_NO, (void *) &(buffer[index]), &(traceheader->si_micros)); index += 2;
		for (i=0;i<19;i++)
			{
        		mb_get_binary_short(MB_NO, (void *) &(buffer[index]), &(traceheader->other_1[i])); index += 2;
			}
        	mb_get_binary_short(MB_NO, (void *) &(buffer[index]), &(traceheader->year)); index += 2;
        	mb_get_binary_short(MB_NO, (void *) &(buffer[index]), &(traceheader->day_of_yr)); index += 2;
        	mb_get_binary_short(MB_NO, (void *) &(buffer[index]), &(traceheader->hour)); index += 2;
        	mb_get_binary_short(MB_NO, (void *) &(buffer[index]), &(traceheader->min)); index += 2;
        	mb_get_binary_short(MB_NO, (void *) &(buffer[index]), &(traceheader->sec)); index += 2;
        	mb_get_binary_short(MB_NO, (void *) &(buffer[index]), &(traceheader->mils)); index += 2;
        	mb_get_binary_short(MB_NO, (void *) &(buffer[index]), &(traceheader->tr_weight)); index += 2;
		for (i=0;i<5;i++)
			{
        		mb_get_binary_short(MB_NO, (void *) &(buffer[index]), &(traceheader->other_2[i])); index += 2;
			}
		mb_get_binary_float(MB_NO, (void *) &(buffer[index]), &(traceheader->delay)); index += 4;
        	mb_get_binary_float(MB_NO, (void *) &(buffer[index]), &(traceheader->smute_sec)); index += 4;
        	mb_get_binary_float(MB_NO, (void *) &(buffer[index]), &(traceheader->emute_sec)); index += 4;
        	mb_get_binary_float(MB_NO, (void *) &(buffer[index]), &(traceheader->si_secs)); index += 4;
        	mb_get_binary_float(MB_NO, (void *) &(buffer[index]), &(traceheader->wbt_secs)); index += 4;
		mb_get_binary_int(MB_NO, (void *) &(buffer[index]), &(traceheader->end_of_rp)); index += 4;
        	mb_get_binary_float(MB_NO, (void *) &(buffer[index]), &(traceheader->dummy1)); index += 4;
        	mb_get_binary_float(MB_NO, (void *) &(buffer[index]), &(traceheader->dummy2)); index += 4;
        	mb_get_binary_float(MB_NO, (void *) &(buffer[index]), &(traceheader->dummy3)); index += 4;
        	mb_get_binary_float(MB_NO, (void *) &(buffer[index]), &(traceheader->dummy4)); index += 4;
        	mb_get_binary_float(MB_NO, (void *) &(buffer[index]), &(traceheader->dummy5)); index += 4;
        	mb_get_binary_float(MB_NO, (void *) &(buffer[index]), &(traceheader->dummy6)); index += 4;
        	mb_get_binary_float(MB_NO, (void *) &(buffer[index]), &(traceheader->dummy7)); index += 4;
        	mb_get_binary_float(MB_NO, (void *) &(buffer[index]), &(traceheader->dummy8)); index += 4;
        	mb_get_binary_float(MB_NO, (void *) &(buffer[index]), &(traceheader->heading)); index += 4;
		}
	
	/* make sure there is adequate memory */
	if (status == MB_SUCCESS)
		{
		/* get bytes per sample */
 		if (fileheader->format == 5 || fileheader->format == 6 || fileheader->format == 11 || fileheader->format == 2)
       			{
			bytes_per_sample = 4;
			}
 		else if (fileheader->format == 1)
       			{
			bytes_per_sample = 4;
			}
 		else if (fileheader->format == 3)
       			{
			bytes_per_sample = 2;
			}
 		else if (fileheader->format == 8)
       			{
			bytes_per_sample = 1;
			}
		else
       			{
			bytes_per_sample = 4;
			}

		/* check buffer memory */
		if (mb_segyio_ptr->bufferalloc < bytes_per_sample * traceheader->nsamps)
			{
			/* allocate buffer memory */
			status = mb_reallocd(verbose, __FILE__, __LINE__, bytes_per_sample * traceheader->nsamps,
					(void **)&(mb_segyio_ptr->buffer),error);
			if (status == MB_SUCCESS)
				mb_segyio_ptr->bufferalloc = bytes_per_sample * traceheader->nsamps;
			else
				mb_segyio_ptr->bufferalloc = 0;
			}

		/* check trace memory */
		if (mb_segyio_ptr->tracealloc < sizeof(float) * traceheader->nsamps)
			{
			/* allocate trace memory */
			status = mb_reallocd(verbose, __FILE__, __LINE__, sizeof(float) * traceheader->nsamps,
					(void **)&(mb_segyio_ptr->trace),error);
			if (status == MB_SUCCESS)
				mb_segyio_ptr->tracealloc = sizeof(float) * traceheader->nsamps;
			else
				mb_segyio_ptr->tracealloc = 0;
			}
		}
	
	/* read trace */
	if (status == MB_SUCCESS)
		{
		buffer = mb_segyio_ptr->buffer;
		if (fread(buffer, 1, bytes_per_sample * traceheader->nsamps, mb_segyio_ptr->fp) 
					!= bytes_per_sample * traceheader->nsamps)
			{
			status = MB_FAILURE;
			*error = MB_ERROR_EOF;
			}
		}
	
	/* extract trace data */
	if (status == MB_SUCCESS)
		{
		trace = (float *) mb_segyio_ptr->trace;
		index = 0;
		for (i=0;i<traceheader->nsamps;i++)
			{
 			if (fileheader->format == 5 || fileheader->format == 6 || fileheader->format == 1)
       				{
				mb_get_binary_float(MB_NO, (void *) &(buffer[index]), &(trace[i])); 
				index += bytes_per_sample;
				}
 			else if (fileheader->format == 11)
       				{
				mb_get_binary_float(MB_YES, (void *) &(buffer[index]), &(trace[i])); 
				index += bytes_per_sample;
				}
 			else if (fileheader->format == 2)
       				{
				mb_get_binary_int(MB_NO, (void *) &(buffer[index]), &(intval)); 
				trace[i] = (float) intval;
				index += bytes_per_sample;
				}
 			else if (fileheader->format == 3)
       				{
				mb_get_binary_short(MB_NO, (void *) &(buffer[index]), &(shortval)); 
				trace[i] = (float) shortval;
				index += bytes_per_sample;
				}
 			else if (fileheader->format == 8)
       				{
				trace[i] = (float) buffer[i]; 
				index++;
				}
			}
		}
		
	/* set return pointers */
	if (status == MB_SUCCESS)
		{
		*traceheaderptr = *traceheader;
		*traceptr = trace;
		}

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",function_name);
		fprintf(stderr,"dbg2  Revision id: %s\n",rcs_id);
		fprintf(stderr,"dbg2  Return value:\n");
		fprintf(stderr,"dbg2       seq_num:       %d\n",traceheader->seq_num);
		fprintf(stderr,"dbg2       seq_reel:      %d\n",traceheader->seq_reel);
		fprintf(stderr,"dbg2       shot_num:      %d\n",traceheader->shot_num);
		fprintf(stderr,"dbg2       shot_tr:       %d\n",traceheader->shot_tr);
		fprintf(stderr,"dbg2       espn:          %d\n",traceheader->espn);
		fprintf(stderr,"dbg2       rp_num:        %d\n",traceheader->rp_num);
		fprintf(stderr,"dbg2       rp_tr:         %d\n",traceheader->rp_tr);
		fprintf(stderr,"dbg2       trc_id:        %d\n",traceheader->trc_id);
		fprintf(stderr,"dbg2       num_vstk:      %d\n",traceheader->num_vstk);
		fprintf(stderr,"dbg2       cdp_fold:      %d\n",traceheader->cdp_fold);
		fprintf(stderr,"dbg2       use:           %d\n",traceheader->use);
		fprintf(stderr,"dbg2       range:         %d\n",traceheader->range);
		fprintf(stderr,"dbg2       grp_elev:      %d\n",traceheader->grp_elev);
		fprintf(stderr,"dbg2       src_elev:      %d\n",traceheader->src_elev);
		fprintf(stderr,"dbg2       src_depth:     %d\n",traceheader->src_depth);
		fprintf(stderr,"dbg2       grp_datum:     %d\n",traceheader->grp_datum);
		fprintf(stderr,"dbg2       src_datum:     %d\n",traceheader->src_datum);
		fprintf(stderr,"dbg2       src_wbd:       %d\n",traceheader->src_wbd);
		fprintf(stderr,"dbg2       grp_wbd:       %d\n",traceheader->grp_wbd);
		fprintf(stderr,"dbg2       elev_scalar:   %d\n",traceheader->elev_scalar);
		fprintf(stderr,"dbg2       coord_scalar:  %d\n",traceheader->coord_scalar);
		fprintf(stderr,"dbg2       src_long:      %d\n",traceheader->src_long);
		fprintf(stderr,"dbg2       src_lat:       %d\n",traceheader->src_lat);
		fprintf(stderr,"dbg2       grp_long:      %d\n",traceheader->grp_long);
		fprintf(stderr,"dbg2       grp_lat:       %d\n",traceheader->grp_lat);
		fprintf(stderr,"dbg2       coord_units:   %d\n",traceheader->coord_units);
		fprintf(stderr,"dbg2       wvel:          %d\n",traceheader->wvel);
		fprintf(stderr,"dbg2       sbvel:         %d\n",traceheader->sbvel);
		fprintf(stderr,"dbg2       src_up_vel:    %d\n",traceheader->src_up_vel);
		fprintf(stderr,"dbg2       grp_up_vel:    %d\n",traceheader->grp_up_vel);
		fprintf(stderr,"dbg2       src_static:    %d\n",traceheader->src_static);
		fprintf(stderr,"dbg2       grp_static:    %d\n",traceheader->grp_static);
		fprintf(stderr,"dbg2       tot_static:    %d\n",traceheader->tot_static);
		fprintf(stderr,"dbg2       laga:          %d\n",traceheader->laga);
		fprintf(stderr,"dbg2       delay_mils:    %d\n",traceheader->delay_mils);
		fprintf(stderr,"dbg2       smute_mils:    %d\n",traceheader->smute_mils);
		fprintf(stderr,"dbg2       emute_mils:    %d\n",traceheader->emute_mils);
		fprintf(stderr,"dbg2       nsamps:        %d\n",traceheader->nsamps);
		fprintf(stderr,"dbg2       si_micros:     %d\n",traceheader->si_micros);
		fprintf(stderr,"dbg2       other_1[19]:   %d\n",traceheader->other_1[19]);
		fprintf(stderr,"dbg2       year:          %d\n",traceheader->year);
		fprintf(stderr,"dbg2       day_of_yr:     %d\n",traceheader->day_of_yr);
		fprintf(stderr,"dbg2       hour:          %d\n",traceheader->hour);
		fprintf(stderr,"dbg2       min:           %d\n",traceheader->min);
		fprintf(stderr,"dbg2       sec:           %d\n",traceheader->sec);
		fprintf(stderr,"dbg2       mils:          %d\n",traceheader->mils);
		fprintf(stderr,"dbg2       tr_weight:     %d\n",traceheader->tr_weight);
		fprintf(stderr,"dbg2       other_2[5]:    %d\n",traceheader->other_2[5]);
		fprintf(stderr,"dbg2       delay:         %f\n",traceheader->delay);
		fprintf(stderr,"dbg2       smute_sec:     %f\n",traceheader->smute_sec);
		fprintf(stderr,"dbg2       emute_sec:     %f\n",traceheader->emute_sec);
		fprintf(stderr,"dbg2       si_secs:       %f\n",traceheader->si_secs);
		fprintf(stderr,"dbg2       wbt_secs:      %f\n",traceheader->wbt_secs);
		fprintf(stderr,"dbg2       end_of_rp:     %d\n",traceheader->end_of_rp);
		fprintf(stderr,"dbg2       dummy1:        %f\n",traceheader->dummy1);
		fprintf(stderr,"dbg2       dummy2:        %f\n",traceheader->dummy2);
		fprintf(stderr,"dbg2       dummy3:        %f\n",traceheader->dummy3);
		fprintf(stderr,"dbg2       dummy4:        %f\n",traceheader->dummy4);
		fprintf(stderr,"dbg2       dummy5:        %f\n",traceheader->dummy5);
		fprintf(stderr,"dbg2       dummy6:        %f\n",traceheader->dummy6);
		fprintf(stderr,"dbg2       dummy7:        %f\n",traceheader->dummy7);
		fprintf(stderr,"dbg2       dummy8:        %f\n",traceheader->dummy8);
		fprintf(stderr,"dbg2       heading:       %f\n",traceheader->heading);
		for (i=0;i<traceheader->nsamps;i++)
			fprintf(stderr,"dbg2       trace[%d]:%f\n",i,trace[i]);
		fprintf(stderr,"dbg2       error:         %d\n",*error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:       %d\n",status);
		}

	/* return success */
	return(status);
}
/*--------------------------------------------------------------------*/
/* 	function mb_segy_write_trace writes a trace header and the trace
	data to an open segy file. The trace data array is passed
	in as a pointer */
int mb_segy_write_trace(int verbose, void *mbsegyio_ptr, 
		struct mb_segytraceheader_struct *traceheader,
		float *trace,
		int *error)
{
  	char	*function_name = "mb_segy_write_trace";
	int	status = MB_SUCCESS;
	struct mb_segyio_struct *mb_segyio_ptr;
	struct mb_segyasciiheader_struct *asciiheader;
	struct mb_segyfileheader_struct *fileheader;
	char	*buffer;
	int	index;
	int	bytes_per_sample;
	int	intval;
	short	shortval;
	int	i;

	/* get segyio pointer */
	mb_segyio_ptr = (struct mb_segyio_struct *) mbsegyio_ptr;
	asciiheader = &(mb_segyio_ptr->asciiheader);
	fileheader = &(mb_segyio_ptr->fileheader);

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",function_name);
		fprintf(stderr,"dbg2  Revision id: %s\n",rcs_id);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:       %d\n",verbose);
		fprintf(stderr,"dbg2       mbsegyio_ptr:  %lu\n",(size_t)mbsegyio_ptr);
		fprintf(stderr,"dbg2       traceheader:   %lu\n",(size_t)traceheader);
		fprintf(stderr,"dbg2       seq_num:       %d\n",traceheader->seq_num);
		fprintf(stderr,"dbg2       seq_reel:      %d\n",traceheader->seq_reel);
		fprintf(stderr,"dbg2       shot_num:      %d\n",traceheader->shot_num);
		fprintf(stderr,"dbg2       shot_tr:       %d\n",traceheader->shot_tr);
		fprintf(stderr,"dbg2       espn:          %d\n",traceheader->espn);
		fprintf(stderr,"dbg2       rp_num:        %d\n",traceheader->rp_num);
		fprintf(stderr,"dbg2       rp_tr:         %d\n",traceheader->rp_tr);
		fprintf(stderr,"dbg2       trc_id:        %d\n",traceheader->trc_id);
		fprintf(stderr,"dbg2       num_vstk:      %d\n",traceheader->num_vstk);
		fprintf(stderr,"dbg2       cdp_fold:      %d\n",traceheader->cdp_fold);
		fprintf(stderr,"dbg2       use:           %d\n",traceheader->use);
		fprintf(stderr,"dbg2       range:         %d\n",traceheader->range);
		fprintf(stderr,"dbg2       grp_elev:      %d\n",traceheader->grp_elev);
		fprintf(stderr,"dbg2       src_elev:      %d\n",traceheader->src_elev);
		fprintf(stderr,"dbg2       src_depth:     %d\n",traceheader->src_depth);
		fprintf(stderr,"dbg2       grp_datum:     %d\n",traceheader->grp_datum);
		fprintf(stderr,"dbg2       src_datum:     %d\n",traceheader->src_datum);
		fprintf(stderr,"dbg2       src_wbd:       %d\n",traceheader->src_wbd);
		fprintf(stderr,"dbg2       grp_wbd:       %d\n",traceheader->grp_wbd);
		fprintf(stderr,"dbg2       elev_scalar:   %d\n",traceheader->elev_scalar);
		fprintf(stderr,"dbg2       coord_scalar:  %d\n",traceheader->coord_scalar);
		fprintf(stderr,"dbg2       src_long:      %d\n",traceheader->src_long);
		fprintf(stderr,"dbg2       src_lat:       %d\n",traceheader->src_lat);
		fprintf(stderr,"dbg2       grp_long:      %d\n",traceheader->grp_long);
		fprintf(stderr,"dbg2       grp_lat:       %d\n",traceheader->grp_lat);
		fprintf(stderr,"dbg2       coord_units:   %d\n",traceheader->coord_units);
		fprintf(stderr,"dbg2       wvel:          %d\n",traceheader->wvel);
		fprintf(stderr,"dbg2       sbvel:         %d\n",traceheader->sbvel);
		fprintf(stderr,"dbg2       src_up_vel:    %d\n",traceheader->src_up_vel);
		fprintf(stderr,"dbg2       grp_up_vel:    %d\n",traceheader->grp_up_vel);
		fprintf(stderr,"dbg2       src_static:    %d\n",traceheader->src_static);
		fprintf(stderr,"dbg2       grp_static:    %d\n",traceheader->grp_static);
		fprintf(stderr,"dbg2       tot_static:    %d\n",traceheader->tot_static);
		fprintf(stderr,"dbg2       laga:          %d\n",traceheader->laga);
		fprintf(stderr,"dbg2       delay_mils:    %d\n",traceheader->delay_mils);
		fprintf(stderr,"dbg2       smute_mils:    %d\n",traceheader->smute_mils);
		fprintf(stderr,"dbg2       emute_mils:    %d\n",traceheader->emute_mils);
		fprintf(stderr,"dbg2       nsamps:        %d\n",traceheader->nsamps);
		fprintf(stderr,"dbg2       si_micros:     %d\n",traceheader->si_micros);
		fprintf(stderr,"dbg2       other_1[19]:   %d\n",traceheader->other_1[19]);
		fprintf(stderr,"dbg2       year:          %d\n",traceheader->year);
		fprintf(stderr,"dbg2       day_of_yr:     %d\n",traceheader->day_of_yr);
		fprintf(stderr,"dbg2       hour:          %d\n",traceheader->hour);
		fprintf(stderr,"dbg2       min:           %d\n",traceheader->min);
		fprintf(stderr,"dbg2       sec:           %d\n",traceheader->sec);
		fprintf(stderr,"dbg2       mils:          %d\n",traceheader->mils);
		fprintf(stderr,"dbg2       tr_weight:     %d\n",traceheader->tr_weight);
		fprintf(stderr,"dbg2       other_2[5]:    %d\n",traceheader->other_2[5]);
		fprintf(stderr,"dbg2       delay:         %f\n",traceheader->delay);
		fprintf(stderr,"dbg2       smute_sec:     %f\n",traceheader->smute_sec);
		fprintf(stderr,"dbg2       emute_sec:     %f\n",traceheader->emute_sec);
		fprintf(stderr,"dbg2       si_secs:       %f\n",traceheader->si_secs);
		fprintf(stderr,"dbg2       wbt_secs:      %f\n",traceheader->wbt_secs);
		fprintf(stderr,"dbg2       end_of_rp:     %d\n",traceheader->end_of_rp);
		fprintf(stderr,"dbg2       dummy1:        %f\n",traceheader->dummy1);
		fprintf(stderr,"dbg2       dummy2:        %f\n",traceheader->dummy2);
		fprintf(stderr,"dbg2       dummy3:        %f\n",traceheader->dummy3);
		fprintf(stderr,"dbg2       dummy4:        %f\n",traceheader->dummy4);
		fprintf(stderr,"dbg2       dummy5:        %f\n",traceheader->dummy5);
		fprintf(stderr,"dbg2       dummy6:        %f\n",traceheader->dummy6);
		fprintf(stderr,"dbg2       dummy7:        %f\n",traceheader->dummy7);
		fprintf(stderr,"dbg2       dummy8:        %f\n",traceheader->dummy8);
		fprintf(stderr,"dbg2       heading:       %f\n",traceheader->heading);
		for (i=0;i<traceheader->nsamps;i++)
			fprintf(stderr,"dbg2       trace[%d]:%f\n",i,trace[i]);
		}
		
	/* if asciiheader has not yet been written, write it */
	if (mb_segyio_ptr->asciiheader_set == MB_NO)
		{
		if (fwrite(asciiheader,1,MB_SEGY_ASCIIHEADER_LENGTH,mb_segyio_ptr->fp) 
					!= MB_SEGY_ASCIIHEADER_LENGTH)
			{
			status = MB_FAILURE;
			*error = MB_ERROR_WRITE_FAIL;
			}
		else
			mb_segyio_ptr->asciiheader_set = MB_YES;
		}
		
	/* if fileheader has not yet been written, write it */
	if (mb_segyio_ptr->fileheader_set == MB_NO)
		{
		/* make sure there is adequate memory in the buffer */
		if (mb_segyio_ptr->bufferalloc < MB_SEGY_FILEHEADER_LENGTH)
			{
			/* allocate buffer memory */
			status = mb_reallocd(verbose, __FILE__, __LINE__, MB_SEGY_FILEHEADER_LENGTH,
					(void **)&(mb_segyio_ptr->buffer),error);
			if (status == MB_SUCCESS)
				mb_segyio_ptr->bufferalloc = MB_SEGY_FILEHEADER_LENGTH;
			else
				mb_segyio_ptr->bufferalloc = 0;
			}

		index = 0;
		buffer = mb_segyio_ptr->buffer;
		mb_put_binary_int(MB_NO, fileheader->jobid, (void *) &(buffer[index])); index += 4;
		mb_put_binary_int(MB_NO, fileheader->line, (void *) &(buffer[index])); index += 4;
		mb_put_binary_int(MB_NO, fileheader->reel, (void *) &(buffer[index])); index += 4;
		mb_put_binary_short(MB_NO, fileheader->channels, (void *) &(buffer[index])); index += 2;
		mb_put_binary_short(MB_NO, fileheader->aux_channels, (void *) &(buffer[index])); index += 2;
		mb_put_binary_short(MB_NO, fileheader->sample_interval, (void *) &(buffer[index])); index += 2;
		mb_put_binary_short(MB_NO, fileheader->sample_interval_org, (void *) &(buffer[index])); index += 2;
		mb_put_binary_short(MB_NO, fileheader->number_samples, (void *) &(buffer[index])); index += 2;
		mb_put_binary_short(MB_NO, fileheader->number_samples_org, (void *) &(buffer[index])); index += 2;
		mb_put_binary_short(MB_NO, fileheader->format, (void *) &(buffer[index])); index += 2;
		mb_put_binary_short(MB_NO, fileheader->cdp_fold, (void *) &(buffer[index])); index += 2;
		mb_put_binary_short(MB_NO, fileheader->trace_sort, (void *) &(buffer[index])); index += 2;
		mb_put_binary_short(MB_NO, fileheader->vertical_sum, (void *) &(buffer[index])); index += 2;
		mb_put_binary_short(MB_NO, fileheader->sweep_start, (void *) &(buffer[index])); index += 2;
		mb_put_binary_short(MB_NO, fileheader->sweep_end, (void *) &(buffer[index])); index += 2;
		mb_put_binary_short(MB_NO, fileheader->sweep_length, (void *) &(buffer[index])); index += 2;
		mb_put_binary_short(MB_NO, fileheader->sweep_type, (void *) &(buffer[index])); index += 2;
		mb_put_binary_short(MB_NO, fileheader->sweep_trace, (void *) &(buffer[index])); index += 2;
		mb_put_binary_short(MB_NO, fileheader->sweep_taper_start, (void *) &(buffer[index])); index += 2;
		mb_put_binary_short(MB_NO, fileheader->sweep_taper_end, (void *) &(buffer[index])); index += 2;
		mb_put_binary_short(MB_NO, fileheader->sweep_taper, (void *) &(buffer[index])); index += 2;
		mb_put_binary_short(MB_NO, fileheader->correlated, (void *) &(buffer[index])); index += 2;
		mb_put_binary_short(MB_NO, fileheader->binary_gain, (void *) &(buffer[index])); index += 2;
		mb_put_binary_short(MB_NO, fileheader->amplitude, (void *) &(buffer[index])); index += 2;
		mb_put_binary_short(MB_NO, fileheader->units, (void *) &(buffer[index])); index += 2;
		mb_put_binary_short(MB_NO, fileheader->impulse_polarity, (void *) &(buffer[index])); index += 2;
		mb_put_binary_short(MB_NO, fileheader->vibrate_polarity, (void *) &(buffer[index])); index += 2;
		mb_put_binary_short(MB_NO, fileheader->domain, (void *) &(buffer[index])); index += 2;
		for (i=0;i<338;i++)
			{
			buffer[index] = fileheader->extra[i]; index++;
			}
		if (fwrite(buffer,1,MB_SEGY_FILEHEADER_LENGTH,mb_segyio_ptr->fp) 
					!= MB_SEGY_FILEHEADER_LENGTH)
			{
			status = MB_FAILURE;
			*error = MB_ERROR_WRITE_FAIL;
			}
		else
			mb_segyio_ptr->fileheader_set = MB_YES;
		}
	
	/* make sure there is adequate memory in the buffer */
	if (mb_segyio_ptr->bufferalloc < MB_SEGY_TRACEHEADER_LENGTH)
		{
		/* allocate buffer memory */
		status = mb_reallocd(verbose, __FILE__, __LINE__, MB_SEGY_TRACEHEADER_LENGTH,
				(void **)&(mb_segyio_ptr->buffer),error);
		if (status == MB_SUCCESS)
			mb_segyio_ptr->bufferalloc = MB_SEGY_TRACEHEADER_LENGTH;
		else
			mb_segyio_ptr->bufferalloc = 0;
		}
	
	/* make sure there is adequate memory */
	if (status == MB_SUCCESS)
		{
		/* get bytes per sample */
 		if (fileheader->format == 5 || fileheader->format == 6 || fileheader->format == 11 || fileheader->format == 2)
       			{
			bytes_per_sample = 4;
			}
 		else if (fileheader->format == 3)
       			{
			bytes_per_sample = 2;
			}
 		else if (fileheader->format == 8)
       			{
			bytes_per_sample = 1;
			}
		else
       			{
			bytes_per_sample = 4;
			}

		/* check buffer memory */
		if (mb_segyio_ptr->bufferalloc < bytes_per_sample * traceheader->nsamps)
			{
			/* allocate buffer memory */
			status = mb_reallocd(verbose, __FILE__, __LINE__, bytes_per_sample * traceheader->nsamps,
					(void **)&(mb_segyio_ptr->buffer),error);
			if (status == MB_SUCCESS)
				mb_segyio_ptr->bufferalloc = bytes_per_sample * traceheader->nsamps;
			else
				mb_segyio_ptr->bufferalloc = 0;
			}
		}

	/* insert trace header data */
	if (status == MB_SUCCESS)
		{
		index = 0;
		buffer = mb_segyio_ptr->buffer;
		mb_put_binary_int(MB_NO, traceheader->seq_num, (void *) &buffer[index]); index += 4;
		mb_put_binary_int(MB_NO, traceheader->seq_reel, (void *) &buffer[index]); index += 4;
		mb_put_binary_int(MB_NO, traceheader->shot_num, (void *) &buffer[index]); index += 4;
		mb_put_binary_int(MB_NO, traceheader->shot_tr, (void *) &buffer[index]); index += 4;
		mb_put_binary_int(MB_NO, traceheader->espn, (void *) &buffer[index]); index += 4;
		mb_put_binary_int(MB_NO, traceheader->rp_num, (void *) &buffer[index]); index += 4;
		mb_put_binary_int(MB_NO, traceheader->rp_tr, (void *) &buffer[index]); index += 4;
		mb_put_binary_short(MB_NO, traceheader->trc_id, (void *) &buffer[index]); index += 2;
		mb_put_binary_short(MB_NO, traceheader->num_vstk, (void *) &buffer[index]); index += 2;
		mb_put_binary_short(MB_NO, traceheader->cdp_fold, (void *) &buffer[index]); index += 2;
		mb_put_binary_short(MB_NO, traceheader->use, (void *) &buffer[index]); index += 2;
		mb_put_binary_int(MB_NO, traceheader->range, (void *) &buffer[index]); index += 4;
		mb_put_binary_int(MB_NO, traceheader->grp_elev, (void *) &buffer[index]); index += 4;
		mb_put_binary_int(MB_NO, traceheader->src_elev, (void *) &buffer[index]); index += 4;
		mb_put_binary_int(MB_NO, traceheader->src_depth, (void *) &buffer[index]); index += 4;
		mb_put_binary_int(MB_NO, traceheader->grp_datum, (void *) &buffer[index]); index += 4;
		mb_put_binary_int(MB_NO, traceheader->src_datum, (void *) &buffer[index]); index += 4;
		mb_put_binary_int(MB_NO, traceheader->src_wbd, (void *) &buffer[index]); index += 4;
		mb_put_binary_int(MB_NO, traceheader->grp_wbd, (void *) &buffer[index]); index += 4;
        	mb_put_binary_short(MB_NO, traceheader->elev_scalar, (void *) &buffer[index]); index += 2;
        	mb_put_binary_short(MB_NO, traceheader->coord_scalar, (void *) &buffer[index]); index += 2;
		mb_put_binary_int(MB_NO, traceheader->src_long, (void *) &buffer[index]); index += 4;
		mb_put_binary_int(MB_NO, traceheader->src_lat, (void *) &buffer[index]); index += 4;
		mb_put_binary_int(MB_NO, traceheader->grp_long, (void *) &buffer[index]); index += 4;
		mb_put_binary_int(MB_NO, traceheader->grp_lat, (void *) &buffer[index]); index += 4;
        	mb_put_binary_short(MB_NO, traceheader->coord_units, (void *) &buffer[index]); index += 2;
        	mb_put_binary_short(MB_NO, traceheader->wvel, (void *) &buffer[index]); index += 2;
        	mb_put_binary_short(MB_NO, traceheader->sbvel, (void *) &buffer[index]); index += 2;
        	mb_put_binary_short(MB_NO, traceheader->src_up_vel, (void *) &buffer[index]); index += 2;
        	mb_put_binary_short(MB_NO, traceheader->grp_up_vel, (void *) &buffer[index]); index += 2;
        	mb_put_binary_short(MB_NO, traceheader->src_static, (void *) &buffer[index]); index += 2;
        	mb_put_binary_short(MB_NO, traceheader->grp_static, (void *) &buffer[index]); index += 2;
        	mb_put_binary_short(MB_NO, traceheader->tot_static, (void *) &buffer[index]); index += 2;
        	mb_put_binary_short(MB_NO, traceheader->laga, (void *) &buffer[index]); index += 2;
		mb_put_binary_int(MB_NO, traceheader->delay_mils, (void *) &buffer[index]); index += 4;
        	mb_put_binary_short(MB_NO, traceheader->smute_mils, (void *) &buffer[index]); index += 2;
        	mb_put_binary_short(MB_NO, traceheader->emute_mils, (void *) &buffer[index]); index += 2;
        	mb_put_binary_short(MB_NO, traceheader->nsamps, (void *) &buffer[index]); index += 2;
        	mb_put_binary_short(MB_NO, traceheader->si_micros, (void *) &buffer[index]); index += 2;
		for (i=0;i<19;i++)
			{
        		mb_put_binary_short(MB_NO, traceheader->other_1[i], (void *) &buffer[index]); index += 2;
			}
        	mb_put_binary_short(MB_NO, traceheader->year, (void *) &buffer[index]); index += 2;
        	mb_put_binary_short(MB_NO, traceheader->day_of_yr, (void *) &buffer[index]); index += 2;
        	mb_put_binary_short(MB_NO, traceheader->hour, (void *) &buffer[index]); index += 2;
        	mb_put_binary_short(MB_NO, traceheader->min, (void *) &buffer[index]); index += 2;
        	mb_put_binary_short(MB_NO, traceheader->sec, (void *) &buffer[index]); index += 2;
        	mb_put_binary_short(MB_NO, traceheader->mils, (void *) &buffer[index]); index += 2;
        	mb_put_binary_short(MB_NO, traceheader->tr_weight, (void *) &buffer[index]); index += 2;
		for (i=0;i<5;i++)
			{
        		mb_put_binary_short(MB_NO, traceheader->other_2[i], (void *) &buffer[index]); index += 2;
			}
		mb_put_binary_float(MB_NO, traceheader->delay, (void *) &buffer[index]); index += 4;
        	mb_put_binary_float(MB_NO, traceheader->smute_sec, (void *) &buffer[index]); index += 4;
        	mb_put_binary_float(MB_NO, traceheader->emute_sec, (void *) &buffer[index]); index += 4;
        	mb_put_binary_float(MB_NO, traceheader->si_secs, (void *) &buffer[index]); index += 4;
        	mb_put_binary_float(MB_NO, traceheader->wbt_secs, (void *) &buffer[index]); index += 4;
		mb_put_binary_int(MB_NO, traceheader->end_of_rp, (void *) &buffer[index]); index += 4;
        	mb_put_binary_float(MB_NO, traceheader->dummy1, (void *) &buffer[index]); index += 4;
        	mb_put_binary_float(MB_NO, traceheader->dummy2, (void *) &buffer[index]); index += 4;
        	mb_put_binary_float(MB_NO, traceheader->dummy3, (void *) &buffer[index]); index += 4;
        	mb_put_binary_float(MB_NO, traceheader->dummy4, (void *) &buffer[index]); index += 4;
        	mb_put_binary_float(MB_NO, traceheader->dummy5, (void *) &buffer[index]); index += 4;
        	mb_put_binary_float(MB_NO, traceheader->dummy6, (void *) &buffer[index]); index += 4;
        	mb_put_binary_float(MB_NO, traceheader->dummy7, (void *) &buffer[index]); index += 4;
        	mb_put_binary_float(MB_NO, traceheader->dummy8, (void *) &buffer[index]); index += 4;
        	mb_put_binary_float(MB_NO, traceheader->heading, (void *) &buffer[index]); index += 4;
	
		/* write trace header */
		if (fwrite(buffer,1,MB_SEGY_TRACEHEADER_LENGTH,mb_segyio_ptr->fp) 
					!= MB_SEGY_TRACEHEADER_LENGTH)
			{
			status = MB_FAILURE;
			*error = MB_ERROR_EOF;
			}
		}
	
	/* insert trace data */
	if (status == MB_SUCCESS)
		{
		index = 0;
		buffer = mb_segyio_ptr->buffer;
		for (i=0;i<traceheader->nsamps;i++)
			{
 			if (fileheader->format == 5 || fileheader->format == 6)
       				{
        			mb_put_binary_float(MB_NO, trace[i], (void *) &buffer[index]);  
				index += bytes_per_sample;
				}
 			else if (fileheader->format == 11)
       				{
        			mb_put_binary_float(MB_YES, trace[i], (void *) &buffer[index]);  
				index += bytes_per_sample;
				}
 			else if (fileheader->format == 2)
       				{
				intval = (int) trace[i];
				mb_put_binary_int(MB_NO, intval, (void *) &buffer[index]); 
				index += bytes_per_sample;
				}
 			else if (fileheader->format == 3)
       				{
				shortval = (short) trace[i];
				mb_put_binary_short(MB_NO, shortval, (void *) &buffer[index]);  
				index += bytes_per_sample;
				}
 			else if (fileheader->format == 8)
       				{
				buffer[i] = (char) trace[i]; 
				index++;
				}
			}
		}

	/* write trace */
	if (status == MB_SUCCESS
		&& fwrite(buffer, 1, bytes_per_sample + traceheader->nsamps, mb_segyio_ptr->fp) 
				!= bytes_per_sample + traceheader->nsamps)
		{
		status = MB_FAILURE;
		*error = MB_ERROR_WRITE_FAIL;
		}

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",function_name);
		fprintf(stderr,"dbg2  Revision id: %s\n",rcs_id);
		fprintf(stderr,"dbg2  Return value:\n");
		fprintf(stderr,"dbg2       error:         %d\n",*error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:       %d\n",status);
		}

	/* return success */
	return(status);
}
/*--------------------------------------------------------------------*/
/* Code for numerical Hilbert Transform taken from web pages of
	Per Stoltze:
		http://www.aue.auc.dk/~stoltze/tools/hilbert/hilbert.htm
	on 18 August 2004 
   Usage: The input data are in delta and the call 
		hilbert(n, delta, kappa) 
	will return the Hilbert transform of delta in 
	kappa. The values of n and delta are not modified.*/
void hilbert(int n, double delta[], double kappa[])
{
	double d1, d2, d3, d4;
	int i1, i2;

	for (i1 = 0; i1 < n; i1++)
	{
		kappa[i1] = 0.;
		for (i2 = 1; i2 < n; i2++)
		{
			d1 = (i1+i2<n)? delta[i1+i2]: 0.;
			d2 = (i1-i2>=0)? delta[i1-i2]: 0.;
			d3 = (i1+i2+1<n)? delta[i1+i2+1]: 0.;
			d4 = (i1-i2-1>=0)? delta[i1-i2-1]: 0.;

			kappa[i1] -= 0.5 * (d1-d2) / i2 + 0.5 * (d3 - d4) / (i2 + 1);
		}
		kappa[i1] /= M_PI;
	}
}

/*--------------------------------------------------------------------*/
/* Code for numerical Hilbert Transform modified from web pages of
	Per Stoltze:
		http://www.aue.auc.dk/~stoltze/tools/hilbert/hilbert.htm
	on 18 August 2004 
   Usage: The input data are in even elements of data 
   		(data[0], data[2], ...data[n-2])
	and the call 
		hilbert2(n, data) 
	will return the Hilbert transform of delta in 
	the odd elements of data: 
   		(data[1], data[3], ...data[n-1])
	The values of n and even elements of data are not modified.*/
void hilbert2(int n, double data[])
{
	double d1, d2, d3, d4;
	int i1, i2;

	for (i1 = 0; i1 < n; i1++)
	{
		data[2*i1+1] = 0.;
		for (i2 = 1; i2 < n; i2++)
		{
			d1 = (i1+i2<n)? data[2*(i1+i2)]: 0.;
			d2 = (i1-i2>=0)? data[2*(i1-i2)]: 0.;
			d3 = (i1+i2+1<n)? data[2*(i1+i2+1)]: 0.;
			d4 = (i1-i2-1>=0)? data[2*(i1-i2-1)]: 0.;

			data[2*i1+1] -= 0.5 * (d1-d2) / i2 + 0.5 * (d3 - d4) / (i2 + 1);
		}
		data[2*i1+1] /= M_PI;
	}
}

/*--------------------------------------------------------------------*/

