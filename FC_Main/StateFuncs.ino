// Function Declarations of what should occur during each FC_State and FC_Startup_Substate

// Flags
boolean standby_timer_set = false;
boolean fc_fan_time_set = false;
boolean timer_time_set = false;

// Counters
unsigned long startup_purge_counter = 0; // Counter for how long the purge valve has been open for
unsigned long timer_start_time = 0;     // Counter for how long to stay in FC_STANDBY
int count = 0;                  // modulo-1000 -- counter for how long to stay in FC_INITIAL

// ----------------- FC STATE FUNCTIONS ----------------- //
void FCInitial() {
  setColorState(0, LED_ON, 0); // Sets state LED to yellow.

  if (Current_Time > 3000) // A timer to determine how long to be in this state.
    stateTransition(FC_INITIAL, FC_STANDBY);
}

void FCStandby() {
  // The stack is not consuming reactant or delivering power
  // and all stack BOP actuators are in their safe state
  // The system remains in FC_STANDBY for STANDBY_DELAY_TIME
  setColorState(0, 0, LED_ON); // BLUE

  if (!standby_timer_set) {
    timer_start_time = Current_Time;
    standby_timer_set = true;
  }

  //Default State
  fanControl(FAN_OFF);    //Fan speed
  setAllSafe();

  if (STANDBY_DELAY_TIME <= Current_Time - timer_start_time && fc_on) {
    standby_timer_set = false;
    stateTransition(FC_STANDBY, FC_STARTUP);
  }
}

// TODO: I changed this but we need to make sure that it is right in terms of the relay states
void FCStartup() {
  // The stack goes from FC_STANDBY to a state where current can
  // safely be drawn from the stack
  setColorState(LED_ON, LED_ON, 0); // RED, YELLOW

  switch (FC_SubState) {
    case (FC_STARTUP_STARTUP_PURGE):
      FCStartup_StartupPurge();
      break;

    case (FC_STARTUP_END):
      FCStartup_StartupEnd();
      break;
  }
}

void FCStartup_StartupPurge() {
  // purge valve and supply valves are opened simultaneously
  // for the start-up purge and the start-up resistor is applied
  // across the stack to limit voltage


  setSupplyState(OPEN);
  setPurgeState(OPEN);
  setRelayState(OPEN); // Open the state relay.
  setResistorState(CLOSED); // Close the resistor relay as we have reached a stage where we no longer need the start up resistor.

  if (!fc_fan_time_set) { // Timer Reused for PURGE
    startup_purge_counter = Current_Time;
    fc_fan_time_set = true;
  }

  if (STARTUP_PURGE_TIME <= Current_Time - startup_purge_counter) {
    fc_fan_time_set = false;
    subStateTransition(FC_SubState, FC_STARTUP_END);
    return;
  }
}

void FCStartup_StartupEnd() {
  setPurgeState(CLOSED);// close purge valve
  setSupplyState(CLOSED);// fan is minimum

  fc_fan_time_set = false;
  startup_purge_counter = 1;
  stateTransition(FC_State, FC_RUN);
  subStateTransition(FC_SubState, FC_STARTUP_STARTUP_PURGE); // switch back to startup purge

  setResistorState(OPEN); // Open the resistor relay as we have reached a stage where we no longer need the start up resistor.
  setRelayState(CLOSED); // Close the system relay, start the damn thing.

  fanControl(FAN_MID_LOW);
}

void FCRun() {
  // ?? manual info is copied from FC_STANDBY
  setColorState(0, LED_ON, LED_ON); //BLUE and YELLOW led
  setSupplyState(OPEN); // The supply value should always be open.
  setRelayState(CLOSED);
  AutomaticPurgeControl();
  AutomaticFanControl(fc_current, stack_temp); // fc_current, stack_temp are used and modified in CheckAlarms.ino

  if (!fc_on)
    stateTransition(FC_RUN, FC_SHUTDOWN);
}

void FCShutdown() {
  // The stack goes from FC_RUN to FC_STANDBY. The system remains in
  // FC_SHUTDOWN for SHUTDOWN_DELAY_TIME
  fanControl(FAN_MID_HIGH);
  setColorState(LED_ON, LED_ON, LED_ON); // RED BLUE YELLOW on

  if (!timer_time_set) {
    timer_start_time = Current_Time;
    timer_time_set = true;
  }

  //Set all Safe includes FAN_OFF this will which turns the fan off. We need it on here hence the new function
  //setAllSafeButFan
  setSupplyState(CLOSED);
  setPurgeState(CLOSED);
  setRelayState(OPEN);
  setResistorState(OPEN);

  //3 minutes or 20 or ambient
  if ((SHUTDOWN_DELAY_TIME <= Current_Time - timer_start_time) || stack_temp <= amb_temp || stack_temp <= 20) {
    fanControl(FAN_OFF);
    timer_time_set = false;
    stateTransition(FC_SHUTDOWN, FC_STANDBY);
  }
}

void FCAlarm() {
  // The stack is shut down because an alarm was triggered.
  // All actuators are in their safe states
  setAllSafe();
  setColorState(LED_ON, 0, 0); // Sets state LED to red.

}


// ----------------- STATE TRANSITION & SUBSTATE TRANSITION -----------------
void stateTransition(int fromState, int toState) {
  FC_State = toState;
}

void subStateTransition(int fromState, int toState) {
  FC_SubState = toState;
}
