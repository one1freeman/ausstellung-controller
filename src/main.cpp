#include <U8g2lib.h>
#include <Wire.h>
#include <DHT.h>
#include "pins.h"
#include "webserver.h"

U8G2_SH1106_128X64_NONAME_F_HW_I2C display(U8G2_R0, /* reset=*/U8X8_PIN_NONE);
u8g2_uint_t offset;
u8g2_uint_t width;

DHT tempSensor(TEMPERATURE, DHT11);

unsigned long lastUpdate;
unsigned long currentTime;

void setup()
{
  Serial.begin(9600);
  delay(200);

  if (!display.begin())
  {
    Serial.println("Display not found!");
  }
  
  display.clearBuffer();
  display.setFont(u8g2_font_logisoso32_tf);
  display.setFontMode(0);
  display.firstPage();
  display.drawUTF8(0, 0, "HELLO WORLD!");
  delay(5000);
  display.nextPage();
  
  rtcSetup();
  
  tempSensor.begin();
  delay(200);

  pinMode(LAMP, OUTPUT);
  pinMode(FAN, OUTPUT);
  pinMode(HEAT, OUTPUT);

  serverSetup();

  mode = "Zeitgeschaltet";
  
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
  
  // Optimisierung(?), update nur alle 500 ms
  if (currentTime > lastUpdate + 2000)
  {
    lastUpdate = currentTime;

    // temp display
    if (mode != "Standby")
    {
      
    }
    else
    {
      
    }

    statusControl();
    digitalWrite(LAMP, lamp ? HIGH : LOW);
    digitalWrite(FAN, fan ? HIGH : LOW);
    digitalWrite(HEAT, heat ? HIGH : LOW);
  }
}
