/*
 *  ezload.h
 *
 */
#ifndef EZLOAD_H
#define EZLOAD_H

#include <stdio.h>
#include <stdlib.h>
#include <usb.h>

#include <QDialog>

//The following constants are for file processing
#define FILE_BUFFER_SIZE  1024    //Max AD6620 file size
#define LINE_SIZE         80      //Line size in file
#define FILTER_SIZE       256     //Max number of taps in AD6620

//#define	USBCS_ADDRESS		0x7F92
#define	USBCS_ADDRESS		0xe600
#define MAX_RECORD_LENGTH 	16

#define kSuccess 0
#define kFailure 1

typedef struct HEX_RECORD {
    unsigned int length;
    unsigned int address;
    unsigned int type;
    unsigned char data[MAX_RECORD_LENGTH];
} HEX_RECORD;

int hexRead(HEX_RECORD *record, FILE *file);

 class FileW : public QDialog
 {
     Q_OBJECT

 public:
    FileW();

 private:
    int uploadFirmware(struct usb_dev_handle *dev);//, const char *filename);
	 
    void EZConfig(int vid, int pid);
    int EZControl(struct usb_dev_handle *dev, int address, int length, char data[]);
    int EZClose(struct usb_dev_handle *dev);
    
    int USBBulkSend(unsigned int, unsigned char*); // fra usbsub.h
    
    int uploadAD6620();
    void LoadAD6620FileButtonClick();//(TObject *Sender);
    void ClearFilterClick();//(TObject *Sender);
    void LoadAD6620HWClick();//(TObject *Sender);
   void AD6620RunStop(unsigned int mode);
    
    const char *filnavn;
    char USBCS;
    int usbVendor;
    int usbProduct;
    int	err;
    struct usb_dev_handle *dev_handle;
    static unsigned char mode_register;
    struct fir_filter_data {
    unsigned int coeff_count;
    int fir_coeff[FILTER_SIZE]; //Array of filter coeffesients
    }fir_filter_data;
      static const unsigned int scic2value[];
  static const unsigned int scic5value[];
  
  // Variables that need to be modified through signal/slots :
  int MCIC2CSpinEditValue,  SCIC2CSpinEditValue, MCIC5CSpinEditValue, SCIC5CSpinEditValue, 
  MRCFCSpinEditValue, SOUTCSpinEditValue, ChipModeBoxItemIndex, ClkMultCSpinEditValue,
   NCOModeBoxItemIndex, FIRModeBoxItemIndex;
  QString SampleFreqEditText;
 bool  LoadAD6620HWEnabled;

 public slots:
     void pick_ihx();
     void pick_imp();
     void init_AD6620();
     void mCIC2SpinEditValue(int m);
     void mCIC5SpinEditValue(int value);
 };
 #endif
 
