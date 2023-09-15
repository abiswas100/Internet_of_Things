//MASTER  NODE = 1

#define CPU_HZ 48000000
#define TIMER_PRESCALER_DIV 1024
#define WINDOW_SIZE 5  // Size of the sliding window   
#include <TemperatureZero.h>                       //Arduino library for internal temperature measurment of the family SAMD21 and SAMD51
TemperatureZero TempZero = TemperatureZero();

#include <SPI.h>                                   //This library allows you to communicate with SPI devices
#include <RH_RF95.h>
#include <FlashStorage.h>


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

//Define functions used in the program
void startTimer(int frequencyHz); 
void configureClockTC4(); 
void configureTimerTC4(int frequencyHz);                //
void setTimerFrequencyTC4(TcCount16* TC, int frequencyHz);
void TC4_Handler();     
float Readtemp();                           
void write_error(int Node_name, int ecode);
void print_errors(char* err);
 
int previouse_packet_id = 0;
int current_packet_id = 0;

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
     startTimer(1);
}

void loop()
{ avgTemperature = Readtemp();
  // for Node =  slave 2
  if(reportCount == 1 && Slave2 == false)
  {    Master = false;
       Slave2 = true;
      // Send the authentication packet
      SerialUSB.println("Asking Node 2 to send it's temp. ");

      // Create the packet
      Packet packet;
      packet.nodeID = 1;  // Node 1
      packet.packetID = packetCounter++;
      packet.timestamp = millis();  // Current time in milliseconds
      packet.payload = avgTemperature;
      //packet.error = 0;
      packet.authID = 2;

      // Serialize the packet into a byte array
      uint8_t toSend[sizeof(Packet)];
      memcpy(toSend, &packet, sizeof(Packet));

      // Print and send the message
      SerialUSB.print("Sending authentication packet to Node 2: Packet Number :");
      SerialUSB.println(packet.packetID);
      rf95.send(toSend, sizeof(Packet));

      // Recieve the data packet from Node 2
      // SerialUSB.println(rf95.available());
      while(rf95.available() == 0){}

    // Recieve the data packet from Node 2
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
            
            //SAVE THE TEMPERATURE OF NODE 2
            node2_temperature = receivedPacket->payload;
            node1_temperature = avgTemperature;

            // Print the received packet details
            SerialUSB.print("Got message from Node ID: ");
            SerialUSB.print(receivedPacket->nodeID);
            SerialUSB.print(", Packet ID: ");
            SerialUSB.print(receivedPacket->packetID);
            SerialUSB.print(", Timestamp: ");
            SerialUSB.print(receivedPacket->timestamp);
            SerialUSB.print(", Payload (Temperature): ");
            SerialUSB.print(receivedPacket->payload);

            SerialUSB.println();
            print_errors(receivedPacket->error);
            //FLASH STORAGE DETAILS PRINT
            SerialUSB.println("FLASH STORAGE RESULTS");
            SerialUSB.print("AMLAN: ");SerialUSB.print(error_Amlan.read());
            SerialUSB.print(", SHASWATI: ");SerialUSB.print(error_Shaswati.read());
            SerialUSB.print(", ANURUDDHA: ");SerialUSB.print(error_Anuruddha.read());
            SerialUSB.print(", AVHISHEK: ");SerialUSB.print(error_Avhishek.read());
            SerialUSB.print(", ERROR RECEIVE ");SerialUSB.println(error_receive.read());

            SerialUSB.println();
            SerialUSB.println();
            
            SerialUSB.println("PRINTING EVERYONE'S TEMPERATURE");
            SerialUSB.print("(Avhi, "); SerialUSB.print(node1_temperature, 2); 
            SerialUSB.print("), (Amlan, "); SerialUSB.print(node2_temperature, 2); 
            SerialUSB.print("), (Shaswati, "); SerialUSB.print(node3_temperature, 2); 
            SerialUSB.print("), (Anu, "); SerialUSB.print(node4_temperature, 2); 
            SerialUSB.println(")");
            
            SerialUSB.println();
            SerialUSB.println();


            write_error(receivedPacket->nodeID, receivedPacket->error) ;                       //Write the received packet error in the correct node memory allocation in the server------changed 
      
               current_packet_id = receivedPacket->packetID;                 //Assign the current packet ID to the current_packet_id variable 
               
                     if(current_packet_id-previouse_packet_id > 1) {
                       receivedPacket->error[11]='1';
                     }           //check the missing packet 
                    //  write_error(receivedPacket->nodeID, 19) ; 
                     previouse_packet_id = current_packet_id;
         } 
         else
         {
         SerialUSB.println("Recieve failed");
         char temp1[]={'0','0','0','0','0','0','0','0','0','0','0','0','1','0','0','0'};
         write_error(5, temp1);                               // Save the error code as message receive failed 
         }           
     }
     timeSinceLastPacket = millis();       
  }

  // for Node = slave 3
  else if(reportCount == 2 && Slave3 == false)
  {   Slave3 = true;
      // Send the authentication packet
      SerialUSB.println("Asking Node 3 to send it's temp. ");

      // Create the packet
      Packet packet;
      packet.nodeID = 1;  // Node 1
      packet.packetID = packetCounter++;
      packet.timestamp = millis();  // Current time in milliseconds
      packet.payload = avgTemperature;
      // packet.error = 0;
      packet.authID = 3;

      // Serialize the packet into a byte array
      uint8_t toSend[sizeof(Packet)];
      memcpy(toSend, &packet, sizeof(Packet));

      // Print and send the message
      SerialUSB.print("Sending authentication packet to Node 3 : Packet Number : ");
      SerialUSB.println(packet.packetID);
      rf95.send(toSend, sizeof(Packet));

      // Recieve the data packet from Node 3
      // SerialUSB.println(rf95.available());
      while(rf95.available() == 0){}

    // Recieve the data packet from Node 3
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

            //SAVE THE TEMPERATURE OF NODE 2
            node3_temperature = receivedPacket->payload;
            node1_temperature = avgTemperature;

            // Print the received packet details
            SerialUSB.print("Got message from Node ID: ");
            SerialUSB.print(receivedPacket->nodeID);
            SerialUSB.print(", Packet ID: ");
            SerialUSB.print(receivedPacket->packetID);
            SerialUSB.print(", Timestamp: ");
            SerialUSB.print(receivedPacket->timestamp);
            SerialUSB.print(", Payload (Temperature): ");
            SerialUSB.print(receivedPacket->payload);
        //     SerialUSB.print(" RSSI: ");
        //     SerialUSB.print(rf95.lastRssi(), DEC);
            SerialUSB.println();
             print_errors(receivedPacket->error);
            //FLASH STORAGE DETAILS PRINT
            SerialUSB.println("FLASH STORAGE RESULTS");
            SerialUSB.print("AMLAN: ");SerialUSB.print(error_Amlan.read());
            SerialUSB.print(", SHASWATI: ");SerialUSB.print(error_Shaswati.read());
            SerialUSB.print(", ANURUDDHA: ");SerialUSB.print(error_Anuruddha.read());
            SerialUSB.print(", AVHISHEK: ");SerialUSB.print(error_Avhishek.read());
            SerialUSB.print(", ERROR RECEIVE ");SerialUSB.println(error_receive.read());

            
            SerialUSB.println();
            SerialUSB.println();
            
            SerialUSB.println("PRINTING EVERYONE'S TEMPERATURE");
            SerialUSB.print("(Avhi, "); SerialUSB.print(node1_temperature, 2); 
            SerialUSB.print("), (Amlan, "); SerialUSB.print(node2_temperature, 2); 
            SerialUSB.print("), (Shaswati, "); SerialUSB.print(node3_temperature, 2); 
            SerialUSB.print("), (Anu, "); SerialUSB.print(node4_temperature, 2); 
            SerialUSB.println(")");
            
            SerialUSB.println();
            SerialUSB.println();

            write_error(receivedPacket->nodeID, receivedPacket->error) ;                       //Write the received packet error in the correct node memory allocation in the server------changed 
      
               current_packet_id = receivedPacket->packetID;                 //Assign the current packet ID to the current_packet_id variable 
               
                     if(current_packet_id-previouse_packet_id > 1) {
                       receivedPacket->error[11]='1';
                     }           //check the missing packet 
                    //  write_error(receivedPacket->nodeID, 19) ; 
                     previouse_packet_id = current_packet_id;
         } 
         else
         {
         SerialUSB.println("Recieve failed");
         char temp1[]={'0','0','0','0','0','0','0','0','0','0','0','0','1','0','0','0'};
         write_error(5, temp1);                               // Save the error code as message receive failed 
         }           
     }
     timeSinceLastPacket = millis();
  }

  // for Node = slave 4
  else if(reportCount == 3 && Slave4 == false)
    {   Slave4 = true; Master = false;
      // Send the authentication packet
      SerialUSB.println("Asking Node 4 to send it's temp. ");

      // Create the packet
      Packet packet;
      packet.nodeID = 1;  // Node 1
      packet.packetID = packetCounter++;
      packet.timestamp = millis();  // Current time in milliseconds
      packet.payload = avgTemperature;
      // packet.error = 0;
      packet.authID = 4;

      // Serialize the packet into a byte array
      uint8_t toSend[sizeof(Packet)];
      memcpy(toSend, &packet, sizeof(Packet));

      // Print and send the message
      SerialUSB.print("Sending authentication packet to Node 4 : Packet Number : ");
      SerialUSB.println(packet.packetID);
      rf95.send(toSend, sizeof(Packet));

      
      // Recieve the data packet from Node 4
      // SerialUSB.println(rf95.available());
      while(rf95.available() == 0){}
      // SerialUSB.println(rf95.available());
      
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

            //SAVE THE TEMPERATURE OF NODE 2
            node4_temperature = receivedPacket->payload;
            node1_temperature = avgTemperature;

            // Print the received packet details
            SerialUSB.print("Got message from Node ID: ");
            SerialUSB.print(receivedPacket->nodeID);
            SerialUSB.print(", Packet ID: ");
            SerialUSB.print(receivedPacket->packetID);
            SerialUSB.print(", Timestamp: ");
            SerialUSB.print(receivedPacket->timestamp);
            SerialUSB.print(", Payload (Temperature): ");
            SerialUSB.print(receivedPacket->payload);
        //     SerialUSB.print(" RSSI: ");
        //     SerialUSB.print(rf95.lastRssi(), DEC);
            SerialUSB.println();
             print_errors(receivedPacket->error);
            //FLASH STORAGE DETAILS PRINT
            SerialUSB.println("FLASH STORAGE RESULTS");
            SerialUSB.print("AMLAN: ");SerialUSB.print(error_Amlan.read());
            SerialUSB.print(", SHASWATI: ");SerialUSB.print(error_Shaswati.read());
            SerialUSB.print(", ANURUDDHA: ");SerialUSB.print(error_Anuruddha.read());
            SerialUSB.print(", AVHISHEK: ");SerialUSB.print(error_Avhishek.read());
            SerialUSB.print(", ERROR RECEIVE ");SerialUSB.println(error_receive.read());


            SerialUSB.println();
            SerialUSB.println();
            
            SerialUSB.println("PRINTING EVERYONE'S TEMPERATURE");
            SerialUSB.print("(Avhi, "); SerialUSB.print(node1_temperature, 2); 
            SerialUSB.print("), (Amlan, "); SerialUSB.print(node2_temperature, 2); 
            SerialUSB.print("), (Shaswati, "); SerialUSB.print(node3_temperature, 2); 
            SerialUSB.print("), (Anu, "); SerialUSB.print(node4_temperature, 2); 
            SerialUSB.println(")");
            
            SerialUSB.println();
            SerialUSB.println();


            write_error(receivedPacket->nodeID, receivedPacket->error) ;                       //Write the received packet error in the correct node memory allocation in the server------changed 
      
               current_packet_id = receivedPacket->packetID;                 //Assign the current packet ID to the current_packet_id variable 
               
                     if(current_packet_id-previouse_packet_id > 1) {
                       receivedPacket->error[11]='1';
                     }           //check the missing packet 
                    //  write_error(receivedPacket->nodeID, 19) ; 
                     previouse_packet_id = current_packet_id;
         } 
         else
         {
         SerialUSB.println("Recieve failed");
         char temp1[]={'0','0','0','0','0','0','0','0','0','0','0','0','1','0','0','0'};
         write_error(5, temp1);                               // Save the error code as message receive failed 
         }           
     }
     timeSinceLastPacket = millis();
  }

  // Master sends it average temperature
  else if(reportCount == 4 && Master == false)
  {  // set the slaves false for the next 5 secs
      Slave2 = false;
      Slave3 = false;
      Slave4 = false;
      Master = true;
      // Send the authentication packet
      SerialUSB.println("Sending Master's Average Temperature. ");

      //SAVE THE TEMPERATURE OF NODE 2
      node1_temperature = avgTemperature;

      // Create the packet
      Packet packet;
      packet.nodeID = 1;  // Node 1
      packet.packetID = packetCounter++;
      packet.timestamp = millis();  // Current time in milliseconds
      packet.payload = avgTemperature;
      // packet.error = 0;
      packet.authID = 10;  // master is sending it's temperature

      // Serialize the packet into a byte array
      uint8_t toSend[sizeof(Packet)];
      memcpy(toSend, &packet, sizeof(Packet));

      // Print and send the message
      SerialUSB.print("SENDING TEMPERATURE OF MASTER TO ALL NODES : PACKET NUMBER :");
      SerialUSB.println(packet.packetID);
      SerialUSB.println();
      SerialUSB.println();
      rf95.send(toSend, sizeof(Packet));

      SerialUSB.println();
      SerialUSB.println();
      
      SerialUSB.println("PRINTING EVERYONE'S TEMPERATURE");
      SerialUSB.print("(Avhi, "); SerialUSB.print(node1_temperature, 2); 
      SerialUSB.print("), (Amlan, "); SerialUSB.print(node2_temperature, 2); 
      SerialUSB.print("), (Shaswati, "); SerialUSB.print(node3_temperature, 2); 
      SerialUSB.print("), (Anu, "); SerialUSB.print(node4_temperature, 2); 
      SerialUSB.println(")");
      
      SerialUSB.println();
      SerialUSB.println();

      
  }      
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

