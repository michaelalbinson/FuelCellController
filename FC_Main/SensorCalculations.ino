// ----------------- COMPUTE SENSOR VALUES -----------------

// TODO: are these A B anc C values the same for both sensors???? -- should be
// TODO: if temp calculation is identical, we only need one function!
int stackTemperatureComputation(double averageValue) {
  //Serial.println("Stack Temp");
  //Serial.print("input to func: ");
  //Serial.println(averageValue);
  float V_in = averageValue * 5 / 1023; // measured signal voltage
  //Serial.print("Ain in func: ");
  //Serial.println((float)A_in);
  long R1 = ((5 * (unsigned int) R2_STACK) - (V_in * (unsigned int) R2_STACK)) / V_in;
  //Serial.print("R1 in func: ");
  //Serial.println(R1);
  
  int temp = 1 / (A_TEMP + (B_TEMP * log(R1)) + C_TEMP * log(R1) * log(R1) * log(R1)) - 273.15;
  //Serial.print("Temp in calc func: ");
  //Serial.println((int)temp);

  return (int) temp;
}

int ambTemperatureComputation(double averageValue) {
  //Serial.println("Ambient Temp");
  //Serial.print("input to func: ");
  //Serial.println(averageValue);
  double V_in = averageValue * 5 / 1023;
  //Serial.print("Ain in func: ");
  //Serial.println((float)A_in);
  double R1 = ((5 * (unsigned int) R2_AMBIENT) - (V_in * (unsigned int) R2_AMBIENT)) / V_in;
  //Serial.print("R1 in func: ");
  //Serial.println(R1);
  
  //temp = a_temp * R1 * R1 + b_temp * R1 + c_temp;
  int temp = 1 / (A_TEMP + (B_TEMP * log(R1)) + C_TEMP * log(R1) * log(R1) * log(R1)) - 273.15;
  //Serial.print("Temp in calc func: ");
  //Serial.println((int)temp);

  return (int) temp;
}

int voltageComputation(double averageValue) {
  double V_v_in = averageValue * 5 / 1023;
  double voltage = V_v_in * 28.5 / 1.7; //Accuracy breaks down at 1.4V. Divide 1.7V because reasons...
  
  printData("Voltage", averageValue, (int) voltage);
  return (int) voltage;
}

// TODO: UPDATE -- THIS IS NOT HOW THIS WORKS ANYMORE
int currentComputation(double averageValue) {
  double V_c_in = averageValue * 5 / 1023; // Measured signal voltage in V
  double current = V_c_in * G; // TODO: determine G, this ratio is currently incorrect (100A/50mV) (this does not get actual current)

//  printData("Current", averageValue, (int) current);
  return (int) current;
}

int hydrogenComputation(double averageValue) {
  double H_in = averageValue * 5 / 1023; // Measured signal voltage in V
  double hydrogen = H_in * H_CONST;
  
//  printData("Hydrogen", averageValue, (int) hydrogen);
  return (int) hydrogen;
}

void printData(char* str, int averageValue, int output) {
  Serial.println("Hydrogen");
  Serial.print("input to func: ");
  Serial.println(averageValue);
  Serial.print("output: ");
  Serial.println(output);
}