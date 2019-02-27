unsigned long purgeLastCallTime = 0;  // last time the purge valve was opened
double AmpSecSincePurge = 0;
long PurgeOpenCounter = 0;

boolean purgeIsOpen = false;

// ----------------- PURGE CONTROL -----------------
// TODO: use amperage threshhold, this may or may not work
// TODO: purge interval should be used, not a constant time i.e. GENERAL_PURGE_TIME
void AutomaticPurgeControl() {
  // If Purge Valve is open, reset the counter. If the Purge Valve is closed, add AmpSec
  if (purgeIsOpen) {
  	AmpSecSincePurge = 0;
  } else {
  	AmpSecSincePurge += fc_current * LOOP_TIME;
  }

  // If PURGE_INTERVAL is exceeded, open the Purge Valve and set the counter for the number of loops
  // to keep it open for.
  if (AmpSecSincePurge > PURGE_INTERVAL) {
  	setPurgeState(OPEN);
  	purgeIsOpen = true;
  	PurgeOpenCounter = PURGE_DURATION;
  } else {
  	// If the counter is greater than 0 (i.e. we have not yet waited PURGE_DURATION), 
  	// keep the Purge Valve open and decrement the counter. 
  	// Once the counter is 0 or goes negative, close the purge valve.
  	if (PurgeOpenCounter > 0) {
  	  setPurgeState(OPEN);
  	  purgeIsOpen = true;
  	  PurgeOpenCounter -= LOOP_TIME;
  	} else {
  	  // Done purging
  	  setPurgeState(CLOSED);
  	  purgeIsOpen = false;
  	}
  }
}
