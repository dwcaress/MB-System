.TH mbgrdviz l "18 February 2005" "MB-System 5.0" "MB-System 5.0"
.SH NAME
\fBmbgrdviz\fP - Simple interactive 2D/3D visualization of GMT grids..

.SH VERSION
Version 5.0

.SH SYNOPSIS
\fBmbgrdviz\fP [-I\fIgridfile\fP \fB-T -V -H\fP]

.SH DESCRIPTION
\fBMBgrdviz\fP is a simple, interactive 2D/3D vizualization tool
for GMT grid files. Essentially, this program is a demonstration and
test platform for a vizualization API called \fBMBview\fP that will be used
in planned (version 6) \fBMB-System\fP processing tools.

This man page does not yet include either a description of the 
program's capabilities or documentation of the interface. The user's success
will therefore depend on some combination of the author's success in 
providing an intutitive interface and the user's cleverness and/or 
determination.

Good luck.

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

.SH INTERACTIVE CONTROLS: MBgrdviz Main Window
.TP
.B [MBgrdviz]:<File->Open Primary Grid
This menu button pops up a file browser that allows the user to
select an input GMT grid file.
.TP
.B [MBgrdviz]:<File->Open Site>
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

