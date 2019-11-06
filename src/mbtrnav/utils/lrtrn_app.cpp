//*****************************************************************************
// Proposed structure of the LRAUV TRN process.
// View it as pseudo-code as it will likely be folded
// into a C++ class.
//
// Note: This code does successfully build on the target (all libs and object
// files are found).
//*****************************************************************************


#include <stdio.h>
#include <lcm/lcm-cpp.hpp>
#include <lcmMessages/DataVectors.hpp>

#include "LcmTrn.h"

using namespace lcmTrn;

int main(int argc, char** argv)
{
  //**************************************************************************
  // Initialization phase
  //**************************************************************************

  char *configfile = NULL;
  if (argc > 1) configfile = argv[1];

  lcmTrn::LcmTrn *_trn = new LcmTrn(configfile);

  _trn->run();

  _trn->reinit("some-other-file.cfg");

  _trn->run();

  return 0;
}
