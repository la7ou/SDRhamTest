/*
 *  ezload.cpp
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <usb.h>
#include <ezload.h>
#include <window.h>

#include <QFileDialog>
#include <QFile>
#include <QString> // Istedet for Ansistring
#include <QMessageBox>

#include <iostream> // cout
using std::ostringstream;
using namespace std;

// from setupsdrhw.cpp
unsigned char FileW::mode_register;
const unsigned int FileW::scic2value[]={0,0,0,2,2,3,4,4,4,5,5,5,6,6,6,6,6};
const unsigned int FileW::scic5value[]={0,0,0,3,5,7,8,10,10,11,12,13,13,14,
      15,15,15,16,16,17,17,17,18,18,18,19,19,19,20,20,20,20,20};


int hexRead(HEX_RECORD *record, FILE *file)
{
    /* Read the next hex record from the file into the structure */
    char c;
    unsigned int i, length;
    int input, data, check;
    unsigned char sum;

    c = getc(file);
    if (c != ':') {
        fprintf(stderr, "hexRead: hex file line did not start with colon, found %c\n", c);
        return 0;
    }

    input = fscanf(file, "%2X%4X%2X", &record->length, &record->address, &record->type);
    if (input != 3) {
        fprintf(stderr, "hexRead: could not read line preamble\n");
        return 0;
    }

    sum = record->length + ((record->address >> 8) & 0xFF) + (record->address & 0xFF) + record->type;

    length = record->length;
    if (length > MAX_RECORD_LENGTH) {
        fprintf(stderr, "hexRead: record too long %u, max %d\n", length, MAX_RECORD_LENGTH);
        return 0;
    }

    for (i = 0; i < length; i++) {
        input = fscanf(file, "%2X", &data);

        if (input != 1) {
            if (i != record->length) {
                fprintf(stderr, "hexRead: incomplete line, found %u, expected %u\n", i, record->length);
                return 0;
            }
        }

        record->data[i] = data;
        sum += data;
    }

    input = fscanf(file, "%2X\n", &check);
    sum = 0x100 - sum;

    if ((input != 1) || (check != sum)) {
        fprintf(stderr, "hexRead: checksum error, read %2X from file, should be %2X\n", check, sum);
        return 0;
    }

    return 1;
}

FileW::FileW()
 {
    //filnavn ="/home/steinee/Dokumenter/ezloadSDR/blink_leds.ihx";
    usbVendor =  0x04b4;
    usbProduct = 0x8613;
    dev_handle = NULL;
    err = kFailure;
 }

//struct usb_dev_handle *EZConfig(int vid, int pid)
void FileW::EZConfig(int vid, int pid)
{
    struct usb_bus *busses;
    struct usb_bus *bus;

    struct usb_device *dev;
//    struct usb_dev_handle *dev_handle = NULL;

    int err;
    //cout << "EZConfig 1 :filename selected is :"  << filnavn << endl;
    /* USB Set up */
    usb_init();
    //cout << "EZConfig 2:filename selected is :"  << filnavn << endl;
    err = usb_find_busses();
    if (err == 0) {
        fprintf(stderr, "EZConfig: no USB busses found\n");
        exit(1);
        //return NULL;
    }
    //cout << "EZConfig 3:filename selected is :"  << filnavn << endl;
    err = usb_find_devices();
    if (err == 0) {
        fprintf(stderr, "EZConfig: no USB devices found\n");
        exit(1);
        //return NULL;
    }
    //cout << "EZConfig 3:filename selected is :"  << filnavn << endl;
    /* I don't like goto either, but it's easier this way */
    busses = usb_get_busses();// due to windows //usb_busses;
    for (bus = busses; bus; bus = bus->next) {
        for (dev = bus->devices; dev; dev = dev->next) {
            if ((dev->descriptor.idVendor == vid) && (dev->descriptor.idProduct == pid))
                goto found;
        }
    }
    //cout << "EZConfig 4:filename selected is :"  << filnavn << endl;
    //fprintf(stderr, "EZConfig: no matching VID/PID found\n");
    exit(1);
    //return NULL;
//    cout << "*EZConfig/return NULL; :filename selected is :"  << filnavn << endl;
found:
    dev_handle = usb_open(dev);
    if (dev_handle == NULL) {
        fprintf(stderr, "EZConfig: couldn't open device\n");
        exit(1);
        //return NULL;
    }
    //cout << "EZConfig 4:filename selected is :"  << filnavn << endl;
    /* set configuration */
    err = usb_set_configuration(dev_handle, dev->config->bConfigurationValue);
    if (err != 0) {
        fprintf(stderr, "EZConfig: couldn't set device configuration\n");
        exit(1);
        //return NULL;
    }
    //cout << "EZConfig 5:filename selected is :"  << filnavn << endl;
    /* claim it */
    err = usb_claim_interface(dev_handle, dev->config->interface->altsetting->bInterfaceNumber);
    if (err != 0) {
        fprintf(stderr, "EZConfig: couldn't claim device interface\n");
        exit(1);
        //return NULL;
    }
    //cout << "EZConfig 6:filename selected is :"  << filnavn << endl;
    //return dev_handle;
}



