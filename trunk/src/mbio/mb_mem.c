/*--------------------------------------------------------------------
 *    The MB-system:	mb_mem.c	3/1/93
 *    $Id: mb_mem.c,v 5.7 2004-12-18 01:34:43 caress Exp $
 *
 *    Copyright (c) 1993, 1994, 2000, 2002, 2003 by
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
 * mb_mem.c contains the MBIO routines for managing memory:
 * mb_malloc and mb_free.  These routines call malloc and free,
 * respectively, and also allow debug messages to be printed out
 * according to the verbosity.
 *
 * Author:	D. W. Caress
 * Date:	March 1, 1993
 *
 * $Log: not supported by cvs2svn $
 * Revision 5.6  2003/04/17 21:05:23  caress
 * Release 5.0.beta30
 *
 * Revision 5.5  2003/04/16 16:47:41  caress
 * Release 5.0.beta30
 *
 * Revision 5.4  2002/11/12 07:25:23  caress
 * Added mb_memory_clear() call.
 *
 * Revision 5.3  2002/10/15 18:34:58  caress
 * Release 5.0.beta25
 *
 * Revision 5.2  2002/09/18 23:32:59  caress
 * Release 5.0.beta23
 *
 * Revision 5.1  2002/05/29 23:36:53  caress
 * Release 5.0.beta18
 *
 * Revision 5.0  2000/12/01 22:48:41  caress
 * First cut at Version 5.0.
 *
 * Revision 4.9  2000/10/11  01:02:30  caress
 * Convert to ANSI C
 *
 * Revision 4.8  2000/09/30  06:26:58  caress
 * Snapshot for Dale.
 *
 * Revision 4.7  1998/10/05  18:32:27  caress
 * MB-System version 4.6beta
 *
 * Revision 4.6  1997/04/21  17:02:07  caress
 * MB-System 4.5 Beta Release.
 *
 * Revision 4.6  1997/04/21  17:02:07  caress
 * MB-System 4.5 Beta Release.
 *
 * Revision 4.5  1994/10/28  03:52:18  caress
 * Got rid of realloc call - will try this again later. Will
 * just use malloc for now.
 *
 * Revision 4.5  1994/10/28  03:52:18  caress
 * Got rid of realloc call - will try this again later. Will
 * just use malloc for now.
 *
 * Revision 4.3  1994/10/21  12:11:53  caress
 * Release V4.0
 *
 * Revision 4.2  1994/07/29  18:46:51  caress
 * Changes associated with supporting Lynx OS (byte swapped) and
 * using unix second time base (for time_d values).
 *
 * Revision 4.1  1994/03/23  22:19:33  caress
 * Previous versions of mb_malloc returned an error if the
 * pointer returned by malloc() was NULL, even if the size
 * requested was zero.  This was ok on Suns running BSD, which
 * return pointer values even for zero sized objects, but on
 * IBM machines malloc() returns NULL for zero sized objects.
 * The function mb_malloc now returns a NULL pointer and no error
 * if the requested size is zero.
 *
 * Revision 4.0  1994/03/06  00:01:56  caress
 * First cut at version 4.0
 *
 * Revision 4.1  1994/03/03  03:39:43  caress
 * Fixed copyright message.
 *
 * Revision 4.0  1994/02/21  04:03:53  caress
 * First cut at new version.  No changes.
 *
 * Revision 3.2  1993/05/15  14:37:45  caress
 * fixed rcs_id message
 *
 * Revision 3.1  1993/05/14  22:27:48  sohara
 * fixed rcs_id message
 *
 * Revision 3.0  1993/04/23  16:03:04  dale
 * Initial version
 *
 *
 */

/* standard include files */
#include <stdio.h>

/* mbio include files */
#include "../../include/mb_status.h"
#include "../../include/mb_define.h"

/* memory allocation list variables */
#define	MB_MEMORY_HEAP_MAX	10000
static int	n_mb_alloc = 0;
static char	*mb_alloc_ptr[MB_MEMORY_HEAP_MAX];
static size_t	mb_alloc_size[MB_MEMORY_HEAP_MAX];
static int	mb_alloc_overflow = MB_NO;

/* Local debug define */
/* #define MB_MEM_DEBUG 1 */

static char rcs_id[]="$Id: mb_mem.c,v 5.7 2004-12-18 01:34:43 caress Exp $";

