#     The MB-system:	Makefile	3.00  1/21/93
#	$Id: Makefile,v 1.5 1993-05-25 20:13:53 caress Exp $
#
#     Copyright (c) 1993 by 
#     D. W. Caress (caress@lamont.ldgo.columbia.edu)
#     and D. N. Chayes (dale@lamont.ldgo.columbia.edu)
#     Lamont-Doherty Earth Observatory
#     Palisades, NY  10964
#
#     See README file for copying and redistribution conditions.
#
# Makefile for MB-system at the src/utilities directory level
# Author:	D. W. Caress
# Date:		January 21, 1993
#
# $Log: not supported by cvs2svn $
# Revision 1.4  1993/05/25  20:01:39  caress
# Fixed chmod commands.
#
# Revision 1.3  1993/05/25  19:26:27  caress
# Added chmod command to cp lines so that later makes can
# overwrite the copied man pages.
#
# Revision 1.2  1993/05/25  17:29:24  caress
# Fixed location of GMT stuff.
#
#

CFLAGS = -O2
FFLAGS = -O2
FWITHCFLAGS = -lF77
LFLAGS = -lm

BINDIR = ../../bin
LIBDIR = ../../lib
INCDIR = ../../include
MANDIR = ../../man/man1
GMTBINDIR = /net/heezen/packages/gmt2/bin
GMTINCDIR = /net/heezen/packages/gmt2/src

### make all makes the MB-system utilites and man pages
all:	$(MANDIR)/mbsystem.1 \
	$(BINDIR)/mbformat $(MANDIR)/mbformat.1 \
	$(BINDIR)/mbdefaults $(MANDIR)/mbdefaults.1 \
	$(BINDIR)/mbinfo $(MANDIR)/mbinfo.1 \
	$(BINDIR)/mbcopy $(MANDIR)/mbcopy.1 \
	$(BINDIR)/mblist $(MANDIR)/mblist.1 \
	$(BINDIR)/mbmerge $(MANDIR)/mbmerge.1 \
	$(BINDIR)/mbclean $(MANDIR)/mbclean.1 \
	$(BINDIR)/mbunclean $(MANDIR)/mbunclean.1 \
	$(BINDIR)/mbgrid $(MANDIR)/mbgrid.1 \
	$(BINDIR)/mblevitus $(MANDIR)/mblevitus.1 \
	$(BINDIR)/hsbath $(MANDIR)/hsbath.1 \
	$(BINDIR)/hsveldump $(MANDIR)/hsveldump.1 \
	$(BINDIR)/mbrollbias $(MANDIR)/mbrollbias.1 \
	$(BINDIR)/mbsynth

### make clean removes the object files
clean:
	rm -f *.o

### link the executables
$(BINDIR)/mbformat:	mbformat.o $(LIBDIR)/libmbio.a
	$(CC) $(CFLAGS) mbformat.o $(LIBDIR)/libmbio.a $(LFLAGS) -o $(BINDIR)/mbformat
$(BINDIR)/mbdefaults:	mbdefaults.o $(LIBDIR)/libmbio.a
	$(CC) $(CFLAGS) mbdefaults.o $(LIBDIR)/libmbio.a $(LFLAGS) -o $(BINDIR)/mbdefaults
$(BINDIR)/mbinfo:	mbinfo.o $(LIBDIR)/libmbio.a
	$(CC) $(CFLAGS) mbinfo.o $(LIBDIR)/libmbio.a $(LFLAGS) -o $(BINDIR)/mbinfo
$(BINDIR)/mbcopy:	mbcopy.o $(LIBDIR)/libmbio.a
	$(CC) $(CFLAGS) mbcopy.o $(LIBDIR)/libmbio.a $(LFLAGS) -o $(BINDIR)/mbcopy
$(BINDIR)/mblist:	mblist.o $(LIBDIR)/libmbio.a
	$(CC) $(CFLAGS) mblist.o $(LIBDIR)/libmbio.a $(LFLAGS) -o $(BINDIR)/mblist
