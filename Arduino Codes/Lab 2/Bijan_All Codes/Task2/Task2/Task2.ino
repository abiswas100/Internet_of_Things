#define CPU_HZ 48000000
#define TIMER_PRESCALER_DIV 1024
#define YELLOW_LED_PIN 12  // Assume PIN 12 for Yellow LED
#define BLUE_LED_PIN 13    // Assume PIN 13 for Blue LED
void startTimer(Tc* TC, int frequencyHz, IRQn_Type irq);
void TC3_Handler();
void TC4_Handler();
bool isYellowLEDOn = false;
bool isBlueLEDOn = false;

void setup() {
  SerialUSB.begin(9600);
  pinMode(YELLOW_LED_PIN, OUTPUT);
  pinMode(BLUE_LED_PIN, OUTPUT);  
  startTimer(TC3, 1, TC3_IRQn);  // for Yellow LED
  startTimer(TC4, 2, TC4_IRQn);  // for Blue LED
}
void loop() { }
void startTimer(Tc* TC, int frequencyHz, IRQn_Type irq) {
  uint16_t GCLK_ID = (irq == TC3_IRQn) ? GCLK_CLKCTRL_ID_TCC2_TC3 : GCLK_CLKCTRL_ID_TC4_TC5;
  REG_GCLK_CLKCTRL = (uint16_t) (GCLK_CLKCTRL_CLKEN | GCLK_CLKCTRL_GEN_GCLK0 | GCLK_ID);
  while (GCLK->STATUS.bit.SYNCBUSY == 1);

  TC->COUNT16.CTRLA.reg &= ~TC_CTRLA_ENABLE;
  while (TC->COUNT16.STATUS.bit.SYNCBUSY == 1);
  
  TC->COUNT16.CTRLA.reg |= TC_CTRLA_MODE_COUNT16 | TC_CTRLA_WAVEGEN_MFRQ | TC_CTRLA_PRESCALER_DIV1024;
  while (TC->COUNT16.STATUS.bit.SYNCBUSY == 1);

  int compareValue = (CPU_HZ / (TIMER_PRESCALER_DIV * frequencyHz)) - 1;
  TC->COUNT16.CC[0].reg = compareValue;
  while (TC->COUNT16.STATUS.bit.SYNCBUSY == 1);

  TC->COUNT16.INTENSET.reg = TC_INTENSET_OVF;  // Enable overflow interrupt
  NVIC_EnableIRQ(irq);

  TC->COUNT16.CTRLA.reg |= TC_CTRLA_ENABLE;
  while (TC->COUNT16.STATUS.bit.SYNCBUSY == 1);
}
void TC3_Handler() {
  Tc* TC = TC3;
  if (TC->COUNT16.INTFLAG.bit.OVF == 1) {
    TC->COUNT16.INTFLAG.bit.OVF = 1;  // Clear the OVF interrupt flag
    isYellowLEDOn = !isYellowLEDOn;
    digitalWrite(YELLOW_LED_PIN, isYellowLEDOn);
    SerialUSB.println(isYellowLEDOn ? "Yellow LED is on" : "Yellow LED is off");
  }
}

void TC4_Handler() {
  Tc* TC = TC4;
  if (TC->COUNT16.INTFLAG.bit.OVF == 1) {
    TC->COUNT16.INTFLAG.bit.OVF = 1;  // Clear the OVF interrupt flag
    isBlueLEDOn = !isBlueLEDOn;
    digitalWrite(BLUE_LED_PIN, isBlueLEDOn);
    SerialUSB.println(isBlueLEDOn ? "Blue LED is on" : "Blue LED is off");
  }
}
