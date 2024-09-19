#include <SPI.h>
#include <WiFi.h>
#include <Wire.h>
#include <I2C_RTC.h>
#include <OLED-Display-SOLDERED.h>
#include <OneWire.h>
#include <DallasTemperature.h>

// pins
#define OW_TEMP 13
#define LAMP 25
#define FAN 26
#define HEAT 27

#define TEMP_INSIDE 0
#define TEMP_OUTSIDE 1

String ssid = "FRITZ!Box 7490";
String pass = "14460688476998183101";

WiFiServer server(80);

static DS1307 RTC;

OLED_Display display;

OneWire tempWire(OW_TEMP);
DallasTemperature sensors(&tempWire);

bool timer;

bool lamp;
bool fan;
bool heat;

bool fanToday;

String mode;

void setup()
{
  Serial.begin(9600);
  delay(200);

  display.begin();

  RTC.begin();

  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.println("Starting...");
  display.display();

  sensors.begin();
  delay(200);

  pinMode(LAMP, OUTPUT);
  pinMode(FAN, OUTPUT);
  pinMode(HEAT, OUTPUT);

  // attempt to connect to Wifi network:
  WiFi.begin(ssid, pass);
  while (WiFi.status() != WL_CONNECTED)
  {
    Serial.print(".");
    if (WiFi.status() == WL_CONNECT_FAILED)
    {
      WiFi.begin(ssid, pass);
      Serial.println(".");
      Serial.print("Connection failed, retrying.");
    }
    delay(1000);
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  Serial.println("RSSI:");
  Serial.println(WiFi.RSSI());

  server.begin();

  mode = "Zeitgeschaltet";

  Serial.println("Setup finished at: ");
  printTime();
  delay(100);
}

void loop()
{
  int weekday = RTC.getWeek();
  int day = RTC.getDay();
  int hour = RTC.getHours();
  int minute = RTC.getMinutes();
  int second = RTC.getSeconds();
  // server handling

  WiFiClient client = server.available();
  if (client)
  {
    String time = RTC.getHours() + ":" + RTC.getMinutes();
    Serial.println("new client");

    bool currentLineBlank = true;
    String currentLine = "";

    while (client.connected())
    {

      if (client.available())
      {
        char c = client.read();
        currentLine += c;
        Serial.write(c);

        if (c == '\n' && currentLineBlank)
        {
          client.println("HTTP/1.1 200 OK");
          client.println("Content-Type:text/html");
          client.println("Connection: close");
          client.println();
          client.println("<!DOCTYPE HTML>");
          client.println("<html>");
          client.println("<p style=\"font-family: arial; font-size:  20px\">");
          client.println("");
          client.print(hour);
          client.print(":");
          client.print(minute);
          client.print(":");
          client.println(second);
          client.println("<br>RSSI:");
          client.println(WiFi.RSSI());
          client.println(" dBm<br>Aktueller Modus: ");
          client.println(mode);
          if (mode == "Zeitgeschaltet")
          {
            client.println("<br><a href=\"/an\"><button>An</button></a>");
            client.println("<a href=\"/standby\"><button>Standby</button></a>");
          }
          else if (mode == "An")
          {
            client.println("<br><a href=\"/zeit\"><button>Zeitgeschaltet</button></a>");
            client.println("<a href=\"/standby\"><button>Standby</button></a>");
          }
          else
          {
            client.println("<br><a href=\"/zeit\"><button>Zeitgeschaltet</button></a>");
            client.println("<a href=\"/an\"><button>An</button></a>");
          }
          client.println("<br> Heute: ");
          client.println(fanToday ? "Ventilator" : "Lampe");
          client.println("<br>Lampe: ");
          client.println(lamp ? "An" : "Aus");
          client.println("<br>Ventilator: ");
          client.println(fan ? "An" : "Aus");
          client.println("<br>Waermepumpe: ");
          client.println(heat ? "An" : "Aus");
          client.println("</p>");
          client.println("</html>");
          client.println();

          break;
        }

        if (currentLine.endsWith("GET /zeit"))
        {
          mode = "Zeitgeschaltet";
        }
        else if (currentLine.endsWith("GET /an"))
        {
          mode = "An";
        }
        else if (currentLine.endsWith("GET /standby"))
        {
          mode = "Standby";
        }

        if (c == '\n')
        {
          currentLineBlank = true;
        }
        else if (c != '\r')
        {
          currentLineBlank = false;
        }
      }
    }

    delay(1);
    client.stop();
    Serial.println("client disconnected");
    printTime();
  }

  // temp display
  if (mode != "Standby")
  {
    sensors.requestTemperatures();

    display.clearDisplay();

    display.setCursor(0, 0);
    display.print("Innen: ");
    display.print(sensors.getTempCByIndex(TEMP_INSIDE));
    display.print("C");

    display.setCursor(0, 10);
    display.print("Aussen: ");
    display.print(sensors.getTempCByIndex(TEMP_OUTSIDE));
    display.print("C");

    display.display();
  }
  else
  {
    display.clearDisplay();
    display.setCursor(0, 0);
    display.print("Standby");
    display.display();
  }

  // Wifi reconnect

  if (WiFi.status() != WL_CONNECTED && WiFi.status() != WL_IDLE_STATUS)
  {
    Serial.println("Disconnected, attempting to reconnect");
    WiFi.begin(ssid, pass);
  }

  if (day % 2 == 0)
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

void printTime()
{
  Serial.print(RTC.getHours());
  Serial.print(":");
  Serial.print(RTC.getMinutes());
  Serial.print(":");
  Serial.println(RTC.getSeconds());
  Serial.print(RTC.getDay());
  Serial.print(".");
  Serial.print(RTC.getMonth());
  Serial.print(".");
  Serial.println(RTC.getYear());
}