void FileW::pick_ihx() // http://doc.trolltech.com/4.3/qfiledialog.html
 {
    err = kFailure;
    QStringList fileNames;	
    QString fName;
    QFileDialog *fileDialog = new QFileDialog(this);
    fileDialog->setFilter(tr( "Images (*.ihx *.hex)"));
    fileDialog->setFileMode(QFileDialog::ExistingFile);
    fileDialog->show();
    if (fileDialog->exec())
	fileNames = fileDialog->selectedFiles();
	//fName = fileDialog->getOpenFileName();
    if (!fileNames.isEmpty())
	fName = fileNames[0];
    //cout << "FileW::picked()::File selected is :"  << qPrintable(fName) << endl;
    //printf("FileW::picked(): firmware file name is :\n", fName);
    //filename = fName.toLatin1();
    filnavn = fName.toUtf8();
    cout << "FileW::pick_ihx :filename selected is :"  << filnavn << endl;
    //printf("FileW::picked(): firmware file name is :\n", filename);
    printf("usbVendor =  %d\n", usbVendor);
    printf("usbProduct =  %d\n", usbProduct);

//    dev = 
    EZConfig(usbVendor, usbProduct);
    if (dev_handle == NULL) {
        QMessageBox::warning(this, "FileW::pick_ihx() : ", "no EZ-USB connected with specified VID/PID");
        fprintf(stderr, "FileW::picked(): no EZ-USB connected with specified VID/PID\n");
        exit(1);
    }
    //cout << "FileW::picked() 2 :filename selected is :"  << filnavn << endl;
    filnavn = fName.toUtf8();
    err = uploadFirmware(dev_handle);//, filnavn);
    if (err != kSuccess) {
        QMessageBox::warning(this, "FileW::pick_ihx() : ", "EZ-USB firmware upload failed");
        fprintf(stderr, "FileW::picked(): EZ-USB firmware upload failed\n");
    }

    /* close device */
    err = EZClose(dev_handle);
    if (err != kSuccess) {
        QMessageBox::warning(this, "FileW::pick_ihx() : ", "did not successfully close device");
        fprintf(stderr, "FileW::picked(): did not successfully close device\n");
        exit(1);
    }
 }

void FileW::pick_imp() 
 {
    printf("FileW::pick_imp() :  Selecting AD6620 File \n");
//    QMessageBox::warning(this, "FileW::pick_imp() :", "Selecting AD6620 File.");
    err = kFailure;
    QStringList fileNames;	
    QString fName;
    QFileDialog *fileDialog = new QFileDialog(this);
    fileDialog->setFilter(tr( "Images (*.dat)"));
    fileDialog->setFileMode(QFileDialog::ExistingFile);
    fileDialog->show();
    if (fileDialog->exec())
	fileNames = fileDialog->selectedFiles();
    if (!fileNames.isEmpty())
	fName = fileNames[0];
//    QMessageBox::warning(this, "FileW::pick_imp() : AD6620 File:",qPrintable(fName));
    // printf("FileW::pick_imp() : AD6620 File is selected\n");
    // cout << "FileW::pick_imp() : AD6620 File selected is :"  << qPrintable(fName) << endl;
    filnavn = fName.toUtf8();
    err = uploadAD6620();
    if (err != kSuccess) {
        QMessageBox::warning(this, "FileW::pick_imp() : \n"
         "AD6620 File upload failed\n",
         qPrintable(fName));
        // fprintf(stderr, "FileW::pick_imp(): AD 6620 upload failed\n");
    }
 }
 


