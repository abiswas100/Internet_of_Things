// LED definitions in the datasheet
// D13 (PIN_LED_13): Blue
// TX (PIN_LED_TXL): Green
// RX (PIN_LED_RXL): Yellow

// LED definitions in the datasheet
// D13 (PIN_LED_13): Blue
// TX (PIN_LED_TXL): Green
// RX (PIN_LED_RXL): Yellow

#define CPU_HZ 48000000
#define TIMER_PRESCALER_DIV 1024
void startTimer(int frequencyHz);
void setTimerFrequency(int frequencyHz);
void TC3_Handler();
bool isBlueLEDOn = false;
bool isYellowLEDOn = false;

unsigned long blueLEDTime = 0;
unsigned long yellowLEDTime = 0;
unsigned long currentTime = 0;


void setup() 
{
SerialUSB.begin(9600);
// while(!SerialUSB);
pinMode(PIN_LED_13, OUTPUT);
startTimer(20);
}


void loop() {}

void setTimerFrequency(int frequencyHz) {

int compareValue = (CPU_HZ / (TIMER_PRESCALER_DIV * frequencyHz)) - 1;
TcCount16* TC = (TcCount16*) TC3;
// Make sure the count is in a proportional position to where it was
// to prevent any jitter or disconnect when changing the compare value.
TC->COUNT.reg = map(TC->COUNT.reg, 0, TC->CC[0].reg, 0, compareValue);
TC->CC[0].reg = compareValue;
SerialUSB.println(TC->COUNT.reg);
SerialUSB.println(TC->CC[0].reg);
while (TC->STATUS.bit.SYNCBUSY == 1);
}


void startTimer(int frequencyHz) 
{
  REG_GCLK_CLKCTRL = (uint16_t) (GCLK_CLKCTRL_CLKEN | 
                              GCLK_CLKCTRL_GEN_GCLK0 |
                              GCLK_CLKCTRL_ID_TCC2_TC3) ; //set the clock controller, set clock 0 to TC2 and TC3

  while ( GCLK->STATUS.bit.SYNCBUSY == 1 ); // wait for sync
  
  TcCount16* TC = (TcCount16*) TC3;  // efficient way to do things, cast TC3 to TC.

  // to configure a enable protected register, like WDT we need to disable it first.
  TC->CTRLA.reg &= ~TC_CTRLA_ENABLE; //Disable timer
  while (TC->STATUS.bit.SYNCBUSY == 1); // wait for sync
  
  // Use the 16-bit timer
  TC->CTRLA.reg |= TC_CTRLA_MODE_COUNT16;
  while (TC->STATUS.bit.SYNCBUSY == 1); // wait for sync
  
  // Use match mode so that the timer counter resets when the count matches the compare register
  TC->CTRLA.reg |= TC_CTRLA_WAVEGEN_MFRQ;
  while (TC->STATUS.bit.SYNCBUSY == 1); // wait for sync
  
  // Set prescaler to 1024
  TC->CTRLA.reg |= TC_CTRLA_PRESCALER_DIV1024;
  while (TC->STATUS.bit.SYNCBUSY == 1); // wait for sync
  
  setTimerFrequency(frequencyHz);
  
  // Enable the compare interrupt at high it is 0,
  TC->INTENSET.reg = 0;
  TC->INTENSET.bit.MC0 = 1;

  NVIC_EnableIRQ(TC3_IRQn);

  // Enable the control register of the TC
  TC->CTRLA.reg |= TC_CTRLA_ENABLE;
  
  while (TC->STATUS.bit.SYNCBUSY == 1); // wait for sync
}


// this is the interrupt handler

void TC3_Handler() {
  TcCount16* TC = (TcCount16*) TC3;
  if (TC->INTFLAG.bit.MC0 == 1) {
    TC->INTFLAG.bit.MC0 = 1;
    
    currentTime += 100; // 100 ms has passed
    
    // Handle Blue LED
    if (currentTime - blueLEDTime >= 750) { // 0.5 seconds
      isBlueLEDOn = !isBlueLEDOn;
      digitalWrite(PIN_LED_13, isBlueLEDOn);
      blueLEDTime = currentTime;
      SerialUSB.println(isBlueLEDOn ? "Blue LED is on" : "Blue LED is off");
    }
    
    // Handle Yellow LED
    if (currentTime - yellowLEDTime >= 1000) { // 1 second
      isYellowLEDOn = !isYellowLEDOn;
      digitalWrite(PIN_LED_RXL, isYellowLEDOn);
      yellowLEDTime = currentTime;
      SerialUSB.println(isYellowLEDOn ? "Yellow LED is on" : "Yellow LED is off");
    }
  }
}