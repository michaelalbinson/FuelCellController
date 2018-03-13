#include "FC_Constants.h" // Include the constants file.

// ----------------- CONSTANTS -----------------

// Flags
boolean fc_on = false;
boolean fc_alarm = false;
boolean fc_fan_time_set = false;
boolean timer_time_set = false;
boolean arrays_filled = false;

// Program constants (determined via testing)
long FAN_SPOOLUP_TIME = 5000;
long STARTUP_PURGE_LOOP_COUNT = 5000;
unsigned long Current_Time = 0; // would overflow after 25 days if left running forever (hopefully)
long STANDBY_DELAY_TIME = 5000;
long SHUTDOWN_DELAY_TIME = 5000;

// Counters
long fan_start_time = 0;
long timer_start_time = 0;
int purge_counter = 1;
long purgeLastCallTime = 0;
long secondCounter;
int arrayIndex = 0;

// Fake Counters
int count = 0;
unsigned long current_time = 0;
int CurrentRequest = 0; // not sure of the type for this yet
int stackTempPos = 0;
unsigned short stackTempArray[ARRAY_SIZE];
unsigned short stackCurrentArray[ARRAY_SIZE];
unsigned short stackVoltageArray[ARRAY_SIZE];
unsigned short ambientTempArray[ARRAY_SIZE];

// States
int FC_State = FC_INITIAL; // initial state perhaps we could enumerate these
int FC_SubState = FC_STARTUP_STARTUP_PURGE;

// Averaged values
  int amb_temp;
  int stack_temp;
  int fc_current;
  int fc_voltage;

// ----------------- SETUP -----------------

void setup() {

  Serial.begin(115200); // Initiates the serial.
  
  pinMode(STATE_LED_RED, OUTPUT); // State LED.
  pinMode(STATE_LED_GREEN, OUTPUT);
  pinMode(STATE_LED_BLUE, OUTPUT);

  pinMode(PURGE_PIN, OUTPUT); // Purge, supply, FC relay, FC fan relay, and resistor relay.
  pinMode(SUPPLY_PIN, OUTPUT);
  pinMode(FC_RELAY_PIN, OUTPUT);
  pinMode(FC_FAN_RELAY_PIN, OUTPUT);
  pinMode(RESISTOR_PIN, OUTPUT);

  delay(100);
  
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
  
  arrayIndex++;
  if(arrayIndex == ARRAY_SIZE){
    arrays_filled = true;
    arrayIndex = 0;
  }

  if(!arrays_filled)
    return;

  unsigned long amb_temp_total = 0;
  unsigned long stack_temp_total = 0;
  unsigned long fc_voltage_total = 0;
  unsigned long fc_current_total = 0;
  
  for(int i = 0; i< ARRAY_SIZE; i++){
    amb_temp_total = amb_temp_total + ambientTempArray[i];
    stack_temp_total = stack_temp_total + stackTempArray[i];
    fc_voltage_total = fc_voltage_total + stackVoltageArray[i];
    fc_current_total = fc_current_total + stackCurrentArray[i];
  }
  
  
  amb_temp = amb_temp_total/100;
  stack_temp = stack_temp_total/100;
  fc_current = fc_voltage_total/100;
  fc_voltage = fc_current_total/100;
  
  if (amb_temp < FC_MIN_TEMP || amb_temp > FC_MAX_TEMP)
    //fc_alarm = true;

  if (stack_temp < FC_MIN_TEMP || stack_temp > FC_MAX_TEMP)
    //fc_alarm = true;

  if(fc_current < FC_MIN_CURRENT || fc_current > FC_MAX_CURRENT)
    //fc_alarm = true;

  if(fc_voltage < FC_RUN_MIN_VOLTAGE || fc_voltage > FC_MAX_VOLTAGE)
    //fc_alarm = true;

  getTemperature(); // Update the temperatures.
}

// ----------------- SYSTEM -----------------

void System() {
  // wait for input to set fc_on to true, set CurrentRequest
  
  //if (digitalRead(SYSTEM_ON_PIN) == HIGH) { // ADD THIS BACK IN WHEN APPLYING TO CAR.
    fc_on = true;
  //}

  CurrentRequest = 0; // need some logic to know how to set CurrentRequest
}

// ----------------- FC -----------------