int FileW::EZControl(struct usb_dev_handle *dev, int address, int length, char data[])
{
    int err;

    /* send a program EZ-USB RAM control message */
    err = usb_control_msg(dev, 0x40, 0xA0, address, 0, data, length, 10000);
    if (err == 0) {
        fprintf(stderr, "EZControl: control message to address 0x%X failed\n", address);
        return 0;
    }

    return err;
}

int FileW::EZClose(struct usb_dev_handle *dev)
{
    int err;

    /* restore interface 0 for future loads */
    err = usb_set_altinterface(dev, 0);
    if (err != 0) {
        fprintf(stderr, "EZClose: couldn't restore alternate interface\n");
        return kFailure;
    }

    /* soft reset device (better safe than sorry) */
    err = usb_reset(dev);
    if (err != 0) {
        fprintf(stderr, "EZClose: couldn't reset device\n");
        return kFailure;
    }

    /* close up open devices, release memory */
    err = usb_close(dev);
    if (err != 0) {
        fprintf(stderr, "EZClose: couldn't close device\n");
        return kFailure;
    }

    return kSuccess;
}


//-----------------------------------------------------------------------
//USBSendBulkData
//-----------------------------------------------------------------------

//int FileW::EZControl(struct usb_dev_handle *dev, int address, int length, char data[])
int FileW::USBBulkSend(unsigned int count, unsigned char *ptr) // fra usbsub.cpp
{
    int err=0;
    /* send a bulk message */
    err = usb_bulk_write(dev_handle, USB_ENDPOINT_TYPE_BULK, (char*) ptr, count, 10000);
    if (err == 0) {
        // fprintf(stderr, "USBBulkSend: bulk message to device 0x%X failed\n", dev_handle);
        fprintf(stderr, "USBBulkSend: bulk message to device failed\n");
        return kFailure;
    }
    return kSuccess;

}

void FileW::mCIC2SpinEditValue(int m)
{
//    QMessageBox::warning(this, "FileW::mCIC2SpinEdit():Is not implemented");
//    QMessageBox::information(this, "FileW::mCIC2SpinEdit() with  %d value",(const char*)m);
    cout << "FileW::mCIC2SpinEdit():Is not yet implemented : " << m << endl;
}

void FileW::mCIC5SpinEditValue(int value)
{
//    QMessageBox::warning(this, "FileW::mCIC2SpinEdit():Is not implemented");
//    QMessageBox::warning(this, "FileW::mCIC5SpinEdit() value : ",1234);
    printf("FileW::mCIC5SpinEditValue(): =  %d\n", value);
    cout << "FileW::mCIC5SpinEdit():Is not yet implemented : " << value << endl;
}



int FileW::uploadFirmware(struct usb_dev_handle *dev)//, const char *filename)
{

    FILE 		*firmware = NULL;
    HEX_RECORD 		record;
    int 		err;

    /* open firmware hex file */
    firmware = fopen(filnavn, "r");
    if (firmware == NULL) {
        fprintf(stderr, "uploadFirmware: firmware file open failed\n");
//        cout << "uploadFirmware:filename selected is :"  << qPrintable(filnavn) << endl;
        //cout << "uploadFirmware:filename selected is :"  << filnavn << endl;
//	printf("uploadFirmware: firmware file name is :%s\n", filename);
        return kFailure;
    }

    /* Assert reset */
    USBCS = 1;
    err = EZControl(dev, USBCS_ADDRESS, 1, &USBCS);
    if (err == 0) {
        QMessageBox::warning(this, "FileW::uploadFirmware() : ", "failed to assert EZ-USB reset");
        fprintf(stderr, "uploadFirmware: failed to assert EZ-USB reset\n");
        return kFailure;
    }

    /* upload firmware */
    for (;;) {
        err = hexRead(&record, firmware);

        if (record.type != 0)
            break;

        if (err == 1)
            err = EZControl(dev, record.address, record.length, (char*) record.data);

        if (err == 0) {
            QMessageBox::warning(this, "FileW::uploadFirmware() : ", "EZ-USB upload to address %x failed");
            fprintf(stderr, "uploadFirmware: EZ-USB upload to address %x failed\n", record.address);
            return kFailure;
        }
    }

    /* De-assert reset */
    USBCS = 0;
    err = EZControl(dev, USBCS_ADDRESS, 1, &USBCS);
    if (err == 0) {
        QMessageBox::warning(this, "FileW::uploadFirmware() : ", "failed to deassert EZ-USB reset");
        fprintf(stderr, "uploadFirmware: failed to deassert EZ-USB reset\n");
        return kFailure;
    }
    
    /* Choose alternate interface setting 1 */
    err = usb_set_altinterface(dev, 1);
    if (err != kSuccess) {
        QMessageBox::warning(this, "FileW::uploadFirmware() : ", "couldn't set alternate interface");
        fprintf(stderr, "uploadFirmware: couldn't set alternate interface\n");
        return kFailure;
    }

    /* close file */
    fclose(firmware);

    return kSuccess;
}

