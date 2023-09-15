
<div align="center">

# Internet of Things lab 3 Fall 2023

## Group 2 : Avhishek Biswas, Anuruddha Ekanayake, Amlan Balabantaray, Shaswati Behera
</div>


  
## Task 1 : Sensing

### Requirements
1. Sense the internal temperature every second.
2. Implement a sliding window to calculate the average temperature over the last 5 second using a stack.

---

#### Development Plan:

#### a. Procedure of Solving the Problem

1. Install the temperature library. ``#include <TemperatureZero.h>``
2. Import and initialize the temperature. `TempZero.init()`
3. Create a function **ReadTemp()** to read the temperature at every second.
4. Set a general clock ``REG_GCLK_CLKCTRL``.
5. Set a timer **TC4** in Matched Freqeuncy mode and make it tick for every 1 second.
6. Create a flag variable that will add turn on to led the temperature to be read.
7. Inside the readtemp function, also create a global counter ``reportCount`` to  count of number of readings.
8. When reportCount is equal to 5 we calculate the average and store it in the variable ``avgTemperature`` which is also defined globally.


#### b. Configuration Table

| Requirement          | Register Name | Register Function       | Register Value |
|----------------------|---------------|-------------------------|----------------|
| Clock Configuration  | `REG_GCLK_CLKCTRL` | Configure Generic Clock Control | `GCLK_CLKCTRL_CLKEN | GCLK_CLKCTRL_GEN_GCLK0 | GCLK_CLKCTRL_ID_TCC2_TC3` |
| Timer Configuration  | `TC->CTRLA.reg` | Timer Control A Register | `TC_CTRLA_MODE_COUNT16 | TC_CTRLA_WAVEGEN_MFRQ | TC_CTRLA_PRESCALER_DIV1024` |
| Timer Frequency      | `TC->CC[0].reg` | Timer Compare/Capture Register | `compareValue` |
| Timer Interrupt      | `TC->INTENSET.reg` | Timer Interrupt Enable Set Register | `TC_INTENSET.bit.MC0 = 1` |
| NVIC Configuration   | `NVIC_EnableIRQ()` | Nested Vector Interrupt Controller | `TC3_IRQn` |

#### b. Run-time Errors
| Error Code       | Error |
|--------------|--------|
mem0 |  memory overflow|
timer | timer counting error |
flag0 | seconds counting flag mismatch | 

---
### 2. Development Process:

#### Subtask 1:

```arduino
void setup() {
  SerialUSB.begin(9600);
  TempZero.init();
  pinMode(PIN_LED_13, OUTPUT);
  startTimer(1);
}

float Readtemp() {
  if (canReadTemp) {
    float temperature = TempZero.readInternalTemperature();
    SerialUSB.print("Internal Temperature is: ");
    SerialUSB.println(temperature);
    }
```
#### Subtask 2: 

```arduino
float Readtemp() {
  if (canReadTemp) {
    float temperature = TempZero.readInternalTemperature();
    SerialUSB.print("Internal Temperature is: ");
    SerialUSB.println(temperature);
    // Remove the oldest temperature from the sum
    tempSum -= tempBuffer[tempIndex];
    
    // Add the new temperature to the buffer and sum
    tempBuffer[tempIndex] = temperature;
    tempSum += temperature;

    // Update the index for the oldest temperature
    tempIndex = (tempIndex + 1) % WINDOW_SIZE;

    // Update the count of temperatures added to the buffer
    if (tempCount < WINDOW_SIZE) {
      tempCount++;
    }

    // Increment the report counter
    reportCount++;

    // If 5 readings have been taken, calculate the average temperature
    if (reportCount >= WINDOW_SIZE) {
      avgTemperature = tempSum / tempCount;
      newAvgAvailable = true;  // Set the flag
      reportCount = 0;  // Reset the report counter
    }

    canReadTemp = false;
  }
  return avgTemperature;
}
```

---

### 3. Test Plan:
1. **LED Blinking**: 
    - Verify that the Blue LED toggles on and off at an interval of 500 milliseconds.
    - Verify that the Yellow LED toggles on and off at an interval of 1000 milliseconds.

2. **Serial Output**: 
    - Verify that the Serial Monitor displays the correct LED status messages ("Blue LED is on/off", "Yellow LED is on/off") in real-time.


