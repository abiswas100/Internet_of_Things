
  

# IOT lab 1

  

## By Avhishek Biswas

  

  

### Task 1 :

  

#### 1. Set up a WDT that reset without clearing it.

  

```arduino

  

void setup() {

  

// Initialize the blue LED pin as an output (assuming it's connected to pin 7)

SerialUSB.begin(9600); // Set the baud rate to 9600

pinMode(LED_BUILTIN, OUTPUT);

digitalWrite(LED_BUILTIN, HIGH); // Set the blue LED HIGH at the beginning

  

GCLK->GENDIV.reg = GCLK_GENDIV_ID(2) | GCLK_GENDIV_DIV(3);

  

// Disable the watchdog

WDT->CTRL.reg = 0;

  

// Initialize Clock Generator 2 for 2048 Hz

GCLK->GENCTRL.reg = GCLK_GENCTRL_ID(2) |

GCLK_GENCTRL_GENEN |

GCLK_GENCTRL_SRC_OSCULP32K |

GCLK_GENCTRL_DIVSEL;

  

while(GCLK->STATUS.bit.SYNCBUSY); // Think about why this is used

GCLK->CLKCTRL.reg = GCLK_CLKCTRL_ID_WDT |

GCLK_CLKCTRL_CLKEN |

GCLK_CLKCTRL_GEN_GCLK2;

  

while (WDT->STATUS.bit.SYNCBUSY); // Wait for synchronizatioN

  

// set the timer to 2 sec

WDT->CONFIG.bit.PER = 0x09; // Set the timeout period to 2 seconds

  

// Enable the WDT

WDT->CTRL.bit.ENABLE = 1; // ENABLE WDT

while(GCLK->STATUS.bit.SYNCBUSY); // Think about why this is used

}

void loop() {

// Do nothing

}

```

  

#### 2. Set the Blue LED at the beginning of the program.

  

```arduino

  

SerialUSB.begin(9600); // Set the baud rate to 9600

pinMode(LED_BUILTIN, OUTPUT);

digitalWrite(LED_BUILTIN, HIGH); // Set the blue LED HIGH at the beginning

```

  

#### 3. Clock (clock generator 2) frequency: 2048 Hz

  

```arduino

GCLK->GENDIV.reg = GCLK_GENDIV_ID(2) | GCLK_GENDIV_DIV(3);

```

  

#### 4. Set the WDT period to 2 seconds.

  

```arduino

// set the timer to 2 sec

WDT->CONFIG.bit.PER = 0x09; // Set the timeout period to 2 seconds

```

  

#### 5. Observe the behavior of the blue LED.

  

#### Output Screenshot

A sized image: ![Alt](question1_output.png)

  

### Task 2 :

  

  

### Case 1

  

  

#### 1. Set the Blue LED at the beginning of the program.

  

```arduino

pinMode(LED_BUILTIN, OUTPUT);

digitalWrite(LED_BUILTIN, HIGH); // Set the blue LED HIGH at the beginning

```

  

#### 2. Clock (clock generator 2) frequency: 2048 Hz

  

```arduino

GCLK->GENDIV.reg = GCLK_GENDIV_ID(2) | GCLK_GENDIV_DIV(3);

```

  

#### 3. Set the loop period to 1 second, loop 10 times in the main loop () function such that it repeats.

  

```arduino

for (int loopCount = 9; loopCount > 0; loopCount--) {

//print the countdown

SerialUSB.print("Countdown ");

SerialUSB.println(loopCount);

WDT->CLEAR.reg = WDT_CLEAR_CLEAR_KEY;

delay(1000); // setting the loop period to 1 second

}

```

  

  

#### 4. Clear(kick) the WDT in the loop

  

```arduino

WDT->CLEAR.reg = WDT_CLEAR_CLEAR_KEY;

```

  

  

#### 5. Count down the number of loops and print a countdown message.

  

```arduino

for (int loopCount = 9; loopCount > 0; loopCount--) {

SerialUSB.print("Countdown ");

SerialUSB.println(loopCount);

// internal code to print and control WDT

}

```

  

  

#### 6. Set the period using the delay(ms) function.

  

```arduino

delay(1000); // setting the loop period to 1 second

```

  

  

