/*********************************************************************
 * Display UID (Universal ID) - V1.0 - 22.4.2017
 *
 * Copyright (C) 2017 Gerald Litzistorf
 * 
 * This example is free software; you can redistribute it and/or
 * modify it under the terms of the Creative Commons Zero License,
 * version 1.0, as published by the Creative Commons Organisation.
 * This effectively puts the file into the public domain.
 *
 * This example is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.   See the
 * LICENSE file for more details.
 * 
 * Thanks to http://desktopstation.net/mfx_identification
 */

#include <due_can.h>

CAN_FRAME CAN0_Tx;

int  CAN_Id;
byte CAN_Len;
byte CAN_Command;
word CAN_Hash;

unsigned char TxBuf[20];

void printHex(int num, int precision) {
     char tmp[16];
     char format[128];

     sprintf(format, "%%.%dX", precision);
     sprintf(tmp, format, num);
     Serial.print(tmp);
}

void printFrame(CAN_FRAME *frame) {
  Serial.print(".");
  if (((frame->id >> 17) == 1) && (frame->length == 5)) {
    Serial.println();
    Serial.print(" UID = "); 
    printHex(frame->data.bytes[0],2); Serial.print(" ");
    printHex(frame->data.bytes[1],2); Serial.print(" ");
    printHex(frame->data.bytes[2],2); Serial.print(" ");
    printHex(frame->data.bytes[3],2); Serial.print(" ");
    Serial.println();
  }
}

void CAN_Send() {  
  CAN_Id = ((uint32_t)CAN_Command) << 17 | (uint32_t)CAN_Hash;
  CAN0_Tx.id = CAN_Id;
  CAN0_Tx.extended = 1;
  CAN0_Tx.length = CAN_Len;
  CAN0_Tx.data.bytes[0] = TxBuf[8];
  CAN0_Tx.data.bytes[1] = TxBuf[9];
  CAN0_Tx.data.bytes[2] = TxBuf[2];
  CAN0_Tx.data.bytes[3] = TxBuf[3];
  CAN0_Tx.data.bytes[4] = TxBuf[4];
  CAN0_Tx.data.bytes[5] = TxBuf[5];
  CAN0_Tx.data.bytes[6] = TxBuf[6];
  CAN0_Tx.data.bytes[7] = TxBuf[7];
  Can0.sendFrame(CAN0_Tx);
}

void Clear() {for(int j=0; j<10; j++) {TxBuf[j]=0;}}

void SystemGo() {
  Clear(); CAN_Command = 0; CAN_Len = 7; TxBuf[4] = 9; TxBuf[6] = 0xd; CAN_Send(); delay(200);
  Clear(); CAN_Command = 0; CAN_Len = 6; TxBuf[4] = 8; TxBuf[5] = 7;   CAN_Send(); delay(200);
  Clear(); CAN_Command = 0; CAN_Len = 5; TxBuf[4] = 1;                 CAN_Send(); delay(1000);
}

void SystemStop() {
  Clear(); CAN_Command = 0; CAN_Len = 5;                               CAN_Send();
}

void setup() {
  Serial.begin(115200);

  CAN_Hash = 0x2f39;                   //"random" value
  Can0.begin(CAN_BPS_250K);            //Maerklin baudrate  
  Can0.watchFor();                     //Watch for all traffic
  Can0.setRXFilter(0, 0, false);       //catch all mailbox, no filter on the monitored traffic.
  Can0.setGeneralCallback(printFrame);
  
  Clear(); CAN_Command = 0x1b; CAN_Len = 5; TxBuf[4] = 0x11; CAN_Send(); delay(1000); // Railuino:Controller-begin = Maerklin mystery

  SystemGo();
  
  Serial.println("type char to exit");
}

void loop() {
  byte b;

  if (Serial.available()) {
    b = Serial.read(); 
    SystemStop();
  }
}
