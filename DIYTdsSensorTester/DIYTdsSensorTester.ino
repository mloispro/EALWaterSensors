#include <LiquidCrystal.h>

// how many samples to take and average, more takes longer
// but is more 'smooth'
#define NUMSAMPLES 5

//setup vars
const int _tdsPin = 3;//1;//#3
const int _temperaturePin = A3;
const int _tdsPowerPin = 11; // the pin that powers the 555 subcircuit

//local vars
int _samples[NUMSAMPLES];
float Volts = 5.0;

 //constants
const int TEMP_MANUAL = 25; // temp. for nominal resistance (almost always 25 C)
const int SAMPLING_PERIOD = 2; // the number of seconds to measure 555 oscillations
const long THERMISTOR_NOMINAL = 10000; //*DONT EVER CHANGE resistance at 25 degrees C
const long SERIES_RESISTOR = 10000;  //*DONT EVER CHANGE the value of the 'other' resistor     
const long BETA_COEFFICIENT = 3950; // The beta coefficient of the thermistor (usually 3000-4000)
const float ALPHA_LTC = 0.022; // Temperature correction coefficient



// conductivity variables
long _pulseCount = 0;  //a pulse counter variable
unsigned long _pulseTime,_lastTime, _duration, _totalDuration;
float Temp, TempF, TDS;

//lcd
LiquidCrystal _lcd(8, 9, 4, 5, 6, 7); 

void setup()
{
  Serial.begin(57600);
  
  pinMode(_tdsPowerPin, OUTPUT);   
  _lcd.begin(16, 2);
  
  // Print a message to the LCD.
  _lcd.print("TDS Tester");
           
}

void lcdPrintLine(int lineNum, String text){
  _lcd.setCursor(0, lineNum);
  // print the number of seconds since reset:
  _lcd.print(text);
}

void loop()
{
  getReadings();
}

void printValsToLCD(){
    

    String tdsString = "TDS:" + String(TDS, 0)+ "," + String(TempF, 1) + "F";
    
    lcdPrintLine(0, tdsString);
}

void printValsToSerial(){
  Serial.println(' ');
  Serial.print("  t ");
  Serial.print(Temp, 1);
  Serial.print("*C ");
  Serial.print(" , ");
  Serial.print(TempF, 1);
  Serial.print("*F ");
  Serial.print("  TDS ");
  Serial.print(TDS, 2);
  Serial.print("   PPM ");
  Serial.print(TDS * 500, 0);
  Serial.print("   Frequency ");
  Serial.print(_pulseCount);
  Serial.println(" Hz");
}

void getReadings(){

  static unsigned long samplingTime = millis();

  long waitTime = SAMPLING_PERIOD * 1000 + 500;//wait .5 sec between readings, according to spec
  if(millis() - samplingTime > waitTime) {

      // now make a conductivity measurement
    float average;
    
      //turn on the 555 system
    digitalWrite(_tdsPowerPin,HIGH); //turns on the 555 timer subcircuit
    
    // temperature ---------------------------------------------
    // take N samples in a row, with a slight delay
    for (int i=0; i< NUMSAMPLES; i++) {
     int tempReading = analogRead(_temperaturePin);
//     Serial.print("tempReading: "); 
//      Serial.println(tempReading);
     _samples[i] = tempReading;
     delay(10);
    }
    
    // average all the samples out
    average = 0;
    for (int i=0; i< NUMSAMPLES; i++) {
       average += _samples[i];
    }
    average /= NUMSAMPLES;
   
    // convert the value to resistance
    average = 1023 / average - 1;
    average = SERIES_RESISTOR / average;
//    Serial.print("Thermistor resistance "); 
//    Serial.println(average);

    float steinhart;
  steinhart = average / THERMISTOR_NOMINAL;     // (R/Ro)
  steinhart = log(steinhart);                  // ln(R/Ro)
  steinhart /= BETA_COEFFICIENT;                   // 1/B * ln(R/Ro)
  steinhart += 1.0 / (TEMP_MANUAL + 273.15); // + (1/To)
  steinhart = 1.0 / steinhart;                 // Invert
  steinhart -= 273.15;                         // convert to C

   Temp = steinhart;
  //  Serial.print(Temp);
  //  Serial.println(" *C");
    
    
    // conductivity --------------------------------------
     
    _pulseCount=0; //reset the pulse counter
    _totalDuration=0;  //reset the totalDuration of all pulses measured
    
    
    attachInterrupt(digitalPinToInterrupt(_tdsPin),onPulse,RISING); //attach an interrupt counter to interrupt pin 1 (digital pin #3) -- the only other possible pin on the 328p is interrupt pin #0 (digital pin #2)
    //attachInterrupt(1,onPulse,RISING);
    
    _pulseTime=micros(); // start the stopwatch

    //todo: change to if
    delay(SAMPLING_PERIOD*1000); //give ourselves samplingPeriod seconds to make this measurement, during which the "onPulse" function will count up all the pulses, and sum the total time they took as 'totalDuration' 
   
    detachInterrupt(digitalPinToInterrupt(_tdsPin)); //we've finished sampling, so detach the interrupt function -- don't count any more pulses
    //detachInterrupt(1);
    
    //turn off the 555 system
    digitalWrite(_tdsPowerPin,LOW);
    
    if (_pulseCount>0) { //use this logic in case something went wrong
    
    double durationS=_totalDuration/double(_pulseCount)/1000000.; //the total duration, in seconds, per pulse (note that totalDuration was in microseconds)
    
    // print out stats
    /*Serial.print("sampling period=");
    Serial.print(samplingPeriod);
      Serial.print(" sec; #pulses=");
    Serial.print(pulseCount);
    Serial.print("; duration per pulse (sec)=");
    */
    
    // print conductivity value
//    Serial.print(durationS,4);
//    Serial.print(", ");
//    Serial.println(Temp);
      
    }
    
    TempF = convTempToFahrenheit(Temp);
    calculateTDS();
    printValsToSerial();
    printValsToLCD();
    
    samplingTime = millis();
  }
  
}

