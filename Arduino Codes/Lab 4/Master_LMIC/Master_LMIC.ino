#define CPU_HZ 48000000
#define TIMER_PRESCALER_DIV 1024
#define WINDOW_SIZE 5  // Size of the sliding window   

// #include <sstream>
// #include <string>
#include <lmic.h>
#include <hal/hal.h>
#include <SPI.h> 
#include <FlashStorage.h>   
#include <TemperatureZero.h>

FlashStorage(error_Amlan, int);                 // Reserve a portion of flash memory, remove/comment when upload to the transmitter 
FlashStorage(error_Shaswati, int);                                                   
FlashStorage(error_Anuruddha, int);
FlashStorage(error_Avhishek, int);
FlashStorage(error_receive, int);

struct Packet {
  uint8_t nodeID;
  uint16_t packetID;
  uint32_t timestamp;
  float payload;
  char error[16];
  uint8_t authID;

  // Constructor to initialize the values
  Packet() : nodeID(0), packetID(0), timestamp(0), payload(0.0), authID(0) {
    // Initialize the error array to zeros
    memset(error, '0', sizeof(error));
}
};


char* PacketToCString(const Packet& packet) {
    // Allocate memory for the C-string. Size depends on your specific needs.
    char* cstr = new char[128];
    
    sprintf(cstr, "nodeID: %u, packetID: %u, timestamp: %u, payload: %f, error: %s, authID: %u",
            packet.nodeID, packet.packetID, packet.timestamp, packet.payload, packet.error, packet.authID);
    
    return cstr;
}

// returns packet from Cstring
Packet CStringToPacket(const char* cstr) {
    Packet packet;
    
    // Use sscanf to parse the string and populate the Packet fields
    int ret = sscanf(cstr, "nodeID: %hhu, packetID: %hu, timestamp: %u, payload: %f, error: %15s, authID: %hhu",
                     &packet.nodeID, &packet.packetID, &packet.timestamp, &packet.payload, packet.error, &packet.authID);    
    // Optionally, you can check if all fields were successfully parsed
    if (ret != 6) {
        // Handle the error, e.g., by setting default values or logging an error message
    }
    return packet;
}



volatile bool canReadTemp = false;

volatile bool Slave2 = false;
volatile bool Slave3 = false;
volatile bool Slave4 = false;
volatile bool Master = false;

float node1_temperature = 0;
float node2_temperature = 0;
float node3_temperature = 0;
float node4_temperature = 0;


float tempBuffer[WINDOW_SIZE];  // Circular buffer to hold temperature values
int tempIndex = 0;  // Index to keep track of the oldest temperature value
float tempSum = 0;  // Sum of temperatures in the buffer
int tempCount = 0;  // Count of temperatures added to the buffer
int reportCount = 0;  // Count of readings before reporting
float avgTemperature = 0;  // To hold the average temperature
bool newAvgAvailable = false;  // Flag to indicate a new average is available

String jsonTemperature = "{";

int previouse_packet_id = 0;
int current_packet_id = 0;

int LED = 13; //Status LED is on pin 13
long timeSinceLastPacket = 0; //Tracks the time stamp of last packet received
uint16_t packetCounter = 0;  // Initialize packet counter


//Define functions used in the program
void startTimer(int frequencyHz); 
void configureClockTC4(); 
void configureTimerTC4(int frequencyHz);                //
void setTimerFrequencyTC4(TcCount16* TC, int frequencyHz);
void TC4_Handler();     
float Readtemp();                           
void write_error(int Node_name, int ecode);
void print_errors(char* err);
 

#define TX_INTERVAL 60000  //Delay between each message in millidecond.

// Pin mapping for SAMD21
const lmic_pinmap lmic_pins = {
    .nss = 12,//RFM Chip Select
    .rxtx = LMIC_UNUSED_PIN,
    .rst = 7,//RFM Reset
    .dio = {6, 10, 11}, //RFM Interrupt, RFM LoRa pin, RFM LoRa pin
};


TemperatureZero TempZero = TemperatureZero();

// These callbacks are only used in over-the-air activation, so they are
// left empty here (we cannot leave them out completely unless
// DISABLE_JOIN is set in arduino-lmoc/project_config/lmic_project_config.h,
// otherwise the linker will complain).
void os_getArtEui (u1_t* buf) { }
void os_getDevEui (u1_t* buf) { }
void os_getDevKey (u1_t* buf) { }

void onEvent (ev_t ev) {
}

osjob_t txjob;
osjob_t timeoutjob;
static void tx_func (osjob_t* job);

// Transmit the given string and call the given function afterwards
void tx(const char *str, osjobcb_t func) {
    os_radio(RADIO_RST); // Stop RX first
    delay(1); // Wait a bit, without this os_radio below asserts, apparently because the state hasn't changed yet
    LMIC.dataLen = 0;
    while (*str)
      LMIC.frame[LMIC.dataLen++] = *str++;
    LMIC.osjob.func = func;
    os_radio(RADIO_TX);
    SerialUSB.println("TX");
    }

    // Enable rx mode and call func when a packet is received