void write_error(uint8_t Node_name, char* ecodeX) //---------changed
{
  int ecode=atoi(ecodeX);
  if(Node_name==3)
  {error_Amlan.write(ecode);}

   if(Node_name==2)
  {error_Shaswati.write(ecode);}

    if(Node_name==4)
  {error_Anuruddha.write(ecode);}

    if(Node_name==1)
  {error_Avhishek.write(ecode);}
    
    if(Node_name==5)
  {error_receive.write(ecode);}

}

void print_errors(char* err){

  SerialUSB.println("Error=  ");
  // SerialUSB.println(err);
  // SerialUSB.println(err.charAt(1));

 if (err[0] == '1') {
 SerialUSB.println("System Reset");
 }
 
 if (err[1] == '1') {
 SerialUSB.println("Watchdog Reset");
 }
 if (err[2] == '1') {
 SerialUSB.println("External Reset");
 }
 if (err[3] == '1') {
 SerialUSB.println("Command not allowed error: PROTECTED STATE");
 }
 if (err[4] == '1') {
 SerialUSB.println("DSU Operation Failure Error");
 }
 if (err[5] == '1') {
 SerialUSB.println("BUS ERROR Detected!");
 }
 if (err[6] == '1') {
 SerialUSB.println("COLD PLUGGING ERROR!!");
 }
 if (err[7] == '1') {
 SerialUSB.println("HOT PLUGGING ERROR!");
 }
 if (err[8] == '1') {
 SerialUSB.println("DCC1 Written Error");
 }
 if (err[9] == '1') {
 SerialUSB.println("DCC0 Written Error");
 }
 if (err[10] == '1') {
 SerialUSB.println("Debugger Probe Error");
 }
 if (err[11] == '1') {
 SerialUSB.println("Missing Packet Error!");
 }
 if (err[12] == '1') {
 SerialUSB.println("Packet Receival Error");
 }
}