/*
 *  This source code has been modified by Christopher Tapper to be used in the
 *  SMA Data Logger project. The original author details and license are below:
 *
 *  ###############################################################################
 *
 *  Description: This example shows how to upload and download files from a FTP 
 *  server. The example configures the module to use FTP funtions, uploads a file 
 *  to the FTP server and then downloads the content of the uploaded file and shows
 *  it. This example only shows the AT commands (and the answers of the module) 
 *  used to use the FTP funtion and how work the FTP functions. For more 
 *  information about the AT commands, refer to the AT command manual.
 *
 *  Copyright (C) 2013 Libelium Comunicaciones Distribuidas S.L.
 *  http://www.libelium.com
 *
 *  This program is free software: you can redistribute it and/or modify 
 *  it under the terms of the GNU General Public License as published by 
 *  the Free Software Foundation, either version 3 of the License, or 
 *  (at your option) any later version. 
 *  
 *  This program is distributed in the hope that it will be useful, 
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of 
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the 
 *  GNU General Public License for more details. 
 *  
 *  You should have received a copy of the GNU General Public License 
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>. 
 *
 *  Version 0.2
 *  Author: Alejandro Gallego 
 *
 */

int8_t answer;

char OK[] = "OK";

void configure_FTP(){

    sendATcommand(F("AT+SAPBR=3,1,\"CONTYPE\",\"GPRS\""), OK, 2000);
    sendATcommand(F("AT+SAPBR=3,1,\"APN\",\"yesinternet\""), OK, 2000);
    sendATcommand(F("AT+SAPBR=3,1,\"USER\",\"\""), OK, 2000);
    sendATcommand(F("AT+SAPBR=3,1,\"PWD\",\"\""), OK, 2000);

    while (sendATcommand(F("AT+SAPBR=1,1"), OK, 20000) != 1);
    sendATcommand(F("AT+FTPCID=1"), OK, 2000);
    sendATcommand(F("AT+FTPTYPE=\"A\""), OK, 2000);
    sendATcommand(F("AT+FTPSERV=\"184.168.189.1\""), OK, 2000);
    sendATcommand(F("AT+FTPPORT=21"), OK, 2000);
    sendATcommand(F("AT+FTPUN=\"solarinverter1\""), OK, 2000);
    sendATcommand(F("AT+FTPPW=\"So1@rProj\""), OK, 2000);


}

void uploadFTP(char * data, int length){
    sendATcommand(F("AT+FTPPUTNAME=\"datalog.txt\""), OK, 2000);
    sendATcommand(F("AT+FTPPUTPATH=\"/\""), OK, 2000);
    sendATcommand(F("AT+FTPPUTOPT=\"APPE\""), OK, 2000);
    
    //we need to construct these commands because they can vary depending on the length of the data string
    String response = "+FTPPUT=2," + String(length);
    String command = "AT" + response;
    
    char commandBuffer[command.length()+1]; //have to add 1 for space for the string terminator
    char responseBuffer[response.length()+1]; //have to add 1 for space for the string terminator
    
    command.toCharArray(commandBuffer, command.length()+1); //have to add 1 for space for the string terminator
    response.toCharArray(responseBuffer, response.length()+1); //have to add 1 for space for the string terminator

    if (sendATcommand(F("AT+FTPPUT=1"), "+FTPPUT:1,1,", 30000) == 1)
    {
      if (sendATcommandChar(commandBuffer, responseBuffer, 30000) == 1){
        Serial.println(sendATcommandChar(data, "+FTPPUT:1,1", 30000),DEC);
        Serial.println(sendATcommand(F("AT+FTPPUT=2,0"), OK, 30000),DEC);
        Serial.println(F("FTP upload complete"));
      } else {
        Serial.println(F("Error sending FTP data"));
      }
    } else {
        Serial.println(F("Error opening the FTP session"));
    }
}




void gsmPowerOn(){
  
  // Configure pins
  pinMode(3, OUTPUT);
  pinMode(4, OUTPUT);
  pinMode(5, OUTPUT);
  
  //set gsm timing
  digitalWrite(5, HIGH);
  delay(1500);
  digitalWrite(5, LOW);
  
  digitalWrite(3, LOW); //enable GSM
  digitalWrite(4, HIGH); //disable GPS
    uint8_t answer=0;
    
    // checks if the module is started
    answer = sendATcommand(F("AT"), OK, 2000);
    if (answer == 0)
    {
    
        while(answer == 0){     // Send AT every two seconds and wait for the answer
            answer = sendATcommand(F("AT"), OK, 2000);    
        }
    }
  sendATcommand(F("AT+CLTS=1"), OK, 2000); //enable collection of timestamps
}

