#include <WiFi.h>
// secret values for SSID/Password
#include "credentials.h"
#include "status.h"

WiFiServer server(80);

void serverSetup()
{
    Serial.println("Setting up AP...");
    WiFi.mode(WIFI_AP);
    WiFi.softAP(SSID, AP_PASSWORD);
    delay(500);
    Serial.print("SSID: ");
    Serial.println(SSID);
    Serial.print("IP: ");
    Serial.println(WiFi.softAPIP());
    server.begin();
}

void serverLoop()
{
    WiFiClient client = server.available();
    if (client)
    {
        Serial.println("new client:");
        Serial.println(client.remoteIP());

        bool currentLineBlank = true;
        String currentLine = "";

        while (client.connected())
        {
            if (client.available())
            {
                char c = client.read();
                currentLine += c;
                Serial.write(c);

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
                else if (currentLine.endsWith("$POST"))
                {
                    String temperatureLine = currentLine.substring(0, currentLine.length() - 5); // cropping für die Daten vom Sensor-ESP: z.b. "27.5$POST" -> "27.5"
                    tempIn = temperatureLine.toFloat();
                }

                statusControl();

                if (c == '\n' && currentLineBlank)
                {
                    Serial.println("Sending answer!");

                    client.print("HTTP/1.1 200 OK");
                    client.print("Content-Type:text/html");
                    client.print("Connection: close");
                    client.println();

                    client.print("<!DOCTYPE HTML>");
                    client.print("<head>");
                    client.print("<title>Austellung</title>");
                    client.print("<style>");
                    client.print("body {background-color: lightgray;font-family: sans-serif;text-align: center;}");
                    client.print("div {font-size: 160%;background-color: white;margin-top: 15px;margin-left: 5%;margin-right: 5%;}");
                    client.print("button {background-color:white;font-size: 160%;border: 0px;border-radius: 7px;padding: 20px;margin: 5px;box-shadow: 5px 5px 0px 0px rgb(0, 0, 0, 0.1);}");
                    client.print("button:hover {box-shadow: 5px 5px 0px 0px rgb(0, 0, 0, 0.5);}");
                    client.print("button:active { background-color: rgb(159, 159, 159); }");
                    client.print("table { font-size: 160%; background-color: rgb(235, 235, 235); margin-top: 15px; margin-left: auto; margin-right: auto; padding: 20px; border-spacing: 30px 0px; }");
                    client.print("on { font-weight: bold; color:green; } off { font-weight: bold; color:red; }");
                    client.print("</style>");
                    client.print("</head>");

                    client.print("<html>");
                    client.print("<h1>Physikausstellung<br><b>Dashboard</b></h1>");

                    client.print(mode == "Zeitgeschaltet" ? "<button><b>Zeitgeschaltet</b></button>" : "<a href=\"/zeit\"><button>Standby</button></a>");
                    client.print(mode == "An" ? "<button><b>An</b></button>" : "<a href=\"/an\"><button>An</button></a>");
                    client.print(mode == "Standby" ? "<button><b>Standby</b></button>" : "<a href=\"/standby\"><button>Standby</button></a>");

                    client.print("<div>");
                    client.print(hour < 10 ? "0" : "");
                    client.print(hour);
                    client.print(":");
                    client.print(minute < 10 ? "0" : "");
                    client.print(minute);
                    client.print(":");
                    client.print(second < 10 ? "0" : "");
                    client.print(second);
                    client.print("<br>");
                    client.print(day);
                    client.print(".");
                    client.print(month);
                    client.print(".");
                    client.print(year);
                    client.print("<br> Heute: <b>");
                    client.print(fanToday ? "Ventilator" : "Lampe");
                    client.print("</b></div>");

                    client.print("<div>Aktueller Modus: <br><b>");
                    client.print(mode);
                    client.print("</b></div>");

                    

                    client.print("<div>Lampe: ");
                    client.print(lamp ? "<on>An</b></on>" : "<off>Aus</off>");
                    client.print("<br>Ventilator: ");
                    client.print(fan ? "<on>An</on>" : "<off>Aus</off>");
                    client.print("<br>Waermepumpe: ");
                    client.print(heat ? "<on>An</on>" : "<off>Aus</off>");
                    client.print("</div>");

                    client.print("<table> <tr> <th>Ventilator/Lampe</th> <th>Wärmepumpe</th> </tr> <tr> <th>09:20 - 09:25<br>11:05 - 11:35</th> <th>08:55 - 09:26<br>10:40 - 11:31</th> </tr> </table>");
                    client.print("</html>");
                    client.println();

                    break;
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
}