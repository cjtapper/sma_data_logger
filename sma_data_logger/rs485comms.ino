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
#include "rs485comms.h"
#include "Arduino.h"
#include "sma_defs.h"

// Encapsulates an SMA Data packet in an SMA Net frame and transmits it
// via the RS485 serial.
void sendSMADataPacket(SMADataPacket * packet, HardwareSerial &serial) {

  //calculate fcs
  word fcs = 0xFFFF;
  byte tempAddr = SMA_NET_ADDR;
  byte tempCtrl = SMA_NET_CTRL;
  word tempProtocol = SMA_DATA_PROTOCOL_NUMBER;

  fcs = calculateFCS(fcs, &tempAddr, 1);
  fcs = calculateFCS(fcs, &tempCtrl, 1);
  fcs = fcsWord(fcs, tempProtocol);
  fcs = fcsWord(fcs, endianSwap16(packet->src));
  fcs = fcsWord(fcs, endianSwap16(packet->dest));
  fcs = calculateFCS(fcs, &packet->ctrl, 1);
  fcs = calculateFCS(fcs, &packet->pktCnt, 1);
  fcs = calculateFCS(fcs, &packet->cmd, 1);
  fcs = calculateFCS(fcs, packet->data, packet->dataLength);

  //take compliment of fcs and change to little endian
  fcs = fcs ^ 0xFFFF;
  fcs = endianSwap16(fcs);

  // Enable RS485 Transmit
  digitalWrite(RS485_ENABLE, HIGH);


  // send pre sequence
  sendWord(0xAAAA, serial);
  
  //write start byte
  //this doesn't use the 'sendByte' function
  sendStartStop(serial);

  //write SMA Net Address
  sendByte(SMA_NET_ADDR, serial);

  //write SMA Net Ctrl Byte
  sendByte(SMA_NET_CTRL, serial);

  //write Protocol header. This needs to be converted from big endian to little endian
  sendWord(SMA_DATA_PROTOCOL_NUMBER, serial);

  //write source and destination address of SMA data packet
  sendWord(endianSwap16(packet->src), serial);
  sendWord(endianSwap16(packet->dest), serial);

  sendByte(packet->ctrl, serial);
  sendByte(packet->pktCnt, serial);
  sendByte(packet->cmd, serial);

  //write SMA Data contents
  int i;
  for (i = 0; i < packet->dataLength; i++) {
    sendByte(packet->data[i], serial);
  }

  //write fcs 
  sendWord(fcs, serial);

  //write stop byte
  sendStartStop(serial);
  sendWord(0x5555, serial);

  while (!(UCSR0A & (1 << UDRE0))) UCSR0A |= 1 << TXC0;  // mark transmission not complete while the transmit buffer is not empty

  while (!(UCSR0A & (1 << TXC0)));   // Wait for the transmission to complete

  //disable rs485 transmission
  digitalWrite(RS485_ENABLE, LOW);

}

//reads a packet received over the rs485 interface into a packet buffer
int rs485Receive(SMADataPacket * packet, HardwareSerial &serial, unsigned int timeout) {

  boolean packetFinished = false;
  word bufferedBytes = 0;
  byte buffer[256];
  byte inputByte = 0;
  unsigned long previous = millis();

  while (inputByte != 0x7E) {
    //wait until we receive a start byte

    while (!serial.available() && ((millis() - previous) < timeout)); //spin until data is available or we timeout
    
    if (millis() - previous > timeout) return -1;

    inputByte = serial.read();
    
  }

  while (!packetFinished) {
    while (!serial.available()); //spin until data is available

    inputByte = serial.read();
    //Serial.print(inputByte, HEX);

    if (inputByte == 0x7E) {
      packetFinished = true;
    } else if (inputByte == 0x7D) {//remove escape characters
      while (!serial.available()); //spin until data is available
      inputByte = serial.read();
      inputByte = inputByte ^ 0x20;
      buffer[bufferedBytes] = inputByte;
      bufferedBytes++;
    } else {
      buffer[bufferedBytes] = inputByte;
      bufferedBytes++;
    }

  }
  
  // Check the checksum
  word goodFCS = makeWord(buffer[bufferedBytes - 1], buffer[bufferedBytes-2]);
  word fcs = 0xFFFF;

  fcs = calculateFCS(fcs, buffer, bufferedBytes - 2);

  fcs = fcs ^ 0xFFFF;

  if (fcs == goodFCS) {
    //serial.println("Packet has valid checksum");
  } else {
    serial.println(F("Packet has invalid checksum."));
    serial.println(goodFCS, HEX);
    serial.println(fcs, HEX);
  }

  //Check that the message we received was sent to the required broadcast address
  if (buffer[SMA_NET_ADDR_BYTE] != SMA_NET_ADDR) {
    serial.println(F("Received SMA Net packet has an invalid address"));
  }

  //Check control byte
  if (buffer[SMA_NET_CTRL_BYTE] != SMA_NET_CTRL) {
    serial.println(F("Received SMA Net packet has an invalid control byte"));
  }

  //parse SMA data packet, starting from the position after the SMA Net header
  parseSMADataPacket(buffer + SMA_NET_HEADER_LENGTH, bufferedBytes - SMA_NET_HEADER_LENGTH - SMA_NET_CHECKSUM_LENGTH, packet);

  //TODO: Proper error checking and returning different error codes
  return 1;
}

