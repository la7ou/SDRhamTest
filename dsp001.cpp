//-----------------------------------------------------------------------
//DSP001.CPP
//-----------------------------------------------------------------------

//#include <vcl.h>
//#pragma hdrstop

#include  "dsp001.h"
#include <arpa/inet.h>  // ntohs()

//The following constants are for DSP functiona
#define FIR_FILTER_SIZE 1024
#define SAMPLE_SIZE     6
#define SAMPMAXIMUM     30000    //this is the maximum sample amplitude
#define AUDIOMAXIMUM    32767    //this is the maximum audio amplitude

#ifdef LINUX
  #define asm_b (					\
     ".intel_syntax noprefix\n"
  #define asm_e );
  #define x1(y...) x2(y \n)
  #define x2(y...) #y
#else
  #define asm_b asm {
  #define asm_e }
  #define x1(y...) y
#endif

//---------------------------------------------------------------------------
// Constructor()
//---------------------------------------------------------------------------

DSP001::DSP001()
{
  psig_buf=new int[FIR_FILTER_SIZE*2];
  pfilt_buf=new int[FIR_FILTER_SIZE*2];
  psignal_buf=psig_buf;
  pfpsig_buf=new float[FIR_FILTER_SIZE*2];
  pfpfilt_buf=new float[FIR_FILTER_SIZE*2];
  pfpsignal_buf=pfpsig_buf;
  
  for(unsigned int i=0;i <(FIR_FILTER_SIZE*2); i++)
  {
    psig_buf[i]=0;   //Intilaize all signal coeffesients to zero
    pfilt_buf[i]=0;   //Intilaize all filter coeffesients to zero
  }
  pfilt_buf[0]=0x7ffff;  //set filter coeff 0 to 1 I-coeffesient
  pfilt_buf[1]=0x7ffff;  //set filter coeff 1 to 1 Q-coeffesient

  for(unsigned int i=0;i <(FIR_FILTER_SIZE*2); i++)
  {
    pfpsig_buf[i]=0;   //Intilaize all signal coeffesients to zero
    pfpfilt_buf[i]=0;   //Intilaize all filter coeffesients to zero
  }
  pfpfilt_buf[0]=1.0;  //set filter coeff 0 to 1 I-coeffesient
  pfpfilt_buf[1]=1.0;  //set filter coeff 1 to 1 Q-coeffesient
  size_filter=17;
}

//---------------------------------------------------------------------------
// Destructor()
//---------------------------------------------------------------------------

DSP001::~DSP001()
{
  delete[] psig_buf;
  delete[] pfilt_buf;
  delete[] pfpsig_buf;
  delete[] pfpfilt_buf;
}

//---------------------------------------------------------------------------


void DSP001::B2Lendian(unsigned char *src_ptr,int *dest_ptr,unsigned int size)
{
#ifndef ASM
	for (unsigned int i=0;i<size;i++)
	{
		dest_ptr[i] = ntohs(src_ptr[i]); // converted from network () to host byte order.
	}
#else
  asm_b
    x1(MOV   ECX,size)      //Size of buffer to convert
    x1(TEST  ECX,-1)
    x1(JZ    no_convert)
    x1(MOV   EDI,dest_ptr)  //EDI is pointer to output buffer
    x1(MOV   ESI,src_ptr)   //ESI is pointer to source buffer

    x1(next_sample:)
    x1(MOV   EAX,[ESI])     //Here to process I-sample
    x1(BSWAP EAX)
    x1(SAR   EAX,8)
    x1(MOV   [EDI],EAX)
    x1(ADD   ESI,(SAMPLE_SIZE/2))         //Inc source pointger
    x1(ADD   EDI,4)         //Inc destination pointer

    x1(MOV   EAX,[ESI])     //Here to process Q-sample
    x1(BSWAP EAX)
    x1(SAR   EAX,8)
    x1(MOV   [EDI],EAX)
    x1(ADD   ESI,(SAMPLE_SIZE/2))         //Inc source pointger
    x1(ADD   EDI,4)         //Inc destination pointer

    x1(LOOP  next_sample)
    x1(no_convert:)
  asm_e
#endif
}

//---------------------------------------------------------------------------

int *DSP001::GetFilterPtr(void)
{
  return pfilt_buf;
}


//---------------------------------------------------------------------------

float *DSP001::GetFPFilterPtr(void)
{
  return pfpfilt_buf;
}


//---------------------------------------------------------------------------

void  DSP001::SetFilterSize(unsigned int filter_size)
{
  size_filter=filter_size;
  psignal_buf=psig_buf;
  pfpsignal_buf=pfpsig_buf;
}

//---------------------------------------------------------------------------

