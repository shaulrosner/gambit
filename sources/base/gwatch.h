//
// $Source$
// $Date$
// $Revision$
//
// DESCRIPTION:
// Interface to a general stopwatch class
//

#ifndef GWATCH_H
#define GWATCH_H

//
// This is a general stopwatch class.  Under Unix, it returns the time elapsed
// in terms of the process' user time; under DOS and MS-Windows, it returns
// the total elapsed time.
//
// Note that time functions can be highly system-dependent, especially under
// Unix, so care should be taken when porting this code.
//
class gWatch    {
  private:
    bool running;
    long start, stop;
    char *time_str;

  public:
    gWatch(bool run = true);
    ~gWatch();

    bool IsRunning(void) const;

    void Start(void);
    void Stop(void);
    double Elapsed(void) const;
    
    char *const ElapsedStr(void);
};

#endif   // GWATCH_H