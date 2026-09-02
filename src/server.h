#ifndef SERVER_H
#define SERVER_H

#include <WiFi.h>
#include <WebServer.h>
#include <ESPmDNS.h>
// secret values for SSID/Password
#include "credentials.h"
#include "status.h"

WebServer server(80);

void mainPage()
{
    // server.send(200, "text/plain", "Hello from ESP32!");
    char temp[2900];
    snprintf(temp, 2900,
        "<!DOCTYPE html>\
<head>\
    <title>Dashboard</title>\
    <style>\
        body {\
            background-color: lightgray;\
            font-family: sans-serif;\
            text-align: center;\
        }\
        div {\
            font-size: 160%;\
            background-color: white;\
            margin-top: 15px;\
            margin-left: 5%;\
            margin-right: 5%;\
        }\
        button {\
            background-color:white;\
            font-size: 160%;\
            border: 0px;\
            border-radius: 7px;\
            padding: 20px;\
            margin: 5px;\
            box-shadow: 5px 5px 0px 0px rgb(0, 0, 0, 0.1);\
        } button:hover {\
            box-shadow: 5px 5px 0px 0px rgb(0, 0, 0, 0.5);\
        }\
        button:active {\
            background-color: rgb(159, 159, 159);\
        }\
        table {\
            font-size: 160%;\
            background-color: rgb(235, 235, 235);\
            margin-top: 15px;\
            margin-left: auto;\
            margin-right: auto;\
            padding: 20px;\
            \
            border-spacing: 30px 0px;\
        }\
        on {\
            font-weight: bold;\
            color:green;\
        }\
        off {\
            font-weight: bold;\
            color:red;\
        }\
    </style>\
</head>\
<html>\
    <h1>Physikausstellung<br><b>Dashboard</b></h1>\
    <a href=\"/zeit\"><button>%s</button></a>\
    <a href=\"/an\"><button>%s</button></a>\
    <a href=\"/standby\"><button>%s</button></a>\
    <a href=\"/temperatur\"><button>%s</button></a>\
    <div>%02d:%02d:%02d<br>%02d.%02d.%04d\
    <br>Heute: <b>%s</b></div>\
    <div>Modus: <b>%s</b></div>\
    <div>Innentemperatur: %s °C (Ziel Modus 4: %.0f°C ±%.0f, Mo-Fr 8-15 Uhr)</div>\
    <div>Lampe: <on>%s</on><br>\
    Ventilator: <off>%s</off></br>\
    Wärmepumpe: <on>%s<on></div>\
    <div>\
        Einzelsteuerung (schaltet automatisch auf Manuell):<br>\
        Lampe: <a href=\"/lampe/an\"><button>An</button></a><a href=\"/lampe/aus\"><button>Aus</button></a><br>\
        Lüfter: <a href=\"/luefter/an\"><button>An</button></a><a href=\"/luefter/aus\"><button>Aus</button></a><br>\
        Wärmepumpe: <a href=\"/waermepumpe/an\"><button>An</button></a><a href=\"/waermepumpe/aus\"><button>Aus</button></a>\
    </div>\
    <table>\
        <tr>\
            <th>Ventilator/Lampe</th>\
            <th>Wärmepumpe</th>\
        </tr>\
        <tr>\
            <th>09:20 - 09:25<br>11:05 - 11:35</th>\
            <th>08:55 - 09:26<br>10:40 - 11:31</th>\
        </tr>\
    </table>\
</html>",
    mode == 2 ? "<b>Zeitgeschaltet</b>" : "Zeitgeschaltet",
    mode == 1 ? "<b>An</b>" : "An",
    mode == 0 ? "<b>Standby</b>" : "Standby",
    mode == 4 ? "<b>Temp. 30°C</b>" : "Temp. 30°C",
    hour, minute, second,
    day, month, year,
    fanToday ? "Ventilator" : "Lampe",
    mode == 0 ? "Standby" : mode == 1 ? "An" : mode == 2 ? "Zeitgeschaltet" : mode == 3 ? "Manuell" : "Temperaturhaltung",
    tempIn.length() ? tempIn.c_str() : "?",
    ZIEL_TEMP, HYSTERESE,
    lamp ? "<on>An</b></on>" : "<off>Aus</off>",
    fan ? "<on>An</on>" : "<off>Aus</off>",
    heat ? "<on>An</on>" : "<off>Aus</off>");
    //support utf -8 
    server.sendHeader("Content-Type", "text/html; charset=utf-8");
    server.send(200, "text/html", temp);
}

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
    server.begin(80);
    server.on("/", []() {
        mainPage();
    });
    server.on("/zeit", []() {
        mode = 2;
        statusControl();
        mainPage();
    });
    server.on("/an", []() {
        mode = 1;
        statusControl();
        mainPage();
    });
    server.on("/standby", []() {
        mode = 0;
        statusControl();
        mainPage();
    });
    server.on("/temperatur", []() {
        mode = 4;
        statusControl();
        mainPage();
    });
    // Einzelsteuerung: schaltet automatisch in den manuellen Modus (3),
    // damit statusControl() die Handschaltung nicht überschreibt.
    server.on("/lampe/an", []() {
        mode = 3;
        lamp = true;
        mainPage();
    });
    server.on("/lampe/aus", []() {
        mode = 3;
        lamp = false;
        mainPage();
    });
    server.on("/luefter/an", []() {
        mode = 3;
        fan = true;
        mainPage();
    });
    server.on("/luefter/aus", []() {
        mode = 3;
        fan = false;
        mainPage();
    });
    server.on("/waermepumpe/an", []() {
        mode = 3;
        heat = true;
        mainPage();
    });
    server.on("/waermepumpe/aus", []() {
        mode = 3;
        heat = false;
        mainPage();
    });
    // handle post requests from sensor esp
    server.on("/temp", HTTP_POST, []() {
        if (server.hasArg("plain") == false) {
            // No body found
            Serial.println("No body received");
            server.send(400, "text/plain", "Body not found");
            return;
        }
        tempIn = server.arg("plain");
        // Remove possible trailing newline characters
        tempIn.trim();
        //keep only first two characters
        tempIn = tempIn.substring(0, 2);
        Serial.print("Received temperature: ");
        Serial.println(tempIn);
        server.send(200, "text/plain", "Temperature received");
    });
    server.onNotFound([]() {
        mainPage();
    });
    if (MDNS.begin("admin")) {
        Serial.println("MDNS responder started as 'admin'");
    }
}