/*--------------------------------------------------------------------*/
int mb_malloc(int verbose, size_t size, void **ptr, int *error)
{
	char	*function_name = "mb_malloc";
	int	status = MB_SUCCESS;
	int	i;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       size:       %d\n",size);
		fprintf(stderr,"dbg2       ptr:        %d\n",ptr);
		fprintf(stderr,"dbg2       *ptr:       %d\n",*ptr);
		}

	/* allocate memory */
	*ptr = NULL;
	if (size > 0)
		{
		if ((*ptr = (char *) malloc(size)) == NULL)
			{
			*error = MB_ERROR_MEMORY_FAIL;
			status = MB_FAILURE;
			}
		else
			{
			*error = MB_ERROR_NO_ERROR;
			status = MB_SUCCESS;
			}
		}
	else
		{
		*ptr = NULL;
		*error = MB_ERROR_NO_ERROR;
		status = MB_SUCCESS;
		}

	/* print debug statements */
	if (verbose >= 5 && size > 0)
		{
		fprintf(stderr,"\ndbg5  Memory allocated in MBIO function <%s>\n",
			function_name);
		fprintf(stderr,"dbg5       i:%d  ptr:%d  size:%d\n",
			n_mb_alloc,*ptr,size);
		}

	/* add to list if size > 0 */
	if (size > 0)
		{
		if (n_mb_alloc < MB_MEMORY_HEAP_MAX)
			{
			mb_alloc_ptr[n_mb_alloc] = *ptr;
			mb_alloc_size[n_mb_alloc] = size;
			n_mb_alloc++;
			}
		else
			{
			mb_alloc_overflow = MB_YES;
#ifdef MB_MEM_DEBUG
		fprintf(stderr,"NOTICE: mbm_mem overflow pointer allocated %d in function %s\n",*ptr, function_name);
#endif		
			}
		}

	/* print debug statements */
	if (verbose >= 6)
		{
		fprintf(stderr,"\ndbg6  Allocated memory list in MBIO function <%s>\n",
			function_name);
		for (i=0;i<n_mb_alloc;i++)
			fprintf(stderr,"dbg6       i:%d  ptr:%d  size:%d\n",
				i,mb_alloc_ptr[i],mb_alloc_size[i]);
		}

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       ptr:        %d\n",*ptr);
		fprintf(stderr,"dbg2       error:      %d\n",*error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:  %d\n",status);
		}

	/* return status */
	return(status);
}
/*--------------------------------------------------------------------*/
int mb_realloc(int verbose, size_t size, void **ptr, int *error)
{
	char	*function_name = "mb_realloc";
	int	status = MB_SUCCESS;
	int	i;
	int	iptr;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       size:       %d\n",size);
		fprintf(stderr,"dbg2       ptr:        %d\n",ptr);
		fprintf(stderr,"dbg2       *ptr:       %d\n",*ptr);
		}

	/* check if pointer is in list */
	iptr = -1;
	for (i=0;i<n_mb_alloc;i++)
		if (mb_alloc_ptr[i] == *ptr)
			iptr = i;
			
	/* if pointer is non-NULL use realloc */
	if (*ptr != NULL)
	    *ptr = (char *) realloc(*ptr, size);

	/* if pointer is NULL use malloc */
	else if (size > 0)
	    *ptr = (char *) malloc(size);
	    
	/* check for success */
	if (size > 0 && *ptr == NULL)
	    {
	    *error = MB_ERROR_MEMORY_FAIL;
	    status = MB_FAILURE;
	    }
	else
	    {
	    *error = MB_ERROR_NO_ERROR;
	    status = MB_SUCCESS;
	    }

	/* if pointer was already in list update it */
	if (status == MB_SUCCESS
	    && iptr > -1)
	    {
	    /* if pointer non-NULL update it */
	    if (size > 0 && *ptr != NULL)
		{
		mb_alloc_ptr[iptr] = *ptr;
		mb_alloc_size[iptr] = size;
		}
		    
	    /* else remove it from list */
	    else
		{
		for (i=iptr;i<n_mb_alloc-1;i++)
			{
			mb_alloc_ptr[i] = mb_alloc_ptr[i+1];
			mb_alloc_size[i] = mb_alloc_size[i+1];
			}
		n_mb_alloc--;
		}
	    }
    
	/* else add to list if possible if size > 0 */
	else if (status == MB_SUCCESS
	    && size > 0)
	    {
	    if (n_mb_alloc < MB_MEMORY_HEAP_MAX)
		{
		mb_alloc_ptr[n_mb_alloc] = *ptr;
		mb_alloc_size[n_mb_alloc] = size;
		n_mb_alloc++;
		}
	    else
		{
		mb_alloc_overflow = MB_YES;
#ifdef MB_MEM_DEBUG
		fprintf(stderr,"NOTICE: mbm_mem overflow pointer allocated %d in function %s\n",*ptr, function_name);
#endif		
		}
	    }

	/* print debug statements */
	if (verbose >= 5 && size > 0)
		{
		fprintf(stderr,"\ndbg5  Memory reallocated in MBIO function <%s>\n",
			function_name);
		fprintf(stderr,"dbg5       i:%d  ptr:%d  size:%d\n",
			n_mb_alloc,*ptr,size);
		}

	/* print debug statements */
	if (verbose >= 6)
		{
		fprintf(stderr,"\ndbg6  Allocated memory list in MBIO function <%s>\n",
			function_name);
		for (i=0;i<n_mb_alloc;i++)
			fprintf(stderr,"dbg6       i:%d  ptr:%d  size:%d\n",
				i,mb_alloc_ptr[i],mb_alloc_size[i]);
		}

	/* assume success */
	*error = MB_ERROR_NO_ERROR;
	status = MB_SUCCESS;

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return value:\n");
		fprintf(stderr,"dbg2       ptr:        %d\n",*ptr);
		fprintf(stderr,"dbg2       error:      %d\n",*error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:     %d\n",status);
		}

	/* return status */
	return(status);
}
/*--------------------------------------------------------------------*/
int mb_free(int verbose, void **ptr, int *error)
{
	char	*function_name = "mb_free";
	int	status = MB_SUCCESS;
	int	i;
	int	iptr, ptrvalue, ptrsize;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       ptr:        %d\n",*ptr);
		}

	/* check if pointer is in list */
	iptr = -1;
	for (i=0;i<n_mb_alloc;i++)
		if (mb_alloc_ptr[i] == *ptr)
			iptr = i;

	/* if pointer is in list remove it from list
		and deallocate the memory */
	if (iptr > -1)
		{
		/* remove it from list */
		ptrvalue = (int) mb_alloc_ptr[iptr];
		ptrsize = mb_alloc_size[iptr];
		for (i=iptr;i<n_mb_alloc-1;i++)
			{
			mb_alloc_ptr[i] = mb_alloc_ptr[i+1];
			mb_alloc_size[i] = mb_alloc_size[i+1];
			}
		n_mb_alloc--;

		/* free the memory */
		free(*ptr);
		*ptr = NULL;
		}

	/* else deallocate the memory if pointer is non-null and
		heap overflow has occurred */
	else if (mb_alloc_overflow == MB_YES && *ptr != NULL)
		{
#ifdef MB_MEM_DEBUG
		fprintf(stderr,"NOTICE: mbm_mem overflow pointer freed %d in function %s\n",*ptr, function_name);
#endif		
		/* free the memory */
		free(*ptr);
		*ptr = NULL;
		}

	/* print debug statements */
	if (verbose >= 5 && iptr > -1)
		{
		fprintf(stderr,"\ndbg5  Allocated memory freed in MBIO function <%s>\n",
			function_name);
		fprintf(stderr,"dbg5       i:%d  ptr:%d  size:%d\n",
			iptr,ptrvalue,ptrsize);
		}

	/* print debug statements */
	if (verbose >= 6)
		{
		fprintf(stderr,"\ndbg6  Allocated memory list in MBIO function <%s>\n",
			function_name);
		for (i=0;i<n_mb_alloc;i++)
			fprintf(stderr,"dbg6       i:%d  ptr:%d  size:%d\n",
				i,mb_alloc_ptr[i],mb_alloc_size[i]);
		}

	/* assume success */
	*error = MB_ERROR_NO_ERROR;
	status = MB_SUCCESS;

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return value:\n");
		fprintf(stderr,"dbg2       error:      %d\n",*error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:     %d\n",status);
		}

	/* return status */
	return(status);
}
/*--------------------------------------------------------------------*/
int mb_memory_clear(int verbose, int *error)
{
	char	*function_name = "mb_memory_clear";
	int	status = MB_SUCCESS;
	int	i;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		}
		
	/* loop over all allocated memory */
	for (i=0;i<n_mb_alloc;i++)
		{
		/* print debug statements */
		if (verbose >= 5)
			{
			fprintf(stderr,"\ndbg5  Allocated memory freed in MBIO function <%s>\n",
				function_name);
			fprintf(stderr,"dbg4       i:%d  ptr:%12d %12x  size:%d\n",
				i,mb_alloc_ptr[i],mb_alloc_ptr[i],mb_alloc_size[i]);
			}

		/* free the memory */
		if (mb_alloc_ptr[i] != NULL)
			free(mb_alloc_ptr[i]);
		mb_alloc_ptr[i] = NULL;
		mb_alloc_size[i] = 0;
		}
	n_mb_alloc = 0;
	
	/* assume success */
	*error = MB_ERROR_NO_ERROR;
	status = MB_SUCCESS;

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return value:\n");
		fprintf(stderr,"dbg2       error:      %d\n",*error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:  %d\n",status);
		}

	/* return status */
	return(status);
}
/*--------------------------------------------------------------------*/
int mb_memory_list(int verbose, int *error)
{
	char	*function_name = "mb_memory_list";
	int	status = MB_SUCCESS;
	int	i;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		}

	/* print debug statements */
	if (verbose >= 4 && n_mb_alloc > 0)
		{
		fprintf(stderr,"\ndbg4  Allocated memory list in MBIO function <%s>\n",
			function_name);
		for (i=0;i<n_mb_alloc;i++)
			fprintf(stderr,"dbg4       i:%d  ptr:%12d %12x  size:%d\n",
				i,mb_alloc_ptr[i],mb_alloc_ptr[i],mb_alloc_size[i]);
		}
	else if (verbose >= 4)
		{
		fprintf(stderr,"\ndbg4  No memory currently allocated in MBIO function <%s>\n",
			function_name);
		}

	/* assume success */
	*error = MB_ERROR_NO_ERROR;
	status = MB_SUCCESS;

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return value:\n");
		fprintf(stderr,"dbg2       error:      %d\n",*error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:  %d\n",status);
		}

	/* return status */
	return(status);
}
/*--------------------------------------------------------------------*/
