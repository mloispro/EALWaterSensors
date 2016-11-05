
#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <WiFiClient.h>
#include <Wire.h>
//#include <SoftwareSerial.h>
#include <EEPROM.h>


extern "C" {
#include "user_interface.h"
}
//SoftwareSerial unoSerial(3, 1); // RX, TX 13, 15

int _ip[4]{192, 168, 10, 162};
const char *hostName = "WaterSensor-2";

const char *ssid = "One Love";//"One Love";//"SMU_Aruba_WiFi";
const char *password = "teddy1207";//teddy1207";

//esp-01 is 0,2
int _sdaPin = 0;//0//4
int _sclPin = 2;//2//5

// TCP server at port 80 will respond to HTTP requests
WiFiServer server(80);

const byte _slave = 8; //has to be greater than 7
uint8_t _wireReqLength = 20;
uint8_t _wireRespLength = 10;

String _newLine = "\r\n"; 

String ProcessRequest(String req){
  String response = "HTTP/1.1 200 OK\r\nContent-Type: application/json\r\n\r\n";
  if (req == "/GetSensorVals")
  {
    response += GetSensorVals();
  }
  else if (req.indexOf("/SetTDSOffset") != -1)
  {
    //..../SetTDSOffset/1210
    String offset = SplitString(req,'/',2);
   
    PrintDebug("Parsed TDS Offset: " + offset);
    
    //"tdsoffset=1210"
    response += SetTDSOffset(offset);
  }
   else if (req.indexOf("/SetPHOffset") != -1)
  {
    //..../SetPHOffset/3.1
    String offset = SplitString(req,'/',2);
   
    PrintDebug("Parsed PH Offset: " + offset);
    
    //"phoffset=1210"
    response += SetPHOffset(offset);
  }
  return response;
}


String SetPHOffset(String val){
  String response="";

  Transmit("/SetPHOffset");
  //"phoffset=1210"
  String request = "phoffset="+val;
  Transmit(request);
  delay(20);

  response = "{\r\n";
  response += "\"msg\":\"PH Offset Updated to -> " + val + "\"\r\n";
  response += "}\r\n";
  
  return response;
}

String SetTDSOffset(String val){
  String response="";

  Transmit("/SetTDSOffset");
  //"tdsoffset=1210"
  String request = "tdsoffset="+val;
  Transmit(request);
  delay(20);
  
  response = "{\r\n";
  response += "\"msg\":\"TDS Offset Updated to -> " + val + "\"\r\n";
  response += "}\r\n";
  return response;
}

String GetSensorVals(){
  String response="";
  
  String ph="";
  //String phAvg="";
  String tds="";
  //String tdsAvg="";
  String phOffset="";
  String tdsOffset="";
  String reading="";
  String readingDur="";
  String readingInter="";

  Transmit("/GetSensorVals");
   
  ph = Request();
  //phAvg = Request();
  tds = Request();
  //tdsAvg = Request();
  phOffset = Request();
  tdsOffset = Request();
  reading = Request();
  readingDur = Request();
  readingInter = Request();
  
  delay(20); //let wire clear.

  //response = "HTTP/1.1 200 OK\r\nContent-Type: application/json\r\n\r\n";
  response = "{\r\n";
  response += "\"host\":\"" + String(hostName) + "\",\r\n";
  response += "\"ph\":" + String(ph)+ ",\r\n";
  //response += "\"phAvg\":\"" + String(phAvg)+ "\"\r\n";
  response += "\"tds\":" + String(tds)+ ",\r\n";
  //response += "\"tdsAvg\":\"" + String(tdsAvg)+ "\"\r\n";
  response += "\"phOffset\":" + String(phOffset)+ ",\r\n";
  response += "\"tdsOffset\":" + String(tdsOffset)+ ",\r\n";
  response += "\"reading\":\"" + String(reading)+ "\",\r\n";
  response += "\"readingDur\":\"" + String(readingDur)+ "\",\r\n";
  response += "\"readingInter\":\"" + String(readingInter)+ "\"\r\n";
  response += "}\r\n";
  
  return response;
  
}

