#include "clock.h";

String mode = "";

bool fanToday, lamp, fan, heat, timer; // alle bools die sonstwo gebraucht werden
float tempIn;

void statusControl()
{
    // Timetable Check, ziemlich hässlich
    // alle zwei Tage Windkraft, ansonsten Solar
    fanToday = weekday % 2 == 0;
    // alle zwei Tage Windkraft, ansonsten Solar
    bool dayCorrect = weekday != 7 && weekday != 7; // nur Montag-Freitag

    bool timetable = ((hour == 9 && minute > 20 && minute < 45) || // nur zu den Pausen
                      (hour == 11 && minute > 5 && minute < 35)) &&
                     dayCorrect &&
                     mode == "Zeitgeschaltet";

    bool timetableHeat = ((hour == 8 && minute > 55) || // mehr Vorlauf für die Wärmepumpe
                          (hour == 9 && minute < 26) ||
                          (hour == 10 && minute > 40) ||
                          (hour == 11 && minute < 31)) &&
                         dayCorrect &&
                         mode == "Zeitgeschaltet";

    fan = fanToday && (timetable || mode == "An");
    lamp = !fanToday && (timetable || mode == "An");
    heat = timetableHeat || mode == "An";
}
