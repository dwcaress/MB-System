BORLANDPATH = "C:\program files\Borland\cbuilder5"

TASM = TASM32
TLIB = tlib
TLINK = ilink32
LIBPATH = $(BORLANDPATH)\LIB
INCLUDEPATH = $(BORLANDPATH)\INCLUDE

DIFF = sdiff
PRE =

CC = bcc32 -W- -v- -H- -3 -N -Og -Oi -Ov -f -I$(INCLUDEPATH)

.cpp.obj:
   $(CC) -c {$< }

everything:    	tmt.exe example.exe test_exc.exe nl_ex.exe sl_ex.exe garch.exe 

newmat_lobj = newmat1.obj newmat2.obj newmat3.obj newmat4.obj newmat5.obj newmat6.obj newmat7.obj newmat8.obj newmatex.obj bandmat.obj submat.obj myexcept.obj cholesky.obj evalue.obj fft.obj hholder.obj jacobi.obj newfft.obj sort.obj svd.obj newmatrm.obj newmat9.obj

newmat.lib:    	$(newmat_lobj)
   $(TLIB) $@ /P32 /u $(newmat_lobj)

tmt_obj = tmt.obj tmt1.obj tmt2.obj tmt3.obj tmt4.obj tmt5.obj tmt6.obj tmt7.obj tmt8.obj tmt9.obj tmta.obj tmtb.obj tmtc.obj tmtd.obj tmte.obj tmtf.obj tmtg.obj tmth.obj tmti.obj tmtj.obj tmtk.obj tmtl.obj tmtm.obj

tmt.exe:       	$(tmt_obj) newmat.lib
   $(TLINK) /x/L$(LIBPATH)/Gn -Tpe -ap -c @&&|
c0x32.obj $(tmt_obj),$@,, newmat.lib import32.lib cw32.lib
|

example_obj = example.obj

example.exe:   	$(example_obj) newmat.lib
   $(TLINK) /x/L$(LIBPATH)/Gn -Tpe -ap -c @&&|
c0x32.obj $(example_obj),$@,, newmat.lib import32.lib cw32.lib
|

test_exc_obj = test_exc.obj

test_exc.exe:  	$(test_exc_obj) newmat.lib
   $(TLINK) /x/L$(LIBPATH)/Gn -Tpe -ap -c @&&|
c0x32.obj $(test_exc_obj),$@,, newmat.lib import32.lib cw32.lib
|

nl_ex_obj = nl_ex.obj newmatnl.obj

nl_ex.exe:     	$(nl_ex_obj) newmat.lib
   $(TLINK) /x/L$(LIBPATH)/Gn -Tpe -ap -c @&&|
c0x32.obj $(nl_ex_obj),$@,, newmat.lib import32.lib cw32.lib
|

sl_ex_obj = sl_ex.obj solution.obj myexcept.obj

sl_ex.exe:     	$(sl_ex_obj)
   $(TLINK) /x/L$(LIBPATH)/Gn -Tpe -ap -c @&&|
c0x32.obj $(sl_ex_obj),$@,,import32.lib cw32.lib
|

garch_obj = garch.obj newmatnl.obj

garch.exe:     	$(garch_obj) newmat.lib
   $(TLINK) /x/L$(LIBPATH)/Gn -Tpe -ap -c @&&|
c0x32.obj $(garch_obj),$@,, newmat.lib import32.lib cw32.lib
|

newmat1.obj:   	newmat1.cpp newmat.h include.h boolean.h myexcept.h

newmat2.obj:   	newmat2.cpp include.h newmat.h newmatrc.h boolean.h myexcept.h controlw.h

newmat3.obj:   	newmat3.cpp include.h newmat.h newmatrc.h boolean.h myexcept.h controlw.h

newmat4.obj:   	newmat4.cpp include.h newmat.h newmatrc.h boolean.h myexcept.h controlw.h

newmat5.obj:   	newmat5.cpp include.h newmat.h newmatrc.h boolean.h myexcept.h controlw.h

newmat6.obj:   	newmat6.cpp include.h newmat.h newmatrc.h boolean.h myexcept.h controlw.h

newmat7.obj:   	newmat7.cpp include.h newmat.h newmatrc.h boolean.h myexcept.h controlw.h

newmat8.obj:   	newmat8.cpp include.h newmat.h newmatrc.h precisio.h boolean.h myexcept.h controlw.h

newmatex.obj:  	newmatex.cpp include.h newmat.h boolean.h myexcept.h

bandmat.obj:   	bandmat.cpp include.h newmat.h newmatrc.h boolean.h myexcept.h controlw.h

submat.obj:    	submat.cpp include.h newmat.h newmatrc.h boolean.h myexcept.h controlw.h

myexcept.obj:  	myexcept.cpp include.h myexcept.h

cholesky.obj:  	cholesky.cpp include.h newmat.h boolean.h myexcept.h

evalue.obj:    	evalue.cpp include.h newmatap.h newmatrm.h precisio.h newmat.h boolean.h myexcept.h

fft.obj:       	fft.cpp include.h newmatap.h newmat.h boolean.h myexcept.h

