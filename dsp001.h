//-----------------------------------------------------------------------
//DSP001.H
//-----------------------------------------------------------------------

#ifndef DSP001_H
#define DSP001_H

class DSP001
{
  public: //user functions
    DSP001();
    ~DSP001();
    void  B2Lendian(unsigned char*, int*, unsigned int);
    int   *GetFilterPtr(void);
    void  MMXConvolute(int *,int *,unsigned int);
    void  SSEConvolute(int *,int *,unsigned int);
    void  GPRConvolute(int *,int *,unsigned int);
    void  SetFilterSize(unsigned int);
    void  MMXdemodSSB(int *,int *,unsigned int,unsigned int);
    void  MakeAudioSample(int *,short *,unsigned int,int,unsigned int);
  private:
    int *psig_buf,*pfilt_buf,*psignal_buf;
    unsigned int size_filter;

};

#endif

