/*
 *  ProcessSamples.cpp
 *  SDR_ham_ez_xcode
 * http://en.wikipedia.org/wiki/Audio_Units
 * http://developer.apple.com/audio/
 * http://www.gnuradio.org/trac/browser/gnuradio/trunk/gr-audio-osx/src/audio_osx_source.cc
 * http://developer.apple.com/documentation/MusicAudio/Conceptual/AudioUnitProgrammingGuide/TheAudioUnit/chapter_4_section_2.html
 * Technical Note TN2097
 * Playing a sound file using the Default Output Audio Unit :
 * http://developer.apple.com/technotes/tn2004/tn2097.html
 * aiff, Apple interchange fiel format
 * 
 * http://en.wikipedia.org/wiki/Aiff
 * Future :
 * Generere mp3 filer for Quicktime
 *
 *  Created by Stein Erik Ellevseth on 11.10.08.
 *  Copyright 2008 __MyCompanyName__. All rights reserved.
 *
 */


#include <QObject> 
#include <QMessageBox>
#include <QDialog>
#include <QString> // Istedet for Ansistring

#include "ezload.h"
#include "ProcessSamples.h"
#include "LoadSamples.h" // rx_buff, MAX_SAMPLES_FRAME, frame_count

//#include "window.h"  // modeType

#include <stdio.h>
#include <stdlib.h>

#include <iostream> // cout

//#include <QSound>
//#include <alsa/asoundlib.h>

#include  <math.h>

#include <dsp001.h>  // AM, SSB ... fra halvor
#include <dsp002.h>  // FM, ...
#include <dsppsk31.h>  // PSK, ...

#include <CoreServices/CoreServices.h>
#include <AudioUnit/AudioUnit.h> // AudioUnit
#include <AudioToolbox/AudioToolbox.h>
#include <CoreAudio/CoreAudioTypes.h>  // AudioStreamBasicDescription

#define _OSX_AU_DEBUG_ 0

//extern char audio_scratch[MAX_SAMPLES];
//extern unsigned int num_of_samples;


OSStatus ProcessSamples::AUOutputCallback
(void *inRefCon, 
 AudioUnitRenderActionFlags *ioActionFlags, 
 const AudioTimeStamp *inTimeStamp, 
 UInt32 inBusNumber, 
 UInt32 inNumberFrames, 
 AudioBufferList *ioData)
 /*
 Parameters
ci
The audio unit to be changed.

ioActionFlags
Flags that provide information on the render; see “Render Action Flags”.

inTimeStamp
The time the render is begun.

inOutputBusNumber
The bus on which the output will be placed.

inNumberFrames
The number of frames to be rendered.

ioData
The audio data, before and after the render.
 **!
	@struct			AURenderCallbackStruct
	@abstract		Used by a host when registering a callback with the audio unit to provide input
***
typedef struct AURenderCallbackStruct {
	AURenderCallback			inputProc;
	void *						inputProcRefCon;
} AURenderCallbackStruct;
 
 
 *********************************************************
    @struct         AudioBuffer
    @abstract       A structure to hold a buffer of audio data.
    @field          mNumberChannels
                        The number of interleaved channels in the buffer.
    @field          mDataByteSize
                        The number of bytes in the buffer pointed at by mData.
    @field          mData
                        A pointer to the buffer of audio data.
**
struct AudioBuffer
{
    UInt32  mNumberChannels;
    UInt32  mDataByteSize;
    void*   mData;
};
typedef struct AudioBuffer  AudioBuffer;

// **!
    @struct         AudioBufferList
    @abstract       A variable length array of AudioBuffer structures.
    @field          mNumberBuffers
                        The number of AudioBuffers in the mBuffers array.
    @field          mBuffers
                        A variable length array of AudioBuffers.
**
struct AudioBufferList
{
    UInt32      mNumberBuffers;
    AudioBuffer mBuffers[kVariableLengthArray];
};
typedef struct AudioBufferList  AudioBufferList;

*!
    @struct         AudioBuffer
    @abstract       A structure to hold a buffer of audio data.
    @field          mNumberChannels
                        The number of interleaved channels in the buffer.
    @field          mDataByteSize
                        The number of bytes in the buffer pointed at by mData.
    @field          mData
                        A pointer to the buffer of audio data.
*

*******************************************************************************/
 