void rx(osjobcb_t func) {
    LMIC.osjob.func = func;
    LMIC.rxtime = os_getTime(); // RX _now_
    // Enable "continuous" RX (e.g. without a timeout, still stops after
    // receiving a packet)
    os_radio(RADIO_RXON);
    SerialUSB.println("RX");
    }

static void rxtimeout_func(osjob_t *job) {
    digitalWrite(LED_BUILTIN, LOW); // off
    }

static void rx_func (osjob_t* job) {
    // Blink once to confirm reception and then keep the led on
    digitalWrite(LED_BUILTIN, LOW); // off
    delay(10);
    digitalWrite(LED_BUILTIN, HIGH); // on

    // Timeout RX (i.e. update led status) after 3 periods without RX
    os_setTimedCallback(&timeoutjob, os_getTime() + ms2osticks(3*TX_INTERVAL), rxtimeout_func);

    // Reschedule TX so that it should not collide with the other side's
    // next TX
    os_setTimedCallback(&txjob, os_getTime() + ms2osticks(TX_INTERVAL/2), tx_func);

    SerialUSB.print("Got ");
    SerialUSB.print(LMIC.dataLen);
    SerialUSB.println(" bytes");
    SerialUSB.write(LMIC.frame, LMIC.dataLen);
    SerialUSB.println();

    // Restart RX
    rx(rx_func);
    }

static void txdone_func (osjob_t* job) {
    delay(1000);
    rx(rx_func);
    }

    // log text to USART and toggle LED
static void tx_func (osjob_t* job) {
    // say hello
    float temperature = TempZero.readInternalTemperature();
    String temperatureString = String(temperature, 2);
    String AvhiTemperature = "Avhi Temp :" + temperatureString;
    // tx("First message going in...!", txdone_func);
    // tx(AvhiTemperature.c_str(), txdone_func);
    // reschedule job every TX_INTERVAL (plus a bit of random to prevent
    // systematic collisions), unless packets are received, then rx_func
    // will reschedule at half this time.
    os_setTimedCallback(job, os_getTime() + ms2osticks(TX_INTERVAL + random(500)), tx_func);
    }

    // application entry point
void setup() 
{
    SerialUSB.begin(115200);
    // while(!SerialUSB);
    SerialUSB.println("Starting");
    GCLK->GENDIV.reg = GCLK_GENDIV_ID(2) | GCLK_GENDIV_DIV(4);
    GCLK->GENCTRL.reg = GCLK_GENCTRL_ID(2) |
    GCLK_GENCTRL_GENEN |
    GCLK_GENCTRL_SRC_OSCULP32K |
    GCLK_GENCTRL_DIVSEL;
    while(GCLK->STATUS.bit.SYNCBUSY);                                // wait while synchronizing the GCLK->STATUS.bit
                                                                      // WDT clock = clock gen 2
    GCLK->CLKCTRL.reg = GCLK_CLKCTRL_ID_WDT |
    GCLK_CLKCTRL_CLKEN |
    GCLK_CLKCTRL_GEN_GCLK2;
    WDT->CTRL.reg = 0;
    WDT->CONFIG.bit.PER = 0x8; // Set period for chip reset from the datasheet
    WDT->INTENSET.bit.EW = 1; // Enable early warning interrupt
    WDT->CTRL.bit.WEN = 0; 
    SerialUSB.begin(115200);
    TempZero.init();


    pinMode(LED_BUILTIN, OUTPUT);

    // initialize runtime env
    os_init();

    // this is automatically set to the proper bandwidth in kHz,
    // based on the selected channel.
    uint32_t uBandwidth;
    //!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
    LMIC.freq = 909900000; //change this for assigned frequencies, match with int freq in loraModem.h
    //!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
    uBandwidth = 125;
    LMIC.datarate = US915_DR_SF7;         // DR4
    LMIC.txpow = 21;

    int previouse_packet_id = 0;
    int current_packet_id = 0;

   
    int LED = 13; //Status LED is on pin 13
    long timeSinceLastPacket = 0; //Tracks the time stamp of last packet received

    float frequency = 910; //Broadcast frequency
    uint16_t packetCounter = 0;  // Initialize packet counter



    // disable RX IQ inversion
    LMIC.noRXIQinversion = true;

    // This sets CR 4/5, BW125 (except for EU/AS923 DR_SF7B, which uses BW250)
    LMIC.rps = updr2rps(LMIC.datarate);

    SerialUSB.print("Frequency: "); SerialUSB.print(LMIC.freq / 1000000);
              SerialUSB.print("."); SerialUSB.print((LMIC.freq / 100000) % 10);
              SerialUSB.print("MHz");
    SerialUSB.print("  LMIC.datarate: "); SerialUSB.print(LMIC.datarate);
    SerialUSB.print("  LMIC.txpow: "); SerialUSB.println(LMIC.txpow);

    // This sets CR 4/5, BW125 (except for DR_SF7B, which uses BW250)
    LMIC.rps = updr2rps(LMIC.datarate);

    // disable RX IQ inversion
    LMIC.noRXIQinversion = true;

    SerialUSB.println("Started");
    SerialUSB.flush();

    // setup initial job
    os_setCallback(&txjob, tx_func);

    WDT->CTRL.bit.ENABLE = 1;
    startTimer(1);

}


