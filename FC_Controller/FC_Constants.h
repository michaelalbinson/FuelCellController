

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
//SENSOR PROXY BOARD
#define CURRENT_PIN 0             //Max expected at 2.5V
#define STACK_THEMRMISTOR_PIN 1 
#define AMB_THEMRMISTOR_PIN 2
#define VOLTAGE_PIN 3             //Expected input 1.7V
#define HYDROGEN_PIN 4

// Digital Pins (finalized for the current configuration)
//VALVE AND CONTACTOR RELAY BANK
#define PURGE_PIN 5 
#define SUPPLY_PIN 6
#define FC_RELAY_PIN 7
#define RESISTOR_PIN 8
//FAN RELAY BANK(Added)
#define RELAY1 9  
#define RELAY2 10
#define RELAY3 11
//ON BTN
#define SYSTEM_ON_PIN 12
//LED READOUT Changed
#define STATE_LED_RED 4
#define STATE_LED_BLUE 3
#define STATE_LED_YELLOW 2

#define LED_ON 1023
 
// Valve, Relay States
#define OPEN 0
#define CLOSED 1

// Alarm Thresholds TODO: set threshes
#define FC_MIN_CURRENT    0 // A
#define FC_MAX_CURRENT    55 // A
#define FC_STANDBY_MIN_VOLTAGE 0 // V
#define FC_RUN_MIN_VOLTAGE  13 // V
#define FC_MAX_VOLTAGE    28 // V
#define FC_MAX_H2_READ     10 // ppm
#define FC_RUN_MIN_TEMP   0 // deg C 
#define FC_MIN_TEMP       15 // deg C 
#define FC_MAX_TEMP       75 // deg C
#define HYDROGEN_MAX      4.5//

// Constant System Parameters
#define PURGE_THRESHOLD   1026.0 //A*s
#define purgeTime         3000

// Delays (number of loops @ 2 ms per loop)
#define GENERAL_PURGE_TIME 5000 // 0.2 s

// Fan Speeds  
#define FAN_MAX       5
#define FAN_MID_HIGH  4
#define FAN_MID       3
#define FAN_MID_LOW   2
#define FAN_MIN       1
#define FAN_OFF       0


// Flags
boolean fc_on = false;
boolean fc_alarm = false;
boolean fc_fan_time_set = false;
boolean timer_time_set = false;
boolean arrays_filled = false;

// Program constants (determined via testing) TODO: VERIFY
long STARTUP_PURGE_LOOP_COUNT = 5000;
unsigned long Current_Time = 0; // would overflow after 25 days if left running forever (hopefully)
long STANDBY_DELAY_TIME = 5000;
long SHUTDOWN_DELAY_TIME = 5000;

// Counters
long timer_start_time = 0;
int purge_counter = 1;
long purgeLastCallTime = 0;
int arrayIndex = 0;

// Fake Counters
int fanCount = 0;
int count = 0;
int stackTempPos = 0;

// Sensor Arrays
#define ARRAY_SIZE 100
unsigned short stackTempArray[ARRAY_SIZE];
unsigned short stackCurrentArray[ARRAY_SIZE];
unsigned short stackVoltageArray[ARRAY_SIZE];
unsigned short ambientTempArray[ARRAY_SIZE];
unsigned short hydrogenArray[ARRAY_SIZE];

// States
int FC_State = FC_INITIAL; // initial state perhaps we could enumerate these
int FC_SubState = FC_STARTUP_STARTUP_PURGE;
int FAN_State = FAN_MID; //First Fan state assumed is MID

// Averaged values
double amb_temp;
double stack_temp;
double fc_current;
double fc_voltage;
double amb_hydrogen;
double tempInput;

// ThermistorCurveCoefficientsForSteinhart-Hart.
#define a_temp 0.000757//0.0052
#define b_temp 0.000274//-1.0979
#define c_temp 0.000000127//81.298

long const R2_STACK=10000;//Originally 60
long const R2_AMBIENT=10000; //Originally 60

//gain value(Current Sens)
#define G 22.0605 //AnalogIn=9.2745Current+1.2991 ->Max Amps reading at 1023= 110.3A -> 110.3/5=22.06
//Hydrogen Const
#define H_CONST 1 //Data sheet shows a linear relation. Set to one for now
