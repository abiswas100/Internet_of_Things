#define YELLOW_LED_PIN 12  // Use appropriate pins
#define BLUE_LED_PIN 13
bool isYellowLEDOn = false;
bool isBlueLEDOn = false;

void setup() {
    SerialUSB.begin(9600);
    pinMode(YELLOW_LED_PIN, OUTPUT);
    pinMode(BLUE_LED_PIN, OUTPUT);

    // Configure Generic Clock Generator 2 with 1024Hz (assuming you've done it in Lab 1)
    // ... Insert that setup code here ...

    // Enable the clock for TC3
    REG_GCLK_CLKCTRL = (uint16_t) (GCLK_CLKCTRL_CLKEN | GCLK_CLKCTRL_GEN_GCLK2 | GCLK_CLKCTRL_ID_TCC2_TC3);
    while (GCLK->STATUS.bit.SYNCBUSY == 1);

    TcCount8* TC = (TcCount8*) TC3;

    TC->CTRLA.reg &= ~TC_CTRLA_ENABLE;
    while (TC->STATUS.bit.SYNCBUSY == 1);

    TC->CTRLA.reg = TC_CTRLA_MODE_COUNT8;
    while (TC->STATUS.bit.SYNCBUSY == 1);

    // Set prescaler to 4
    TC->CTRLA.reg |= TC_CTRLA_PRESCALER_DIV4;
    while (TC->STATUS.bit.SYNCBUSY == 1);

    // Enable Overflow Interrupt
    TC->INTENSET.reg = TC_INTENSET_OVF;
    NVIC_EnableIRQ(TC3_IRQn);

    TC->CTRLA.reg |= TC_CTRLA_ENABLE;
    while (TC->STATUS.bit.SYNCBUSY == 1);
}
void TC3_Handler() {
    TcCount8* TC = (TcCount8*) TC3;

    // Handle the overflow
    if (TC->INTFLAG.bit.OVF == 1) {
        TC->INTFLAG.bit.OVF = 1;  // Clear the interrupt flag

        // Toggle yellow LED every 2 overflows (approximately 1 second)
        static uint8_t yellow_counter = 0;
        yellow_counter++;
        if (yellow_counter >= 2) {
            isYellowLEDOn = !isYellowLEDOn;
            digitalWrite(YELLOW_LED_PIN, isYellowLEDOn);
            SerialUSB.println(isYellowLEDOn ? "Yellow LED is on" : "Yellow LED is off");
            yellow_counter = 0;
        }

        // Toggle blue LED every overflow (approximately 0.5 seconds)
        isBlueLEDOn = !isBlueLEDOn;
        digitalWrite(BLUE_LED_PIN, isBlueLEDOn);
        SerialUSB.println(isBlueLEDOn ? "Blue LED is on" : "Blue LED is off");
    }
}

void loop() {
    // Nothing here
}