void setMac() {

 // Set as SoftAP and client
  WiFi.mode(WIFI_AP_STA);       
  byte *mac = (byte*)malloc(6);
  WiFi.macAddress(mac);

  PrintDebug("reading mac: ");
  String strmac;
  for (int i = 0; i < 6; ++i)
  {
    strmac += mac[i];
  }
  Serial.println(strmac);
  
  PrintDebug("reading eeprom mac: ");
  String eemac;
  for (int i = 0; i < 6; ++i)
  {
    eemac += EEPROM.read(i);
  }
  Serial.println(eemac);
  if(eemac=="000000" || eemac=="255255255255255255"){
    
    PrintDebug("eemac empty.. ");
    PrintDebug("writing mac to eeprom: ");
    for (int i = 0; i < 6; ++i)
    {
      EEPROM.write(i, mac[i]);
      PrintDebug("Wrote: ");
      Serial.println(mac[i]); 
    }
    EEPROM.commit();
    delay(100);
  }

  PrintDebug("setting mac from eeprom..");
  
  for (int i = 0; i < 6; ++i)
  {
    mac[i] = EEPROM.read(i);
  }
  wifi_set_macaddr(SOFTAP_IF, mac);
  free(mac);
  delay(100);
  PrintDebug("finished setting mac from eeprom.");

//// // Change MAC 
//// byte *mac = (byte*)malloc(6);
//// WiFi.macAddress(mac);
//// mac[0] = 0x16;
//// mac[1] = 0x19;
//// mac[2] = 0xCA;
//// wifi_set_macaddr(STATION_IF, mac);
//// mac[2] = 0xCB;
////  wifi_set_macaddr(SOFTAP_IF, mac);
////  free(mac);
//  
//  //uint8_t mac[] = {0x77, 0x01, 0x02, 0x03, 0x04, 0x05};
//  //wifi_set_macaddr(STATION_IF, &mac[0]);
}

void setup(void)
{  
  //unoSerial.begin(9600);
  Serial.begin(57600);
  EEPROM.begin(512);
  delay(10);
  while(!Serial);
  
  IPAddress ip(_ip[0], _ip[1], _ip[2], _ip[3]); 
  IPAddress gateway(192, 168, 10, 1); 
  IPAddress subnet(255, 255, 255, 0); 
  Serial.print(F("Setting static ip to : "));
  Serial.println(ip);
  WiFi.config(ip, gateway, subnet);

    // Connect to WiFi network
  WiFi.hostname(hostName);
  //setMac();
  WiFi.begin(ssid, password);
  //WiFi.hostname(hostName);
  
  
  PrintDebug("");  
  
  // Wait for connection
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    PrintDebug(".");
    delay(10);
  }
  PrintDebug("");
  PrintDebug("Connected to",String(ssid));
  String ipAddress = WiFi.localIP().toString();
  PrintDebug("IP address",ipAddress);

  // Set up mDNS responder:
  // - first argument is the domain name, in this example
  //   the fully-qualified domain name is "esp8266.local"
  // - second argument is the IP address to advertise
  //   we send our IP address on the WiFi network
  //if (!MDNS.begin(hostName,ip)) {
   if (!MDNS.begin(hostName,ip)) {
    PrintDebug("Error setting up MDNS responder!");
    while(1) { 
      delay(1000);
    }
  }
  PrintDebug("mDNS responder started");
  
  // Start TCP (HTTP) server
  server.begin();
  PrintDebug("TCP server started");
  
  // Add service to MDNS-SD
  MDNS.addService("http", "tcp", 80);


  Wire.begin(_sdaPin, _sclPin);
  //Wire.pins(_sdaPin, _sclPin);
  //Wire.setClockStretchLimit(1500);
  Wire.setClockStretchLimit(15000);
  
  //EnsureWireConnected(); //dont do this here it messes up wire.

   delay(100);
}

void loop(void)
{

  //ensure wire connected every 2 min
  static unsigned long samplingTime = millis();
  if(millis() - samplingTime > 120000) { //check every 2 minute.
      EnsureWireConnected();
      samplingTime = millis();
  }
  
  // Check if a client has connected
  WiFiClient client = server.available();
  if (!client) {
    return;
  }
  PrintDebug("");
  PrintDebug("New client");

  // Wait for data from client to become available
  while(client.connected() && !client.available()){
    delay(1);
  }
  
  // Read the first line of HTTP request
  String req = client.readStringUntil('\r');
  
  // First line of HTTP request looks like "GET /path HTTP/1.1"
  // Retrieve the "/path" part by finding the spaces
  int addr_start = req.indexOf(' ');
  int addr_end = req.indexOf(' ', addr_start + 1);
  if (addr_start == -1 || addr_end == -1) {
    PrintDebug("Invalid request",req);
    return;
  }
  req = req.substring(addr_start + 1, addr_end);
  PrintDebug("Request",req);
  client.flush();
  
  String response;
  if (req == "/")
  {
    IPAddress ip = WiFi.localIP();
    String ipStr = String(ip[0]) + '.' + String(ip[1]) + '.' + String(ip[2]) + '.' + String(ip[3]);
    response = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n<!DOCTYPE HTML>\r\n<html>Hello from " + String(hostName) + " at ";
    response += ipStr;
    response += "<p>/GetSensorVals</p>";
    response += "<p>/SetPHOffset/3.1</p>";
    response += "<p>/SetTDSOffset/1210</p>";
    response += "<p>/scan</p>";
    response += "<p>/bootload</p>";
    response += "<p>/checkcom</p>";
    response += "<p>/clearcom</p>";
    response += "</html>\r\n\r\n";
    
    PrintDebug("Sending 200");
  }
  else if (req == "/bootload")
  {
    response = "Resetting ESP8266..";
    //EnsureWireConnected();
    //ESP.reset();
    ESP.restart();
    wdt_reset();
  }
  else if (req == "/scan")
  {
    response = ScanI2C();
   
  }
  else if (req == "/checkcom")
  {
    
    response = EnsureWireConnected();
   
  }
  else if (req == "/clearcom")
  {

    response = ClearCom();
   
  }
  else
  {
    response = ProcessRequest(req);
  }

  if(response==""){
    response = "HTTP/1.1 404 Not Found\r\n\r\n";
    PrintDebug("Sending 404");
  }

  client.print(response);
  //response = ""; //needed to clear serial buffer.
  PrintDebug("Done with client");
  client.flush();
  delay(10);
}


