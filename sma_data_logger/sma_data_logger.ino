 /*
  * SMA Data Logger
  * Copyright (C) 2014, Christopher Tapper
  *
  * This program is free software: you can redistribute it and/or modify
  * it under the terms of the GNU General Public License as published by
  * the Free Software Foundation, either version 3 of the License, or
  * (at your option) any later version.
  *
  * This program is distributed in the hope that it will be useful,
  * but WITHOUT ANY WARRANTY; without even the implied warranty of
  * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  * GNU General Public License for more details.
  *
  * You should have received a copy of the GNU General Public License
  * along with this program.  If not, see <http://www.gnu.org/licenses/>
  *
  */

#include "sma_defs.h"
#include "rs485comms.h"
#include <Time.h>
#include <SoftwareSerial.h>
#include <SD.h>

#define MY_ADDRESS 0x0000        //SMA network address of the data logger
#define DEVICE_ADDRESS 0x00F3    //address that we want to assign to the inverter during network configuration
#define GSM_BAUD 9600            //baud rate for the GSM shield
#define RS485_BAUD 1200          //baud rate for RS485 comms
#define LOG_PERIOD 300           //time between data samples in seconds


SMADataPacket packetBuffer;     //buffer used for packets to be sent or received
SMADevice device1;              //stores info about the inverter we are connected to
SMADevice receivedDevice;       //device buffer for comparing the device we get responses from to device1
SoftwareSerial gsmSerial(8,9);  //gsm serial interface. Pins 8 and 9 were chosen as they are the pins we have rewired
                                //   to the bent pins on the GSM board
                                
time_t lastSent = 0;            //time that we last logged data
unsigned long eTotal = 0;       //variable used for storing the last received data response

void setup () {
  
  //Initialise the Serial port (pins 0 and 1) for RS485 comms and printing to USB serial
  Serial.begin(RS485_BAUD);
  
  //Initialise SD card. Make sure that the default chip select pin is set to
  // output, even if you don't use it:
  Serial.print(F("Initializing SD card..."));
  pinMode(10, OUTPUT);
  
  // see if the card is present and can be initialized:
  if (!SD.begin(10)) {
    Serial.println(F("Card failed, or not present"));
    // don't do anything more:
    return;
  }
  Serial.println(F("card initialised."));
  
  // Start the software serial for the GSM shield at the correct baud rate
  gsmSerial.begin(GSM_BAUD);
  
  //Configure the RS485 read/write pin
  pinMode(RS485_ENABLE, OUTPUT);
  digitalWrite(RS485_ENABLE, LOW);
  
  //Initialise the GSM shield
  Serial.println(F("Initialising GSM..."));
  gsmPowerOn();
  delay(5000);
  Serial.println(F("Connecting to the network..."));
  
  //Check that network is connected
  while( (sendATcommand(F("AT+CREG?"), "+CREG: 0,1", 500) 
            || sendATcommand(F("AT+CREG?"), "+CREG: 0,5", 500)) == 0 );
  configure_FTP();

  // attempt to set the system time, but timeout after 2 seconds
  setArduinoTime(2000);
  Serial.println(now(), DEC); //print the current time
  
  //set the address of this device to the correct one
  device1.addr = DEVICE_ADDRESS;
  
  // CONFIGURE SMA NETWORK
 
  // First we must signal that we are beginning the configuration 
  Serial.println(F("Sending CMD_GET_NET_START"));
  makeCMD_GET_NET_START(&packetBuffer, MY_ADDRESS);
  sendSMADataPacket(&packetBuffer, Serial);
  
  //Wait for a response, but timeout after 5 seconds
  rs485Receive(&packetBuffer, Serial, 5000);

  //parse the response, perform error checking   
  if (parseCMD_GET_NET_START(&packetBuffer, &device1)) {
    Serial.print(F("\nReceived CMD_GET_NET_START response from device "));
    Serial.print(device1.serial);
    Serial.print(F("\n"));
  } else {
    Serial.print(F("Invalid response packet.\n"));
    return;
    //TODO: implement what should happen if we receive an invalid packet
  }
  
  delay(30); // This delay is required between sending and receiving messages

  // Now we attempt to set the address of the device
  Serial.print(F("Setting network address of device "));
  Serial.print(device1.serial);
  Serial.print(F(" to "));
  Serial.print(DEVICE_ADDRESS, HEX);
  Serial.print(F("\nSending CMD_CFG_NETADR\n"));
  
  makeCMD_CFG_NET_ADDR(&packetBuffer, MY_ADDRESS, &device1);
  sendSMADataPacket(&packetBuffer, Serial);
  
  //Wait for a response, but timeout after 5 seconds
  rs485Receive(&packetBuffer, Serial, 5000); // timeout after 5 seconds
  
  //parse the response, perform error checking   
  if (parseCMD_CFG_NETADR(&packetBuffer, &receivedDevice)) {
    
    Serial.print(F("\nReceived CMD_CFG_NETADR response from device "));
    Serial.print(receivedDevice.serial);
    Serial.print(F("\n"));
    
    if (receivedDevice.addr == DEVICE_ADDRESS) { //check that the address was changed properly
      device1.addr = DEVICE_ADDRESS;
      Serial.print(F("Address of device "));
      Serial.print(device1.serial);
      Serial.print(F(" succesfully changed to "));
      Serial.print(device1.addr, HEX);
      Serial.print(F("\n"));
    } else {
      Serial.print(F("Address not set succesfully\n"));
      Serial.println(device1.addr, HEX);
      //TODO: implement what should happen if we receive an invalid packet
    }
    
  } else {
    Serial.print(F("Invalid response packet.\n"));
    //TODO: implement what should happen if we receive an invalid packet
    
  }
  
  delay(30); // This delay is required between sending and receiving messages

  // Send a search device command to confirm that everything is configured correctly
  Serial.print(F("Searching for device "));
  Serial.print(device1.serial);
  Serial.print(F("\nSending CMD_SEARCH_DEV\n"));
  makeCMD_SEARCH_DEV(&packetBuffer, MY_ADDRESS, &device1);
  sendSMADataPacket(&packetBuffer, Serial);
  
  //Wait for a response, but timeout after 5 seconds
  rs485Receive(&packetBuffer, Serial, 5000); // timeout after 5 seconds
  
  //parse the response, perform error checking
  if (parseCMD_SEARCH_DEV(&packetBuffer, &receivedDevice)) {
    
    Serial.print(F("\nReceived CMD_SEARCH_DEV response from device "));
    Serial.print(receivedDevice.serial);
    Serial.print(F("\n"));
    
    if (receivedDevice.addr == DEVICE_ADDRESS) { //check that the address was changed properly
      Serial.println(F("Device found successfully"));
    } else {
      Serial.print(F("Address not set succesfully.\n"));
      //TODO: implement what should happen if we receive an invalid packet
    }
    
  } else {
    Serial.print(F("Invalid response packet.\n"));
    //TODO: implement what should happen if we receive an invalid packet
  }

}


