/*--------------------------------------------------------------------
 *    The MB-system:	mb_mem.c	3/1/93
 *    $Id$
 *
 *    Copyright (c) 1993-2015 by
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
 */

/* standard include files */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* mbio include files */
#include "mb_status.h"
#include "mb_define.h"
#include "mb_io.h"

/* memory allocation list variables */
#define	MB_MEMORY_ALLOC_STEP	100
#define	MB_MEMORY_HEAP_MAX	10000
static int	mb_mem_debug = MB_NO;
static int	n_mb_alloc = 0;
static void	*mb_alloc_ptr[MB_MEMORY_HEAP_MAX];
static size_t	mb_alloc_size[MB_MEMORY_HEAP_MAX];
static mb_name	mb_alloc_sourcefile[MB_MEMORY_HEAP_MAX];
static int	mb_alloc_sourceline[MB_MEMORY_HEAP_MAX];
static int	mb_alloc_overflow = MB_NO;

/* Local debug define */
/* #define MB_MEM_DEBUG 1 */

static char svn_id[]="$Id$";

/*--------------------------------------------------------------------*/
int mb_mem_debug_on(int verbose, int *error)
{
	char	*function_name = "mb_mem_debug_on";
	int	status = MB_SUCCESS;
	int	i;

	/* turn debug output on */
	mb_mem_debug = MB_YES;

	/* print input debug statements */
	if (verbose >= 2 || mb_mem_debug)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",function_name);
		fprintf(stderr,"dbg2  Revision id: %s\n",svn_id);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		}

	/* print debug statements */
	if (verbose >= 6 || mb_mem_debug)
		{
		fprintf(stderr,"\ndbg6  Allocated memory list in MBIO function <%s>\n",
			function_name);
		for (i=0;i<n_mb_alloc;i++)
			fprintf(stderr,"dbg6       i:%d  ptr:%p  size:%zu source:%s line:%d\n",
				i,(void *)mb_alloc_ptr[i],mb_alloc_size[i],
				mb_alloc_sourcefile[i],mb_alloc_sourceline[i]);
		}

	/* print output debug statements */
	if (verbose >= 2 || mb_mem_debug)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",function_name);
		fprintf(stderr,"dbg2  Revision id: %s\n",svn_id);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       error:      %d\n",*error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:  %d\n",status);
		}

	/* return status */
	return(status);
}

/*--------------------------------------------------------------------*/
int mb_mem_debug_off(int verbose, int *error)
{
	char	*function_name = "mb_mem_debug_off";
	int	status = MB_SUCCESS;
	int	i;

	/* turn debug output off */
	mb_mem_debug = MB_NO;

	/* print input debug statements */
	if (verbose >= 2 || mb_mem_debug)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",function_name);
		fprintf(stderr,"dbg2  Revision id: %s\n",svn_id);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		}

	/* print debug statements */
	if (verbose >= 6 || mb_mem_debug)
		{
		fprintf(stderr,"\ndbg6  Allocated memory list in MBIO function <%s>\n",
			function_name);
		for (i=0;i<n_mb_alloc;i++)
			fprintf(stderr,"dbg6       i:%d  ptr:%p  size:%zu source:%s line:%d\n",
				i,(void *)mb_alloc_ptr[i],mb_alloc_size[i],
				mb_alloc_sourcefile[i],mb_alloc_sourceline[i]);
		}

	/* print output debug statements */
	if (verbose >= 2 || mb_mem_debug)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",function_name);
		fprintf(stderr,"dbg2  Revision id: %s\n",svn_id);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       error:      %d\n",*error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:  %d\n",status);
		}

	/* return status */
	return(status);
}

