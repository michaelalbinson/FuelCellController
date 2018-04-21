// FC States
#define FC_INITIAL 0
#define FC_STANDBY 1
#define FC_STARTUP 2
#define FC_RUN 3
#define FC_ALARM 4
#define FC_SHUTDOWN 5

// FC Startup Substates
#define FC_STARTUP_FAN_SPOOLUP 6
#define FC_STARTUP_STARTUP_PURGE 7
#define FC_STARTUP_END 8

// Analog Pins
#define HYDROGEN_PIN 2
#define STACK_THEMRMISTOR_PIN 5
#define CURRENT_PIN 3
#define VOLTAGE_PIN 1
#define AMB_THEMRMISTOR_PIN 4

// Digital Pins (finalized for the current configuration)
#define PURGE_PIN 10 
#define SUPPLY_PIN 11
#define FC_RELAY_PIN 12
#define RESISTOR_PIN 13

#define SYSTEM_ON_PIN 7

#define STATE_LED_RED 5
#define STATE_LED_BLUE 8
#define STATE_LED_GREEN 6
#define RELAY1 2
#define RELAY2 3
#define RELAY3 4
 
// Valve, Relay States
#define OPEN 0
#define CLOSED 1

// Other
#define ARRAY_SIZE 100

// Alarm Thresholds TODO: set threshes
#define FC_MIN_CURRENT 0 // A
#define FC_MAX_CURRENT 55 // A
#define FC_STANDBY_MIN_VOLTAGE 0 // V
#define FC_RUN_MIN_VOLTAGE 13 // V
#define FC_MAX_VOLTAGE 28 // V
#define FC_MAX_H2_READ 10 // ppm
#define FC_RUN_MIN_TEMP 0 // deg C 
#define FC_MIN_TEMP 15 // deg C 
#define FC_MAX_TEMP 75 // deg C

// Constant System Parameters
#define PURGE_THRESHOLD 1026.0 //A*s
#define purgeTime 3000

// Delays (number of loops @ 2 ms per loop)
#define GENERAL_PURGE_TIME 5000 // 0.2 s

// Fan Speed Constant.
#define FAN_SPEED_LOW 300
#define FAN_SPEED_MEDIUM 600
#define FAN_SPEED_HIGH 1000

// ThermistorCurveCoefficientsForSteinhart-Hart.
#define a_temp 0.0052
#define b_temp -1.0979
#define c_temp 81.298

#define R2_STACK 60
#define R2_AMBIENT 60

// TODO: redetermine gain value
#define G -78.125

