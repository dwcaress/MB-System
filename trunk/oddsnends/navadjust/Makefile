MKFLAGS = -f $(MAKEFILE) MAKEFILE=$(MAKEFILE) ARCH=$(ARCH)

BINDIR = ../../bin/$(ARCH)
LIBDIR = ../../lib/$(ARCH)

all: $(BINDIR)/navcross $(BINDIR)/naverr $(BINDIR)/navsolve $(BINDIR)/navadjust

$(BINDIR)/navcross:	navcross.o ../../lib/$(ARCH)/mbio.a
	$(CC) navcross.o ../../lib/$(ARCH)/mbio.a -lm  -o $(BINDIR)/navcross

$(BINDIR)/naverr:	naverr.o zgrid.o $(LIBDIR)/mb_contour.a $(LIBDIR)/mbio.a $(LIBDIR)/label.a
	$(CC) naverr.o zgrid.o $(LIBDIR)/mb_contour.a $(LIBDIR)/mbio.a $(LIBDIR)/label.a -lF77 -lsuntool -lsunwindow -lpixrect -lc -lm  -o $(BINDIR)/naverr

$(BINDIR)/navsolve:	navsolve.o $(LIBDIR)/mbio.a cheby.o
	$(CC) navsolve.o $(LIBDIR)/mbio.a cheby.o -lF77 -lc -lm  -o $(BINDIR)/navsolve

$(BINDIR)/navadjust:	navadjust.o $(LIBDIR)/mbio.a
	$(CC) navadjust.o $(LIBDIR)/mbio.a -lc -lm  -o $(BINDIR)/navadjust

navcross.o:	navcross.c
	$(CC) -c navcross.c

naverr.o:	naverr.c
	$(CC) -c naverr.c

navsolve.o:	navsolve.c
	$(CC) -c navsolve.c

navadjust.o:	navadjust.c
	$(CC) -c navadjust.c

cheby.o:	cheby.f
	$(FC) -c cheby.f

zgrid.o:	zgrid.f
	$(FC) -c zgrid.f

$(LIBDIR)/mb_contour.a:
	cd ../contour; make $(MKFLAGS) all

$(LIBDIR)/mbio.a:
	cd ../mbio
	make $(MKFLAGS) all

clean:
	rm -f *.o


