/****************************************************************************
 **
 **
 ** SDRham Setup
 **
 ****************************************************************************/

 #include <QApplication>

 #include "window.h"

 int main(int argc, char *argv[])
 {
     QApplication app(argc, argv);
     SDRhamMainWindow mWindow;
   //  SetupWindow sWindow;
     mWindow.show();
  //   sWindow.show();
     return app.exec();
 }

