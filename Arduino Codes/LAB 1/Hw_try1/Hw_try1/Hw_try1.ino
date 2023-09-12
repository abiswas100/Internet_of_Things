int setWatchdog(int period)
{
  // Convert the period from milliseconds to clock cycles
  int clock_frequency = 2048;
  int desired_cycles = period * clock_frequency / 1000;
  
  // Define an array of available WDT periods in clock cycles
  int available_cycles[] = {8,     16,   32,   64,  128,  256,  512, 1024, 2048, 4096, 8192, 16384};
  int register_values_per_cycles[] = {0x0, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0X0B};
  // Initialize register_value to the smallest available period
  int register_value = 0x0;

  // Loop through the array to find the closest available period that is less than or equal to the desired period
  for (int i = 0; i < sizeof(available_cycles) / sizeof(available_cycles[0]); ++i) {
    // find the closest cycle based on the floor calculation 
    if (available_cycles[i] <= desired_cycles) { 
      // register_value = register_values_per_cycles[i];  // Register values are indexed from 0x0
      register_value = i;
    } else {
      break;  // Exit the loop as soon as we find a period greater than the desired period
    }
  }  
  // register_value = 0x0A;
  return register_value;
}


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

  while (WDT->STATUS.bit.SYNCBUSY); // Wait for synchronization

  // set the timer to n milisec
  WDT->CONFIG.bit.PER = setWatchdog(5000); 
  
  // Enable the WDT
  WDT->CTRL.bit.ENABLE = 1; //  ENABLE WDT

  while(GCLK->STATUS.bit.SYNCBUSY); // Think about why this is used

}

void loop() {
    // Do nothing
}
