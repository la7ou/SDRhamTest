/*
 *  ProcessSamples.h
 *  SDR_ham_ez_xcode
 *
 *  Created by Stein Erik Ellevseth on 11.10.08.
 *  Copyright 2008 __MyCompanyName__. All rights reserved.
 *
 */

#ifndef PROCESSSAMPLES_H
#define PROCESSSAMPLES_H

#include <QObject>
#include <QDialog>
#include <QMessageBox>
#include <LoadSamples.h>  // MAX_SAMPLES, rx_buff

#include <QSound>

#include <QtGui>

//#include <alsa/asoundlib.h> // sjekk http://lists.trolltech.com/qt-interest/2007-01/thread00147-0.html
//#include <AudioFile.h> // http://developer.apple.com/documentation/MusicAudio/Conceptual/CoreAudioOverview/CoreAudioFrameworks/chapter_950_section_2.html#//apple_ref/doc/uid/TP40003577-CH9-DontLinkElementID_7
// http://developer.apple.com/audio/ //AudioToolbox og AudioUnit Frameworks
//#include <CoreAudio/CoreAudioTypes.h>  // AudioStreamBasicDescription

//#include <CoreServices/CoreServices.h>
//#include <AudioUnit/AudioUnit.h> // AudioUnit

#include "portaudio.h"


#include <dsp001.h>  // AM, SSB ... fra halvor
#include <dsp002.h>  // FM, ...
#include <dsppsk31.h>  // PSK, ...

#define NUM_OF_BUFFERS    8   //number of audio buffers

// fra systemc.h
#define PACKET_COUNT      32
#define UP_FRACTION       0.5

/* #define SAMPLE_RATE  (17932) // Test failure to open with this value. */
#define SAMPLE_RATE  (44100)
#define FRAMES_PER_BUFFER (1024)
#define NUM_SECONDS     (5)
#define NUM_CHANNELS    (2)
/* #define DITHER_FLAG     (paDitherOff) */
#define DITHER_FLAG     (0) /**/

#define noErr 0

#ifndef M_PI
#define M_PI  (3.14159265)
#endif

#define TABLE_SIZE   (1024)

/* Select sample format. */
#if 0
#define PA_SAMPLE_TYPE  paFloat32
typedef float SAMPLE;
#define SAMPLE_SILENCE  (0.0f)
#define PRINTF_S_FORMAT "%.8f"
#elif 1
#define PA_SAMPLE_TYPE  paInt16
typedef short SAMPLE;
#define SAMPLE_SILENCE  (0)
#define PRINTF_S_FORMAT "%d"
#elif 0
#define PA_SAMPLE_TYPE  paInt8
typedef char SAMPLE;
#define SAMPLE_SILENCE  (0)
#define PRINTF_S_FORMAT "%d"
#else
#define PA_SAMPLE_TYPE  paUInt8
typedef unsigned char SAMPLE;
#define SAMPLE_SILENCE  (128)
#define PRINTF_S_FORMAT "%d"
#endif

class ProcessSamples : public QThread
 {	Q_OBJECT
	private:
 
	 typedef struct
	 {
		 //short sine[TABLE_SIZE];//float sine[TABLE_SIZE];
		 
		 //short *sine;
		 int left_phase;
		 int right_phase;
		 unsigned int frameIndex;  // Index into sample array. 
		 int maxFrameIndex;
		 //SAMPLE      *recordedSamples;
		 short      *recordedSamples;
	 }
	 paTestData;
 
	 // portaudio
	 PaStreamParameters	 outputParameters;
	 PaStream*           stream;
	 PaError             err;// = paNoError;
	 paTestData          data;

	// fra main.cpp ProcessSamples :
	unsigned int frames,j,n,index,disp_index,audio_index,fill_buff;
	unsigned int wave_result,waveCurrentBlock;
	 double in_sample_rate,decim_rate,fracpart;
	 
	 int sample,*psamp_buffer, *pdemod_buffer;//,*pfilter_buffer,*pdemod_buffer;
	 float *pfl_buffer, *pfilter_buffer, *pampl_buffer;
	short *paudio_scratch;
	char audio_scratch[MAX_SAMPLES];
	unsigned int  num_of_samples;
	int nChannels;
	// audio_buffer *paudio_buffer;


// Alsa player?

    volatile long waveFreeBlockCount;
	
	// fra main.h
	//static i Halvors kode gir linkeproblemer i Qt koden:
	unsigned int samplesource, wout_sample_rate;
	unsigned int decim, filedelay;
	//	
	//	double decim_rate,fracpart;
	
	// fra Halvors main.cpp
	unsigned int PacketSize, MaxSamplesFrame;
	
	LoadSamples *lSamptr;
	
	 static int playCallback( const void *inputBuffer, void *outputBuffer,
							 unsigned long framesPerBuffer,
							 const PaStreamCallbackTimeInfo* timeInfo,
							 PaStreamCallbackFlags statusFlags,
							 void *userData );
//#ifdef LINUX	
	typedef double Float64;				
//#endif
		Float64 outSampleRate;
	
	public:
		ProcessSamples(QObject *parent, LoadSamples *lsam);
		DSP001 *pDSP001;
		DSP002 *pDSP002;
		DSPPSK31 *pDSPpsk31;
		
		//enum modeType {am,fm,usb,lsb,psk31 }; // ProcessSamples
		unsigned int run_flag;
		int *pdisplay_buffer;
//		int mode;//modeType mode;
		int if_gain;
		
	public slots:
		void ProcessSamplesSet();
		void run(); // this is virtual method, we must implement it in our subclass of QThread

 };
 #endif