hholder.obj:   	hholder.cpp include.h newmatap.h newmat.h boolean.h myexcept.h

jacobi.obj:    	jacobi.cpp include.h newmatap.h precisio.h newmatrm.h newmat.h boolean.h myexcept.h

newfft.obj:    	newfft.cpp newmatap.h newmat.h include.h boolean.h myexcept.h

sort.obj:      	sort.cpp include.h newmatap.h newmat.h boolean.h myexcept.h

svd.obj:       	svd.cpp include.h newmatap.h newmatrm.h precisio.h newmat.h boolean.h myexcept.h

newmatrm.obj:  	newmatrm.cpp newmat.h newmatrm.h include.h boolean.h myexcept.h

newmat9.obj:   	newmat9.cpp include.h newmat.h newmatio.h newmatrc.h boolean.h myexcept.h controlw.h

tmt.obj:       	tmt.cpp include.h newmat.h tmt.h boolean.h myexcept.h

tmt1.obj:      	tmt1.cpp include.h newmat.h tmt.h boolean.h myexcept.h

tmt2.obj:      	tmt2.cpp include.h newmat.h tmt.h boolean.h myexcept.h

tmt3.obj:      	tmt3.cpp include.h newmat.h tmt.h boolean.h myexcept.h

tmt4.obj:      	tmt4.cpp include.h newmat.h tmt.h boolean.h myexcept.h

tmt5.obj:      	tmt5.cpp include.h newmat.h tmt.h boolean.h myexcept.h

tmt6.obj:      	tmt6.cpp include.h newmatap.h tmt.h newmat.h boolean.h myexcept.h

tmt7.obj:      	tmt7.cpp include.h newmat.h tmt.h boolean.h myexcept.h

tmt8.obj:      	tmt8.cpp include.h newmatap.h tmt.h newmat.h boolean.h myexcept.h

tmt9.obj:      	tmt9.cpp include.h newmatap.h tmt.h newmat.h boolean.h myexcept.h

tmta.obj:      	tmta.cpp include.h newmatap.h tmt.h newmat.h boolean.h myexcept.h

tmtb.obj:      	tmtb.cpp include.h newmat.h tmt.h boolean.h myexcept.h

tmtc.obj:      	tmtc.cpp include.h newmat.h tmt.h boolean.h myexcept.h

tmtd.obj:      	tmtd.cpp include.h newmatap.h tmt.h newmat.h boolean.h myexcept.h

tmte.obj:      	tmte.cpp include.h newmatap.h tmt.h newmat.h boolean.h myexcept.h

tmtf.obj:      	tmtf.cpp include.h newmatap.h tmt.h newmat.h boolean.h myexcept.h

tmtg.obj:      	tmtg.cpp include.h newmatap.h tmt.h newmat.h boolean.h myexcept.h

tmth.obj:      	tmth.cpp include.h newmatap.h tmt.h newmat.h boolean.h myexcept.h

tmti.obj:      	tmti.cpp include.h newmatap.h tmt.h newmat.h boolean.h myexcept.h

tmtj.obj:      	tmtj.cpp include.h newmatap.h tmt.h newmat.h boolean.h myexcept.h

tmtk.obj:      	tmtk.cpp include.h newmatap.h newmatio.h tmt.h newmat.h boolean.h myexcept.h

tmtl.obj:      	tmtl.cpp newmat.h tmt.h include.h boolean.h myexcept.h

tmtm.obj:      	tmtm.cpp newmat.h tmt.h include.h boolean.h myexcept.h

example.obj:   	example.cpp newmatap.h newmatio.h newmat.h include.h boolean.h myexcept.h

test_exc.obj:  	test_exc.cpp newmatap.h newmatio.h newmat.h include.h boolean.h myexcept.h

nl_ex.obj:     	nl_ex.cpp newmatnl.h newmatio.h newmat.h include.h boolean.h myexcept.h

newmatnl.obj:  	newmatnl.cpp newmatap.h newmatnl.h newmat.h include.h boolean.h myexcept.h

sl_ex.obj:     	sl_ex.cpp include.h solution.h boolean.h myexcept.h

solution.obj:  	solution.cpp include.h boolean.h myexcept.h solution.h

garch.obj:     	garch.cpp newmatap.h newmatio.h newmatnl.h newmat.h include.h boolean.h myexcept.h

tmt.txx:       	tmt.exe
		$(PRE)tmt > tmt.txx
		$(DIFF) tmt.txt tmt.txx

example.txx:   	example.exe
		$(PRE)example > example.txx
		$(DIFF) example.txt example.txx

test_exc.txx:  	test_exc.exe
		$(PRE)test_exc > test_exc.txx
		$(DIFF) test_exc.txt test_exc.txx

nl_ex.txx:     	nl_ex.exe
		$(PRE)nl_ex > nl_ex.txx
		$(DIFF) nl_ex.txt nl_ex.txx

sl_ex.txx:     	sl_ex.exe
		$(PRE)sl_ex > sl_ex.txx
		$(DIFF) sl_ex.txt sl_ex.txx

garch.txx:     	garch.exe
		$(PRE)garch > garch.txx
		$(DIFF) garch.txt garch.txx