| Component       | Test Description                           | Result  | Comment                                       |
|-----------------|--------------------------------------------|---------|------------------------------------------------|
| Blue LED        | Toggle on/off at 1s interval            | Pass    | LED toggled as expected at 1-second intervals  |
| Serial Output   | Display "Blue LED is on/off"                | Pass    | Messages displayed correctly in the serial monitor |
| Record Temp at 1sec   | Display "Internal Temperature is: "                | Pass    | Messages displayed correctly in the serial monitor |
| Record Average Temp at 5sec   | Display "Average Temperature over last 5 seconds is:"                | Pass    | Messages displayed correctly in the serial monitor |
| System Level    | Both LEDs toggle at correct intervals       | Pass    | Both LEDs toggled at their respective intervals without conflict |


---
### Screenshot

<figure style="text-align: center;">
  <img src="images_for_lab3/temperature_persecond_output.png">
  <figcaption style="font-weight: bold;">Fig.1 - Output showing temperature being recorded and reported every second,with LED turned on and off for each second.</figcaption>
</figure>

<figure style="text-align: center;">
  <img src="images_for_lab3/AverageTemp_output.png">
  <figcaption style="font-weight: bold;">Fig.2 - Output showing temperature being recorded and reported every 5 seconds,with LED turned on and off for each second.</figcaption>
</figure>

<div style="margin-top: 200px;"></div>

---
---
---

  ## Task 3 : Error Logging

### Requirements
1.   System reset
    1. Implement a WDT and enable the WDT interrupt as we learnt in lab 1 and lab 2

2. Communication error
   1. Packet reception failure: see example code
   2. Missing packet
   
3. Error log structure
   1. Timestamp
   2. Error code
   
4.  Store only the error code in the flash storage

---

#### Development Plan:

#### a. Procedure of Solving the Problem
1.	WDT is implemented for every 2s and is kicked in `TC4_handler()`
2.	When WDT interrupt occurs, `DSU_STATUSA` and `DSU_STATUSB` register data are collected.
3.	Error codes from `RCAUSE_REG, REG_DSU_STATUSA and REG_DSU_STATUS` is added to the transmission packet from every slave node to the master node. The master node will save those error codes in the flash storage. 
4.	Communication error logging code is like example code, but packet IDs (current and previous id) are tracked for each transmission and reception.
5.	In the serial monitor status of the flash storage are printed with the time stamp.



#### b. Configuration Table
#### Bit Number and Associated Error Information

| Bit Number | Concerned Register | Error Information |
|------------|--------------------|-------------------|
| 0          | RCAUSE             | System Reset      |
| 1          |       RCAUSE             | External Reset    |
| 2          |             RCAUSE       | Watchdog Reset    |
| 3          | STATUS A           | This bit is set when a command that is not allowed in protected state is issued. |
| 4          | STATUS A                   | This bit is set when a DSU operation failure is detected. |
| 5          |STATUS A                    | This bit is set when a bus error is detected. |
| 6          | STATUS A                   | This bit is set when a debug adapter Cold-Plugging is detected, which extends the CPU reset phase. |
| 7          | STATUS B           | This value is set when Hot-Plugging is enabled. |
| 8          |    STATUS B                | This bit is set when DCC1 is written. This bit is cleared when DCC1 is read. |
| 9          |         STATUS B           | This bit is set when DCC0 is written. This bit is cleared when DCC0 is read. |
| 10         |  STATUS B                  | This bit is set when a debugger probe is detected. |
| 11         | STATUS B                   | Missing Packet Error |
| 12         | STATUS B                   | Packet Receive Error |



#### b. Run-time Errors



---
### 2. Development Process:
1.	For task 3, we used code from LAB1 for setting up the WDT but to set up the early warning interrupt we are using WDT_Handler that we got from the Arduino Zero website. 
2.	For transmitting error codes, we used a 16 cells char array with each cell used store a flag for a particular error. 
3.	Then, this char array was passed on to master node where we traverse this error code using if-else constraints to display the right error message. The following table shows the cell number and the respective error causes. 

### 3. Test Plan:

1.	Test of packets: The packets were printed to serial monitor in all the nodes to check whether all the error logging messages were contained on the packet. 
2.	Test our code with system reset, WDT reset and external reset. 
3.	Print all the packets gathered by leader node to check if all the communications are happening correctly
4.	Write all the error codes to flash storage 
 
#### Component Test Results and Comments

| Component      | Test                  | Results | Comments                                              |
|----------------|-----------------------|---------|-------------------------------------------------------|
| communication  | Missing packet        | Pass    | No errors due to good collision avoidance in code     |
|                | Packet reception failure | Pass  | No errors due to good collision avoidance in code     |
| WDT            | Resetting             | -       | Could not test                                        |
| STATUSA        | -                     | -       | Could not test                                        |
| STATUSB        | -                     | -       | Could not test                                        |