#### 7. Set the WDT period to 4 seconds.

  

```arduino

WDT->CONFIG.bit.PER = 0x0A; // Set the timeout period to 4 seconds

```

  

  

### Case 2

  

  

#### Compare case 1 with clearing WDT and case 2 without clearing WDT and record the system behavior from the LED and the serial monitor message.

  

```arduino

  

for (int loopCount = 9; loopCount > 0; loopCount--) {

//print the countdown

SerialUSB.print("Countdown ");

SerialUSB.println(loopCount);

//comment the clear code

//WDT->CLEAR.reg = WDT_CLEAR_CLEAR_KEY;

delay(1000);

}

```

  

  

#### Discuss and explain the differences between the two cases.

  

<p>The visual difference can be seen by carefully following the LED, in the first case the LED will be static and will keep on being in the ON state as the Watchdog timer is getting kicked before it can reset. </p>

  

<p>For the Second case, as we comment out the code that kicks the Watchdog therefore it resets the MCU. As a result we will see that the LED starts blinking after every 4 sec, because the watchdog timer is set to a window of 4 sec. </p>

  

  

### Task 3 :

  

  

#### 1. Write a function that generates a WDT period by arbitrary input

  

  

```arduino

int setWatchdog(int period)

{

// Convert the period from milliseconds to clock cycles

int clock_frequency = 2048;

int desired_cycles = period * clock_frequency / 1000;

  

// Define an array of available WDT periods in clock cycles

int available_cycles[] = {8, 16, 32, 64, 128, 256, 512, 1024, 2048, 4096, 8192, 16384};

int register_values_per_cycles[] = {0x0, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0X0B};

// Initialize register_value to the smallest available period

int register_value = 0x0;

  

// Loop through the array to find the closest available period that is less than or equal to the desired period

for (int i = 0; i < sizeof(available_cycles) / sizeof(available_cycles[0]); ++i)

{

// find the closest cycle based on the floor calculation

if (available_cycles[i] <= desired_cycles) {

// register_value = register_values_per_cycles[i]; // Register values are indexed from 0x0

register_value = i;

} else {

break; // Exit the loop as soon as we find a period greater than the desired period

}

}

  

// register_value = 0x0A;

  

return register_value;

}

```

  

  

#### 2. Clock (clock generator 2) frequency: 2048 Hz

  

  

```arduino

GCLK->GENDIV.reg = GCLK_GENDIV_ID(2) | GCLK_GENDIV_DIV(3);

```

  

  

#### 3. Function input: period (millisecond)

  

```arduino

// set the timer to n milisecond

WDT->CONFIG.bit.PER = setWatchdog(5000);

```

  

#### 4a. Calculate the register value based on the period

  

```arduino

// Convert the period from milliseconds to clock cycles

int clock_frequency = 2048;

int desired_cycles = period * clock_frequency / 1000;

```

  

#### 4b. Take the floor to the closest value for values that cannot be mapped to register value.

  

  

```arduino

// Loop through the array to find the closest available period that is less than or equal to the desired period

for (int i = 0; i < sizeof(available_cycles) / sizeof(available_cycles[0]); ++i)

{

// find the closest cycle based on the floor calculation

if (available_cycles[i] <= desired_cycles) {

// register_value = register_values_per_cycles[i]; // Register values are indexed from 0x0

register_value = i;

} else {

break; // Exit the loop as soon as we find a period greater than the desired period

}

}

  

```

  

  

### Task 4 :

  

#### After a reset event is there a way for the MCU to figure out if the last reset was due to WDT? If yes,

  

#### Write code that detects if the last reset was due to WDT:

```arduino

// Check if the last reset was due to the Watchdog Timer

  

if (PM->RCAUSE.reg & (1 << 5)) { // WDT is the 5th bit in RCAUSE

SerialUSB.println("Last reset was due to the Watchdog Timer.");

}

else {

SerialUSB.println("Last reset was NOT due to the Watchdog Timer.");

}

```

#### If it was due to WDT, print a message in the console

```

SerialUSB.println("Last reset was due to the Watchdog Timer.");

```

#### What could be the importance of checking if the last reset was due to WDT?

  

#### If no, Explain why it is not possible.