#define CPU_HZ 48000000
#define TIMER_PRESCALER_DIV 1024
#include <TemperatureZero.h>

TemperatureZero TempZero = TemperatureZero();
volatile bool canToggleLED = false;
volatile bool canReadTemp = false;

void setup() {
  SerialUSB.begin(9600);
  TempZero.init();
  pinMode(PIN_LED_13, OUTPUT);
  startTimer(1);
}

void loop() {
  Readtemp();
}

void startTimer(int frequencyHz) {
  configureClock();
  configureTimer(frequencyHz);
}

void configureClock() {
  REG_GCLK_CLKCTRL = (uint16_t)(GCLK_CLKCTRL_CLKEN | GCLK_CLKCTRL_GEN_GCLK0 | GCLK_CLKCTRL_ID_TCC2_TC3);
  while (GCLK->STATUS.bit.SYNCBUSY == 1);
}

void configureTimer(int frequencyHz) {
  TcCount16* TC = (TcCount16*)TC3;
  TC->CTRLA.reg &= ~TC_CTRLA_ENABLE;
  while (TC->STATUS.bit.SYNCBUSY == 1);

  TC->CTRLA.reg |= TC_CTRLA_MODE_COUNT16 | TC_CTRLA_WAVEGEN_MFRQ | TC_CTRLA_PRESCALER_DIV1024;
  while (TC->STATUS.bit.SYNCBUSY == 1);

  setTimerFrequency(TC, frequencyHz);
  TC->INTENSET.reg = 0;
  TC->INTENSET.bit.MC0 = 1;

  NVIC_EnableIRQ(TC3_IRQn);
  TC->CTRLA.reg |= TC_CTRLA_ENABLE;
  while (TC->STATUS.bit.SYNCBUSY == 1);
}

void setTimerFrequency(TcCount16* TC, int frequencyHz) {
  int compareValue = (CPU_HZ / (TIMER_PRESCALER_DIV * frequencyHz)) - 1;
  TC->CC[0].reg = compareValue;
  while (TC->STATUS.bit.SYNCBUSY == 1);
}

void TC3_Handler() {
  static bool isLEDOn = false;
  TcCount16* TC = (TcCount16*)TC3;
  if (TC->INTFLAG.bit.MC0 == 1) {
    TC->INTFLAG.bit.MC0 = 1;
    digitalWrite(PIN_LED_13, isLEDOn);
    SerialUSB.println(isLEDOn ? "Blue LED is on" : "Blue LED is off");
    isLEDOn = !isLEDOn;
    canReadTemp = true;
  }
}

void Readtemp() {
  if (canReadTemp) {
    float temperature = TempZero.readInternalTemperature();
    // SerialUSB.print("Internal Temperature is: ");
    // SerialUSB.println(temperature);
    canReadTemp = false;
  }
}