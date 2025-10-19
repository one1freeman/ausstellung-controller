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

void relay (String modul, bool modus) {
  if (modul.equals("lamp"))
  {
    lamp = modus;
    digitalWrite(LAMP, modus ? HIGH : LOW);
  } else if (modul.equals("fan"))
  {
    fan = modus;
    digitalWrite(FAN, modus ? HIGH : LOW);
  } else if (modul.equals("heat"))
  {
    heat = modus;
    digitalWrite(HEAT, modus ? HIGH : LOW);
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


  // alle zwei Tage Windkraft, ansonsten Solar
  if (weekday % 2 == 0)
  {
    fanToday = true;
  } else
  {
    fanToday = false;
  }

  // relay control
  if (mode == "Zeitgeschaltet")
  {
    if (weekday != 1 && weekday != 7)
    {
      // Wind
      if (fanToday)
      {
        if (hour == 9 && minute > 20 && minute < 45)
        {
          relay("fan", true);
        }
        else if (hour == 11 && minute > 5 && minute < 35)
        {
          relay("fan", true);
        }
        else
        {
          relay("fan", false);
        }

        relay("lamp", false);
      }
      else
      { 
        // Solar
        if (hour == 9 && minute > 20 && minute < 45)
        {
          relay("lamp", true);
        }
        else if (hour == 11 && minute > 5 && minute < 35)
        {
          relay("lamp", true);
        }
        else
        {
          relay("lamp", false);
        }

        relay("fan", false);
      }

      // heat
      if (hour == 8 && minute > 55)
      {
        relay("heat", true);
      }
      else if (hour == 9 && minute < 26)
      {
        relay("heat", true);
      }
      else if (hour == 10 && minute > 40)
      {
        relay("heat", true);
      }
      else if (hour == 11 && minute < 31)
      {
        relay("heat", true);
      }
      else
      {
        relay("heat", false);
      }
    } else
    {
      relay("lamp", false);
      relay("fan", false);
      relay("heat", false);
    }
    
  }
  else if (mode == "An")
  {
    relay("lamp", true);
    relay("fan", true);
    relay("heat", true);
  }
  else
  {
    relay("lamp", false);
    relay("fan", false);
    relay("heat", false);
  }

  delay(50);
}
