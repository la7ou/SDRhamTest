//-----------------------------------------------------------------------
//DSP002.H
//-----------------------------------------------------------------------

#ifndef DSP002_H
#define DSP002_H

class DSP002
{
  public: //user functions
    void  FPdemodFM(int *, int *, unsigned int);
    void  FPdemodAM(int *, int *, unsigned int);

  private:
    inline double signum(double);
    double old_sample[2],dc_value;
};

#endif
