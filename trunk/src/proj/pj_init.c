/******************************************************************************
 * $Id: pj_init.c 1630 2009-09-24 02:14:06Z warmerdam $
 *
 * Project:  PROJ.4
 * Purpose:  Initialize projection object from string definition.  Includes
 *           pj_init(), pj_init_plus() and pj_free() function.
 * Author:   Gerald Evenden, Frank Warmerdam <warmerdam@pobox.com>
 *
 ******************************************************************************
 * Copyright (c) 1995, Gerald Evenden
 * Copyright (c) 2002, Frank Warmerdam <warmerdam@pobox.com>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 *****************************************************************************/

#define PJ_LIB__
#include <projects.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <locale.h>

PJ_CVSID("$Id: pj_init.c 1630 2009-09-24 02:14:06Z warmerdam $");

extern FILE *pj_open_lib(char *, char *);

/************************************************************************/
/*                              get_opt()                               */
/************************************************************************/
static paralist *
get_opt(paralist **start, FILE *fid, char *name, paralist *next) {
    char sword[302], *word = sword+1;
    int first = 1, len, c;

    len = strlen(name);
    *sword = 't';
    while (fscanf(fid, "%300s", word) == 1) {
        if (*word == '#') /* skip comments */
            while((c = fgetc(fid)) != EOF && c != '\n') ;
        else if (*word == '<') { /* control name */
            if (first && !strncmp(name, word + 1, len)
                && word[len + 1] == '>')
                first = 0;
            else if (!first && *word == '<') {
                while((c = fgetc(fid)) != EOF && c != '\n') ;
                break;
            }
        } else if (!first && !pj_param(*start, sword).i) {
            /* don't default ellipse if datum, ellps or any earth model
               information is set. */
            if( strncmp(word,"ellps=",6) != 0 
                || (!pj_param(*start, "tdatum").i 
                    && !pj_param(*start, "tellps").i 
                    && !pj_param(*start, "ta").i 
                    && !pj_param(*start, "tb").i 
                    && !pj_param(*start, "trf").i 
                    && !pj_param(*start, "tf").i) )
            {
                next = next->next = pj_mkparam(word);
            }
        }
    }

    if (errno == 25)
        errno = 0;
    return next;
}

/************************************************************************/
/*                            get_defaults()                            */
/************************************************************************/
static paralist *
get_defaults(paralist **start, paralist *next, char *name) {
	FILE *fid;

	if (fid = pj_open_lib("proj_def.dat", "rt")) {
		next = get_opt(start, fid, "general", next);
		rewind(fid);
		next = get_opt(start, fid, name, next);
		(void)fclose(fid);
	}
	if (errno)
		errno = 0; /* don't care if can't open file */
	return next;
}

/************************************************************************/
/*                              get_init()                              */
/************************************************************************/
static paralist *
get_init(paralist **start, paralist *next, char *name) {
	char fname[MAX_PATH_FILENAME+ID_TAG_MAX+3], *opt;
	FILE *fid;
	paralist *init_items = NULL;
	const paralist *orig_next = next;

	(void)strncpy(fname, name, MAX_PATH_FILENAME + ID_TAG_MAX + 1);
	
	/* 
	** Search for file/key pair in cache 
	*/
	
	init_items = pj_search_initcache( name );
	if( init_items != NULL )
	  {
	    next->next = init_items;
	    while( next->next != NULL )
	      next = next->next;
	    return next;
	  }

	/*
	** Otherwise we try to open the file and search for it.
	*/
	if (opt = strrchr(fname, ':'))
		*opt++ = '\0';
	else { pj_errno = -3; return(0); }
	if (fid = pj_open_lib(fname, "rt"))
		next = get_opt(start, fid, opt, next);
	else
		return(0);
	(void)fclose(fid);
	if (errno == 25)
		errno = 0; /* unknown problem with some sys errno<-25 */

	/* 
	** If we seem to have gotten a result, insert it into the 
	** init file cache.
	*/
	if( next != NULL && next != orig_next )
	  pj_insert_initcache( name, orig_next->next );

	return next;
}