int FileW::uploadAD6620()
{

//   FILE *filterware ;
//    filterware = fopen(filnavn, "r");
//   if (filterware == NULL) 
    QFile file(filnavn);//FILE *FileHandle;
    if (!file.open(QFile::ReadOnly | QFile::Text))
    {
    //    QMessageBox::warning(this, "FileW::uploadAD6620()", "Open AD6620 file failed.");
        QMessageBox::warning(this, "FileW::uploadAD6620() : AD6620 File:",filnavn);
        return kFailure;
    }

 
 //   QByteArray pReadData = file.read(FILE_BUFFER_SIZE);
    // === Nedenfor er hentet fra Halvors setupsdrhw.cpp

    unsigned int state=0,file_status;//,*fir_coeff;
    unsigned long num_bytes;
    unsigned char *pLineBuff,*pReadData;
    QString LineBuff,FileName;// AnsiString LineBuff,FileName;
    
    pReadData=new unsigned char[FILE_BUFFER_SIZE];
    
    file_status = file.read((char*)pReadData, FILE_BUFFER_SIZE);
    num_bytes = file.bytesAvailable();
    printf("FileW::uploadAD6620() file_status : =  %d\n", file_status);
    printf("FileW::uploadAD6620() num_bytes : =  %ld\n", num_bytes);


        LineBuff.resize(LINE_SIZE);
        pLineBuff= (unsigned char*)LineBuff.utf16();//.c_str();
        fir_filter_data.coeff_count=0;  //Set coeffesient count to zero
        unsigned int j=0; //line position index
        unsigned int n;   //line position read index
    

 //       while(file_status && num_bytes)  //Process file if read success
 //       {
          for(unsigned int i=0; i<(unsigned int)num_bytes; i++)
          {
            if((pReadData[i]=='\r' || pReadData[i]=='\n') && j)
            {
              pLineBuff[j]=0;   //here when a line is found

              switch(state)
              {
                case 0: //Here for CIC2
         //       QMessageBox::warning(this, "FileW::uploadAD6620()",
         //       "case 0: //Here for CIC2");
                  for(n=0; pLineBuff[n]!='&' && pLineBuff[n]!=0; n++) ;
                  LineBuff.resize(n);
                  MCIC2CSpinEditValue=LineBuff.toInt();
                  SCIC2CSpinEditValue=scic2value[LineBuff.toInt()];
                  //MCIC2CSpinEdit->Value=LineBuff.ToInt();
                  //SCIC2CSpinEdit->Value=scic2value[LineBuff.ToInt()];
                  state++;  //Go to next state CIC5
                  LineBuff.resize(LINE_SIZE);
                  break;

                case 1: //Here to for CIC5
         //       QMessageBox::warning(this, "FileW::uploadAD6620()",
         //       "case 1: //Here to for CIC5");
                  for(n=0; pLineBuff[n]!='&' && pLineBuff[n]!=0; n++) ;
                  LineBuff.resize(n);
                  MCIC5CSpinEditValue=LineBuff.toInt();
                  SCIC5CSpinEditValue=scic5value[LineBuff.toInt()];
                  //MCIC5CSpinEdit->Value=LineBuff.ToInt();
                  //SCIC5CSpinEdit->Value=scic5value[LineBuff.ToInt()];
                  state++;  //Go to next state
                  LineBuff.resize(LINE_SIZE);
                  break;

                case 2: //Here to for RCF
           //     QMessageBox::warning(this, "FileW::uploadAD6620()",
           //     "case 2: //Here to for RCF");
                  for(n=0; pLineBuff[n]!='&' && pLineBuff[n]!=0; n++) ;
                  LineBuff.resize(n);
                  MRCFCSpinEditValue=LineBuff.toInt();
                  SOUTCSpinEditValue=4;
                  //MRCFCSpinEdit->Value=LineBuff.ToInt();
                  //SOUTCSpinEdit->Value=4;
                  LineBuff.resize(LINE_SIZE);
                  state++;  //Go to next state
                  break;

                case 3: //Here to for CLK frequency
          //      QMessageBox::warning(this, "FileW::uploadAD6620()",
          //      "case 3: //Here to for CLK frequency");
                  for(n=0; pLineBuff[n]!=0; n++) ;
                  LineBuff.resize(n);
                  SampleFreqEditText=LineBuff;
                  //  SampleFreqEdit->Text=LineBuff;
                  LineBuff.resize(LINE_SIZE);
                  state++;
                  break;

                case 4: //Here for Chip Mode
          //      QMessageBox::warning(this, "FileW::uploadAD6620()",
          //      "case 4: //Here for Chip Mode");
                  for(n=0; pLineBuff[n]!=0; n++) ;
                  LineBuff.resize(n);
                  ChipModeBoxItemIndex=LineBuff.toInt();
                  //ChipModeBox->ItemIndex=LineBuff.ToInt();
                  LineBuff.resize(LINE_SIZE);
                  state++;  //Go to next state
                  break;

                case 5: //Here for CLK Mode
          //      QMessageBox::warning(this, "FileW::uploadAD6620()",
          //      " case 5: //Here for CLK Mode");
                  for(n=0; pLineBuff[n]!=0; n++) ;
                  LineBuff.resize(n);
                  ClkMultCSpinEditValue=LineBuff.toInt();
                  //ClkMultCSpinEdit->Value=LineBuff.ToInt();
                  LineBuff.resize(LINE_SIZE);
                  state++;  //Go to next state
                  break;

                case 6: //Here for FIR filter coeffesients
            //    QMessageBox::warning(this, "FileW::uploadAD6620()",
            //    "case 6: //Here for FIR filter coeffesients");
                  for(n=0; pLineBuff[n]!=0; n++) ;
                  LineBuff.resize(n);
                  fir_filter_data.fir_coeff[fir_filter_data.coeff_count]=LineBuff.toInt();//ToInt();
                  fir_filter_data.coeff_count++;
                  LineBuff.resize(LINE_SIZE);
                  break;
              } // switch
              j=0;
            } else if(pReadData[i]!='\r' && pReadData[i]!='\n' && pReadData[i]!=' ' && j<LINE_SIZE)
            {
                pLineBuff[j]=pReadData[i];
                j++;
            }
        } // end for(unsigned int i=0; i<(unsigned int)num_bytes; i++)
//    } // end while(file_status && num_bytes)
     QMessageBox::warning(this, "FileW::uploadAD6620()",
                "fir_filter_data read");
    return kSuccess;
} // end int FileW::uploadAD6620()
//---------------------------------------------------------------------------

