#     The MB-system:	Makefile  1/21/93
#	$Id: Makefile,v 4.1 1994-04-12 17:03:51 caress Exp $
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
# Revision 4.0  1994/03/06  00:13:22  caress
# First cut at version 4.0
#
# Revision 4.0  1994/03/01  18:53:57  caress
# First cut at new version.
#
# Revision 1.11  1993/11/08  03:58:16  caress
# Reset location of Levitus annual file to /ocean/mgg/multibeam/levitus.
#
# Revision 1.10  1993/11/05  18:10:34  caress
# Added mbsmooth.1 man page.
#
# Revision 1.9  1993/11/05  16:02:14  caress
# Set up mblevitus so that location of annual database file
# is found from an include file generated each time make is
# called.  Thus, the database location can be set in the
# Makefile rather than hardwired in the code.
#
# Revision 1.8  1993/11/04  19:35:01  caress
# Added mbps.
#
# Revision 1.1  1993/08/17  02:32:19  caress
# Initial revision
#
# Revision 1.7  1993/08/16  22:21:44  caress
# Don't know changes exactly, but probably involved setting
# L-DEO locations of gmt stuff.
#
# Revision 1.6  1993/06/16  23:09:17  caress
# Added hsdump and removed hsveldump.
#
# Revision 1.5  1993/05/25  20:13:53  caress
# Fixed chmod commands again.
#
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

# *** CHANGE THESE TO FIT YOUR LOCAL ENVIRONMENT ***
#     - locations of GMT libraries and include files
GMTLIBDIR = /ocean/mgg/gmt2/sun4/lib
GMTINCDIR = /ocean/mgg/gmt2/src
#     - location of Levitus annual database file
LEVITUS = /ocean/mgg/multibeam/levitus/annual
# *** LEAVE EVERYTHING ELSE BELOW ALONE ***

BINDIR = ../../bin
LIBDIR = ../../lib
INCDIR = ../../include
MANDIR = ../../man/man1

CFLAGS = -O2
FFLAGS = -O2
FWITHCFLAGS = -lF77
LFLAGS = -lm

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
	$(BINDIR)/mbbath $(MANDIR)/mbbath.1 \
	$(BINDIR)/hsbath $(MANDIR)/hsbath.1 \
	$(BINDIR)/hsdump $(MANDIR)/hsdump.1 \
	$(BINDIR)/mbrollbias $(MANDIR)/mbrollbias.1 \
	$(BINDIR)/mbgetmask $(MANDIR)/mbgetmask.1 \
	$(BINDIR)/mbmask $(MANDIR)/mbmask.1 \
	$(BINDIR)/mbsmooth $(MANDIR)/mbsmooth.1 \
	$(BINDIR)/mbps $(MANDIR)/mbps.1

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
	$(CC) $(CFLAGS) mbgrid.o zgrid.o $(LIBDIR)/libmbio.a $(GMTLIBDIR)/libgmt.a -lnetcdf $(FWITHCFLAGS) $(LFLAGS) -o $(BINDIR)/mbgrid
$(BINDIR)/mblevitus:	mblevitus.o
	$(CC) $(CFLAGS) mblevitus.o $(LFLAGS) -o $(BINDIR)/mblevitus
$(BINDIR)/mbbath:	mbbath.o $(LIBDIR)/libmbio.a
	$(CC) $(CFLAGS) mbbath.o $(LIBDIR)/libmbio.a $(LFLAGS) -o $(BINDIR)/mbbath
$(BINDIR)/hsbath:	$(BINDIR)/mbbath
	cd $(BINDIR); pwd; rm -f hsbath; ln -s mbbath hsbath
$(BINDIR)/hsdump:	hsdump.o $(LIBDIR)/libmbio.a
	$(CC) $(CFLAGS) hsdump.o $(LIBDIR)/libmbio.a $(LFLAGS) -o $(BINDIR)/hsdump
$(BINDIR)/mbrollbias:	mbrollbias.o $(LIBDIR)/libmbio.a
	$(CC) $(CFLAGS) mbrollbias.o $(LIBDIR)/libmbio.a $(LFLAGS) -o $(BINDIR)/mbrollbias
