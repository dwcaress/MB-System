/*--------------------------------------------------------------------
 *    The MB-system:  mb_mem.c  3/1/93
  *
 *    Copyright (c) 1993-2023 by
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
 * mb_mem.c contains the MBIO routines for managing memory:
 * mb_malloc and mb_free.  These routines call malloc and free,
 * respectively, and also allow debug messages to be printed out
 * according to the verbosity.
 *
 * Author:  D. W. Caress
 * Date:  March 1, 1993
 */

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "mb_define.h"
#include "mb_io.h"
#include "mb_status.h"

/* memory allocation list variables */
static const int MB_MEMORY_ALLOC_STEP = 100;
#define MB_MEMORY_HEAP_MAX 10000
static bool mb_memory_list_enabled = true;
static bool mb_mem_debug = false;
static int n_mb_alloc = 0;
static void *mb_alloc_ptr[MB_MEMORY_HEAP_MAX];
static size_t mb_alloc_size[MB_MEMORY_HEAP_MAX];
static mb_name mb_alloc_sourcefile[MB_MEMORY_HEAP_MAX];
static int mb_alloc_sourceline[MB_MEMORY_HEAP_MAX];
static bool mb_alloc_overflow = false;

/*--------------------------------------------------------------------*/
int mb_mem_list_enable(int verbose, int *error) {

  /* turn memory list on */
  mb_memory_list_enabled = true;

  if (verbose >= 2 || mb_mem_debug) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
  }

  if (verbose >= 6 || mb_mem_debug) {
    fprintf(stderr, "\ndbg6  Allocated memory list in MBIO function <%s>\n", __func__);
    for (int i = 0; i < n_mb_alloc; i++)
      fprintf(stderr, "dbg6       i:%d  ptr:%p  size:%zu source:%s line:%d\n", i, (void *)mb_alloc_ptr[i], mb_alloc_size[i],
              mb_alloc_sourcefile[i], mb_alloc_sourceline[i]);
  }

  const int status = MB_SUCCESS;

  if (verbose >= 2 || mb_mem_debug) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       error:      %d\n", *error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:  %d\n", status);
  }

  return (status);
}

/*--------------------------------------------------------------------*/
int mb_mem_list_disable(int verbose, int *error) {

  /* turn memory list off */
  mb_memory_list_enabled = false;

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
  }

  const int status = MB_SUCCESS;

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       error:      %d\n", *error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:  %d\n", status);
  }

  return (status);
}


/*--------------------------------------------------------------------*/
int mb_mem_debug_on(int verbose, int *error) {

  /* turn debug output on */
  mb_mem_debug = true;

  /* if (verbose >= 2 || mb_mem_debug) */ {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
  }

  /* if (verbose >= 6 || mb_mem_debug) */ {
    fprintf(stderr, "\ndbg6  Allocated memory list in MBIO function <%s>\n", __func__);
    for (int i = 0; i < n_mb_alloc; i++)
      fprintf(stderr, "dbg6       i:%d  ptr:%p  size:%zu source:%s line:%d\n", i, (void *)mb_alloc_ptr[i], mb_alloc_size[i],
              mb_alloc_sourcefile[i], mb_alloc_sourceline[i]);
  }

  const int status = MB_SUCCESS;

  /* if (verbose >= 2 || mb_mem_debug) */ {
    fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       error:      %d\n", *error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:  %d\n", status);
  }

  return (status);
}

/*--------------------------------------------------------------------*/
int mb_mem_debug_off(int verbose, int *error) {

  /* turn debug output off */
  mb_mem_debug = false;

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
  }

  if (verbose >= 6) {
    fprintf(stderr, "\ndbg6  Allocated memory list in MBIO function <%s>\n", __func__);
    for (int i = 0; i < n_mb_alloc; i++)
      fprintf(stderr, "dbg6       i:%d  ptr:%p  size:%zu source:%s line:%d\n", i, (void *)mb_alloc_ptr[i], mb_alloc_size[i],
              mb_alloc_sourcefile[i], mb_alloc_sourceline[i]);
  }

  const int status = MB_SUCCESS;

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       error:      %d\n", *error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:  %d\n", status);
  }

  return (status);
}