void FileW::ClearFilterClick()//(TObject *Sender)
{
  unsigned int size_filter;

  size_filter=MCIC2CSpinEditValue*MCIC5CSpinEditValue*
              MRCFCSpinEditValue*ClkMultCSpinEditValue;
  //size_filter=MCIC2CSpinEdit->Value*MCIC5CSpinEdit->Value*
  //            MRCFCSpinEdit->Value*ClkMultCSpinEdit->Value;
  if(size_filter>FILTER_SIZE)
    size_filter=FILTER_SIZE;  //Max filter size id FILTER_SIZE=256

  for(unsigned int i=0; i<size_filter; i++)
    fir_filter_data.fir_coeff[i]=0; //clear filter coeffesients
            //Write a +1 in filter center position
  fir_filter_data.fir_coeff[0]=0x0007FFFF;
  fir_filter_data.coeff_count=size_filter; //coeffesient count
  LoadAD6620HWEnabled=true;
  //  LoadAD6620HW->Enabled=true;
}
//---------------------------------------------------------------------------

void FileW::init_AD6620() 
 {
    QMessageBox::warning(this, "FileW::init_AD6620() : ", "initializing AD6620");
    // printf("FileW::init_AD6620() : initialize AD6620 may be needed\n");
    // cout << "FileW::init_AD6620() : initialize AD6620 may be needed " << endl;
/* }

void FileW::LoadAD6620HWClick()//(TObject *Sender)
{
*/
  QString text;//AnsiString  text;
  unsigned char setup_buffer[10];
    FileW fileWref; //temp

  setup_buffer[0]=0x09;
  setup_buffer[1]=0xa0;
  setup_buffer[2]=0x00;
  setup_buffer[3]=0x03;
  setup_buffer[4]=0x01;   //Set the AD6620 in soft reset mode
  USBBulkSend(9,setup_buffer);

  unsigned short filter_address=0;  //Initialze filter coeffesients
  for(unsigned int i=0; i<fir_filter_data.coeff_count; i++)
  {
    setup_buffer[0]=0x09;
    setup_buffer[1]=0xa0;
    setup_buffer[2]=(unsigned char)filter_address;
    setup_buffer[3]=(unsigned char)(filter_address>>8);
    setup_buffer[4]=(unsigned char)fir_filter_data.fir_coeff[i];
    setup_buffer[5]=(unsigned char)(fir_filter_data.fir_coeff[i]>>8);
    setup_buffer[6]=(unsigned char)(fir_filter_data.fir_coeff[i]>>16);
    USBBulkSend(9,setup_buffer);
    filter_address++;
  }

  setup_buffer[0]=0x09;
  setup_buffer[1]=0xa0;
  setup_buffer[2]=0x00;
  setup_buffer[3]=0x03;
  //switch(ChipModeBox->ItemIndex)
  switch(ChipModeBoxItemIndex)
  {
    case 1: setup_buffer[4]=0x09; //select Single channel real mode
            break;
    case 2: setup_buffer[4]=0x0b; //Select Dual channel real mode
            break;
    case 3: setup_buffer[4]=0x0d; //Select Single channel complex
            break;    //All modes with sync master
    default: setup_buffer[4]=0x09; //default Single channel real mode
            break;
  }
  mode_register=setup_buffer[4];
  USBBulkSend(9,setup_buffer);

  setup_buffer[0]=0x09;
  setup_buffer[1]=0xa0;
  setup_buffer[2]=0x01;
  setup_buffer[3]=0x03;
  switch(NCOModeBoxItemIndex)
  //  switch(NCOModeBox->ItemIndex)
  {
    case 0: setup_buffer[4]=0x00; //NCO normal mode
            break;
    case 1: setup_buffer[4]=0x01; //NCO bypass
            break;
    case 2: setup_buffer[4]=0x02; //NCO Phase Dither
            break;
    case 3: setup_buffer[4]=0x04; //NCO amplitude duther
            break;
    case 4: setup_buffer[4]=0x06; //NCO ph and amp dither
            break;
  }
  USBBulkSend(9,setup_buffer);

  setup_buffer[0]=0x09;
  setup_buffer[1]=0xa0;
  setup_buffer[2]=0x02;
  setup_buffer[3]=0x03;
  setup_buffer[4]=0xff;
  setup_buffer[5]=0xff;
  setup_buffer[6]=0xff;
  setup_buffer[7]=0xff; //NCO sync mask
  USBBulkSend(9,setup_buffer);

  setup_buffer[0]=0x09;
  setup_buffer[1]=0xa0;
  setup_buffer[2]=0x03;
  setup_buffer[3]=0x03;
  setup_buffer[4]=0x8b;
  setup_buffer[5]=0x4f;
  setup_buffer[6]=0x01;
  setup_buffer[7]=0x00; //Default frequency
  USBBulkSend(9,setup_buffer);

  setup_buffer[0]=0x09;
  setup_buffer[1]=0xa0;
  setup_buffer[2]=0x04;
  setup_buffer[3]=0x03;
  setup_buffer[4]=0x00;
  setup_buffer[5]=0x00; //NCO phase offset zero
  USBBulkSend(9,setup_buffer);

  setup_buffer[0]=0x09;
  setup_buffer[1]=0xa0;
  setup_buffer[2]=0x05;
  setup_buffer[3]=0x03;
  setup_buffer[4]=(unsigned char)SCIC2CSpinEditValue;//SCIC2CSpinEdit->Value;
  USBBulkSend(9,setup_buffer);

  setup_buffer[0]=0x09;
  setup_buffer[1]=0xa0;
  setup_buffer[2]=0x06;
  setup_buffer[3]=0x03;
  setup_buffer[4]=(unsigned char)(MCIC2CSpinEditValue-1);//(MCIC2CSpinEdit->Value-1);
  USBBulkSend(9,setup_buffer);

  setup_buffer[0]=0x09;
  setup_buffer[1]=0xa0;
  setup_buffer[2]=0x07;
  setup_buffer[3]=0x03;
  setup_buffer[4]=(unsigned char)SCIC5CSpinEditValue;//SCIC5CSpinEdit->Value;
  USBBulkSend(9,setup_buffer);

  setup_buffer[0]=0x09;
  setup_buffer[1]=0xa0;
  setup_buffer[2]=0x08;
  setup_buffer[3]=0x03;
  setup_buffer[4]=(unsigned char)(MCIC5CSpinEditValue-1);//(MCIC5CSpinEdit->Value-1);
  USBBulkSend(9,setup_buffer);

  setup_buffer[0]=0x09;
  setup_buffer[1]=0xa0;
  setup_buffer[2]=0x09;
  setup_buffer[3]=0x03;
  setup_buffer[4]=(unsigned char)SOUTCSpinEditValue;//SOUTCSpinEdit->Value;
  switch(FIRModeBoxItemIndex)
  //  switch(FIRModeBox->ItemIndex)
  {
    case 0: setup_buffer[4]=setup_buffer[4]&0xf7; //FIR normal mode
            break;
    case 1: setup_buffer[4]=setup_buffer[4]|0x08; //FIR Unique B mode
            break;
  }
  USBBulkSend(9,setup_buffer);

  setup_buffer[0]=0x09;
  setup_buffer[1]=0xa0;
  setup_buffer[2]=0x0a;
  setup_buffer[3]=0x03;
  setup_buffer[4]=(unsigned char)(MRCFCSpinEditValue-1);//(MRCFCSpinEdit->Value-1);
  USBBulkSend(9,setup_buffer);

  setup_buffer[0]=0x09;
  setup_buffer[1]=0xa0;
  setup_buffer[2]=0x0b;
  setup_buffer[3]=0x03;
  setup_buffer[4]=0x00; //RCF filter offset is zero
  USBBulkSend(9,setup_buffer);

  setup_buffer[0]=0x09;
  setup_buffer[1]=0xa0;
  setup_buffer[2]=0x0c;
  setup_buffer[3]=0x03;
  setup_buffer[4]=fir_filter_data.coeff_count-1;
  USBBulkSend(9,setup_buffer);

  setup_buffer[0]=0x09;
  setup_buffer[1]=0xa0;
  setup_buffer[2]=0x0d;
  setup_buffer[3]=0x03;
  setup_buffer[4]=0x00;
  USBBulkSend(9,setup_buffer);

  setup_buffer[0]=0x09;
  setup_buffer[1]=0xa0;
  setup_buffer[2]=0x00;
  setup_buffer[3]=0x03;
  setup_buffer[4]=fileWref.mode_register&0xfe;
  USBBulkSend(9,setup_buffer);

QMessageBox::warning(this, "RSP configured and %d filter coeffesients loaded",(const char*)fir_filter_data.coeff_count);
  //text.printf("RSP configured and %d filter coeffesients loaded",fir_filter_data.coeff_count);
  printf("RSP configured and %d filter coeffesients loaded",fir_filter_data.coeff_count);
  //ShowMessage(text);
}
//---------------------------------------------------------------------------

void  FileW::AD6620RunStop(unsigned int mode)
{
  unsigned char setup_buffer[10];
  FileW fileWref; //temp

  setup_buffer[0]=0x09;
  setup_buffer[1]=0xa0;
  setup_buffer[2]=0x00;
  setup_buffer[3]=0x03;
  if(mode)
    setup_buffer[4]=fileWref.mode_register&0xfe;//SetupSDRForm->mode_register&0xfe;
  else
    setup_buffer[4]=fileWref.mode_register|0x01;//SetupSDRForm->mode_register|0x01;
  USBBulkSend(9,setup_buffer);
}


