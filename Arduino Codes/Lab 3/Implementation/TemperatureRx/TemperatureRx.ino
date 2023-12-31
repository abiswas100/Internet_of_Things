struct Packet {
  uint8_t nodeID;
  uint16_t packetID;
  uint32_t timestamp;
  float payload;
};


#include <SPI.h>
//Radio Head Library:
#include <RH_RF95.h>
// We need to provide the RFM95 module's chip select and interrupt pins to the
// rf95 instance below.On the SparkFun ProRF those pins are 12 and 6 respectively.
RH_RF95 rf95(12, 6);
int LED = 13; //Status LED is on pin 13
int packetCounter = 0; //Counts the number of packets sent
long timeSinceLastPacket = 0; //Tracks the time stamp of last packet received
float frequency = 910; //Broadcast frequency


void setup()
{
    pinMode(LED, OUTPUT);
    SerialUSB.begin(9600);
    // It may be difficult to read serial messages on startup. The following line
    // will wait for serial to be ready before continuing. Comment out if not needed.
    // while (!SerialUSB);  // can be used to debug, waits for serial port to get ready
    SerialUSB.println("RFM Client!");
    //Initialize the Radio.
    if (rf95.init() == false) 
    {
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
  }
  // Set frequency
  rf95.setFrequency(frequency);
  // Transmitter power can range from 14-20dbm.
  rf95.setTxPower(20, false);
}


void loop()
{
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
            SerialUSB.print(", Packet ID: ");
            SerialUSB.print(receivedPacket->packetID);
            SerialUSB.print(", Timestamp: ");
            SerialUSB.print(receivedPacket->timestamp);
            SerialUSB.print(", Payload (Temperature): ");
            SerialUSB.print(receivedPacket->payload);
            SerialUSB.print(" RSSI: ");
            SerialUSB.print(rf95.lastRssi(), DEC);
            SerialUSB.println();
        }
        else
        SerialUSB.println("Recieve failed");
    }
    //Turn off status LED if we haven't received a packet after 1s
    if(millis() - timeSinceLastPacket > 1000)
    {
        digitalWrite(LED, LOW); //Turn off status LED
        timeSinceLastPacket = millis(); //Don't write LED but every 1s
    }
}