/*--------------------------------------------------------------------*/
int mb_malloc(int verbose, size_t size, void **ptr, int *error)
{
	char	*function_name = "mb_malloc";
	int	status = MB_SUCCESS;
	int	i;

	/* print input debug statements */
	if (verbose >= 2 || mb_mem_debug)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",function_name);
		fprintf(stderr,"dbg2  Revision id: %s\n",svn_id);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       size:       %zu\n",size);
		fprintf(stderr,"dbg2       ptr:        %p\n",(void *)ptr);
		fprintf(stderr,"dbg2       *ptr:       %p\n",(void *)*ptr);
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
	if ((verbose >= 5 || mb_mem_debug) && size > 0)
		{
		fprintf(stderr,"\ndbg5  Memory allocated in MBIO function <%s>\n",
			function_name);
		fprintf(stderr,"dbg5       i:%d  ptr:%p  size:%zu\n",
			n_mb_alloc,(void *)*ptr,size);
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
	if (verbose >= 6 || mb_mem_debug)
		{
		fprintf(stderr,"\ndbg6  Allocated memory list in MBIO function <%s>\n",
			function_name);
		for (i=0;i<n_mb_alloc;i++)
			fprintf(stderr,"dbg6       i:%d  ptr:%p  size:%zu source:%s line:%d\n",
				i,(void *)mb_alloc_ptr[i],mb_alloc_size[i],
				mb_alloc_sourcefile[i],mb_alloc_sourceline[i]);
		}

	/* print output debug statements */
	if (verbose >= 2 || mb_mem_debug)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",function_name);
		fprintf(stderr,"dbg2  Revision id: %s\n",svn_id);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       ptr:        %p\n",(void *)*ptr);
		fprintf(stderr,"dbg2       error:      %d\n",*error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:  %d\n",status);
		}

	/* return status */
	return(status);
}
/*--------------------------------------------------------------------*/
int mb_mallocd(int verbose, const char *sourcefile, int sourceline,
		size_t size, void **ptr, int *error)
{
	char	*function_name = "mb_mallocd";
	int	status = MB_SUCCESS;
	int	i;

	/* print input debug statements */
	if (verbose >= 2 || mb_mem_debug)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",function_name);
		fprintf(stderr,"dbg2  Revision id: %s\n",svn_id);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       sourcefile: %s\n",sourcefile);
		fprintf(stderr,"dbg2       sourceline: %d\n",sourceline);
		fprintf(stderr,"dbg2       size:       %zu\n",size);
		fprintf(stderr,"dbg2       ptr:        %p\n",(void *)ptr);
		fprintf(stderr,"dbg2       *ptr:       %p\n",(void *)*ptr);
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
	if ((verbose >= 5 || mb_mem_debug) && size > 0)
		{
		fprintf(stderr,"\ndbg5  Memory allocated in MBIO function <%s>\n",
			function_name);
		fprintf(stderr,"dbg5       i:%d  ptr:%p  size:%zu\n",
			n_mb_alloc,(void *)*ptr,size);
		}

	/* add to list if size > 0 */
	if (size > 0)
		{
		if (n_mb_alloc < MB_MEMORY_HEAP_MAX)
			{
			mb_alloc_ptr[n_mb_alloc] = *ptr;
			mb_alloc_size[n_mb_alloc] = size;
			strncpy(mb_alloc_sourcefile[n_mb_alloc],sourcefile,MB_NAME_LENGTH-1);
			mb_alloc_sourceline[n_mb_alloc] = sourceline;
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
	if (verbose >= 6 || mb_mem_debug)
		{
		fprintf(stderr,"\ndbg6  Allocated memory list in MBIO function <%s>\n",
			function_name);
		for (i=0;i<n_mb_alloc;i++)
			fprintf(stderr,"dbg6       i:%d  ptr:%p  size:%zu source:%s line:%d\n",
				i,(void *)mb_alloc_ptr[i],mb_alloc_size[i],
				mb_alloc_sourcefile[i],mb_alloc_sourceline[i]);
		}

	/* print output debug statements */
	if (verbose >= 2 || mb_mem_debug)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",function_name);
		fprintf(stderr,"dbg2  Revision id: %s\n",svn_id);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       ptr:        %p\n",(void *)*ptr);
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
	if (verbose >= 2 || mb_mem_debug)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",function_name);
		fprintf(stderr,"dbg2  Revision id: %s\n",svn_id);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       size:       %zu\n",size);
		fprintf(stderr,"dbg2       ptr:        %p\n",(void *)ptr);
		fprintf(stderr,"dbg2       *ptr:       %p\n",(void *)*ptr);
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
		fprintf(stderr,"NOTICE: mbm_mem overflow pointer allocated %p in function %s\n",*ptr, function_name);
#endif
		}
	    }

	/* print debug statements */
	if ((verbose >= 5 || mb_mem_debug) && size > 0)
		{
		fprintf(stderr,"\ndbg5  Memory reallocated in MBIO function <%s>\n",
			function_name);
		fprintf(stderr,"dbg5       i:%d  ptr:%p  size:%zu\n",
			n_mb_alloc,(void *)*ptr,size);
		}

	/* print debug statements */
	if (verbose >= 6 || mb_mem_debug)
		{
		fprintf(stderr,"\ndbg6  Allocated memory list in MBIO function <%s>\n",
			function_name);
		for (i=0;i<n_mb_alloc;i++)
			fprintf(stderr,"dbg6       i:%d  ptr:%p  size:%zu source:%s line:%d\n",
				i,(void *)mb_alloc_ptr[i],mb_alloc_size[i],
				mb_alloc_sourcefile[i],mb_alloc_sourceline[i]);
		}

	/* assume success */
	*error = MB_ERROR_NO_ERROR;
	status = MB_SUCCESS;

	/* print output debug statements */
	if (verbose >= 2 || mb_mem_debug)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",function_name);
		fprintf(stderr,"dbg2  Revision id: %s\n",svn_id);
		fprintf(stderr,"dbg2  Return value:\n");
		fprintf(stderr,"dbg2       ptr:        %p\n",(void *)*ptr);
		fprintf(stderr,"dbg2       error:      %d\n",*error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:     %d\n",status);
		}

	/* return status */
	return(status);
}
/*--------------------------------------------------------------------*/
int mb_reallocd(int verbose, const char *sourcefile, int sourceline,
		size_t size, void **ptr, int *error)
{
	char	*function_name = "mb_reallocd";
	int	status = MB_SUCCESS;
	int	i;
	int	iptr;

	/* print input debug statements */
	if (verbose >= 2 || mb_mem_debug)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",function_name);
		fprintf(stderr,"dbg2  Revision id: %s\n",svn_id);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       sourcefile: %s\n",sourcefile);
		fprintf(stderr,"dbg2       sourceline: %d\n",sourceline);
		fprintf(stderr,"dbg2       size:       %zu\n",size);
		fprintf(stderr,"dbg2       ptr:        %p\n",(void *)ptr);
		fprintf(stderr,"dbg2       *ptr:       %p\n",(void *)*ptr);
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
		strncpy(mb_alloc_sourcefile[iptr],sourcefile,MB_NAME_LENGTH-1);
		mb_alloc_sourceline[iptr] = sourceline;
		}

	    /* else remove it from list */
	    else
		{
		for (i=iptr;i<n_mb_alloc-1;i++)
			{
			mb_alloc_ptr[i] = mb_alloc_ptr[i+1];
			mb_alloc_size[i] = mb_alloc_size[i+1];
			strncpy(mb_alloc_sourcefile[i],mb_alloc_sourcefile[i+1],MB_NAME_LENGTH-1);
			mb_alloc_sourceline[i] = mb_alloc_sourceline[i+1];
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
		strncpy(mb_alloc_sourcefile[n_mb_alloc],sourcefile,MB_NAME_LENGTH-1);
		mb_alloc_sourceline[n_mb_alloc] = sourceline;
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
	if ((verbose >= 5 || mb_mem_debug) && size > 0)
		{
		fprintf(stderr,"\ndbg5  Memory reallocated in MBIO function <%s>\n",
			function_name);
		fprintf(stderr,"dbg5       i:%d  ptr:%p  size:%zu source:%s line:%d\n",
			n_mb_alloc,(void *)*ptr,size,sourcefile,sourceline);
		}

	/* print debug statements */
	if (verbose >= 6 || mb_mem_debug)
		{
		fprintf(stderr,"\ndbg6  Allocated memory list in MBIO function <%s>\n",
			function_name);
		for (i=0;i<n_mb_alloc;i++)
			fprintf(stderr,"dbg6       i:%d  ptr:%p  size:%zu source:%s line:%d\n",
				i,(void *)mb_alloc_ptr[i],mb_alloc_size[i],
				mb_alloc_sourcefile[i],mb_alloc_sourceline[i]);
		}

	/* assume success */
	*error = MB_ERROR_NO_ERROR;
	status = MB_SUCCESS;

	/* print output debug statements */
	if (verbose >= 2 || mb_mem_debug)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",function_name);
		fprintf(stderr,"dbg2  Revision id: %s\n",svn_id);
		fprintf(stderr,"dbg2  Return value:\n");
		fprintf(stderr,"dbg2       ptr:        %p\n",(void *)*ptr);
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
	int	iptr;
	size_t	ptrsize;
	void	*ptrvalue;

	/* print input debug statements */
	if (verbose >= 2 || mb_mem_debug)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",function_name);
		fprintf(stderr,"dbg2  Revision id: %s\n",svn_id);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       ptr:        %p\n",(void *)*ptr);
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
		ptrvalue = mb_alloc_ptr[iptr];
		ptrsize = mb_alloc_size[iptr];
		for (i=iptr;i<n_mb_alloc-1;i++)
			{
			mb_alloc_ptr[i] = mb_alloc_ptr[i+1];
			mb_alloc_size[i] = mb_alloc_size[i+1];
			strncpy(mb_alloc_sourcefile[i],mb_alloc_sourcefile[i+1],MB_NAME_LENGTH-1);
			mb_alloc_sourceline[i] = mb_alloc_sourceline[i+1];
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
	if ((verbose >= 5 || mb_mem_debug) && iptr > -1)
		{
		fprintf(stderr,"\ndbg5  Allocated memory freed in MBIO function <%s>\n",
			function_name);
		fprintf(stderr,"dbg5       i:%d  ptr:%p  size:%zu\n",
			iptr,ptrvalue,ptrsize);
		}

	/* print debug statements */
	if (verbose >= 6 || mb_mem_debug)
		{
		fprintf(stderr,"\ndbg6  Allocated memory list in MBIO function <%s>\n",
			function_name);
		for (i=0;i<n_mb_alloc;i++)
			fprintf(stderr,"dbg6       i:%d  ptr:%p  size:%zu source:%s line:%d\n",
				i,(void *)mb_alloc_ptr[i],mb_alloc_size[i],
				mb_alloc_sourcefile[i],mb_alloc_sourceline[i]);
		}

	/* assume success */
	*error = MB_ERROR_NO_ERROR;
	status = MB_SUCCESS;

	/* print output debug statements */
	if (verbose >= 2 || mb_mem_debug)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",function_name);
		fprintf(stderr,"dbg2  Revision id: %s\n",svn_id);
		fprintf(stderr,"dbg2  Return value:\n");
		fprintf(stderr,"dbg2       error:      %d\n",*error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:     %d\n",status);
		}

	/* return status */
	return(status);
}
/*--------------------------------------------------------------------*/
int mb_freed(int verbose, const char *sourcefile, int sourceline, void **ptr, int *error)
{
	char	*function_name = "mb_freed";
	int	status = MB_SUCCESS;
	int	i;
	int	iptr;
	size_t	ptrsize;
	long	ptrvalue;

	/* print input debug statements */
	if (verbose >= 2 || mb_mem_debug)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",function_name);
		fprintf(stderr,"dbg2  Revision id: %s\n",svn_id);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       sourcefile: %s\n",sourcefile);
		fprintf(stderr,"dbg2       sourceline: %d\n",sourceline);
		fprintf(stderr,"dbg2       ptr:        %p\n",(void *)*ptr);
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
		ptrvalue = (size_t) mb_alloc_ptr[iptr];
		ptrsize = mb_alloc_size[iptr];
		for (i=iptr;i<n_mb_alloc-1;i++)
			{
			mb_alloc_ptr[i] = mb_alloc_ptr[i+1];
			mb_alloc_size[i] = mb_alloc_size[i+1];
			strncpy(mb_alloc_sourcefile[i],mb_alloc_sourcefile[i+1],MB_NAME_LENGTH-1);
			mb_alloc_sourceline[i] = mb_alloc_sourceline[i+1];
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
	if ((verbose >= 5 || mb_mem_debug) && iptr > -1)
		{
		fprintf(stderr,"\ndbg5  Allocated memory freed in MBIO function <%s>\n",
			function_name);
		fprintf(stderr,"dbg5       i:%d  ptr:%p  size:%zu\n",
			iptr,(void *)ptrvalue,ptrsize);
		}

	/* print debug statements */
	if (verbose >= 6 || mb_mem_debug)
		{
		fprintf(stderr,"\ndbg6  Allocated memory list in MBIO function <%s>\n",
			function_name);
		for (i=0;i<n_mb_alloc;i++)
			fprintf(stderr,"dbg6       i:%d  ptr:%p  size:%zu source:%s line:%d\n",
				i,(void *)mb_alloc_ptr[i],mb_alloc_size[i],
				mb_alloc_sourcefile[i],mb_alloc_sourceline[i]);
		}

	/* assume success */
	*error = MB_ERROR_NO_ERROR;
	status = MB_SUCCESS;

	/* print output debug statements */
	if (verbose >= 2 || mb_mem_debug)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",function_name);
		fprintf(stderr,"dbg2  Revision id: %s\n",svn_id);
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
	if (verbose >= 2 || mb_mem_debug)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",function_name);
		fprintf(stderr,"dbg2  Revision id: %s\n",svn_id);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		}

	/* loop over all allocated memory */
	for (i=0;i<n_mb_alloc;i++)
		{
		/* print debug statements */
		if (verbose >= 5 || mb_mem_debug)
			{
			fprintf(stderr,"\ndbg5  Allocated memory freed in MBIO function <%s>\n",
				function_name);
			fprintf(stderr,"dbg4       i:%d  ptr:%12p  size:%zu\n",
				i,(void *)mb_alloc_ptr[i],mb_alloc_size[i]);
			}

		/* free the memory */
		if (mb_alloc_ptr[i] != NULL)
			free(mb_alloc_ptr[i]);
		mb_alloc_ptr[i] = NULL;
		mb_alloc_size[i] = 0;
		(mb_alloc_sourcefile[i])[0] = 0;
		mb_alloc_sourceline[i] = 0;
		}
	n_mb_alloc = 0;

	/* assume success */
	*error = MB_ERROR_NO_ERROR;
	status = MB_SUCCESS;

	/* print output debug statements */
	if (verbose >= 2 || mb_mem_debug)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",function_name);
		fprintf(stderr,"dbg2  Revision id: %s\n",svn_id);
		fprintf(stderr,"dbg2  Return value:\n");
		fprintf(stderr,"dbg2       error:      %d\n",*error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:  %d\n",status);
		}

	/* return status */
	return(status);
}
/*--------------------------------------------------------------------*/
int mb_memory_status(int verbose, int *nalloc, int *nallocmax,
			int *overflow, size_t *allocsize, int *error)
{
	char	*function_name = "mb_memory_status";
	int	status = MB_SUCCESS;
	int	i;

	/* print input debug statements */
	if (verbose >= 2 || mb_mem_debug)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",function_name);
		fprintf(stderr,"dbg2  Revision id: %s\n",svn_id);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		}

	/* get status */
	*nalloc = n_mb_alloc;
	*nallocmax = MB_MEMORY_HEAP_MAX;
	*overflow = mb_alloc_overflow;
	*allocsize = 0;
	for (i=0;i<n_mb_alloc;i++)
		*allocsize += mb_alloc_size[i];

	/* assume success */
	*error = MB_ERROR_NO_ERROR;
	status = MB_SUCCESS;

	/* print output debug statements */
	if (verbose >= 2 || mb_mem_debug)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",function_name);
		fprintf(stderr,"dbg2  Revision id: %s\n",svn_id);
		fprintf(stderr,"dbg2  Return value:\n");
		fprintf(stderr,"dbg2       nalloc:     %d\n",*nalloc);
		fprintf(stderr,"dbg2       nallocmax:  %d\n",*nallocmax);
		fprintf(stderr,"dbg2       overflow:   %d\n",*overflow);
		fprintf(stderr,"dbg2       allocsize:  %zu\n",*allocsize);
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
	if (verbose >= 2 || mb_mem_debug)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",function_name);
		fprintf(stderr,"dbg2  Revision id: %s\n",svn_id);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		}

	/* print debug statements */
	if ((verbose >= 4  || mb_mem_debug) && n_mb_alloc > 0)
		{
		fprintf(stderr,"\ndbg4  Allocated memory list in MBIO function <%s>\n",
			function_name);
		for (i=0;i<n_mb_alloc;i++)
			fprintf(stderr,"dbg6       i:%d  ptr:%p  size:%zu source:%s line:%d\n",
				i,(void *)mb_alloc_ptr[i],mb_alloc_size[i],
				mb_alloc_sourcefile[i],mb_alloc_sourceline[i]);
		}
	else if (verbose >= 4 || mb_mem_debug)
		{
		fprintf(stderr,"\ndbg4  No memory currently allocated in MBIO function <%s>\n",
			function_name);
		}

	/* assume success */
	*error = MB_ERROR_NO_ERROR;
	status = MB_SUCCESS;

	/* print output debug statements */
	if (verbose >= 2 || mb_mem_debug)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",function_name);
		fprintf(stderr,"dbg2  Revision id: %s\n",svn_id);
		fprintf(stderr,"dbg2  Return value:\n");
		fprintf(stderr,"dbg2       error:      %d\n",*error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:  %d\n",status);
		}

	/* return status */
	return(status);
}
/*--------------------------------------------------------------------*/
int mb_register_array(int verbose, void *mbio_ptr,
		int type, size_t size, void **handle, int *error)
{
	static char svn_id[]="$Id$";
	char	*function_name = "mb_register_array";
	int	status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	int	nalloc;
	int	i;

	/* print input debug statements */
	if (verbose >= 2 || mb_mem_debug)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",function_name);
		fprintf(stderr,"dbg2  Revision id: %s\n",svn_id);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       mb_ptr:     %p\n",(void *)mbio_ptr);
		fprintf(stderr,"dbg2       type:       %d\n",type);
		fprintf(stderr,"dbg2       size:       %p\n",(void *)size);
		fprintf(stderr,"dbg2       handle:     %lx\n",(size_t)handle);
		}

	/* get mbio descriptor */
	mb_io_ptr = (struct mb_io_struct *) mbio_ptr;

	/* allocate larger array for register if necessary */
	if (mb_io_ptr->n_regarray >= mb_io_ptr->n_regarray_alloc)
		{
		mb_io_ptr->n_regarray_alloc += MB_MEMORY_ALLOC_STEP;
		status = mb_realloc(verbose, mb_io_ptr->n_regarray_alloc * sizeof(void *),
					(void **)&(mb_io_ptr->regarray_handle), error);
		if (status == MB_SUCCESS)
		status = mb_realloc(verbose, mb_io_ptr->n_regarray_alloc * sizeof(void *),
					(void **)&(mb_io_ptr->regarray_ptr), error);
		if (status == MB_SUCCESS)
		status = mb_realloc(verbose, mb_io_ptr->n_regarray_alloc * sizeof(void *),
					(void **)&(mb_io_ptr->regarray_oldptr), error);
		if (status == MB_SUCCESS)
		status = mb_realloc(verbose, mb_io_ptr->n_regarray_alloc * sizeof(int),
					(void **)&(mb_io_ptr->regarray_type), error);
		if (status == MB_SUCCESS)
		status = mb_realloc(verbose, mb_io_ptr->n_regarray_alloc * sizeof(size_t),
					(void **)&(mb_io_ptr->regarray_size), error);
		for (i=mb_io_ptr->n_regarray;i<mb_io_ptr->n_regarray_alloc;i++)
			{
			mb_io_ptr->regarray_handle[i] = NULL;
			mb_io_ptr->regarray_ptr[i] = NULL;
			mb_io_ptr->regarray_oldptr[i] = NULL;
			mb_io_ptr->regarray_type[i] = MB_MEM_TYPE_NONE;
			mb_io_ptr->regarray_size[i] = 0;
			}
		}

	/* register the array */
	if (status == MB_SUCCESS)
		{
		/* get dimension of desired array */
		if (type == MB_MEM_TYPE_BATHYMETRY)
			nalloc = mb_io_ptr->beams_bath_max;
		else if (type == MB_MEM_TYPE_AMPLITUDE)
			nalloc = mb_io_ptr->beams_amp_max;
		else if (type == MB_MEM_TYPE_SIDESCAN)
			nalloc = mb_io_ptr->pixels_ss_max;
		else
			nalloc = 0;

		/* allocate the array - always allocate at least one dimension so the pointer is non-null */
		if (nalloc <= 0)
			nalloc = 1;
		status = mb_realloc(verbose, nalloc * size,
					handle, error);
		if (status == MB_SUCCESS)
			{
			mb_io_ptr->regarray_handle[mb_io_ptr->n_regarray] = (void *) (handle);
			mb_io_ptr->regarray_ptr[mb_io_ptr->n_regarray] = (void *) (*handle);
			mb_io_ptr->regarray_oldptr[mb_io_ptr->n_regarray] = NULL;
			mb_io_ptr->regarray_type[mb_io_ptr->n_regarray] = type;
			mb_io_ptr->regarray_size[mb_io_ptr->n_regarray] = size;
			mb_io_ptr->n_regarray++;
/*fprintf(stderr,"Array registered: handle:%lx ptr:%lx  stored handle:%lx ptr:%lx type:%d size:%zu\n",
(size_t)handle, (size_t)*handle,
(size_t)mb_io_ptr->regarray_handle[mb_io_ptr->n_regarray-1],
(size_t)mb_io_ptr->regarray_ptr[mb_io_ptr->n_regarray-1],
mb_io_ptr->regarray_type[mb_io_ptr->n_regarray-1],
mb_io_ptr->regarray_size[mb_io_ptr->n_regarray-1]);*/
			}
		}

	/* print output debug statements */
	if (verbose >= 2 || mb_mem_debug)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",function_name);
		fprintf(stderr,"dbg2  Revision id: %s\n",svn_id);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       *handle:    %p\n",(void *)*handle);
		fprintf(stderr,"dbg2       error:      %d\n",*error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:  %d\n",status);
		}

	/* return status */
	return(status);
}
/*--------------------------------------------------------------------*/
int mb_update_arrays(int verbose, void *mbio_ptr,
		int nbath, int namp, int nss, int *error)
{
	static char svn_id[]="$Id$";
	char	*function_name = "mb_update_arrays";
	int	status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	void	**handle;
	int	i;

	/* print input debug statements */
	if (verbose >= 2 || mb_mem_debug)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",function_name);
		fprintf(stderr,"dbg2  Revision id: %s\n",svn_id);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       mb_ptr:     %p\n",(void *)mbio_ptr);
		fprintf(stderr,"dbg2       nbath:      %d\n",nbath);
		fprintf(stderr,"dbg2       namp:       %d\n",namp);
		fprintf(stderr,"dbg2       nss:        %d\n",nss);
		}

	/* get mbio descriptor */
	mb_io_ptr = (struct mb_io_struct *) mbio_ptr;
