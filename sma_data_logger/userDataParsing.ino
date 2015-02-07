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

#define CMD_GET_NET_START_TYPE_OFFSET 4
#define E_TOTAL_OFFSET 47
#define TIMESTAMP_OFFSET 5

// Parses the user data in packet into the device information.
// Returns 1 on success, 0 on failure
// This is for CMD_GET_NET_START packets
// The first 4 bytes of the user data are the serial number of the device
// (little endian) and the last 8 bytes are the device type in ASCII. If the
// device type is less than 8 characters long it is padded with ASCII zeros.
int parseCMD_GET_NET_START(SMADataPacket * packet, SMADevice * device) {

  //Check that the correct cmd type was sent
  if (packet->cmd != CMD_GET_NET_START) return 0;

  // Check that the response flag is set
  if (!(packet->ctrl & SMADATA_FLAG_RESPONSE)) return 0;

  // The user data section of the packet should be 12 bytes long
  if (packet->dataLength != 12) return 0;

  // Copy the serial number. Arduino is little endian so there shouldn't be any
  // problems directly copying it
  memcpy(&device->serial, packet->data, 4*sizeof(byte));

  // Copy the device type
  memcpy(&device->deviceType, packet->data + CMD_GET_NET_START_TYPE_OFFSET, 8 * sizeof(byte));

  // Add string terminator
  device->deviceType[8] = '\0';

  return 1; 
}

// Parses the user data in packet into the device information.
// Returns 1 on success, 0 on failure
// This is for CMD_CFG_NETADR packets
// The 4 bytes of the user data are the serial number of the device
// (little endian)
int parseCMD_CFG_NETADR(SMADataPacket * packet, SMADevice * device) {
  //Check that the correct cmd type was sent
  if (packet->cmd != CMD_CFG_NETADR) return 0;

  // Check that the response flag is set
  if (!(packet->ctrl & SMADATA_FLAG_RESPONSE)) return 0;

  // The user data section of the packet should be 4 bytes long
  if (packet->dataLength != 4) return 0;
  
  // Copy the serial number. Arduino is little endian so there shouldn't be any
  // problems directly copying it
  memcpy(&device->serial, packet->data, 4*sizeof(byte));
  
  //set the address of the device to the source of the message
  device->addr = packet->src;
  
  return 1;
}

// Parses the user data in packet into the device information.
// Returns 1 on success, 0 on failure
// This is for CMD_SEARCH_DEV packets
// The first 4 bytes of the user data are the serial number of the device
// (little endian) and the last 8 bytes are the device type in ASCII. If the
// device type is less than 8 characters long it is padded with ASCII zeros.
int parseCMD_SEARCH_DEV(SMADataPacket * packet, SMADevice * device) {

  //Check that the correct cmd type was sent
  if (packet->cmd != CMD_SEARCH_DEV) return 0;

  // Check that the response flag is set
  if (!(packet->ctrl & SMADATA_FLAG_RESPONSE)) return 0;

  // The user data section of the packet should be 12 bytes long
  if (packet->dataLength != 12) return 0;

  // Copy the serial number. Arduino is little endian so there shouldn't be any
  // problems directly copying it
  memcpy(&device->serial, packet->data, 4*sizeof(byte));

  // Copy the device type
  memcpy(&device->deviceType, packet->data + CMD_GET_NET_START_TYPE_OFFSET, 8 * sizeof(byte));

  // Add string terminator
  device->deviceType[8] = '\0';

  return 1; 
}

// Parses the user data in packet into the device information.
// Returns the total energy generated
// This is for CMD_GET_DATA packets
unsigned long parseCMD_GET_DATA(SMADataPacket * packet, time_t timestamp) {
  
  //Check that the correct cmd type was sent
  if (packet->cmd != CMD_GET_DATA) return -1;

  // Check that the response flag is set
  if (!(packet->ctrl & SMADATA_FLAG_RESPONSE)) return -1;

  // The user data section of the packet should be 73 bytes long, although this 
  // is most likely specifically for this inverter model
  if (packet->dataLength != 73) return -1;
  
  //check that this is the data for the requested timestamp
  time_t packetTime;
  memcpy(&packetTime, packet->data + TIMESTAMP_OFFSET, sizeof(time_t));
  
  if (packetTime != timestamp) {
    return -1;
  }
  
  // Copy the energy information
  unsigned long e_total;
  memcpy(&e_total, packet->data + E_TOTAL_OFFSET, sizeof(unsigned long));

  return e_total;  
  
}

// Reads 4 bytes from an array, assuming they are in little endian format and returns them as a long
unsigned long data2long(byte *array) {
  unsigned long result;

  result = array[0] & 0x000000FFL;
  result += (array[1] << 8) & 0x0000FF00L;
  result += (array[2] << 16) & 0x00FF0000L;
  result += (array[3] << 24) & 0xFF000000L;

  return result;
}
