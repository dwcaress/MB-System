//*****************************************************************************
// Proposed structure of the LRAUV TRN process.
// View it as pseudo-code as it will likely be folded
// into a C++ class.
//
// Note: This code does successfully build on the target (all libs and object
// files are found).
//*****************************************************************************


#include <stdio.h>
#include "LcmTrn.h"

using namespace lcmTrn;

// Return a default configuration file pathname using the environment variable, etc.
//
char* getDefaultConfig()
{
  // Use the environment variable for the config directory. If the
  // variable is not defined, use the local directory.
  //
  char *cfg = LcmTrn::constructFullName(LCMTRN_CONFIG_ENV, LCMTRN_DEFAULT_CONFIG);
  return cfg;
}

void usage()
{
  fprintf(stderr, "Usage:\n");
  fprintf(stderr, "  lrtrn_app [cfgfile]\n\n");
  fprintf(stderr, "  cfgfile: optional path to lrtrn configuration file\n");
  fprintf(stderr, "           default is $%s/%s\n", LCMTRN_CONFIG_ENV, LCMTRN_DEFAULT_CONFIG);
  fprintf(stderr, "           local directory used if $%s is undefined\n\n", LCMTRN_CONFIG_ENV);
}

int main(int argc, char** argv)
{
  //**************************************************************************
  // Initialization phase
  //**************************************************************************

  // Use default configuration file if none was specified on the command line.
  //
  char *configfile = NULL;
  if (argc == 1)
    configfile = getDefaultConfig();
  else if (argc == 2)
    configfile = argv[1];
  else
    usage();

  // Quit if no config
  //
  if (NULL == configfile) exit(0);

  // Create LcmTrn object and run it
  //
  lcmTrn::LcmTrn _trn(configfile);

  // Run if the LcmTrn object setup was a success.
  // run() only stops when the LcmTrn object is not good.
  //
  if (_trn.good())
  {
    fprintf(stderr, "%s listening for messages...\n", argv[0]);
    _trn.run();
  }

  fprintf(stderr, "%s Done\n", argv[0]);

  // Clean
  //
  DELOBJ(configfile);

  return 0;
}