void Transmit(String request) {
  int diff = _wireReqLength - request.length();
  for(int i = 0;i<diff;i++){
    request+=" ";
  }
  PrintDebug("req",request);
  PrintDebug("reqLen",request.length());

  byte response[_wireReqLength];
  for (byte i=0;i<_wireReqLength;i++) {
    response[i] = (byte)request.charAt(i);
  }
  
  Wire.beginTransmission(_slave); // transmit to device #8
  //Wire.write(request.c_str());        // sends 10 byte
  Wire.write(response,sizeof(response));
  Wire.endTransmission(_slave);    // stop transmitting
  delay(10); //this delay is for request.
}

String Request(){
  String wireResponse;
  Wire.requestFrom(_slave,_wireRespLength); //32 bytes is max, so call multiple
  delay(20); //may need to lower this to 10 if having timeouts
  while (Wire.available() > 0) {
      char p = (char)Wire.read();
      //delay(10);
      //Serial.println(p);

      wireResponse.concat(p);
  }

 // wireResponse.replace('\0',' ');
  wireResponse.trim();
  
  PrintDebug("wireResponse",String(wireResponse));
  return wireResponse;
}

String EnsureWireConnected(){
  Transmit("/");
  String response = Request();
  if(response.length()>0){
    String msg = "Wire connection is Ok";
    PrintDebug(msg);
    return msg;
  }else{
    String msg = "Wire connection failed";
    PrintDebug(msg);
    return msg;
//    int busCleared = I2C_ClearBus();
//    while(busCleared!=0){
//      busCleared = I2C_ClearBus();
//    }
    //ScanI2C();
////    Wire.begin(_sdaPin, _sclPin);
////    Wire.setClockStretchLimit(15000);
    //PrintDebug("Wire cleared successfuly");
    //PrintDebug("Wire response failed, restarting..");
    //ESP.reset(); // hard reset
    //ESP.restart(); //boot loader
    
  }
  
}

String ScanI2C(){
  String response="";
  byte error, address;
  int nDevices;
 
  Serial.println("Scanning...");
 
  nDevices = 0;
  for(address = 1; address < 127; address++ ) 
  {
    
    Wire.beginTransmission(address);
    error = Wire.endTransmission();
    //WDT.alive();
    if (error == 0)
    {
      String found = "I2C device found at pin: ";
      Serial.print(found);
      found+=String(address,HEX);
      Serial.print(address,HEX);
      found+= "\r\n";
      response += found;
 
      nDevices++;
    }
    else if (error==4) 
    {
      Serial.print("pin: ");
      Serial.println(address);
      String error = "Unknow error at address ";
      Serial.print(error);
      error+=String(address,HEX);
      Serial.print(address,HEX);
      error+= "\r\n";
      response += error;
    }    
  }
  if (nDevices == 0){
    String notFound = "No I2C devices found";
    response = notFound+"\r\n";
    Serial.println(notFound+"\n");
  }else{
    Serial.println("done\n");
  }

  return response;
}

String SplitString(String data, char separator, int index){

  int found = 0;
  int strIndex[] = {0, -1};
  int maxIndex = data.length()-1;
  for(int i=0; i<=maxIndex && found<=index; i++){
    if(data.charAt(i)==separator || i==maxIndex){
      found++;
      strIndex[0] = strIndex[1]+1;
      strIndex[1] = (i == maxIndex) ? i+1 : i;
    }
 }
  return found>index ? data.substring(strIndex[0], strIndex[1]) : "";
}

void PrintDebug(String msg) {
  Serial.print("~esp~ ");
  Serial.println(msg);
  Serial.flush();
}

void PrintDebug(String msg, int val) {
  Serial.print("~esp~ ");
  Serial.print(msg);
  Serial.print(": ");
  Serial.println(val);
  Serial.flush();
}

