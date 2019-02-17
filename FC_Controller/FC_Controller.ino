#include "FC_Constants.h" // Include the constants file.

// ----------------- CONSTANTS -----------------

// Flags
boolean fc_on = false;
boolean fc_alarm = false;
boolean fc_fan_time_set = false;
boolean timer_time_set = false;
boolean arrays_filled = false;

// Program constants (determined via testing) TODO: VERIFY
long STARTUP_PURGE_LOOP_COUNT = 5000;
unsigned long Current_Time = 0; // would overflow after 25 days if left running forever (hopefully)
long STANDBY_DELAY_TIME = 5000;
long SHUTDOWN_DELAY_TIME = 5000;

// Counters
long timer_start_time = 0;
int purge_counter = 1;
long purgeLastCallTime = 0;
int arrayIndex = 0;

// Fake Counters
int count = 0;
int stackTempPos = 0;

// Sensor Arrays
unsigned short stackTempArray[ARRAY_SIZE];
unsigned short stackCurrentArray[ARRAY_SIZE];
unsigned short stackVoltageArray[ARRAY_SIZE];
unsigned short ambientTempArray[ARRAY_SIZE];
//unsigned short hydrogenArray[ARRAY_SIZE];

// States
int FC_State = FC_INITIAL; // initial state perhaps we could enumerate these
int FC_SubState = FC_STARTUP_STARTUP_PURGE;
int FAN_State = FAN_MID; //First Fan state assumed is MID

// Averaged values
double amb_temp;
double stack_temp;
double fc_current;
double fc_voltage;
double tempInput;


// ----------------- SETUP -----------------
void setup() {
  pinMode(STATE_LED_RED, OUTPUT); // State LED. //Red
  pinMode(STATE_LED_YELLOW, OUTPUT); //Yellow
  pinMode(STATE_LED_BLUE, OUTPUT); //Blue

  pinMode(PURGE_PIN, OUTPUT); // Purge, supply, FC relay, FC fan relay, and resistor relay.
  pinMode(SUPPLY_PIN, OUTPUT);
  pinMode(FC_RELAY_PIN, OUTPUT);
  pinMode(RELAY1, OUTPUT);
  pinMode(RELAY2, OUTPUT);
  pinMode(RELAY3, OUTPUT);
  pinMode(RESISTOR_PIN, OUTPUT);

  delay(100);

  setAllSafe();
}

// ----------------- LOOP -----------------
void loop() {
  Check_Alarms();
  System();
  FC();
}


// ----------------- CHECK ALARMS -----------------
void Check_Alarms() {
  // Check all alarm properties to make sure they're within the safe range
  // If not, set fc_alarm to true.
  // Temperature, Current, Voltage, Hydrogen are checked

  // Take measurements
  ambientTempArray[arrayIndex] = analogRead(AMB_THEMRMISTOR_PIN);
  stackTempArray[arrayIndex] = analogRead(STACK_THEMRMISTOR_PIN);
  stackVoltageArray[arrayIndex] = analogRead(VOLTAGE_PIN);
  stackCurrentArray[arrayIndex] = analogRead(CURRENT_PIN);
  // hydrogenArray[arrayIndex] = analogRead(HYDROGEN_PIN); NOT USED IN THIS ITERATION.

  arrayIndex++;
  if (arrayIndex == ARRAY_SIZE) {
    arrays_filled = true;
    arrayIndex = 0;
  }

  if (!arrays_filled)
    return;

  unsigned long amb_temp_total = 0;
  unsigned long stack_temp_total = 0;
  unsigned long fc_voltage_total = 0;
  unsigned long fc_current_total = 0;
  //unsigned long amb_hydrogen_total = 0;


  for (int i = 0; i < ARRAY_SIZE; i++) {
    amb_temp_total = amb_temp_total + ambientTempArray[i];
    stack_temp_total = stack_temp_total + stackTempArray[i];
    fc_voltage_total = fc_voltage_total + stackVoltageArray[i];
    fc_current_total = fc_current_total + stackCurrentArray[i];
    //amb_hydrogen_total = amb_hydrogen_total + hydrogenArray[i];
  }

  // TODO: potential issue is that we switch states and then the array value are still too low...
  // TODO: could enter alarm state erroneously, may need to allow a few loops between state changes

  // Uses the inputed raw data value to determine the actual values for ambient temp, stack temp, etc.
  amb_temp = ambTemperatureComputation(amb_temp_total / 100);
  stack_temp = stackTemperatureComputation(stack_temp_total / 100);
  fc_voltage = voltageComputation(fc_voltage_total / 100);
  fc_current = currentComputation(fc_current_total / 100);
  //amb_hydrogen = hydrogenComputation(amb_hydrogen_total / 100);

  //if (amb_hydrogen > HYDROGEN_MAX) //Always checking for hydrogen leaking so checks in every state
  //  fc_alarm = true;
  if (fc_current < FC_MIN_CURRENT || fc_current > FC_MAX_CURRENT)
    fc_alarm = true;

  if (FC_State == FC_RUN) {
    if (fc_voltage < FC_RUN_MIN_VOLTAGE || fc_voltage > FC_MAX_VOLTAGE)
      fc_alarm = true;

    if (amb_temp < FC_RUN_MIN_TEMP || amb_temp > FC_MAX_TEMP)
      fc_alarm = true;

    if (stack_temp < FC_RUN_MIN_TEMP || stack_temp > FC_MAX_TEMP)
      fc_alarm = true;
  }
  else {
    if (fc_voltage < FC_STANDBY_MIN_VOLTAGE || fc_voltage > FC_MAX_VOLTAGE)
      fc_alarm = true;

    if (amb_temp < FC_MIN_TEMP || amb_temp > FC_MAX_TEMP)
      fc_alarm = true;

    if (stack_temp < FC_MIN_TEMP || stack_temp > FC_MAX_TEMP)
      fc_alarm = true;
  }

}

