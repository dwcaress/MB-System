#     The MB-system:	Makefile	8/12/93
#	$Id: Makefile,v 4.1 1994-04-12 21:03:58 caress Exp $
#
#     Copyright (c) 1993 by 
#     D. W. Caress (caress@lamont.ldgo.columbia.edu)
#     and D. N. Chayes (dale@lamont.ldgo.columbia.edu)
#     Lamont-Doherty Earth Observatory
#     Palisades, NY  10964
#
#     See README file for copying and redistribution conditions.
#
# Makefile for MB-system in the src/realtime directory level
# Author:	D. W. Caress
# Date:		August 12, 1993
#
# $Log: not supported by cvs2svn $
# Revision 4.0  1994/03/06  00:12:00  caress
# First cut at version 4.0
#
# Revision 1.1  1993/11/05  18:56:23  caress
# Initial revision
#
#
#

# *** CHANGE THESE TO FIT YOUR LOCAL ENVIRONMENT ***
#
# Locations of GMT libraries, binaries, and include files:
#
# - For L-DEO installation:
GMTLIBDIR = /ocean/mgg/gmt2/sun4/lib
GMTINCDIR = /ocean/mgg/gmt2/src
GMTBINDIR = /ocean/mgg/gmt2/sun4
#
# - For Ewing installation:
#GMTLIBDIR = /net/heezen/packages/gmt2/bin
#GMTINCDIR = /net/heezen/packages/gmt2/src
#GMTBINDIR = /net/heezen/packages/gmt2/bin
#
# - For generic installation:
#GMTLIBDIR = /usr/local/lib
#GMTINCDIR = /usr/local/include
#GMTBINDIR = /usr/local/bin
#
#
# Compile and load flags:
#
# - For installations under SunOs:
CFLAGS = -O2
FFLAGS = -O2
FWITHCFLAGS = -lF77
LFLAGS = -lm
#
# - For installations under IRIX:
#CFLAGS = -O2
#FFLAGS = -O2
#FWITHCFLAGS = -lF77
#LFLAGS = -lm -lsun
#
# *** LEAVE EVERYTHING ELSE BELOW ALONE ***

### sets locations of vital objects
BINDIR = ../../bin
LIBDIR = ../../lib
INCDIR = ../../include
MANDIR = ../../man/man1

### make all makes the MB-system macros and man pages
all:	$(BINDIR)/mbcontourfilter $(MANDIR)/mbcontourfilter.1 \
	$(BINDIR)/mbplotfilter $(MANDIR)/mbplotfilter.1 \
	$(BINDIR)/mbm_calcomp_plot $(MANDIR)/mbm_calcomp_plot.1

### make clean removes the object files
clean:
	rm -f *.o

### link or copy the executables
$(BINDIR)/mbcontourfilter: mbcontourfilter.o ../gmt/mb_contour.o mb_track.o label.o letter.o blocka.o
	$(CC) $(CFLAGS) mbcontourfilter.o ../gmt/mb_contour.o mb_track.o label.o letter.o blocka.o ../../lib/libmbio.a $(FWITHCFLAGS) $(LFLAGS) -o $(BINDIR)/mbcontourfilter

$(BINDIR)/mbplotfilter:	mbplotfilter.o
	$(CC) mbplotfilter.o -g $(LFLAGS) -lgraphics  -o $(BINDIR)/mbplotfilter

$(BINDIR)/mbm_calcomp_plot: mbm_calcomp_plot.perl
	cp mbm_calcomp_plot.perl $(BINDIR)/mbm_calcomp_plot
	chmod +x $(BINDIR)/mbm_calcomp_plot

### compile the source files
mbcontour.o:	mbcontourfilter.c  $(INCDIR)/mb_status.h
	$(CC) -c $(CFLAGS) -I$(GMTINCDIR) mbcontourfilter.c
mbplotfilter.o:	mbplotfilter.c
	$(CC) -c -g -I$(GMTINCDIR) mbplotfilter.c
../gmt/mb_contour.o:	../gmt/mb_contour.c  $(INCDIR)/mb_status.h
	cd ../gmt; make all
mb_track.o:	mb_track.c  $(INCDIR)/mb_status.h
	$(CC) -c $(CFLAGS) mb_track.c
label.o: label.f
	$(FC) -c label.f -o label.o
letter.o: letter.f
	$(FC) -c letter.f -o letter.o
blocka.o: blocka.f
	$(FC) -c blocka.f -o blocka.o

# copy the man pages to the man page directory
$(MANDIR)/mbcontourfilter.1:	mbcontourfilter.1
	cp mbcontourfilter.1 $(MANDIR)/mbcontourfilter.1
	chmod 664 $(MANDIR)/mbcontourfilter.1
$(MANDIR)/mbplotfilter.1:	mbplotfilter.1
	cp mbplotfilter.1 $(MANDIR)/mbplotfilter.1
	chmod 664 $(MANDIR)/mbplotfilter.1
$(MANDIR)/mbm_calcomp_plot.1:	mbm_calcomp_plot.1
	cp mbm_calcomp_plot.1 $(MANDIR)/mbm_calcomp_plot.1
	chmod 664 $(MANDIR)/mbm_calcomp_plot.1

