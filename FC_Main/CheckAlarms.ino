// Index Counter
int arrayIndex = 0; // modulo-100  -- counter for averaging sensor value arrays
int hydrogen_delay = 2000;
// Flags
boolean arrays_filled = false;
boolean arrays_emptied = false;

// Averaged values
double amb_temp;
double stack_temp;
double fc_current;
double fc_voltage;
double amb_hydrogen;
double tempInput;

// Sensor Arrays
#define ARRAY_SIZE 100
unsigned short stackTempArray[ARRAY_SIZE];
unsigned short stackCurrentArray[ARRAY_SIZE];
unsigned short stackVoltageArray[ARRAY_SIZE];
unsigned short ambientTempArray[ARRAY_SIZE];
unsigned short hydrogenArray[ARRAY_SIZE];

// ----------------- CHECK ALARMS -----------------
void Check_Alarms() {
  // Check all alarm properties to make sure they're within the safe range
  // If not, set fc_alarm to true.
  // Current, Voltage, Hydrogen and Temperature (Stack and Ambient) are checked
  if (!arrays_emptied) {
    for (int i = 0; i < ARRAY_SIZE; i++) {
      ambientTempArray[i] = 0;
      stackTempArray[i] = 0;
      stackVoltageArray[i] = 0;
      stackCurrentArray[i] = 0;
      hydrogenArray[i] = 0;
    }
    arrays_emptied = true;
  }
  // Take measurements
  ambientTempArray[arrayIndex]  = analogRead(AMB_THEMRMISTOR_PIN);
  stackTempArray[arrayIndex]    = analogRead(STACK_THEMRMISTOR_PIN);
  stackVoltageArray[arrayIndex] = analogRead(VOLTAGE_PIN);
  stackCurrentArray[arrayIndex] = analogRead(CURRENT_PIN);
  hydrogenArray[arrayIndex]     = analogRead(HYDROGEN_PIN);

  arrayIndex++;
  if (arrayIndex == ARRAY_SIZE) {
    arrays_filled = true;
    arrayIndex = 0;
  }

  // if the arrays have not yet been filled keep taking measurements or their averages will be incorrect
  if (!arrays_filled)
    return;

  unsigned long amb_temp_total     = 0;
  unsigned long stack_temp_total   = 0;
  unsigned long fc_voltage_total   = 0;
  unsigned long fc_current_total   = 0;
  unsigned long amb_hydrogen_total = 0;

  for (int i = 0; i < ARRAY_SIZE; i++) {
    amb_temp_total     += ambientTempArray[i];
    stack_temp_total   += stackTempArray[i];
    fc_voltage_total   += stackVoltageArray[i];
    fc_current_total   += stackCurrentArray[i];
    amb_hydrogen_total += hydrogenArray[i];
  }

  // Uses the average sensor value to determine the actual values for ambient temp, stack temp, etc.
  amb_temp     = ambTemperatureComputation(amb_temp_total / ARRAY_SIZE);
  stack_temp   = stackTemperatureComputation(stack_temp_total / ARRAY_SIZE);
  fc_voltage   = voltageComputation(fc_voltage_total / ARRAY_SIZE);
  fc_current   = currentComputation(fc_current_total / ARRAY_SIZE);
  amb_hydrogen = hydrogenComputation(amb_hydrogen_total / ARRAY_SIZE);

  //Always checking for hydrogen leaking, regardless of state
  //  if (Current_Time > hydrogen_delay) {
  //    if (amb_hydrogen < HYDROGEN_MIN ){
  //      fc_alarm = true;
  //      alarm_sens = amb_hydrogen;
  //      alarm_val= 0;
  //  }
  //}
  if (fc_current < FC_MIN_CURRENT || fc_current > FC_MAX_CURRENT) {
    fc_alarm = true;
    alarm_sens = fc_current;
    alarm_val = 1;
  }
  if (FC_State == FC_RUN) {
    //    if (fc_voltage < FC_RUN_MIN_VOLTAGE || fc_voltage > FC_MAX_VOLTAGE) {
    //      fc_alarm = true;
    //      alarm_sens = fc_voltage;
    //      alarm_val = 2;
    //    }
    if (amb_temp < FC_RUN_MIN_TEMP || amb_temp > FC_MAX_TEMP) {
      fc_alarm = true;
      alarm_sens = amb_temp;
      alarm_val = 3;
    }
    if (stack_temp < FC_RUN_MIN_TEMP || stack_temp > FC_MAX_TEMP) {
      fc_alarm = true;
      alarm_sens = stack_temp;
      alarm_val = 4;
    }
  }
  else {
    if (amb_temp < FC_MIN_TEMP || amb_temp > FC_MAX_TEMP) {
      fc_alarm = true;
      alarm_sens = amb_temp;
      alarm_val = 5;
    }
    if (stack_temp < FC_MIN_TEMP || stack_temp > FC_MAX_TEMP) {
      fc_alarm = true;
      alarm_sens = stack_temp;
      alarm_val = 6;
    }
  }
}
