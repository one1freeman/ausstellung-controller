#include <U8g2lib.h>
#include <Wire.h>
#include <DHT.h>
#include "pins.h"
#include "server.h"

U8G2_SH1106_128X64_NONAME_F_HW_I2C display(U8G2_R0, /* reset=*/ U8X8_PIN_NONE);

DHT tempSensor(TEMPERATURE, DHT11);

unsigned long lastUpdate;
unsigned long currentTime;

void setup()
{
  display.setBusClock(10000);
  display.begin();
  display.setFont(u8g2_font_courB14_tf);
  
  display.clearBuffer();
  display.drawStr(0,10, "Hochfahren...");
  display.sendBuffer();
  delay(500);

  Serial.begin(9600);
  
  rtcSetup();
  
  tempSensor.begin();
  delay(200);

  pinMode(LAMP, OUTPUT);
  pinMode(FAN, OUTPUT);
  pinMode(HEAT, OUTPUT);

  serverSetup();

  mode = 2; // Standardmäßig Zeitgeschaltet
  
  Serial.println("Setup finished at: ");
  printTime();
  delay(100);
}

void loop()
{
  rtcLoop();
  // server handling
  serverLoop();

  currentTime = millis();
  
  // Optimisierung(?), update nur alle 2000 ms
  if (currentTime > lastUpdate + 2000)
  {
    lastUpdate = currentTime;

    // temp display
    if (mode != 0)
    {
      char temp[5];
      dtostrf(tempSensor.readTemperature(), 5, 2, temp);
      display.clearBuffer();
      display.drawStr(0, 30, "INNEN:");
      display.drawStr(75, 30, tempIn.c_str());
      display.drawStr(110, 30, "C");
      
      display.drawStr(0, 54, "AUSSEN:");
      display.drawStr(75, 54, String((int)tempSensor.readTemperature()).c_str());
      display.drawStr(110, 54, "C");

      display.sendBuffer();
      delay(500);
    }
    else
    {
      display.clearBuffer();
      display.drawStr(0,10,"Standby");
      display.sendBuffer();
      delay(500);
    }
    
    statusControl();
    digitalWrite(LAMP, lamp ? HIGH : LOW);
    digitalWrite(FAN, fan ? HIGH : LOW);
    digitalWrite(HEAT, heat ? HIGH : LOW);
  }
}
