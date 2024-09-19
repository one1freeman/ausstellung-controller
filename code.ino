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

String mode;

void setup() {
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
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    if(WiFi.status() == WL_CONNECT_FAILED) {
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

  Serial.println("Setup finished at: ");
  printTime();
  delay(100);
}

void loop() {

  int weekday = RTC.getWeek();
  int day = RTC.getDay();
  int hour = RTC.getHours();
  int minute = RTC.getMinutes();
  int second = RTC.getSeconds();
  // server handling

  WiFiClient client = server.available();
  if (client) {
    String time = RTC.getHours() + ":" + RTC.getMinutes();
    Serial.println("new client");

    bool currentLineBlank = true;
    String currentLine = "";

    while (client.connected()) {

      if (client.available()) {
        char c = client.read();
        currentLine += c;
        Serial.write(c);

        if(c == '\n' && currentLineBlank) {
          client.println("HTTP/1.1 200 OK");
          client.println("Content-Type:text/html");
          client.println("Connection: close");
          client.println();
          client.println("<!DOCTYPE HTML>");
          client.println("<html>");
          client.println("<p style=\"font-family: impact; font-size:  20px\">");
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
          client.println(" <a href=\"/zeit\"><button>Zeitgeschaltet</button></a>");
          client.println(" <a href=\"/an\"><button>Immer An</button></a>");
          client.println("</p>");
          client.println("</html>");
          client.println();

          break;
        }

        if(c == '\n') {
          currentLineBlank = true;
        } else if (c != '\r') {
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


  // Wifi reconnect

  if (WiFi.status() != WL_CONNECTED && WiFi.status() != WL_IDLE_STATUS)
  {
    Serial.println("Disconnected, attempting to reconnect");
    WiFi.begin(ssid, pass);
  }
  
  
  // relay control

  if (weekday != 1 && weekday != 7)
  {
    //solar
    if(day % 2 == 0){
      if (hour == 9 && minute > 20 && minute < 45)
      {
        digitalWrite(LAMP, HIGH);        
      } else if (hour = 11 && minute > 5 && minute < 35)
      {
        digitalWrite(LAMP, HIGH);
      } else
      {
        digitalWrite(LAMP, LOW);
      }
    } else // wind
    {
      if (hour == 9 && minute > 20 && minute < 45)
      {
        digitalWrite(FAN, HIGH);        
      } else if (hour = 11 && minute > 5 && minute < 35)
      {
        digitalWrite(FAN, HIGH);
      } else
      {
        digitalWrite(FAN, LOW);
      }
    }

    // heat
    if (hour == 8 && minute > 55)
    {
      digitalWrite(HEAT, HIGH);
    } else if (hour == 9 && minute < 26)
    {
      digitalWrite(HEAT, HIGH);
    } else if (hour == 10 && minute > 40)
    {
      digitalWrite(HEAT, HIGH);
    } else if (hour == 11 && minute < 31)
    {
      digitalWrite(HEAT, HIGH);
    } else
    {
      digitalWrite(HEAT, LOW);
    }
  }
  
  

  delay(50);
}

void printTime() {
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