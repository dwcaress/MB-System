#include <cxxtest/TestSuite.h>

#include "SwathReader.h"

#ifndef _SWATHREADER_TEST_H
#define _SWATHREADER_TEST_H

using namespace std;

/**
cxxtest Unit tests for SwathReader class
*/
class SwathReader_Test : public CxxTest::TestSuite {

public:
  void testReadSwath(void) {
    cerr << "create reader" << endl;
    
    mb_system::SwathReader *reader =
      vtkSmartPointer<mb_system::SwathReader>::New();

    cerr << "return from testReadSwath()" << endl;
  }
  
};


#endif
