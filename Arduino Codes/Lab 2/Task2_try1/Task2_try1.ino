#define CPU_HZ 48000000
#define TIMER_PRESCALER_DIV 1024
void startTimer(int frequencyHz);
void setTimerFrequency(int frequencyHz);

void TC3_Handler();
void TC4_Handler();


bool isBlueLEDOn = false;  // State of Blue LED
bool isYellowLEDOn = false;  // State of Yellow LED


void startTimer(TcCount16* TC, int frequencyHz, uint16_t clkctrl) {
  // Configure the Generic Clock (GCLK) to provide a clock source
  REG_GCLK_CLKCTRL = clkctrl;
  while (GCLK->STATUS.bit.SYNCBUSY);

  // Disable, set to 16-bit mode, use match frequency mode, and set prescaler
  TC->CTRLA.reg = TC_CTRLA_MODE_COUNT16 | TC_CTRLA_WAVEGEN_MFRQ | TC_CTRLA_PRESCALER_DIV1024;
  while (TC->STATUS.bit.SYNCBUSY);

  // Set frequency
  int compareValue = (CPU_HZ / (TIMER_PRESCALER_DIV * frequencyHz)) - 1;
  TC->COUNT.reg = 0;
  TC->CC[0].reg = compareValue;
  while (TC->STATUS.bit.SYNCBUSY);

  // Enable overflow interrupt
  TC->INTENSET.reg = TC_INTENSET_OVF;
  
  // Enable TC
  TC->CTRLA.reg |= TC_CTRLA_ENABLE;
  while (TC->STATUS.bit.SYNCBUSY);
}

void setup() {
  SerialUSB.begin(9600);
  pinMode(PIN_LED_13, OUTPUT);  // Blue LED
  pinMode(PIN_LED_RXL, OUTPUT); // Yellow LED

  // Start TC3 and TC4
  startTimer((TcCount16*) TC3, 2, GCLK_CLKCTRL_CLKEN | GCLK_CLKCTRL_GEN_GCLK0 | GCLK_CLKCTRL_ID_TCC2_TC3);
  startTimer((TcCount16*) TC4, 1, GCLK_CLKCTRL_CLKEN | GCLK_CLKCTRL_GEN_GCLK0 | GCLK_CLKCTRL_ID_TC4_TC5);

  // Enable interrupts
  NVIC_EnableIRQ(TC3_IRQn);
  NVIC_EnableIRQ(TC4_IRQn);
}

void loop() {
  // Empty loop
}

// Interrupt Service Routine (ISR) for TC3
void TC3_Handler() {
  
  TcCount16* TC = (TcCount16*) TC3;
  if (TC->INTFLAG.bit.OVF) {
    TC->INTFLAG.bit.OVF = 1;
    // Toggle Blue LED
    isBlueLEDOn = !isBlueLEDOn;
    digitalWrite(PIN_LED_13, isBlueLEDOn ? HIGH : LOW);
    SerialUSB.println(isBlueLEDOn ? "Blue LED is on" : "Blue LED is off");
  }
}

// Interrupt Service Routine (ISR) for TC4
void TC4_Handler() {
  TcCount16* TC = (TcCount16*) TC4;
  if (TC->INTFLAG.bit.OVF) {
    TC->INTFLAG.bit.OVF = 1;
    // Toggle Yellow LED
    isYellowLEDOn = !isYellowLEDOn;
    digitalWrite(PIN_LED_RXL, isYellowLEDOn ? HIGH : LOW);
    SerialUSB.println(isYellowLEDOn ? "Yellow LED is on" : "Yellow LED is off");
  }
}
