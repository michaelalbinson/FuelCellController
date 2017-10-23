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
#define THEMRMISTOR_PIN 0
#define CURRENT_PIN 1
#define VOLTAGE_PIN 2
#define HYDROGEN_PIN 3
#define PRESSURE_PIN 4

// Digital Pins
#define SYSTEM_ON_PIN 2
#define PURGE_PIN 3
#define SUPPLY_PIN 4
#define FC_RELAY_PIN 5
#define FC_FAN_RELAY_PIN 14
#define RESISTOR_PIN 1

//Digital State LED Pins
#define STACK_ON_LED 13
#define STANDBY_LED 12
#define STARTUP_LED 11
#define STARTUP_PURGE_LED 10
#define STARTUP_END_LED 9
#define RUN_LED 8
#define SHUTDOWN_LED 7
#define ALARM_LED 6

// Valve, Relay States (note that this may need to change)
#define OPEN 0
#define CLOSED 1

// Other
#define ARRAY_SIZE 300 // will need to be reduced for testing on an uno
#define StackTempFixedResistance 10000

// Alarm Thresholds TODO: set threshes
#define FC_MIN_CURRENT 0 // A
#define FC_MAX_CURRENT 30 // A
#define FC_STANDBY_MIN_VOLTAGE 0 // V
#define FC_RUN_MIN_VOLTAGE 13 // V
#define FC_MAX_VOLTAGE 55 // V
#define FC_MAX_H2_READ 10 // ppm
#define FC_MIN_TEMP 15 // deg C
#define FC_MAX_TEMP 70 // deg C

// Constant System Parameters
#define PURGE_THRESHOLD 1026.0 //A*s
#define LOOP_TIME 0.002

// Delays (number of loops @ 2 ms per loop)
#define GENERAL_PURGE_TIME 100 // 0.2 s