//fprintf(stderr,"\nSTART mb_update_arrays: nbath:%d %d  namp:%d %d  nss:%d %d\n",
//nbath,mb_io_ptr->beams_bath_alloc,
//namp,mb_io_ptr->beams_amp_alloc,
//nss,mb_io_ptr->pixels_ss_alloc);
//fprintf(stderr,"KEYPTRS1:\n\tbeamflag: %p\n\tbath:     %p\n\tbathxtrk: %p\n\tbathltrk: %p\n\tbathnum:  %p\n",
//mb_io_ptr->beamflag,mb_io_ptr->bath,mb_io_ptr->bath_acrosstrack,mb_io_ptr->bath_alongtrack,mb_io_ptr->bath_num);
//fprintf(stderr,"KEYHNDL1:\n\tbeamflag: %p\n\tbath:     %p\n\tbathxtrk: %p\n\tbathltrk: %p\n\tbathnum:  %p\n",
//&mb_io_ptr->beamflag,&mb_io_ptr->bath,&mb_io_ptr->bath_acrosstrack,&mb_io_ptr->bath_alongtrack,&mb_io_ptr->bath_num);
//if (mb_io_ptr->beamflag != NULL)
//{
//fprintf(stderr,"VALUES1: \n\tbeamflag: %d\n\tbath:     %f\n\tbathxtrk: %f\n\tbathltrk: %f\n\tbathnum:  %d\n",
//mb_io_ptr->beamflag[0],mb_io_ptr->bath[0],mb_io_ptr->bath_acrosstrack[0],mb_io_ptr->bath_alongtrack[0],mb_io_ptr->bath_num[0]);
//for (i=0;i<mb_io_ptr->beams_bath_alloc;i++)
//{
//fprintf(stderr,"%d ",mb_io_ptr->beamflag[i]);
//if (i%25==0 || i==mb_io_ptr->beams_bath_alloc-1) fprintf(stderr,"\n");
//}
//}

	/* reallocate larger arrays if necessary */
	if (nbath > mb_io_ptr->beams_bath_alloc)
		{
		/* set allocation size */
		if (nbath < 1024)
			{
			mb_io_ptr->beams_bath_alloc = (nbath / 256) * 256;
			if (nbath % 256 > 0)
				mb_io_ptr->beams_bath_alloc += 256;
			}
		else
			{
			mb_io_ptr->beams_bath_alloc = (nbath / 1024) * 1024;
			if (nbath % 1024 > 0)
				mb_io_ptr->beams_bath_alloc += 1024;
			}
//fprintf(stderr,"CHECK mb_update_arrays: nbath:%d %d  namp:%d %d  nss:%d %d\n",
//nbath,mb_io_ptr->beams_bath_alloc,
//namp,mb_io_ptr->beams_amp_alloc,
//nss,mb_io_ptr->pixels_ss_alloc);


		/* allocate mb_io_ptr arrays */
		if (status == MB_SUCCESS)
			status = mb_realloc(verbose,mb_io_ptr->beams_bath_alloc*sizeof(char),
					(void **)&mb_io_ptr->beamflag,error);
		if (status == MB_SUCCESS)
			status = mb_realloc(verbose,mb_io_ptr->beams_bath_alloc*sizeof(double),
					(void **)&mb_io_ptr->bath,error);
		if (status == MB_SUCCESS)
			status = mb_realloc(verbose,mb_io_ptr->beams_bath_alloc*sizeof(double),
					(void **)&mb_io_ptr->bath_acrosstrack,error);
		if (status == MB_SUCCESS)
			status = mb_realloc(verbose,mb_io_ptr->beams_bath_alloc*sizeof(double),
					(void **)&mb_io_ptr->bath_alongtrack,error);
		if (status == MB_SUCCESS)
			status = mb_realloc(verbose,mb_io_ptr->beams_bath_alloc*sizeof(int),
					(void **)&mb_io_ptr->bath_num,error);
		if (status == MB_SUCCESS)
			status = mb_realloc(verbose,mb_io_ptr->beams_bath_alloc*sizeof(char),
					(void **)&mb_io_ptr->new_beamflag,error);
		if (status == MB_SUCCESS)
			status = mb_realloc(verbose,mb_io_ptr->beams_bath_alloc*sizeof(double),
					(void **)&mb_io_ptr->new_bath,error);
		if (status == MB_SUCCESS)
			status = mb_realloc(verbose,mb_io_ptr->beams_bath_alloc*sizeof(double),
					(void **)&mb_io_ptr->new_bath_acrosstrack,error);
		if (status == MB_SUCCESS)
			status = mb_realloc(verbose,mb_io_ptr->beams_bath_alloc*sizeof(double),
					(void **)&mb_io_ptr->new_bath_alongtrack,error);
		for (i=mb_io_ptr->beams_bath_max;i<mb_io_ptr->beams_bath_alloc;i++)
			{
			mb_io_ptr->beamflag[i] = 0;
			mb_io_ptr->bath[i] = 0.0;
			mb_io_ptr->bath_acrosstrack[i] = 0.0;
			mb_io_ptr->bath_alongtrack[i] = 0.0;
			mb_io_ptr->bath_num[i] = 0;
			mb_io_ptr->new_beamflag[i] = 0;
			mb_io_ptr->new_bath[i] = 0.0;
			mb_io_ptr->new_bath_acrosstrack[i] = 0.0;
			mb_io_ptr->new_bath_alongtrack[i] = 0.0;
			}
		mb_io_ptr->beams_bath_max = nbath;
//fprintf(stderr,"KEYPTRS2:\n\tbeamflag: %p\n\tbath:     %p\n\tbathxtrk: %p\n\tbathltrk: %p\n\tbathnum:  %p\n",
//mb_io_ptr->beamflag,mb_io_ptr->bath,mb_io_ptr->bath_acrosstrack,mb_io_ptr->bath_alongtrack,mb_io_ptr->bath_num);
//fprintf(stderr,"KEYHNDL2:\n\tbeamflag: %p\n\tbath:     %p\n\tbathxtrk: %p\n\tbathltrk: %p\n\tbathnum:  %p\n",
//&mb_io_ptr->beamflag,&mb_io_ptr->bath,&mb_io_ptr->bath_acrosstrack,&mb_io_ptr->bath_alongtrack,&mb_io_ptr->bath_num);
//if (mb_io_ptr->beamflag != NULL)
//{
//fprintf(stderr,"VALUES2: \n\tbeamflag: %d\n\tbath:     %f\n\tbathxtrk: %f\n\tbathltrk: %f\n\tbathnum:  %d\n",
//mb_io_ptr->beamflag[0],mb_io_ptr->bath[0],mb_io_ptr->bath_acrosstrack[0],mb_io_ptr->bath_alongtrack[0],mb_io_ptr->bath_num[0]);
//for (i=0;i<mb_io_ptr->beams_bath_alloc;i++)
//{
//fprintf(stderr,"%d ",mb_io_ptr->beamflag[i]);
//if (i%25==0 || i==mb_io_ptr->beams_bath_alloc-1) fprintf(stderr,"\n");
//}
//}

		/* allocate registered arrays */
		for (i=0;i<mb_io_ptr->n_regarray;i++)
			{
			/* allocate the bathymetry dimensioned arrays */
			if (mb_io_ptr->regarray_type[i] == MB_MEM_TYPE_BATHYMETRY
				&& mb_io_ptr->beams_bath_alloc > 0)
				{
				mb_io_ptr->regarray_oldptr[i] = mb_io_ptr->regarray_ptr[i];
				handle = (void **)(mb_io_ptr->regarray_handle[i]);
				status = mb_realloc(verbose,
						mb_io_ptr->beams_bath_alloc * mb_io_ptr->regarray_size[i],
						handle, error);
				mb_io_ptr->regarray_ptr[i] = *handle;
				}
			}

		/* set reallocation flag */
		mb_io_ptr->bath_arrays_reallocated = MB_YES;
		}
	if (namp > mb_io_ptr->beams_amp_alloc)
		{
		/* set allocation size */
		if (namp < 1024)
			{
			mb_io_ptr->beams_amp_alloc = (namp / 256) * 256;
			if (namp % 256 > 0)
				mb_io_ptr->beams_amp_alloc += 256;
			}
		else
			{
			mb_io_ptr->beams_amp_alloc = (namp / 1024) * 1024;
			if (namp % 1024 > 0)
				mb_io_ptr->beams_amp_alloc += 1024;
			}

		if (status == MB_SUCCESS)
			status = mb_realloc(verbose,mb_io_ptr->beams_amp_alloc*sizeof(double),
					(void **)&mb_io_ptr->amp,error);
		if (status == MB_SUCCESS)
			status = mb_realloc(verbose,mb_io_ptr->beams_amp_alloc*sizeof(int),
					(void **)&mb_io_ptr->amp_num,error);
		if (status == MB_SUCCESS)
			status = mb_realloc(verbose,mb_io_ptr->beams_amp_alloc*sizeof(double),
					(void **)&mb_io_ptr->new_amp,error);
		for (i=mb_io_ptr->beams_amp_max;i<mb_io_ptr->beams_amp_alloc;i++)
			{
			mb_io_ptr->amp[i] = 0.0;
			mb_io_ptr->amp_num[i] = 0;
			mb_io_ptr->new_amp[i] = 0.0;
			}
		mb_io_ptr->beams_amp_max = namp;

		/* allocate registered arrays */
		for (i=0;i<mb_io_ptr->n_regarray;i++)
			{
			/* allocate the amplitude dimensioned arrays */
			if (mb_io_ptr->regarray_type[i] == MB_MEM_TYPE_AMPLITUDE
				&& mb_io_ptr->beams_amp_alloc > 0)
				{
				mb_io_ptr->regarray_oldptr[i] = mb_io_ptr->regarray_ptr[i];
				handle = (void **)(mb_io_ptr->regarray_handle[i]);
				status = mb_realloc(verbose,
						mb_io_ptr->beams_amp_alloc * mb_io_ptr->regarray_size[i],
						handle, error);
				mb_io_ptr->regarray_ptr[i] = *handle;
				}
			}

		/* set reallocation flag */
		mb_io_ptr->amp_arrays_reallocated = MB_YES;
		}
	if (nss > mb_io_ptr->pixels_ss_alloc)
		{
		/* set allocation size */
		if (nss < 1024)
			{
			mb_io_ptr->pixels_ss_alloc = (nss / 256) * 256;
			if (nss % 256 > 0)
				mb_io_ptr->pixels_ss_alloc += 256;
			}
		else
			{
			mb_io_ptr->pixels_ss_alloc = (nss / 1024) * 1024;
			if (nss % 1024 > 0)
				mb_io_ptr->pixels_ss_alloc += 1024;
			}

		if (status == MB_SUCCESS)
			status = mb_realloc(verbose,mb_io_ptr->pixels_ss_alloc*sizeof(double),
					(void **)&mb_io_ptr->ss,error);
		if (status == MB_SUCCESS)
			status = mb_realloc(verbose,mb_io_ptr->pixels_ss_alloc*sizeof(double),
					(void **)&mb_io_ptr->ss_acrosstrack,error);
		if (status == MB_SUCCESS)
			status = mb_realloc(verbose,mb_io_ptr->pixels_ss_alloc*sizeof(double),
					(void **)&mb_io_ptr->ss_alongtrack,error);
		if (status == MB_SUCCESS)
			status = mb_realloc(verbose,mb_io_ptr->pixels_ss_alloc*sizeof(int),
					(void **)&mb_io_ptr->ss_num,error);
		if (status == MB_SUCCESS)
			status = mb_realloc(verbose,mb_io_ptr->pixels_ss_alloc*sizeof(double),
					(void **)&mb_io_ptr->new_ss,error);
		if (status == MB_SUCCESS)
			status = mb_realloc(verbose,mb_io_ptr->pixels_ss_alloc*sizeof(double),
					(void **)&mb_io_ptr->new_ss_acrosstrack,error);
		if (status == MB_SUCCESS)
			status = mb_realloc(verbose,mb_io_ptr->pixels_ss_alloc*sizeof(double),
					(void **)&mb_io_ptr->new_ss_alongtrack,error);
		for (i=mb_io_ptr->pixels_ss_max;i<mb_io_ptr->pixels_ss_alloc;i++)
			{
			mb_io_ptr->ss[i] = 0.0;
			mb_io_ptr->ss_acrosstrack[i] = 0.0;
			mb_io_ptr->ss_alongtrack[i] = 0.0;
			mb_io_ptr->ss_num[i] = 0;
			mb_io_ptr->new_ss[i] = 0.0;
			mb_io_ptr->new_ss_acrosstrack[i] = 0.0;
			mb_io_ptr->new_ss_alongtrack[i] = 0.0;
			}
		mb_io_ptr->pixels_ss_max = nss;

		/* allocate registered arrays */
		for (i=0;i<mb_io_ptr->n_regarray;i++)
			{
			/* allocate the sidescan dimensioned arrays */
			if (mb_io_ptr->regarray_type[i] == MB_MEM_TYPE_SIDESCAN
				&& mb_io_ptr->pixels_ss_alloc > 0)
				{
				mb_io_ptr->regarray_oldptr[i] = mb_io_ptr->regarray_ptr[i];
				handle = (void **)(mb_io_ptr->regarray_handle[i]);
				status = mb_realloc(verbose,
						mb_io_ptr->pixels_ss_alloc * mb_io_ptr->regarray_size[i],
						handle, error);
				mb_io_ptr->regarray_ptr[i] = *handle;
				}
			}

		/* set reallocation flag */
		mb_io_ptr->ss_arrays_reallocated = MB_YES;
		}

	/* deal with a memory allocation failure */
	if (status == MB_FAILURE)
		{
		/* free the registered arrays */
		for (i=0;i<mb_io_ptr->n_regarray;i++)
			{
			/* free all arrays */
			if (mb_io_ptr->regarray_handle[i] != NULL)
				{
				status = mb_free(verbose, (void **)(mb_io_ptr->regarray_handle[i]), error);
				}
			}

		/* free the standard arrays */
		status = mb_free(verbose,(void **)&mb_io_ptr->beamflag,error);
		status = mb_free(verbose,(void **)&mb_io_ptr->bath,error);
		status = mb_free(verbose,(void **)&mb_io_ptr->amp,error);
		status = mb_free(verbose,(void **)&mb_io_ptr->bath_acrosstrack,error);
		status = mb_free(verbose,(void **)&mb_io_ptr->bath_alongtrack,error);
		status = mb_free(verbose,(void **)&mb_io_ptr->bath_num,error);
		status = mb_free(verbose,(void **)&mb_io_ptr->amp_num,error);
		status = mb_free(verbose,(void **)&mb_io_ptr->ss,error);
		status = mb_free(verbose,(void **)&mb_io_ptr->ss_acrosstrack,error);
		status = mb_free(verbose,(void **)&mb_io_ptr->ss_alongtrack,error);
		status = mb_free(verbose,(void **)&mb_io_ptr->ss_num,error);
		status = mb_free(verbose,(void **)&mb_io_ptr->beamflag,error);
		status = mb_free(verbose,(void **)&mb_io_ptr->new_bath,error);
		status = mb_free(verbose,(void **)&mb_io_ptr->new_amp,error);
		status = mb_free(verbose,(void **)&mb_io_ptr->new_bath_acrosstrack,error);
		status = mb_free(verbose,(void **)&mb_io_ptr->new_bath_alongtrack,error);
		status = mb_free(verbose,(void **)&mb_io_ptr->new_ss,error);
		status = mb_free(verbose,(void **)&mb_io_ptr->new_ss_acrosstrack,error);
		status = mb_free(verbose,(void **)&mb_io_ptr->new_ss_alongtrack,error);

		/* free the mb_io structure */
		status = mb_free(verbose,(void **)&mb_io_ptr,error);
		mb_io_ptr->beams_bath_alloc = 0;
		mb_io_ptr->beams_amp_alloc = 0;
		mb_io_ptr->pixels_ss_alloc = 0;
		status = MB_FAILURE;
		*error = MB_ERROR_MEMORY_FAIL;
		}

	/* print output debug statements */
	if (verbose >= 2 || mb_mem_debug)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",function_name);
		fprintf(stderr,"dbg2  Revision id: %s\n",svn_id);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       error:      %d\n",*error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:  %d\n",status);
		}
