#include <WiFi.h>
#include <clock.h>
// secret values for SSID/Password
#include "credentials.h"
#include "status.h"

WiFiServer server(80);

void serverSetup() {
    Serial.println("Setting up AP...");
    WiFi.softAP(SSID, AP_PASSWORD);
    Serial.print("SSID: ");
    Serial.println(SSID);
    Serial.print("IP: ");
    Serial.println(WiFi.softAPIP());
}

void serverLoop()
{
    WiFiClient client = server.available();
    if (client)
    {
        String time = hour + ":" + minute;
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
                    client.print(hour < 10 ? "0" : "");
                    client.print(hour);
                    client.print(":");
                    client.print(minute < 10 ? "0" : "");
                    client.print(minute);
                    client.print(":");
                    client.print(second < 10 ? "0" : "");
                    client.println(second);
                    client.println("<br>");
                    client.print(day);
                    client.print(".");
                    client.print(month);
                    client.print(".");
                    client.println(year);
                    client.println(" dBm<br>Aktueller Modus: ");
                    client.println(mode);
                    client.println("<br>");

                    client.println(mode == "Zeitgeschaltet" ? "" : "<a href=\"/an\"><button>Zeitgeschaltet</button></a><br>");
                    client.println(mode == "An" ? "" : "<a href=\"/an\"><button>An</button></a><br>");
                    client.println(mode == "Standby" ? "" : "<a href=\"/an\"><button>Standby</button></a><br>");
                
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