//-----------------------------------------------------------------------
//DSP001.CPP
//-----------------------------------------------------------------------

//#include <vcl.h>
//#pragma hdrstop

#include  "dsp001.h"
//#include  "systemc.h"

//The following constants are for DSP finctiona
#define   FIR_FILTER_SIZE 1024
#define SAMPLE_SIZE       6


//---------------------------------------------------------------------------
// Constructor()
//---------------------------------------------------------------------------

DSP001::DSP001()
{
  psig_buf=new int[FIR_FILTER_SIZE*2];
  pfilt_buf=new int[FIR_FILTER_SIZE*2];
  psignal_buf=psig_buf;

  for(unsigned int i=0;i <(FIR_FILTER_SIZE*2); i++)
  {
    psig_buf[i]=0;   //Intilaize all signal coeffesients to zero
    pfilt_buf[i]=0;   //Intilaize all filter coeffesients to zero
  }
  pfilt_buf[0]=0x7fffff;  //set filter coeff 0 to 1 I-coeffesient
  pfilt_buf[1]=0x7fffff;  //set filter coeff 1 to 1 Q-coeffesient
  size_filter=17;
}

//---------------------------------------------------------------------------
// Destructor()
//---------------------------------------------------------------------------

DSP001::~DSP001()
{
  delete[] psig_buf;
  delete[] pfilt_buf;
}

//---------------------------------------------------------------------------


void DSP001::B2Lendian(unsigned char *src_ptr,int *dest_ptr,unsigned int size)
{
  asm
  {
      MOV   ECX,size      //Size of buffer to convert
      TEST  ECX,-1
      JZ    no_convert
      MOV   EDI,dest_ptr  //EDI is pointer to output buffer
      MOV   ESI,src_ptr   //ESI is pointer to source buffer

    next_sample:
      MOV   EAX,[ESI]     //Here to process I-sample
      BSWAP EAX
      SAR   EAX,8
      MOV   [EDI],EAX
      ADD   ESI,(SAMPLE_SIZE/2)         //Inc source pointger
      ADD   EDI,4         //Inc destination pointer

      MOV   EAX,ESI		//MOV   EAX,[ESI]     //Here to process Q-sample
      BSWAP EAX
      SAR   EAX,8
      MOV   [EDI],EAX
      ADD   ESI,(SAMPLE_SIZE/2)         //Inc source pointger
      ADD   EDI,4         //Inc destination pointer

      LOOP  next_sample
    no_convert:
  }
}

//---------------------------------------------------------------------------

int *DSP001::GetFilterPtr(void)
{
  return pfilt_buf;
}

//---------------------------------------------------------------------------

