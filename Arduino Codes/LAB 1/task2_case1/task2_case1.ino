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

  // set the timer to 2 sec
  WDT->CONFIG.bit.PER = 0x0A; // Set the timeout period to 4 seconds
  SerialUSB.println("enabling WDT");
  // Enable the WDT
  WDT->CTRL.bit.ENABLE = 1; //  ENABLE WDT
  WDT->INTENCLR.bit.EW = 1; // Disable early warning interrupt
  WDT->CTRL.bit.WEN = 0; // Disable window mode


}

void loop() {
  
  SerialUSB.println(" Case 1 Starting loop ");
  // Loop ten times
  for (int loopCount = 9; loopCount > 0; loopCount--) {
    //print the countdown
    SerialUSB.print("Countdown ");
    SerialUSB.println(loopCount);
    WDT->CLEAR.reg = WDT_CLEAR_CLEAR_KEY;
    delay(1000);
  }
}