/*--------------------------------------------------------------------*/
int mb_malloc(int verbose, size_t size, void **ptr, int *error) {
  if (verbose >= 2 || mb_mem_debug) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
    fprintf(stderr, "dbg2       size:       %zu\n", size);
    fprintf(stderr, "dbg2       ptr:        %p\n", (void *)ptr);
    fprintf(stderr, "dbg2       *ptr:       %p\n", (void *)*ptr);
  }

  /* allocate memory */
  *ptr = NULL;
  int status = MB_SUCCESS;
  if (size > 0) {
    if ((*ptr = (char *)malloc(size)) == NULL) {
      *error = MB_ERROR_MEMORY_FAIL;
      status = MB_FAILURE;
    }
    else {
      *error = MB_ERROR_NO_ERROR;
      status = MB_SUCCESS;
    }
  }
  else {
    *ptr = NULL;
    *error = MB_ERROR_NO_ERROR;
    status = MB_SUCCESS;
  }

  if ((verbose >= 5 || mb_mem_debug) && size > 0) {
    fprintf(stderr, "\ndbg5  Memory allocated in MBIO function <%s>\n", __func__);
    fprintf(stderr, "dbg5       i:%d  ptr:%p  size:%zu\n", n_mb_alloc, (void *)*ptr, size);
  }

  /* keep list of allocated memory */
  if (mb_memory_list_enabled) {
    /* add to list if size > 0 */
    if (size > 0) {
      if (n_mb_alloc < MB_MEMORY_HEAP_MAX) {
        mb_alloc_ptr[n_mb_alloc] = *ptr;
        mb_alloc_size[n_mb_alloc] = size;
        n_mb_alloc++;
      }
      else {
        mb_alloc_overflow = true;
#ifdef MB_MEM_DEBUG
        fprintf(stderr, "NOTICE: mbm_mem overflow pointer allocated %d in function %s\n", *ptr, __func__);
#endif
      }
    }

    if (verbose >= 6 || mb_mem_debug) {
      fprintf(stderr, "\ndbg6  Allocated memory list in MBIO function <%s>\n", __func__);
      for (int i = 0; i < n_mb_alloc; i++)
        fprintf(stderr, "dbg6       i:%d  ptr:%p  size:%zu source:%s line:%d\n", i, (void *)mb_alloc_ptr[i], mb_alloc_size[i],
                mb_alloc_sourcefile[i], mb_alloc_sourceline[i]);
    }
  }
  if (verbose >= 2 || mb_mem_debug) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       ptr:        %p\n", (void *)*ptr);
    fprintf(stderr, "dbg2       error:      %d\n", *error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:  %d\n", status);
  }

  return (status);
}
/*--------------------------------------------------------------------*/
int mb_mallocd(int verbose, const char *sourcefile, int sourceline, size_t size, void **ptr, int *error) {
  if (verbose >= 2 || mb_mem_debug) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
    fprintf(stderr, "dbg2       sourcefile: %s\n", sourcefile);
    fprintf(stderr, "dbg2       sourceline: %d\n", sourceline);
    fprintf(stderr, "dbg2       size:       %zu\n", size);
    fprintf(stderr, "dbg2       ptr:        %p\n", (void *)ptr);
    fprintf(stderr, "dbg2       *ptr:       %p\n", (void *)*ptr);
  }

  /* allocate memory */
  *ptr = NULL;
  int status = MB_SUCCESS;
  if (size > 0) {
    if ((*ptr = (char *)malloc(size)) == NULL) {
      *error = MB_ERROR_MEMORY_FAIL;
      status = MB_FAILURE;
    }
    else {
      *error = MB_ERROR_NO_ERROR;
      status = MB_SUCCESS;
    }
  }
  else {
    *ptr = NULL;
    *error = MB_ERROR_NO_ERROR;
    status = MB_SUCCESS;
  }

  /* keep list of allocated memory */
  if (mb_memory_list_enabled) {

    if ((verbose >= 5 || mb_mem_debug) && size > 0) {
      fprintf(stderr, "\ndbg5  Memory allocated in MBIO function <%s>\n", __func__);
      fprintf(stderr, "dbg5       i:%d  ptr:%p  size:%zu\n", n_mb_alloc, (void *)*ptr, size);
    }

    /* add to list if size > 0 */
    if (size > 0) {
      if (n_mb_alloc < MB_MEMORY_HEAP_MAX) {
        mb_alloc_ptr[n_mb_alloc] = *ptr;
        mb_alloc_size[n_mb_alloc] = size;
        strncpy(mb_alloc_sourcefile[n_mb_alloc], sourcefile, MB_NAME_LENGTH - 1);
        mb_alloc_sourceline[n_mb_alloc] = sourceline;
        n_mb_alloc++;
      }
      else {
        mb_alloc_overflow = true;
#ifdef MB_MEM_DEBUG
        fprintf(stderr, "NOTICE: mbm_mem overflow pointer allocated %d in function %s\n", *ptr, __func__);
#endif
      }
    }

    if (verbose >= 6 || mb_mem_debug) {
      fprintf(stderr, "\ndbg6  Allocated memory list in MBIO function <%s>\n", __func__);
      for (int i = 0; i < n_mb_alloc; i++)
        fprintf(stderr, "dbg6       i:%d  ptr:%p  size:%zu source:%s line:%d\n", i, (void *)mb_alloc_ptr[i], mb_alloc_size[i],
                mb_alloc_sourcefile[i], mb_alloc_sourceline[i]);
    }
  }

  if (verbose >= 2 || mb_mem_debug) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       ptr:        %p\n", (void *)*ptr);
    fprintf(stderr, "dbg2       error:      %d\n", *error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:  %d\n", status);
  }

  return (status);
}
/*--------------------------------------------------------------------*/
int mb_realloc(int verbose, size_t size, void **ptr, int *error) {
  if (verbose >= 2 || mb_mem_debug) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
    fprintf(stderr, "dbg2       size:       %zu\n", size);
    fprintf(stderr, "dbg2       ptr:        %p\n", (void *)ptr);
    fprintf(stderr, "dbg2       *ptr:       %p\n", (void *)*ptr);
  }


  /* keep list of allocated memory */
  int iptr = -1;
  if (mb_memory_list_enabled) {
    /* check if pointer is in list */
    for (int i = 0; i < n_mb_alloc; i++)
      if (mb_alloc_ptr[i] == *ptr)
        iptr = i;
  }

  /* if pointer is non-NULL use realloc */
  if (*ptr != NULL)
    *ptr = (char *)realloc(*ptr, size);

  /* if pointer is NULL use malloc */
  else if (size > 0)
    *ptr = (char *)malloc(size);

  /* check for success */
  int status = MB_SUCCESS;
  if (size > 0 && *ptr == NULL) {
    *error = MB_ERROR_MEMORY_FAIL;
    status = MB_FAILURE;
  } else {
    *error = MB_ERROR_NO_ERROR;
    status = MB_SUCCESS;
  }

  /* keep list of allocated memory */
  if (mb_memory_list_enabled) {
    /* if pointer was already in list update it */
    if (status == MB_SUCCESS && iptr > -1) {
      /* if pointer non-NULL update it */
      if (size > 0 && *ptr != NULL) {
        mb_alloc_ptr[iptr] = *ptr;
        mb_alloc_size[iptr] = size;
      }

      /* else remove it from list */
      else {
        for (int i = iptr; i < n_mb_alloc - 1; i++) {
          mb_alloc_ptr[i] = mb_alloc_ptr[i + 1];
          mb_alloc_size[i] = mb_alloc_size[i + 1];
        }
        n_mb_alloc--;
      }
    }

    /* else add to list if possible if size > 0 */
    else if (status == MB_SUCCESS && size > 0) {
      if (n_mb_alloc < MB_MEMORY_HEAP_MAX) {
        mb_alloc_ptr[n_mb_alloc] = *ptr;
        mb_alloc_size[n_mb_alloc] = size;
        n_mb_alloc++;
      }
      else {
        mb_alloc_overflow = true;
#ifdef MB_MEM_DEBUG
        fprintf(stderr, "NOTICE: mbm_mem overflow pointer allocated %p in function %s\n", *ptr, __func__);
#endif
      }
    }

    if ((verbose >= 5 || mb_mem_debug) && size > 0) {
      fprintf(stderr, "\ndbg5  Memory reallocated in MBIO function <%s>\n", __func__);
      fprintf(stderr, "dbg5       i:%d  ptr:%p  size:%zu\n", n_mb_alloc, (void *)*ptr, size);
    }

    if (verbose >= 6 || mb_mem_debug) {
      fprintf(stderr, "\ndbg6  Allocated memory list in MBIO function <%s>\n", __func__);
      for (int i = 0; i < n_mb_alloc; i++)
        fprintf(stderr, "dbg6       i:%d  ptr:%p  size:%zu source:%s line:%d\n", i, (void *)mb_alloc_ptr[i], mb_alloc_size[i],
                mb_alloc_sourcefile[i], mb_alloc_sourceline[i]);
    }
  }

  /* assume success */
  *error = MB_ERROR_NO_ERROR;
  status = MB_SUCCESS;

  if (verbose >= 2 || mb_mem_debug) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return value:\n");
    fprintf(stderr, "dbg2       ptr:        %p\n", (void *)*ptr);
    fprintf(stderr, "dbg2       error:      %d\n", *error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:     %d\n", status);
  }

  return (status);
}
/*--------------------------------------------------------------------*/
int mb_reallocd(int verbose, const char *sourcefile, int sourceline, size_t size, void **ptr, int *error) {
  if (verbose >= 2 || mb_mem_debug) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
    fprintf(stderr, "dbg2       sourcefile: %s\n", sourcefile);
    fprintf(stderr, "dbg2       sourceline: %d\n", sourceline);
    fprintf(stderr, "dbg2       size:       %zu\n", size);
    fprintf(stderr, "dbg2       ptr:        %p\n", (void *)ptr);
    fprintf(stderr, "dbg2       *ptr:       %p\n", (void *)*ptr);
  }

  /* keep list of allocated memory */
  int iptr = -1;
  if (mb_memory_list_enabled) {
    /* check if pointer is in list */
    for (int i = 0; i < n_mb_alloc; i++) {
      if (mb_alloc_ptr[i] == *ptr) {
        iptr = i;
      }
    }
  }

  /* if pointer is non-NULL use realloc */
  if (*ptr != NULL)
    *ptr = (char *)realloc(*ptr, size);

  /* if pointer is NULL use malloc */
  else if (size > 0)
    *ptr = (char *)malloc(size);

  /* check for success */
  int status = MB_SUCCESS;
  if (size > 0 && *ptr == NULL) {
    *error = MB_ERROR_MEMORY_FAIL;
    status = MB_FAILURE;
  }
  else {
    *error = MB_ERROR_NO_ERROR;
    status = MB_SUCCESS;
  }

  /* keep list of allocated memory */
  if (mb_memory_list_enabled) {
    /* if pointer was already in list update it */
    if (status == MB_SUCCESS && iptr > -1) {
      /* if pointer non-NULL update it */
      if (size > 0 && *ptr != NULL) {
        mb_alloc_ptr[iptr] = *ptr;
        mb_alloc_size[iptr] = size;
        strncpy(mb_alloc_sourcefile[iptr], sourcefile, MB_NAME_LENGTH - 1);
        mb_alloc_sourceline[iptr] = sourceline;
      }

      /* else remove it from list */
      else {
        for (int i = iptr; i < n_mb_alloc - 1; i++) {
          mb_alloc_ptr[i] = mb_alloc_ptr[i + 1];
          mb_alloc_size[i] = mb_alloc_size[i + 1];
          strncpy(mb_alloc_sourcefile[i], mb_alloc_sourcefile[i + 1], MB_NAME_LENGTH - 1);
          mb_alloc_sourceline[i] = mb_alloc_sourceline[i + 1];
        }
        n_mb_alloc--;
      }
    }

    /* else add to list if possible if size > 0 */
    else if (status == MB_SUCCESS && size > 0) {
      if (n_mb_alloc < MB_MEMORY_HEAP_MAX) {
        mb_alloc_ptr[n_mb_alloc] = *ptr;
        mb_alloc_size[n_mb_alloc] = size;
        strncpy(mb_alloc_sourcefile[n_mb_alloc], sourcefile, MB_NAME_LENGTH - 1);
        mb_alloc_sourceline[n_mb_alloc] = sourceline;
        n_mb_alloc++;
      }
      else {
        mb_alloc_overflow = true;
        if (mb_mem_debug)
          fprintf(stderr, "NOTICE: mbm_mem overflow pointer allocated %p in function %s\n", *ptr, __func__);
      }
    }

    if ((verbose >= 5 || mb_mem_debug) && size > 0) {
      fprintf(stderr, "\ndbg5  Memory reallocated in MBIO function <%s>\n", __func__);
      fprintf(stderr, "dbg5       i:%d  ptr:%p  size:%zu source:%s line:%d\n", n_mb_alloc, (void *)*ptr, size, sourcefile,
              sourceline);
    }

    if (verbose >= 6 || mb_mem_debug) {
      fprintf(stderr, "\ndbg6  Allocated memory list in MBIO function <%s>\n", __func__);
      for (int i = 0; i < n_mb_alloc; i++)
        fprintf(stderr, "dbg6       i:%d  ptr:%p  size:%zu source:%s line:%d\n", i, (void *)mb_alloc_ptr[i], mb_alloc_size[i],
                mb_alloc_sourcefile[i], mb_alloc_sourceline[i]);
    }
  }

  /* assume success */
  *error = MB_ERROR_NO_ERROR;
  status = MB_SUCCESS;

  if (verbose >= 2 || mb_mem_debug) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return value:\n");
    fprintf(stderr, "dbg2       ptr:        %p\n", (void *)*ptr);
    fprintf(stderr, "dbg2       error:      %d\n", *error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:     %d\n", status);
  }

  return (status);
}
/*--------------------------------------------------------------------*/
int mb_free(int verbose, void **ptr, int *error) {
  verbose = 5;
  if (verbose >= 2 || mb_mem_debug) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
    fprintf(stderr, "dbg2       ptr:        %p\n", (void *)*ptr);
  }

  /* if keeping list of allocated memory then free memory only if it is in
      the list or list has overflowed */
  if (mb_memory_list_enabled) {
    /* check if pointer is in list */
    int iptr = -1;
    for (int i = 0; i < n_mb_alloc; i++) {
      if (mb_alloc_ptr[i] == *ptr) {
        iptr = i;
      }
    }

    /* if pointer is in list remove it from list */
    size_t ptrsize;
    void *ptrvalue;
    if (iptr > -1) {
      /* remove it from list */
      ptrvalue = mb_alloc_ptr[iptr];
      ptrsize = mb_alloc_size[iptr];
      for (int i = iptr; i < n_mb_alloc - 1; i++) {
        mb_alloc_ptr[i] = mb_alloc_ptr[i + 1];
        mb_alloc_size[i] = mb_alloc_size[i + 1];
        strncpy(mb_alloc_sourcefile[i], mb_alloc_sourcefile[i + 1], MB_NAME_LENGTH - 1);
        mb_alloc_sourceline[i] = mb_alloc_sourceline[i + 1];
      }
      n_mb_alloc--;

      /* free the memory */
      if (*ptr != NULL) {
        free(*ptr);
        *ptr = NULL;
      }
    }

    /* else heap overflow has occurred */
    else if (mb_alloc_overflow && *ptr != NULL) {
#ifdef MB_MEM_DEBUG
      fprintf(stderr, "NOTICE: mbm_mem overflow pointer freed %d in function %s\n", *ptr, __func__);
#endif

      /* free the memory */
      free(*ptr);
      *ptr = NULL;
    }

    if ((verbose >= 5 || mb_mem_debug) && iptr > -1) {
      fprintf(stderr, "\ndbg5  Allocated memory freed in MBIO function <%s>\n", __func__);
      fprintf(stderr, "dbg5       i:%d  ptr:%p  size:%zu\n", iptr, ptrvalue, ptrsize);
    }

    if (verbose >= 6 || mb_mem_debug) {
      fprintf(stderr, "\ndbg6  Allocated memory list in MBIO function <%s>\n", __func__);
      for (int i = 0; i < n_mb_alloc; i++)
        fprintf(stderr, "dbg6       i:%d  ptr:%p  size:%zu source:%s line:%d\n", i, (void *)mb_alloc_ptr[i], mb_alloc_size[i],
                mb_alloc_sourcefile[i], mb_alloc_sourceline[i]);
    }
  }

  /* else if memory list is not being kept and *ptr != NULL, just free the memory */
  else if (*ptr != NULL) {
    free(*ptr);
    *ptr = NULL;
  }

  /* assume success */
  *error = MB_ERROR_NO_ERROR;
  const int status = MB_SUCCESS;

  if (verbose >= 2 || mb_mem_debug) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return value:\n");
    fprintf(stderr, "dbg2       error:      %d\n", *error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:     %d\n", status);
  }

  return (status);
}
/*--------------------------------------------------------------------*/
int mb_freed(int verbose, const char *sourcefile, int sourceline, void **ptr, int *error) {
  if (verbose >= 2 || mb_mem_debug) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
    fprintf(stderr, "dbg2       sourcefile: %s\n", sourcefile);
    fprintf(stderr, "dbg2       sourceline: %d\n", sourceline);
    fprintf(stderr, "dbg2       *ptr:       %p\n", (void *)ptr);
    fprintf(stderr, "dbg2       ptr:        %p\n", (void *)*ptr);
  }

  /* if keeping list of allocated memory then free memory only if it is in
      the list or list has overflowed */
  if (mb_memory_list_enabled) {
    /* check if pointer is in list */
    int iptr = -1;
    for (int i = 0; i < n_mb_alloc; i++)
      if (ptr != NULL && mb_alloc_ptr[i] == *ptr)
        iptr = i;

    /* if pointer is in list remove it from list */
    size_t ptrsize;
    long ptrvalue;
    if (iptr > -1) {
      /* remove it from list */
      ptrvalue = (size_t)mb_alloc_ptr[iptr];
      ptrsize = mb_alloc_size[iptr];
      for (int i = iptr; i < n_mb_alloc - 1; i++) {
        mb_alloc_ptr[i] = mb_alloc_ptr[i + 1];
        mb_alloc_size[i] = mb_alloc_size[i + 1];
        strncpy(mb_alloc_sourcefile[i], mb_alloc_sourcefile[i + 1], MB_NAME_LENGTH - 1);
        mb_alloc_sourceline[i] = mb_alloc_sourceline[i + 1];
      }
      n_mb_alloc--;

      /* free the memory */
      if (*ptr != NULL) {
        free(*ptr);
        *ptr = NULL;
      }
    }

    /* else  heap overflow has occurred */
    else if (mb_alloc_overflow && *ptr != NULL) {
  #ifdef MB_MEM_DEBUG
      fprintf(stderr, "NOTICE: mbm_mem overflow pointer freed %d in function %s\n", *ptr, __func__);
  #endif

      /* free the memory */
      free(*ptr);
      *ptr = NULL;
    }

    if ((verbose >= 5 || mb_mem_debug) && iptr > -1) {
      fprintf(stderr, "\ndbg5  Allocated memory freed in MBIO function <%s>\n", __func__);
      fprintf(stderr, "dbg5       i:%d  ptr:%p  size:%zu\n", iptr, (void *)ptrvalue, ptrsize);
    }

    if (verbose >= 6 || mb_mem_debug) {
      fprintf(stderr, "\ndbg6  Allocated memory list in MBIO function <%s>\n", __func__);
      for (int i = 0; i < n_mb_alloc; i++)
        fprintf(stderr, "dbg6       i:%d  ptr:%p  size:%zu source:%s line:%d\n", i, (void *)mb_alloc_ptr[i], mb_alloc_size[i],
                mb_alloc_sourcefile[i], mb_alloc_sourceline[i]);
    }
  }

  /* else if memory list is not being kept and *ptr != NULL, just free the memory */
  else if (*ptr != NULL) {
    free(*ptr);
    *ptr = NULL;
  }

  /* assume success */
  *error = MB_ERROR_NO_ERROR;
  const int status = MB_SUCCESS;

  if (verbose >= 2 || mb_mem_debug) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return value:\n");
    fprintf(stderr, "dbg2       error:      %d\n", *error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:     %d\n", status);
  }

  return (status);
}
/*--------------------------------------------------------------------*/
int mb_memory_clear(int verbose, int *error) {
  if (verbose >= 2 || mb_mem_debug) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
  }

  /* keep list of allocated memory */
  if (mb_memory_list_enabled) {
    /* loop over all allocated memory */
    for (int i = 0; i < n_mb_alloc; i++) {
      if (verbose >= 5 || mb_mem_debug) {
        fprintf(stderr, "\ndbg5  Allocated memory freed in MBIO function <%s>\n", __func__);
        fprintf(stderr, "dbg4       i:%d  ptr:%12p  size:%zu\n", i, (void *)mb_alloc_ptr[i], mb_alloc_size[i]);
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
  }

  /* assume success */
  *error = MB_ERROR_NO_ERROR;
  const int status = MB_SUCCESS;

  if (verbose >= 2 || mb_mem_debug) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return value:\n");
    fprintf(stderr, "dbg2       error:      %d\n", *error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:  %d\n", status);
  }

  return (status);
}
/*--------------------------------------------------------------------*/
int mb_memory_status(int verbose, int *nalloc, int *nallocmax, int *overflow, size_t *allocsize, int *error) {
  if (verbose >= 2 || mb_mem_debug) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
  }

  *nalloc = 0;
  *nallocmax = 0;
  *overflow = 0;
  *allocsize = 0;

  /* keep list of allocated memory */
  if (mb_memory_list_enabled) {
    /* get status */
    *nalloc = n_mb_alloc;
    *nallocmax = MB_MEMORY_HEAP_MAX;
    *overflow = mb_alloc_overflow;
    *allocsize = 0;
    for (int i = 0; i < n_mb_alloc; i++)
      *allocsize += mb_alloc_size[i];
  }

  /* assume success */
  *error = MB_ERROR_NO_ERROR;
  const int status = MB_SUCCESS;

  if (verbose >= 2 || mb_mem_debug) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return value:\n");
    fprintf(stderr, "dbg2       nalloc:     %d\n", *nalloc);
    fprintf(stderr, "dbg2       nallocmax:  %d\n", *nallocmax);
    fprintf(stderr, "dbg2       overflow:   %d\n", *overflow);
    fprintf(stderr, "dbg2       allocsize:  %zu\n", *allocsize);
    fprintf(stderr, "dbg2       error:      %d\n", *error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:  %d\n", status);
  }

  return (status);
}
/*--------------------------------------------------------------------*/
int mb_memory_list(int verbose, int *error) {
  if (verbose >= 2 || mb_mem_debug) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
  }

  /* keep list of allocated memory */
  if (mb_memory_list_enabled) {
    if (verbose >= 4 || mb_mem_debug) {
      if (n_mb_alloc > 0) {
        fprintf(stderr, "\ndbg4  Allocated memory list in MBIO function <%s>\n", __func__);
        for (int i = 0; i < n_mb_alloc; i++)
          fprintf(stderr, "dbg6       i:%d  ptr:%p  size:%zu source:%s line:%d\n", i, (void *)mb_alloc_ptr[i],
                  mb_alloc_size[i], mb_alloc_sourcefile[i], mb_alloc_sourceline[i]);
      }
      else {
        fprintf(stderr, "\ndbg4  No memory currently allocated in MBIO function <%s>\n", __func__);
      }
    }
    else if (n_mb_alloc > 0) {
      fprintf(stderr, "\nWarning: some objects are still allocated in memory:\n");
      for (int i = 0; i < n_mb_alloc; i++)
        fprintf(stderr, "     i:%d  ptr:%p  size:%zu source:%s line:%d\n", i, (void *)mb_alloc_ptr[i], mb_alloc_size[i],
                mb_alloc_sourcefile[i], mb_alloc_sourceline[i]);
      fprintf(stderr, "Probable failure in MB-System garbage collection...\n");
    }
  }

  /* assume success */
  *error = MB_ERROR_NO_ERROR;
  const int status = MB_SUCCESS;

  if (verbose >= 2 || mb_mem_debug) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return value:\n");
    fprintf(stderr, "dbg2       error:      %d\n", *error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:  %d\n", status);
  }

  return (status);
}
/*--------------------------------------------------------------------*/
int mb_register_array(int verbose, void *mbio_ptr, int type, size_t size, void **handle, int *error) {
  if (verbose >= 2 || mb_mem_debug) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
    fprintf(stderr, "dbg2       mb_ptr:     %p\n", (void *)mbio_ptr);
    fprintf(stderr, "dbg2       type:       %d\n", type);
    fprintf(stderr, "dbg2       size:       %p\n", (void *)size);
    fprintf(stderr, "dbg2       handle:     %lx\n", (size_t)handle);
  }

  /* get mbio descriptor */
  struct mb_io_struct *mb_io_ptr = (struct mb_io_struct *)mbio_ptr;

  /* allocate larger array for register if necessary */
  int status = MB_SUCCESS;
  if (mb_io_ptr->n_regarray >= mb_io_ptr->n_regarray_alloc) {
    mb_io_ptr->n_regarray_alloc += MB_MEMORY_ALLOC_STEP;
    status = mb_reallocd(verbose, __FILE__, __LINE__, mb_io_ptr->n_regarray_alloc * sizeof(void *),
                         (void **)&(mb_io_ptr->regarray_handle), error);
    if (status == MB_SUCCESS)
      status = mb_reallocd(verbose, __FILE__, __LINE__, mb_io_ptr->n_regarray_alloc * sizeof(void *),
                           (void **)&(mb_io_ptr->regarray_ptr), error);
    if (status == MB_SUCCESS)
      status = mb_reallocd(verbose, __FILE__, __LINE__, mb_io_ptr->n_regarray_alloc * sizeof(void *),
                           (void **)&(mb_io_ptr->regarray_oldptr), error);
    if (status == MB_SUCCESS)
      status = mb_reallocd(verbose, __FILE__, __LINE__, mb_io_ptr->n_regarray_alloc * sizeof(int),
                           (void **)&(mb_io_ptr->regarray_type), error);
    if (status == MB_SUCCESS)
      status = mb_reallocd(verbose, __FILE__, __LINE__, mb_io_ptr->n_regarray_alloc * sizeof(size_t),
                           (void **)&(mb_io_ptr->regarray_size), error);
    for (int i = mb_io_ptr->n_regarray; i < mb_io_ptr->n_regarray_alloc; i++) {
      mb_io_ptr->regarray_handle[i] = NULL;
      mb_io_ptr->regarray_ptr[i] = NULL;
      mb_io_ptr->regarray_oldptr[i] = NULL;
      mb_io_ptr->regarray_type[i] = MB_MEM_TYPE_NONE;
      mb_io_ptr->regarray_size[i] = 0;
    }
  }

  /* register the array */
  if (status == MB_SUCCESS) {
    int nalloc;
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
    status = mb_reallocd(verbose, __FILE__, __LINE__, nalloc * size, handle, error);
    if (status == MB_SUCCESS) {
      mb_io_ptr->regarray_handle[mb_io_ptr->n_regarray] = (void *)(handle);
      mb_io_ptr->regarray_ptr[mb_io_ptr->n_regarray] = (void *)(*handle);
      mb_io_ptr->regarray_oldptr[mb_io_ptr->n_regarray] = NULL;
      mb_io_ptr->regarray_type[mb_io_ptr->n_regarray] = type;
      mb_io_ptr->regarray_size[mb_io_ptr->n_regarray] = size;
      mb_io_ptr->n_regarray++;
    }
  }

  if (verbose >= 2 || mb_mem_debug) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       *handle:    %p\n", (void *)*handle);
    fprintf(stderr, "dbg2       error:      %d\n", *error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:  %d\n", status);
  }

  return (status);
}
/*--------------------------------------------------------------------*/
int mb_update_arrays(int verbose, void *mbio_ptr, int nbath, int namp, int nss, int *error) {
  if (verbose >= 2 || mb_mem_debug) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
    fprintf(stderr, "dbg2       mb_ptr:     %p\n", (void *)mbio_ptr);
    fprintf(stderr, "dbg2       nbath:      %d\n", nbath);
    fprintf(stderr, "dbg2       namp:       %d\n", namp);
    fprintf(stderr, "dbg2       nss:        %d\n", nss);
  }

  /* get mbio descriptor */
  struct mb_io_struct *mb_io_ptr = (struct mb_io_struct *)mbio_ptr;

  /* reallocate larger arrays if necessary */
  int status = MB_SUCCESS;
  if (nbath > mb_io_ptr->beams_bath_alloc) {
    /* set allocation size */
    if (nbath < 1024) {
      mb_io_ptr->beams_bath_alloc = (nbath / 256) * 256;
      if (nbath % 256 > 0)
        mb_io_ptr->beams_bath_alloc += 256;
    }
    else {
      mb_io_ptr->beams_bath_alloc = (nbath / 1024) * 1024;
      if (nbath % 1024 > 0)
        mb_io_ptr->beams_bath_alloc += 1024;
    }

    /* allocate mb_io_ptr arrays */
    if (status == MB_SUCCESS)
      status = mb_reallocd(verbose, __FILE__, __LINE__, mb_io_ptr->beams_bath_alloc * sizeof(char),
                           (void **)&mb_io_ptr->beamflag, error);
    if (status == MB_SUCCESS)
      status = mb_reallocd(verbose, __FILE__, __LINE__, mb_io_ptr->beams_bath_alloc * sizeof(double),
                           (void **)&mb_io_ptr->bath, error);
    if (status == MB_SUCCESS)
      status = mb_reallocd(verbose, __FILE__, __LINE__, mb_io_ptr->beams_bath_alloc * sizeof(double),
                           (void **)&mb_io_ptr->bath_acrosstrack, error);
    if (status == MB_SUCCESS)
      status = mb_reallocd(verbose, __FILE__, __LINE__, mb_io_ptr->beams_bath_alloc * sizeof(double),
                           (void **)&mb_io_ptr->bath_alongtrack, error);
    if (status == MB_SUCCESS)
      status = mb_reallocd(verbose, __FILE__, __LINE__, mb_io_ptr->beams_bath_alloc * sizeof(int),
                           (void **)&mb_io_ptr->bath_num, error);
    if (status == MB_SUCCESS)
      status = mb_reallocd(verbose, __FILE__, __LINE__, mb_io_ptr->beams_bath_alloc * sizeof(char),
                           (void **)&mb_io_ptr->new_beamflag, error);
    if (status == MB_SUCCESS)
      status = mb_reallocd(verbose, __FILE__, __LINE__, mb_io_ptr->beams_bath_alloc * sizeof(double),
                           (void **)&mb_io_ptr->new_bath, error);
    if (status == MB_SUCCESS)
      status = mb_reallocd(verbose, __FILE__, __LINE__, mb_io_ptr->beams_bath_alloc * sizeof(double),
                           (void **)&mb_io_ptr->new_bath_acrosstrack, error);
    if (status == MB_SUCCESS)
      status = mb_reallocd(verbose, __FILE__, __LINE__, mb_io_ptr->beams_bath_alloc * sizeof(double),
                           (void **)&mb_io_ptr->new_bath_alongtrack, error);
    for (int i = mb_io_ptr->beams_bath_max; i < mb_io_ptr->beams_bath_alloc; i++) {
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

    /* allocate registered arrays */
    for (int i = 0; i < mb_io_ptr->n_regarray; i++) {
      /* allocate the bathymetry dimensioned arrays */
      if (mb_io_ptr->regarray_type[i] == MB_MEM_TYPE_BATHYMETRY && mb_io_ptr->beams_bath_alloc > 0) {
        mb_io_ptr->regarray_oldptr[i] = mb_io_ptr->regarray_ptr[i];
        void **handle = (void **)(mb_io_ptr->regarray_handle[i]);
        status = mb_reallocd(verbose, __FILE__, __LINE__, mb_io_ptr->beams_bath_alloc * mb_io_ptr->regarray_size[i],
                             handle, error);
        mb_io_ptr->regarray_ptr[i] = *handle;
      }
    }

    /* set reallocation flag */
    mb_io_ptr->bath_arrays_reallocated = true;
  }
  if (namp > mb_io_ptr->beams_amp_alloc) {
    /* set allocation size */
    if (namp < 1024) {
      mb_io_ptr->beams_amp_alloc = (namp / 256) * 256;
      if (namp % 256 > 0)
        mb_io_ptr->beams_amp_alloc += 256;
    }
    else {
      mb_io_ptr->beams_amp_alloc = (namp / 1024) * 1024;
      if (namp % 1024 > 0)
        mb_io_ptr->beams_amp_alloc += 1024;
    }

    if (status == MB_SUCCESS)
      status = mb_reallocd(verbose, __FILE__, __LINE__, mb_io_ptr->beams_amp_alloc * sizeof(double),
                           (void **)&mb_io_ptr->amp, error);
    if (status == MB_SUCCESS)
      status = mb_reallocd(verbose, __FILE__, __LINE__, mb_io_ptr->beams_amp_alloc * sizeof(int),
                           (void **)&mb_io_ptr->amp_num, error);
    if (status == MB_SUCCESS)
      status = mb_reallocd(verbose, __FILE__, __LINE__, mb_io_ptr->beams_amp_alloc * sizeof(double),
                           (void **)&mb_io_ptr->new_amp, error);
    for (int i = mb_io_ptr->beams_amp_max; i < mb_io_ptr->beams_amp_alloc; i++) {
      mb_io_ptr->amp[i] = 0.0;
      mb_io_ptr->amp_num[i] = 0;
      mb_io_ptr->new_amp[i] = 0.0;
    }
    mb_io_ptr->beams_amp_max = namp;

    /* allocate registered arrays */
    for (int i = 0; i < mb_io_ptr->n_regarray; i++) {
      /* allocate the amplitude dimensioned arrays */
      if (mb_io_ptr->regarray_type[i] == MB_MEM_TYPE_AMPLITUDE && mb_io_ptr->beams_amp_alloc > 0) {
        mb_io_ptr->regarray_oldptr[i] = mb_io_ptr->regarray_ptr[i];
        void **handle = (void **)(mb_io_ptr->regarray_handle[i]);
        status = mb_reallocd(verbose, __FILE__, __LINE__, mb_io_ptr->beams_amp_alloc * mb_io_ptr->regarray_size[i],
                             handle, error);
        mb_io_ptr->regarray_ptr[i] = *handle;
      }
    }

    /* set reallocation flag */
    mb_io_ptr->amp_arrays_reallocated = true;
  }
  if (nss > mb_io_ptr->pixels_ss_alloc) {
    /* set allocation size */
    if (nss < 1024) {
      mb_io_ptr->pixels_ss_alloc = (nss / 256) * 256;
      if (nss % 256 > 0)
        mb_io_ptr->pixels_ss_alloc += 256;
    }
    else {
      mb_io_ptr->pixels_ss_alloc = (nss / 1024) * 1024;
      if (nss % 1024 > 0)
        mb_io_ptr->pixels_ss_alloc += 1024;
    }

    if (status == MB_SUCCESS)
      status = mb_reallocd(verbose, __FILE__, __LINE__, mb_io_ptr->pixels_ss_alloc * sizeof(double),
                           (void **)&mb_io_ptr->ss, error);
    if (status == MB_SUCCESS)
      status = mb_reallocd(verbose, __FILE__, __LINE__, mb_io_ptr->pixels_ss_alloc * sizeof(double),
                           (void **)&mb_io_ptr->ss_acrosstrack, error);
    if (status == MB_SUCCESS)
      status = mb_reallocd(verbose, __FILE__, __LINE__, mb_io_ptr->pixels_ss_alloc * sizeof(double),
                           (void **)&mb_io_ptr->ss_alongtrack, error);
    if (status == MB_SUCCESS)
      status = mb_reallocd(verbose, __FILE__, __LINE__, mb_io_ptr->pixels_ss_alloc * sizeof(int),
                           (void **)&mb_io_ptr->ss_num, error);
    if (status == MB_SUCCESS)
      status = mb_reallocd(verbose, __FILE__, __LINE__, mb_io_ptr->pixels_ss_alloc * sizeof(double),
                           (void **)&mb_io_ptr->new_ss, error);
    if (status == MB_SUCCESS)
      status = mb_reallocd(verbose, __FILE__, __LINE__, mb_io_ptr->pixels_ss_alloc * sizeof(double),
                           (void **)&mb_io_ptr->new_ss_acrosstrack, error);
    if (status == MB_SUCCESS)
      status = mb_reallocd(verbose, __FILE__, __LINE__, mb_io_ptr->pixels_ss_alloc * sizeof(double),
                           (void **)&mb_io_ptr->new_ss_alongtrack, error);
    for (int i = mb_io_ptr->pixels_ss_max; i < mb_io_ptr->pixels_ss_alloc; i++) {
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
    for (int i = 0; i < mb_io_ptr->n_regarray; i++) {
      /* allocate the sidescan dimensioned arrays */
      if (mb_io_ptr->regarray_type[i] == MB_MEM_TYPE_SIDESCAN && mb_io_ptr->pixels_ss_alloc > 0) {
        mb_io_ptr->regarray_oldptr[i] = mb_io_ptr->regarray_ptr[i];
        void **handle = (void **)(mb_io_ptr->regarray_handle[i]);
        status = mb_reallocd(verbose, __FILE__, __LINE__, mb_io_ptr->pixels_ss_alloc * mb_io_ptr->regarray_size[i],
                             handle, error);
        mb_io_ptr->regarray_ptr[i] = *handle;
      }
    }

    /* set reallocation flag */
    mb_io_ptr->ss_arrays_reallocated = true;
  }

  /* deal with a memory allocation failure */
  if (status == MB_FAILURE) {
    /* free the registered arrays */
    for (int i = 0; i < mb_io_ptr->n_regarray; i++) {
      /* free all arrays */
      if (mb_io_ptr->regarray_handle[i] != NULL) {
        status = mb_free(verbose, (void **)(mb_io_ptr->regarray_handle[i]), error);
      }
    }

    /* free the standard arrays */
    status = mb_freed(verbose, __FILE__, __LINE__, (void **)&mb_io_ptr->beamflag, error);
    status = mb_freed(verbose, __FILE__, __LINE__, (void **)&mb_io_ptr->bath, error);
    status = mb_freed(verbose, __FILE__, __LINE__, (void **)&mb_io_ptr->amp, error);
    status = mb_freed(verbose, __FILE__, __LINE__, (void **)&mb_io_ptr->bath_acrosstrack, error);
    status = mb_freed(verbose, __FILE__, __LINE__, (void **)&mb_io_ptr->bath_alongtrack, error);
    status = mb_freed(verbose, __FILE__, __LINE__, (void **)&mb_io_ptr->bath_num, error);
    status = mb_freed(verbose, __FILE__, __LINE__, (void **)&mb_io_ptr->amp_num, error);
    status = mb_freed(verbose, __FILE__, __LINE__, (void **)&mb_io_ptr->ss, error);
    status = mb_freed(verbose, __FILE__, __LINE__, (void **)&mb_io_ptr->ss_acrosstrack, error);
    status = mb_freed(verbose, __FILE__, __LINE__, (void **)&mb_io_ptr->ss_alongtrack, error);
    status = mb_freed(verbose, __FILE__, __LINE__, (void **)&mb_io_ptr->ss_num, error);
    status = mb_freed(verbose, __FILE__, __LINE__, (void **)&mb_io_ptr->beamflag, error);
    status = mb_freed(verbose, __FILE__, __LINE__, (void **)&mb_io_ptr->new_bath, error);
    status = mb_freed(verbose, __FILE__, __LINE__, (void **)&mb_io_ptr->new_amp, error);
    status = mb_freed(verbose, __FILE__, __LINE__, (void **)&mb_io_ptr->new_bath_acrosstrack, error);
    status = mb_freed(verbose, __FILE__, __LINE__, (void **)&mb_io_ptr->new_bath_alongtrack, error);
    status = mb_freed(verbose, __FILE__, __LINE__, (void **)&mb_io_ptr->new_ss, error);
    status = mb_freed(verbose, __FILE__, __LINE__, (void **)&mb_io_ptr->new_ss_acrosstrack, error);
    status = mb_freed(verbose, __FILE__, __LINE__, (void **)&mb_io_ptr->new_ss_alongtrack, error);

    /* free the mb_io structure */
    status = mb_free(verbose, (void **)&mb_io_ptr, error);
    mb_io_ptr->beams_bath_alloc = 0;
    mb_io_ptr->beams_amp_alloc = 0;
    mb_io_ptr->pixels_ss_alloc = 0;
    status = MB_FAILURE;
    *error = MB_ERROR_MEMORY_FAIL;
  }

  if (verbose >= 2 || mb_mem_debug) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       error:      %d\n", *error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:  %d\n", status);
  }

  return (status);
}
/*--------------------------------------------------------------------*/
int mb_update_arrayptr(int verbose, void *mbio_ptr, void **handle, int *error) {
  if (verbose >= 2 || mb_mem_debug) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
    fprintf(stderr, "dbg2       handle:     %p\n", (void *)handle);
    fprintf(stderr, "dbg2       *handle:    %p\n", (void *)*handle);
  }

  /* get mbio descriptor */
  struct mb_io_struct *mb_io_ptr = (struct mb_io_struct *)mbio_ptr;

  /* look for handle in registered arrays */
  bool found = false;
  for (int i = 0; i < mb_io_ptr->n_regarray && !found; i++) {
    if (*handle == mb_io_ptr->regarray_oldptr[i]) {
      *handle = mb_io_ptr->regarray_ptr[i];
      found = true;
    }
  }

  const int status = MB_SUCCESS;

  if (verbose >= 2 || mb_mem_debug) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       *handle:    %p\n", (void *)*handle);
    fprintf(stderr, "dbg2       error:      %d\n", *error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:  %d\n", status);
  }

  return (status);
}
/*--------------------------------------------------------------------*/
int mb_list_arrays(int verbose, void *mbio_ptr, int *error) {
  if (verbose >= 2 || mb_mem_debug) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
    fprintf(stderr, "dbg2       mbio_ptr:   %p\n", (void *)mbio_ptr);
  }

  /* get mbio descriptor */
  struct mb_io_struct *mb_io_ptr = (struct mb_io_struct *)mbio_ptr;

  /* look for handle in registered arrays */
  fprintf(stderr, "\nRegistered Array List (%d arrays):\n", mb_io_ptr->n_regarray);
  for (int i = 0; i < mb_io_ptr->n_regarray; i++) {
    fprintf(stderr, "Array %d: handle:%p ptr:%p type:%d size:%zu\n", i, (void *)mb_io_ptr->regarray_handle[i],
            (void *)mb_io_ptr->regarray_ptr[i], mb_io_ptr->regarray_type[i], mb_io_ptr->regarray_size[i]);
  }

  const int status = MB_SUCCESS;

  if (verbose >= 2 || mb_mem_debug) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       error:      %d\n", *error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:  %d\n", status);
  }

  return (status);
}
/*--------------------------------------------------------------------*/
int mb_deall_ioarrays(int verbose, void *mbio_ptr, int *error) {
  if (verbose >= 2 || mb_mem_debug) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
    fprintf(stderr, "dbg2       mb_ptr:     %p\n", (void *)mbio_ptr);
  }

  /* get mbio descriptor */
  struct mb_io_struct *mb_io_ptr = (struct mb_io_struct *)mbio_ptr;

  /* deallocate mb_io_ptr arrays */
  int status = MB_SUCCESS;
  status &= mb_freed(verbose, __FILE__, __LINE__, (void **)&mb_io_ptr->beamflag, error);
  status &= mb_freed(verbose, __FILE__, __LINE__, (void **)&mb_io_ptr->bath, error);
  status &= mb_freed(verbose, __FILE__, __LINE__, (void **)&mb_io_ptr->bath_acrosstrack, error);
  status &= mb_freed(verbose, __FILE__, __LINE__, (void **)&mb_io_ptr->bath_alongtrack, error);
  status &= mb_freed(verbose, __FILE__, __LINE__, (void **)&mb_io_ptr->bath_num, error);
  status &= mb_freed(verbose, __FILE__, __LINE__, (void **)&mb_io_ptr->new_beamflag, error);
  status &= mb_freed(verbose, __FILE__, __LINE__, (void **)&mb_io_ptr->new_bath, error);
  status &= mb_freed(verbose, __FILE__, __LINE__, (void **)&mb_io_ptr->new_bath_acrosstrack, error);
  status &= mb_freed(verbose, __FILE__, __LINE__, (void **)&mb_io_ptr->new_bath_alongtrack, error);
  status &= mb_freed(verbose, __FILE__, __LINE__, (void **)&mb_io_ptr->amp, error);
  status &= mb_freed(verbose, __FILE__, __LINE__, (void **)&mb_io_ptr->amp_num, error);
  status &= mb_freed(verbose, __FILE__, __LINE__, (void **)&mb_io_ptr->new_amp, error);
  status &= mb_freed(verbose, __FILE__, __LINE__, (void **)&mb_io_ptr->ss, error);
  status &= mb_freed(verbose, __FILE__, __LINE__, (void **)&mb_io_ptr->ss_acrosstrack, error);
  status &= mb_freed(verbose, __FILE__, __LINE__, (void **)&mb_io_ptr->ss_alongtrack, error);
  status &= mb_freed(verbose, __FILE__, __LINE__, (void **)&mb_io_ptr->ss_num, error);
  status &= mb_freed(verbose, __FILE__, __LINE__, (void **)&mb_io_ptr->new_ss, error);
  status &= mb_freed(verbose, __FILE__, __LINE__, (void **)&mb_io_ptr->new_ss_acrosstrack, error);
  status &= mb_freed(verbose, __FILE__, __LINE__, (void **)&mb_io_ptr->new_ss_alongtrack, error);
  mb_io_ptr->beams_bath_max = 0;
  mb_io_ptr->beams_bath_alloc = 0;
  mb_io_ptr->beams_amp_max = 0;
  mb_io_ptr->beams_amp_alloc = 0;
  mb_io_ptr->pixels_ss_max = 0;
  mb_io_ptr->pixels_ss_alloc = 0;

  /* deallocate registered arrays */
  for (int i = 0; i < mb_io_ptr->n_regarray; i++) {
    if (status == MB_SUCCESS && mb_io_ptr->regarray_handle[i] != NULL) {
      status = mb_freed(verbose, __FILE__, __LINE__, (void **)(mb_io_ptr->regarray_handle[i]), error);
    }
  }
  mb_io_ptr->n_regarray = 0;
  mb_io_ptr->n_regarray_alloc = 0;

  /* deallocate registry itself */
  if (status == MB_SUCCESS)
    status = mb_freed(verbose, __FILE__, __LINE__, (void **)&(mb_io_ptr->regarray_handle), error);
  if (status == MB_SUCCESS)
    status = mb_freed(verbose, __FILE__, __LINE__, (void **)&(mb_io_ptr->regarray_ptr), error);
  if (status == MB_SUCCESS)
    status = mb_freed(verbose, __FILE__, __LINE__, (void **)&(mb_io_ptr->regarray_oldptr), error);
  if (status == MB_SUCCESS)
    status = mb_freed(verbose, __FILE__, __LINE__, (void **)&(mb_io_ptr->regarray_type), error);
  if (status == MB_SUCCESS)
    status = mb_freed(verbose, __FILE__, __LINE__, (void **)&(mb_io_ptr->regarray_size), error);

  if (verbose >= 2 || mb_mem_debug) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       error:      %d\n", *error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:  %d\n", status);
  }

  return (status);
}
/*--------------------------------------------------------------------*/