//writes a byte, first checking if it's an escape char
void sendByte(byte input, HardwareSerial &serial) {

  //Escape characters 0x7E and 0x7D get 'OR'd with 0x20
  // It seems that 0x11, 0x12 and 0x13 are also escape chars
  // ACCM value = 0x000E0000
  if (input == 0x7E || input == 0x7D || input == 0x11 || input == 0x12 || input == 0x13) {
    input = input ^ 0x20;
    serial.write(0x7D);
  }

  serial.write(input);
}

void sendWord(word input, HardwareSerial &serial) {

  byte secondByte = input & 0xFF;
  byte firstByte = (byte) ((input >> 8) & 0xFF);

  sendByte(firstByte, serial);
  sendByte(secondByte, serial); 
}

void sendStartStop(HardwareSerial &serial) {
  serial.write(SMA_NET_START_STOP);
}

//Parse "buffer" of length "len" into SMADataPacket "packet"
//TODO: get rid of magic numbers
void parseSMADataPacket(byte * buffer, int len, SMADataPacket * packet) {
  packet->src = makeWord(buffer[SRC_ADDR_MSB], buffer[SRC_ADDR_LSB]);
  packet->dest = makeWord(buffer[DEST_ADDR_MSB], buffer[DEST_ADDR_LSB]);
  packet->ctrl = buffer[SMA_DATA_CTRL];
  packet->cmd = buffer[SMA_DATA_CMD];
  packet->pktCnt = buffer[SMA_DATA_PKT_CNT];

  //loop through rest of buffer to read into data
  int i;
  for (i = SMA_DATA_HEADER_LENGTH; i < len; i++){
    packet->data[i - SMA_DATA_HEADER_LENGTH] = buffer[i];
  }

  packet->dataLength = len - SMA_DATA_HEADER_LENGTH;
}

//Makes a 16-bit word from 2 8-bit bytes
word makeWord(byte msb, byte lsb) {
  word output = (msb << 8) & 0xFF00;

  output += lsb;

  return output;
}

//swaps the order of the bytes in a 16 bit word
word endianSwap16(word input) {
  word output = input << 8;
  output = output ^ (input >> 8);

  return output;
}

//print out an SMA Data Packet, used for debugging
void printSMADataPacket(SMADataPacket * packet) {
  Serial.println(F("\nSMA Data Packet\n---------------"));
  Serial.print(F("SRC: "));
  Serial.println(packet->src, HEX);

  Serial.print(F("DEST: "));
  Serial.println(packet->dest, HEX);

  Serial.print(F("CTRL: "));
  Serial.println(packet->ctrl, HEX);

  Serial.print(F("PKTCNT: "));
  Serial.println(packet->pktCnt, HEX);

  Serial.print(F("CMD: "));
  Serial.println(packet->cmd, HEX);

  Serial.print(F("PAYLOAD LENGTH: "));
  Serial.println(packet->dataLength);

  Serial.print(F("DATA: "));

  int i;
  for (i = 0; i < packet->dataLength; i++) {
    if (packet->data[i] < 0x10) {
      Serial.print(F("0"));
    }
    Serial.print(packet->data[i], HEX);
  }
}