void onPulse()
{
 
  _pulseCount++;
//  Serial.print("pulsecount=");
//  Serial.println(pulseCount);
  _lastTime = _pulseTime;
  _pulseTime = micros();
  _duration=_pulseTime-_lastTime;
  _totalDuration+=_duration;
  //Serial.println(totalDuration);
}


const float noEC = 0.00;   //  The conductivity probe is dry
const float lowEC = 2.00;   //  Value of calibration buffers
const float medEC = 12.88;  //  You can use any other buffers
const float medHighEC = 60.00;
const float highEC = 80.00;  

//calibration vars, these change after calibration
int noPulse = 230;
int lowPulse = 1340;
int medPulse = 4140;
int medHighPulse = 5100;
int highPulse = 7350;

//pen   diy   lvl   match
//-------------------------
//90    70    high  no       <-need intermediate level
//180   180   high  yes

void calculateTDS() {  //  Calculate the measurement

  float a;
  float b;
  float c = 0;

  long lowPulseCount = (noPulse + lowPulse);
  long medPulseCount = (medPulse + lowPulse + noPulse);
  long medHighPulseCount = (medHighPulse + medPulse + lowPulse + noPulse);

  int higherPulseVar;
  int lowerPulseVar;
  float higherEcVar;
  float lowerEcVar;
  
  if (_pulseCount >= noPulse && _pulseCount < lowPulseCount) { //low
    Serial.println("calc case - low");
    higherPulseVar = lowPulse;
    lowerPulseVar = noPulse;
    higherEcVar = lowEC;
    lowerEcVar = noEC;
  }
  else if (_pulseCount >= lowPulseCount && _pulseCount < medPulseCount) { //med
    Serial.println("calc case - med");
    higherPulseVar = medPulse;
    lowerPulseVar = lowPulse;
    higherEcVar = medEC;
    lowerEcVar = lowEC;
  }
  else if (_pulseCount >= medPulseCount && _pulseCount < medHighPulseCount) { //med high
    Serial.println("calc case - med high");
    higherPulseVar = medHighPulse;
    lowerPulseVar = medPulse;
    higherEcVar = medHighEC;
    lowerEcVar = medEC;
  }
  else if (_pulseCount >= medHighPulseCount) { //high
    Serial.println("calc case - high");
    higherPulseVar = highPulse;
    lowerPulseVar = medPulse;
    higherEcVar = highEC;
    lowerEcVar = medEC;
//     higherPulseVar = highPulse;
//    lowerPulseVar = medHighPulse;
//    higherEcVar = highEC;
//    lowerEcVar = medHighEC;
  }
//  else if (_pulseCount >= medPulseCount) { //high
//    Serial.println("calc case - high");
//    a = (highPulse - medPulse) / (highEC - medEC);
//    b = medPulse - (a * medEC);
//    c = (_pulseCount - b) / a;
//  }

  if(_pulseCount > noPulse){
    a = (higherPulseVar - lowerPulseVar) / (higherEcVar - lowerEcVar);
    b = lowerPulseVar - (a * lowerEcVar);
    c = (_pulseCount  - b) / a;
  }
  else{
    Serial.println("calc case -none");
    c=0;
  }

  TDS = (c / (1 + ALPHA_LTC * (Temp - 25.00))); //  temperature compensation
}
double convTempToFahrenheit(double temp) {
    float vIn = Volts;//3.1;//5;
    if (vIn < 5.0) {
        float voltDiff = 5.0 - vIn; //0, 1 ,2
        vIn = 5.0 + voltDiff;
    }

    double  tempF = (temp * 9.0) / vIn + 32.0;
    return tempF;
}


