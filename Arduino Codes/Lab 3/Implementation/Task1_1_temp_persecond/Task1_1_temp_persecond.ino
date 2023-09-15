#define CPU_HZ 48000000
#define TIMER_PRESCALER_DIV 1024
#define WINDOW_SIZE 5  // Size of the sliding window

#include <TemperatureZero.h>

TemperatureZero TempZero = TemperatureZero();
volatile bool canReadTemp = false;
volatile bool canToggleLED = false;


void setup() {
  SerialUSB.begin(9600);
  TempZero.init();
  pinMode(PIN_LED_13, OUTPUT);
  startTimer(1);
}

void loop() {
  Readtemp_and_toggleLED();
}

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

void Readtemp_and_toggleLED() {
  if (canToggleLED) {
    float temperature = TempZero.readInternalTemperature();
    static bool isLEDOn = false;

    SerialUSB.print("Internal Temperature is: ");
    SerialUSB.println(temperature);
    digitalWrite(PIN_LED_13, isLEDOn);
    // SerialUSB.println(isLEDOn ? "Blue LED is on" : "Blue LED is off");
    isLEDOn = !isLEDOn;
    canToggleLED = false;
  }
}