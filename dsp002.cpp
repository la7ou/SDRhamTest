//-----------------------------------------------------------------------
//DSP002.CPP
//-----------------------------------------------------------------------

//#pragma hdrstop

#include  <math.h>
#include  "dsp002.h"


#define   AMPLITUDE     2670000  //max amplitude divide by PI
#define   PI            3.141592654
#define   PI_2          1.570796327



//----------------------------------------------------------------------

// FPdemodFM()

// This routine demodulates FM using floating point mathematics

//----------------------------------------------------------------------

void DSP002::FPdemodFM(int *src_ptr,int *dest_ptr,unsigned int size)
{
  double real,imag,angle,next[2],prev[2];

  //Do the first calculation using the last sample from previous frame

  prev[0]=old_sample[0];
  prev[1]=old_sample[1];
  next[0]=(double)src_ptr[0];
  next[1]=(double)src_ptr[1];

  real=next[0]*prev[0]+next[1]*prev[1];
  imag=next[1]*prev[0]-next[0]*prev[1];

  if(real==0)
    angle=signum(imag)*PI_2;
  else
  {
    if(real>=0)
      angle=atan(imag/real);
    else
      angle=atan(imag/real)+signum(imag)*PI;
  }
  dest_ptr[0]=(int)(((double)AMPLITUDE)*angle);

  //Do the remaining calculations

  for(unsigned int i=1,n=0; i<size; i++,n=n+2)
  {
    prev[0]=(double)src_ptr[n];
    prev[1]=(double)src_ptr[n+1];
    next[0]=(double)src_ptr[n+2];
    next[1]=(double)src_ptr[n+3];

    real=next[0]*prev[0]+next[1]*prev[1];
    imag=next[1]*prev[0]-next[0]*prev[1];

    if(real==0)
      angle=signum(imag)*PI_2;
    else
    {
      if(real>=0)
        angle=atan(imag/real);
      else
        angle=atan(imag/real)+signum(imag)*PI;
    }
    dest_ptr[i]=(int)(((double)AMPLITUDE)*angle);
  }

  //Store the last sample in current frame for next frame calculation

  old_sample[0]=next[0];
  old_sample[1]=next[1];
}


//----------------------------------------------------------------------

// FPdemodAM()

// This routine demodulates AM using floating point mathematics

//----------------------------------------------------------------------

void DSP002::FPdemodAM(int *src_ptr,int *dest_ptr,unsigned int size){
  double  i_sample,q_sample,mid_value,temp_value;

  mid_value=0;

  for(unsigned int i=0,n=0; i<size; i++,n=n+2)
  {
    i_sample=(double)src_ptr[n];
    q_sample=(double)src_ptr[n+1];
    temp_value=sqrt(i_sample*i_sample+q_sample*q_sample);
    mid_value=mid_value+temp_value;
    dest_ptr[i]=(int)(temp_value-dc_value);//(int)(temp_value-dc_value);
  }
  dc_value=mid_value/(double)size;
}

//----------------------------------------------------------------------
// signum()
//----------------------------------------------------------------------

double DSP002::signum(double var)
{
  if(var>=0)
    return 1;
  else
    return -1;
}

