.TH mbgrdviz l "22 October 2007" "MB-System 5.0" "MB-System 5.0"
.SH NAME
\fBmbgrdviz\fP - Simple interactive 2D/3D visualization of GMT grids..

.SH VERSION
Version 5.0

.SH SYNOPSIS
\fBmbgrdviz\fP [-I\fIgridfile\fP \fB-T -V -H\fP]

.SH DESCRIPTION

\fIOverview\fP

\fBMBgrdviz\fP is an interactive 2D/3D visualization tool for \fBGMT\fP topography 
grid files. \fBMBgrdviz\fP can be used for general data visualization and also has features 
integrating it with swath mapping data processing and swath survey planning.

Once a topographic grid has been read, it may be displayed either as a 2D map or as 
a 3D model. The 2D view can be panned and zoomed; the 3D view also allows arbitrary 
rotations of both the topography model and the viewpoint. 

In addition to the primary topography, other data sets (e.g. sidescan sonar
mosaics) can be read and displayed as overlays on the topography. Navigation
of ships or underwater vehicles can be read and displayed; if associated with
swath data the extent of mapping coverage can be displayed. Site data (individual
point locations) can be read, displayed, interactively created and modified, and
saved. Route data (collections of waypoints) can also be read, displayed, 
interactively created and modified, and saved.

The topography data can be shown using a number of colortables. The colortables can
be linearly stretched between minimum and maximum values, or applied using
a histogram equalization. The display may also be shaded using an
illumination model, slope magnitude, or overlay grid data. Overlay data
can be displayed draped on the topography. Topographic 
contours can be displayed using a user defined contour interval. 

In addition to modifying the view, users can interact with the display in 
several ways. Users can pick single points on the surface to determine position 
and topography values, or pick and drag to determine range and bearing between 
two locations. Arbitrary rectangular regions with north-south and east-west boundaries or
arbitrarily oriented rectangular areas can be defined by picking and dragging. 
The data within a region can be opened as a new display; areas can be used
for survey planning.

Topographic profiles associated with two-point picks, selected navigation, or 
selected routes can be displayed in a separate window and saved.

\fBMBgrdviz\fP supports as many as ten displays deriving either from reading
topography grids or extracting user defined regions from existing displays.

\fIStarting Up\fP

When \fBmbgrdviz\fP is invoked on the command line, a single window comes up with
the title \fBMBgrdviz\fP and two menus: \fBFile\fP and \fBHelp\fP. The \fBHelp\fP 
menu contains a single item named \fBAbout\fP, which brings up a simple 
information window including the MB-System release version identifier. 
The \fBFile\fP menu contains several items, but initially only the \fBOpen Primary 
Grid\fP and \fBQuit\fP items are enabled. Selecting \fBOpen Primary Grid\fP brings 
up a filesystem browser window that allows users to find and select the desired 
\fBGMT\fP topography grid. By default, the browser filter is set to display only 
files ending with a ".grd" suffix. Users can change this filter (e.g. setting it 
to "*" will display all files) and then hit the \fBFilter\fP button to relist the 
files as desired. Once the desired topography grid file has been located and 
selected so that its full path appears in the \fBSelection\fP text item, clicking 
the \fBOK\fP button will cause \fBMBgrdviz\fP to read the grid and open an \fBMBview\fP 
display window.

Initially, the topography will be displayed in 2D, or map view mode. The color 


.SH AUTHORSHIP
David W. Caress (caress@mbari.org)
.br
  Monterey Bay Aquarium Research Institute
.br
Dale N. Chayes (dale@ldeo.columbia.edu)
.br
  Lamont-Doherty Earth Observatory

.SH OPTIONS
.TP
.B \-H
This "help" flag cause the program to print out a description
of its operation and then exit immediately.
.TP
.B \-I
\fIgridfile\fP
.br
Sets a GMT grid file to be read and displayed at startup.
This option is usually used only when \fBMBgrdviz\fP is started
automatically from some other process. 
.TP
.B \-T
Sets a test grid to be calculated and displayed at startup.This
allows the program to be tested even when a grid file is
unavailable. 
.TP
.B \-V
Normally, \fBMBgrdviz\fP outputs limited information to the stderr stream.  
If the \fB-V\fP flag is given, then \fBMBgrdviz\fP works in a "verbose" mode and
outputs the program version being used, all error status messages, 
and a large amount of other information including all of the
beams flagged or zeroed.