$(BINDIR)/mbgetmask:	mbgetmask.o $(LIBDIR)/libmbio.a
	$(CC) $(CFLAGS) mbgetmask.o $(LIBDIR)/libmbio.a $(LFLAGS) -o $(BINDIR)/mbgetmask
$(BINDIR)/mbmask:	mbmask.o $(LIBDIR)/libmbio.a
	$(CC) $(CFLAGS) mbmask.o $(LIBDIR)/libmbio.a $(LFLAGS) -o $(BINDIR)/mbmask
$(BINDIR)/mbsmooth:	mbsmooth.o $(LIBDIR)/libmbio.a
	$(CC) $(CFLAGS) mbsmooth.o $(LIBDIR)/libmbio.a $(LFLAGS) -o $(BINDIR)/mbsmooth
$(BINDIR)/mbps:	mbps.o $(LIBDIR)/libmbio.a
	$(CC) $(CFLAGS) mbps.o $(LIBDIR)/libmbio.a $(GMTLIBDIR)/libgmt.a $(GMTLIBDIR)/libpsl.a $(LFLAGS) -o $(BINDIR)/mbps

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
	echo "	char *ifile = \"$(LEVITUS)\";" > levitus.h
	$(CC) -c $(CFLAGS) mblevitus.c
zgrid.o:	zgrid.f
	$(FC) -c $(FFLAGS) zgrid.f
mbbath.o:	mbbath.c  $(INCDIR)/mb_format.h $(INCDIR)/mb_status.h $(INCDIR)/mbsys_hsds.h
	$(CC) -c $(CFLAGS) mbbath.c
hsdump.o:	hsdump.c  $(INCDIR)/mb_format.h $(INCDIR)/mb_status.h $(INCDIR)/mbsys_hsds.h
	$(CC) -c $(CFLAGS) hsdump.c
mbrollbias.o:	mbrollbias.c  $(INCDIR)/mb_format.h $(INCDIR)/mb_status.h
	$(CC) -c $(CFLAGS) mbrollbias.c
mbgetmask.o:	mbgetmask.c  $(INCDIR)/mb_format.h $(INCDIR)/mb_status.h
	$(CC) -c $(CFLAGS) mbgetmask.c
mbmask.o:	mbmask.c  $(INCDIR)/mb_format.h $(INCDIR)/mb_status.h
	$(CC) -c $(CFLAGS) mbmask.c
mbsmooth.o:	mbsmooth.c  $(INCDIR)/mb_format.h $(INCDIR)/mb_status.h
	$(CC) -c $(CFLAGS) mbsmooth.c
mbps.o:	mbps.c  $(INCDIR)/mb_format.h $(INCDIR)/mb_status.h
	$(CC) -c $(CFLAGS) -I$(GMTINCDIR) mbps.c

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
$(MANDIR)/mbbath.1:	mbbath.1
	cp mbbath.1 $(MANDIR)/mbbath.1
	chmod 664 $(MANDIR)/mbbath.1
$(MANDIR)/hsbath.1:	$(MANDIR)/mbbath.1
	cd $(MANDIR); pwd; rm -f hsbath.1; ln -s mbbath.1 hsbath.1
$(MANDIR)/hsdump.1:	hsdump.1
	cp hsdump.1 $(MANDIR)/hsdump.1
	chmod 664 $(MANDIR)/hsdump.1
$(MANDIR)/mbrollbias.1:	mbrollbias.1
	cp mbrollbias.1 $(MANDIR)/mbrollbias.1
	chmod 664 $(MANDIR)/mbrollbias.1
$(MANDIR)/mbgetmask.1:	mbmask.1
	cp mbmask.1 $(MANDIR)/mbgetmask.1
	chmod 664 $(MANDIR)/mbgetmask.1
$(MANDIR)/mbmask.1:	mbmask.1
	cp mbmask.1 $(MANDIR)/mbmask.1
	chmod 664 $(MANDIR)/mbmask.1
$(MANDIR)/mbsmooth.1:	mbsmooth.1
	cp mbsmooth.1 $(MANDIR)/mbsmooth.1
	chmod 664 $(MANDIR)/mbsmooth.1
$(MANDIR)/mbps.1:	mbps.1
	cp mbps.1 $(MANDIR)/mbps.1
	chmod 664 $(MANDIR)/mbps.1

