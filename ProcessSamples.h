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
#include <CoreAudio/CoreAudioTypes.h>  // AudioStreamBasicDescription

#include <CoreServices/CoreServices.h>
#include <AudioUnit/AudioUnit.h> // AudioUnit


#include <dsp001.h>  // AM, SSB ... fra halvor
#include <dsp002.h>  // FM, ...
#include <dsppsk31.h>  // PSK, ...

#define NUM_OF_BUFFERS    8   //number of audio buffers

// fra systemc.h
#define PACKET_COUNT      32
#define UP_FRACTION       0.5


class ProcessSamples : public QThread
 {	Q_OBJECT
	private:

	// fra main.cpp ProcessSamples :
	unsigned int frames,j,n,index,disp_index,audio_index,fill_buff;
	unsigned int wave_result,waveCurrentBlock;

	int sample,*psamp_buffer,*pfilter_buffer,*pdemod_buffer;
	short *paudio_scratch;
	char audio_scratch[MAX_SAMPLES];
	unsigned int  num_of_samples;
	int nChannels;

  AudioStreamBasicDescription  waveFormat;
  AudioUnit	gOutputUnit;


// Alsa player?

    volatile long waveFreeBlockCount;
	
	// fra main.h
	//static i Halvors kode gir linkeproblemer i Qt koden:
	unsigned int samplesource, wout_sample_rate;
	unsigned int decim, filedelay;
	int if_gain;
	
	// fra Halvors main.cpp
	unsigned int PacketSize, MaxSamplesFrame;
	
	LoadSamples *lSamptr;
	
  
	// GnuRadio gr_audio_osx-sink.cc
	static OSStatus AUOutputCallback (void *inRefCon, 
				    AudioUnitRenderActionFlags *ioActionFlags, 
				    const AudioTimeStamp *inTimeStamp, 
				    UInt32 inBusNumber, 
				    UInt32 inNumberFrames, 
				    AudioBufferList *ioData);
					
		Float64 outSampleRate;

	
	public:
		ProcessSamples(QObject *parent, LoadSamples *lsam);
		DSP001 *pDSP001;
		DSP002 *pDSP002;
		DSPPSK31 *pDSPpsk31;
		
		enum modeType {am,fm,usb,lsb,psk31 }; // ProcessSamples
		unsigned int run_flag;
			int *pdisplay_buffer;
//				int mode;//modeType mode;
		
	public slots:
		void ProcessSamplesSet();
		void run(); // this is virtual method, we must implement it in our subclass of QThread

 };
 #endif