uint8_t setArduinoTime(unsigned int timeout) {
  uint8_t x=0,  answer=0;
    char response[100];
    unsigned long previous;

    memset(response, '\0', 100);    // Initialize the string

    delay(100);

    while( gsmSerial.available() > 0) gsmSerial.read();    // Clean the input buffer

    gsmSerial.println(F("AT+CCLK?"));    // Send the AT command

    x = 0;
    previous = millis();

    // this loop waits for the answer
    do{
        if(gsmSerial.available() != 0){    
            // if there are data in the UART input buffer, reads it and checks for the asnwer
            response[x] = gsmSerial.read();
            //Serial.print(response[x]);
            x++;
            // check if the desired answer  is in the response of the module
            if (strstr(response, "+CCLK: \"") != NULL)    
            {
                answer = 1;
                while (gsmSerial.available() < 17); //spin until all characters are in the buffer
                int xyear = 2000 + (gsmSerial.read()-48)*10 + gsmSerial.read()-48;
                gsmSerial.read(); //discard this character
                int xmonth = (gsmSerial.read()-48)*10 + gsmSerial.read()-48;
                gsmSerial.read(); //discard this character
                int xday = (gsmSerial.read()-48)*10 + gsmSerial.read()-48;
                gsmSerial.read(); //discard this character
                int xhours = (gsmSerial.read()-48)*10 + gsmSerial.read()-48;
                gsmSerial.read(); //discard this character
                int xminutes = (gsmSerial.read()-48)*10 + gsmSerial.read()-48;
                gsmSerial.read(); //discard this character
                int xseconds = (gsmSerial.read()-48)*10 + gsmSerial.read()-48;
  
                
                setTime(xhours, xminutes, xseconds, xday, xmonth, xyear);
                
            }
        }
    }
    // Waits for the asnwer with time out
    while((answer == 0) && ((millis() - previous) < timeout));    

        return answer;
}

int8_t sendATcommand(const __FlashStringHelper* ATcommand, char* expected_answer, unsigned int timeout){

    uint8_t x=0,  answer=0;
    char response[100];
    unsigned long previous;

    memset(response, '\0', 100);    // Initialize the string

    delay(100);

    while( gsmSerial.available() > 0) gsmSerial.read();    // Clean the input buffer

    gsmSerial.println(ATcommand);    // Send the AT command
    Serial.println(ATcommand);


        x = 0;
    previous = millis();

    // this loop waits for the answer
    do{
        if(gsmSerial.available() != 0){    
            // if there are data in the UART input buffer, reads it and checks for the asnwer
            response[x] = gsmSerial.read();
            //Serial.print(response[x]);
            x++;
            // check if the desired answer  is in the response of the module
            if (strstr(response, expected_answer) != NULL)    
            {
                answer = 1;
            }
        }
    }
    // Waits for the asnwer with time out
    while((answer == 0) && ((millis() - previous) < timeout));    

        return answer;
}

int8_t sendATcommandChar(char* ATcommand, char* expected_answer, unsigned int timeout){

    uint8_t x=0,  answer=0;
    char response[100];
    unsigned long previous;

    memset(response, '\0', 100);    // Initialize the string

    delay(100);

    while( gsmSerial.available() > 0) gsmSerial.read();    // Clean the input buffer

    gsmSerial.println(ATcommand);    // Send the AT command
    Serial.println(ATcommand);


        x = 0;
    previous = millis();

    // this loop waits for the answer
    do{
        if(gsmSerial.available() != 0){    
            // if there are data in the UART input buffer, reads it and checks for the asnwer
            response[x] = gsmSerial.read();
            //Serial.print(response[x]);
            x++;
            // check if the desired answer  is in the response of the module
            if (strstr(response, expected_answer) != NULL)    
            {
                answer = 1;
            }
        }
    }
    // Waits for the asnwer with time out
    while((answer == 0) && ((millis() - previous) < timeout));    

        return answer;
}