void PrintDebug(String msg, String val) {
  Serial.print("~esp~ ");
  Serial.print(msg);
  Serial.print(": ");
  Serial.println(val);
  Serial.flush();
}
void PrintDebug(String msg, char *val) {
  Serial.print("~esp~ ");
  Serial.print(msg);
  Serial.print(": ");
  Serial.println(val);
  Serial.flush();
}

String ClearCom(){
  String msg="";
  int rtn = I2C_ClearBus(); // clear the I2C bus first before calling Wire.begin()
  if (rtn != 0) {
    msg="I2C bus error. Could not clear";
    if (rtn == 1) {
      msg="SCL clock line held low";
    } else if (rtn == 2) {
      msg="SCL clock line held low by slave clock stretch";
    } else if (rtn == 3) {
      msg="SDA data line held low";
    }
  } else { // bus clear
    // re-enable Wire
    // now can start Wire Arduino master
    msg="No problem with Com";
    Wire.begin(_sdaPin, _sclPin);
   
  }
  return msg;
}
/**
 * This routine turns off the I2C bus and clears it
 * on return SCA and SCL pins are tri-state inputs.
 * You need to call Wire.begin() after this to re-enable I2C
 * This routine does NOT use the Wire library at all.
 *
 * returns 0 if bus cleared
 *         1 if SCL held low.
 *         2 if SDA held low by slave clock stretch for > 2sec
 *         3 if SDA held low after 20 clocks.
 */
int I2C_ClearBus() {
#if defined(TWCR) && defined(TWEN)
  TWCR &= ~(_BV(TWEN)); //Disable the Atmel 2-Wire interface so we can control the SDA and SCL pins directly
#endif

  pinMode(_sdaPin, INPUT_PULLUP); // Make SDA (data) and SCL (clock) pins Inputs with pullup.
  pinMode(_sclPin, INPUT_PULLUP);

  delay(2500);  // Wait 2.5 secs. This is strictly only necessary on the first power
  // up of the DS3231 module to allow it to initialize properly,
  // but is also assists in reliable programming of FioV3 boards as it gives the
  // IDE a chance to start uploaded the program
  // before existing sketch confuses the IDE by sending Serial data.

  boolean SCL_LOW = (digitalRead(_sclPin) == LOW); // Check is SCL is Low.
  if (SCL_LOW) { //If it is held low Arduno cannot become the I2C master. 
    return 1; //I2C bus error. Could not clear SCL clock line held low
  }

  boolean SDA_LOW = (digitalRead(_sdaPin) == LOW);  // vi. Check SDA input.
  int clockCount = 20; // > 2x9 clock

  while (SDA_LOW && (clockCount > 0)) { //  vii. If SDA is Low,
    clockCount--;
  // Note: I2C bus is open collector so do NOT drive SCL or SDA high.
    pinMode(_sclPin, INPUT); // release SCL pullup so that when made output it will be LOW
    pinMode(_sclPin, OUTPUT); // then clock SCL Low
    delayMicroseconds(10); //  for >5uS
    pinMode(_sclPin, INPUT); // release SCL LOW
    pinMode(_sclPin, INPUT_PULLUP); // turn on pullup resistors again
    // do not force high as slave may be holding it low for clock stretching.
    delayMicroseconds(10); //  for >5uS
    // The >5uS is so that even the slowest I2C devices are handled.
    SCL_LOW = (digitalRead(_sclPin) == LOW); // Check if SCL is Low.
    int counter = 20;
    while (SCL_LOW && (counter > 0)) {  //  loop waiting for SCL to become High only wait 2sec.
      counter--;
      delay(100);
      SCL_LOW = (digitalRead(_sclPin) == LOW);
    }
    if (SCL_LOW) { // still low after 2 sec error
      return 2; // I2C bus error. Could not clear. SCL clock line held low by slave clock stretch for >2sec
    }
    SDA_LOW = (digitalRead(_sdaPin) == LOW); //   and check SDA input again and loop
  }
  if (SDA_LOW) { // still low
    return 3; // I2C bus error. Could not clear. SDA data line held low
  }

  // else pull SDA line low for Start or Repeated Start
  pinMode(_sdaPin, INPUT); // remove pullup.
  pinMode(_sdaPin, OUTPUT);  // and then make it LOW i.e. send an I2C Start or Repeated start control.
  // When there is only one I2C master a Start or Repeat Start has the same function as a Stop and clears the bus.
  /// A Repeat Start is a Start occurring after a Start with no intervening Stop.
  delayMicroseconds(10); // wait >5uS
  pinMode(_sdaPin, INPUT); // remove output low
  pinMode(_sdaPin, INPUT_PULLUP); // and make SDA high i.e. send I2C STOP control.
  delayMicroseconds(10); // x. wait >5uS
  pinMode(_sdaPin, INPUT); // and reset pins as tri-state inputs which is the default state on reset
  pinMode(_sclPin, INPUT);
  return 0; // all ok
}