{
  ProcessSamples* This = (ProcessSamples*) inRefCon;
  OSStatus err = noErr;
  
  ioActionFlags=0;
  inBusNumber=1;
  inTimeStamp=NULL;
  inNumberFrames=This->lSamptr->sample_count;

//-->  This->d_internal->lock ();

#if _OSX_AU_DEBUG_
  fprintf (stderr, "cb_in: SC = %4ld, in#F = %4ld\n",
	This->lSamptr->sample_count, inNumberFrames);
	   //This->d_queueSampleCount, inNumberFrames);
#endif
		AudioBufferList fillBufList;
		fillBufList.mNumberBuffers = 8;//#define NUM_OF_BUFFERS    8
		fillBufList.mBuffers[0].mNumberChannels = 1;
		fillBufList.mBuffers[0].mDataByteSize = This->audio_index<<1;
		//we're just going to copy the data into each channel
		This->audio_index=0;
	for (UInt32 channel = 1; channel < ioData->mNumberBuffers; channel++)
	{
		fillBufList.mBuffers[0].mData = &This->paudio_scratch[This->audio_index];
		ioData = &fillBufList;
		memcpy (ioData->mBuffers[channel].mData, ioData->mBuffers[0].mData, ioData->mBuffers[0].mDataByteSize);

		This->audio_index=This->audio_index+128;
    }
	
  if (  This->lSamptr->sample_count < inNumberFrames) {
// not enough data to fill request
    err = -1;
  } else {
// enough data; remove data from our buffers into the AU's buffers
    int l_counter = This->nChannels;		//d_n_channels;

    while (--l_counter >= 0) {
      UInt32 t_n_output_items = inNumberFrames;
 //     float* outBuffer = (float*) ioData->mBuffers[l_counter].mData;
	  
//-->      This->d_buffers[l_counter]->dequeue (outBuffer, &t_n_output_items);
      if (t_n_output_items != inNumberFrames) {
/*-->	throw std::runtime_error ("audio_osx_sink::AUOutputCallback(): "
				  "number of available items changing "
				  "unexpectedly.\n");*/
      }
    }

	This->lSamptr->sample_count -= inNumberFrames;
  }

#if _OSX_AU_DEBUG_
  fprintf (stderr, "cb_out: SC = %4ld\n", This->sample_count);  //d_queueSampleCount);
#endif

// signal that data is available
//-->  This->d_cond_data->signal ();

// release control to allow for other processing parts to run
//-->  This->d_internal->unlock ();

  return (err);
}





  void ProcessSamples::run()
 {
 
	double in_sample_rate,decim_rate,fracpart;
	run_flag = true;
	std::cout << " run_flag  = " <<  run_flag << std::endl;

/*
  switch(samplesource)
  {
    case 0:   //USB is source
      PacketSize=SubUSB->USBGetPacketSize();
      MaxSamplesFrame=(PacketSize/SAMPLE_SIZE);
      in_sample_rate=(double)SetupSDRForm->GetSampFreq()/(double)SetupSDRForm->GetDecimation();
      break;

    case 1:   //File is source
*/			
      PacketSize=MAX_SAMPLES_FRAME*SAMPLE_SIZE;
      MaxSamplesFrame=MAX_SAMPLES_FRAME;
      in_sample_rate=8000;//FILE_AUDIO_RATE;//FILE_AUDIO_RATE fra (double)SourceForm->GetFileSampleRate();
      filedelay=(lSamptr->frame_count*MAX_SAMPLES_FRAME*1000)/in_sample_rate;
	  	printf("File delay: %d",(unsigned int)filedelay);
/*
		break;
  }
*/

	 pdisplay_buffer=new int[MAX_SAMPLES]; //display buffer

	wout_sample_rate=8000;//44100.0;	// AudioProp->GetAudioSampleRate();
	fracpart=modf((in_sample_rate/(double)wout_sample_rate),&decim_rate);
	//decim_rate = 5; //test
	decim=(unsigned int)decim_rate;
	if(fracpart>UP_FRACTION)
		decim++;

	
	run_flag = true;

	ProcessSamplesSet(); 
	
	printf("Sample Rate from Source: %d Hz   Audio Decimation Rate: %d",(unsigned int)in_sample_rate,decim);

	qDebug() << "Executing ProcessSamples in new independant thread, GUI is NOT blocked";
	exec();
}


