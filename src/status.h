#include "clock.h"
//refactor, so that mode is an int for the three possible values (off, on, timer)
int mode = 0; // 0 = Aus, 1 = An, 2 = Zeitgeschaltet

bool fanToday, lamp, fan, heat, timer; // alle bools die sonstwo gebraucht werden
String tempIn;

void statusControl()
{
    // Timetable Check, ziemlich hässlich
    // alle zwei Tage Windkraft, ansonsten Solar
    fanToday = weekday % 2 == 0;
    // alle zwei Tage Windkraft, ansonsten Solar
    bool dayCorrect = weekday != 7 && weekday != 1; // nur Montag-Freitag

    bool timetable = ((hour == 9 && minute > 20 && minute < 45) || // nur zu den Pausen
                      (hour == 11 && minute > 5 && minute < 35)) &&
                     dayCorrect &&
                     mode == 2;

    bool timetableHeat = ((hour == 8 && minute > 55) || // mehr Vorlauf für die Wärmepumpe
                          (hour == 9 && minute < 26) ||
                          (hour == 10 && minute > 40) ||
                          (hour == 11 && minute < 31)) &&
                         dayCorrect &&
                         mode == 2;

    fan = (fanToday && timetable) || mode == 1;
    lamp = (!fanToday && timetable) || mode == 1;
    heat = timetableHeat || mode == 1;
}
