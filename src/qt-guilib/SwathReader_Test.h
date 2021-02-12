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

    const char *datafile =
      "/home/oreilly/projects/mb-system/testData/test.mb88";

    cerr << "read " << datafile << endl;
    bool success = reader->readSwathFile(datafile);
    TSM_ASSERT("reader->read()", success);
    
    cerr << "return from testReadSwath()" << endl;
  }
  
};


#endif
