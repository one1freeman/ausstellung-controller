#include "clock.h"
//refactor, so that mode is an int for the three possible values (off, on, timer)
int mode = 0; // 0 = Aus, 1 = An, 2 = Zeitgeschaltet, 3 = Manuell (Einzelsteuerung), 4 = Temperaturhaltung

// Zielwerte für die Temperaturhaltung (Modus 4)
#define ZIEL_TEMP 30.0
#define HYSTERESE 1.0

bool fanToday, lamp, fan, heat, timer; // alle bools die sonstwo gebraucht werden
String tempIn;

void statusControl()
{
    // Im manuellen Modus werden lamp/fan/heat direkt über die Einzel-Buttons
    // gesetzt (server.h) und hier nicht überschrieben.
    if (mode == 3)
    {
        return;
    }

    // alle zwei Tage Windkraft, ansonsten Solar
    fanToday = weekday % 2 == 0;
    bool dayCorrect = weekday != 7 && weekday != 1; // nur Montag-Freitag

    // Timetable Check für Lampe/Lüfter, ziemlich hässlich
    // gilt sowohl im normalen Zeitgeschaltet-Modus als auch in der Temperaturhaltung
    bool timetable = ((hour == 9 && minute > 20 && minute < 45) || // nur zu den Pausen
                      (hour == 11 && minute > 5 && minute < 35)) &&
                     dayCorrect &&
                     (mode == 2 || mode == 4);

    fan = (fanToday && timetable) || mode == 1;
    lamp = (!fanToday && timetable) || mode == 1;

    if (mode == 4)
    {
        // Temperaturhaltung: Wärmepumpe hält die Innentemperatur (vom Sensor-ESP32)
        // während der Schulzeit (Mo-Fr 8-15 Uhr) bei ZIEL_TEMP, mit Hysterese
        // gegen ständiges Ein/Aus-Klackern. Lampe/Lüfter laufen dabei ganz normal
        // nach dem üblichen Zeitplan (s.o.) weiter.
        bool schulzeit = dayCorrect && hour >= 8 && hour < 15;
        if (!schulzeit || tempIn.length() == 0)
        {
            // außerhalb der Schulzeit, oder noch keine Temperatur vom Sensor
            // empfangen: sicherheitshalber aus
            heat = false;
        }
        else
        {
            float innenTemp = tempIn.toFloat();
            if (innenTemp <= ZIEL_TEMP - HYSTERESE)
            {
                heat = true;
            }
            else if (innenTemp >= ZIEL_TEMP + HYSTERESE)
            {
                heat = false;
            }
            // sonst: innerhalb des Toleranzbands, Zustand beibehalten
        }
        return;
    }

    bool timetableHeat = ((hour == 8 && minute > 55) || // mehr Vorlauf für die Wärmepumpe
                          (hour == 9 && minute < 26) ||
                          (hour == 10 && minute > 40) ||
                          (hour == 11 && minute < 31)) &&
                         dayCorrect &&
                         mode == 2;

    heat = timetableHeat || mode == 1;
}