ProcessSamples:: ProcessSamples(QObject *parent, LoadSamples *lsam)
: QThread(parent)
{

		pDSP001=new DSP001();
		pDSP002=new DSP002();
		pDSPpsk31=new DSPPSK31();
		// InitializeCriticalSection(&cs_0);
		// parent class pointer 
		lSamptr = lsam;

	OSStatus err = noErr;

	// Open the default output unit
	ComponentDescription desc;
	desc.componentType = kAudioUnitType_Output;
	desc.componentSubType = kAudioUnitSubType_DefaultOutput;
	desc.componentManufacturer = kAudioUnitManufacturer_Apple;
	desc.componentFlags = 0;
	desc.componentFlagsMask = 0;
	
	Component comp = FindNextComponent(NULL, &desc);
	if (comp == NULL) { printf ("FindNextComponent\n"); return; }
	
	err = OpenAComponent(comp, &gOutputUnit);
	if (comp == NULL) { printf ("OpenAComponent=%ld\n", err); return; }

}

void ProcessSamples:: ProcessSamplesSet()
{	
	
OSStatus err = noErr;	

	psamp_buffer=new int[2*MaxSamplesFrame]; //makr room for I- and Q-samples
	pfilter_buffer=new int[2*MaxSamplesFrame]; //makr room for I- and Q-samples
	pdemod_buffer=new int[MaxSamplesFrame]; //demodlated samples
	//paudio_buffer=new audio_buffer; //audio buffer for windows
	paudio_scratch=new short[MAX_SAMPLES]; //audio scratch buffer

  waveFreeBlockCount=NUM_OF_BUFFERS;
  waveCurrentBlock=0;
  
 nChannels=1;
 
if_gain=0;
	
		printf (" mode = %d\n", lSamptr->mode);

// CoreAudio/CoreAudioTypes.h :
	// We tell the Output Unit what format we're going to supply data to it
	// this is necessary if you're providing data through an input callback
	// AND you want the DefaultOutputUnit to do any format conversions
	// necessary from your format to the device's format.
  waveFormat.mSampleRate=wout_sample_rate; // nSamplesPerSec=wout_sample_rate;
  waveFormat.mFormatID=kAudioFormatLinearPCM;//wFormatTag=WAVE_FORMAT_PCM;
 // waveFormat.mFormatFlags=kAudioFormatFlagIsBigEndian; //<==
  waveFormat.mFormatFlags=kLinearPCMFormatFlagIsSignedInteger 
								| kAudioFormatFlagsNativeEndian
								| kLinearPCMFormatFlagIsPacked
								| kAudioFormatFlagIsNonInterleaved;
  waveFormat.mBytesPerPacket=2;//4;//8;
  waveFormat.mFramesPerPacket=1; // In uncompressed audio, a Packet is one frame
  waveFormat.mBytesPerFrame=2;//4;//gr_audio_osx 32;//?? //nAvgBytesPerSec=waveFormat.nSamplesPerSec*waveFormat.nBlockAlign;
  waveFormat.mChannelsPerFrame=nChannels;
  waveFormat.mBitsPerChannel=16;//32?//wBitsPerSample=16;
  waveFormat.mReserved=0; //cbSize=0;
  //waveFormat.nBlockAlign=waveFormat.nChannels*(waveFormat.wBitsPerSample/8);

	  

//  Hentet fra DefaultOutputUnits
//#if _OSX_AU_DEBUG_
	printf("\nRendering source:\n\t");
	printf ("SampleRate=%f\n\t", waveFormat.mSampleRate);
	printf ("BytesPerPacket=%ld\n\t", waveFormat.mBytesPerPacket);
	printf ("FramesPerPacket=%ld\n\t", waveFormat.mFramesPerPacket);
	printf ("BytesPerFrame=%ld\n\t", waveFormat.mBytesPerFrame);
	printf ("BitsPerChannel=%ld\n\t", waveFormat.mBitsPerChannel);
	printf ("ChannelsPerFrame=%ld\n", waveFormat.mChannelsPerFrame);
	
//#endif


//Set the stream format of the output to match the input
//result = 
err = AudioUnitSetProperty (gOutputUnit,			//*theUnit,
                            kAudioUnitProperty_StreamFormat,
                            kAudioUnitScope_Input,
                            0,
                            &waveFormat,
							sizeof(AudioStreamBasicDescription));
 
if (err) { 
	printf ("AudioUnitSetProperty-SF=%4.4s, %ld\n", (char*)&err, err); 
	return; 
}

	    // Initialize unit
err = AudioUnitInitialize(gOutputUnit);
if (err) { 
	printf ("AudioUnitInitialize=%ld\n", err); 
	return;
	}


	//Float64 outSampleRate;
	UInt32 size = sizeof(Float64);
	err = AudioUnitGetProperty (gOutputUnit,
							kAudioUnitProperty_SampleRate,
							kAudioUnitScope_Output,
							0,	
							&outSampleRate,
							&size);
	if (err) { 
		printf ("AudioUnitGetProperty-GF=%4.4s, %ld\n", (char*)&err, err); 
		return;
	}
 
	
	printf("\nProcess Parameters : \n\t");
	printf ("PacketSize( =MAX_SAMPLES_FRAME*SAMPLE_SIZE) = %d\n\t", PacketSize);
	printf ("MaxSamplesFrame (=MAX_SAMPLES_FRAME)= %d\n\t", MaxSamplesFrame);
	printf ("frames<lSamptr->frame_count)= %d\n\t", frames<lSamptr->frame_count);
	printf("num_of_samples=%d\n\t", num_of_samples);
	printf ("if_gain = %i\n\t", if_gain);
	printf ("decim = %i\n\t", decim);
	printf (" filedelay (=(lSamptr->frame_count*MAX_SAMPLES_FRAME*1000)/in_sample_rate) =%d\n", filedelay);

  num_of_samples=MaxSamplesFrame/decim;

  while(run_flag)
  {
//    if(WaitForSingleObject(hProcessEvent,1000)==WAIT_OBJECT_0)
//    {
      disp_index=0;
      audio_index=0;

      for(frames=0,j=0; frames < lSamptr->frame_count; frames++,j=j+PacketSize)
      {

       pDSP001->B2Lendian(&lSamptr->rx_buff[j],psamp_buffer,MaxSamplesFrame);

        switch(lSamptr->mode)
        {
          case 0: // AM
            pDSP002->FPdemodAM(psamp_buffer,pdemod_buffer,MaxSamplesFrame);
            pDSP001->MakeAudioSample(pdemod_buffer,&paudio_scratch[audio_index],num_of_samples,if_gain,decim);
            audio_index=audio_index+num_of_samples;
            break;

          case 1: // FM
            pDSP002->FPdemodFM(psamp_buffer,pdemod_buffer,MaxSamplesFrame);
            for(unsigned int m=0,k=0; m<MaxSamplesFrame; m=m+1,k=k+2)
              psamp_buffer[k]=pdemod_buffer[m];
            pDSP001->GPRConvolute(psamp_buffer,pfilter_buffer,MaxSamplesFrame);
            for(unsigned int m=0,k=0; m<MaxSamplesFrame; m=m+1,k=k+2)
              pdemod_buffer[m]=pfilter_buffer[k];
            pDSP001->MakeAudioSample(pdemod_buffer,&paudio_scratch[audio_index],num_of_samples,-8,decim);
            audio_index=audio_index+num_of_samples;
            break;

          case 2: //USB
            pDSP001->GPRConvolute(psamp_buffer,pfilter_buffer,MaxSamplesFrame);
            pDSP001->MMXdemodSSB(pfilter_buffer,pdemod_buffer,MaxSamplesFrame,0);
            pDSP001->MakeAudioSample(pdemod_buffer,&paudio_scratch[audio_index],num_of_samples,if_gain,decim);
            audio_index=audio_index+num_of_samples;
            break;

          case 3: //LSB
            pDSP001->GPRConvolute(psamp_buffer,pfilter_buffer,MaxSamplesFrame);
            pDSP001->MMXdemodSSB(pfilter_buffer,pdemod_buffer,MaxSamplesFrame,1);
            pDSP001->MakeAudioSample(pdemod_buffer,&paudio_scratch[audio_index],num_of_samples,if_gain,decim);
            audio_index=audio_index+num_of_samples;
            break;

          case 4: //PSK31
            break;
        } //switch(mode)
			//printf (" &paudio_scratch[%d] = %d,\n",audio_index, &paudio_scratch[audio_index]);
				qDebug() << "&paudio_scratch["<< audio_index << "] = "<<  &paudio_scratch[audio_index] << endl;

        for(index=0; index<MaxSamplesFrame; index++)
        {
          pdisplay_buffer[disp_index]=pdemod_buffer[index];
          disp_index++;
        }
		
		
      } // end of      for(frames=0,j=0; frames<frame_count; frames++,j=j+PacketSize)

//    } // end of //    if(WaitForSingleObject(hProcessEvent,1000)==WAIT_OBJECT_0)


    switch(waveFreeBlockCount)
    {
      case 0:   //do nothing if no free blocks
        fill_buff=0;
        break;

      case (NUM_OF_BUFFERS): //if buffers empty fill two
        fill_buff=(NUM_OF_BUFFERS/2);
        break;

      case (NUM_OF_BUFFERS-1): //if buffers empty fill two
        fill_buff=2;
        break;

      default:  //fill one buffer
        fill_buff=1;
        break;
    } // end of switch(waveFreeBlockCount)
	
	printf("\nProcessed : \n\t");
	printf ("audio_index = %d\n\t", audio_index);
	printf ("disp_index =  %d\n\t", disp_index);
	printf ("lSamptr->frame_count = %d\n\t", lSamptr->frame_count);
	printf("num_of_samples=%d\n\t", num_of_samples);
	printf ("if_gain = %d\n\t", if_gain);
	printf ("decim = %d\n", decim);


/*
    for(; fill_buff; fill_buff--)
    {
	
	
//      if(waveHeader[waveCurrentBlock].dwFlags&WHDR_PREPARED)
//        waveOutUnprepareHeader(outHandle,&waveHeader[waveCurrentBlock],sizeof(WAVEHDR));
		
      for(index=0; index<audio_index; index++)
          paudio_buffer->buffer[waveCurrentBlock][index]=paudio_scratch[index]; 		
      waveHeader[waveCurrentBlock].lpData=(char*)&paudio_buffer->buffer[waveCurrentBlock][0];
      waveHeader[waveCurrentBlock].dwBufferLength=(audio_index<<1); //length in bytes
      waveHeader[waveCurrentBlock].dwUser=0;
      waveHeader[waveCurrentBlock].dwLoops=0;
      waveHeader[waveCurrentBlock].dwFlags=0;
      waveOutPrepareHeader(outHandle,&waveHeader[waveCurrentBlock],sizeof(WAVEHDR));
      waveOutWrite(outHandle,&waveHeader[waveCurrentBlock],sizeof(WAVEHDR));  //sends a data block to the given waveform-audio output device.

      //InterlockedDecrement(&waveFreeBlockCount); // Use QMutex istedet?
      waveCurrentBlock++;
      if(waveCurrentBlock==NUM_OF_BUFFERS)
        waveCurrentBlock=0;

    } // end of  for(; fill_buff; fill_buff--)
	
*/

	// Set up a callback function to generate output to the output unit
    AURenderCallbackStruct input;	
//	input.inputProc = MyRenderer;
	 input.inputProc = (AURenderCallback)(AUOutputCallback);
	input.inputProcRefCon = this;//NULL; // this

	err = AudioUnitSetProperty (gOutputUnit, 
								kAudioUnitProperty_SetRenderCallback,
								kAudioUnitScope_Input,
								0, 
								&input, 
								sizeof(input));

	if (err) { 
	
		printf ("AudioUnitSetProperty-CB=%ld\n", err);
		return;
	}

	
	// Start the rendering
	// The DefaultOutputUnit will do any format conversions to the format of the default device
	err = AudioOutputUnitStart (gOutputUnit);
	if (err) { 
		printf ("AudioOutputUnitStart=%ld\n", err); 
		return;
	}
			
	// we call the CFRunLoopRunInMode to service any notifications that the audio
	// system has to deal with
	CFRunLoopRunInMode(kCFRunLoopDefaultMode, 2, false);


  } //while(run_flag)  
  

  sleep(NUM_OF_BUFFERS*PACKET_COUNT);

/*
  for(unsigned i=0; i<NUM_OF_BUFFERS; i++)
  {
    if(waveHeader[i].dwFlags&WHDR_PREPARED)
      waveOutUnprepareHeader(outHandle,&waveHeader[i],sizeof(WAVEHDR));
  } // end of for(unsigned i=0; i<NUM_OF_BUFFERS; i++)

  if(wave_result==MMSYSERR_NOERROR)
  {
    waveOutReset(outHandle);
    waveOutClose(outHandle);
  } // end of if(wave_result==MMSYSERR_NOERROR)
*/

// REALLY after you're finished playing STOP THE AUDIO OUTPUT UNIT!!!!!!	
// but we never get here because we're running until the process is nuked...	
	verify_noerr (AudioOutputUnitStop (gOutputUnit));
	
    err = AudioUnitUninitialize (gOutputUnit);
	if (err) { 
		printf ("AudioUnitUninitialize=%ld\n", err); 
		return;
	}


  delete[] psamp_buffer;
  delete[] pfilter_buffer;
  delete[] pdemod_buffer;
  //delete paudio_buffer;
  delete[] paudio_scratch;
  //delete[] pdisplay_buffer;

  //return 0;
  
  	// Clean up
	CloseComponent (gOutputUnit);
	
}
