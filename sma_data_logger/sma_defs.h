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
#include "Arduino.h"

#ifndef SMA_DEFS_H_
#define SMA_DEFS_H_

typedef struct SMADataPacket {
  word src;
  word dest;
  byte ctrl;
  byte pktCnt;
  byte cmd;
  byte data[255];
  int dataLength;
} SMADataPacket;

typedef struct SMADevice {
  unsigned long serial;
  char deviceType[9]; // device type is actually only 8 chars, but we need the extra slot for the string terminator
  word addr;
} SMADevice;

#define SMADATA_FLAG_BROADCAST 0b10000000
#define SMADATA_FLAG_RESPONSE  0b01000000
#define SMADATA_FLAG_GATEWAY_BLOCK 0b00010000
#define SMA_DATA_LENGTH 255

#define CMD_GET_NET_START       6
#define CMD_SEARCH_DEV		2   	
#define CMD_CFG_NETADR		3
#define CMD_SYN_ONLINE          10
#define CMD_GET_DATA            11

#endif

