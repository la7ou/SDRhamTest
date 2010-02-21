/*
 *  Readsamples.h
 *  SDR_ham_ez_xcode
 *
 *  Created by Stein Erik Ellevseth on 10.10.08.
 *  Copyright 2008 __MyCompanyName__. All rights reserved.
 *
 */
#ifndef READSAMPLES_H
#define READSAMPLES_H

#include <QObject>
#include <QDialog>
#include <QFileDialog>
#include <QFile>  
#include <QString> 

#include <QtGui>
 
class ReadSamples : public QThread 
{
	Q_OBJECT
	
private:
		const char *filnavn;//temporarily
	
	unsigned int file_status,j,n,index,sample,sample_count,file_index;//,*fir_coeff;
    unsigned long num_bytes;
	int err;
	
	//static 
	unsigned int frame_count;

    QString LineBuff,FileName;// AnsiString LineBuff,FileName;
	QFile file;
	
	QStringList fileNames;	
    QString fName;
    QFileDialog *fileDialog;
	
public:
		ReadSamples (QObject *parent);
		void run(); // this is virtual method, we must implement it in our subclass of QThread
		
public slots:
		void ReadSamplesFromFE();

 };
 #endif