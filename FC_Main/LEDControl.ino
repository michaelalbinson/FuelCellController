// ----------------- LED CONTROL -----------------
void setColorState(int red, int yellow, int blue) {
  analogWrite(STATE_LED_RED, red);
  analogWrite(STATE_LED_BLUE, blue);
  analogWrite(STATE_LED_YELLOW, yellow);
}       
