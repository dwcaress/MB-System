#include <cxxtest/TestSuite.h>

#include "SwathReader.h"

#ifndef _SWATHREADER_TEST_H
#define _SWATHREADER_TEST_H

// These first three lines address
// issue described at
// https://stackoverflow.com/questions/18642155/no-override-found-for-vtkpolydatamapper
#include "vtkAutoInit.h"
VTK_MODULE_INIT(vtkRenderingOpenGL2); // VTK was built with vtkRenderingOpenGL2
VTK_MODULE_INIT(vtkInteractionStyle);
VTK_MODULE_INIT(vtkRenderingFreeType)


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

    // In case constructor is public...
    // mb_system::SwathReader *reader = new mb_system::SwathReader();
    
    const char *datafile =
      "/home/oreilly/projects/mb-system/testData/test.mb88";

    cerr << "read " << datafile << endl;
    bool success = reader->readSwathFile(datafile);
    TSM_ASSERT("reader->read()", success);
    
    cerr << "return from testReadSwath()" << endl;
  }
  
};


#endif
