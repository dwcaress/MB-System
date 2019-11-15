The GCTP FORTRAN version and GCTPc have differences with the UTM projection
as the zones are not handled the same.  GCTPc will use the entered zone first,
positive or negative.  If zone is zero, the first two values in the parameter
array (See Projection Parameters documentation) will be used to calculate the
zone.

The original tests comparing C and Fortran were done in several steps.
Three zones (negative CM, positive CM, and CM around zero) were tested with
both positive and negative zones on all systems against the FORTRAN version.
Then a file was made setting zone to zero and the parameter array lat and long
were used.  This file was compared to GCTPc's file with the appropriate zone
entered.  Also, files were made with each zone and transported to the other
systems and compared.

Note:  Since the UTM projection is intended to be used for relatively small
regions, the test grids are set to cover smaller areas than for global maps.
As projected points get further from the central meridian, the projected values
are less reliable.  Any points more than 90 degrees either side of the 
central meridian were not tested.


