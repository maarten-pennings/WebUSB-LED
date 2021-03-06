# WebUSB-LED
WebUSB demo: control a LED on a USB stick via a website

## Introduction
This demo explores the WebUSB technology.
This promisses
 * Plug in a USB dongle (which has WebUSB compliant firmware)
 * Chrome suggests a URL to visit (which is published by the dongle)
 * The (javascript on the) webpage loaded from the URL is allowed to communicate with the dongle
 * The webpage demos the features of the dongle
 
This project is a trial of this technology.
It consists of
 * An arduino Pro Micro (with an atmega32u4 - this is a chip with USB control via firmware)
 * A LED (an optionally a button) connected to the Pro Micro (see Schematics below)
 * A firmware that makes button presses switch the LED
 * A firmware that makes commands over Serial inspect and switch the LED
 * A firmware with WebUSB support so that Chrome can give those commands
 * A [webpage](web) with LED and button state. It is also published on a real [server](https://maarten-pennings.github.io/WebUSB-LED).
 * Via the [console](https://webusb.github.io/arduino/demos/console/) from the tutorial manual commands can be tested (try h for help)
 
## Schematics
The Pro Micro must have a LED (but can be built in) and must have a USB connection.
Adding a button also shows events going from hardware to Chrome - so a nice extension.
For debugging, a reset (RES or RST) and hardware terminal are helpful.

![Schematics](WebUSB-LED.png)

For development, I use a "large" Pro Micro

![Large](ProMicro.png)

The final form factor will be a small board sometimes known as "BadUSB" or "Virtual Keyboard".

![Large](ProMicroSFF.png)


## Things learned
This was my first project with an Arduino Pro Micro, and my first WebUSB project.
The following things were new to me. I did my development on Windows.


### Software
 * The Pro Micro flash stores a bootloader and the customer app.
   Both use USB, both typically manifest itself as serial port, each having its own (that is, different) port name (COMxx).
   This means that during a power up (or during a programming action which starts with a reset) 
   there are lots of USB pings from the OS and different serial ports are shown in the device manager.
 * There seem to be two bootloaders around. The default one waits 8 seconds after startup before it jumps to the customer app. 
   The alternative one, from [sparkfun](https://learn.sparkfun.com/tutorials/pro-micro--fio-v3-hookup-guide/troubleshooting-and-faq), 
   only waits 0.75 seconds, which gives a faster programming/debug cycle. However if the customer app is broken there is only a 
   0.75s break-in window to reach the bootlaoder. 
   Fortunately the Sparkfun bootloader can be given _two_ reset pulses in quick succession, then it also wait 8 seconds.
 * The [tutorial](https://github.com/webusb/arduino) I followed, explains that WebUSB requires the USB device to have 
   at least USB version 2.1. Unfortunately, this cannot be set for a _project_, this requires a change in the _library_,
   and hence is applicable for all projects. To change the USB version, open
   ` %AppData%\Local\Arduino15\packages\arduino\hardware\avr\1.6.21\cores\arduino\USBCore.h` 
   find `#define USB_VERSION 0x200` and change `0x200` to `0x210`.
 * The same tutorial explains that USB 2.1 devices need to have a Binary Object Store (BOS) descriptor when they are enumerated. 
   The WebUSB library of the tutorial has that BOS. However, if you make the switch to USB version 2.1 and do not use 
   the WebUSB library, your device will no longer enumerate, so it cannot be flashed anymore. To solve this "brick"
   start the bootloader (see above), and flash a correct firmware (with either USB version reset, or with WebUSB instantiated).
 * Downoad the WebUSB from the tutorial, and add it as Arduino library.
   Ensure the sketch has `#include <WebUSB.h>`, and ensure it instantiates
   `WebUSB WebUSBSerial(1 /* https:// */, "webusb.github.io/arduino/demos/console");`.
 * As far as I understand, instantiating WebUSB in the sketch adds three features
   * A serial port towards the OS, which manifests itself as COMxx in Windows. This seems to be standard a CDC class serial port.
   * A serial port towards a web browser, which manifests itself in browsers that support WebUSB.
   * A hint to the user to visit a web page, in this example `https://webusb.github.io/arduino/demos/rgb`.
 * The two serial ports are different data pipes. They can be uses in parallel (side by side) in code via
   `Serial` for serial over USB (pops up as COM port) or `WebUSBSerial` for serial towards browser.
   There is even a third pipe, `Serial1` for serial over hardware UART 1 
   (The TX and RX pins on the Pro Micro board, for which you need an FTDI, CP2102 or CH340 UART-to-USB bridge).
 * There is a generic serial [console](https://webusb.github.io/arduino/demos/console/) implementation, 
   which is device application independent.
 * Unintentionally, I once flashed the Pro Micro with the WebUSB library, but with the old USB_VERSION 0x200.
   This triggered a new driver association in my windows PC, and whatever I tried, WebUSB never worked again.
   It appeared indeed a _association_ problem: when I changed the `WEBUSB_SHORT_NAME` 
   (this is actually a wrong name, since it actually should be a unique USB serial number) in `WebUSB.h`from `WUART` to e.g. `WEBUART`
   WebUSB started working again. Then I found out the real solution: remove the association.
   With the non working Pro Micro attached to the PC, Open Device Manager, expand Universal Serial Bus devices, right click 
   Arduino Micro, and select Uninstall device. This removes the association. Power cycle the Pro Micro, and the correct driver
   will be activated again.

### Support
 * At this moment the only browser to support WebUSB seems to be Chrome, 
   see [table](https://caniuse.com/#feat=webusb).
 * At this moment Chrome on Windows does not give the URL hint 
   (a pop-up with the suggested URL to visit for the WebUSB device that is plugged in).
   So, the user needs to enter the URL by hand.
 * Chrome on Ubuntu works (even the URL hint). But you need to create a udev rule. I made a new file `22-maarten.rules` (use `sudo`)
   in `/etc/udev/rules.s` with one line (note the vendor id is defined in the Arduino USB stack)
   `SUBSYSTEM=="usb", ATTRS{idVendor}=="2341", MODE:="0666", GROUP="plugdev"`
 * Chrome on Mac works out of the box, even the URL hint.
 * Chrome on Android works out of the box (with a USB on the go cable). I did not see the URL hint.
 * iPhone will be hard, because it does not expose USB.
 * I believe the URL hint is generated by the browser, not the OS, because it only pops up when the browser was running.
   Hence it is unclear how this would work on Android.
   
### Web server
 * The web page that is opened in Chrome to access the WebUSB device needs to be on a _web server_.
   If the html file is opened from a _local file system_, it cannot access the device. 
   I don't know why yet, maybe a security artefact.
 * Similarly, as [google explains](https://developers.google.com/web/updates/2016/03/access-usb-devices-on-the-web) during 
   development we can interact with WebUSB through http://localhost but to deploy it on a site we'll need to use HTTPS.
 * The disconnect/connect from Chrome to the device is not reliable. 
   Fortunately, a webpage reload always works for me.
   
 
