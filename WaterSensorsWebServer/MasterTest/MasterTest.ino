/*
  Serial Event example

 When new serial data arrives, this sketch adds it to a String.
 When a newline is received, the loop prints the string and
 clears it.

 A good test for this is to try it with a GPS receiver
 that sends out NMEA 0183 sentences.

 Created 9 May 2011
 by Tom Igoe

 This example code is in the public domain.

 http://www.arduino.cc/en/Tutorial/SerialEvent
*/

 #include <SoftwareSerial.h>
#define rxPin 10
#define txPin 11
SoftwareSerial rf(rxPin, txPin);

void setup() {
  Serial.begin(9600);
  while(!Serial);
  rf.begin(9600);
  while(!rf);
  Serial.println("Water Sensors Started!");
  //EstablishContact();
}

double _ph = 5.0;
double _phAvg=5.0;
double _tds = 40.0;
double _tdsAvg=40.0;
double _phOffset=0.1;
double _tdsOffset=1000;
String _response;
String _cmd="";

void loop() {
  ProcessCommand();
}

void ProcessCommand(){  
  Receive();
  if(_cmd != ""){
    //delay(100);
    Serial.println(_cmd);
    delay(10);
    if(_cmd="/GetSensorVals"){
      _ph += 0.1;
      _phAvg+=0.2;
      _tds += 1.2;
      _tdsAvg+=0.8;
      _phOffset+=0.1;
      _tdsOffset+=5;
      _response += "\"ph\":\"" + String(_ph,2)+ "\"\r\n";
      _response += "\"phAvg\":\"" + String(_phAvg,2)+ "\"\r\n";
      _response += "\"tds\":\"" + String(_tds,2)+ "\"\r\n";
      _response += "\"tdsAvg\":\"" + String(_tdsAvg,2)+ "\"\r\n";
      _response += "\"phOffset\":\"" + String(_phOffset,2)+ "\"\r\n";
      _response += "\"tdsOffset\":\"" + String(_tdsOffset,2)+ "\"\r\n";
//    
      Transmit(_response);
      //delay(20);
    }
    _cmd="";
  }
  //delay(100);
}
void Transmit(String msg) {
  for(int i=0;i<msg.length();i++){
    char aChar = msg.charAt(i);
    rf.print(aChar);
    delay(20);
  }
  rf.print('\n');
  rf.flush();
}
void Receive(){
  char aChar;
   while( rf.available() > 0 ){
    //while(true){
      aChar = rf.read();
      if(aChar == '\n' || aChar == '\r') break;
      _cmd.concat(aChar);
      delay(10);
    //}
  }

  
  
}

void EstablishContact() {
  rf.print('c');
  while (rf.available() <= 0) {
    rf.print('c');
    //rf.begin(9600);
    Serial.println(F("Not connected to Esp"));   // send a capital A
    delay(300);
  }
  Serial.println(F("Connected to Esp!"));
}

//
//String inputString = "";         // a string to hold incoming data
//boolean stringComplete = false;  // whether the string is complete
//
//void setup() {
//  // initialize serial:
//  Serial.begin(9600);
//  // reserve 200 bytes for the inputString:
//  inputString.reserve(200);
//}
//
//void loop() {
//  // print the string when a newline arrives:
//  if (stringComplete) {
//    Serial.println(inputString);
//    // clear the string:
//    inputString = "";
//    stringComplete = false;
//  }
//}
//
///*
//  SerialEvent occurs whenever a new data comes in the
// hardware serial RX.  This routine is run between each
// time loop() runs, so using delay inside loop can delay
// response.  Multiple bytes of data may be available.
// */
//void serialEvent() {
//  while (Serial.available()) {
//    // get the new byte:
//    char inChar = (char)Serial.read();
//    // add it to the inputString:
//    inputString += inChar;
//    // if the incoming character is a newline, set a flag
//    // so the main loop can do something about it:
//    if (inChar == '\n') {
//      stringComplete = true;
//    }
//  }
//}


