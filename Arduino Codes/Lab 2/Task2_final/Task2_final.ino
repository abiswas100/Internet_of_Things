void startTimer();
void TC3_Handler();

bool isYellowLEDOn = false;
bool isBlueLEDOn = false;
uint16_t overflowCount = 0;

void setup() {
  SerialUSB.begin(9600);
  pinMode(PIN_LED_13, OUTPUT);  // Blue LED
  pinMode(PIN_LED_RXL, OUTPUT); // Yellow LED

  // Configure Generic Clock Generator 2 with a 1024Hz clock
  GCLK->GENDIV.reg = GCLK_GENDIV_ID(2) | GCLK_GENDIV_DIV(46875); // 48MHz / 46875 = 1024Hz
  while (GCLK->STATUS.bit.SYNCBUSY);
  
  GCLK->GENCTRL.reg = GCLK_GENCTRL_ID(2) | GCLK_GENCTRL_SRC_DFLL48M | GCLK_GENCTRL_GENEN;
  while (GCLK->STATUS.bit.SYNCBUSY);

  startTimer();
}

void loop() {}

void startTimer() {
  // Disable the timer
  TC3->COUNT8.CTRLA.bit.ENABLE = 0;
  while (TC3->COUNT8.STATUS.bit.SYNCBUSY);

  // Configure the clock source to use Generic Clock Generator 2
  REG_GCLK_CLKCTRL = (uint16_t) (GCLK_CLKCTRL_CLKEN | GCLK_CLKCTRL_GEN_GCLK2 | GCLK_CLKCTRL_ID_TCC2_TC3);
  while (GCLK->STATUS.bit.SYNCBUSY);

  // Use the 8-bit timer in Normal Frequency Operation mode with a prescaler of 256
  TC3->COUNT8.CTRLA.reg = TC_CTRLA_MODE_COUNT8 | TC_CTRLA_WAVEGEN_NFRQ | TC_CTRLA_PRESCALER_DIV256;
  while (TC3->COUNT8.STATUS.bit.SYNCBUSY);

  // Enable the overflow interrupt
  TC3->COUNT8.INTENSET.reg = TC_INTENSET_OVF;

  // Enable the IRQ for the timer
  NVIC_EnableIRQ(TC3_IRQn);

  // Enable the timer
  TC3->COUNT8.CTRLA.bit.ENABLE = 1;
  while (TC3->COUNT8.STATUS.bit.SYNCBUSY);
}

void TC3_Handler() {
  // Check for the overflow interrupt
  if (TC3->COUNT8.INTFLAG.bit.OVF) {
    TC3->COUNT8.INTFLAG.bit.OVF = 1;  // Clear the overflow interrupt flag

    overflowCount++;

    // Toggle Yellow LED every 2 overflows (approx. 2 Hz)
    if (overflowCount % 16 == 0) {
      digitalWrite(PIN_LED_RXL, isYellowLEDOn);
      SerialUSB.println(isYellowLEDOn ? "Yellow LED is on" : "Yellow LED is off");
      isYellowLEDOn = !isYellowLEDOn;
    }

    // Toggle Blue LED every 2 overflows (approx. 2 Hz)
    if (overflowCount % 8 == 0) {
      digitalWrite(PIN_LED_13, isBlueLEDOn);
      SerialUSB.println(isBlueLEDOn ? "Blue LED is on" : "Blue LED is off");
      isBlueLEDOn = !isBlueLEDOn;
    }
  }
}