/************************************************************************/
/*                            pj_init_plus()                            */
/*                                                                      */
/*      Same as pj_init() except it takes one argument string with      */
/*      individual arguments preceeded by '+', such as "+proj=utm       */
/*      +zone=11 +ellps=WGS84".                                         */
/************************************************************************/

PJ *
pj_init_plus( const char *definition )

{
#define MAX_ARG 200
    char	*argv[MAX_ARG];
    char	*defn_copy;
    int		argc = 0, i;
    PJ	        *result;
    
    /* make a copy that we can manipulate */
    defn_copy = (char *) pj_malloc( strlen(definition)+1 );
    strcpy( defn_copy, definition );

    /* split into arguments based on '+' and trim white space */

    for( i = 0; defn_copy[i] != '\0'; i++ )
    {
        switch( defn_copy[i] )
        {
          case '+':
            if( i == 0 || defn_copy[i-1] == '\0' )
            {
                if( argc+1 == MAX_ARG )
                {
                    pj_errno = -44;
                    return NULL;
                }
                
                argv[argc++] = defn_copy + i + 1;
            }
            break;

          case ' ':
          case '\t':
          case '\n':
            defn_copy[i] = '\0';
            break;

          default:
            /* do nothing */;
        }
    }

    /* perform actual initialization */
    result = pj_init( argc, argv );

    pj_dalloc( defn_copy );

    return result;
}

/************************************************************************/
/*                              pj_init()                               */
/*                                                                      */
/*      Main entry point for initialing a PJ projections                */
/*      definition.  Note that the projection specific function is      */
/*      called to do the initial allocation so it can be created        */
/*      large enough to hold projection specific parameters.            */
/************************************************************************/

