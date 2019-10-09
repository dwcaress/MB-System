CXX = g++
CXXFLAGS = -O2 -Wall

DIFF = ./sdiff
PRE = ./
MAJOR = 1
MINOR = 0

%.o:           	%.cpp
		$(CXX) $(CXXFLAGS) -c $*.cpp

everything:    	tmt example test_exc nl_ex sl_ex garch 

newmat_lobj = newmat1.o newmat2.o newmat3.o newmat4.o newmat5.o newmat6.o newmat7.o newmat8.o newmatex.o bandmat.o submat.o myexcept.o cholesky.o evalue.o fft.o hholder.o jacobi.o newfft.o sort.o svd.o newmatrm.o newmat9.o

libnewmat.a:   	$(newmat_lobj)
		$(AR) -cr $@ $(newmat_lobj)
		ranlib $@

tmt_obj = tmt.o tmt1.o tmt2.o tmt3.o tmt4.o tmt5.o tmt6.o tmt7.o tmt8.o tmt9.o tmta.o tmtb.o tmtc.o tmtd.o tmte.o tmtf.o tmtg.o tmth.o tmti.o tmtj.o tmtk.o tmtl.o tmtm.o

tmt:           	$(tmt_obj) libnewmat.a
		$(CXX) -o $@ $(tmt_obj) -L. -lnewmat -lm

example_obj = example.o

example:       	$(example_obj) libnewmat.a
		$(CXX) -o $@ $(example_obj) -L. -lnewmat -lm

test_exc_obj = test_exc.o

test_exc:      	$(test_exc_obj) libnewmat.a
		$(CXX) -o $@ $(test_exc_obj) -L. -lnewmat -lm

nl_ex_obj = nl_ex.o newmatnl.o

nl_ex:         	$(nl_ex_obj) libnewmat.a
		$(CXX) -o $@ $(nl_ex_obj) -L. -lnewmat -lm

sl_ex_obj = sl_ex.o solution.o myexcept.o

sl_ex:         	$(sl_ex_obj)
		$(CXX) -o $@ $(sl_ex_obj) -L. -lm

garch_obj = garch.o newmatnl.o

garch:         	$(garch_obj) libnewmat.a
		$(CXX) -o $@ $(garch_obj) -L. -lnewmat -lm

newmat1.o:     	newmat1.cpp newmat.h include.h boolean.h myexcept.h

newmat2.o:     	newmat2.cpp include.h newmat.h newmatrc.h boolean.h myexcept.h controlw.h

newmat3.o:     	newmat3.cpp include.h newmat.h newmatrc.h boolean.h myexcept.h controlw.h

newmat4.o:     	newmat4.cpp include.h newmat.h newmatrc.h boolean.h myexcept.h controlw.h

newmat5.o:     	newmat5.cpp include.h newmat.h newmatrc.h boolean.h myexcept.h controlw.h

newmat6.o:     	newmat6.cpp include.h newmat.h newmatrc.h boolean.h myexcept.h controlw.h

newmat7.o:     	newmat7.cpp include.h newmat.h newmatrc.h boolean.h myexcept.h controlw.h

newmat8.o:     	newmat8.cpp include.h newmat.h newmatrc.h precisio.h boolean.h myexcept.h controlw.h

newmatex.o:    	newmatex.cpp include.h newmat.h boolean.h myexcept.h

bandmat.o:     	bandmat.cpp include.h newmat.h newmatrc.h boolean.h myexcept.h controlw.h

submat.o:      	submat.cpp include.h newmat.h newmatrc.h boolean.h myexcept.h controlw.h

myexcept.o:    	myexcept.cpp include.h myexcept.h

cholesky.o:    	cholesky.cpp include.h newmat.h boolean.h myexcept.h

evalue.o:      	evalue.cpp include.h newmatap.h newmatrm.h precisio.h newmat.h boolean.h myexcept.h

fft.o:         	fft.cpp include.h newmatap.h newmat.h boolean.h myexcept.h

hholder.o:     	hholder.cpp include.h newmatap.h newmat.h boolean.h myexcept.h

jacobi.o:      	jacobi.cpp include.h newmatap.h precisio.h newmatrm.h newmat.h boolean.h myexcept.h

newfft.o:      	newfft.cpp newmatap.h newmat.h include.h boolean.h myexcept.h

sort.o:        	sort.cpp include.h newmatap.h newmat.h boolean.h myexcept.h

svd.o:         	svd.cpp include.h newmatap.h newmatrm.h precisio.h newmat.h boolean.h myexcept.h

