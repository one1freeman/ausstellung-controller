#include <SPI.h>
#include <Wire.h>
#include <OLED-Display-SOLDERED.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include "pins.h"
#include "webserver.h"

OLED_Display display;

OneWire tempWire(OW_TEMP);
DallasTemperature sensors(&tempWire);

unsigned long lastUpdate;
unsigned long currentTime;

void relay (String modul, bool modus) {
  if (modul.equals("lamp"))
  {
    digitalWrite(LAMP, modus);
  } else if (modul.equals("fan"))
  {
    digitalWrite(FAN, modus);
  } else if (modul.equals("heat"))
  {
    digitalWrite(HEAT, modus);
  }
}

void setup()
{
  Serial.begin(9600);
  delay(200);

  display.begin();

  RTC.begin();

  display.clearDisplay();
  display.setTextSize(2);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.println("Starting...");
  display.display();

  sensors.begin();
  delay(200);

  pinMode(LAMP, OUTPUT);
  pinMode(FAN, OUTPUT);
  pinMode(HEAT, OUTPUT);

  Serial.println("");
  serverSetup();

  server.begin();

  mode = "Zeitgeschaltet";

  display.println("HELLO :)");
  display.println(mode);
  display.display();

  Serial.println("Setup finished at: ");
  printTime();
  delay(100);
}

void loop()
{
  // server handling
  serverLoop();

  currentTime = millis();
  
  // Optimisierung(?), update nur alle 500 ms
  if (currentTime > lastUpdate + 500)
  {
    lastUpdate = currentTime;

    // temp display
    if (mode != "Standby")
    {
      sensors.requestTemperatures();
  
      display.clearDisplay();
  
      display.setCursor(0, 0);
      display.print("IN:  ");
      display.print(sensors.getTempCByIndex(TEMP_INSIDE));
  
      display.setCursor(0, 20);
      display.print("OUT: ");
      display.print(tempIn);
  
      display.display();
    }
    else
    {
      display.clearDisplay();
      display.setCursor(0, 0);
      display.print("Standby");
      display.display();
    }

    relay("lamp", lamp);
    relay("fan", fan);
    relay("heat", heat);
  }

  delay(50);
}