#### Code solutions:
1. **System reset** : The below code checks the reason of reset from the `RCAUSE` register. The `WDT_Handler` checks the `REG_DSU_STATUSA` and the `REG_DSU_STATUSB`. The Watchdog is there to track the failure from the nodes, below code is from one of the codes. 
   
  ###### Code
  ```arduino
  void reset_reason(){
 //determine last reset type
 uint8_t reset_reg = PM->RCAUSE.reg;
 SerialUSB.println(bitRead(reset_reg, 6)); 
 if(bitRead(reset_reg, 6)) {error_bits[0]=1;}//--------------------------------------------changed
 if(bitRead(reset_reg, 5)) {error_bits[1]=1;}
 if(bitRead(reset_reg, 4)) {error_bits[2]=1;}
//  bitWrite(error_bits, 0, bitRead(reset_reg, 6));
//  bitWrite(error_bits, 1, bitRead(reset_reg, 5));
//  bitWrite(error_bits, 2, bitRead(reset_reg, 4));
//   bitWrite(error_bits, 13, 1);
}

void WDT_Handler() {
    //SerialUSB.println("WDT Interrupt");
    uint8_t reg_statusA = REG_DSU_STATUSA;
    uint8_t reg_statusB = REG_DSU_STATUSB;
    uint16_t status_errors = 0x0000;
    status_errors = reg_statusA;
    status_errors = status_errors << 8;
    status_errors |= reg_statusB;
    
    if(bitRead(status_errors, 12)) {error_bits[3]=1;}
    if(bitRead(status_errors, 11)) {error_bits[4]=1;}
    if(bitRead(status_errors, 10)) {error_bits[5]=1;}
    if(bitRead(status_errors, 9)) {error_bits[6]=1;}
    if(bitRead(status_errors, 4)) {error_bits[7]=1;}
    if(bitRead(status_errors, 3)) {error_bits[8]=1;}
    if(bitRead(status_errors, 2)) {error_bits[9]=1;}
    if(bitRead(status_errors, 1)) {error_bits[10]=1;}
}
  ```
  ---
2.**Communication Error** 
  ###### Code from Master Node when recieveing from each node
```arduino
           
            if(current_packet_id-previouse_packet_id > 1) {
              receivedPacket->error[11]='1';
            }           
          // check the missing packet 
          // write_error(receivedPacket->nodeID, 19) ; 
            previouse_packet_id = current_packet_id;
          } 
         else
         {
         SerialUSB.println("Recieve failed");
         char temp1[]={'0','0','0','0','0','0','0','0','0','0','0','0','1','0','0','0'};
         write_error(5, temp1);                               // Save the error code as message receive failed 
         }
```

3. **Error log structure** - Error Log structure from Master Node which checks the error code from each node with the node error database to log the exact error in the flash memory later and print it.
           
```arduino
void print_errors(char* err){
  SerialUSB.print("Timestamp :")
  SerialUSB.print(millis())
  SerialUSB.println("Error =  ");
  // SerialUSB.println(err);
  // SerialUSB.println(err.charAt(1));

 if (err[0] == '1') {
 SerialUSB.println("System Reset");
 }
 
 if (err[1] == '1') {
 SerialUSB.println("Watchdog Reset");
 }
 if (err[2] == '1') {
 SerialUSB.println("External Reset");
 }
 if (err[3] == '1') {
 SerialUSB.println("Command not allowed error: PROTECTED STATE");
 }
 if (err[4] == '1') {
 SerialUSB.println("DSU Operation Failure Error");
 }
 if (err[5] == '1') {
 SerialUSB.println("BUS ERROR Detected!");
 }
 if (err[6] == '1') {
 SerialUSB.println("COLD PLUGGING ERROR!!");
 }
 if (err[7] == '1') {
 SerialUSB.println("HOT PLUGGING ERROR!");
 }
 if (err[8] == '1') {
 SerialUSB.println("DCC1 Written Error");
 }
 if (err[9] == '1') {
 SerialUSB.println("DCC0 Written Error");
 }
 if (err[10] == '1') {
 SerialUSB.println("Debugger Probe Error");
 }
 if (err[11] == '1') {
 SerialUSB.println("Missing Packet Error!");
 }
 if (err[12] == '1') {
 SerialUSB.println("Packet Receival Error");
 }
}
```

4. **Store only the error code in the flash storage**

