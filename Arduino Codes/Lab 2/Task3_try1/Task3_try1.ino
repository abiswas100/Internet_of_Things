


// // LED definitions in the datasheet
// // D13 (PIN_LED_13): Blue
// // TX (PIN_LED_TXL): Green
// // RX (PIN_LED_RXL): Yellow

// void startTimer();
// void TC3_Handler();

// bool isYellowLEDOn = false;
// bool isBlueLEDOn = false;
// uint16_t overflowCount = 0;

// void setup() {
//   SerialUSB.begin(9600);
//   pinMode(PIN_LED_13, OUTPUT);  // Blue LED
//   pinMode(PIN_LED_RXL, OUTPUT); // Yellow LED

//   startTimer();
// }

// void loop() {}

// void startTimer() {
//   // Disable the timer
//   TC3->COUNT8.CTRLA.bit.ENABLE = 0;
//   while (TC3->COUNT8.STATUS.bit.SYNCBUSY);

//   // Configure the clock source
//   REG_GCLK_CLKCTRL = (uint16_t) (GCLK_CLKCTRL_CLKEN | GCLK_CLKCTRL_GEN_GCLK2 | GCLK_CLKCTRL_ID_TCC2_TC3);
//   while (GCLK->STATUS.bit.SYNCBUSY);

//   // Use the 8-bit timer in Normal Frequency Operation mode
//   TC3->COUNT8.CTRLA.reg = TC_CTRLA_MODE_COUNT8 | TC_CTRLA_WAVEGEN_NFRQ | TC_CTRLA_PRESCALER_DIV1024;
//   while (TC3->COUNT8.STATUS.bit.SYNCBUSY);

//   // Enable the overflow interrupt
//   TC3->COUNT8.INTENSET.reg = TC_INTENSET_OVF;

//   // Enable the IRQ for the timer
//   NVIC_EnableIRQ(TC3_IRQn);

//   // Enable the timer
//   TC3->COUNT8.CTRLA.bit.ENABLE = 1;
//   while (TC3->COUNT8.STATUS.bit.SYNCBUSY);
// }

// void TC3_Handler() {
//   // Check for the overflow interrupt
//   if (TC3->COUNT8.INTFLAG.bit.OVF) {
//     TC3->COUNT8.INTFLAG.bit.OVF = 1;  // Clear the overflow interrupt flag

//     overflowCount++;

//     // Toggle Yellow LED every 16 overflows (approx. 1 second)
//     if (overflowCount % 1 == 0) {
//       digitalWrite(PIN_LED_RXL, isYellowLEDOn);
//       // SerialUSB.println(isYellowLEDOn ? "Yellow LED is on" : "Yellow LED is off");
//       isYellowLEDOn = !isYellowLEDOn;
//     }

//     // Toggle Blue LED every 8 overflows (approx. 0.5 seconds)
//     if (overflowCount % 1 == 0) {
//       digitalWrite(PIN_LED_13, isBlueLEDOn);
//       // SerialUSB.println(isBlueLEDOn ? "Blue LED is on" : "Blue LED is off");
//       isBlueLEDOn = !isBlueLEDOn;
//     }
//   }
// }
