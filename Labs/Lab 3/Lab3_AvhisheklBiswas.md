
<div align="center">

# Internet of Things lab 3 Fall 2023

## Avhishek Biswas, Anurddha , Amlan, Shaswati

</div>


  
## Task 1 : Sensing

### Requirements

1. Sense the internal temperature every second
2. Implement a sliding window to calculate the average temperature over the last 5 second using a stack

---

#### Development Plan:

#### a. Procedure of Solving the Problem




#### b. Configuration Table


---
### 2. Development Process:



#### Subtask 1:


#### Subtask 2: 


---

### 3. Test Plan:

---
### Output:
  <figure>
  <img src="images_for_lab2/task1_output.png">
  <figcaption style="font-weight: bold;">Fig.1 - Console Output.</figcaption>
</figure>

---
### Screenshot

<figure style="text-align: center;">
  <img src="images_for_lab2/task1_serial.png">
  <figcaption style="font-weight: bold;">Fig.2 - Output in the serial monitor.</figcaption>
</figure>

<figure style="text-align: center;">
  <img src="images_for_lab2/task1_yellowonly.jpeg">
  <figcaption style="font-weight: bold;">Fig.3 - Only blinking of Yellow LED.</figcaption>
</figure>

<figure style="text-align: center;">
  <img src="images_for_lab2/task1_blueonly.jpeg">
  <figcaption style="font-weight: bold;">Fig.4 - Only blinking of Blue LED. </figcaption>
</figure>

<figure style="text-align: center;">
  <img src="images_for_lab2/task1_blueandyellow.jpeg">
  <figcaption style="font-weight: bold;">Fig.5 - Blinking of both Yellow and Blue LED.</figcaption>
</figure>

---
### Video Link

##### See working video -  [Link to Video](https://drive.google.com/file/d/1cECTZJBecwsLMgHnVz9khmInNBYX7sHD/view?usp=sharing).

---
---
<div style="margin-top: 200px;"></div>






### Requirements

1. **Blue LED**: The Blue LED should toggle its state every 0.5 seconds.
2. **Yellow LED**: The Yellow LED should toggle its state every 1 second.
3. **Serial Monitor Output**: The program should output the current state of each LED ("on" or "off") to the Serial Monitor whenever the LED state changes.
---

#### Development Plan:

#### a. Procedure of Solving the Problem


1. **Initialization**
    - Open the Arduino IDE and select the SparkFun RF Pro board from the board manager.
    - Select the appropriate port to which the board is connected.

2. **Pin Configuration**
    - Configure the digital pins for the Blue (`PIN_LED_13`) and Yellow (`PIN_LED_RXL`) LEDs as output pins using the `pinMode()` function.

3. **Serial Communication Setup**
    - Initialize Serial communication with a baud rate of 9600 using `SerialUSB.begin(9600)` for debugging and monitoring.

4. **Timer Configuration**
    - Use TC3 in Match Frequency Operation mode.
    - Configure the clock source using `REG_GCLK_CLKCTRL`.
    - Set the timer frequency to 20 Hz using `startTimer(20)`.

5. **Interrupt Handling**
    - Implement the `TC3_Handler()` function to handle timer interrupts.
    - Toggle the Blue LED every 750 ms and the Yellow LED every 1000 ms.

6. **Testing**
    - Verify that the LEDs are blinking at the correct intervals.
    - Monitor the Serial output to ensure the program is running as expected.


#### b. Configuration Table


| Register Name          | Register Function                     | Register Value                  |
|------------------------|---------------------------------------|---------------------------------|
| `REG_GCLK_CLKCTRL`     | Clock Control                         | `GCLK_CLKCTRL_CLKEN`, `GCLK_CLKCTRL_GEN_GCLK0`, `GCLK_CLKCTRL_ID_TCC2_TC3` |
| `TC->CTRLA.reg`        | Timer Control A                       | `TC_CTRLA_ENABLE`               |
| `TC->COUNT.reg`        | Timer Count Register                  | Calculated based on `frequencyHz`|
| `TC->CC[0].reg`        | Timer Compare/Capture Register        | Calculated based on `frequencyHz`|
| `TC->INTFLAG.bit.MC0`  | Timer Interrupt Flag for Match        | `1` to clear flag               |
| `GCLK->GENDIV.reg`     | Clock Generator Division              | `GCLK_GENDIV_ID(2)`, `GCLK_GENDIV_DIV(46875)` |
| `GCLK->GENCTRL.reg`    | Clock Generator Control               | `GCLK_GENCTRL_ID(2)`, `GCLK_GENCTRL_SRC_DFLL48M`, `GCLK_GENCTRL_GENEN` |
| `TC->INTENSET.reg`     | Timer Interrupt Enable Set Register   | `TC_INTENSET_OVF`               |
| `NVIC_EnableIRQ()`     | Nested Vector Interrupt Controller    | `TC3_IRQn`                      |


---
### 2. Development Process:
### Development Process