```arduino
void write_error(uint8_t Node_name, char* ecodeX) //---------changed
{
  int ecode=atoi(ecodeX);
  if(Node_name==3)
  {error_Amlan.write(ecode);}

   if(Node_name==2)
  {error_Shaswati.write(ecode);}

    if(Node_name==4)
  {error_Anuruddha.write(ecode);}

    if(Node_name==1)
  {error_Avhishek.write(ecode);}
    
    if(Node_name==5)
  {error_receive.write(ecode);}
}

FlashStorage(error_Amlan, int);                 // Reserve a portion of flash memory, remove/comment when upload to the transmitter 
FlashStorage(error_Shaswati, int);                                                   
FlashStorage(error_Anuruddha, int);
FlashStorage(error_Avhishek, int);
FlashStorage(error_receive, int);

SerialUSB.println();
//FLASH STORAGE DETAILS PRINT
SerialUSB.println("FLASH STORAGE RESULTS");
SerialUSB.print("AMLAN: ");SerialUSB.print(error_Amlan.read());
SerialUSB.print(", SHASWATI: ");SerialUSB.print(error_Shaswati.read());
SerialUSB.print(", ANURUDDHA: ");SerialUSB.print(error_Anuruddha.read());
SerialUSB.print(", AVHISHEK: ");SerialUSB.print(error_Avhishek.read());
SerialUSB.print(", ERROR RECEIVE ");SerialUSB.println(error_receive.read());

```
   
<div style="margin-top: 200px;"></div>

---
---
---






## Task 4 : Timer

### Requirements
1. For periodical tasks, use the timer technique we learnt from last lab.

---

#### Development Plan:

#### a. Procedure of Solving the Problem

1. The function `startTimer(int frequencyHz)` is the entry point for setting up the timer. It calls two other functions: `configureClock()` and `configureTimer(frequencyHz)`.
2. The function `configureClock()` sets up the general clock by writing to the `REG_GCLK_CLKCTRL` register and waiting for synchronization.
3. The function `configureTimer(int frequencyHz)` sets up the timer by configuring its control register, setting its frequency, and enabling interrupts.
4. Inside `configureTimer()`, the function `setTimerFrequency(TC, frequencyHz)`` is called to set the timer frequency based on the input frequency in Hz.
5. The code enables timer interrupts by setting the `TC->INTENSET.bit.MC0 = 1` and specifies that the interrupt to be used is `TC3_IRQn`.
6. The function `TC3_Handler()` serves as the interrupt handler. It toggles an LED and prints a message to the serial port.
7. The function `setTimerFrequency(TcCount16* TC, int frequencyHz)` calculates the compare value for the timer based on the desired frequency.


#### b. Configuration Table

| Requirement          | Register Name       | Register Function                          | Register Value                                                                 |
|----------------------|---------------------|--------------------------------------------|---------------------------------------------------------------------------------|
| Clock Configuration  | `REG_GCLK_CLKCTRL`  | Configure Generic Clock Control            | `GCLK_CLKCTRL_CLKEN \| GCLK_CLKCTRL_GEN_GCLK0 \| GCLK_CLKCTRL_ID_TCC2_TC3`     |
| Timer Configuration  | `TC->CTRLA.reg`     | Timer Control A Register                   | `TC_CTRLA_MODE_COUNT16 \| TC_CTRLA_WAVEGEN_MFRQ \| TC_CTRLA_PRESCALER_DIV1024`  |
| Timer Frequency      | `TC->CC[0].reg`     | Timer Compare/Capture Register              | `compareValue` (Calculated based on `frequencyHz`, `CPU_HZ`, and `TIMER_PRESCALER_DIV`) |
| Timer Interrupt      | `TC->INTENSET.reg`  | Timer Interrupt Enable Set Register        | `TC_INTENSET.bit.MC0 = 1`                                                       |
| NVIC Configuration   | `NVIC_EnableIRQ()`  | Nested Vector Interrupt Controller         | `TC3_IRQn`                                                                      |
| Clock Sync           | `GCLK->STATUS.bit.SYNCBUSY` | Clock Synchronization Busy Status    | `1` (Wait until it becomes 0)                                                   |
| Timer Sync           | `TC->STATUS.bit.SYNCBUSY`   | Timer Synchronization Busy Status      | `1` (Wait until it becomes 0)                                                   |

#### b. Run-time Errors

| Error Code       | Error Description |
|------------------|-------------------|
| `clock_sync`     | Synchronization busy flag for the clock never clears, causing an infinite loop in `configureClock()` |
| `timer_sync`     | Synchronization busy flag for the timer never clears, causing an infinite loop in `configureTimer()` and `setTimerFrequency()` |
| `clock_config`   | Incorrect configuration of the general clock in `configureClock()` leading to undesired behavior |
| `timer_config`   | Incorrect timer configuration in `configureTimer()` leading to undesired behavior |
| `freq_calc`      | Incorrect calculation of `compareValue` in `setTimerFrequency()` leading to incorrect timer frequency |
| `irq_fail`       | Failure to enable the interrupt correctly, causing `TC3_Handler()` not to be called |
| `flag_mismatch`  | `TC->INTFLAG.bit.MC0` is not set or cleared correctly, causing issues in `TC3_Handler()` |

---
### 2. Development Process:

#### Subtask 1:

```arduino
void startTimer(int frequencyHz) {
  configureClockTC4();
  configureTimerTC4(frequencyHz);
}
void configureClockTC4() {
  REG_GCLK_CLKCTRL = (uint16_t)(GCLK_CLKCTRL_CLKEN | GCLK_CLKCTRL_GEN_GCLK0 | GCLK_CLKCTRL_ID_TC4_TC5);
  while (GCLK->STATUS.bit.SYNCBUSY == 1);
}

