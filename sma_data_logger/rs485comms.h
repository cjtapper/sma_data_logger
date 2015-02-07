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

#ifndef RS485COMMS_H_
#define RS485COMMS_H_

#include "Arduino.h"
#include "sma_defs.h"

#define RS485_ENABLE 2

//Some constants for SMA Net messages
#define SMA_NET_START_STOP 0x7E
#define SMA_DATA_PROTOCOL_NUMBER 0x4041
#define SMA_NET_CTRL 0x03
#define SMA_NET_ADDR 0xFF

//Byte offsets for parsing SMA Net Packets
#define SMA_NET_ADDR_BYTE 0
#define SMA_NET_CTRL_BYTE 1

//The following are the byte offsets used for parsing a packet from the buffer
#define SRC_ADDR_LSB 0
#define SRC_ADDR_MSB 1
#define DEST_ADDR_LSB 2
#define DEST_ADDR_MSB 3
#define SMA_DATA_CTRL 4
#define SMA_DATA_PKT_CNT 5
#define SMA_DATA_CMD 6
#define SMA_DATA 7
        
#define SMA_DATA_HEADER_LENGTH 7

#define SMA_NET_HEADER_LENGTH 4 //doesn't include the start byte
#define SMA_NET_CHECKSUM_LENGTH 2
#endif
