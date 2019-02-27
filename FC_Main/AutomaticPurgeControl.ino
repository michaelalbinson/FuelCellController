
unsigned long purgeLastCallTime = 0;  // last time the purge valve was opened

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
