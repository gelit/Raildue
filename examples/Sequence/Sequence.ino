/*********************************************************************
 * Sequence with 2 MFX locomotives & 4 fixed turns - V1.0 - 22.4.2017
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
#include <DueTimer.h>

const byte SID1 = 1; // Schienen ID
const byte SID2 = 2;

const word Loc1 = ADDR_MFX + SID1; // DB Loc
const word Loc2 = ADDR_MFX + SID2; // Cargo Loc

boolean Power, Sequence;        // User Interface
word Status;                    // State machine

const int  Turn_Delay = 500;    // min value = 300 ms 

const int S88_Nb = 32;          // 2 modules Littfinski RM-88-N-Opto --> 32 inputs
const int S88_Interval = 20000; // Interrupt period = 20 ms
const int Limit = 5;            // Change R value if 5 identical successive logic states of P value
boolean   R[S88_Nb+1];          // Register (R0 not used)
boolean   P[S88_Nb+1];          // Physical
int       M[S88_Nb+1];          // Memory

const int DATA = A0, CLOCK = 8, LOAD = 3, RESET = 4, TIME = 50;    // Arduino pin to S88 bus

Raildue cmd(0x2f34);  

void setup() {
  int j;

  cmd.Init(); 
  cmd.StartMFX(); 
  Power = true;
  cmd.BindMFX(0x7c,0xFE,0xAC,0x27,SID1); cmd.LocSpeed(Loc1,0);
  cmd.BindMFX(0x7D,0xFA,0x5A,0xC3,SID2); cmd.LocSpeed(Loc2,0);

  cmd.SetT(1,false,Turn_Delay); cmd.SetT(8,true,Turn_Delay); cmd.SetT(9,false,Turn_Delay); cmd.SetT(14,true,Turn_Delay);  //Turns
  
  pinMode(DATA, INPUT); pinMode(CLOCK, OUTPUT); pinMode(LOAD, OUTPUT); pinMode(RESET, OUTPUT);    // S88
  for (j=1;j<=S88_Nb;j++) {M[j]=0;}                                         // antiglitch
  Timer3.attachInterrupt(S88_Interrupt); 
  Timer3.start(S88_Interval);
  
  Serial.begin(115200);
  Serial.println(" p=Power(ON-off) / s=Sequence(on-OFF) / r=Retro ");

  Sequence = false; Status=1;
}

void S88_Interrupt() {  // Duration = 1.7 ms for 32 inputs
  int j,k;

  j=1;                                                        
  digitalWrite(LOAD, HIGH);     delayMicroseconds(TIME);  //  from Railuino
  digitalWrite(CLOCK, HIGH);    delayMicroseconds(TIME);
  digitalWrite(CLOCK, LOW);     delayMicroseconds(TIME);
  digitalWrite(RESET, HIGH);    delayMicroseconds(TIME);
  digitalWrite(RESET, LOW);     delayMicroseconds(TIME);
  digitalWrite(LOAD, LOW);      delayMicroseconds(TIME / 2);
  P[j] = digitalRead(DATA);j++;  delayMicroseconds(TIME / 2);
  for (k=1; k<=S88_Nb; k++) {
    digitalWrite(CLOCK, HIGH);   delayMicroseconds(TIME);
    digitalWrite(CLOCK, LOW);    delayMicroseconds(TIME / 2);
    P[j] = digitalRead(DATA);j++;delayMicroseconds(TIME / 2);
  }
  for (j=1;j<=S88_Nb;j++) {if (P[j] != R[j]) {M[j]++; if (M[j] == Limit) {M[j]=0; R[j]=P[j];}} else {M[j]=0;}}      // antiglitch
}

void loop() {
  byte b;
  int j;

  cmd.ControlMFX(false);  // Mandatory with MFX loc (no Sniffer)

  switch (Status) {
    case 1 : if (Sequence) {cmd.LocSpeed(Loc1,800); cmd.LocFunction(Loc1,0,1);  Status=2;}  break;
    case 2 : if (R[27]==1) {cmd.LocSpeed(Loc1,400); cmd.LocFunction(Loc1,0,0);  Status=3;}  break;
    case 3 : if (R[10]==1) {cmd.LocSpeed(Loc1,0);                               Status=4;}  break;
    case 4 :                cmd.LocSpeed(Loc2,600); cmd.LocFunction(Loc2,0,1);  Status=5;   break;
    case 5 : if (R[20]==1) {cmd.LocSpeed(Loc2,300); cmd.LocFunction(Loc2,0,0);  Status=6;}  break;
    case 6 : if (R[5] ==1) {cmd.LocSpeed(Loc2,0);                               Status=7;}  break;
    case 7 :                cmd.LocSpeed(Loc1,800); cmd.LocFunction(Loc1,0,1);  Status=8;   break;
    case 8 : if (R[6] ==1) {cmd.LocSpeed(Loc1,400); cmd.LocFunction(Loc1,0,0);  Status=9;}  break;
    case 9 : if (R[18]==1) {cmd.LocSpeed(Loc1,0);                               Status=10;} break;
    case 10:                cmd.LocSpeed(Loc2,600); cmd.LocFunction(Loc2,0,1);  Status=11;  break;
    case 11: if (R[12]==1) {cmd.LocSpeed(Loc2,300); cmd.LocFunction(Loc2,0,0);  Status=12;} break;
    case 12: if (R[28] ==1){cmd.LocSpeed(Loc2,0);                               Status=1;}  break;
  }

  if (Serial.available()) {
    b = Serial.read();
    switch (b) {
      case 'p': if (Power) {cmd.Stop();Serial.print(" PowerOFF");Power=false;} else {cmd.StartMFX();Serial.print(" PowerON");Power=true;}        break;
      case 's': if (Sequence) {Sequence=false;Serial.print(" SequenceOFF");} else {Sequence=true;Serial.print(" SequenceON");}                   break;
      case 'r': Serial.println(); Serial.print("Retro ="); for (j=1; j<=S88_Nb+1; j++) {if (R[j] == true) {Serial.print(" R");Serial.print(j);}} break;
    }
  }
}

