// Constants for clock and timer
#define CPU_HZ 48000000  // 48MHz
#define TIMER_PRESCALER_DIV 1024

void setupTC3();
void TC3_Handler();

bool isYellowLEDOn = false;
bool isBlueLEDOn = false;

void setup() {
  Serial.begin(9600);
  pinMode(12, OUTPUT);  // Yellow LED on pin 12
  pinMode(13, OUTPUT);  // Blue LED on pin 13
  setupTC3();           // Setup Timer Counter 3
}

void loop() {
  // Your continuous code here
}

void setupTC3() {
  // Configure clock and timer
  REG_GCLK_CLKCTRL = (uint16_t) (GCLK_CLKCTRL_CLKEN | GCLK_CLKCTRL_GEN_GCLK0 | GCLK_CLKCTRL_ID_TCC2_TC3);
  while (GCLK->STATUS.bit.SYNCBUSY);  // Wait for synchronization

  TcCount16* TC = (TcCount16*) TC3;
  
  TC->CTRLA.reg &= ~TC_CTRLA_ENABLE;  // Disable TC3
  while (TC->STATUS.bit.SYNCBUSY); // Wait for synchronization

  TC->CTRLA.reg = TC_CTRLA_MODE_COUNT16;  // Set TC3 to COUNT16 mode
  
  // Configure match frequency operation mode
  TC->CTRLA.reg |= TC_CTRLA_WAVEGEN_MFRQ; 
  while (TC->STATUS.bit.SYNCBUSY); // Wait for synchronization

  // Configure prescaler
  TC->CTRLA.reg |= TC_CTRLA_PRESCALER_DIV1024; 
  while (TC->STATUS.bit.SYNCBUSY); // Wait for synchronization

  // Calculate and set the compare value for the timer
  int compareValue = (CPU_HZ / (TIMER_PRESCALER_DIV * 1)) - 1;  // 1Hz for Yellow LED
  TC->CC[0].reg = compareValue;
  while (TC->STATUS.bit.SYNCBUSY); // Wait for synchronization

  // Enable overflow interrupt
  TC->INTENSET.reg = TC_INTENSET_MC0;  
  NVIC_EnableIRQ(TC3_IRQn); // Enable TC3 interrupts
  
  TC->CTRLA.reg |= TC_CTRLA_ENABLE;  // Enable TC3
  while (TC->STATUS.bit.SYNCBUSY);  // Wait for synchronization
}

void TC3_Handler() {
  TcCount16* TC = (TcCount16*) TC3;
  
  static uint8_t counter = 0;
  counter++;

  // Toggle Yellow LED every 2nd interrupt
  if (counter % 2 == 0) {
    digitalWrite(12, isYellowLEDOn);
    Serial.println(isYellowLEDOn ? "Yellow LED is on" : "Yellow LED is off");
    isYellowLEDOn = !isYellowLEDOn;
  }
  
  // Toggle Blue LED every interrupt
  digitalWrite(13, isBlueLEDOn);
  Serial.println(isBlueLEDOn ? "Blue LED is on" : "Blue LED is off");
  isBlueLEDOn = !isBlueLEDOn;

  TC->INTFLAG.bit.MC0 = 1;  // Clear the MC0 interrupt flag
}