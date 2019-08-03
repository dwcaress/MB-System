# Format MBF_GSFGENMB (format 121, Generic Sensor Format, GSF)

## Introduction

The libgsf library code is released by Leidos under a LGPL2.1 license, and is
included directly into the MB-System source code with minor changes. The changes
required are to avoid redefinition of several key system functions (e.g. fseek);
these redefinitions are avoided using a preprocessor command that is only
activated in the MB-System build system. The command `mbformat -F121` outputs
the version of the GSF library embedded in MB-System as a part of the format
description.

## Updating

When importing a new version from upstream (Leidos), run the following commands
before committing the update:

```bash
dos2unix *.[ch]
perl -pi -e 's/\s+\n/\n/g' *.[ch]
chmod 640 *.[ch] *.pdf
```
