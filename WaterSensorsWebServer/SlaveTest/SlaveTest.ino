/*
  Serial Call and Response
 Language: Wiring/Arduino

 This program sends an ASCII A (byte of value 65) on startup
 and repeats that until it gets some data in.
 Then it waits for a byte in the serial port, and
 sends three sensor values whenever it gets a byte in.

 Thanks to Greg Shakar and Scott Fitzgerald for the improvements

   The circuit:
 * potentiometers attached to analog inputs 0 and 1
 * pushbutton attached to digital I/O 2

 Created 26 Sept. 2005
 by Tom Igoe
 modified 24 April 2012
 by Tom Igoe and Scott Fitzgerald

 This example code is in the public domain.

 http://www.arduino.cc/en/Tutorial/SerialCallResponse

 */
#include <SoftwareSerial.h>

SoftwareSerial mySerial(3, 1); // RX, TX 13, 15

String data = "";

void setup(){
// Open serial communications and wait for port to open:
  Serial.begin(9600);
  mySerial.begin(9600);
  mySerial.println("Setup");
}//setup

void loop(){
  if( mySerial.available()>0 ){
    char ch=mySerial.read();
    data.concat(ch);
    Serial.print((int)ch, HEX); // debug: print *every* character, in hex.
    if( ch=='\n' ){
      Serial.println();
      // Use "F()" macro as recommended by
      // https://learn.adafruit.com/memories-of-an-arduino/optimizing-sram .
      Serial.print(F("complete message:"));
      Serial.println(data);
      data="";
      Serial.print(F("waiting for next message ..."));
      }
  }
  if( Serial.available()>0 ){
    mySerial.write(Serial.read());
  }
}//loop


//void setup() {
//  // start serial port at 9600 bps:
//  Serial.begin(9600);
//  while (!Serial) {
//    ; // wait for serial port to connect. Needed for native USB port only
//  }
//
// 
//  establishContact();  // send a byte to establish contact until receiver responds
//}
//
//void loop() {
//  // if we get a valid byte, read analog ins:
//  if (Serial.available() > 0) {
//    // get incoming byte:
//    String inVal = Serial.readString();
//   
//    // delay 10ms to let the ADC recover:
//    delay(10);
//
//  }
//}
//
//void establishContact() {
//  while (Serial.available() <= 0) {
//    Serial.print('A');   // send a capital A
//    delay(300);
//  }
//}


