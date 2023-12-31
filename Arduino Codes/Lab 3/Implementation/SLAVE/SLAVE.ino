//SERVER NODE

#define CPU_HZ 48000000
#define TIMER_PRESCALER_DIV 1024
#define WINDOW_SIZE 5  // Size of the sliding window   
#include <TemperatureZero.h>                       //Arduino library for internal temperature measurment of the family SAMD21 and SAMD51
TemperatureZero TempZero = TemperatureZero();

#include <SPI.h>                                   //This library allows you to communicate with SPI devices
#include <RH_RF95.h>
#include <FlashStorage.h>

// FlashStorage(error_Amlan, int);                 // Reserve a portion of flash memory, remove/comment when upload to the transmitter 
// FlashStorage(error_Shaswati, int);                                                   
// FlashStorage(error_Anuruddha, int);
// FlashStorage(error_Avhishek, int);
// FlashStorage(error_receive, int);

FlashStorage(error_storage, uint16_t);
uint16_t error_bits = 0x0000;

struct Packet {
  uint8_t nodeID;
  uint16_t packetID;
  uint32_t timestamp;
  float payload;
  int error;
  uint8_t authID;
};
                                                                 
volatile bool canReadTemp = false;
float tempBuffer[WINDOW_SIZE];  // Circular buffer to hold temperature values
int tempIndex = 0;  // Index to keep track of the oldest temperature value
float tempSum = 0;  // Sum of temperatures in the buffer
int tempCount = 0;  // Count of temperatures added to the buffer
int reportCount = 0;  // Count of readings before reporting
float avgTemperature = 0;  // To hold the average temperature
bool newAvgAvailable = false;  // Flag to indicate a new average is available

//Define functions used in the program
void startTimer(int frequencyHz); 
void configureClockTC4(); 
void configureTimerTC4(int frequencyHz);                //
void setTimerFrequencyTC4(TcCount16* TC, int frequencyHz);
void TC4_Handler();     

float Readtemp();                           
void write_error(String Node_name, int ecode);
//void print_errors(String err);
 

// //Define variables used 
// float movingAverage(float value);
// void Temperature();
// float temperature = 0.0;
// int i = 0;
// float total_temp = 0.0;

int previouse_packet_id = 0;
int current_packet_id = 0;

// String time_stamp;
// String node_id_r;
// String packet_id_r;
// String temp_value;
// String error;

// We need to provide the RFM95 module's chip select and interrupt pins to the 
// rf95 instance below.On the SparkFun ProRF those pins are 12 and 6 respectively.
RH_RF95 rf95(12, 6);
int LED = 13; //Status LED is on pin 13
long timeSinceLastPacket = 0; //Tracks the time stamp of last packet received

float frequency = 910; //Broadcast frequency
uint16_t packetCounter = 0;  // Initialize packet counter

// long timeSinceLastPacket = 0; 

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
      // It may be difficult to read serial messages on startup. The following line
    // will wait for serial to be ready before continuing. Comment out if not needed.
    // while (!SerialUSB);
     SerialUSB.println("RFM Client!");
    //Initialize the Radio.
     if (rf95.init() == false) {
        SerialUSB.println("Radio Init Failed - Freezing");
        while (1);
}
     else {
        //An LED inidicator to let us know radio initialization has completed.
        // rf95.setModemConfig(Bw125Cr48Sf4096); // slow and reliable?
        SerialUSB.println("Transmitter up!");
        digitalWrite(LED, HIGH);
        delay(500);
        digitalWrite(LED, LOW);
        delay(500);
        // Set frequency
     rf95.setFrequency(frequency);
        // Transmitter power can range from 14-20dbm.
     rf95.setTxPower(20, false);
     }
     pinMode(PIN_LED_13, OUTPUT);
     WDT->CTRL.bit.ENABLE = 1;
     reset_reason();
     startTimer(1);
}