void loop() {
    // execute scheduled jobs and events
    os_runloop_once(); 
    avgTemperature = Readtemp();

  // Asking and recieving from Node 2
  if (reportCount == 1 && Slave2 == false)
    {  Master = false;
      //  Slave2 = true;
      // Send the authentication packet
      SerialUSB.println("Asking Node 2 to send it's temp. ");

      // Create the packet
      Packet packet;
      packet.nodeID = 1;  // Node 1
      packet.packetID = packetCounter++;
      packet.timestamp = millis();  // Current time in milliseconds
      packet.payload = avgTemperature;
      // packet.error = 0;
      packet.authID = 2;

      tx(PacketToCString(packet), txdone_func);

      Packet receivedPacket = CStringToPacket((const char*)LMIC.frame);
      timeSinceLastPacket = millis(); //Timestamp this packet

      SerialUSB.println();
      SerialUSB.println();
      
      // Print the received packet details
      SerialUSB.println("Got message from Node ID: ");
      uint8_t nodeid = receivedPacket.nodeID;
      node2_temperature = receivedPacket.payload;
      SerialUSB.print(nodeid);
      if(nodeid == 2){
        SerialUSB.println(node2_temperature);
        }
  }

  // Master is transmitting
  // else if(reportCount == 4 && Master == false)  // 
  // {  // set the slaves false for the next 5 secs
  //     Slave2 = false;
  //     Master = true;

  //     // Send the authentication packet
  //     SerialUSB.println("Sending Master's Average Temperature. ");

  //     //SAVE THE TEMPERATURE OF NODE 2
  //     node1_temperature = avgTemperature;

  //     // Create the packet
  //     Packet packet;
  //     packet.nodeID = 1;  // Node 1
  //     packet.packetID = packetCounter++;
  //     packet.timestamp = millis();  // Current time in milliseconds
  //     packet.payload = avgTemperature;
  //     // packet.error = 0;
  //     packet.authID = 10;  // master is sending it's temperature

  //     // Print and send the message
  //     SerialUSB.print("SENDING TEMPERATURE OF MASTER TO ALL NODES : PACKET NUMBER :");
  //     SerialUSB.println(packet.packetID);
  //     SerialUSB.println();
  //     SerialUSB.println();
      

  //     jsonTemperature += "\"Avhi\": " + String(node1_temperature, 2) + ", ";
  //     jsonTemperature += "\"Amlan\": " + String(node2_temperature, 2) + ", ";
  //     jsonTemperature += "\"Shaswati\": " + String(node3_temperature, 2) + ", ";
  //     jsonTemperature += "\"Anu\": " + String(node4_temperature, 2);
  //     jsonTemperature += "}";

  //     tx(PacketToCString(packet), txdone_func);
  //     // tx(jsonTemperature.c_str(), txdone_func);

  //     // SerialUSB.println(jsonTemperature);
  //     SerialUSB.println();
  //     SerialUSB.println();
  // }      
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
  if (TC->INTFLAG.bit.MC0 == 1) 
  {
    TC->INTFLAG.bit.MC0 = 1;
    WDT->CLEAR.reg = WDT_CLEAR_CLEAR_KEY; 
    canReadTemp = true;
   digitalWrite(PIN_LED_13, isLEDOn);
   SerialUSB.println(isLEDOn ? "Blue LED is on" : "Blue LED is off");
   isLEDOn = !isLEDOn;
   Master = false;
  }
}

float Readtemp() {
  if (canReadTemp) {
    float temperature = TempZero.readInternalTemperature();

    // Remove the oldest temperature from the sum
    tempSum -= tempBuffer[tempIndex];
    
    // Add the new temperature to the buffer and sum
    tempBuffer[tempIndex] = temperature;
    tempSum += temperature;

    // Update the index for the oldest temperature
    tempIndex = (tempIndex + 1) % WINDOW_SIZE;

    // Update the count of temperatures added to the buffer
    if (tempCount < WINDOW_SIZE) {
      tempCount++;
    }

    // Increment the report counter
    reportCount++;

    // If 5 readings have been taken, calculate the average temperature
    if (reportCount >= WINDOW_SIZE) {
      avgTemperature = tempSum / tempCount;
      newAvgAvailable = true;  // Set the flag
      reportCount = 0;  // Reset the report counter
    }

    canReadTemp = false;

  }
  return avgTemperature;
}