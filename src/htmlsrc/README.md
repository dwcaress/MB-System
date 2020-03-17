### MB-System

Source Directory: **src/htmlsrc/**

This directory contains the source files used to construct the web page documentation included in MB-System distributions and installations. These source files, including html fragments, images, and a perl script named make_mbhtml, generate html pages in src/html/ and pdf versions of those pages in **src/pdf/**. In addition to pages constructed from the files in this directory, the script also uses a compiled C program man2html to convert the Troff format manual pages of individual programs found in **src/man/** to html and Postscript documents in the directories **src/html/** and **src/ps/**, respectively.

These source files are copyright by David Caress and Dale Chayes and licensed using GPL3 as part of MB-System.