void DSP001::GPRConvolute(int *src_ptr,int *dest_ptr,unsigned int size)
{
  int i_accum[2],q_accum[2];
  int *end_of_buffer=&psig_buf[(size_filter<<1)-2];
  int *pbuf_sig=psig_buf;
  int *pbuf_filt=pfilt_buf;
  int *pbuf_signal=psignal_buf;
  unsigned int filtersize=size_filter;

#ifndef ASM
    // C kode her 
#else
  asm_b
    x1(MOV     ECX,size)
    x1(TEST    ECX,-1)
    x1(JZ      no_convolute)
                  //here to perform convolution
    x1(MOV     EBX,pbuf_signal) //EBX is pointer to data line
    x1(MOV     ESI,src_ptr)   //ESI is pointer to source data
    x1(MOV     EDI,dest_ptr)  //EDI is pointer to destination data
    x1(do_next_conv:)
    x1(MOV     EAX,[ESI])
    x1(MOV     [EBX],EAX)   //move I-sample to signal line
    x1(MOV     EAX,[ESI+4])
    x1(MOV     [EBX+4],EAX)   //move Q-sample to signal line
    x1(CMP     EBX,end_of_buffer)
    x1(JGE     ret_start_buffer)
    x1(ADD     EBX,8)       //next pos in signal buffer
    x1(JMP     do_convolute)
    x1(ret_start_buffer:)
    x1(MOV     EBX,pbuf_sig)
    x1(do_convolute:)
    x1(PUSH    ECX)
    x1(PUSH    EDI)
    x1(MOV     ECX,filtersize)
    x1(MOV     EDI,pbuf_filt)
    x1(MOV     i_accum,0)
    x1(MOV     i_accum[4],0) //i_accum+4,0)      //clear I sum reg before start
    x1(MOV     q_accum,0)
    x1(MOV     q_accum[4],0)//q_accum+4,0)      //clear Q sum reg before start

    x1(filter_loop:)
    x1(MOV     EAX,[EBX])   //I-sample processing
    x1(IMUL    [EDI])
    x1(ADD     i_accum,EAX)
    x1(ADC     i_accum[4],EDX)//i_accum+4,EDX)
    x1(MOV     EAX,[EBX+4])   //Q-sample processing
    x1(IMUL    [EDI+4])
    x1(ADD     q_accum,EAX)
    x1(ADC     q_accum[4],EDX)//q_accum+4,EDX)
    x1(CMP     EBX,end_of_buffer)
    x1(JGE     new_start_buffer)
    x1(ADD     EBX,8)
    x1(JMP     next_loop)
    x1(new_start_buffer:)
    x1(MOV     EBX,pbuf_sig)
    x1(next_loop:)
    x1(ADD     EDI,8)
    x1(LOOP    filter_loop)

    x1(POP     EDI)
    x1(POP     ECX)

    x1(MOV     EAX,i_accum+4)   //I-sample amplitude shift
    x1(SHL     EAX,8)
    x1(MOV     AL,BYTE PTR i_accum[3])//PTR i_accum+3)
    x1(MOV     [EDI],EAX)
    x1(MOV     EAX,q_accum+4)   //Q-sample amplitude shift
    x1(SHL     EAX,8)
    x1(MOV     AL,BYTE PTR q_accum[3]) // PTR q_accum+3)
    x1(MOV     [EDI+4],EAX)

    x1(ADD     EDI,8)     //next pos in destnation buffer
    x1(ADD     ESI,8)     //next pos in source buffer
    x1(DEC     ECX)
    x1(JZ      conv_done)
    x1(JMP     do_next_conv)
    x1(conv_done:)
    x1(MOV     pbuf_signal,EBX)

    x1(no_convolute:)
  asm_e
#endif
  psignal_buf=pbuf_signal;
}


//---------------------------------------------------------------------------