PJ *
pj_init(int argc, char **argv) {
	char *s, *name;
        paralist *start = NULL;
	PJ *(*proj)(PJ *);
	paralist *curr;
	int i;
	PJ *PIN = 0;
        const char *old_locale;

	errno = pj_errno = 0;
        start = NULL;

        old_locale = setlocale(LC_NUMERIC, NULL); 
        setlocale(LC_NUMERIC,"C");

	/* put arguments into internal linked list */
	if (argc <= 0) { pj_errno = -1; goto bum_call; }
	for (i = 0; i < argc; ++i)
		if (i)
			curr = curr->next = pj_mkparam(argv[i]);
		else
			start = curr = pj_mkparam(argv[i]);
	if (pj_errno) goto bum_call;

	/* check if +init present */
	if (pj_param(start, "tinit").i) {
		paralist *last = curr;

		if (!(curr = get_init(&start, curr, pj_param(start, "sinit").s)))
			goto bum_call;
		if (curr == last) { pj_errno = -2; goto bum_call; }
	}

	/* find projection selection */
	if (!(name = pj_param(start, "sproj").s))
		{ pj_errno = -4; goto bum_call; }
	for (i = 0; (s = pj_list[i].id) && strcmp(name, s) ; ++i) ;
	if (!s) { pj_errno = -5; goto bum_call; }

	/* set defaults, unless inhibited */
	if (!pj_param(start, "bno_defs").i)
		curr = get_defaults(&start, curr, name);
	proj = (PJ *(*)(PJ *)) pj_list[i].proj;

	/* allocate projection structure */
	if (!(PIN = (*proj)(0))) goto bum_call;
	PIN->params = start;
        PIN->is_latlong = 0;
        PIN->is_geocent = 0;
        PIN->long_wrap_center = 0.0;

        /* set datum parameters */
        if (pj_datum_set(start, PIN)) goto bum_call;

	/* set ellipsoid/sphere parameters */
	if (pj_ell_set(start, &PIN->a, &PIN->es)) goto bum_call;

        PIN->a_orig = PIN->a;
        PIN->es_orig = PIN->es;

	PIN->e = sqrt(PIN->es);
	PIN->ra = 1. / PIN->a;
	PIN->one_es = 1. - PIN->es;
	if (PIN->one_es == 0.) { pj_errno = -6; goto bum_call; }
	PIN->rone_es = 1./PIN->one_es;

        /* Now that we have ellipse information check for WGS84 datum */
        if( PIN->datum_type == PJD_3PARAM 
            && PIN->datum_params[0] == 0.0
            && PIN->datum_params[1] == 0.0
            && PIN->datum_params[2] == 0.0
            && PIN->a == 6378137.0
            && ABS(PIN->es - 0.006694379990) < 0.000000000050 )/*WGS84/GRS80*/
        {
            PIN->datum_type = PJD_WGS84;
        }
        
	/* set PIN->geoc coordinate system */
	PIN->geoc = (PIN->es && pj_param(start, "bgeoc").i);

	/* over-ranging flag */
	PIN->over = pj_param(start, "bover").i;

	/* longitude center for wrapping */
	PIN->long_wrap_center = pj_param(start, "rlon_wrap").f;

	/* central meridian */
	PIN->lam0=pj_param(start, "rlon_0").f;

	/* central latitude */
	PIN->phi0 = pj_param(start, "rlat_0").f;

	/* false easting and northing */
	PIN->x0 = pj_param(start, "dx_0").f;
	PIN->y0 = pj_param(start, "dy_0").f;

	/* general scaling factor */
	if (pj_param(start, "tk_0").i)
		PIN->k0 = pj_param(start, "dk_0").f;
	else if (pj_param(start, "tk").i)
		PIN->k0 = pj_param(start, "dk").f;
	else
		PIN->k0 = 1.;
	if (PIN->k0 <= 0.) {
		pj_errno = -31;
		goto bum_call;
	}

	/* set units */
	s = 0;
	if (name = pj_param(start, "sunits").s) { 
		for (i = 0; (s = pj_units[i].id) && strcmp(name, s) ; ++i) ;
		if (!s) { pj_errno = -7; goto bum_call; }
		s = pj_units[i].to_meter;
	}
	if (s || (s = pj_param(start, "sto_meter").s)) {
		PIN->to_meter = strtod(s, &s);
		if (*s == '/') /* ratio number */
			PIN->to_meter /= strtod(++s, 0);
		PIN->fr_meter = 1. / PIN->to_meter;
	} else
		PIN->to_meter = PIN->fr_meter = 1.;

	/* prime meridian */
	s = 0;
	if (name = pj_param(start, "spm").s) { 
            const char *value = NULL;
            char *next_str = NULL;

            for (i = 0; pj_prime_meridians[i].id != NULL; ++i )
            {
                if( strcmp(name,pj_prime_meridians[i].id) == 0 )
                {
                    value = pj_prime_meridians[i].defn;
                    break;
                }
            }
            
            if( value == NULL 
                && (dmstor(name,&next_str) != 0.0  || *name == '0')
                && *next_str == '\0' )
                value = name;

            if (!value) { pj_errno = -46; goto bum_call; }
            PIN->from_greenwich = dmstor(value,NULL);
	}
        else
            PIN->from_greenwich = 0.0;

	/* projection specific initialization */
	if (!(PIN = (*proj)(PIN)) || errno || pj_errno) {
bum_call: /* cleanup error return */
		if (!pj_errno)
			pj_errno = errno;
		if (PIN)
			pj_free(PIN);
		else
			for ( ; start; start = curr) {
				curr = start->next;
				pj_dalloc(start);
			}
		PIN = 0;
	}
        setlocale(LC_NUMERIC,old_locale);

	return PIN;
}

/************************************************************************/
/*                              pj_free()                               */
/*                                                                      */
/*      This is the application callable entry point for destroying     */
/*      a projection definition.  It does work generic to all           */
/*      projection types, and then calls the projection specific        */
/*      free function (P->pfree()) to do local work.  This maps to      */
/*      the FREEUP code in the individual projection source files.      */
/************************************************************************/

void
pj_free(PJ *P) {
	if (P) {
		paralist *t = P->params, *n;

		/* free parameter list elements */
		for (t = P->params; t; t = n) {
			n = t->next;
			pj_dalloc(t);
		}

		/* free projection parameters */
		P->pfree(P);
	}
}


