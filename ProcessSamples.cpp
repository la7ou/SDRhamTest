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

#include "portaudio.h"

#define _OSX_AU_DEBUG_ 0

//extern char audio_scratch[MAX_SAMPLES];
//extern unsigned int num_of_samples;

/* This routine will be called by the PortAudio engine when audio is needed.
 ** It may be called at interrupt level on some machines so don't do anything
 ** that could mess up the system like calling malloc() or free().
 */
int ProcessSamples::playCallback( const void *inputBuffer, void *outputBuffer,
						unsigned long framesPerBuffer,
						const PaStreamCallbackTimeInfo* timeInfo,
						PaStreamCallbackFlags statusFlags,
						void *userData )
{
    paTestData *data = (paTestData*)userData;
    SAMPLE *rptr = &data->recordedSamples[data->frameIndex * NUM_CHANNELS];
    SAMPLE *wptr = (SAMPLE*)outputBuffer;
    unsigned int i;
    int finished;
    unsigned int framesLeft = data->maxFrameIndex - data->frameIndex;
	
    (void) inputBuffer; /* Prevent unused variable warnings. */
    (void) timeInfo;
    (void) statusFlags;
    (void) userData;
	
    if( framesLeft < framesPerBuffer )
    {
        /* final buffer... */
        for( i=0; i<framesLeft; i++ )
        {
            *wptr++ = *rptr++;  /* left */
            if( NUM_CHANNELS == 2 ) *wptr++ = *rptr++;  /* right */
        }
        for( ; i<framesPerBuffer; i++ )
        {
            *wptr++ = 0;  /* left */
            if( NUM_CHANNELS == 2 ) *wptr++ = 0;  /* right */
        }
        data->frameIndex += framesLeft;
        finished = paComplete;
    }
    else
    {
        for( i=0; i<framesPerBuffer; i++ )
        {
            *wptr++ = *rptr++;  /* left */
            if( NUM_CHANNELS == 2 ) *wptr++ = *rptr++;  /* right */
        }
        data->frameIndex += framesPerBuffer;
        finished = paContinue;
    }
    return finished;
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
	err = paNoError;

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

	  
	  data.recordedSamples = paudio_scratch;

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

  
	  /* Playback recorded data.  -------------------------------------------- */
	  data.frameIndex = 0;
	  
	  err = Pa_Initialize();
	  if( err != paNoError ) goto done;
	  
	  outputParameters.device = Pa_GetDefaultOutputDevice(); /* default output device */
	  if (outputParameters.device == paNoDevice) {
		  fprintf(stderr,"Error: No default output device.\n");
		  goto done;
	  }
	  outputParameters.channelCount = 2;                     /* stereo output */
	  outputParameters.sampleFormat =  PA_SAMPLE_TYPE;
	  outputParameters.suggestedLatency = Pa_GetDeviceInfo( outputParameters.device )->defaultLowOutputLatency;
	  outputParameters.hostApiSpecificStreamInfo = NULL;
	  
	  printf("Begin playback.\n"); fflush(stdout);
	  err = Pa_OpenStream(
						  &stream,
						  NULL, /* no input */
						  &outputParameters,
						  SAMPLE_RATE,
						  FRAMES_PER_BUFFER,
						  paClipOff,      /* we won't output out of range samples so don't bother clipping them */
						  playCallback,
						  &data );
	  if( err != paNoError ) goto done;
	  
	  if( stream )
	  {
		  err = Pa_StartStream( stream );
		  if( err != paNoError ) goto done;
		  
		  printf("Waiting for playback to finish.\n"); fflush(stdout);
		  
		  while( ( err = Pa_IsStreamActive( stream ) ) == 1 ) Pa_Sleep(100);
		  if( err < 0 ) goto done;
		  
		  err = Pa_CloseStream( stream );
		  if( err != paNoError ) goto done;
		  
		  printf("Done.\n"); fflush(stdout);
	  }
	  
  done:
	  Pa_Terminate();



  } //while(run_flag)  
  

  sleep(NUM_OF_BUFFERS*PACKET_COUNT);


  delete[] psamp_buffer;
  delete[] pfilter_buffer;
  delete[] pdemod_buffer;
  //delete paudio_buffer;
  delete[] paudio_scratch;
  //delete[] pdisplay_buffer;

  //return 0;
  

	
}