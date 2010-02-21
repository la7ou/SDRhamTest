/*
 *  Readsamples.cpp
 *  SDR_ham_ez_xcode
 *
 *  Created by Stein Erik Ellevseth on 10.10.08.
 *  Copyright 2008 __MyCompanyName__. All rights reserved.
 *
 */
//#include <QObject> 
#include <QMessageBox>
//#include <QDialog>
//#include <QFileDialog>
//#include <QFile>
//#include <QString> // Istedet for Ansistring
//#include "ezload.h"
#include "Readsamples.h"
//#include <stdio.h>
//#include <stdlib.h>

ReadSamples:: ReadSamples(QObject *parent)
: QThread(parent)
{
 //   QMessageBox::warning(this, "ReadSamples:: ReadSamples()",
 //   "class ReadSamples is created.");
		printf("ReadSamples::ReadSamples is created.\n");
}


void ReadSamples::ReadSamplesFromFE()
{
    //QMessageBox::warning(this, "ReadSamples:: ReadSamplesFromFE()",
	    //QMessageBox::Information(this,"ReadSamplesFromFE() is not implemented.",QMessageBox::Ok);
	printf("ReadSamples:: ReadSamplesFromFE() is not implemented.\n");						

}

void ReadSamples::run()
{
	qDebug() << "Executing ReadSamples in new independant thread, GUI is NOT blocked";
	exec();
}