//fprintf(stderr,"KEYPTRS3:\n\tbeamflag: %p\n\tbath:     %p\n\tbathxtrk: %p\n\tbathltrk: %p\n\tbathnum:  %p\n",
//mb_io_ptr->beamflag,mb_io_ptr->bath,mb_io_ptr->bath_acrosstrack,mb_io_ptr->bath_alongtrack,mb_io_ptr->bath_num);
//fprintf(stderr,"KEYHNDL3:\n\tbeamflag: %p\n\tbath:     %p\n\tbathxtrk: %p\n\tbathltrk: %p\n\tbathnum:  %p\n",
//&mb_io_ptr->beamflag,&mb_io_ptr->bath,&mb_io_ptr->bath_acrosstrack,&mb_io_ptr->bath_alongtrack,&mb_io_ptr->bath_num);
//if (mb_io_ptr->beamflag != NULL)
//{
//fprintf(stderr,"VALUES3: \n\tbeamflag: %d\n\tbath:     %f\n\tbathxtrk: %f\n\tbathltrk: %f\n\tbathnum:  %d\n",
//mb_io_ptr->beamflag[0],mb_io_ptr->bath[0],mb_io_ptr->bath_acrosstrack[0],mb_io_ptr->bath_alongtrack[0],mb_io_ptr->bath_num[0]);
//for (i=0;i<mb_io_ptr->beams_bath_alloc;i++)
//{
//fprintf(stderr,"%d ",mb_io_ptr->beamflag[i]);
//if (i%25==0 || i==mb_io_ptr->beams_bath_alloc-1) fprintf(stderr,"\n");
//}
//}

	/* return status */
	return(status);
}
/*--------------------------------------------------------------------*/
int mb_update_arrayptr(int verbose, void *mbio_ptr,
		void **handle, int *error)
{
	static char svn_id[]="$Id$";
	char	*function_name = "mb_update_arrayptr";
	int	status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	int	found;
	int	i;

	/* print input debug statements */
	if (verbose >= 2 || mb_mem_debug)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",function_name);
		fprintf(stderr,"dbg2  Revision id: %s\n",svn_id);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       handle:     %p\n",(void *)handle);
		fprintf(stderr,"dbg2       *handle:    %p\n",(void *)*handle);
		}

	/* get mbio descriptor */
	mb_io_ptr = (struct mb_io_struct *) mbio_ptr;

	/* look for handle in registered arrays */
	found = MB_NO;
	for (i=0;i<mb_io_ptr->n_regarray && found == MB_NO;i++)
		{
		if (*handle == mb_io_ptr->regarray_oldptr[i])
			{
			*handle = mb_io_ptr->regarray_ptr[i];
/*fprintf(stderr,"check handle:%x old ptrs:%x %x",handle,*handle,mb_io_ptr->regarray_oldptr[i]);
fprintf(stderr," found : new ptr:%x",*handle);
fprintf(stderr,"\n");*/
			found = MB_YES;
			}
		}

	/* print output debug statements */
	if (verbose >= 2 || mb_mem_debug)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",function_name);
		fprintf(stderr,"dbg2  Revision id: %s\n",svn_id);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       *handle:    %p\n",(void *)*handle);
		fprintf(stderr,"dbg2       error:      %d\n",*error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:  %d\n",status);
		}

	/* return status */
	return(status);
}
/*--------------------------------------------------------------------*/
int mb_list_arrays(int verbose, void *mbio_ptr, int *error)
{
	static char svn_id[]="$Id$";
	char	*function_name = "mb_list_arrays";
	int	status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	int	i;

	/* print input debug statements */
	if (verbose >= 2 || mb_mem_debug)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",function_name);
		fprintf(stderr,"dbg2  Revision id: %s\n",svn_id);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       mbio_ptr:   %p\n",(void *)mbio_ptr);
		}

	/* get mbio descriptor */
	mb_io_ptr = (struct mb_io_struct *) mbio_ptr;

	/* look for handle in registered arrays */
	fprintf(stderr,"\nRegistered Array List:\n");
	for (i=0;i<mb_io_ptr->n_regarray;i++)
		{
		fprintf(stderr,"Array %d: handle:%p ptr:%p type:%d size:%zu\n",
				i,(void *)mb_io_ptr->regarray_handle[i],(void *)mb_io_ptr->regarray_ptr[i],
				mb_io_ptr->regarray_type[i],mb_io_ptr->regarray_size[i]);
		}

	/* print output debug statements */
	if (verbose >= 2 || mb_mem_debug)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",function_name);
		fprintf(stderr,"dbg2  Revision id: %s\n",svn_id);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       error:      %d\n",*error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:  %d\n",status);
		}

	/* return status */
	return(status);
}
/*--------------------------------------------------------------------*/
int mb_deall_ioarrays(int verbose, void *mbio_ptr, int *error)
{
	static char svn_id[]="$Id$";
	char	*function_name = "mb_deall_ioarrays";
	int	status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	int	i;

	/* print input debug statements */
	if (verbose >= 2 || mb_mem_debug)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",function_name);
		fprintf(stderr,"dbg2  Revision id: %s\n",svn_id);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       mb_ptr:     %p\n",(void *)mbio_ptr);
		}

	/* get mbio descriptor */
	mb_io_ptr = (struct mb_io_struct *) mbio_ptr;

	/* deallocate mb_io_ptr arrays */
	status = mb_free(verbose, (void **)&mb_io_ptr->beamflag,error);
	status = mb_free(verbose, (void **)&mb_io_ptr->bath,error);
	status = mb_free(verbose, (void **)&mb_io_ptr->bath_acrosstrack,error);
	status = mb_free(verbose, (void **)&mb_io_ptr->bath_alongtrack,error);
	status = mb_free(verbose, (void **)&mb_io_ptr->bath_num,error);
	status = mb_free(verbose, (void **)&mb_io_ptr->new_beamflag,error);
	status = mb_free(verbose, (void **)&mb_io_ptr->new_bath,error);
	status = mb_free(verbose, (void **)&mb_io_ptr->new_bath_acrosstrack,error);
	status = mb_free(verbose, (void **)&mb_io_ptr->new_bath_alongtrack,error);
	status = mb_free(verbose, (void **)&mb_io_ptr->amp,error);
	status = mb_free(verbose, (void **)&mb_io_ptr->amp_num,error);
	status = mb_free(verbose, (void **)&mb_io_ptr->new_amp,error);
	status = mb_free(verbose, (void **)&mb_io_ptr->ss,error);
	status = mb_free(verbose, (void **)&mb_io_ptr->ss_acrosstrack,error);
	status = mb_free(verbose, (void **)&mb_io_ptr->ss_alongtrack,error);
	status = mb_free(verbose, (void **)&mb_io_ptr->ss_num,error);
	status = mb_free(verbose, (void **)&mb_io_ptr->new_ss,error);
	status = mb_free(verbose, (void **)&mb_io_ptr->new_ss_acrosstrack,error);
	status = mb_free(verbose, (void **)&mb_io_ptr->new_ss_alongtrack,error);
	mb_io_ptr->beams_bath_max = 0;
	mb_io_ptr->beams_bath_alloc = 0;
	mb_io_ptr->beams_amp_max = 0;
	mb_io_ptr->beams_amp_alloc = 0;
	mb_io_ptr->pixels_ss_max = 0;
	mb_io_ptr->pixels_ss_alloc = 0;

	/* deallocate registered arrays */
