#ifndef _TIMEIF_H
#define _TIMEIF_H


class TimeIF {

  public:

  /////////////////////////////////////////////////////////
  // Structure to hold clock time
  struct TimeSpec {
    long seconds;
    long nanoSeconds;
  };
};

#endif
