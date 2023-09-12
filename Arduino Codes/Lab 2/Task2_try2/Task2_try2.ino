// LED definitions in the datasheet
// D13 (PIN_LED_13): Blue
// TX (PIN_LED_TXL): Green
// RX (PIN_LED_RXL): Yellow


#define CPU_HZ 48000000
#define TIMER_PRESCALER_DIV 1024

void startTimer(TcCount16* TC, int frequencyHz, uint16_t clockSelection);
void TC3_Handler();
void TC4_Handler();

bool isYellowLEDOn = false;
bool isBlueLEDOn = false;

void setup() {
  SerialUSB.begin(9600);
  pinMode(PIN_LED_13, OUTPUT);  // Blue LED
  pinMode(PIN_LED_RXL, OUTPUT); // Yellow LED

  startTimer((TcCount16*) TC3, 2, GCLK_CLKCTRL_ID_TCC2_TC3);  // 1 Hz for Yellow LED
  startTimer((TcCount16*) TC4, 3, GCLK_CLKCTRL_ID_TC4_TC5);  // 2 Hz for Blue LED
}

void loop() {}

void startTimer(TcCount16* TC, int frequencyHz, uint16_t clockSelection) {
  // Disable the timer
  TC->CTRLA.bit.ENABLE = 0;
  while (TC->STATUS.bit.SYNCBUSY);  // Wait for synchronization
  
  // Calculate the compare value
  int compareValue = (CPU_HZ / (TIMER_PRESCALER_DIV * frequencyHz)) - 1;
  
  // Configure the clock source
  REG_GCLK_CLKCTRL = (uint16_t) (GCLK_CLKCTRL_CLKEN | GCLK_CLKCTRL_GEN_GCLK0 | clockSelection);
  while (GCLK->STATUS.bit.SYNCBUSY);
  
  // Configure the timer
  TC->CTRLA.reg = TC_CTRLA_MODE_COUNT16 | TC_CTRLA_WAVEGEN_MFRQ | TC_CTRLA_PRESCALER_DIV1024;
  while (TC->STATUS.bit.SYNCBUSY);
  
  // Adjust the current count to be in a proportional position to the new compare value
  TC->COUNT.reg = map(TC->COUNT.reg, 0, TC->CC[0].reg, 0, compareValue);
  
  // Set the compare register
  TC->CC[0].reg = compareValue;
  while (TC->STATUS.bit.SYNCBUSY);
  
  // Enable the compare interrupt
  TC->INTENSET.reg = TC_INTENSET_MC0;  // Use match interrupt
  
  // Enable the IRQ for the timer
  if (TC == (TcCount16*) TC3) {
    NVIC_EnableIRQ(TC3_IRQn);
  } else if (TC == (TcCount16*) TC4) {
    NVIC_EnableIRQ(TC4_IRQn);
  }
  
  // Enable the timer
  TC->CTRLA.bit.ENABLE = 1;
  while (TC->STATUS.bit.SYNCBUSY);
}



void TC3_Handler() {
  TcCount16* TC = (TcCount16*) TC3;
  if (TC->INTFLAG.bit.MC0) {
    TC->INTFLAG.bit.MC0 = 1;
    digitalWrite(PIN_LED_RXL, isYellowLEDOn);
    SerialUSB.println(isYellowLEDOn ? "Yellow LED is on" : "Yellow LED is off");
    isYellowLEDOn = !isYellowLEDOn;
  }
}

void TC4_Handler() {
  TcCount16* TC = (TcCount16*) TC4;
  if (TC->INTFLAG.bit.MC0) {
    TC->INTFLAG.bit.MC0 = 1;
    digitalWrite(PIN_LED_13, isBlueLEDOn);
    SerialUSB.println(isBlueLEDOn ? "Blue LED is on" : "Blue LED is off");
    isBlueLEDOn = !isBlueLEDOn;
  }
}