//Main loop of the software starts here
void loop () {
   
   // Check if it is time to log data
   if (now() - lastSent >= LOG_PERIOD) {
     time_t currentTime = now();
     lastSent = now();
    
    // Sync online data
    Serial.print(F("Synchronising online data..."));
    Serial.print(F("\nSending CMD_SYN_ONLINE\n"));
    makeCMD_SYN_ONLINE(&packetBuffer, MY_ADDRESS, &device1, currentTime);
    sendSMADataPacket(&packetBuffer, Serial);
    
    delay(30); //delay required between sending packets
      
    // Get data
    Serial.println(F("\nSending CMD_GET_DATA"));
    makeCMD_GET_DATA(&packetBuffer, MY_ADDRESS, &device1);  
    sendSMADataPacket(&packetBuffer, Serial);
  
    // try to receive packet, but if we timeout after 5 seconds, resend the CMD_SYN_ONLINE
    if (rs485Receive(&packetBuffer, Serial, 5000) == -1) {
       Serial.println(F("\nResponse timed out.")); 
       return; 
    }
    
    //parse to get the energy. Error checking probably doesn't quite work properly here yet.
    eTotal = parseCMD_GET_DATA(&packetBuffer, currentTime);
    if (eTotal) {
      Serial.print(F("\nReceived CMD_GET_GET_DATA response: "));
    } else {
      Serial.print(F("Invalid response packet.\n"));
    }
    
    //Build data string from the timestamp and measured energy
    String dataString = "";
    dataString += String(currentTime);
    dataString += ",";
    dataString += eTotal;
    
    // Write the data to the SD card
    File dataFile = SD.open("datalog.txt", FILE_WRITE);
    
    Serial.println(dataString); //print to serial monitor just for checking
    
    // if the file is available, write to it:
    if (dataFile) {
      dataFile.println(dataString);
      dataFile.close();
      // print to the serial port too:
    } 
    
    // Upload data via FTP
    dataString += "\n"; //we need to add the new line character
    char buffer[dataString.length() + 1]; //we add 1 to ensure that the newline character is included
  
    dataString.toCharArray(buffer, dataString.length()+1);
    uploadFTP(buffer, dataString.length());
     
    setArduinoTime(2000); //refresh time in case it has changed
   }
  
}
