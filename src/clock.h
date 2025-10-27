#include <RTClib.h>

RTC_DS3231 rtc;

int weekday;
int day;
int month;
int year;
int hour;
int minute;
int second;

void rtcSetup()
{
  if (!rtc.begin())
  {
    Serial.println("Couldn't find RTC");
    while (1)
      delay(10);
  }
}

void rtcLoop()
{
  DateTime now = rtc.now();
  weekday = now.dayOfTheWeek();
  day = now.day();
  month = now.month();
  year = now.year();
  hour = now.hour();
  if ((month >= 11 || month <= 2 ) || (month == 10 && day >= 25) || (month == 3 && day <=29)) //winterzeit, richtig bis märz 2027
  {
    hour--;
  }
  minute = now.minute();
  second = now.second();
}

void printTime()
{
  rtcLoop();
  Serial.print(hour < 10 ? "0" : "");
  Serial.print(hour);
  Serial.print(minute < 10 ? ":0" : ":");
  Serial.print(minute);
  Serial.print(second < 10 ? ":0" : ":");
  Serial.println(second);
  Serial.print(day);
  Serial.print(".");
  Serial.print(month);
  Serial.print(".");
  Serial.println(year);
}