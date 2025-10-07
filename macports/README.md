This directory contains Portfiles that install MB-System with macports.
The Portfile to install MB-System is science/mb-system/Portfile.

You must have macports installed locally to use a Portfile.

You can install MBSystem with this local Portfile, even if the Portfile
isn't yet published to the official MacPorts port tree; do the following:

1. Open sources.conf in a text editor with 'sudo'.
For example, to open it into TextEdit:

% sudo open -e /opt/local/etc/macports/sources.conf

2. Insert a URL pointing to your local repository location before the rsync
URL as shown:
```
file:///*MBSystemHome*/macports
rsync://rsync.macports.org/macports/release/tarballs/ports.tar [default]
```
(Note the file URL should appear before the rsync URL)

3. Add these Portfiles to your system's port index:
```
% cd *MBSystemHome*/macports
% sudo portindex
``
Now you can reference and install these local port files as with any other
macports port command. To install MB-System, including the vtk/qt-based
GUI applications with macports, install the 'buildQt' variant as follows:

```
% cd *MBSystemHome*/macports
% sudo port install mb-system +buildQt
```
