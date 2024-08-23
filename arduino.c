#include <Wire.h>
#include "MAX30105.h"
#include "heartRate.h"
MAX30105 particleSensor;
#include <LiquidCrystal.h>

const int rs = 7, en = 6, d4 = 5, d5 = 4, d6 = 3, d7 = 2;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);
const byte RATE_SIZE = 4; //Increase this for more averaging. 4 is good.
byte rates[RATE_SIZE]; //Array of heart rates
byte rateSpot = 0;
long lastBeat = 0; //Time at which the last beat occurred
float beatsPerMinute;
int beatAvg;
int ecgs=A0;
void setup()
{
  Serial.begin(9600);
  //Serial.println("Initializing...");

  // Initialize sensor
  if (!particleSensor.begin(Wire, I2C_SPEED_FAST)) //Use default I2C port, 400kHz speed
  {
    //Serial.println("MAX30105 was not found. Please check wiring/power. ");
    while (1);
  }
  //Serial.println("Place your index finger on the sensor with steady pressure.");

  particleSensor.setup(); //Configure sensor with default settings
  particleSensor.setPulseAmplitudeRed(0x0A); //Turn Red LED to low to indicate sensor is running
  particleSensor.setPulseAmplitudeGreen(0); //Turn off Green LED
  particleSensor.enableDIETEMPRDY(); 
  lcd.begin(16, 2);
  lcd.print("   WELCOME");
  lcd.setCursor(0,1);
  lcd.print("RES:");
}

void loop()
{
  long irValue = particleSensor.getIR();
  float temp = particleSensor.readTemperature();
  int ecg=analogRead(ecgs);

  if (checkForBeat(irValue) == true)
  {
    //We sensed a beat!
    long delta = millis() - lastBeat;
    lastBeat = millis();

    beatsPerMinute = 60 / (delta / 1000.0);

    if (beatsPerMinute < 255 && beatsPerMinute > 20)
    {
      rates[rateSpot++] = (byte)beatsPerMinute; //Store this reading in the array
      rateSpot %= RATE_SIZE; //Wrap variable

      //Take average of readings
      beatAvg = 0;
      for (byte x = 0 ; x < RATE_SIZE ; x++)
        beatAvg += rates[x];
      beatAvg /= RATE_SIZE;
    }
  }

  Serial.print("PPG:");
  Serial.print(irValue/1000);
  Serial.print(",BPM:");
  Serial.print(beatAvg);
  Serial.print(",TMP:");
  Serial.print(temp);
  Serial.print(",ECG:");
  Serial.print(ecg/10);

  if (irValue < 50000)
  {
    Serial.print("PPG:");
  Serial.print(0);
  Serial.print(",BPM=");
  Serial.print(0);
  Serial.print(",TEMP=");
  Serial.print(0);
  Serial.print(",ECG:");
  Serial.print(0);
  }
    

  Serial.println();
}
