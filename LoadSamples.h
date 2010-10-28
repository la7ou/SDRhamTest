/*
 *  LoadSamples.h
 *  SDR_ham_ez_xcode
 *
 *  Created by Stein Erik Ellevseth on 11.10.08.
 *  Copyright 2008 __MyCompanyName__. All rights reserved.
 *
 */

#ifndef LOADSAMPLES_H
#define LOADSAMPLES_H

#include <QObject>
#include <QDialog>
#include <QFileDialog>  
#include <QFile> 

#define MAX_SAMPLES       16384  //Maximum samples in receive buffer
#define MAX_SAMPLES_FRAME 128
#define SAMPLE_SIZE       6
#define FILE_AUDIO_RATE   48000
 
class LoadSamples : public QDialog
 {	Q_OBJECT
 
 	public:
	//enum modeType {am,fm,usb,lsb,psk31 }; // ProcessSamples
	 int mode;//modeType mode;
	 
		unsigned int sample_count;
		//static 
		unsigned int frame_count; // benyttes i ProcessSamples
		unsigned char* rx_buff;// [MAX_SAMPLES*SAMPLE_SIZE];
		LoadSamples();
 
	private:
		const char *filnavnSamples;//temporarily
		unsigned int sample,file_index;//,*fir_coeff;
		QFile file;
	
		QStringList fileNames;	
		QString fName;
		QFileDialog *fileDialog;

	public slots:
		int LoadSamplesFile();

 };
 #endif