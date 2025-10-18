#include <I2C_RTC.h>

static DS1307 RTC;

int weekday;
int day;
int month;
int year;
int hour;
int minute;
int second;

void rtcLoop() {
    weekday = RTC.getWeek();
    day = RTC.getDay();
    month = RTC.getMonth();
    year = RTC.getYear();
    hour = RTC.getHours();
    minute = RTC.getMinutes();
    second = RTC.getSeconds();
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