// ----------------- SYSTEM -----------------

void System() {
  // wait for input to set fc_on to true

  if (!fc_on)
    setColorState(LED_ON, 0, LED_ON); //Red and Blue
  if (digitalRead(SYSTEM_ON_PIN) == HIGH) {
    fc_on = !fc_on;
    setColorState(LED_ON, LED_ON, LED_ON);
    delay(1000); // to prevent us from flipping back and forth between on and off
    //TODO: counter or delay? delay may cause a delay in control before switching off which may not be ideal.
    // then again, turning the FC off kills the controller so I suppose it really doesn't matter
  }


}

// ----------------- FC -----------------

void FC() {
  if (fc_alarm)
    stateTransition(FC_State, FC_ALARM);

  if (!fc_on) {
    setAllSafe();
    FC_State = FC_INITIAL;
    return;
  }

  // FSM
  switch (FC_State) {
    case (FC_INITIAL):
      setColorState(0, LED_ON, 0); // Sets state LED to yellow.

      if (count > 1000) { // A timer to determine how long to be in this state.
        stateTransition(FC_INITIAL, FC_STANDBY);
        count = 0;
      }
      break;

    case (FC_STANDBY):
      // The stack is not consuming reactant or delivering power
      // and all stack BOP actuators are in their safe state
      // The system remains in FC_STANDBY for STANDBY_DELAY_TIME

      setColorState(0, 0, LED_ON); // Sets state LED to blue.

      if (!timer_time_set) {
        timer_start_time = Current_Time;
        timer_time_set = true;
      }
      //Default State
      fanControl(FAN_OFF);//Fan speed
      setSupplyState(CLOSED);//Supply valve CLOSED
      setPurgeState(CLOSED);//Purge Valve CLOSED
      setRelayState(OPEN);//FC Relay OPEN
      setResistorState(OPEN);//Startup Resistor OPEN

      if (STANDBY_DELAY_TIME <= Current_Time - timer_start_time && fc_on) {
        timer_time_set = false;
        stateTransition(FC_STANDBY, FC_STARTUP);
      }
      break;

    case (FC_STARTUP): // TODO: I changed this but we need to make sure that it is right in terms of the relay states
      // The stack goes from FC_STANDBY to a state where current can
      // safely be drawn from the stack
      setColorState(LED_ON, LED_ON, 0); // Sets state LED to red, yellow.
      FCStartup();
      break;

    case (FC_RUN):
      // ?? manual info is copied from FC_STANDBY
      setColorState(0, LED_ON, LED_ON); //BLUE and YELLOW led
      setSupplyState(OPEN); // The supply value should always be open.
      setRelayState(CLOSED);
      AutomaticPurgeControl();
      AutomaticFanControl(fc_current, stack_temp);

      if (!fc_on)
        stateTransition(FC_RUN, FC_SHUTDOWN);
      break;

    case (FC_SHUTDOWN):
      // The stack goes from FC_RUN to FC_STANDBY. The system remains in
      // FC_SHUTDOWN for SHUTDOWN_DELAY_TIME

      setColorState(LED_ON, LED_ON, LED_ON); // RED BLUE YELLOW on

      if (!timer_time_set) {
        timer_start_time = Current_Time;
        timer_time_set = true;
      }

      setAllSafe();

      if (SHUTDOWN_DELAY_TIME <= Current_Time - timer_start_time) {
        timer_time_set = false;
        stateTransition(FC_SHUTDOWN, FC_STANDBY);
      }
      break;

    case (FC_ALARM):
      // The stack is shut down because an alarm was triggered.
      // All actuators are in their safe states
      setAllSafe();

      setColorState(LED_ON, 0, 0); // Sets state LED to red.

      break;

    default:
      fc_alarm = true; //If we somehow enter an invalid state something is wrong so we should transfer to the FC_Alarm
      break;

  }

  // eventually can add a delay here depending on what kind of timing resolution we need
  if (count > 1000) {
    count = 0;
  }
  count++;

  Current_Time++;
}