.SH INTERACTIVE CONTROLS: MBgrdviz Startup Window
.TP
.B [MBgrdviz]:<File->Open Primary Grid>
This menu button pops up a file browser that allows the user to
select an input \fBGMT\fP grid file. If the OK button is clicked, \fBMBgrdviz\fP will
attempt to read a topography grid from the specified file. If successful,
a new \fBMBview\fP window will appear displaying the topography.
.TP
.B [MBgrdviz]:<File->Open Site>
This menu button pops up a file browser that allows the user to
select an input site file. A site file specifies one or more 
individual locations that can be displayed and interactively moved.
Site files are text with each line delineating a single site with
white space delimited columns
.TP
.B [MBgrdviz]:<File->Open Route>
.TP
.B [MBgrdviz]:<File->Open Navigation>
.TP
.B [MBgrdviz]:<File->Open Swath>
.TP
.B [MBgrdviz]:<File->Save Site>
.TP
.B [MBgrdviz]:<File->Save Route>
.TP
.B [MBgrdviz]:<File->Quit>
This button causes the program to exit (mostly) gracefully. 
.TP
.B [MBgrdviz]:<Help->About>
This button causes the program to bring up a dialog showing the
program's name, version, and authors.

.SH INTERACTIVE CONTROLS: MBview Main Window
.TP
.B [MBview]:<View->Map Display>
.TP
.B [MBview]:<View->3D Display>
.TP
.B [MBview]:<View->Topography>
.TP
.B [MBview]:<View->Topography Slope>
.TP
.B [MBview]:<View->Overlay>
.TP
.B [MBview]:<View->No Shading>
.TP
.B [MBview]:<View->Shading by Illumination>
.TP
.B [MBview]:<View->Shading by Slope>
.TP
.B [MBview]:<View->Shading by Overlay>
.TP
.B [MBview]:<View->Topography Contours>
.TP
.B [MBview]:<View->Sites>
.TP
.B [MBview]:<View->Routes>
.TP
.B [MBview]:<View->Navigation>
.TP
.B [MBview]:<View->Draped Navigation>
.TP
.B [MBview]:<View->Haxby Colortable>
.TP
.B [MBview]:<View->Bright Rainbow Colortable>
.TP
.B [MBview]:<View->Muted Rainbow Colortable>
.TP
.B [MBview]:<View->Grayscale Colortable>
.TP
.B [MBview]:<View->Flat Gray Colortable>
.TP
.B [MBview]:<View->Sealevel Colortable>
.TP
.B [MBview]:<Controls->Colors and Contours>
.TP
.B [MBview]:<Controls->2D Controls>
.TP
.B [MBview]:<Controls->3D Controls>
.TP
.B [MBview]:<Controls->Shading>
.TP
.B [MBview]:<Controls->Resolution>
.TP
.B [MBview]:<Controls->Projections>
.TP
.B [MBview]:<Controls->Site List>
.TP
.B [MBview]:<Controls->Route List>
.TP
.B [MBview]:<Controls->Navigation List>
.TP
.B [MBview]:<Mouse->Full>
.TP
.B [MBview]:<Mouse->Reset>
.TP
.B [MBview]:<Mouse->Clear>
.TP
.B [MBview]:<Mouse->Pan and Zoom>
.TP
.B [MBview]:<Mouse->Rotate Model>
.TP
.B [MBview]:<Mouse->Rotate View>
.TP
.B [MBview]:<Mouse->Shading>
.TP
.B [MBview]:<Mouse->Pick Area>
.TP
.B [MBview]:<Mouse->Edit Sites>
.TP
.B [MBview]:<Mouse->Edit Routes>
.TP
.B [MBview]:<Mouse->Pick Nav>
.TP
.B [MBview]:<Action->About MBview...>
.TP
.B [MBview]:<Action->Open Overlay Grid>
.TP
.B [MBview]:<Action->Open Site File>
.TP
.B [MBview]:<Action->Open Route File>
.TP
.B [MBview]:<Action->Open Navigation>
.TP
.B [MBview]:<Action->Open Swath File>
.TP
.B [MBview]:<Action->Save Site File>
.TP
.B [MBview]:<Action->Save Route File>
.TP
.B [MBview]:<Action->Open Region as New View>
.TP
.B [MBview]:<Action->Generate Survey Route from Area>
.TP
.B [MBview]:<Dismiss->Dismiss>
.TP
.B [MBview]:<Full>
.TP
.B [MBview]:<Reset>
.TP
.B [MBview]:<Clear>
.TP
.B [MBview]:<Pan and Zoom>
.TP
.B [MBview]:<Rotate Model>
.TP
.B [MBview]:<Rotate View>
.TP
.B [MBview]:<Shading>
.TP
.B [MBview]:<Pick Area>
.TP
.B [MBview]:<Edit Sites>
.TP
.B [MBview]:<Edit Routes>
.TP
.B [MBview]:<Pick Nav>

