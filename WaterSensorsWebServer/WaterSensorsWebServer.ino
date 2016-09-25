
#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <WiFiClient.h>
#include <Wire.h>
//#include <SoftwareSerial.h>

//SoftwareSerial unoSerial(3, 1); // RX, TX 13, 15

int _sdaPin = 4;
int _sclPin = 5;

const char *ssid = "One Love";//"One Love";//"SMU_Aruba_WiFi";
const char *password = "teddy1207";//teddy1207";
const char *hostName = "WaterSensor-1";

// TCP server at port 80 will respond to HTTP requests
WiFiServer server(80);

const byte _slave = 8;
uint8_t _wireReqLength = 20;
uint8_t _wireRespLength = 10;

String ProcessRequest(String req){
  String response = "";
  if (req == "/GetSensorVals")
  {
    response = GetSensorVals();
  }
  else if (req.indexOf("/SetTDSOffset") != -1)
  {
    //..../SetTDSOffset/1210
    String offset = SplitString(req,'/',2);
   
    PrintDebug("Parsed TDS Offset: " + offset);
    
    //"tdsoffset=1210"
    response = SetTDSOffset(offset);
  }
   else if (req.indexOf("/SetPHOffset") != -1)
  {
    //..../SetPHOffset/3.1
    String offset = SplitString(req,'/',2);
   
    PrintDebug("Parsed PH Offset: " + offset);
    
    //"phoffset=1210"
    response = SetPHOffset(offset);
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
  
  response = GetSensorVals();
  response = "PH Offset Updated to: " + val + "\"\r\n" + response;
  return response;
}

String SetTDSOffset(String val){
  String response="";

  Transmit("/SetTDSOffset");
  //"tdsoffset=1210"
  String request = "tdsoffset="+val;
  Transmit(request);
  delay(20);
  
  response = GetSensorVals();
  response = "TDS Offset Updated to: " + val + "\"\r\n" + response;
  return response;
}

String GetSensorVals(){
  String response="";
  
  String ph="";
  String phAvg="";
  String tds="";
  String tdsAvg="";
  String phOffset="";
  String tdsOffset="";

  Transmit("/GetSensorVals");
   
  ph = Request();
  phAvg = Request();
  tds = Request();
  tdsAvg = Request();
  phOffset = Request();
  tdsOffset = Request();
  
  delay(20); //let wire clear.
//
  //response = "HTTP/1.1 200 OK\r\nContent-Type: text/plain\r\n\r\n";
  response = "\"host\":\"" + String(hostName) + "\"\r\n";
  response += "\"ph\":\"" + String(ph)+ "\"\r\n";
  response += "\"phAvg\":\"" + String(phAvg)+ "\"\r\n";
  response += "\"tds\":\"" + String(tds)+ "\"\r\n";
  response += "\"tdsAvg\":\"" + String(tdsAvg)+ "\"\r\n";
  response += "\"phOffset\":\"" + String(phOffset)+ "\"\r\n";
  response += "\"tdsOffset\":\"" + String(tdsOffset)+ "\"\r\n";
  
  return response;
  
}

void setup(void)
{  
  Wire.begin(_sdaPin, _sclPin);
  Wire.setClockStretchLimit(15000);
  //Wire.setClockStretchLimit(20000);

  //unoSerial.begin(9600);
  Serial.begin(9600);
  while(!Serial);
  //EstablishContact();
  
//  IPAddress ip(192, 168, 10, 102); 
//  IPAddress gateway(192, 168, 10, 1); 
//  IPAddress subnet(255, 255, 255, 0); 
//  Serial.print(F("Setting static ip to : "));
//  Serial.println(ip);
//  WiFi.config(ip, gateway, subnet);

    // Connect to WiFi network
  WiFi.hostname(hostName);
  WiFi.begin(ssid, password);
  WiFi.hostname(hostName);
  
  PrintDebug("");  
  
  // Wait for connection
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    PrintDebug(".");
    delay(10);
  }
  PrintDebug("");
  PrintDebug("Connected to: " + String(ssid));
  String ipAddress = WiFi.localIP().toString();
  PrintDebug("IP address: " + ipAddress);

  // Set up mDNS responder:
  // - first argument is the domain name, in this example
  //   the fully-qualified domain name is "esp8266.local"
  // - second argument is the IP address to advertise
  //   we send our IP address on the WiFi network
  //if (!MDNS.begin(hostName,ip)) {
   if (!MDNS.begin(hostName)) {
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
}

void loop(void)
{
  
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
    PrintDebug("Invalid request: " + req);
    return;
  }
  req = req.substring(addr_start + 1, addr_end);
  PrintDebug("Request: " + req);
  client.flush();
  
  String response;
  if (req == "/")
  {
    IPAddress ip = WiFi.localIP();
    String ipStr = String(ip[0]) + '.' + String(ip[1]) + '.' + String(ip[2]) + '.' + String(ip[3]);
    response = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n<!DOCTYPE HTML>\r\n<html>Hello from " + String(hostName) + " at ";
    response += ipStr;
    response += "</html>\r\n\r\n";
    
    PrintDebug("Sending 200");
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
  response = ""; //needed to clear serial buffer.
  PrintDebug("Done with client");

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
  Serial.println("~esp~ " + msg);
  Serial.flush();
}
void Transmit(String request) {
  int diff = _wireReqLength - request.length();
  for(int i = 0;i<diff;i++){
    request+=" ";
  }
  PrintDebug("req: " + request);
  PrintDebug("reqLen: " + request.length());

  byte response[_wireReqLength];
  for (byte i=0;i<_wireReqLength;i++) {
    response[i] = (byte)request.charAt(i);
  }
  
  Wire.beginTransmission(_slave); // transmit to device #8
  //Wire.write(request.c_str());        // sends 10 byte
  Wire.write(response,sizeof(response));
  Wire.endTransmission();    // stop transmitting
}

String Request(){
  String wireResponse;
  Wire.requestFrom(_slave,_wireRespLength); //32 bytes is max, so call multiple
  while (Wire.available()) {
      char p = Wire.read();
      
      wireResponse.concat(p);
  }
  PrintDebug("wireResponse: " +wireResponse);
  wireResponse.trim();
  return wireResponse;
}



