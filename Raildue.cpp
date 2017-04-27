/*********************************************************************
* Raildue - Control your train with Arduino Due - V1.0 - 22.4.2017
*
* Copyright (C) 2017 Gerald Litzistorf
*
* This library is free software; you can redistribute it and/or
* modify it under the terms of the GNU Lesser General Public
* License as published by the Free Software Foundation; either
* version 2.1 of the License, or (at your option) any later version.
*
* This library is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* LICENSE file for more details.
*/

#include "Raildue.h"

CAN_FRAME CAN0_Tx, CAN0_Rx;

boolean Send;

Raildue::Raildue(int Hash)
{ 
	CAN_Hash = Hash; 
	Send = false;
}

void Raildue::Init() {
	Can0.begin(CAN_BPS_250K);                                                           delay(1000);  // Maerklin baudrate
	Can0.watchFor();               //Watch for all traffic
	Can0.setRXFilter(0, 0, false); //catch all mailbox, no filter on the monitored traffic.
	
	Clear(); CAN_Command = 0x1b; CAN_Len = 5; TxBuf[4] = 0x11;              CAN_Send(); delay(1000);  // Railuino:Controller-begin = Maerklin mystery
}

void Raildue::Start() {
	Clear(); CAN_Command = 0;    CAN_Len = 5; TxBuf[4] = 1;                 CAN_Send(); delay(1000);  // System-Go p13
}

void Raildue::StartMFX() {
	Clear(); CAN_Command = 0;    CAN_Len = 7; TxBuf[4] = 9; TxBuf[6] = 0xd; CAN_Send(); delay(2);
	Clear(); CAN_Command = 0;    CAN_Len = 6; TxBuf[4] = 8; TxBuf[5] = 7;   CAN_Send(); delay(2);
	Clear(); CAN_Command = 0;    CAN_Len = 5; TxBuf[4] = 1;                 CAN_Send(); delay(1000);  // System-Go p13
}

void Raildue::BindMFX(byte MFX1, byte MFX2, byte MFX3, byte MFX4, byte UID) {
	Clear(); CAN_Command = 2;    CAN_Len = 6; TxBuf[8] = MFX1; TxBuf[9] = MFX2; TxBuf[2] = MFX3; TxBuf[3] = MFX4; TxBuf[4] = 0; TxBuf[5] = UID; CAN_Send(); delay(300);  // MFX Bind p28
}

	
void Raildue::ControlMFX(boolean Sniffer) {
	if (Send) { CAN_Send(); Send = false; }
	if (Can0.rx_avail()) {
		Can0.read(CAN0_Rx);
		if (Sniffer) {
			if ((CAN0_Rx.id >> 16) == 1) {
				Serial.print("                                             ");  // Response
			}
			//Time2=millis(); Delta=Time2-Time1; Serial.print(Delta); Time1=Time2; Serial.print("\t");
			Serial.print(" Cmd= ");  Serial.print((CAN0_Rx.id >> 17), HEX);
			if ((CAN0_Rx.id >> 17) < 0x10) { Serial.print(" "); }
			Serial.print(" Data= ");
			for (int count = 0; count < CAN0_Rx.length; count++) { printHex(CAN0_Rx.data.bytes[count], 2); Serial.print(" "); }
			Serial.print("\r\n");
		}

		if (((CAN0_Rx.id >> 17) == 2) && (CAN0_Rx.length == 6)) {
			Clear();
			CAN_Command = 3;
			CAN_Len = 6;
			TxBuf[8] = (CAN0_Rx.data.bytes[0]);
			TxBuf[9] = (CAN0_Rx.data.bytes[1]);
			TxBuf[2] = (CAN0_Rx.data.bytes[2]);
			TxBuf[3] = (CAN0_Rx.data.bytes[3]);
			TxBuf[4] = (CAN0_Rx.data.bytes[4]);
			TxBuf[5] = (CAN0_Rx.data.bytes[5]);
			Send = true;  //p29 SetMfxVerify()
		}
	}
}

void Raildue::printHex(int num, int precision) {
	char tmp[16];
	char format[128];

	sprintf(format, "%%.%dX", precision);
	sprintf(tmp, format, num);
	Serial.print(tmp);
}

void Raildue::Stop() {
	Clear(); CAN_Command = 0;    CAN_Len = 5;                               CAN_Send(); delay(1000);  // System-Stopp p12
}

void Raildue::Clear() {for (int i=0; i<10; i++) {TxBuf[i]=0;}}

void Raildue::CAN_Send() { 
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

void Raildue::LocSpeed(word LocAdr, word Speed) {
	Clear();
	CAN_Command = 4;
	CAN_Len = 6;
	TxBuf[2] = highByte(LocAdr);
	TxBuf[3] = lowByte(LocAdr);
	TxBuf[4] = highByte(Speed);
	TxBuf[5] = lowByte(Speed);
	CAN_Send();
}

void Raildue::LocDir(word LocAdr, byte dir) { //p31
	Clear();
	CAN_Command = 0;
	CAN_Len = 5;
	TxBuf[2] = highByte(LocAdr);
	TxBuf[3] = lowByte(LocAdr);
	TxBuf[4] = 3;
	CAN_Send();                   
	Clear();
	CAN_Command = 5;
	CAN_Len = 5;
	TxBuf[2] = highByte(LocAdr);
	TxBuf[3] = lowByte(LocAdr);
	TxBuf[4] = dir;
	CAN_Send();
}

void Raildue::LocFunction(word LocAdr, byte function, byte value) {  //p32
	Clear();
	CAN_Command = 6;
	CAN_Len = 6;  //Aktivieren
	TxBuf[2] = highByte(LocAdr);
	TxBuf[3] = lowByte(LocAdr);
	TxBuf[4] = function;
	TxBuf[5] = value;
	CAN_Send();
}

void Raildue::SetTurnHW(word address, byte value) {  //p37
	Clear();
	CAN_Command = 0x0b;
	CAN_Len = 6;  //Senden
	TxBuf[2] = highByte(address);
	TxBuf[3] = lowByte(address);
	TxBuf[4] = value;
	TxBuf[5] = 1;  //Einschalten
	CAN_Send();
}

void Raildue::SetT(int Device, boolean Dir, int Pause) {
	word ADR;

	ADR = Device + ADDR_ACC_MM2;
	SetTurnHW(ADR, Dir);  
	delay(Pause);	// danger car appel bloquant
}