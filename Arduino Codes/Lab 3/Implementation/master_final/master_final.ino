//MASTER NODE

//NODE1=AVHI
//NODE2=SHASWATI
//NODE3=AMLAN
//NODE4=ANU

#define CPU_HZ 48000000
#define TIMER_PRESCALER_DIV 1024
#define WINDOW_SIZE 5  // Size of the sliding window   
#include <TemperatureZero.h>                       //Arduino library for internal temperature measurment of the family SAMD21 and SAMD51
TemperatureZero TempZero = TemperatureZero();

#include <SPI.h>                                   //This library allows you to communicate with SPI devices
#include <RH_RF95.h>
#include <FlashStorage.h>
#include <string.h>
#include <math.h>

FlashStorage(error_Amlan, int);                 // Reserve a portion of flash memory, remove/comment when upload to the transmitter 
FlashStorage(error_Shaswati, int);                                                   
FlashStorage(error_Anuruddha, int);
FlashStorage(error_Avhishek, int);
FlashStorage(error_receive, int);

struct Packet {
  int nodeID;
  int packetID;
  uint32_t timestamp;
  float payload;
  int error[13];
  int authID;
};

                                                                 
volatile bool canReadTemp = false;
volatile bool Slave2 = false;
volatile bool Slave3 = false;
volatile bool Slave4 = false;
volatile bool Master = false;

float tempBuffer[WINDOW_SIZE];  // Circular buffer to hold temperature values
int tempIndex = 0;  // Index to keep track of the oldest temperature value
float tempSum = 0;  // Sum of temperatures in the buffer
int tempCount = 0;  // Count of temperatures added to the buffer
int reportCount = 0;  // Count of readings before reporting
float avgTemperature = 0;  // To hold the average temperature
bool newAvgAvailable = false;  // Flag to indicate a new average is available
bool did_receive = true;

//Define functions used in the program
void startTimer(int frequencyHz); 
void configureClockTC4(); 
void configureTimerTC4(int frequencyHz);                //
void setTimerFrequencyTC4(TcCount16* TC, int frequencyHz);
void TC4_Handler();     

float Readtemp();                           
void write_error();
void print_errors();

float amlan_temp=0;
float avhi_temp=0;
float shaswati_temp=0;
float anu_temp=0;
 

// //Define variables used 

int previouse_packet_id = 0;
int current_packet_id = 0;

int global_error[13]={0,0,0,0,0,0,0,0,0,0,0,0,0};
int global_nodeID=0;

