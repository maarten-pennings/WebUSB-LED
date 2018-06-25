# WebUSB-LED
WebUSB demo: control a LED on a USB stick via a website

## Things learned
This was my first project with an Arduino Pro Micro, and my first WebUSB project.
The following things were new to me. I did my development on Windows.

 * The Pro Micro flash stores a bootloader and the customer app.
   Both use USB, both typically manifest itself as serial port, each having its own port name (COMxx).
   This means that during a power up (or during a programming action which starts with a reset) there are lots of USB pings from the OS
   and different serial ports are shown in the device manager.
 * There seem to be two bootloaders around. The default one waits 8 seconds after startup before it jumps to the customer app. 
   The alternative one [sparkfun](https://learn.sparkfun.com/tutorials/pro-micro--fio-v3-hookup-guide/troubleshooting-and-faq) 
   only waits 0.75 seconds, which gives a faster programming/debug cycle. However if the customer app is broken there is only a 
   0.75s break-in window. Fortunately the Sparkfun bootloader can be given two reset pulses, then it also wait 8 seconds.
 * The [tutorial](https://github.com/webusb/arduino) I followed, explains that WebUSB requires the USB device to have 
   at least USB version 2.1. Unfortunately, this can not be set for a project, this requires a change in the library.
   In hardware/arduino/avr/cores/arduino/USBCore.h find `#define USB_VERSION 0x200` and change `0x200` to `0x210`.
 * The same tutorial explains that USB 2.1 devices need to have a Binary Object Store (BOS) descriptor when they are enumerated. 
   The "WebUSB" library of the tutorial has that BOS. However, if you make the switch to USB version 2.1 and do not use 
   the WebUSB library, your device will no longer enumerate, so it cannot be flashed anymore. To solve this "brick"
   start the bootloader (see above), and flash a correct firmware.
 * Install the WebUSB from the tutorial, and add it as Arduino library.
   Ensure the sketch `#include <WebUSB.h>`, and instantiate
   `WebUSB WebUSBSerial(1 /* https:// */, "webusb.github.io/arduino/demos/rgb");`.
 * As far as I understand, instantiating `WebUSB` in the sketch adds three features
   * A serial port towards your application, which manifests itself as COMxx in Windows. This seems to ne standard CDC.
   * A serial port towards a web browser, which manifests itself in browsers that support WebUSB.
   * A hint to the user to visit a web page, in this example1 `https://webusb.github.io/arduino/demos/rgb`.
 * The two serial ports are different data pipes, use in code as
   `Serial` for serial over USB (pops up as COM port) or `WebUSBSerial` for serial towards browser).
   There is even a `Serial1` for serial over hardware UART 1.
 * At this moment only Chrome seems to support WebUSB, see [table](https://caniuse.com/#feat=webusb)
 * At this moment Chrome on Windows does not give the hint.
   So, the user needs to enter the URL by hand.
 * The web page that is opened in Chrome to access the WebUSB device needs to be on a _web server_.
   If the html/js file is opened from a _local file system_, they can not access the device. 
   Don't know why, maybe a security artefact.
 * The disconnect/connect from Chrome to the device is not reliable. A webpage reload always works for me.
 * There is a generic serial [console](https://webusb.github.io/arduino/demos/console/) implementation, 
   which is device application independent.
   
 
 
 