void  DSP001::SetFilterSize(unsigned int filter_size)
{
  size_filter=filter_size;
  psignal_buf=psig_buf;
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

  asm
  {
      MOV     ECX,size
      TEST    ECX,-1
      JZ      no_convolute
                  //here to perform convolution
      MOV     EBX,pbuf_signal //EBX is pointer to data line
      MOV     ESI,src_ptr   //ESI is pointer to source data
      MOV     EDI,dest_ptr  //EDI is pointer to destination data
    do_next_conv:
      MOV     EAX,[ESI]
      MOV     [EBX],EAX   //move I-sample to signal line
      MOV     EAX,[ESI+4]
      MOV     [EBX+4],EAX   //move Q-sample to signal line
      CMP     EBX,end_of_buffer
      JGE     ret_start_buffer
      ADD     EBX,8       //next pos in signal buffer
      JMP     do_convolute
    ret_start_buffer:
      MOV     EBX,pbuf_sig
    do_convolute:
      PUSH    ECX
      PUSH    EDI
      MOV     ECX,filtersize
      MOV     EDI,pbuf_filt
	  MOV     i_accum[0],0		//MOV     i_accum,0
      MOV     i_accum[4],0		// MOV     i_accum+4,0      //clear I sum reg before start
      MOV     q_accum[0],0		//MOV     q_accum,0
      MOV     q_accum[4],0		//MOV     q_accum+4,0      //clear Q sum reg before start
	  	        
    filter_loop:
      MOV     EAX,[EBX]   //I-sample processing
		IMUL    EDI				//IMUL	[EDI]
		ADD     i_accum[0],EAX		//ADD     i_accum,EAX
		ADC     i_accum[4],EDX		//ADC     i_accum+4,EDX
		MOV     EAX,[EBX+4]   //Q-sample processing
		IMUL    EDI		//		IMUL    [EDI+4]  
		ADD     q_accum[0], EAX		//ADD     q_accum,EAX
		ADC     q_accum[4],EDX		//ADC     q_accum+4,EDX
      CMP     EBX,end_of_buffer
      JGE     new_start_buffer
      ADD     EBX,8
      JMP     next_loop
    new_start_buffer:
      MOV     EBX,pbuf_sig
    next_loop:
      ADD     EDI,8
      LOOP    filter_loop

      POP     EDI
      POP     ECX

      MOV     EAX,i_accum+4   //I-sample amplitude shift
      SHL     EAX,8
	  MOV     AL,BYTE PTR i_accum[3]	// MOV     AL,BYTE PTR i_accum+3
      MOV     [EDI],EAX
      MOV     EAX,q_accum+4   //Q-sample amplitude shift
      SHL     EAX,8
       MOV     AL,BYTE PTR q_accum[3]	//MOV     AL,BYTE PTR q_accum+3
      MOV     [EDI+4],EAX

      ADD     EDI,8     //next pos in destnation buffer
      ADD     ESI,8     //next pos in source buffer
      DEC     ECX
      JZ      conv_done
      JMP     do_next_conv
    conv_done:
      MOV     pbuf_signal,EBX

    no_convolute:
  }
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

  asm
  {
      MOV     ECX,size
      TEST    ECX,-1
      JZ      no_convolute
                  //here to perform convolution
      MOV     EBX,pbuf_signal //EBX is pointer to data line
      MOV     ESI,src_ptr   //ESI is pointer to source data
      MOV     EDI,dest_ptr  //EDI is pointer to destination data
    next_conv:
      MOV     EAX,[ESI]
      MOV     [EBX],EAX   //move I-sample to signal line
      MOV     EAX,[ESI+4]
      MOV     [EBX+4],EAX   //move Q-sample to signal line
      CMP     EBX,end_of_buffer
      JGE     ret_start_buffer
      ADD     EBX,8       //next pos in signal buffer
      JMP     do_convolute
    ret_start_buffer:
      MOV     EBX,pbuf_sig
    do_convolute:
      PUSH    ECX
      PUSH    EDI
      MOV     ECX,filtersize
      MOV     EDI,pbuf_filt
      MOVQ    MM2,QWORD PTR dummy   //clear sum reg before start
    filter_loop:
      MOVQ    MM0,QWORD PTR [EBX]
      PSRAD   MM0,8
      PAND    MM0,QWORD PTR mask
      MOVQ    MM1,QWORD PTR [EDI]
      PSRAD   MM1,8
      PAND    MM1,QWORD PTR mask
      PMADDWD MM0,MM1
      PADDD   MM2,MM0
      CMP     EBX,end_of_buffer
      JGE     new_start_buffer
      ADD     EBX,8
      JMP     next_loop
    new_start_buffer:
      MOV     EBX,pbuf_sig
    next_loop:
      ADD     EDI,8
      LOOP    filter_loop
      POP     EDI
      POP     ECX

      PSRAD   MM2,8     //Shift result to 24 bit
      MOVQ    QWORD PTR [EDI],MM2
      ADD     EDI,8     //next pos in destnation buffer
      ADD     ESI,8     //next pos in source buffer
      LOOP    next_conv

      MOV     pbuf_signal,EBX
      EMMS

    no_convolute:
  }
  psignal_buf=pbuf_signal;
}


//---------------------------------------------------------------------------

