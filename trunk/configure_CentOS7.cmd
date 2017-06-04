#------------------------------------------------------------------------------
# CentOs configure script command line example:
#------------------------------------------------------------------------------
#
# The following commands install MB-System on CentOs 6 or 7
#
# Install the prerequisites using yum:
sudo yum install openmotif openmotif-devel fftw fftw-devel netcdf netcdf-devel \
		proj proj-devel gdal-devel gmt gmt-devel gv
# 
# These steps assume you have downloaded an MB-System distribution tar.gz file 
# from the ftp site:
#       ftp://mbsystemftp@ftp.mbari.org
# The current distribution file is mbsystem-5.5.2307.tar.gz
# 
# First, unpack the distribution using tar. This can be done in any location,
# but if one is working in a root-owned area such as /usr/local/src, obtaining
# root privileges using sudo is necessary for all steps:
#       sudo tar xvzf mbsystem-5.5.2307.tar.gz
# and then cd into the resulting directory:
#       cd mbsystem-5.5.2307
#
# If the prerequisites have all been installed with yum and it is desired to
# install MB-System in /usr/local, then only a simple call to configure is required:
sudo ./configure
#
#------------------------------------------------------------------------------
