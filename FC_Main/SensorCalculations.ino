// ----------------- COMPUTE SENSOR VALUES -----------------
int TemperatureComputation(double averageValue) { 

  float V_in = averageValue * 5 / 1023; // measured signal voltage
  long R1 = (V_in*(unsigned int)R2)/(6-V_in);  
  //  long R1 = ((6 * (unsigned int) R2) - (V_in * (unsigned int) R2)) / 6; //
  int temp = 1 / (A_TEMP + (B_TEMP * log(R1)) + C_TEMP * log(R1) * log(R1) * log(R1)) - 273.15;

  return temp;
}

int voltageComputation(double averageValue) {
  double V_v_in = averageValue * 5 / 1023;
  double voltage = V_v_in * 26 / 0.62; //Accuracy breaks down at 1.4V. Divide 1.7V because reasons...
  
  return (int) voltage;
}

int currentComputation(double averageValue) {
  double V_c_in = averageValue * 5 / 1023; // Measured signal voltage in V
  double current = V_c_in * G; // TODO: determine G, this ratio is currently incorrect (100A/50mV) (this does not get actual current)

  return (int) current;
}

int hydrogenComputation(double averageValue) {
  double H_in = averageValue * 5 / 1023; // Measured signal voltage in V
  double hydrogen = H_in * H_CONST;
  
  return (int) hydrogen;
}
