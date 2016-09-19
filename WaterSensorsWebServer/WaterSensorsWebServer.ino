/*
  ESP8266 mDNS responder sample

  This is an example of an HTTP server that is accessible
  via http://esp8266.local URL thanks to mDNS responder.

  Instructions:
  - Update WiFi SSID and password as necessary.
  - Flash the sketch to the ESP8266 board
  - Install host software:
    - For Linux, install Avahi (http://avahi.org/).
    - For Windows, install Bonjour (http://www.apple.com/support/bonjour/).
    - For Mac OSX and iOS support is built in through Bonjour already.
  - Point your browser to http://esp8266.local, you should see a response.

 */


#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <WiFiClient.h>
#include <Wire.h>



int _sdaPin = 4;
int _sclPin = 5;

const char *ssid = "One Love";
const char *password = "teddy1207";
const char *hostName = "WaterSensor-1";

String _ph;
String _tds;
String _phAvg;
String _tdsAvg;

// TCP server at port 80 will respond to HTTP requests
WiFiServer server(80);

String GetSensorVals(){
  String response="";
  String wireResponse="";
  _ph="";
  _tds="";
  _phAvg="";
  _tdsAvg="";

  Wire.requestFrom(9,30);
  while (Wire.available()) {
      char p = Wire.read();
      wireResponse += p;
  }
  
  Serial.println("wireResponse: " +wireResponse);
  int end = wireResponse.indexOf(";");
  wireResponse.remove(end);
  _ph = SplitString(wireResponse, ',', 0);
  _tds = SplitString(wireResponse, ',', 1);
  _phAvg = SplitString(wireResponse, ',', 2);
  _tdsAvg = SplitString(wireResponse, ',', 3);

  response = "\"host\":\"" + String(hostName) + "\"\r\n";
  response += "\"ph\":\"" + String(_ph)+ "\"\r\n";
  response += "\"tds\":\"" + String(_tds)+ "\"\r\n";
  response += "\"phAvg\":\"" + String(_phAvg)+ "\"\r\n";
  response += "\"tdsAvg\":\"" + String(_tdsAvg)+ "\"\r\n";
  
  return response;
  
}

void setup(void)
{  
  Serial.begin(115200);
  
  IPAddress ip(192, 168, 10, 102); 
  IPAddress gateway(192, 168, 10, 1); 
  Serial.print(F("Setting static ip to : "));
  Serial.println(ip);
  IPAddress subnet(255, 255, 255, 0); 
  WiFi.config(ip, gateway, subnet);

    // Connect to WiFi network
  WiFi.hostname(hostName);
  WiFi.begin(ssid, password);
  WiFi.hostname(hostName);
  
  Serial.println("");  
  
  // Wait for connection
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  // Set up mDNS responder:
  // - first argument is the domain name, in this example
  //   the fully-qualified domain name is "esp8266.local"
  // - second argument is the IP address to advertise
  //   we send our IP address on the WiFi network
  if (!MDNS.begin(hostName,ip)) {
    Serial.println("Error setting up MDNS responder!");
    while(1) { 
      delay(1000);
    }
  }
  Serial.println("mDNS responder started");
  
  // Start TCP (HTTP) server
  server.begin();
  Serial.println("TCP server started");
  
  // Add service to MDNS-SD
  MDNS.addService("http", "tcp", 80);

  Wire.begin(_sdaPin, _sclPin);
  Wire.setClockStretchLimit(15000);

}

void loop(void)
{
  // Check if a client has connected
  WiFiClient client = server.available();
  if (!client) {
    return;
  }
  Serial.println("");
  Serial.println("New client");

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
    Serial.print("Invalid request: ");
    Serial.println(req);
    return;
  }
  req = req.substring(addr_start + 1, addr_end);
  Serial.print("Request: ");
  Serial.println(req);
  client.flush();
  
  String response;
  if (req == "/")
  {
    IPAddress ip = WiFi.localIP();
    String ipStr = String(ip[0]) + '.' + String(ip[1]) + '.' + String(ip[2]) + '.' + String(ip[3]);
    response = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n<!DOCTYPE HTML>\r\n<html>Hello from " + String(hostName) + " at ";
    response += ipStr;
    response += "</html>\r\n\r\n";
    
    Serial.println("Sending 200");
  }
  else if (req == "/GetSensorVals")
  {
    response = GetSensorVals();
  }
  else
  {
    response = "HTTP/1.1 404 Not Found\r\n\r\n";
    Serial.println("Sending 404");
  }
  
  client.print(response);
  Serial.println("Done with client");

}
String SplitString(String data, char separator, int index)
{
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

