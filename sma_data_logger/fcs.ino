/*
/*
 *  This source code has been modified by Christopher Tapper to be used in the
 *  SMA Data Logger project. The original author details and license are below:
 *
 *  ###############################################################################
 *
 *
 *      YASDI - (Y)et (A)nother (S)MA(D)ata (I)mplementation
 *      Copyright(C) 2001-2008 SMA Solar Technology AG
 *
 *      This library is free software; you can redistribute it and/or
 *      modify it under the terms of the GNU Lesser General Public
 *      License as published by the Free Software Foundation; either
 *      version 2.1 of the License, or (at your option) any later version.
 * 
 *      This library is distributed in the hope that it will be useful,
 *      but WITHOUT ANY WARRANTY; without even the implied warranty of
 *      MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *      Lesser General Public License for more details.
 * 
 *      You should have received a copy of the GNU Lesser General Public
 *      License along with this library in the file COPYING.LIB;
 *      if not, write to the Free Software Foundation, Inc.,
 *      59 Temple Place - Suite 330, Boston, MA 02111-1307, USA
 *
 */
 
#include "Arduino.h"
#include <EEPROM.h>

//Calculate the checksum using a byte or array of bytes and the lookup table
word calculateFCS(word fcs, byte * data, word length) {
   
  while(length--) { 
    fcs = (word)((fcs >> 8) ^ fcstab((fcs ^ *data++) & 0xff)); 
  } 
   return (fcs);
}

//Calculate the checksum using a 2 byte Word and the lookup table
word fcsWord(word fcs, word data) {
  byte msb = (byte) ((data >> 8) & 0xFF);
  byte lsb = (byte) (data & 0xFF);
  
  fcs = calculateFCS(fcs, &msb, 1);
  fcs = calculateFCS(fcs, &lsb, 1);
  return fcs;
}

// The fcstab array (the lookup table) is now stored in the EEPROM. This function
// returns the value that would have been stored at index
word fcstab(int index) {
  return EEPROMreadWord(index*2);
}

word EEPROMreadWord(int addr) {
  //reads a little endian word stored in the EEPROM at addr
  return ((EEPROM.read(addr+1) << 8) + EEPROM.read(addr));
}