void FC() {
  setRelayState(OPEN); // Open the relay.
  setResistorState(OPEN); // Open the resistor to disipate power while starting up.
  if (fc_alarm)
    stateTransition(FC_State, FC_ALARM);

  // FSM
  switch (FC_State) {
    case (FC_INITIAL):
      setColorState(200, 0, 0); // Shows in initial state.
      if (count > 1000){ // A timer to determine how long to be in this state.
        stateTransition(FC_INITIAL, FC_STANDBY);
        count = 0;
      }
      break;
    case (FC_STANDBY):
      // The stack is not consuming reactant or delivering power
      // and all stack BOP actuators are in their safe state
      // The system remains in FC_STANDBY for STANDBY_DELAY_TIME
      
      setColorState(200, 50, 0);
      
      if (!timer_time_set) {
        timer_start_time = Current_Time;
        timer_time_set = true;
      }

      // probably need to set the default state of all the things in the fuel cell

      if (STANDBY_DELAY_TIME <= Current_Time - timer_start_time && fc_on) {
        timer_time_set = false;
        stateTransition(FC_STANDBY, FC_STARTUP);
      }
      break;

    case (FC_STARTUP):
      // The stack goes from FC_STANDBY to a state where current can
      // safely be drawn from the stack
      //digitalWrite(10, HIGH);//remove
      
      setResistorState(CLOSED); // Close the resistor relay as we have reached a stage where we no longer need the start up resistor.
      setRelayState(CLOSED); // Close the state relay.
      
      FCStartup();
      
      break;

    case (FC_RUN):
      // ?? manual info is copied from FC_STANDBY
      
      setColorState(0, 200, 0);
      
      if (!fc_on)
        stateTransition(FC_RUN, FC_SHUTDOWN);

      AutomaticPurgeControl();
      AutomaticFanControl(fc_current, stack_temp);
      break;

    case (FC_SHUTDOWN):
      
      setColorState(0, 0, 200);
      
      // The stack goes from FC_RUN to FC_STANDBY. The system remains in
      // FC_SHUTDOWN for SHUTDOWN_DELAY_TIME
      if (!timer_time_set) {
        timer_start_time = Current_Time;
        timer_time_set = true;
      }

      // do something to shut down the stack?

      if (SHUTDOWN_DELAY_TIME <= Current_Time - timer_start_time) {
        timer_time_set = false;
        stateTransition(FC_SHUTDOWN, FC_STANDBY);
      }
      break;

    case (FC_ALARM):
      // The stack is shut down because an alarm was triggered.
      // All actuators are in their safe states

      // seems like we need to do some work to make sure everything is in the right state
      // in the event of an alarm... we could specify the alert via a Serial.println if we really
      // wanted but we could also build a visual representation of the system that failed
      
      setColorState(100, 0, 100);
      
      Serial.println("AN ALARM HAS BEEN TRIGGERED");
      break;

    default:
      fc_alarm = true; //If we somehow enter an invalid state something is wrong so we should transfer to the FC_Alarm
      break;

  }

  // eventually can add a delay here depending on what kind of timing resolution we need
  if (count > 1000){
    count = 0;

    if (FC_State == FC_STARTUP)
    turnAllOff();
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

      digitalWrite(PURGE_PIN,HIGH);
      digitalWrite(SUPPLY_PIN,HIGH);
      
      setColorState(200, 150, 0);
      
      if (!fc_fan_time_set) {
        purge_counter = Current_Time;
        fc_fan_time_set = true;
      }

      if (STARTUP_PURGE_LOOP_COUNT <= Current_Time - purge_counter) {
        fc_fan_time_set = false;
        
        digitalWrite(PURGE_PIN,LOW);
        digitalWrite(SUPPLY_PIN,LOW);
        
        subStateTransition(FC_SubState, FC_STARTUP_END);
        return;
      }
      break;

    case (FC_STARTUP_END):
      // close purge valve
      // fan is minimum
      fc_fan_time_set = false;
      setColorState(200, 200, 0);
      purge_counter = 1;
      stateTransition(FC_State, FC_RUN);
      subStateTransition(FC_SubState, FC_STARTUP_FAN_SPOOLUP); // switch back to startup purge
      break;
  }
}

// ----------------- FAN CONTROL -----------------

