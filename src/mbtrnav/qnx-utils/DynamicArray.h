/****************************************************************************/
/* Copyright (c) 2000 MBARI                                                 */
/* MBARI Proprietary Information. All rights reserved.                      */
/****************************************************************************/
/* Summary  :                                                               */
/* Filename : DynamicArray.h                                                */
/* Author   :                                                               */
/* Project  :                                                               */
/* Version  : 1.0                                                           */
/* Created  : 02/07/2000                                                    */
/* Modified :                                                               */
/* Archived :                                                               */
/****************************************************************************/
/* Modification History:                                                    */
/****************************************************************************/
#ifndef DYNAMICARRAY_H
#define DYNAMICARRAY_H
#define MAXINT 99999

/*
$Log$
Revision 1.1.2.2  2019/07/10 17:15:46  rob
Now includes support for Mac OS if you uncomment appropriate statements.

Revision 1.1.2.1  2018/11/20 20:03:21  henthorn
QNX-additions is a library of routines for use outside of QNX - stripped down version of DataLog, TimeP, and NavUtils. auv-qnx dependencies removed. Link to libqnx.a.

Revision 1.4  2007/03/16 22:54:30  henthorn
Fix insert function

Revision 1.3  2007/03/08 19:27:57  henthorn
Add functions for inserting and deleting from the array

Revision 1.2  2000/02/07 23:57:00  pean
Added revised banner to include MBARI Proprietary Information

Revision 1.1  1999/12/08 17:02:38  pean
Added automatic dependency generation to makefile. Moved all auv framework
sources into framework subdirectory.

Revision 1.2  1999/10/20 22:54:56  oreilly
*** empty log message ***

Revision 1.1.1.1  1999/06/24 20:08:57  oreilly

Revision 1.5  1998/06/09 21:13:21  oreilly
VxWorks needs memLib.h instead of malloc.h

Revision 1.4  1997/03/20 12:27:01  oreilly
*** empty log message ***

 * Revision 1.3  96/10/28  09:00:36  09:00:36  oreilly (Thomas C. O'Reilly)
 * *** empty log message ***
 * 
*/

#include <stdio.h>
#include <stdlib.h>

#if !defined(__APPLE__)
#include <malloc.h>
#endif

#define MAX(a,b)  (((a) > (b)) ? (a) : (b))

typedef unsigned char Boolean;
#define True 1
#define False 0

/*
CLASS 
DynamicArray

DESCRIPTION
Template for an array that grows dynamically as elements are
added.

AUTHOR
Tom O'Reilly
*/
template<class T>
class DynamicArray {

  public:

  ///////////////////////////////////////////////////////////////////
  // Constructor
  // [input] incr: Memory is allocated in "incr" element increments
  DynamicArray(int incr = 10) {

    if (incr <= 0)    
    {
      mem_error = True;
      return;
    }
    n_elems = 0;
    n_allocd = 0;
    alloc_incr = incr;
  }

  ///////////////////////////////////////////////////////////////////
  // Destructor
  ~DynamicArray() {
    if (n_allocd)
      free(array);
  }

  ///////////////////////////////////////////////////////////////////
  // Set specified element to specified value
  int set(int i, T* val)
  {
    if (i < 0)
    {
      mem_error = True;
      return -1;
    }

    if (i >= n_allocd)
    {
      // Compute blocks needed to index specified element
      int nblocks = (i + 1) / alloc_incr;
      if ((i+1) % alloc_incr)
        nblocks++;

      if (!n_allocd)
      {
        n_allocd = nblocks * alloc_incr;
        if ((array = (T *)malloc(n_allocd*sizeof(T))) == NULL)
        {
          mem_error = True;
          return -1;
        }
      }
      else
      {
        n_allocd += (nblocks * alloc_incr);
        if ((array = (T *)realloc(array, n_allocd*sizeof(T))) == NULL)
        {
          mem_error = True;
          return -1;
        }
      }
    }
    array[i] = *val;
    mem_error = False;
    n_elems = MAX((i+1), n_elems);
    return 0;
  }

  ///////////////////////////////////////////////////////////////////
  // Insert after the given position in the array
  // Research-grade attempt, here. Kinda sloppy.
  //
  int insert(int pos, T* val)
  {
    if (pos < 0 || pos >= n_elems)
    {
      mem_error = True;
      return -2;
    }

    // Starting at the end of the array, move elements down
    // until we reach the spot we need to insert val
    //
    pos++;   // Insert after pos
    for (int i = n_elems; i > pos; i--)
    {
      T temp;
      if (0 != get(i-1, &temp)) return -3;
      if (0 != set( i, &temp)) return -4;
    }

    // Set array element pos to val
    //
    return set(pos, val);
  }

  ///////////////////////////////////////////////////////////////////
  // Remove at the given position in the array, return element that
  // was removed
  int remove(int pos, T* val)
  {
    if (pos < 0 || pos >= n_elems)
    {
      mem_error = True;
      return -1;
    }
    
    if (0 != get(pos, val)) return -1;
    // Starting at the position to eliminate, move all the elements
    // below it up one position
    //
    for (int i = pos; i < n_elems-1; i++)
    {
      T temp;
      if (0 != get(i+1, &temp)) return -1;
      if (0 != set(  i, &temp)) return -1;
    }
    n_elems -= 1;
      
    mem_error = False;
    return 0;
  }
  
  ///////////////////////////////////////////////////////////////////
  // Get value of specified element
  int get(int i, T* val)
  {
    if (i < 0 || i >= n_elems)
    {
      mem_error = True;
      return -1;
    }
    *val = array[i];
    mem_error = False;
    return 0;
  }

  ///////////////////////////////////////////////////////////////////
  // Number of elements currently filled
  int size() 
  {
    return n_elems;
  }
  
  int get_n_elems() { return n_elems; }
  int get_n_allocd() { return n_allocd; }
  T *get_array() { return array; }

  ///////////////////////////////////////////////////////////////////
  // Add element to end of array
  int add(T* val)
  { 
    return set(size(), val);
  }
  
  ///////////////////////////////////////////////////////////////////
  // Remove all elements
  void clear()
  {
    n_elems = 0;
  }
  
  // Check for memory allocation error
  Boolean mem_error;
 
  private:
  T *array;
  int n_elems;
  int n_allocd;

  ///////////////////////////////////////////////////////////////////
  // Size of malloc'ed and realloc'ed blocks (in # elems)
  int alloc_incr;

};


#endif