$(BINDIR)/mbmerge:	mbmerge.o $(LIBDIR)/libmbio.a
	$(CC) $(CFLAGS) mbmerge.o $(LIBDIR)/libmbio.a $(LFLAGS) -o $(BINDIR)/mbmerge
$(BINDIR)/mbclean:	mbclean.o $(LIBDIR)/libmbio.a
	$(CC) $(CFLAGS) mbclean.o $(LIBDIR)/libmbio.a $(LFLAGS) -o $(BINDIR)/mbclean
$(BINDIR)/mbunclean:	mbunclean.o $(LIBDIR)/libmbio.a
	$(CC) $(CFLAGS) mbunclean.o $(LIBDIR)/libmbio.a $(LFLAGS) -o $(BINDIR)/mbunclean
$(BINDIR)/mbgrid:	mbgrid.o zgrid.o $(LIBDIR)/libmbio.a
	$(CC) $(CFLAGS) mbgrid.o zgrid.o $(LIBDIR)/libmbio.a $(GMTBINDIR)/gmtlib.a -lnetcdf $(FWITHCFLAGS) $(LFLAGS) -o $(BINDIR)/mbgrid
$(BINDIR)/mblevitus:	mblevitus.o
	$(CC) $(CFLAGS) mblevitus.o $(LFLAGS) -o $(BINDIR)/mblevitus
$(BINDIR)/hsbath:	hsbath.o $(LIBDIR)/libmbio.a
	$(CC) $(CFLAGS) hsbath.o $(LIBDIR)/libmbio.a $(LFLAGS) -o $(BINDIR)/hsbath
$(BINDIR)/hsveldump:	hsveldump.o $(LIBDIR)/libmbio.a
	$(CC) $(CFLAGS) hsveldump.o $(LIBDIR)/libmbio.a $(LFLAGS) -o $(BINDIR)/hsveldump
$(BINDIR)/mbrollbias:	mbrollbias.o $(LIBDIR)/libmbio.a
	$(CC) $(CFLAGS) mbrollbias.o $(LIBDIR)/libmbio.a $(LFLAGS) -o $(BINDIR)/mbrollbias
$(BINDIR)/mbsynth:	mbsynth.o $(LIBDIR)/libmbio.a
	$(CC) $(CFLAGS) mbsynth.o $(LIBDIR)/libmbio.a $(LFLAGS) -o $(BINDIR)/mbsynth

### compile the source files
mbformat.o:	mbformat.c  $(INCDIR)/mb_format.h $(INCDIR)/mb_status.h
	$(CC) -c $(CFLAGS) mbformat.c
mbdefaults.o:	mbdefaults.c  $(INCDIR)/mb_format.h $(INCDIR)/mb_status.h
	$(CC) -c $(CFLAGS) mbdefaults.c
mbinfo.o:	mbinfo.c  $(INCDIR)/mb_format.h $(INCDIR)/mb_status.h
	$(CC) -c $(CFLAGS) mbinfo.c
mbcopy.o:	mbcopy.c  $(INCDIR)/mb_format.h $(INCDIR)/mb_status.h
	$(CC) -c $(CFLAGS) mbcopy.c
mblist.o:	mblist.c  $(INCDIR)/mb_format.h $(INCDIR)/mb_status.h
	$(CC) -c $(CFLAGS) mblist.c
mbmerge.o:	mbmerge.c  $(INCDIR)/mb_format.h $(INCDIR)/mb_status.h
	$(CC) -c $(CFLAGS) mbmerge.c
mbclean.o:	mbclean.c  $(INCDIR)/mb_format.h $(INCDIR)/mb_status.h
	$(CC) -c $(CFLAGS) mbclean.c
mbunclean.o:	mbunclean.c  $(INCDIR)/mb_format.h $(INCDIR)/mb_status.h
	$(CC) -c $(CFLAGS) mbunclean.c