float AutomaticFanControl(int current, int temp_average) {
  float OptTemp;
  float UpdatedFanCmd;
  //does the fan control thing.

  getTemperature(); // Updates the temperature.

  int temp_opt = 0.53*current + 26.01;
  int temp_max = 0.3442*current + 52.143;
  int temp_min = 0.531*current + 6.0025;
  
  if(temp_average >= temp_max || temp_average > 73){
    digitalWrite(RELAY1, LOW);
    digitalWrite(RELAY2, LOW);
    digitalWrite(RELAY3, HIGH);
  }
  else if(temp_average > temp_opt && temp_average <= temp_max-2){
    digitalWrite(RELAY1, LOW);
    digitalWrite(RELAY2, HIGH);
    digitalWrite(RELAY3, LOW);
  }
  else if(temp_average >= temp_opt-2 && temp_average <=temp_opt+2){ // Perfect temp
    digitalWrite(RELAY1, LOW);
    digitalWrite(RELAY2, HIGH);
    digitalWrite(RELAY3, HIGH);
  }
  else if(temp_average <= temp_opt && temp_average <= temp_min+2){ // Kinda cold.
    digitalWrite(RELAY1, HIGH);
    digitalWrite(RELAY2, LOW);
    digitalWrite(RELAY3, LOW);
  }
  else if(temp_average < temp_min+2){ // Too cold.
    digitalWrite(RELAY1, HIGH);
    digitalWrite(RELAY2, HIGH);
    digitalWrite(RELAY3, LOW);
  }
}

// ----------------- PURGE CONTROL -----------------

boolean AutomaticPurgeControl() {

  digitalWrite(SUPPLY_PIN, HIGH); // The supply value should always be open.
  
  long time_purge = millis() - purgeLastCallTime;

  if(GENERAL_PURGE_TIME + purgeTime < time_purge) {
   digitalWrite(PURGE_PIN, LOW);
   purgeLastCallTime = millis(); 
   
  }
  else if(GENERAL_PURGE_TIME < time_purge) {
    digitalWrite(PURGE_PIN, HIGH);
  }
}

// ----------------- TEMPERATURE CONTROL -----------------

void getTemperature() {
  /* C20
  Reads the voltage, which is used to approximate the resistance, since the input voltage is
  the controlled 5V output from the arduino. Steinhart_Hart then used to approximate temperature
  Expected Input: Voltage from [0, 5V], or [0, 1024)
  */
  int vTemp = analogRead(STACK_THEMRMISTOR_PIN);
  stackTempArray[stackTempPos] = vTemp;
  stackTempPos = int((stackTempPos + 1) % ARRAY_SIZE);
}

// ----------------- CHANGE STATES FOR PURGE, SUPPLY, FAN, RELAY, RESISTOR -----------------
void setPurgeState(int state) {
  digitalWrite(PURGE_PIN, state);
}
void setSupplyState(int state) {
  digitalWrite(SUPPLY_PIN, state);
}
void setFanState(int state) {
  // will need to use a switch-case 
}

void setRelayState(int state) {
  digitalWrite(FC_RELAY_PIN, state);
}

void setResistorState(int state) {
  digitalWrite(RESISTOR_PIN, state);
}

// ----------------- TURN ALL PINS OFF -----------------

void turnAllOff() {
  digitalWrite(PURGE_PIN, LOW);
  digitalWrite(SUPPLY_PIN, LOW);
  digitalWrite(FC_RELAY_PIN, LOW);
  digitalWrite(FC_FAN_RELAY_PIN, LOW);
  digitalWrite(RESISTOR_PIN, LOW);
}

// ----------------- FLASH ALL PINS -----------------

void flashOn() {
  digitalWrite(PURGE_PIN, HIGH);
  digitalWrite(SUPPLY_PIN, HIGH);
  digitalWrite(FC_RELAY_PIN, HIGH);
  digitalWrite(FC_FAN_RELAY_PIN, HIGH);
  digitalWrite(RESISTOR_PIN, HIGH);
  //digitalWrite(7, HIGH);
  delay(1000);
  turnAllOff();

}

// ----------------- CONTROL OF RGB LEDs -----------------

void setColorState(int red, int green, int blue)
{
  analogWrite(STATE_LED_RED,red);
  analogWrite(STATE_LED_BLUE, blue);
  analogWrite(STATE_LED_GREEN, green);
  
}

//void setColorFan(int red, int green, int blue)
//{
//  analogWrite(FAN_LED_RED,red);
//  analogWrite(FAN_LED_BLUE, blue);
//  analogWrite(FAN_LED_GREEN, green);
//  
//}