// We need to provide the RFM95 module's chip select and interrupt pins to the 
// rf95 instance below.On the SparkFun ProRF those pins are 12 and 6 respectively.
RH_RF95 rf95(12, 6);
int LED = 13; //Status LED is on pin 13
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
{    
    Readtemp();
    //for slave node 2
    if(reportCount == 1)
  {   
    if (Slave2 == false){
      Master=false;
      Slave2=true;
      // Send the authentication packet
      SerialUSB.println("Asking Node 2 to send it's temp and details ");
      send_packet(2);
        }
     if(did_receive==true)
     {
      receive();    
     }
      
   }





    //for slave node 3
    else if(reportCount == 2)
  {   
    if (Slave3 == false){
      Master=false;
      Slave3=true;
      // Send the authentication packet
      SerialUSB.println("Asking Node 3 to send it's temp and details ");
      send_packet(3);
        }
     if(did_receive==true)
     {
      receive();    
     }
      
   }







 // for Slave node 4
  else if(reportCount == 3)
  {   
    if (Slave4 == false){
      Master=false;
      Slave4=true;
      // Send the authentication packet
      SerialUSB.println("Asking Node 4 to send it's temp and details ");
      send_packet(4);
        }
     if(did_receive==true)
     {
      receive();    
     }
      
   }







      // Master sends it average temperature
  else if(reportCount == 4 && Master == false && newAvgAvailable==true)
  {  // set the slaves false for the next 5 secs
      Slave2 = false;
      Slave3 = false;
      Slave4 = false;
      Master = true;
      SerialUSB.println("Sending Master's Average Temperature. ");
      // Create the packet
      struct Packet packet = {};
      packet.nodeID = 1;  // Node 2
      packet.packetID = packetCounter++;
      packet.timestamp = millis();  // Current time in milliseconds
      packet.payload = avgTemperature;
      packet.authID=10;

      avhi_temp=avgTemperature;
      
      // Serialize the packet into a byte array
      uint8_t toSend[sizeof(Packet)];
      memcpy(toSend, &packet, sizeof(Packet));

      // Print and send the message
      SerialUSB.print("Sending packet with ID: ");
      SerialUSB.println(packet.packetID);
      rf95.send(toSend, sizeof(Packet));


      SerialUSB.println("PRINTING EVERYONE'S TEMPERATURE");
      SerialUSB.print("(Avhi: ");SerialUSB.print(avhi_temp);
      SerialUSB.print("), (Shaswati: ");SerialUSB.print(shaswati_temp);
      SerialUSB.print("), (Amlan: ");SerialUSB.print(amlan_temp);
      SerialUSB.print("), (Anu: ");SerialUSB.print(anu_temp);
      SerialUSB.println(")");
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


void write_error() //---------changed
{
  int flash_val=0;
  int counter=12;
  for(int i=0;i<13;i++)
  {
    flash_val=flash_val+(global_error[i]*pow(2,counter));
    counter--;
  }
  if(global_nodeID==3)
  {error_Amlan.write(flash_val);}

   if(global_nodeID==2)
  {error_Shaswati.write(flash_val);}

    if(global_nodeID==4)
  {error_Anuruddha.write(flash_val);}

    if(global_nodeID==1)
  {error_Avhishek.write(flash_val);}
    
    if(global_nodeID==5)
  {error_receive.write(flash_val);}
  SerialUSB.println(" THE ERRORS IN THE PACKET ARE: ");
  print_errors();
  SerialUSB.println("FLASH STORAGE RESULTS");
  SerialUSB.print("AMLAN: ");SerialUSB.print(error_Amlan.read());
  SerialUSB.print(", SHASWATI: ");SerialUSB.print(error_Shaswati.read());
  SerialUSB.print(", ANURUDDHA: ");SerialUSB.print(error_Anuruddha.read());
  SerialUSB.print(", AVHISHEK: ");SerialUSB.print(error_Avhishek.read());
  SerialUSB.print(", ERROR RECEIVE ");SerialUSB.println(error_receive.read());
  SerialUSB.println();
  SerialUSB.println();
}

void print_errors(){

 if (global_error[0] == 1) {
 SerialUSB.println("External Reset");
 }
 
 if (global_error[1] == 1) {
 SerialUSB.println("Watchdog Reset");
 }
 if (global_error[2] == 1) {
 SerialUSB.println("System Reset");
 }
 if (global_error[3] == 1) {
 SerialUSB.println("Command not allowed error: PROTECTED STATE");
 }
 if (global_error[4] == 1) {
 SerialUSB.println("DSU Operation Failure Error");
 }
 if (global_error[5] == 1) {
 SerialUSB.println("BUS ERROR Detected!");
 }
 if (global_error[6] == 1) {
 SerialUSB.println("COLD PLUGGING ERROR!!");
 }
 if (global_error[7] == 1) {
 SerialUSB.println("HOT PLUGGING ERROR!");
 }
 if (global_error[8] == 1) {
 SerialUSB.println("DCC1 Written Error");
 }
 if (global_error[9] == 1) {
 SerialUSB.println("DCC0 Written Error");
 }
 if (global_error[10] == 1) {
 SerialUSB.println("Debugger Probe Error");
 }
 if (global_error[11] == 1) {
 SerialUSB.println("Missing Packet Error!");
 }
 if (global_error[12] == 1) {
 SerialUSB.println("Packet Receival Error");
 }  
}


void send_packet(int auth)
{
      struct Packet packet = {};
      packet.nodeID = 1;  // Node 2
      packet.packetID = packetCounter++;
      packet.timestamp = millis();  // Current time in milliseconds
      packet.payload = avgTemperature;
      packet.authID=auth;

      // Serialize the packet into a byte array
      uint8_t toSend[sizeof(Packet)];
      memcpy(toSend, &packet, sizeof(Packet));

      // Print and send the message
      SerialUSB.print("Sending packet with ID: ");
      SerialUSB.println(packet.packetID);
      rf95.send(toSend, sizeof(Packet));
      did_receive=true;
}

bool receive()
      {
      // Recieve the data packet from Node 2
      if (rf95.available()){
        //Should be a message for us now
        uint8_t buf[RH_RF95_MAX_MESSAGE_LEN];
        uint8_t len = sizeof(buf);
        if (rf95.recv(buf, &len))
        {
            // Cast the received buffer to the Packet struct type
            Packet *receivedPacket = (Packet *)buf;
//            buf[len] = '\0';  // Null-terminate the received string
            digitalWrite(LED, HIGH); //Turn on status LED
            timeSinceLastPacket = millis(); //Timestamp this packet
            // Print the received packet details
            SerialUSB.print("Got message from Node ID: ");
            SerialUSB.print(receivedPacket->nodeID);
            global_nodeID=receivedPacket->nodeID;
            SerialUSB.print(", Packet ID: ");
            SerialUSB.print(receivedPacket->packetID);
            SerialUSB.print(", Timestamp: ");
            SerialUSB.print(receivedPacket->timestamp);
            SerialUSB.print(", Payload (Temperature): ");
            SerialUSB.print(receivedPacket->payload);
            if(receivedPacket->nodeID==2){shaswati_temp=receivedPacket->payload;}
            if(receivedPacket->nodeID==3){amlan_temp=receivedPacket->payload;}
            if(receivedPacket->nodeID==4){anu_temp=receivedPacket->payload;}
        //     SerialUSB.print(" RSSI: ");
        //     SerialUSB.print(rf95.lastRssi(), DEC);
            SerialUSB.println();
            for(int j=0;j<13;j++){
              global_error[j]=receivedPacket->error[j];
            }
            current_packet_id = receivedPacket->packetID;                 //Assign the current packet ID to the current_packet_id variable 
            if(current_packet_id-previouse_packet_id > 1) {
                       global_error[11]='1';
                     }           //check the missing packet 
                    //  write_error(receivedPacket->nodeID, 19) ; 
            previouse_packet_id = current_packet_id;
            write_error(); 
                     //save the error in the flash memory  
         }                                                            //Received message saving
         else
         {
         SerialUSB.println("Recieve failed");
         global_nodeID=5;
         global_error[12]='1';
         write_error();                              // Save the error code as message receive failed 
         }
         did_receive=false;
     }  
     timeSinceLastPacket = millis();
  }