void DSP001::MMXConvolute(int *src_ptr,int *dest_ptr,unsigned int size)
{
  int dummy[]={0,0};
  unsigned int mask[]={0x0000ffff,0x0000ffff};
  int *end_of_buffer=&psig_buf[(size_filter<<1)-2];
  int *pbuf_sig=psig_buf;
  int *pbuf_filt=pfilt_buf;
  int *pbuf_signal=psignal_buf;
  unsigned int filtersize=size_filter;

#ifndef ASM
    // C kode her 
#else
  asm_b

    x1(MOV     ECX,size)
    x1(TEST    ECX,-1)
    x1(JZ      no_convolute)
                  //here to perform convolution
    x1(MOV     EBX,pbuf_signal) //EBX is pointer to data line
    x1(MOV     ESI,src_ptr)   //ESI is pointer to source data
    x1(MOV     EDI,dest_ptr)  //EDI is pointer to destination data
    x1(next_conv:)
    x1(MOV     EAX,[ESI])
    x1(MOV     [EBX],EAX)   //move I-sample to signal line
    x1(MOV     EAX,[ESI+4])
    x1(MOV     [EBX+4],EAX)   //move Q-sample to signal line
    x1(CMP     EBX,end_of_buffer)
    x1(JGE     ret_start_buffer)
    x1(ADD     EBX,8)       //next pos in signal buffer
    x1(JMP     do_convolute)
    x1(ret_start_buffer:)
    x1(MOV     EBX,pbuf_sig)
    x1(do_convolute:)
    x1(PUSH    ECX)
    x1(PUSH    EDI)
    x1(MOV     ECX,filtersize)
    x1(MOV     EDI,pbuf_filt)
    x1(MOVQ    MM2,QWORD PTR dummy)   //clear sum reg before start
    x1(filter_loop:)
    x1(MOVQ    MM0,QWORD PTR [EBX])
    x1(PSRAD   MM0,8)
    x1(PAND    MM0,QWORD PTR mask)
    x1(MOVQ    MM1,QWORD PTR [EDI])
    x1(PSRAD   MM1,8)
    x1(PAND    MM1,QWORD PTR mask)
    x1(PMADDWD MM0,MM1)
    x1(PADDD   MM2,MM0)
    x1(CMP     EBX,end_of_buffer)
    x1(JGE     new_start_buffer)
    x1(ADD     EBX,8)
    x1(JMP     next_loop)
    x1(new_start_buffer:)
    x1(MOV     EBX,pbuf_sig)
    x1(next_loop:)
    x1(ADD     EDI,8)
    x1(LOOP    filter_loop)
    x1(POP     EDI)
    x1(POP     ECX)

    x1(PSRAD   MM2,8)     //Shift result to 24 bit
    x1(MOVQ    QWORD PTR [EDI],MM2)
    x1(ADD     EDI,8)     //next pos in destnation buffer
    x1(ADD     ESI,8)     //next pos in source buffer
    x1(LOOP    next_conv)

    x1(MOV     pbuf_signal,EBX)
    x1(EMMS)

    x1(no_convolute:)
  asm_e
#endif
  psignal_buf=pbuf_signal;
}


//---------------------------------------------------------------------------

void DSP001::SSEConvolute(float *src_ptr,float *dest_ptr,unsigned int size)
{
  float dummy[]={0,0};
  float *end_of_buffer=&pfpsig_buf[(size_filter<<1)-2];
  float *pbuf_sig=pfpsig_buf;
  float *pbuf_filt=pfpfilt_buf;
  float *pbuf_signal=pfpsignal_buf;
  unsigned int filtersize=size_filter;

#ifndef ASM
    // C kode her 
#else
  asm_b
    x1( MOV     ECX,size)
      x1( TEST    ECX,-1)
      x1( JZ      no_convolute)
                  //here to perform convolution
      x1( MOV     EBX,pbuf_signal) //EBX is pointer to data line
      x1( MOV     ESI,src_ptr)   //ESI is pointer to source data
      x1( MOV     EDI,dest_ptr)  //EDI is pointer to destination data
      x1(next_conv:)
      x1( MOV     EAX,[ESI])
      x1( MOV     [EBX],EAX)   //move I-sample to signal line
      x1( MOV     EAX,[ESI+4])
      x1( MOV     [EBX+4],EAX)   //move Q-sample to signal line
      x1( CMP     EBX,end_of_buffer)
      x1( JGE     ret_start_buffer)
      x1( ADD     EBX,8)       //next pos in signal buffer
      x1( JMP     do_convolute)
      x1(ret_start_buffer:)
      x1( MOV     EBX,pbuf_sig)
      x1(do_convolute:)
      x1( PUSH    ECX)
      x1( PUSH    EDI)
      x1( MOV     ECX,filtersize)
      x1( MOV     EDI,pbuf_filt)
      x1( MOVLPS  XMM2,QWORD PTR dummy)   //clear sum reg before start
      x1(filter_loop:)
      x1( MOVLPS  XMM0,QWORD PTR [EBX])
      x1( MOVLPS  XMM1,QWORD PTR [EDI])
      x1( MULPS   XMM0,XMM1)
      x1( ADDPS   XMM2,XMM0)
      x1( CMP     EBX,end_of_buffer)
      x1( JGE     new_start_buffer)
      x1( ADD     EBX,8)
      x1( JMP     next_loop)
      x1(new_start_buffer:)
      x1( MOV     EBX,pbuf_sig)
      x1(next_loop:)
      x1( ADD     EDI,8)
      x1( LOOP    filter_loop)
      x1( POP     EDI)
      x1( POP     ECX)

      x1( MOVLPS  QWORD PTR [EDI],XMM2)
      x1( ADD     EDI,8)     //next pos in destnation buffer
      x1( ADD     ESI,8)     //next pos in source buffer
      x1( LOOP    next_conv)

      x1( MOV     pbuf_signal,EBX)
      x1(no_convolute:)
  asm_e
#endif
  pfpsignal_buf=pbuf_signal;
}