// ----------------- STATE TRANSITION & SUBSTATE TRANSITION -----------------

void stateTransition(int fromState, int toState) {
  FC_State = toState;
}

void subStateTransition(int fromState, int toState) {
  FC_SubState = toState;
}

// ----------------- FC STARTUP -----------------

void FCStartup() {
  switch (FC_SubState) {
    case (FC_STARTUP_STARTUP_PURGE):
      // purge valve and supply valves are opened simultaneously
      // for the start-up purge and the start-up resistor is applied
      // across the stack to limit voltage

      setSupplyState(OPEN);
      setPurgeState(OPEN);
      setRelayState(OPEN); // Close the state relay.
      setResistorState(CLOSED); // Close the resistor relay as we have reached a stage where we no longer need the start up resistor.

      if (!fc_fan_time_set) { //Timer Reused for PURGE
        purge_counter = Current_Time;
        fc_fan_time_set = true;
      }

      if (STARTUP_PURGE_LOOP_COUNT <= Current_Time - purge_counter) {
        fc_fan_time_set = false;

        setPurgeState(CLOSED);  //Done in substate STARTUP_END
        setSupplyState(CLOSED); //Why was this put here?

        subStateTransition(FC_SubState, FC_STARTUP_END);
        return;
      }
      break;

    case (FC_STARTUP_END):
      setPurgeState(CLOSED);// close purge valve
      setSupplyState(CLOSED);// fan is minimum

      fc_fan_time_set = false;
      purge_counter = 1;
      stateTransition(FC_State, FC_RUN);
      subStateTransition(FC_SubState, FC_STARTUP_STARTUP_PURGE); // switch back to startup purge

      setResistorState(OPEN); // Open the resistor relay as we have reached a stage where we no longer need the start up resistor.
      setRelayState(OPEN); // Close the system relay, start the damn thing.

      fanControl(FAN_MID_LOW);
      break;
  }
}

// ----------------- FAN CONTROL -----------------

void AutomaticFanControl(int current, int temp_average) {
  int temp_opt = 0.53 * current + 26.01;
  int temp_max = 0.3442 * current + 52.143;
  int temp_min = 0.531 * current + 6.0025;

  if (temp_average >= temp_max || temp_average > 73) { //MAX
    fanControl(FAN_MAX);
  }

  else if (temp_average > temp_opt && temp_average <= temp_max - 2) {//MID_HIGH
    fanControl(FAN_MID_HIGH);
  }
  else if (temp_average >= temp_opt - 2 && temp_average <= temp_opt + 2) { // Perfect temp //MID
    fanControl(FAN_MID);
  }
  else if (temp_average <= temp_opt && temp_average <= temp_min + 2) { // Kinda cold.//MID_LOW
    fanControl(FAN_MID_LOW);
  }
  else if (temp_average < temp_min + 2) { // Too cold. //MIN
    fanControl(FAN_MIN);
  }
  else {
    fanControl(FAN_MAX);//If something is wrong and we get through all the cases FAN to MAX
  }
  /*
    switch (FAN_State) {
    case FAN_MAX: //STACK IS HOT
      if (temp_average >= temp_max || temp_average > 73) { //MAX
        fanControl(FAN_MAX);
        FAN_State = FAN_MAX;
      }
      break;
    case FAN_MID_HIGH:
      if (temp_average > temp_opt && temp_average <= temp_max - 2) {//MID_HIGH
        fanControl(FAN_MID_HIGH);
        FAN_State =FAN_MID_HIGH;
      }
      break;
    case FAN_MID:
      if (temp_average >= temp_opt - 2 && temp_average <= temp_opt + 2) { // Perfect temp //MID
        fanControl(FAN_MID);
        FAN_State =FAN_MID;
      }
      break;
    case FAN_MID_LOW:
      if (temp_average <= temp_opt && temp_average <= temp_min + 2) { // Kinda cold.//MID_LOW
        fanControl(FAN_MID_LOW);
        FAN_State =FAN_MID_LOW;
      }
      break;
    case FAN_MIN: //STACK IS COLD
      if (temp_average < temp_min + 2) { // Too cold. //MIN
        fanControl(FAN_MIN);
        FAN_State =FAN_MIN;
      }
      break;
    default:
      fanControl(FAN_MAX);
      break;
    }
  */
}