void configureTimerTC4(int frequencyHz) {
  TcCount16* TC = (TcCount16*)TC4;
  TC->CTRLA.reg &= ~TC_CTRLA_ENABLE;
  while (TC->STATUS.bit.SYNCBUSY == 1);

  TC->CTRLA.reg |= TC_CTRLA_MODE_COUNT16 | TC_CTRLA_WAVEGEN_MFRQ | TC_CTRLA_PRESCALER_DIV1024;
  while (TC->STATUS.bit.SYNCBUSY == 1);

  setTimerFrequencyTC4(TC, frequencyHz);
  TC->INTENSET.reg = 0;
  TC->INTENSET.bit.MC0 = 1;

  NVIC_EnableIRQ(TC4_IRQn);
  TC->CTRLA.reg |= TC_CTRLA_ENABLE;
  while (TC->STATUS.bit.SYNCBUSY == 1);
}

void setTimerFrequencyTC4(TcCount16* TC, int frequencyHz) {
  int compareValue = (CPU_HZ / (TIMER_PRESCALER_DIV * frequencyHz)) - 1;
  TC->CC[0].reg = compareValue;
  while (TC->STATUS.bit.SYNCBUSY == 1);
}

void TC4_Handler() {
  static bool isLEDOn = false;
  TcCount16* TC = (TcCount16*)TC4;
  if (TC->INTFLAG.bit.MC0 == 1) {
    TC->INTFLAG.bit.MC0 = 1;
    digitalWrite(PIN_LED_13, isLEDOn);
    SerialUSB.println(isLEDOn ? "Blue LED is on" : "Blue LED is off");
    isLEDOn = !isLEDOn;
    canReadTemp = true;
  }
}
```


---

### 3. Test Plan:
1. **LED Blinking using Timer**: 
    - Verify that the Yellow LED toggles on and off at an interval of 1000 milliseconds.

2. **Serial Output**: 
    - Verify that the Serial Monitor displays the correct LED status messages ("Blue LED is on/off") in real-time.


| Component       | Test Description                           | Result  | Comment                                       |
|-----------------|--------------------------------------------|---------|------------------------------------------------|
| Blue LED        | Toggle on/off at 1s interval            | Pass    | LED toggled as expected at 1-second intervals  |
| Serial Output   | Display "Blue LED is on/off"                | Pass    | Messages displayed correctly in the serial monitor |
| System Level    | LEDs toggle at correct intervals  of 1sec  | Pass    | Both LEDs toggled at their respective intervals without conflict |


---
### Screenshot

<figure style="text-align: center;">
  <img src="images_for_lab3/AverageTemp_output.png">
  <figcaption style="font-weight: bold;">Fig.2 - Blue LED turning on and off every 1 sec based on ticks from the timer. </figcaption>
</figure>

<div style="margin-top: 200px;"></div>


---
---
---




## Appendix

### Task 1  and Task 4 :
```arduino
#define CPU_HZ 48000000
#define TIMER_PRESCALER_DIV 1024
#define WINDOW_SIZE 5  // Size of the sliding window

#include <TemperatureZero.h>

