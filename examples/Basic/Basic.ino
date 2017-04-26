/*********************************************************************
 * Basic functions to control train - V1.0 - 22.4.2017
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
 */

#include <Raildue.h>

const word BLS  = ADDR_MM2 + 25;  // Adapt to your device
word Loc;
word Speed;
byte Dir;
boolean Light;
boolean Power;

const int  Turn_Delay = 500;    // min value = 300 ms 

Raildue cmd(0x2f34);  

void setup() {
  int j;
  Loc=BLS;
  Speed=0;
  Dir=1;
  Light=false;

  cmd.Init();
  cmd.Start();
  Power = true;
  
  Serial.begin(115200);
  Serial.println(" p=Power(ON-off) / d=Direction / l=Light / +=Accelerate / -=Brake / t=TurnTrue / f=TurnFalse");
}

void loop() {
    byte b;

  if (Serial.available()) {
    b = Serial.read();
    switch (b) {
      case 'p': if (Power) {cmd.Stop();Serial.print(" PowerOFF");Power=false;} else {cmd.StartMFX();Serial.print(" PowerON");Power=true;}  break;
      case 'a': if (Speed < 600) {Speed = Speed + 100; cmd.LocSpeed(Loc,Speed);} break;
      case 'b': if (Speed != 0)  {Speed = Speed - 100; cmd.LocSpeed(Loc,Speed);} break;
      case 'd': if (Dir == 1) {Dir = 2;} else {Dir = 1;} cmd.LocDir(Loc,Dir); break;
      case 'l': if (Light) {Light=false; cmd.LocFunction(Loc,0,0);} else {Light=true;cmd.LocFunction(Loc,0,1);} break;
      case 't': cmd.SetT(9,true, Turn_Delay); break;
      case 'f': cmd.SetT(9,false,Turn_Delay); break;
    }
  }
}

