//-----------------------------------------------------------------------
//DSP001.H
//-----------------------------------------------------------------------

#ifndef DSP001H
#define DSP001H

class DSP001
{
  public: //user functions
    DSP001();
    ~DSP001();
    void  B2Lendian(unsigned char*, int*, unsigned int);
    int   *GetFilterPtr(void);
    float   *GetFPFilterPtr(void);
    void  MMXConvolute(int*, int*, unsigned int);
    void  SSEConvolute(float*,float*,unsigned int);
    void  GPRConvolute(int* ,int*, unsigned int);
    void  SetFilterSize(unsigned int);
    void  MMXdemodSSB(int*, int*, unsigned int, unsigned int);
    void  SSEdemodSSB(float*, float*, unsigned int, unsigned int);
    void  SSEMakeAudioSample(float *,short *, unsigned int, unsigned int);

  private:
    int *psig_buf,*pfilt_buf,*psignal_buf;
    float *pfpsig_buf,*pfpfilt_buf,*pfpsignal_buf;
    unsigned int size_filter;

};

#endif