void loop() 
{    
    Readtemp();
    if (newAvgAvailable) {
      // Print and send the average temperature
      SerialUSB.print("Average Temperature over last 5 seconds is: ");
      SerialUSB.println(avgTemperature);

      // Create the packet
      Packet packet;
      packet.nodeID = "Shaswati";  // Node 1
      packet.packetID = packetCounter++;
      packet.timestamp = millis();  // Current time in milliseconds
      packet.payload = avgTemperature;
      packet.error= String(error_bits, BIN);

      // Serialize the packet into a byte array
      uint8_t toSend[sizeof(Packet)];
      memcpy(toSend, &packet, sizeof(Packet));

      // Print and send the message
      SerialUSB.print("Sending packet with ID: ");
      SerialUSB.println(packet.packetID);
      rf95.send(toSend, sizeof(Packet));

      // Reset the flag
      newAvgAvailable = false;
    }  
    if (rf95.available()){
        // Should be a message for us now
        uint8_t buf[RH_RF95_MAX_MESSAGE_LEN];
        uint8_t len = sizeof(buf);
        if (rf95.recv(buf, &len))
        {
            // Cast the received buffer to the Packet struct type
            Packet *receivedPacket = (Packet *)buf;
            buf[len] = '\0';  // Null-terminate the received string
            digitalWrite(LED, HIGH); //Turn on status LED
            timeSinceLastPacket = millis(); //Timestamp this packet
            
            // SerialUSB.print("Got message: ");
            // SerialUSB.print((char*)buf);
            // SerialUSB.print(" RSSI: ");
            // SerialUSB.print(rf95.lastRssi(), DEC);
            // SerialUSB.println();

                        // Print the received packet details
            SerialUSB.print("Got message from Node ID: ");
            SerialUSB.print(receivedPacket->nodeID);
            // SerialUSB.print(", Packet ID: ");
            // SerialUSB.print(receivedPacket->packetID);
            // SerialUSB.print(", Timestamp: ");
            // SerialUSB.print(receivedPacket->timestamp);
            SerialUSB.print(", Payload (Temperature): ");
            SerialUSB.print(receivedPacket->payload);
        //     SerialUSB.print(" RSSI: ");
        //     SerialUSB.print(rf95.lastRssi(), DEC);
            SerialUSB.println();
            // print_errors(receivedPacket->error);
            //int int_error = receivedPacket->error.toInt();                            // convert the received error in String to integer 
              //  write_error(receivedPacket->nodeID, int_error) ;                       //Write the received packet error in the correct node memory allocation in the server 
      
               current_packet_id = receivedPacket->packetID;                 //Assign the current packet ID to the current_packet_id variable 
               
                     if(current_packet_id-previouse_packet_id > 1)            //check the missing packet 
                    //  write_error(receivedPacket->nodeID, 19) ; 
                     previouse_packet_id = current_packet_id;                    //save the error in the flash memory  
         }                                                            //Received message saving
         else
         {
         SerialUSB.println("Recieve failed");
        //  write_error("PKTError", 18);                               // Save the error code as message receive failed 
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
//    digitalWrite(PIN_LED_13, isLEDOn);
//    SerialUSB.println(isLEDOn ? "Blue LED is on" : "Blue LED is off");
//    isLEDOn = !isLEDOn;
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
 SerialUSB.println(bitRead(reset_reg, 6)); 
 bitWrite(error_bits, 0, bitRead(reset_reg, 6));
 bitWrite(error_bits, 1, bitRead(reset_reg, 5));
 bitWrite(error_bits, 2, bitRead(reset_reg, 4));
  bitWrite(error_bits, 13, 1);
}

void WDT_Handler() {
 //SerialUSB.println("WDT Interrupt");
 uint8_t reg_statusA = REG_DSU_STATUSA;
 uint8_t reg_statusB = REG_DSU_STATUSB;
 uint16_t status_errors = 0x0000;
 status_errors = reg_statusA;
 status_errors = status_errors << 8;
 status_errors |= reg_statusB;
 

 bitWrite(error_bits, 3, bitRead(status_errors, 12));
 bitWrite(error_bits, 4, bitRead(status_errors, 11));
 bitWrite(error_bits, 5, bitRead(status_errors, 10));
 bitWrite(error_bits, 6, bitRead(status_errors, 9));
 bitWrite(error_bits, 7, bitRead(status_errors, 4));
 bitWrite(error_bits, 8, bitRead(status_errors, 3));
 bitWrite(error_bits, 9, bitRead(status_errors, 2));
 bitWrite(error_bits, 10, bitRead(status_errors, 1));

}


// void write_error(String Node_name, String ecode)
// {
//   if(Node_name=="Amlan")
//   {error_Amlan.write(ecode);}

//    if(Node_name=="Shaswati")
//   {error_Shaswati.write(ecode);}

//     if(Node_name=="Anuruddha")
//   {error_Anuruddha.write(ecode);}

//     if(Node_name=="Avhishek")
//   {error_Avhishek.write(ecode);}
    
//     if(Node_name=="PKTError")
//   {error_receive.write(ecode);}

// }

// void print_errors(String err){

//   SerialUSB.println("Error");
//   // SerialUSB.println(err);
//   // SerialUSB.println(err.charAt(1));

//  if (err.charAt(13) == '1') {
//  SerialUSB.println("System Reset");
//  }
 
//  if (err.charAt(12) == '1') {
//  SerialUSB.println("Watchdog Reset");
//  }
//  if (err.charAt(11) == '1') {
//  SerialUSB.println("External Reset");
//  }
//  if (err.charAt(10) == '1') {
//  SerialUSB.println("Command not allowed error: PROTECTED STATE");
//  }
//  if (err.charAt(9) == '1') {
//  SerialUSB.println("DSU Operation Failure Error");
//  }
//  if (err.charAt(8) == '1') {
//  SerialUSB.println("BUS ERROR Detected!");
//  }
//  if (err.charAt(7) == '1') {
//  SerialUSB.println("COLD PLUGGING ERROR!!");
//  }
//  if (err.charAt(6) == '1') {
//  SerialUSB.println("HOT PLUGGING ERROR!");
//  }
//  if (err.charAt(5) == '1') {
//  SerialUSB.println("DCC1 Written Error");
//  }
//  if (err.charAt(4) == '1') {
//  SerialUSB.println("DCC0 Written Error");
//  }
//  if (err.charAt(3) == '1') {
//  SerialUSB.println("Debugger Probe Error");
//  }
//  if (err.charAt(2) == '1') {
//  SerialUSB.println("Missing Packet Error!");
//  }
//  if (err.charAt(1) == '1') {
//  SerialUSB.println("Packet Receival Error");
//  }
// // if (err.charAt(0) == '1') {
// // SerialUSB.println("Packet Receival Error");
// // }
 
  
// }

// //
// //if (ierror > 0){
// // SerialUSB.print("Node ");
// // SerialUSB.print(packet[0]);
// // SerialUSB.println(" had the following error(s):");
// // for (uint8_t i = 0; i < 16; i++){
// // if bitRead(ierror, i){
