//SLAVE NODE 2 CODE

//NODE1 = AVHI
//NODE2 = SHASWATI
//NODE3 = AMLAN
//NODE4 = ANU

#define CPU_HZ 48000000
#define TIMER_PRESCALER_DIV 1024
#define WINDOW_SIZE 5  // Size of the sliding window   
#include <TemperatureZero.h>                       //Arduino library for internal temperature measurment of the family SAMD21 and SAMD51
TemperatureZero TempZero = TemperatureZero();

#include <SPI.h>                                   //This library allows you to communicate with SPI devices
#include <RH_RF95.h>

// Packet structure
struct Packet {
  int nodeID;
  int packetID;
  uint32_t timestamp;
  float payload;
  int error[13];
  int authID; 
  
  Packet() : nodeID(0), packetID(0), timestamp(0), payload(0.0) {
    // Initialize the error array to zeros
    memset(error, 0, sizeof(error));
  }
};

 
int global_error[13]={0,0,0,0,0,0,0,0,0,0,0,0,0};// for storing the error

volatile bool canReadTemp = false;
volatile bool Node1asked = false;

float tempBuffer[WINDOW_SIZE];  // Circular buffer to hold temperature values
int tempIndex = 0;  // Index to keep track of the oldest temperature value
float tempSum = 0;  // Sum of temperatures in the buffer
int tempCount = 0;  // Count of temperatures added to the buffer
int reportCount = 0;  // Count of readings before reporting
float avgTemperature = 0;  // To hold the average temperature
bool newAvgAvailable = false;  // Flag to indicate a new average is available

//Define variables for storing temperature of all nodes
float node1_temperature = 0;
float node2_temperature = 0;
float node3_temperature = 0;
float node4_temperature = 0;

//Define functions used in the program
void startTimer(int frequencyHz); 
void configureClockTC4(); 
void configureTimerTC4(int frequencyHz);                //
void setTimerFrequencyTC4(TcCount16* TC, int frequencyHz);
void TC4_Handler();     
float Readtemp();                           

// //Define variables used 
int previous_packet_id = 0;
int current_packet_id = 0;

// We need to provide the RFM95 module's chip select and interrupt pins to the 
// rf95 instance below.On the SparkFun ProRF those pins are 12 and 6 respectively.
RH_RF95 rf95(12, 6);

long timeSinceLastPacket = 0; //Tracks the time stamp of last packet received

float frequency = 910; //Broadcast frequency
uint16_t packetCounter = 0;  // Initialize packet counter

void setup() {
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
     SerialUSB.begin(9600);
     TempZero.init();
     SerialUSB.println("RFM Client!");
    
    //Initialize the Radio.
     if (rf95.init() == false) {
        SerialUSB.println("Radio Init Failed - Freezing");
        while (1);
      }
     else {
       rf95.setFrequency(frequency);
          // Transmitter power can range from 14-20dbm.
       rf95.setTxPower(20, false);
     }
     
     WDT->CTRL.bit.ENABLE = 1;
     reset_reason();
     startTimer(1);
}


