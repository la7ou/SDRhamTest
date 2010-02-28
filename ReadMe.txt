Beskrivelse :

Status :

To Do :

Innhold :
main.cpp - hovedprogrammet
window.cpp/h - vindu klasser basert p� Qt
exload.cpp/h - metoder for nedlasting av firmware til FX2 og filterinformasjon AD6620

Filer fra Halvor
LoadSamples.cpp/h	- Laste opp lagrede samples
ReadSamples.cpp/h	- Read samples fra SDRham FE
ProcessSamples.cpp/h	- prosessere og stream til lydenhet
dsp001.cpp/h
dsp002.cpp/h
dsppsk31.cpp/h

sdrham_main.ihx		- GnuRadio firmware modifisert for SDRham
just_blink_leds.ihx 	- for test a nedlasting av firmware
blink_leds.ihx 		- for test a nedlasting av firmware
usbfx2.hex		- Halvors firmware basert p� Keil

d1000fc25k.imp		- AD6620 filter koeffisienter for nedlasting
ReadMe.txt - denne filen

moc_* er filer generert automatisk av Qt


1. Installere verkt�y
   Qt Open Source Edition (Windows/X11/Mac) lastes ned herfra :
   - http://trolltech.com/developer/downloads/qt/index

   libusb finner du her:
http://libusb.sourceforge.net/http://libusb-win32.sourceforge.net/

   P� Ubuntu kan det gj�res enkelt det med Synaptic package manager, s�k p� libusb og Qt4

   Git, scm
	http://git-scm.com/
	http://code.google.com/p/git-osx-installer/wiki/OpenInGitGui

   qwt, grafisk presentasjon

2. Gj�re klar prosjektet
qmake -project
qmake

Legg til -lusb p� slutten av LIBS i den genererte Makefile
LIBS          = ..... -lusb
I xcode,Existing Framework :
Add /opt/local/lib/libusb.a
Add /System/Bibliotek/Frameworks/AudioUnitFramework

3. Bygging og eksekvering
make
sudo ./SDRham_qt
(m� bruke sudo p� linux, pga. noen rettigheter jeg ikke har funnet ut av)
evt med en annen style
sudo ./SDRham_qt -style windows
(se http://doc.trolltech.com/4.3/qapplication.html)
open SDRham_qt.app