1. **Initialization**
   - Open the Arduino IDE.
   - Select the SparkFun RF Pro board from the board manager.
   - Choose the appropriate COM port to which the board is connected.

 2. **Pin Configuration**
   - Configure the digital pins for the LEDs:
     - Use `pinMode()` to set `PIN_LED_13` (Blue LED) and `PIN_LED_RXL` (Yellow LED) as output pins.

 3. **Serial Communication Setup**
   - Initialize Serial communication for debugging and real-time monitoring:
     - Use `SerialUSB.begin(9600)` to set the baud rate to 9600.

 4. **Timer Configuration**

    a. Clock Source Configuration
   - Use the `REG_GCLK_CLKCTRL` register to configure the clock source for the timer. This register enables the clock and specifies the clock source.

    b. Timer Configuration
   - Use the `startTimer()` function to configure the timer.
   - Set the timer frequency to 20 Hz by passing `20` as an argument to `startTimer()`.

   c. **Compare Value Calculation**
   - The `setTimerFrequency()` function calculates the compare value (`TC->CC[0].reg`) based on the desired frequency (passed as an argument) and the CPU frequency.
   - This compare value determines when the timer will generate an interrupt.

 5. **Interrupt Handling:**

    a.TC3_Handler() Function - 
   - `TC3_Handler()` is the interrupt service routine for handling timer interrupts from TC3.
   - It checks the `TC->INTFLAG.bit.MC0` bit to see if a match condition has occurred.
   - If a match is detected, the interrupt flag is cleared (`TC->INTFLAG.bit.MC0 = 1`).
   - The function tracks time using variables:
     - `currentTime`: Keeps track of the current time in milliseconds.
     - `blueLEDTime`: Records the time when the Blue LED was last toggled.
     - `yellowLEDTime`: Records the time when the Yellow LED was last toggled.

    b. LED Toggling - 
   - Inside `TC3_Handler()`, conditional statements check the time elapsed since the last LED toggle.
   - If the time exceeds a certain threshold, the corresponding LED is toggled:
     - Blue LED toggles every 750 ms.
     - Yellow LED toggles every 1000 ms.

    c. Serial Output - 
   - SerialUSB.println() is used to print the status of the LEDs to the Serial Monitor for real-time monitoring and debugging.

This development process provides a detailed overview of how the code initializes, configures timers, handles interrupts, and controls LED toggling based on time tracking.
<div style="margin-top: 80px;"></div>
#### Approach 4: Using one timer unit

##### a. TC3 must be in the Match Frequency Operation mode
```arduino
// Use match mode so that the timer counter resets when the count matches the compare register
TC->CTRLA.reg |= TC_CTRLA_WAVEGEN_MFRQ;
while (TC->STATUS.bit.SYNCBUSY == 1); // Wait for synchronization
```
##### b. Report how to configure the CC[0].reg register.
```arduino
// Set the compare value for the desired frequency
TC->CC[0].reg = compareValue;
while (TC->STATUS.bit.SYNCBUSY == 1); // Wait for synchronization
```
---

### 3. Test Plan:

| Component       | Test Description                             | Result | Comment                                       |
|-----------------|----------------------------------------------|-----------------|------------------------------------------------|
| Timer Frequency | Set timer frequency to 20 Hz                | Pass            | To be tested                                   |
| Timer Interrupt | Verify timer interrupt toggles LEDs         | Pass            | To be tested                                   |
| Blue LED        | Toggle on/off every 0.5 seconds (750 ms)    | Pass            | To be tested                                   |
| Yellow LED      | Toggle on/off every 1 second (1000 ms)      | Pass            | To be tested                                   |
| Serial Output   | Display "Blue LED is on/off"                | Pass            | To be tested                                   |
| Serial Output   | Display "Yellow LED is on/off"              | Pass            | To be tested                                   |
| System Level    | Both LEDs toggle at correct intervals      | Pass            | To be tested                                   |


---
### Output:
  <figure>
  <img src="">
  <figcaption style="font-weight: bold;"></figcaption>
</figure>

---
### Screenshot

<figure style="text-align: center;">
  <img src="images_for_lab2/task4_serial.png">
  <figcaption style="font-weight: bold;">Fig.17 - Output in the serial monitor.</figcaption>
</figure>

<figure style="text-align: center;">
  <img src="images_for_lab2/task4_yellowonly.jpeg">
  <figcaption style="font-weight: bold;">Fig.18 - Only blinking of Yellow LED.</figcaption>
</figure>

<figure style="text-align: center;">
  <img src="images_for_lab2/task4_blueonly.jpeg">
  <figcaption style="font-weight: bold;">Fig.19 - Only blinking of Blue LED. </figcaption>
</figure>

<figure style="text-align: center;">
  <img src="images_for_lab2/task4_blueandyellow.jpeg">
  <figcaption style="font-weight: bold;">Fig.20 - Blinking of both Yellow and Blue LED.</figcaption>
</figure>

---
### Video Link

##### See working video -  [Link to Video](https://drive.google.com/file/d/1Z0Eevlciv-8c5KaLxPnYd47Qzbv7vPpI/view?usp=sharing).




<div style="margin-top: 120px;"></div>

---
---

## Appendix 
The following codes for the different tasks:

### Task 1:



```

<div style="margin-top: 200px;"></div>

---
---
---


## References: 
The refernces are as follows:
1. [Arduino Official Documentation](https://gist.github.com/jdneo/43be30d85080b175cb5aed3500d3f989)
2. [Adafruit Learning Resource](https://learn.adafruit.com/multi-tasking-the-arduino-part-1?view=all)
3. Noergaard, Tammy. *Embedded Systems Architecture: A Comprehensive Guide for Engineers and Programmers.* Newnes, 2012.
4. Davies, John H. *MSP430 Microcontroller Basics.* Elsevier, 2008.


