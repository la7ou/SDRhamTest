/*
 *  LoadSamples.cpp
 *  SDR_ham_ez_xcode
 *
 *  Created by Stein Erik Ellevseth on 11.10.08.
 *  Copyright 2008 __MyCompanyName__. All rights reserved.
 *
 */

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

    int err = kSuccess;
    fileDialog = new QFileDialog(this);
    fileDialog->setFilter(tr( "Images (*.dat)"));
    fileDialog->setFileMode(QFileDialog::ExistingFile);
	fileDialog->setDirectory("../../../../");
    fileDialog->show();
    if (fileDialog->exec()) fileNames = fileDialog->selectedFiles();
    if (!fileNames.isEmpty()) fName = fileNames[0];
	filnavn = fName.toUtf8();

    QFile file(filnavn);//FILE *FileHandle;
    if (!file.open(QFile::ReadOnly | QFile::Text))
    {
    //    QMessageBox::warning(this, "FileW::uploadAD6620()", "Open AD6620 file failed.");
        QMessageBox::warning(this, "LoadSamples:: LoadSamplesFile() : samples File:",filnavn);
        return kFailure;
    }
	
      sample_count=0;
      file_index=0;

	     while (!file.atEnd())
		  {
			QByteArray line = file.readLine(); // Read a line
			QByteArray ba = line.trimmed(); // Remove obsolete spaces, \r and \n
			QByteArray I_sample, Q_sample;
			for (unsigned int n=0; ba[n]!=' ';n++)  // finner spacen mellom de to samplene
			{
				I_sample = ba.left(n+1);		// extract I sample part from line
				Q_sample = ba.right(n+1);	// extract Q sample part from line
			}
			sample= atoi(I_sample); // I sample convert to integer		
            // gjør om I sample til little endian :
			rx_buff[file_index]=(unsigned char)(sample>>16);
            file_index++;
            rx_buff[file_index]=(unsigned char)(sample>>8);
            file_index++;
            rx_buff[file_index]=(unsigned char)sample;
            file_index++;
			
			sample= atoi(Q_sample); // convert Q sample to integer
            // gjør om Q sample til little endian :
			rx_buff[file_index]=(unsigned char)(sample>>16);
            file_index++;
            rx_buff[file_index]=(unsigned char)(sample>>8);
            file_index++;
            rx_buff[file_index]=(unsigned char)sample;
            file_index++;
		
            //line_index=0;              //reset line pointer
            sample_count++;

      } // while (!file.atEnd())
		frame_count=sample_count/MAX_SAMPLES_FRAME;
		std::cout << "Samples Loaded (sample_count) = " << sample_count << std::endl;
		std::cout << "Frames (sample_count/MAX_SAMPLES_FRAME) = " << frame_count << std::endl;
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
        // fprintf(stderr, "FileW::pick_imp(): AD 6620 upload failed\n");
    }
	return err;
}
