/****************************************************************************
 **
 **
 ** SDRham Setup
 ** window.h
 **
 ****************************************************************************/

#ifndef WINDOW_H
#define WINDOW_H

#include <QWidget>
#include <QDialog>
#include <QMainWindow>
#include <QtGui> //QSpinBox

#include "ezload.h"
#include "Loadsamples.h"
#include "ProcessSamples.h"
#include "ReadSamples.h"

// http://web.mit.edu/qt-dynamic_v4.2.1/www/widgets-charactermap.html
class QFontComboBox; // midlertidig

class QGroupBox;

class SetupWindow : public QDialog //QWidget
 {
     Q_OBJECT

 public:
     SetupWindow(QWidget *parent = 0);
     FileW *fileWdialog;
 private:
     QGroupBox *createEZUSBFileGroup();
     QGroupBox *createAD6620FilterFileGroup();
     QGroupBox *createSpinBoxGroup();  
     
     QSpinBox *mCIC2SpinBox;

//     QLineEdit *sampleFreqLineEdit;
 };

class InitWidget : public QWidget
 {
 public:
     InitWidget(QWidget *parent = 0);
 };


class SDRhamMainWindow : public QMainWindow //QWidget
 {
     Q_OBJECT

 public:
     SDRhamMainWindow(); //QWidget *parent = 0);
	enum {am,fm,usb,lsb,psk31 }; // ProcessSamples
	//	enum modeType {am,fm,usb,lsb,psk31 }; // ProcessSamples

 private:
     QGroupBox *createDisplayGroup();
     QGroupBox *createFreqGroup();
     QGroupBox *createIFGroup();
     QGroupBox *createModeGroup();
     QGroupBox *createActionGroup();
	 
	FileW *fileWdialog;
	ProcessSamples *procSam;
	LoadSamples *lSam;

	ReadSamples *readSam;

/*-->
		int mode;//modeType mode;
*/	
signals:
	void okClicked(int);//modeType);
	 
public slots:
	void SetupSDRhamHW();
	void ConnectHW();
	void Read();
	void RunClick();
	void Stop();
	void Clear();
	void SourceBox();
	void SaveSamples();
		void setMode(int);//modeType);
		void setAmMode();
		void setFMMode();
		void setUSBMode();
		void setLSBMode();
		void setPSKMode();

 };

 #endif

