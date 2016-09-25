#include <Arduino.h>
#include <SimpleTimer.h>
#include <Wire.h>

#include <StandardCplusplus.h>
#include <vector>
#include <string>
using namespace std;

#include "_globalsWaterSensors.h"
#include "WaterSensorWire.h"
//#include "CmdMessengerExt.h"
using namespace Globals;

SimpleTimer _asyncTimer;

void AsyncDoWork();

void setup(void) {
    // Listen on serial connection for messages from the pc
    Serial.begin(9600);
    while(!Serial);

    WaterSensorWire::Setup();

    _asyncTimer.setInterval(1000, AsyncDoWork);

    //CmdMessengerExt::Init();
}

void loop(void) {

    _asyncTimer.run();

    ThePHSensor.PrintPHToLCD();
    TheTDSSensor.PrintTDSToLCD(); //todo: uncomment this

    //CmdMessengerExt::Loop();
}

void AsyncDoWork() {
    ThePHSensor.CalculatePH();
    TheTDSSensor.CalculateTDS();//todo: uncomment this
}