mbgrid.o:	mbgrid.c  $(INCDIR)/mb_format.h $(INCDIR)/mb_status.h
	$(CC) -c $(CFLAGS) -I$(GMTINCDIR) mbgrid.c
mblevitus.o:	mblevitus.c $(INCDIR)/mb_status.h
	$(CC) -c $(CFLAGS) mblevitus.c
zgrid.o:	zgrid.f
	$(FC) -c $(FFLAGS) zgrid.f
hsbath.o:	hsbath.c  $(INCDIR)/mb_format.h $(INCDIR)/mb_status.h $(INCDIR)/mbsys_hsds.h
	$(CC) -c $(CFLAGS) hsbath.c
hsveldump.o:	hsveldump.c  $(INCDIR)/mb_format.h $(INCDIR)/mb_status.h $(INCDIR)/mbsys_hsds.h
	$(CC) -c $(CFLAGS) hsveldump.c
mbrollbias.o:	mbrollbias.c  $(INCDIR)/mb_format.h $(INCDIR)/mb_status.h
	$(CC) -c $(CFLAGS) mbrollbias.c
mbsynth.o:	mbsynth.c  $(INCDIR)/mb_format.h $(INCDIR)/mb_status.h
	$(CC) -c $(CFLAGS) mbsynth.c

### copy the man pages to the man page directory
$(MANDIR)/mbsystem.1:	mbsystem.1
	cp mbsystem.1 $(MANDIR)/mbsystem.1
	chmod 664 $(MANDIR)/mbsystem.1
$(MANDIR)/mbformat.1:	mbformat.1
	cp mbformat.1 $(MANDIR)/mbformat.1
	chmod 664 $(MANDIR)/mbformat.1
$(MANDIR)/mbdefaults.1:	mbdefaults.1
	cp mbdefaults.1 $(MANDIR)/mbdefaults.1
	chmod 664 $(MANDIR)/mbdefaults.1
$(MANDIR)/mbinfo.1:	mbinfo.1
	cp mbinfo.1 $(MANDIR)/mbinfo.1
	chmod 664 $(MANDIR)/mbinfo.1
$(MANDIR)/mbcopy.1:	mbcopy.1
	cp mbcopy.1 $(MANDIR)/mbcopy.1
	chmod 664 $(MANDIR)/mbcopy.1
$(MANDIR)/mblist.1:	mblist.1
	cp mblist.1 $(MANDIR)/mblist.1
	chmod 664 $(MANDIR)/mblist.1
$(MANDIR)/mbmerge.1:	mbmerge.1
	cp mbmerge.1 $(MANDIR)/mbmerge.1
	chmod 664 $(MANDIR)/mbmerge.1
$(MANDIR)/mbclean.1:	mbclean.1
	cp mbclean.1 $(MANDIR)/mbclean.1
	chmod 664 $(MANDIR)/mbclean.1
$(MANDIR)/mbunclean.1:	mbunclean.1
	cp mbunclean.1 $(MANDIR)/mbunclean.1
	chmod 664 $(MANDIR)/mbunclean.1
$(MANDIR)/mbgrid.1:	mbgrid.1
	cp mbgrid.1 $(MANDIR)/mbgrid.1
	chmod 664 $(MANDIR)/mbgrid.1
$(MANDIR)/mblevitus.1:	mblevitus.1
	cp mblevitus.1 $(MANDIR)/mblevitus.1
	chmod 664 $(MANDIR)/mblevitus.1
$(MANDIR)/hsbath.1:	hsbath.1
	cp hsbath.1 $(MANDIR)/hsbath.1
	chmod 664 $(MANDIR)/hsbath.1
$(MANDIR)/hsveldump.1:	hsveldump.1
	cp hsveldump.1 $(MANDIR)/hsveldump.1
	chmod 664 $(MANDIR)/hsveldump.1
$(MANDIR)/mbrollbias.1:	mbrollbias.1
	cp mbrollbias.1 $(MANDIR)/mbrollbias.1
	chmod 664 $(MANDIR)/mbrollbias.1

