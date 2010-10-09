/****************************************************************************
 **
 ** SDRham Setup
 ** window.cpp
 **
 ****************************************************************************/

#include <QtGui>
#include <QFileDialog>
#include <QWidget>
#include <QMessageBox>
#include <QtGui>

//#include <QHBoxLayout>

#include <iostream> // cout
//#include <stdio> //println
using std::ostringstream;
using namespace std;

#include "window.h"
#include "ezload.h"
#include "LoadSamples.h"
#include "ProcessSamples.h"
#include "Readsamples.h"
 
void SDRhamMainWindow::SetupSDRhamHW()
 {
    SetupWindow *sWindow = new SetupWindow(this);
    sWindow->show();

 //   cout << "SDRhamMainWindow::SetupHW() is called" << endl;
 }

void SDRhamMainWindow::ConnectHW()
 {
    QMessageBox::warning(this, "SDRhamMainWindow::ConnectHW()",
    "This feature is not yet implemented.");
//    cout << "SDRhamMainWindow::ConnectHW() is not yet implemented" << endl;
/* Fra Halvors main.cpp :
  USBConnect();
  int status=GetUSBStatus();
  if(status)
  {
    DSPInit();
    freq=0;
    deltafreq=1;
    Label1->Caption="x 1Hz";
    FreqUpdate();
    Run->Enabled=true;
    SDRHW1->Enabled=true;
    SaveSamples1->Enabled=true;
    ydev=16;      //Start with this valuie for Y-axes deviation
    xdev=1;       //Start with X-axes deviation multilplayer 1
    if_gain=0;    //start with no IF gain
    MainStatusBar->SimpleText="SDR HW Connected";
  }
  else
    ShowMessage("SDR HW not found");
	*/
 }
 
 void SDRhamMainWindow::Read()
 {
	qDebug() << "Executing ReadSamples in new independant thread, GUI is NOT blocked";

	readSam = new ReadSamples(this);
	readSam->start(); // after this, ReadSamples's run() method starts 
 }
 
void SDRhamMainWindow::RunClick()
{
	qDebug() << "Executing ProcessSamples in new independant thread, GUI is NOT blocked";
	procSam = new ProcessSamples(this,lSam);
	procSam->start(); // after this, ProcessSamples's run() method starts 

}
 
  void SDRhamMainWindow::Stop()
 {
	std::cout << " Stop is pressed " << std::endl;
	procSam->run_flag = false;
	std::cout << " run_flag  = " <<  procSam->run_flag << std::endl;
	  delete[] procSam->pdisplay_buffer;

 }
 
  void SDRhamMainWindow::Clear()
 {
	qDebug() << "SDRhamMainWindow::Clear() is not implemented";
 }
 
   void SDRhamMainWindow::SourceBox()
 {
	qDebug() << "SSDRhamMainWindow::SourceBox() is not implemented";
 }

   void SDRhamMainWindow::SaveSamples()
 {
	qDebug() << "SDRhamMainWindow::SaveSamples() is not implemented";
 }
 
