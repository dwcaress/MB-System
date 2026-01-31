#ifndef MBVIEW_H
#define MBVIEW_H

namespace mb_system {
/** MBView contains static utility functions that replicate functionality
of the legacy mbview C library.
 */
class MBView {

 public:


  struct World {
  };
 
  // mbview_getdataptr()
  static int getDataPointer(int verbose, size_t instance,
			    struct mbview_struct **datahandle, int *error);

  // mbview_getsharedptr()
  static int getSharedPointer(int verbose,
			      struct mbview_shareddata_struct **sharedhandle,
			      int *error);

  // mbview_colorvalue_instance()
  static int colorValueInstance(size_t instance, double value,
				float *red, float *green, float *blue);  

  // mbview_plothigh()
  static int plotHigh(size_t instance);

  // mbview_updateprimarygrid()
  static int updatePrimaryGrid(int verbose, size_t instance,
			       int primary_n_columns, int primary_n_rows,
			       float *primary_data, int *error);

  // mbview_updateprimarygridcell()
  static int updatePrimaryGridCell(int verbose, size_t instance,
				   int primary_ix, int primary_jy,
				   float value, int *error);
 
  // mbview_updatesecondarygrid()
  static int updateSecondaryGrid(int verbose, size_t instance,
				 int secondary_n_columns, int secondary_n_rows,
				 float *secondary_data);
};

}



#endif


