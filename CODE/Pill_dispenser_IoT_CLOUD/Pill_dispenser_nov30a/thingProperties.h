// Code generated by Arduino IoT Cloud, DO NOT EDIT.

#include <ArduinoIoTCloud.h>
#include <Arduino_ConnectionHandler.h>


const char THING_ID[] = "423500f3-3614-42d4-9d62-40d97c1c3a0a";

const char SSID[]     = SECRET_SSID;    // Network SSID (name)
const char PASS[]     = SECRET_PASS;    // Network password (use for WPA, or use as key for WEP)

void onMinutesAlarmChange();
void onHoursAlarmChange();
void onScheduleVarChange();
void onAlarmStateChange();
void onSecondsAlarmChange();

int minutes_alarm;
int hours_alarm;
//CloudSchedule schedule_var;
bool alarm_state;
int seconds_alarm;

void initProperties(){

  ArduinoCloud.setThingId(THING_ID);
  ArduinoCloud.addProperty(minutes_alarm, READWRITE, ON_CHANGE, onMinutesAlarmChange);
  ArduinoCloud.addProperty(hours_alarm, READWRITE, ON_CHANGE, onHoursAlarmChange);
  //ArduinoCloud.addProperty(schedule_var, READWRITE, ON_CHANGE, onScheduleVarChange);
  ArduinoCloud.addProperty(alarm_state, READWRITE, ON_CHANGE, onAlarmStateChange);
  ArduinoCloud.addProperty(seconds_alarm, READWRITE, ON_CHANGE, onSecondsAlarmChange);

}

WiFiConnectionHandler ArduinoIoTPreferredConnection(SSID, PASS);