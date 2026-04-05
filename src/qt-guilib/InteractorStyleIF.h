#ifndef InteractorStyleIF_H
#define InteractorStyleIF_H


namespace mb_system {

  /** Interface defines additional methods that vtkInteractorStyle subclasses
      should implement. 
  */
  class InteractorStyleIF {

  public:

    /// Print help message describing mouse actions
    virtual const char *printHelp()  = 0;
  };
}



#endif

