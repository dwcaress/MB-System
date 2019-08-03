# Format MBF_GSFGENMB (format 121, Generic Sensor Format, GSF)

## Introduction

The libgsf library code is released by Leidos under a LGPL2.1 license, and is
included directly into the MB-System source code with minor changes. 

## Changes required for integration with MB-System

Three changes are required in gsf.c and gsf_indx.c to avoid redefinition of several 
key system functions (e.g. fseek); these redefinitions are avoided using a preprocessor 
command that is only activated in the MB-System build system. The other change is
to set MAX_GSF_SF_MULT_VALUE in gsf_ft.h from ULONG_MAX to a hard-wired value of
4294967295. The problem is that this value is used for limiting scaling and
needs to be the same on every system because four byte insigned integers are 
always used for the scaled storage. The value of ULONG_MAX varies amongst operating 
system implementations.

Starting with release 5.7.6beta4, command `mbformat -F121` outputs the version of the 
GSF library embedded in MB-System as a part of the format description.

## Updating the source files to be consistent with the rest of the repository

The source files in the Leidos distribution tarballs are in dos text form and have
odd permissions. When importing a new version from upstream (Leidos), run the following 
commands before committing the update:

```bash
dos2unix *.[ch]
perl -pi -e 's/\s+\n/\n/g' *.[ch]
chmod 640 *.[ch] *.pdf
```