/*fprintf(stderr,"deallocate %d registered arrays\n",mb_io_ptr->n_regarray);*/
	for (i=0;i<mb_io_ptr->n_regarray;i++)
		{
/*fprintf(stderr,"i:%d registered array: stored handle:%x ptr:%x\n",
i,mb_io_ptr->regarray_handle[mb_io_ptr->n_regarray-1],
mb_io_ptr->regarray_ptr[mb_io_ptr->n_regarray-1]);*/
		if (status == MB_SUCCESS)
			status = mb_free(verbose, (void **)(mb_io_ptr->regarray_handle[i]), error);
		}
	mb_io_ptr->n_regarray = 0;
	mb_io_ptr->n_regarray_alloc = 0;

	/* deallocate registry itself */
	if (status == MB_SUCCESS)
		status = mb_free(verbose, (void **)&(mb_io_ptr->regarray_handle), error);
	if (status == MB_SUCCESS)
		status = mb_free(verbose, (void **)&(mb_io_ptr->regarray_ptr), error);
	if (status == MB_SUCCESS)
		status = mb_free(verbose, (void **)&(mb_io_ptr->regarray_oldptr), error);
	if (status == MB_SUCCESS)
		status = mb_free(verbose, (void **)&(mb_io_ptr->regarray_type), error);
	if (status == MB_SUCCESS)
		status = mb_free(verbose, (void **)&(mb_io_ptr->regarray_size), error);

	/* print output debug statements */
	if (verbose >= 2 || mb_mem_debug)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",function_name);
		fprintf(stderr,"dbg2  Revision id: %s\n",svn_id);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       error:      %d\n",*error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:  %d\n",status);
		}

	/* return status */
	return(status);
}
/*--------------------------------------------------------------------*/
