/*
 *  LoadSamples.cpp
 *  SDR_ham_ez_xcode
 *
 *  Created by Stein Erik Ellevseth on 11.10.08.
 *  Copyright 2008 __MyCompanyName__. All rights reserved.
 *
 */

#include <QTextStream>

#include <QObject> 
#include <QMessageBox>
#include <QDialog>
#include <QFileDialog>
#include <QFile>
#include <QString> // Istedet for Ansistring
#include "ezload.h"
#include "LoadSamples.h"
#include <stdio.h>
#include <stdlib.h>
#include <iostream> // cout

LoadSamples:: LoadSamples()
{
//    QMessageBox::warning(this, "LoadSamples:: LoadSamples()",
//    "class LoadSamples is created.");
	
	//rx_buff = NULL;
	//(unsigned char*) rx_buff[MAX_SAMPLES*SAMPLE_SIZE]=new unsigned char [MAX_SAMPLES*SAMPLE_SIZE];
	rx_buff=new unsigned char [MAX_SAMPLES*SAMPLE_SIZE];
}

int LoadSamples:: LoadSamplesFile()
{
//    QMessageBox::warning(this, "LoadSamples:: LoadSamples()",
//    "This feature is partly implemented.");
	
//		static unsigned int frame_count;
	unsigned char temp;
    int err = kSuccess;
    fileDialog = new QFileDialog(this);
    fileDialog->setFilter(tr( "Images (*.dat)"));
    fileDialog->setFileMode(QFileDialog::ExistingFile);
	fileDialog->setDirectory("../../../../../");
    fileDialog->show();
    if (fileDialog->exec()) fileNames = fileDialog->selectedFiles();
    if (!fileNames.isEmpty()) fName = fileNames[0];
	filnavnSamples = fName.toUtf8();
	//QString filnavnSamples = fName.toUtf8();
    QFile file(filnavnSamples);//
	//QMessageBox::information(this, "LoadSamples:: LoadSamplesFile() : samples File:",filnavnSamples);
	//QFile file("/Users/steinerik/Documents/SDRham/am_samples.dat");
	//QFile file("am_samples.dat");
	//file.setFileName("/Users/steinerik/Documents/SDRham/am_samples.dat");
	//file.setFileName(filnavn);
	//printf ("filnavn : %s\n\t", file.fileName());
	
    if (!file.open(QFile::ReadOnly | QFile::Text))
    {
        QMessageBox::warning(this, "LoadSamples:: LoadSamplesFile() : samples File:",filnavnSamples);
        return kFailure;
    }

      sample_count=0;
      file_index=0;
	     while (!file.atEnd())
		  {

			QByteArray line = file.readLine(40); // Read a line
			//	printf ("line  %d = %s\n\t", sample_count, line.data());
			QByteArray ba = line.trimmed(); // Remove obsolete spaces, \r and \n
			//    printf ("line trimmed %d = %s\n\t", sample_count, line.data());
			QByteArray I_sample, Q_sample;
			for (unsigned int n=0; ba[n]!=' ';n++)  // finner spacen mellom de to samplene
			{
				I_sample = ba.left(n+1);		// extract I sample part from line
				Q_sample = ba.right(n+1);	// extract Q sample part from line
			}
			//  printf ("I_sample = %s, Q_sample = %s\n\t", I_sample.data(), Q_sample.data());
			
			/*
			 Samplene som leses fra filen som 32-bit integer  i Little Endian format 
			 (Intel Pentium prosessor bruker Little Endian format). 
			 Samplene fra AD6620 kommer som 24-bit i Big Endian format. 
			 Skiftingen av bits konverterer 32-bits Little Endian format til 24-bit Big Endian format 
			 slik at samplene i receivebufferet rx_buff,
			 som er input til ProcessSample() funksjon, ser like ut enten de kommer fra AD6620 eller fra fil.
			*/  
			sample= atoi(I_sample); // I sample convert to integer	
			//  printf ("int I_sample = %d (0x%x)\n\t", sample, sample);
            // konverterer I sample til Big endian :
			  rx_buff[file_index]=(unsigned char)sample;
			  file_index++;
			  rx_buff[file_index]=(unsigned char)(sample>>8);
			  file_index++;
			  rx_buff[file_index]= (unsigned char)(sample>>16);
			  file_index++;


			
			sample= atoi(Q_sample); // convert Q_sample to integer
			//  printf ("int Q_sample = %d (0x%x)\n\t", sample, sample);
            // Konverterer Q sample til Big endian :
			  rx_buff[file_index]=(unsigned char)sample;
			  file_index++;
			  rx_buff[file_index]=(unsigned char)(sample>>8);
			  file_index++;
			  rx_buff[file_index]= (unsigned char)(sample>>16);
			  file_index++;
		
            //line_index=0;              //reset line pointer
            sample_count++;

      } // while (!file.atEnd())
		frame_count=sample_count/MAX_SAMPLES_FRAME;
		std::cout << "LoadSamples::Samples Loaded (sample_count) = " << sample_count << std::endl;
		std::cout << "LoadSamples::Frames (sample_count/MAX_SAMPLES_FRAME) = " << frame_count << std::endl;
		printf ("rx_buff[i] = ");	
		for (unsigned int i=0; i<20;i++) printf ("%x, ", rx_buff[i]);
		//	std::cout << "Samples in rx_buff[0] = " << rx_buff[0] << std::endl;
		//printf("Samples Loaded (sample_count) : %d \n",sample_count);
		//printf("Frames (sample_count/MAX_SAMPLES_FRAME) : %d \n",frame_count);
/*		char text[512];
		sprintf(text, "Samples Loaded : %d",sample_count);
		QMessageBox::information(this, "LoadSample::LoadSamples()", text, QMessageBox::Ok);
*/		

    if (err != kSuccess) {
        QMessageBox::warning(this, "LoadSample::LoadSamples() : \n"
         "samples load failed\n\n",
         qPrintable(fName));
    }
	return err;
}
