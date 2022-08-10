# qplot  

qplot is a config-file driven bash script for generating gnuplot scripts from CSV data.   
Plots may be generated in a number of formats, and combined into a single PDF file.  

The original application for qplot is automating generation of "quick-look" web content to review multiple data streams in real time. qplot enables fine-grained control of plot styling and output, including placing multiple plots on the same graph. 

## Required Packages
* bash
* gnuplot
* ghostscript
* libpng
* libgd or gd2
* macports or homebrew [1]
* cygwin [2]

[1] OS X  
[2] Windows  

## Quick Start
* clone qplot repository  
`cd qplot/example`
* Run the example  
`../bin/qplot -f qp-example.conf`  

This will generate a .PNG file called qp_example.png in the qplot example directory




