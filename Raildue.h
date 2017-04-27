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

#ifndef Raildue_h
#define Raildue_h

#include "variant.h"
#include <due_can.h>

#define ADDR_MM2     0x0000 // MM2 locomotive (from Railuino)
#define ADDR_MFX     0x4000 // MFX locomotive
#define ADDR_DCC     0xC000 // DCC locomotive
#define ADDR_ACC_MM2 0x2FFF // MM2 magnetic accessory

class Raildue
{						
	public:
		Raildue(int Hash);
           void Init();
		void Start();    // MM2 + DCC protocols
           void StartMFX(); // MFX + MM2 + DCC protocols
           void BindMFX(byte MFX1, byte MFX2, byte MFX3, byte MFX4, byte UID);
           void Stop();
		void LocSpeed(word LocAdr, word Speed);
		void LocDir(word LocAdr, byte dir);
		void LocFunction(word LocAdr, byte function, byte value);
		void SetT(int Device, boolean Dir, int Pause);
           void ControlMFX(boolean Sniffer);  // mandatory for MFX loc
	private:
		int CAN_Id;
		byte CAN_Len;
		byte CAN_Command;
		word CAN_Hash;
		unsigned char TxBuf[10];
		void Clear();
		void CAN_Send();
		void SetTurnHW(word address, byte value);
           void printHex(int num, int precision);
};

#endif