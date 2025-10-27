#include <OLED-Display-SOLDERED.h>
#include <DHT.h>
#include "pins.h"
#include "webserver.h"

OLED_Display display;

DHT tempSensor(TEMPERATURE, DHT11);

unsigned long lastUpdate;
unsigned long currentTime;

void setup()
{
  Serial.begin(9600);
  delay(200);

  display.begin();

  rtcSetup();

  display.clearDisplay();
  display.setTextSize(2);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.println("Starting...");
  display.display();

  tempSensor.begin();
  delay(200);

  pinMode(LAMP, OUTPUT);
  pinMode(FAN, OUTPUT);
  pinMode(HEAT, OUTPUT);

  serverSetup();

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
      display.clearDisplay();
  
      display.setCursor(0, 0);
      display.print("OUT: ");
      display.print(tempSensor.readTemperature());
  
      display.setCursor(0, 20);
      display.print("IN: ");
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

    statusControl();
    digitalWrite(LAMP, lamp ? HIGH : LOW);
    digitalWrite(FAN, fan ? HIGH : LOW);
    digitalWrite(HEAT, heat ? HIGH : LOW);
  }
}