.SH INTERACTIVE CONTROLS: MBview Colors and Contours Window
.TP
.B [Colors and Contours]:{Data Color Bounds}"Minimum"
.TP
.B [Colors and Contours]:{Data Color Bounds}"Maximum"
.TP
.B [Colors and Contours]:{Data Color Bounds}<<Cold-to-Hot><Hot-to-Cold>>
.TP
.B [Colors and Contours]:"Data Contour Interval"
.TP
.B [Colors and Contours]:{Data Slope Color Bounds}"Minimum"
.TP
.B [Colors and Contours]:{Data Slope Color Bounds}"Maximum"
.TP
.B [Colors and Contours]:{Data Slope Color Bounds}<<Cold-to-Hot><Hot-to-Cold>>
.TP
.B [Colors and Contours]:{Overlay Color Bounds}"Minimum"
.TP
.B [Colors and Contours]:{Overlay Color Bounds}"Maximum"
.TP
.B [Colors and Contours]:{Overlay Color Bounds}<<Cold-to-Hot><Hot-to-Cold>>
.TP
.B [Colors and Contours]:<Apply>
.TP
.B [Colors and Contours]:<Dismiss>

.SH INTERACTIVE CONTROLS: MBview Colors and Contours Window
.TP
.B [2D Parameters]:{View Offset}"X"
.TP
.B [2D Parameters]:{View Offset}"Y"
.TP
.B [2D Parameters]:{View Offset}"Zoom"
.TP
.B [2D Parameters]:<Apply>
.TP
.B [2D Parameters]:<Dismiss>

.SH INTERACTIVE CONTROLS: MBview Colors and Contours Window
.TP
.B [3D Parameters]:{Model Orientation & Zoom}"Azimuth"
.TP
.B [3D Parameters]:{Model Orientation & Zoom}"Elevation"
.TP
.B [3D Parameters]:{Model Orientation & Zoom}"Zoom"
.TP
.B [3D Parameters]:{View Orientation & Zoom}"Azimuth"
.TP
.B [3D Parameters]:{View Orientation & Zoom}"Elevation"
.B [3D Parameters]:{View Orientation & Zoom}"Zoom"
.TP
.B [3D Parameters]:"Vertical Exageration"
.TP
.B [3D Parameters]:{Pan}"X"
.TP
.B [3D Parameters]:{Pan}"Y"
.TP
.B [3D Parameters]:<Apply>
.TP
.B [2D Parameters]:<Dismiss>

.SH INTERACTIVE CONTROLS: MBview Colors and Contours Window
.TP
.B [Shading Parameters]:{Shading by Illumination}"Amplitude"
.TP
.B [Shading Parameters]:{Shading by Illumination}"Azimuth"
.TP
.B [Shading Parameters]:{Shading by Illumination}"Elevation"
.TP
.B [Shading Parameters]:{Shading by Slope}"Amplitude"
.TP
.B [Shading Parameters]:{Shading by Overlay}"Amplitude"
.TP
.B [Shading Parameters]:{Shading by Overlay}"Center"
.TP
.B [Shading Parameters]:{Shading by Overlay}<<Cold-to-Hot><Hot-to-Cold>>
.TP
.B [Shading Parameters]:<Apply>
.TP
.B [Shading Parameters]:<Dismiss>

.SH INTERACTIVE CONTROLS: MBview Colors and Contours Window
.TP
.B [Rendering Resolution]:<Dismiss>
.TP
.B [Rendering Resolution]:==Low Resolution Dimension==
.TP
.B [Rendering Resolution]:==High Resolution Dimension==
.TP
.B [Rendering Resolution]:<Dismiss>
.TP
.B [Rendering Resolution]:<Dismiss>

.SH INTERACTIVE CONTROLS: MBview Colors and Contours Window
.TP
.B [Projections]:{Display Projection}<<Geographic><UTM><Spheroid>>
.TP
.B [Projections]:<Dismiss>

.SH INTERACTIVE CONTROLS: MBview Colors and Contours Window
.TP
.B [Site List]:|Site List|
.TP
.B [Site List]:<Delete Selected Sites>
.TP
.B [Site List]:<Dismiss>

.SH INTERACTIVE CONTROLS: MBview Colors and Contours Window
.TP
.B [Route List]:|Route List|
.TP
.B [Route List]:<Delete Selected Routes>
.TP
.B [Route List]:<Dismiss>

.SH INTERACTIVE CONTROLS: MBview Colors and Contours Window
.TP
.B [Navigation List]:|Navigation List|
.TP
.B [Navigation List]:<Delete Selected Navigation>
.TP
.B [Navigation List]:<Dismiss>

.SH MOUSE ACTIONS

.SH KEYBOARD ACTIONS

.SH SEE ALSO
\fBmbsystem\fP(l)

.SH BUGS
This program is not done, nor is it adequately documented.

