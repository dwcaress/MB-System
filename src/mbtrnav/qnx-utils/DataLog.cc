/****************************************************************************/
/* Copyright (c) 2000 MBARI                                                 */
/* MBARI Proprietary Information. All rights reserved.                      */
/****************************************************************************/
/* Summary  :                                                               */
/* Filename : DataLog.cc                                                    */
/* Author   :                                                               */
/* Project  :                                                               */
/* Version  : 1.0                                                           */
/* Created  : 02/07/2000                                                    */
/* Modified :                                                               */
/* Archived :                                                               */
/****************************************************************************/
/* Modification History:                                                    */
/****************************************************************************/
#include <stdlib.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/stat.h>

#if !defined(__APPLE__)
#include <malloc.h>
#endif

#include <string.h>
#include "DataLog.h"
#include "ourTypes.h"
#include "Exception.h"
#include "BinaryFile.h"

// Function attempts to make a unique name for a directory to be created
// inside the directory specified in homeDir. The new name uses the format:
//     YYYY.JJJ.NNN  (e.g. 2021.001.000 <- first name for Jan 1, 2021)
// where YYYY is the 4-digit year, JJJ is the Julian day of the year plus one,
// and NNN is an integer appended to ensure the name is unique.
// The directory is NOT created by this function.
//
// The new directory name is returned in dirname.
// Returns true if successful and the directory can be created, otherwise false.
bool DataLog::newJulianDayLogDirName(std::string& dirName,
                                     const std::string& homeDir)
{
  // Ensure that homeDir exists
  struct stat sb={0};
  if (stat(homeDir.c_str(), &sb) != 0 || !S_ISDIR(sb.st_mode)) {
    return false;
  }

  // Get the current date
  time_t now = time(NULL);
  struct tm st={0};
  localtime_r(&now, &st);

  // Make a place for the name
  const uint32_t max_dirs = 100000;
  uint32_t max_dirs_digits = (unsigned int)floor(log10(max_dirs)) + 1;
  char name[homeDir.length() + strlen("/YYYY.JJJ.") + max_dirs_digits + 1];

  // Test dirnames until one pops
  uint32_t dirs = 0;
  for (dirs = 0; dirs < max_dirs; dirs++) {
    snprintf(name,homeDir.length(), "%s/%4d.%03d.%03d", homeDir.c_str(),
            st.tm_year+1900, st.tm_yday+1, dirs);
    // We're done if the pathname does not exist
    if (stat(name, &sb) != 0) {
      break;
    }
  }

  // Success?
  if (dirs >= max_dirs) {
    return false;
  } else {
    dirName = name;
    return true;
  }
}

// Function attempts to create a new directory for log files inside the
// existing directory specified in homeDir. The new directory name is created
// using newJulianDayLogDirName() function above. If homeDir does not exist,
// the current directory will be used (i.e., ".").
// Assuming the new directory is created in homeDir, this function will also
// create a "latest" symbolic link to the new directory if one is provided
// in the latest argument. To avoid creating the link, use and empty string.
// The new directory name is returned in dirname.
// Returns true if successful, otherwise false.
bool DataLog::createJulianDayLogDir(std::string& dirname,
                                    const std::string& homeDir,
                                    const std::string& latest)
{
  // Check if homeDir exists. If not default to current directory.
  std::string usedir = homeDir;
  if (!DataLog::newJulianDayLogDirName(dirname, usedir)) {
    std::string usedir = ".";
    if (!DataLog::newJulianDayLogDirName(dirname, usedir)) {
      return false;
    }
  }

  // Create the directory
  if (0 != mkdir(dirname.c_str(), 0755)) {
    return false;
  }

  // Create symbolic link if the length of link name is non-zero
  if (latest.length() > 0) {
    std::string symname = usedir + "/" + latest;
    // first remove the current link if it exists
    struct stat sb={0};
    if (0 == lstat(symname.c_str(), &sb) && S_ISLNK(sb.st_mode)) {
      if (0 != remove(symname.c_str())) {
        return false;
      }
    }
    // create the link
    if (0 != symlink(dirname.c_str(), symname.c_str())) {
      return false;
    }
  }

  // all good
  return true;
}

// Member functions
DataLog::DataLog(const char *name, DataLog::Access access,
		 DataLog::FileFormat fileFormat)
{
  if (name == NULL || strlen(name) < 1)
  {
    printf("DataLog::DataLog() - No log name supplied. Using \"logfile\"\n");
    _name = strdup(NoName);
  }
  else
  {
    _name = strdup(name);
  }

  _mnemonic = strdup(_name);

  _access = access;
  _fileFormat = fileFormat;
  _handledHeader = False;
  _logFile = 0;
  strcpy(_fileName, "");

  /* *************
  char *auvLogDir = getenv(AuvLogDirName);

  if (auvLogDir == 0) {

    printf("DataLog::DataLog() - environment variable %s not set\n",
		  AuvLogDirName);
    exit(1);
  }

  snprintf(_fileName, DLOG_FILENAME_BYTES, "%s/%s.log", auvLogDir, _name);

  openFile();
  ************* */

}


DataLog::~DataLog()
{
  //  delete _logFile;

  if (_logFileStream)
    fclose(_logFileStream);

  free((void *)_name);
  free((void *)_mnemonic);
}


void DataLog::openFile()
{
  char accessString[10];

  switch (access()) {

  case Read:
    strcpy(accessString, "r");
    break;

  case Write:
    strcpy(accessString, "w");
    break;
  }

  FILE *logfile;
  char origName[100];
  snprintf(origName, 100, "%s", "");
  strcpy(origName, _fileName);

  // only do this check if access mode is write
  // otherwise we "break" logToMatlab.
  if (access() == Write) {
    int numtries = 1;

    while ((logfile = fopen(_fileName, "r")) != NULL) {
      //printf("logfile already exists!\n");
      //printf("I will try to append a %d to your file\n", numtries);
      snprintf(_fileName, DLOG_FILENAME_BYTES, "%s.%d",origName,numtries);
      fclose(logfile);
      numtries++;
      // exit(1);
    }
  }

  if ((_logFileStream = fopen(_fileName, accessString)) == 0) {

    printf("DataLog::DataLog() - couldn't open log file %s\n",
		  _fileName);
    exit(1);
  }
}



void DataLog::setName(const char *name)
{
  free((void *)_name);
  _name = strdup(name);
}


void DataLog::setMnemonic(const char *mnemonic)
{
  free((void *)_mnemonic);
  _mnemonic = strdup(mnemonic);
}



TimeTag *DataLog::timeTag()
{
  DataField *field;

  for (int i = 0; i < fields.size(); i++) {
    fields.get(i, &field);
    if (!strcmp(field->name(), TimeTagFieldName))
      return (TimeTag *)field;
  }

  // Field not found!
  return 0;
}