newmatrm.o:    	newmatrm.cpp newmat.h newmatrm.h include.h boolean.h myexcept.h

newmat9.o:     	newmat9.cpp include.h newmat.h newmatio.h newmatrc.h boolean.h myexcept.h controlw.h

tmt.o:         	tmt.cpp include.h newmat.h tmt.h boolean.h myexcept.h

tmt1.o:        	tmt1.cpp include.h newmat.h tmt.h boolean.h myexcept.h

tmt2.o:        	tmt2.cpp include.h newmat.h tmt.h boolean.h myexcept.h

tmt3.o:        	tmt3.cpp include.h newmat.h tmt.h boolean.h myexcept.h

tmt4.o:        	tmt4.cpp include.h newmat.h tmt.h boolean.h myexcept.h

tmt5.o:        	tmt5.cpp include.h newmat.h tmt.h boolean.h myexcept.h

tmt6.o:        	tmt6.cpp include.h newmatap.h tmt.h newmat.h boolean.h myexcept.h

tmt7.o:        	tmt7.cpp include.h newmat.h tmt.h boolean.h myexcept.h

tmt8.o:        	tmt8.cpp include.h newmatap.h tmt.h newmat.h boolean.h myexcept.h

tmt9.o:        	tmt9.cpp include.h newmatap.h tmt.h newmat.h boolean.h myexcept.h

tmta.o:        	tmta.cpp include.h newmatap.h tmt.h newmat.h boolean.h myexcept.h

tmtb.o:        	tmtb.cpp include.h newmat.h tmt.h boolean.h myexcept.h

tmtc.o:        	tmtc.cpp include.h newmat.h tmt.h boolean.h myexcept.h

tmtd.o:        	tmtd.cpp include.h newmatap.h tmt.h newmat.h boolean.h myexcept.h

tmte.o:        	tmte.cpp include.h newmatap.h tmt.h newmat.h boolean.h myexcept.h

tmtf.o:        	tmtf.cpp include.h newmatap.h tmt.h newmat.h boolean.h myexcept.h

tmtg.o:        	tmtg.cpp include.h newmatap.h tmt.h newmat.h boolean.h myexcept.h

tmth.o:        	tmth.cpp include.h newmatap.h tmt.h newmat.h boolean.h myexcept.h

tmti.o:        	tmti.cpp include.h newmatap.h tmt.h newmat.h boolean.h myexcept.h

tmtj.o:        	tmtj.cpp include.h newmatap.h tmt.h newmat.h boolean.h myexcept.h

tmtk.o:        	tmtk.cpp include.h newmatap.h newmatio.h tmt.h newmat.h boolean.h myexcept.h

tmtl.o:        	tmtl.cpp newmat.h tmt.h include.h boolean.h myexcept.h

tmtm.o:        	tmtm.cpp newmat.h tmt.h include.h boolean.h myexcept.h

example.o:     	example.cpp newmatap.h newmatio.h newmat.h include.h boolean.h myexcept.h

test_exc.o:    	test_exc.cpp newmatap.h newmatio.h newmat.h include.h boolean.h myexcept.h

nl_ex.o:       	nl_ex.cpp newmatnl.h newmatio.h newmat.h include.h boolean.h myexcept.h

newmatnl.o:    	newmatnl.cpp newmatap.h newmatnl.h newmat.h include.h boolean.h myexcept.h

sl_ex.o:       	sl_ex.cpp include.h solution.h boolean.h myexcept.h

solution.o:    	solution.cpp include.h boolean.h myexcept.h solution.h

garch.o:       	garch.cpp newmatap.h newmatio.h newmatnl.h newmat.h include.h boolean.h myexcept.h

tmt.txx:       	tmt
		$(PRE)tmt > tmt.txx
		$(DIFF) tmt.txt tmt.txx

example.txx:   	example
		$(PRE)example > example.txx
		$(DIFF) example.txt example.txx

test_exc.txx:  	test_exc
		$(PRE)test_exc > test_exc.txx
		$(DIFF) test_exc.txt test_exc.txx

nl_ex.txx:     	nl_ex
		$(PRE)nl_ex > nl_ex.txx
		$(DIFF) nl_ex.txt nl_ex.txx

sl_ex.txx:     	sl_ex
		$(PRE)sl_ex > sl_ex.txx
		$(DIFF) sl_ex.txt sl_ex.txx

garch.txx:     	garch
		$(PRE)garch > garch.txx
		$(DIFF) garch.txt garch.txx