void DSP001::SSEConvolute(int *src_ptr,int *dest_ptr,unsigned int size)
{
  float dummy[]={0,0,0,0};
  int *end_of_buffer=&psig_buf[(size_filter<<1)-2];
  int *pbuf_sig=psig_buf;
  int *pbuf_filt=pfilt_buf;
  int *pbuf_signal=psignal_buf;
  unsigned int filtersize=size_filter;

  asm
  {
      MOV     ECX,size
      TEST    ECX,-1
      JZ      no_convolute
                  //here to perform convolution
      MOV     EBX,pbuf_signal //EBX is pointer to data line
      MOV     ESI,src_ptr   //ESI is pointer to source data
      MOV     EDI,dest_ptr  //EDI is pointer to destination data
    next_conv:
      MOV     EAX,[ESI]
      MOV     [EBX],EAX   //move I-sample to signal line
      MOV     EAX,[ESI+4]
      MOV     [EBX+4],EAX   //move Q-sample to signal line
      CMP     EBX,end_of_buffer
      JGE     ret_start_buffer
      ADD     EBX,8       //next pos in signal buffer
      JMP     do_convolute
    ret_start_buffer:
      MOV     EBX,pbuf_sig
    do_convolute:
      PUSH    ECX
      PUSH    EDI
      MOV     ECX,filtersize
      MOV     EDI,pbuf_filt	
	  MOVUPS  XMM2,QWORD PTR dummy // 	  MOVUPS  XMM2,DQWORD PTR dummy   //clear sum reg before start
    filter_loop:
      CVTPI2PS XMM0,QWORD PTR [EBX]
      CVTPI2PS XMM1,QWORD PTR [EDI]
      MULPS   XMM0,XMM1
      ADDPS   XMM2,XMM0
      CMP     EBX,end_of_buffer
      JGE     new_start_buffer
      ADD     EBX,8
      JMP     next_loop
    new_start_buffer:
      MOV     EBX,pbuf_sig
    next_loop:
      ADD     EDI,8
      LOOP    filter_loop
      POP     EDI
      POP     ECX

      CVTPS2PI MM0,XMM2
      MOVQ    QWORD PTR [EDI],MM0
      ADD     EDI,8     //next pos in destnation buffer
      ADD     ESI,8     //next pos in source buffer
      LOOP    next_conv

      MOV     pbuf_signal,EBX
      EMMS

    no_convolute:
  }
  psignal_buf=pbuf_signal;
}


//---------------------------------------------------------------------------

void DSP001::MMXdemodSSB(int *src_ptr,int *dest_ptr,unsigned int size,unsigned int mode)
{
  int i_sample[2],q_sample[2];

  asm
  {
      MOV     ECX,size
      TEST    ECX,-1
      JZ      no_demod

      SHR     ECX,1     //two samples is processed in each loop
      MOV     ESI,src_ptr
      MOV     EDI,dest_ptr
    demod_loop:
      MOV     EAX,[ESI]
      MOV     i_sample[0],EAX	//MOV     i_sample,EAX
      MOV     EAX,[ESI+8]
      MOV     i_sample[4],EAX	//MOV     i_sample+4,EAX
      MOV     EAX,[ESI+4]
      MOV     q_sample[0],EAX	//MOV     q_sample,EAX
      MOV     EAX,[ESI+12]
      MOV     q_sample[4],EAX	//MOV     q_sample+4,EAX
      MOVQ    MM0,QWORD PTR i_sample
      PSRAD   MM0,1     //divide by two before summation
      MOVQ    MM1,QWORD PTR q_sample
      PSRAD   MM1,1
      test    mode,-1
      JNZ     do_sub
      PADDD   MM0,MM1
      JMP     sum_done
    do_sub:
      PSUBD   MM0,MM1
    sum_done:
      MOVQ    QWORD PTR [EDI],MM0
      ADD     ESI,16
      ADD     EDI,8
      LOOP    demod_loop

      EMMS
    no_demod:
  }
}

//---------------------------------------------------------------------------

void DSP001::MakeAudioSample(int *src_ptr,short *dest_ptr,unsigned int size,int gain, unsigned int decim)
{
  unsigned int byteoffset;

  byteoffset=4*decim;
  
  asm
  {
      MOV     EBX,size
      MOV     ECX,gain
      TEST    EBX,-1
      JZ      no_conv   //do nothing if size is zero
      MOV     ESI,src_ptr
      MOV     EDI,dest_ptr
      TEST    ECX,0x80000000  //see if sign bit is set anf gain is negative
      JZ      do_inc_gain  //if shift count positive increment gain
      NOT     ECX       //here if shift count negative increment gain
      INC     ECX       //negative shift count is now converted to positive
    do_dec_gain:         //here if decrement gain
      MOV     EAX,[ESI]
      SAR     EAX,CL
      MOV     [EDI],AX
      ADD     ESI,byteoffset
      ADD     EDI,2
      DEC     EBX
      JNZ     do_dec_gain
      JMP     no_conv
    do_inc_gain:
      MOV     EAX,[ESI]
      SAL     EAX,CL
      MOV     [EDI],AX
      ADD     ESI,byteoffset
      ADD     EDI,2
      DEC     EBX
      JNZ     do_inc_gain
    no_conv:
  }
}