void fanControl(int FAN_SPEED) {
  switch (FAN_SPEED) {
    case FAN_MAX: //STACK IS HOT
      fanRelayControl(LOW, LOW, HIGH);
      break;
    case FAN_MID_HIGH:
      fanRelayControl(LOW, HIGH, LOW);
      break;
    case FAN_MID:
      fanRelayControl(LOW, HIGH, HIGH);
      break;
    case FAN_MID_LOW:
      fanRelayControl(HIGH, LOW, LOW);
      break;
    case FAN_MIN: //STACK IS COLD
      fanRelayControl(HIGH, HIGH, LOW);
      break;
    case FAN_OFF:
      fanRelayControl(LOW,  LOW,  LOW);
      break;
  }
}

void fanRelayControl(boolean relayOneState, boolean relayTwoState, boolean relayThreeState) {
  digitalWrite(RELAY1, relayOneState);
  digitalWrite(RELAY2, relayTwoState);
  digitalWrite(RELAY3, relayThreeState);
}

// ----------------- PURGE CONTROL -----------------
// TODO: use amperage threshhold, this may or may not work
boolean AutomaticPurgeControl() {
  long time_purge = millis() - purgeLastCallTime;

  if (GENERAL_PURGE_TIME + purgeTime < time_purge) {
    setPurgeState(CLOSED);
    purgeLastCallTime = millis();

  }
  else if (GENERAL_PURGE_TIME < time_purge) {
    setPurgeState(OPEN);
  }
}

// ----------------- CHANGE STATES FOR PURGE, SUPPLY, FAN, RELAY, RESISTOR -----------------
void setPurgeState(int state) {
  // !state means that when you "close" the valve it actually closes (sends 0 instead of 1)
  // and when you set setPurgeState(OPEN) it sends a 1, actually opening the valve
  digitalWrite(PURGE_PIN, !state);
}

void setSupplyState(int state) {
  // !state means that when set setPurgeState(CLOSED) the valve closes (sends 0 instead of 1)
  // and when you set setPurgeState(OPEN) it sends a 1, actually opening the valve
  digitalWrite(SUPPLY_PIN, !state);
}

void setRelayState(int state) {
  digitalWrite(FC_RELAY_PIN, state);
}

void setResistorState(int state) {
  digitalWrite(RESISTOR_PIN, state);
}

void setAllSafe(void) {
  fanControl(FAN_OFF);

  setSupplyState(CLOSED);
  setPurgeState(CLOSED);
  setRelayState(OPEN);
  setResistorState(OPEN);
}

// ----------------- CONTROL OF LEDs -----------------

void setColorState(int red, int yellow, int blue) {
  analogWrite(STATE_LED_RED, red);
  analogWrite(STATE_LED_BLUE, blue);
  analogWrite(STATE_LED_YELLOW, yellow);
}

// TODO: are these A B anc C values the same for both sensors????
int stackTemperatureComputation(int averageValue) {
  double A_in = averageValue * 5 / 1023; // ??? not sure this is right need more descriptive var names
  double R1 = (5 * R2_STACK - A_in * R2_STACK) / A_in;
  double temp = a_temp * R1 * R1 + b_temp * R1 + c_temp;

  return (int) temp;
}

int ambTemperatureComputation(int averageValue) {
  double A_in = averageValue * 5 / 1023;
  double R1 = (5 * R2_AMBIENT - A_in * R2_AMBIENT) / A_in;
  double temp = a_temp * R1 * R1 + b_temp * R1 + c_temp;

  return (int) temp;
}

int voltageComputation(int averageValue) {
  double V_v_in = averageValue * 5 / 1023;
  double voltage = V_v_in * 28.5 / 1.7; //Accuracy breaks down at 1.4V. Divide 1.7V because reasons...

  return (int) voltage;
}

int currentComputation(int averageValue) {
  double V_c_in = averageValue * 5 / 1023; // this gets the value in V but not the actual current
  double current = V_c_in * G; // TODO: determine G, this ratio is currently incorrect (100A/50mV) (this does not get actual current)

  return (int) current;

}

//int hydrogenComputation(int averageValue){
//
//  double H_in = averageValue*5/1023; // Determine later.
//  double hydrogen = H_in * H_CONST;
//
//  return (int) hydrogen
//
//}