TemperatureZero TempZero = TemperatureZero();
volatile bool canReadTemp = false;
float tempBuffer[WINDOW_SIZE];  // Circular buffer to hold temperature values
int tempIndex = 0;  // Index to keep track of the oldest temperature value
float tempSum = 0;  // Sum of temperatures in the buffer
int tempCount = 0;  // Count of temperatures added to the buffer
int reportCount = 0;  // Count of readings before reporting
float avgTemperature = 0;  // To hold the average temperature
bool newAvgAvailable = false;  // Flag to indicate a new average is available


void setup() {
  SerialUSB.begin(9600);
  TempZero.init();
  pinMode(PIN_LED_13, OUTPUT);
  startTimer(1);
}

void loop() {
  Readtemp();
  if (newAvgAvailable) {
    SerialUSB.print("Average Temperature over last 5 seconds is: ");
    SerialUSB.println(avgTemperature);
    newAvgAvailable = false;  // Reset the flag
  }
}

void startTimer(int frequencyHz) {
  configureClock();
  configureTimer(frequencyHz);
}

void configureClock() {
  REG_GCLK_CLKCTRL = (uint16_t)(GCLK_CLKCTRL_CLKEN | GCLK_CLKCTRL_GEN_GCLK0 | GCLK_CLKCTRL_ID_TCC2_TC3);
  while (GCLK->STATUS.bit.SYNCBUSY == 1);
}

void configureTimer(int frequencyHz) {
  TcCount16* TC = (TcCount16*)TC3;
  TC->CTRLA.reg &= ~TC_CTRLA_ENABLE;
  while (TC->STATUS.bit.SYNCBUSY == 1);

  TC->CTRLA.reg |= TC_CTRLA_MODE_COUNT16 | TC_CTRLA_WAVEGEN_MFRQ | TC_CTRLA_PRESCALER_DIV1024;
  while (TC->STATUS.bit.SYNCBUSY == 1);

  setTimerFrequency(TC, frequencyHz);
  TC->INTENSET.reg = 0;
  TC->INTENSET.bit.MC0 = 1;

  NVIC_EnableIRQ(TC3_IRQn);
  TC->CTRLA.reg |= TC_CTRLA_ENABLE;
  while (TC->STATUS.bit.SYNCBUSY == 1);
}

void setTimerFrequency(TcCount16* TC, int frequencyHz) {
  int compareValue = (CPU_HZ / (TIMER_PRESCALER_DIV * frequencyHz)) - 1;
  TC->CC[0].reg = compareValue;
  while (TC->STATUS.bit.SYNCBUSY == 1);
}

void TC3_Handler() {
  static bool isLEDOn = false;
  TcCount16* TC = (TcCount16*)TC3;
  if (TC->INTFLAG.bit.MC0 == 1) 
  {
    TC->INTFLAG.bit.MC0 = 1;
    digitalWrite(PIN_LED_13, isLEDOn);
    SerialUSB.println(isLEDOn ? "Blue LED is on" : "Blue LED is off");
    isLEDOn = !isLEDOn;
    canReadTemp = true;
  }
}

float Readtemp() {
  if (canReadTemp) {
    float temperature = TempZero.readInternalTemperature();
    SerialUSB.print("Internal Temperature is: ");
    SerialUSB.println(temperature);
    // Remove the oldest temperature from the sum
    tempSum -= tempBuffer[tempIndex];
    
    // Add the new temperature to the buffer and sum
    tempBuffer[tempIndex] = temperature;
    tempSum += temperature;

    // Update the index for the oldest temperature
    tempIndex = (tempIndex + 1) % WINDOW_SIZE;

    // Update the count of temperatures added to the buffer
    if (tempCount < WINDOW_SIZE) {
      tempCount++;
    }

    // Increment the report counter
    reportCount++;

    // If 5 readings have been taken, calculate the average temperature
    if (reportCount >= WINDOW_SIZE) {
      avgTemperature = tempSum / tempCount;
      newAvgAvailable = true;  // Set the flag
      reportCount = 0;  // Reset the report counter
    }

    canReadTemp = false;
  }
  return avgTemperature;
}
```








## References: 
The refernces are as follows:
1. [Arduino Official Documentation](https://gist.github.com/jdneo/43be30d85080b175cb5aed3500d3f989)
2. [Adafruit Learning Resource](https://learn.adafruit.com/multi-tasking-the-arduino-part-1?view=all)
3. Noergaard, Tammy. *Embedded Systems Architecture: A Comprehensive Guide for Engineers and Programmers.* Newnes, 2012.
4. Davies, John H. *MSP430 Microcontroller Basics.* Elsevier, 2008.


