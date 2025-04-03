/*
  Test.h - Test library for Wiring - description
  Copyright (c) 2006 John Doe.  All right reserved.
*/

// ensure this library description is only included once
#ifndef _bms_h

#define _bms_h

#define U1TXD  4
#define U1DIR  16
#define U1RXD  17

#define BMS_ID    0x01

#define VOLT_AMP_CMD        0x90
#define CHG_DISCHG_STATUS   0x93
#define STATUS_INFO         0x94



char calChecksum(char *buf , char len);
void SendDataToBMS(char cmd);
unsigned int RequestDataFromBMD(char cmd);

extern float fBattVolt,fBattCurrent,fBattSOC;
extern unsigned int ChargeStatus, BmsLife;
extern int BattCurrent;
extern unsigned long  BattCap; 

/*
extern "C" 
{	
	void DoSomeThing100();
	void DoSomeThing300();
  byte calChecksum(byte *buf , byte len);
  void SendDataToBMS(byte cmd);
}
*/


#endif