//---------------------------------------------------------------------------

void DSP001::SSEdemodSSB(float *pscr,float *pdest,unsigned int size,unsigned int mode)
{
#ifndef ASM
    // C kode her 
#else
  asm_b
    x1(MOV       ESI,pscr)
    x1(MOV       EDI,pdest)
    x1(MOV       ECX,size)

    x1(dodemodssb:)
    x1(MOVLPS    XMM0,QWORD PTR [ESI]) //source sample
    x1(MOVAPS    XMM1,XMM0)            //copy sample to XMM1
    x1(SHUFPS    XMM1,XMM1,0x01)       //Mov Q-sample to low DWORD of XMM1
    x1(MOV       EAX,mode)
    x1(OR        EAX,EAX)
    x1(JZ        low_side)
    x1(ADDSS     XMM0,XMM1)
    x1(JMP       do_side)
    x1(low_side:)
    x1(SUBSS     XMM0,XMM1)
    x1(do_side:)
    x1(MOVSS     DWORD PTR [EDI],XMM0)
    x1(ADD       ESI,8)
    x1(ADD       EDI,4)
    x1(LOOP      dodemodssb)
  asm_e
#endif
}

//---------------------------------------------------------------------------

void DSP001::MMXdemodSSB(int *src_ptr,int *dest_ptr,unsigned int size,unsigned int mode)
{
  int i_sample[2],q_sample[2];
	
#ifndef ASM
    // C kode her 
#else
  asm_b
    x1(MOV     ECX,size)
    x1(TEST    ECX,-1)
    x1(JZ      no_demod)

    x1(SHR     ECX,1)     //two samples is processed in each loop
    x1(MOV     ESI,src_ptr)
    x1(MOV     EDI,dest_ptr)
    x1(demod_loop:)
    x1(MOV     EAX,[ESI])
    x1(MOV     i_sample,EAX)
    x1(MOV     EAX,[ESI+8])
    x1(MOV     i_sample[4],EAX) //i_sample+4,EAX)
    x1(MOV     EAX,[ESI+4])
    x1(MOV     q_sample,EAX)
    x1(MOV     EAX,[ESI+12])
    x1(MOV     q_sample[4],EAX) //q_sample+4,EAX)
    x1(MOVQ    MM0,QWORD PTR i_sample)
    x1(PSRAD   MM0,1)     //divide by two before summation
    x1(MOVQ    MM1,QWORD PTR q_sample)
    x1(PSRAD   MM1,1)
    x1(test    mode,-1)
    x1(JNZ     do_sub)
    x1(PADDD   MM0,MM1)
    x1(JMP     sum_done)
    x1(do_sub:)
    x1(PSUBD   MM0,MM1)
    x1(sum_done:)
    x1(MOVQ    QWORD PTR [EDI],MM0)
    x1(ADD     ESI,16)
    x1(ADD     EDI,8)
    x1(LOOP    demod_loop)

    x1(EMMS)
    x1(no_demod:)
  asm_e
#endif
}

//---------------------------------------------------------------------------

void DSP001::SSEMakeAudioSample(float *src_ptr,short *dest_ptr,unsigned int size,unsigned int decim)
{
  unsigned int byteoffset;
  float amplitude;

  byteoffset=4*decim;
  amplitude=SAMPMAXIMUM;
	
#ifndef ASM
    // C kode her 
#else
  asm_b
    x1(MOV       ECX,size)
    x1(TEST      ECX,-1)
    x1(JZ        no_conv)   //do nothing if size is zero
    x1(MOV       ESI,src_ptr)
    x1(MOV       EDI,dest_ptr)
    x1(MOVSS     XMM1,DWORD PTR amplitude)

    x1(do_dec_gain:)
    x1(MOVSS     XMM0,DWORD PTR [ESI])
    x1(MULSS     XMM0,XMM1)   //scale sample for 16-bit audio
    x1(CVTSS2SI  EAX,XMM0)
    x1(CMP       EAX,AUDIOMAXIMUM)
    x1(JLE       pos_lev_ok)
    x1(MOV       AX,AUDIOMAXIMUM)  //saturate AX to max positiv value
    x1(JMP       level_ok)
    x1(pos_lev_ok:)
    x1(CMP       EAX,-AUDIOMAXIMUM)
    x1(JGE       level_ok)
    x1(MOV       AX,-AUDIOMAXIMUM)  //saturate AX to max negative value
    x1(level_ok:)
    x1(MOV       [EDI],AX)
    x1(ADD       ESI,byteoffset)
    x1(ADD       EDI,2)
    x1(LOOP      do_dec_gain)
    x1(no_conv:)
  asm_e
#endif
}