void loop() 
{    
    node2_temperature = Readtemp();

    if (rf95.available()){
       uint8_t buf[RH_RF95_MAX_MESSAGE_LEN];
       uint8_t len = sizeof(buf);
       if (rf95.recv(buf, &len)){
          // Cast the received buffer to the Packet struct type
            Packet *receivedPacket = (Packet *)buf;
//            buf[len] = '\0';  // Null-terminate the received string
            timeSinceLastPacket = millis(); //Timestamp this packet
            
            

             // Print the received packet details
            SerialUSB.print("Got message from Node ID: ");
            SerialUSB.print(receivedPacket->nodeID);
            SerialUSB.print(" auth ID: ");
            SerialUSB.print(receivedPacket->authID);
            SerialUSB.println();
           
            if(receivedPacket->nodeID == 1 && receivedPacket->authID == 2 && Node1asked == false){
              Node1asked = true;              
              SerialUSB.print("Average Temperature over last 5 seconds is: ");
              SerialUSB.println(avgTemperature);

              // Create the packet
              Packet packet;
              
              packet.nodeID = 2;  // Node 2
              packet.packetID = packetCounter++;
              packet.timestamp = millis();  // Current time in milliseconds
              packet.payload = avgTemperature;
              packet.authID  = 0;
              for(int i=0;i<13;i++){
                packet.error[i]=global_error[i];
              }
              // Serialize the packet into a byte array
              uint8_t toSend[sizeof(Packet)];
              memcpy(toSend, &packet, sizeof(Packet));
              SerialUSB.print("Node 2 Sending packet with ID: ");
              SerialUSB.println(packet.packetID);

//              // Print and send the message
//              node1_temperature = receivedPacket->payload;
              rf95.send(toSend, sizeof(Packet));              SerialUSB.println("sent the val");
              // Reset the flag
              newAvgAvailable = false;
              for(int i=0;i<13;i++){
                global_error[i]=0;
              }

            }

            else if(receivedPacket->nodeID == 3){
                // SAVE NODE 3 'S TEMPERATURE
                node3_temperature = receivedPacket->payload;
              }
              
            else if(receivedPacket->nodeID == 4){
                // SAVE NODE 4 'S TEMPERATURE
                node4_temperature = receivedPacket->payload;
              }
            else if (receivedPacket->nodeID == 1 && receivedPacket->authID == 10){
              node1_temperature = receivedPacket->payload;
              Node1asked = false;
//              SerialUSB.print(", Packet ID: ");
//              SerialUSB.print(receivedPacket->packetID);
//              SerialUSB.print(", Timestamp: ");
//              SerialUSB.print(receivedPacket->timestamp);
//              SerialUSB.print(", Payload (Temperature): ");
//              SerialUSB.print(receivedPacket->payload);
//              SerialUSB.print(" RSSI: ");
//              SerialUSB.print(rf95.lastRssi(), DEC);
//              
//              SerialUSB.println();
//              SerialUSB.println();
              SerialUSB.println("PRINTING EVERYONE'S TEMPERATURE");
              SerialUSB.print("(Avhi: "); SerialUSB.print(node1_temperature, 2); 
              SerialUSB.print("), (Amlan: "); SerialUSB.print(node3_temperature, 2); 
              SerialUSB.print("), (Shaswati: "); SerialUSB.print(node2_temperature, 2); 
              SerialUSB.print("), (Anu: "); SerialUSB.print(node4_temperature, 2); 
              SerialUSB.println(")");
              
              SerialUSB.println();
              SerialUSB.println();
              }   
              
       
    }
    else {
       SerialUSB.println("Recieve failed");
    }

  }
  timeSinceLastPacket = millis();
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
    WDT->CLEAR.reg = WDT_CLEAR_CLEAR_KEY; 
    canReadTemp = true;
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


void reset_reason(){
 //determine last reset type
 uint8_t reset_reg = PM->RCAUSE.reg;
 if(bitRead(reset_reg, 6)==0){global_error[0]=1;}
 if(bitRead(reset_reg, 5)==1){global_error[1]=1;}
 if(bitRead(reset_reg, 4)==0){global_error[2]=1;}
 
}

void WDT_Handler() {
 //SerialUSB.println("WDT Interrupt");
 uint8_t reg_statusA = REG_DSU_STATUSA;
 uint8_t reg_statusB = REG_DSU_STATUSB;
 uint16_t status_errors = 0x0000;
 status_errors = reg_statusA;
 status_errors = status_errors << 8;
 status_errors |= reg_statusB;
 
 global_error[3]=bitRead(status_errors, 12);
 global_error[4]=bitRead(status_errors, 11);
 global_error[5]=bitRead(status_errors, 10);
 global_error[6]=bitRead(status_errors, 9);
 global_error[7]=bitRead(status_errors, 4);
 global_error[8]=bitRead(status_errors, 3);
 global_error[9]=bitRead(status_errors, 2);
 global_error[10]=bitRead(status_errors, 1);
}
