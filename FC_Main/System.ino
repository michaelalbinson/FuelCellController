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
