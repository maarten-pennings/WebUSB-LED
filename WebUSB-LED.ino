/* WebUSB-LED demo on Arduino pro micro
** 2018 June 24 Maarten Pennings
*/


// ==== USB ======================================================================================

// WebUSB only works for USB 2.1 and up. Therefore, change USB_VERSION to 0x210 in file
// %AppData%\Local\Arduino15\packages\arduino\hardware\avr\1.6.21\cores\arduino\USBCore.h
// See https://github.com/webusb/arduino, and also see warning on (semi) bricking.
#include <WebUSB.h> 

// Creating an instance of WebUSBSerial will add an additional USB interface to
// the device that is marked as vendor-specific (rather than USB CDC-ACM) and
// is therefore accessible to the browser. The URL is a hint to the browser.
WebUSB WebUSBSerial(1 /* https:// */, "maarten-pennings.github.io");


// ==== LED ======================================================================================

// The UART Tx and Rx led are used as signal LEDs.
// They are turned off by making them input, so no RX or TX activity will turn them on.
// However, when the leds are on, any RX or TX action makes them blink or turn off.

// One extra LED is used. Define the pin and which level is needed to switch it on and off.
#define LED_EXTRA_PIN  13   // This is the L led on the "Beetle Virtual Keyboard BadUSB Pro Micro ATMEGA32U4"
#define LED_EXTRA_ON   HIGH
#define LED_EXTRA_OFF  LOW

// Record the led state
int led_state;

void led_init( void ) {
  pinMode( LED_BUILTIN_TX, INPUT);
  pinMode( LED_BUILTIN_RX, INPUT);    
  pinMode( LED_EXTRA_PIN, OUTPUT);
  digitalWrite( LED_EXTRA_PIN, LED_EXTRA_OFF);
  led_state= 0;
}

void led_set( int on ) {
  if( on ) { 
    // TX pin is low active; make pin low to turn led on.
    pinMode( LED_BUILTIN_TX, OUTPUT);    // pin as output.
    digitalWrite( LED_BUILTIN_TX, LOW);  // pin low
    // RX pin is low active; make pin low to turn led on.
    pinMode( LED_BUILTIN_RX, OUTPUT);
    digitalWrite( LED_BUILTIN_RX, LOW);
    // The extra led
    digitalWrite( LED_EXTRA_PIN, LED_EXTRA_ON);
  } else { // led off
    pinMode( LED_BUILTIN_TX, INPUT);
    pinMode( LED_BUILTIN_RX, INPUT);    
    // The extra led
    digitalWrite( LED_EXTRA_PIN, LED_EXTRA_OFF);
  }
  led_state= on;
}

int led_get( void ) {
  return led_state;
}


// ==== BUT ======================================================================================

// Pin 7 has a 2k2 pull up. It is also connected to a button. A button press will connect pin 7 to GND.
const int BUT_PIN = 7;

void but_init( void ) {
  pinMode(BUT_PIN, INPUT);
}

int but_get( void ) {
  int state= digitalRead(BUT_PIN);
  delay(20); // Anti bounce
  return !state; // Button is low active
}


// ==== Serial ===================================================================================

// Pick one for main output
//#define MainSerial Serial        // Serial over USB (use Arduino COM port)
#define MainSerial WebUSBSerial    // Serial over WebUSB
//#define MainSerial Serial1       // Serial over hardware UART 1

// Pick one for debug output
//#define DebugSerial Serial       // Serial over USB (use Arduino COM port)
//#define DebugSerial WebUSBSerial // Serial over WebUSB
#define DebugSerial Serial1        // Serial over hardware UART 1


// ==== Setup/Loop ===============================================================================

void prompt() {
  MainSerial.print("> ");  
  MainSerial.flush();
}

int mainup;
void setup() {
  // Debug port
  DebugSerial.begin(9600); 
  DebugSerial.println("dbg: Welcome to WebUSB-LED");

  // Pins init
  but_init();
  led_init();
  DebugSerial.println("dbg: lED & button inited");
  
  // Wait till serial is up (for USB this means till PC is connected)
  int count=100;
  while( !MainSerial && count>0 ) { count--; delay(50); }
  mainup= MainSerial;
  DebugSerial.println(mainup ? "dbg: MainSerial running" : "dbg: MainSerial missing");
  
  // Print banner
  MainSerial.begin(9600); 
  MainSerial.println();
  MainSerial.println();
  MainSerial.println("Welcome to WebUSB-LED");
  MainSerial.flush();
  prompt();
}

static int prev; 
static int cmd=-1; // -1=none, 0=led-to-0, 1=led-to-1
void loop() {
  if( mainup!=MainSerial ) {
    mainup= MainSerial;
    DebugSerial.println(mainup ? "dbg: MainSerial running" : "dbg: MainSerial missing");
  }
  // Check button (changes)
  int down = but_get();
  if( down!=prev ) { 
    DebugSerial.print("dbg: but="); DebugSerial.println(down); 
    MainSerial.print("but="); MainSerial.println(down); prompt(); 
    cmd= down; 
  }
  prev= down;
  // Get queries
  if( MainSerial && MainSerial.available() ) {
    int byte = MainSerial.read();
    if( byte=='\n' || byte=='\r' ) { /*skip*/ }
    else if( byte=='0' ) { MainSerial.println("0: off"); cmd=0; prompt(); }
    else if( byte=='1' ) { MainSerial.println("1: on"); cmd=1; prompt(); }
    else if( byte=='t' ) { MainSerial.println("t: toggle"); cmd=!led_get(); prompt(); }
    else if( byte=='l' ) { MainSerial.print("l: led="); MainSerial.println(led_get()); prompt(); }
    else if( byte=='b' ) { MainSerial.print("b: but="); MainSerial.println(but_get()); prompt(); }
    else if( byte=='h' ) { MainSerial.println("h: 0=led-off, 1=led-on, t=led-toggle, l=led-get, b=but-get, h=help"); prompt(); }
    else                 { MainSerial.print((char)byte); MainSerial.println(": <error, h for help>"); prompt(); }
    DebugSerial.print("dbg: cmd="); DebugSerial.println((char)byte); 
  }
  // Update led
  if( cmd!=-1 ) {
    led_set(cmd);
    cmd= -1;
  }
}

