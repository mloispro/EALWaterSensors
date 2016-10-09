﻿#include <Arduino.h>
#include <SimpleTimer.h>
#include <avr/wdt.h> //watch dog timer
//#include <EasyTransfer.h>
//#include <Wire.h>

#include <StandardCplusplus.h>
#include <vector>
#include <string>
using namespace std;

#include "_globalsWaterSensors.h"
#include "MathExt.h"

#include "WaterSensorWire.h"
//#include "CmdMessengerExt.h"
using namespace Globals;

SimpleTimer _asyncTimer;


void AsyncDoWork();

void setup(void) {
    // Listen on serial connection for messages from the pc
    Serial.begin(57600);
    while(!Serial);

    wdt_enable(WDTO_8S);

    WaterSensorWire::Setup();
    ThePHSensor.TurnOn();

    _asyncTimer.setInterval(1000, AsyncDoWork); //todo: set to 1000;

    //CmdMessengerExt::Init();
}

//String _request;
//String _response;
void loop(void) {

    _asyncTimer.run();

    ThePHSensor.PrintPHToLCD();
    TheTDSSensor.PrintTDSToLCD(); //todo: uncomment this

    //CmdMessengerExt::Loop();
}

void AsyncDoWork() {

    wdt_reset();

    static unsigned long lastSensorReadTime = millis();

    if(TheLCD.DetectKeyPress() == LcdKeyPress::Select) {
        Serial.println(F("[Selelct] Pressed"));
        SwitchSensors();
        lastSensorReadTime = millis();
    }
    SensorReadDuration = millis() - lastSensorReadTime;
    WaterSensorWire::Loop();
    if(SensorReadDuration > SensorReadInterval) {

        SwitchSensors();
        lastSensorReadTime = millis();
    }
    if(ReadingTDS) {
        TheTDSSensor.CalculateTDS();
    }
    else {
        ThePHSensor.CalculatePH();
    }

    //WaterSensorWire::Scan();
    //WaterSensorWire::I2C_ClearBus();
}






