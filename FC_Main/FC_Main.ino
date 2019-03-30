#include "FC_Constants.h"
#include <EEPROM.h>

int alarm_val;
int alarm_sens;

// System-Wide Flags
boolean fc_on = false;
boolean fc_alarm = false;
boolean eeprom_stw = false;

// System-Wide Counters
unsigned long Current_Time = 1; // would overflow after 25 days if left running forever (hopefully)
unsigned long Last_Time = 0;
unsigned long LOOP_TIME = 0;

// Global States
int FC_State = FC_INITIAL;
int FC_SubState = FC_STARTUP_STARTUP_PURGE;
int FAN_State = FAN_MID;                    // First Fan state assumed is MID at startup

// ----------------- SETUP ----------------- //
void setup() {
  // State LEDs
  pinMode(STATE_LED_RED, OUTPUT);
  pinMode(STATE_LED_YELLOW, OUTPUT);
  pinMode(STATE_LED_BLUE, OUTPUT);

  // Purge, supply, FC relay, FC fan relay, and resistor relay.
  pinMode(PURGE_PIN, OUTPUT);
  pinMode(SUPPLY_PIN, OUTPUT);
  pinMode(FC_RELAY_PIN, OUTPUT);
  pinMode(RELAY1, OUTPUT);
  pinMode(RELAY2, OUTPUT);
  pinMode(RELAY3, OUTPUT);
  pinMode(RESISTOR_PIN, OUTPUT);

  delay(100);
  setAllSafe();
}

// ----------------- LOOP ----------------- //
void loop() {
  Check_Alarms();
  System();
  FC();

  // keep track of the time between loops
  Last_Time = Current_Time;
  Current_Time = millis();
  LOOP_TIME = Current_Time - Last_Time;

  // if we enter alarm, save the sensor that sent us into alarm in memory
  if(!eeprom_stw && fc_alarm && Current_Time > 4000){
    eeprom_stw = true;
    EEPROM.write(0,alarm_sens);
    EEPROM.write(1,alarm_val);
    }
}