void SDRhamMainWindow::setMode(int inMode)//modeType inMode)
{
	lSam->mode=inMode;
	printf (" mode = %d\n", lSam->mode);
};
void SDRhamMainWindow::setAmMode()//modeType inMode)
{
	lSam->mode=am;
	printf (" mode = am \n");
};
void SDRhamMainWindow::setFMMode()//modeType inMode)
{
	lSam->mode=fm;
	printf (" mode = fm \n");
};
void SDRhamMainWindow::setUSBMode()//modeType inMode)
{
	lSam->mode=usb;
	printf (" mode = usb \n");
};
void SDRhamMainWindow::setLSBMode()
{
	lSam->mode=lsb;
	printf (" mode = lsb \n");
};
void SDRhamMainWindow::setPSKMode()
{
	lSam->mode=psk31;
	std::cout << " mode = psk31 \n" << std::endl;
};

 
SDRhamMainWindow::SDRhamMainWindow()
 {
    QAction *setuphwaction,*filteraction,*sourceaction,*savesamplesaction,*loadsamplesaction;

    QMenu *filemenu;
    QMenu *setupmenu;
    QMenu *helpmenu; 
	fileWdialog = new FileW();
	lSam = new LoadSamples();
	procSam = NULL;
	readSam = NULL;

	savesamplesaction= new QAction("&Save samples", this);
    connect(savesamplesaction, SIGNAL(triggered()), this, SLOT(SaveSamples()));
	loadsamplesaction= new QAction("&Load samples", this);
    connect(loadsamplesaction, SIGNAL(triggered()), lSam, SLOT(LoadSamplesFile()));
  
    filteraction = new QAction("&LoadFIRFilterFile", this);
    connect(filteraction, SIGNAL(triggered()), fileWdialog, SLOT(pick_imp()));
	
	sourceaction= new QAction("&Source", this);
    connect(sourceaction, SIGNAL(triggered()), this, SLOT(SourceBox()));
    setuphwaction = new QAction("&SDRhamHW", this);
    connect(setuphwaction, SIGNAL(triggered()), this, SLOT(SetupSDRhamHW()));
	
    filemenu = menuBar()->addMenu(tr("&File"));
    setupmenu = menuBar()->addMenu(tr("&Setup"));
    helpmenu = menuBar()->addMenu(tr("&Help"));
	
	//filemenu->addAction(quitaction);

    filemenu->addAction(savesamplesaction);
    filemenu->addAction(loadsamplesaction);

	setupmenu->addAction(setuphwaction);
    setupmenu->addAction(filteraction);
	setupmenu->addAction(sourceaction);

     QWidget *centralWidget = new QWidget;

     QTextEdit *textEdit = new QTextEdit;

     QHBoxLayout *controlLayout = new QHBoxLayout;
     controlLayout->addWidget(createDisplayGroup());
     controlLayout->addWidget(createFreqGroup());
     controlLayout->addWidget(createIFGroup());
     controlLayout->addWidget(createModeGroup());
     controlLayout->addWidget(createActionGroup());	 

     QVBoxLayout *centralLayout = new QVBoxLayout;
     centralLayout->addWidget(textEdit);
     centralLayout->addLayout(controlLayout);

     centralWidget->setLayout(centralLayout);

     setCentralWidget(centralWidget);
     setWindowTitle(tr("SDRham Main Window"));
     resize(480, 320);
 } 

 QGroupBox *SDRhamMainWindow::createDisplayGroup()
 {
     QGroupBox *groupBoxDisplay = new QGroupBox(tr("Display Characteristics"));
     groupBoxDisplay->setCheckable(true);
     groupBoxDisplay->setChecked(false);

     QLabel *yDev = new QLabel(tr("yDev"));
     QSpinBox *yDevSpinBox = new QSpinBox;
     yDevSpinBox->setRange(0, 20);
     yDevSpinBox->setSingleStep(1);
     yDevSpinBox->setValue(1);
     
     QLabel *xDev = new QLabel(tr("xDev"));
     QSpinBox *xDevSpinBox = new QSpinBox;
     xDevSpinBox->setRange(0, 20);
     xDevSpinBox->setSingleStep(1);
     xDevSpinBox->setValue(1);
     
     QLabel *fftSize = new QLabel(tr("FFT Size"));
     QSpinBox *fftSizeSpinBox = new QSpinBox;
     fftSizeSpinBox->setRange(0, 1024);
     fftSizeSpinBox->setSingleStep(1);
     fftSizeSpinBox->setValue(512);

     QGridLayout *grid = new QGridLayout;
     grid->addWidget(yDev,1,0);
     grid->addWidget(yDevSpinBox,0,0);
     grid->addWidget(xDev,1,1);
     grid->addWidget(xDevSpinBox,0,1);
     grid->addWidget(fftSize,1,2);
     grid->addWidget(fftSizeSpinBox,0,2);
     
     groupBoxDisplay->setLayout(grid);

     return groupBoxDisplay;
 }

 QGroupBox *SDRhamMainWindow::createFreqGroup()
 {
     QGroupBox *groupBoxFreq = new QGroupBox(tr("Frequency"));
     groupBoxFreq->setCheckable(true);
     groupBoxFreq->setChecked(false);
     
//     QLabel *setFreq = new QLabel(tr("Set Freq (Hz/MHz) : "));
     QLineEdit *setFreqLineEdit = new QLineEdit;
     setFreqLineEdit->setFocus();

     
     QLabel *tenMHz = new QLabel(tr("10 Mhz"));
     QSpinBox *tenMHzSpinBox = new QSpinBox;
     tenMHzSpinBox->setRange(0, 20);
     tenMHzSpinBox->setSingleStep(1);
     tenMHzSpinBox->setValue(1);
     
     QLabel *upDwn = new QLabel(tr("Up/Dwn"));
     QSpinBox *upDwnSpinBox = new QSpinBox;
     upDwnSpinBox->setRange(0, 20);
     upDwnSpinBox->setSingleStep(1);
     upDwnSpinBox->setValue(1);

     QGridLayout *grid = new QGridLayout;

 //    grid->addWidget(setFreq,0,0);
     grid->addWidget(setFreqLineEdit,0,0);
     grid->setColumnStretch(0, 1);
     grid->addWidget(tenMHzSpinBox,1,0);
     grid->addWidget(tenMHz,2,0);
     grid->addWidget(upDwnSpinBox,1,1);
     grid->addWidget(upDwn,2,1);

     groupBoxFreq->setLayout(grid);

     return groupBoxFreq;
 }

 QGroupBox *SDRhamMainWindow::createIFGroup()
 {
     QGroupBox *groupBoxFreq = new QGroupBox(tr("IF Gain"));
     
     QRadioButton *radio1 = new QRadioButton(tr("Aut"));
     QRadioButton *radio2 = new QRadioButton(tr("Man"));
     
     QLabel *gain = new QLabel(tr("Main Gain"));
     QSpinBox *gainSpinBox = new QSpinBox;
     gainSpinBox->setRange(0, 20);
     gainSpinBox->setSingleStep(1);
     gainSpinBox->setValue(1);
    

     QVBoxLayout *vbox = new QVBoxLayout;
     vbox->addWidget(radio1,0);
     vbox->addWidget(radio2,1);
     vbox->addWidget(gainSpinBox,2);
     vbox->addWidget(gain,2);

     groupBoxFreq->setLayout(vbox);

     return groupBoxFreq;
 }

 QGroupBox *SDRhamMainWindow::createModeGroup()
 {

     QGroupBox *groupBoxFreq = new QGroupBox(tr("Mode"));

     QRadioButton *amMode = new QRadioButton(tr("AM"));
     QRadioButton *fmMode = new QRadioButton(tr("FM"));
     QRadioButton *usbMode = new QRadioButton(tr("USB"));
     QRadioButton *lsbMode = new QRadioButton(tr("LSB"));
	 QRadioButton *pskMode = new QRadioButton(tr("PSK31"));

     QVBoxLayout *vbox = new QVBoxLayout;
     vbox->addWidget(amMode,0);
     vbox->addWidget(fmMode,1);
     vbox->addWidget(usbMode,2);
     vbox->addWidget(lsbMode,3);
	 vbox->addWidget(pskMode,4);

     groupBoxFreq->setLayout(vbox);

/* Fikk ikke denne til å virke :
	 if(amMode->isChecked()) emit okClicked(am);
	 if(fmMode->isChecked()) emit okClicked(fm);
	 if(usbMode->isChecked()) emit okClicked(usb);		 
	 if(lsbMode->isChecked()) emit okClicked(lsb);
	 if(pskMode->isChecked()) emit okClicked( psk31);
	connect(this, SIGNAL(okClicked(int)), procSam,SLOT(setMode(int)));
*/	 
	 	connect(amMode, SIGNAL(clicked()),this ,SLOT(setAmMode()));
		connect(fmMode, SIGNAL(clicked()), this ,SLOT(setFMMode()));
		connect(usbMode, SIGNAL(clicked()), this ,SLOT(setUSBMode()));
		connect(lsbMode, SIGNAL(clicked()), this ,SLOT(setUSBMode()));
		connect(pskMode, SIGNAL(clicked()), this ,SLOT(setPSKMode()));

	 
     return groupBoxFreq;
 }
 
  QGroupBox *SDRhamMainWindow::createActionGroup()
 {
     //QGroupBox *groupBoxAction = new QGroupBox(tr("Action"));
     QGroupBox *groupBoxAction = new QGroupBox();
	QPushButton *read = new QPushButton(tr("Read"));
	QPushButton *runclick = new QPushButton(tr("Run"));
	QPushButton *stop = new QPushButton(tr("Stop"));
	QPushButton *clear = new QPushButton(tr("Clear"));
	
	connect(read, SIGNAL(clicked()), this , SLOT(Read()));
	connect(runclick, SIGNAL(clicked()), this, SLOT(RunClick()));	
	//connect(run, SIGNAL(clicked()), procSam , SLOT(ProcessSamplesSet())); // fjernes senere
	connect(stop, SIGNAL(clicked()), this , SLOT(Stop()));	
	connect(clear, SIGNAL(clicked()),this , SLOT(Clear()));

	QVBoxLayout *vbox = new QVBoxLayout;	
	vbox->addWidget(read,0);	
	vbox->addWidget(runclick,1);	
	vbox->addWidget(stop,1);	
	vbox->addWidget(clear,3);

     groupBoxAction->setLayout(vbox);

     return groupBoxAction;
 }

