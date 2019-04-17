# FuelCellController
Arduino code for Queen's Fuel Cell Team

## Code Documentation

### Arduino Standard


|**Name**     |**Desctiption**                                                        |**Aruguments** |
|-------------|-----------------------------------------------------------------------|---------------|
|setup()    |Initiates all the GPIO pins used by the Arduino, it is only run once on Arduino startup|None         |
|loop()|Runs repeatedly and controls the main flow of the program                              |None         |

#### FC Controller Specific


|**Name**                   |**Description**                                                                          |**Aruguments** |
|-------------------------|---------------------------------------------------------------------------------------|-------------|
|Check_Alarms()           | Checks all GPIO threshholds to make sure they are all in the safe range. If one is not, kick off the shutdown of the FC Controller and fuel cell. Temperature, Pressure, Current, Voltage, Hydrogen are checked. | None|
|System()                 | As defined in the documentation, is hypothetically used to set CurrentRequest, which would, again, hypothetically control the output of the fuel cell. However, in practice, it has been unclear what the actual purpose of the function is as it does not appear that the controller itself is really capable of ramping up or down the output of the fuel cell. Remains included for cohesion with the documentation| None|
|FC()                     | The top-level controller of the Finite State Machine that controls the actuators connected to the fuel cell. Keeps track of the state of the FSM and executes the objectives of the current state.| None|
|stateTransision()        | Transisions the FSM from one state to the next| fromState: The current state, toState: the state to transision to|
|subStateTransition()     | Transisions the FSM from one subState to the next (only applicable within the FC_STARTUP state)| fromState: The current state, toState: the state to transision to|
|FCStartup()              | A state machine within a state machine only for the FC_STARTUP state| None|
|Steinhart_Hart()         | Used to convert the input signal of the thermistor to a temperature in degrees celcius| resistance: the input signal from the thermistor, a [b, c]: correction coefficients for the model, should be calibrated using at least a three point model for each thermistor that is used|
|getStackTemperature()    | | None|
|AutomaticFanControl()    | Theoretically would control a fan although there have been arguments that a fan would not be needed to run the stack| None|
|AutomaticPurgeControl()  | Calculates the whether or not to open and close the purge valve, does so independently of the rest of the system |None |
|StackCurrentRampControl()| Theoretically would control the current output of the stack as the CurrentRequest changes, however, we need to know more about the motor before this could be implemented| None |
|updateStackTemperature() | Reads the voltage, which is used to approximate the resistance, since the input voltage is the controlled 5V output from the Arduino. Steinhart_Hart then used to approximate temperature. Expected Analog Read values: Voltage from [0, 5V], or [0, 1024)  |None|
|getAverageIntArray()| Averages and returns the contents of an integer array| int* arr: the integer array to average|
|average()| Averages and returns the contents of an integer array **duplicate**| 



### Things I need answered/want:
*A secondary system showing system performance to the driver