void serverLoop()
{
    server.handleClient();
    delay(2);
}

// void serverLoop()
// {
//     WiFiClient client = server.available();
//     if (client)
//     {
//         Serial.println("new client:");
//         Serial.println(client.remoteIP());

//         bool currentLineBlank = true;
//         String currentLine = "";

//         while (client.connected())
//         {
//             if (client.available())
//             {
//                 char c = client.read();
//                 currentLine += c;
//                 Serial.write(c);

//                 if (currentLine.endsWith("GET /zeit"))
//                 {
//                     mode = "Zeitgeschaltet";
//                 }
//                 else if (currentLine.endsWith("GET /an"))
//                 {
//                     mode = "An";
//                 }
//                 else if (currentLine.endsWith("GET /standby"))
//                 {
//                     mode = "Standby";
//                 }
//                 else if (currentLine.endsWith("$POST"))
//                 {
//                     tempIn = currentLine.substring(0, currentLine.length() - 5); // cropping für die Daten vom Sensor-ESP: z.b. "27.5$POST" -> "27.5"
//                     break;
//                 }

//                 statusControl();

//                 if (c == '\n' && currentLineBlank)
//                 {
//                     Serial.println("Sending answer!");

//                     client.println("HTTP/1.1 200 OK");
//                     client.println("Content-Type:text/html");
//                     client.println("Connection: close");
//                     client.println();

//                     client.println("<!DOCTYPE HTML>");
//                     client.println("<html lang=\"de\">");
//                     client.println("<head>");
//                     client.println("<meta charset=\"UTF-8\">");
//                     client.println("<title>Austellung</title>");
//                     client.println("<style>");
//                     client.println("body {background-color: lightgray;font-family: sans-serif;text-align: center;}");
//                     client.println("div {font-size: 160%;background-color: white;margin-top: 15px;margin-left: 5%;margin-right: 5%;}");
//                     client.println("button {background-color:white;font-size: 160%;border: 0px;border-radius: 7px;padding: 20px;margin: 5px;box-shadow: 5px 5px 0px 0px rgb(0, 0, 0, 0.1);}");
//                     client.println("button:hover {box-shadow: 5px 5px 0px 0px rgb(0, 0, 0, 0.5);}");
//                     client.println("button:active { background-color: rgb(159, 159, 159); }");
//                     client.println("table { font-size: 160%; background-color: rgb(235, 235, 235); margin-top: 15px; margin-left: auto; margin-right: auto; padding: 20px; border-spacing: 30px 0px; }");
//                     client.println("on { font-weight: bold; color:green; }");
//                     client.println("off { font-weight: bold; color:red; }");
//                     client.println("</style>");
//                     client.println("</head>");

//                     client.print("<html>");
//                     client.print("<h1>Physikausstellung<br><b>Dashboard</b></h1>");

//                     client.print(mode == "Zeitgeschaltet" ? "<button><b>Zeitgeschaltet</b></button>" : "<a href=\"/zeit\"><button>Zeitgeschaltet</button></a>");
//                     client.print(mode == "An" ? "<button><b>An</b></button>" : "<a href=\"/an\"><button>An</button></a>");
//                     client.print(mode == "Standby" ? "<button><b>Standby</b></button>" : "<a href=\"/standby\"><button>Standby</button></a>");

//                     client.print("<div>");
//                     client.print(hour < 10 ? "0" : "");
//                     client.print(hour);
//                     client.print(":");
//                     client.print(minute < 10 ? "0" : "");
//                     client.print(minute);
//                     client.print(":");
//                     client.print(second < 10 ? "0" : "");
//                     client.print(second);
//                     client.print("<br>");
//                     client.print(day);
//                     client.print(".");
//                     client.print(month);
//                     client.print(".");
//                     client.print(year);
//                     client.print("<br> Heute: <b>");
//                     client.print(fanToday ? "Ventilator" : "Lampe");
//                     client.println("</b></div>");

//                     client.print("<div>Lampe: ");
//                     client.print(lamp ? "<on>An</b></on>" : "<off>Aus</off>");
//                     client.print("<br>Ventilator: ");
//                     client.print(fan ? "<on>An</on>" : "<off>Aus</off>");
//                     client.print("<br>Wärmepumpe: ");
//                     client.print(heat ? "<on>An</on>" : "<off>Aus</off>");
//                     client.println("</div>");

//                     client.print("<table> <tr> <th>Ventilator/Lampe</th> <th>Wärmepumpe</th> </tr> <tr> <th>09:20 - 09:25<br>11:05 - 11:35</th> <th>08:55 - 09:26<br>10:40 - 11:31</th> </tr> </table>");
//                     client.print("</html>");
//                     client.println();

//                     Serial.println("Answer sent!");

//                     break;
//                 }

//                 if (c == '\n')
//                 {
//                     currentLineBlank = true;
//                 }
//                 else if (c != '\r')
//                 {
//                     currentLineBlank = false;
//                 }
//             }
//         }

//         delay(1);
//         client.stop();
//         Serial.println("client disconnected");
//         printTime();
//     }
// }
#endif