SetupWindow::SetupWindow(QWidget *parent)
     : QDialog(parent)//QWidget(parent)
 {
     fileWdialog = new FileW();
     QGridLayout *grid = new QGridLayout;
     grid->addWidget(createEZUSBFileGroup(), 0, 0);
     grid->addWidget(createAD6620FilterFileGroup(), 1, 0);
     grid->addWidget(createSpinBoxGroup(), 1, 1);

     QLabel *sampleFreq = new QLabel(tr("Sample Freq (Hz) : "));
     QLineEdit *sampleFreqLineEdit = new QLineEdit;
     sampleFreqLineEdit->setFocus();

     grid->addWidget(sampleFreq,2,0);
     grid->addWidget(sampleFreqLineEdit,2,1);
     grid->setColumnStretch(2, 2);

    QPushButton *quit = new QPushButton(tr("Close"));
    //quit->setFont(QFont("Times", 18, QFont::Bold));
    connect(quit, SIGNAL(clicked()),this , SLOT(close()));
	// SLOT(done()));// qApp, SLOT(quit()));

     grid->addWidget(quit,3,0);

     setLayout(grid);

     setWindowTitle(tr("SDRham Setup"));
     resize(480, 320);
 }

 QGroupBox *SetupWindow::createEZUSBFileGroup()
