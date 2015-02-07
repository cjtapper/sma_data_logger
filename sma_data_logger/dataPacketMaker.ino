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
#include <Time.h>

#define GROUP0_ADDR 0x0000
#define CFG_ADDR_OFFSET 4
#define CHANNEL_IDX_OFFSET 2

void makeCMD_GET_NET_START(SMADataPacket * packet, word src){
  packet->src = src;
  packet->dest = GROUP0_ADDR;
  packet->ctrl = SMADATA_FLAG_BROADCAST;
  packet->pktCnt = 0;
  packet->cmd = CMD_GET_NET_START;
  packet->dataLength = 0;

  memset(packet->data, 0, SMA_DATA_LENGTH); // clear the data field
}

void makeCMD_CFG_NET_ADDR(SMADataPacket * packet, word src, SMADevice * device){
  packet->src = src;
  packet->dest = GROUP0_ADDR;
  packet->ctrl = SMADATA_FLAG_BROADCAST;
  packet->pktCnt = 0;
  packet->cmd = CMD_CFG_NETADR;
  
  memset(packet->data, 0, SMA_DATA_LENGTH); // clear the data field
  
  // The data field of a CMD_CFG_NET_ADDR packet contains a 32 bit serial number
  // followed by a 16 bit network address
  long2data(device->serial, packet->data);
  word2data(device->addr, packet->data + CFG_ADDR_OFFSET);
  
  packet->dataLength = 6;
}

void makeCMD_SEARCH_DEV(SMADataPacket * packet, word src, SMADevice * device){
  packet->src = src;
  packet->dest = GROUP0_ADDR;
  packet->ctrl = SMADATA_FLAG_BROADCAST;
  packet->pktCnt = 0;
  packet->cmd = CMD_SEARCH_DEV;

  memset(packet->data, 0, SMA_DATA_LENGTH); // clear the data field
  
  // The data field of a CMD_SEARCH_DEV packet contains the 32 bit serial number
  // of the device.
  long2data(device->serial, packet->data);
  
  packet->dataLength = 4;
}

void makeCMD_SYN_ONLINE(SMADataPacket * packet, word src, SMADevice * device, time_t timestamp){
  packet->src = src;
  packet->dest = GROUP0_ADDR;
  packet->ctrl = SMADATA_FLAG_BROADCAST;
  packet->pktCnt = 0;
  packet->cmd = CMD_SYN_ONLINE;

  memset(packet->data, 0, SMA_DATA_LENGTH); // clear the data field

  // The data field should contain a unix timestamp
  long2data((unsigned long) timestamp, packet->data);

  packet->dataLength = 4;

}

void makeCMD_GET_DATA(SMADataPacket * packet, word src, SMADevice * device){
  packet->src = src;
  packet->dest = device->addr;
  packet->ctrl = 0;
  packet->pktCnt = 0;
  packet->cmd = CMD_GET_DATA;

  memset(packet->data, 0, SMA_DATA_LENGTH); // clear the data field

  // The data field contains a 3 byte transfer mask. The first 2 bytes are the
  // channel types, the 3rd byte is the channel index
  word channelType = 0x090F;
  byte channelIndex = 0;

  word2data(channelType, packet->data);
  packet->data[CHANNEL_IDX_OFFSET] = channelIndex;

  packet->dataLength = 3;
  
}

void long2data(unsigned long input, byte *array) {
  memcpy(array, &input, sizeof(input));
}

void word2data(word input, byte *array) {
  memcpy(array, &input, sizeof(input));
}