//	: QFileDialog
 {
     QGroupBox *groupBox = new QGroupBox(tr("EZ USB File"));
     groupBox->setCheckable(true);
     groupBox->setChecked(false);

     QRadioButton *radio1 = new QRadioButton(tr("HEX  "));
     QRadioButton *radio2 = new QRadioButton(tr("OBJ  "));
     QPushButton *pushButton = new QPushButton(tr("Load EZ USB"));

 //    QString filter = "*.htm *.html *.txt *.xml";
//     FileW *file = new FileW();
    connect(pushButton, SIGNAL(clicked()), fileWdialog, SLOT(pick_ihx()));
 //    radio1->setChecked(true);

//    QVBoxLayout *vbox = new QVBoxLayout;
     QGridLayout *grid = new QGridLayout;
     grid->addWidget(radio1, 0,0);
     grid->addWidget(radio2, 1,0);
     grid->addWidget(pushButton, 0,1);

 //    vbox->addStretch(1);
     groupBox->setLayout(grid);
//	fileDialog->show();

     return groupBox;
 }

 QGroupBox *SetupWindow::createAD6620FilterFileGroup()
 {
     QGroupBox *groupBox = new QGroupBox(tr("&AD6620 Filter File"));

     QRadioButton *radio1 = new QRadioButton(tr("AD6620 File"));
     QRadioButton *radio2 = new QRadioButton(tr("Filter Coeffisients"));
 
     radio1->setChecked(true);


     QVBoxLayout *vbox = new QVBoxLayout;
     vbox->addWidget(radio1);
     vbox->addWidget(radio2);

     QPushButton *loadAD6620FileButton = new QPushButton(tr("Load File"));
     vbox->addWidget(loadAD6620FileButton);

     QPushButton *initFilterButton = new QPushButton(tr("Initialize Filter"));
     vbox->addWidget(initFilterButton);

//     FileW *fileDialog = new FileW();
    connect(loadAD6620FileButton, SIGNAL(clicked()), fileWdialog, SLOT(pick_imp()));
    connect(initFilterButton, SIGNAL(clicked()), fileWdialog, SLOT(init_AD6620()));
//     vbox->addStretch(1);
     groupBox->setLayout(vbox);

     return groupBox;
 }




 QGroupBox *SetupWindow::createSpinBoxGroup()
 {

     QGroupBox *groupBox = new QGroupBox(tr("AD 6620 filter characterstics"));
     groupBox->setCheckable(true);
     groupBox->setChecked(false);

     QLabel *mCIC2 = new QLabel(tr("MCIC 2"));
     //QSpinBox *mCIC2SpinBox = new QSpinBox;
     mCIC2SpinBox = new QSpinBox;
     mCIC2SpinBox->setRange(0, 20);
     mCIC2SpinBox->setSingleStep(1);
     mCIC2SpinBox->setValue(1);

     QLabel *mCIC5 = new QLabel(tr("MCIC 5"));
     QSpinBox *mCIC5SpinBox = new QSpinBox;
     mCIC5SpinBox->setRange(0, 20);
     mCIC5SpinBox->setSingleStep(1);
     mCIC5SpinBox->setValue(1);

     QLabel *mRCF = new QLabel(tr("MRCF"));
     QSpinBox *mRCFSpinBox = new QSpinBox;
     mRCFSpinBox->setRange(0, 20);
     mRCFSpinBox->setSingleStep(1);
     mRCFSpinBox->setValue(1);

    QLabel *sCIC2 = new QLabel(tr("SCIC 2"));
     QSpinBox *sCIC2SpinBox = new QSpinBox;
     sCIC2SpinBox->setRange(0, 20);
     sCIC2SpinBox->setSingleStep(1);
     sCIC2SpinBox->setValue(0);

     QLabel *sCIC5 = new QLabel(tr("SCIC 5"));
     QSpinBox *sCIC5SpinBox = new QSpinBox;
     sCIC5SpinBox->setRange(0, 20);
     sCIC5SpinBox->setSingleStep(1);
     sCIC5SpinBox->setValue(0);

     QLabel *sRCF = new QLabel(tr("SRCF"));
     QSpinBox *sRCFSpinBox = new QSpinBox;
     sRCFSpinBox->setRange(0, 20);
     sRCFSpinBox->setSingleStep(1);
     sRCFSpinBox->setValue(0);

     QLabel *clockMult = new QLabel(tr("Clock Multiplier"));
     QSpinBox *clockMultSpinBox = new QSpinBox;
     sRCFSpinBox->setRange(0, 20);
     sRCFSpinBox->setSingleStep(1);
     sRCFSpinBox->setValue(1);

     QLabel *ncoMode = new QLabel(tr("NCO Mode"));
     QComboBox *ncoModeComboBox = new QComboBox;
     ncoModeComboBox->addItem("Normal");
     ncoModeComboBox->addItem("tbd");

     QLabel *chipMode = new QLabel(tr("Chip Mode"));
     QComboBox *chipModeComboBox = new QComboBox;
     chipModeComboBox->addItem("Single Channel Real");
     chipModeComboBox->addItem("tbd");

     QLabel *firFilterMode = new QLabel(tr("FIR Filter Mode"));
     QComboBox *firFilterModeComboBox = new QComboBox;
     firFilterModeComboBox->addItem("Normal");
     firFilterModeComboBox->addItem("tbd");

//     QVBoxLayout *vbox = new QVBoxLayout;
     QGridLayout *grid = new QGridLayout;
     grid->addWidget(mCIC2,0,0);
     grid->addWidget(mCIC2SpinBox,0,1);
     grid->addWidget(mCIC5,1,0);
     grid->addWidget(mCIC5SpinBox,1,1);
     grid->addWidget(mRCF,2,0);
     grid->addWidget(mRCFSpinBox,2,1);

     grid->addWidget(sCIC2,0,2);
     grid->addWidget(sCIC2SpinBox,0,3);
     grid->addWidget(sCIC5,1,2);
     grid->addWidget(sCIC5SpinBox,1,3);
     grid->addWidget(sRCF,2,2);
     grid->addWidget(sRCFSpinBox,2,3);
     grid->addWidget(clockMult,3,1);
     grid->addWidget(clockMultSpinBox,3,3);

     grid->addWidget(ncoMode,0,4);
     grid->addWidget(ncoModeComboBox,0,5);
     grid->addWidget(chipMode,1,4);
     grid->addWidget(chipModeComboBox,1,5);
     grid->addWidget(firFilterMode,2,4);
     grid->addWidget(firFilterModeComboBox,2,5);
     
//    FileW *fileDialog = new FileW();
    connect(mCIC2SpinBox, SIGNAL(valueChanged(int)), fileWdialog , SLOT(mCIC2SpinEditValue(int)));
//    connect(mCIC2SpinBox, SIGNAL(valueChanged(int)), fileWdialog , SLOT(display(int)));
    
    connect(mCIC5SpinBox, SIGNAL(valueChanged(int)), fileWdialog, SLOT(mCIC5SpinEditValue(int)));
 //    vbox->addStretch(1);
     groupBox->setLayout(grid);

     return groupBox;
 }

