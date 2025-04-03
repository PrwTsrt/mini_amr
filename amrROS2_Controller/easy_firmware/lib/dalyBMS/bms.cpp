

#include <Arduino.h>
#include "bms.h"


unsigned int BattVolt, BattSOC;
int BattCurrent;
unsigned int ChargeStatus, BmsLife;
unsigned long  BattCap;

float fBattVolt,fBattCurrent,fBattSOC;


void Init_Var()
{
  BattVolt = 0;
  BattCurrent = 0;
  BattSOC = 0;
  ChargeStatus = 0;
  BmsLife = 0;
}


char calChecksum(char *buf , char len)
{
  char i, res=0;

  for(i=0;i<len;i++)
  {
    res = res + buf[i];
  }
  return res;
}

void SendDataToBMS(char cmd)
{
  char dat_arr[13];
  //char arr_dat[5]={1,2,3,4,5};

 dat_arr[0] = 0xa5; dat_arr[1] = 0x40; 
 dat_arr[2] = cmd;  dat_arr[3] = 0x08;
 

  
 for(char i =4; i<12; i++)
 {
    dat_arr[i] = 0;
 }

 dat_arr[12] = calChecksum(dat_arr , 12);

digitalWrite(U1DIR, HIGH);
delayMicroseconds(100);
Serial2.write(dat_arr, 13);
// while(Serial2.availableForWrite()>0);
Serial2.flush();
digitalWrite(U1DIR, LOW);
 

}

unsigned int RequestDataFromBMD(char cmd)
{
  unsigned int req_sta;
  unsigned char len=0,chksum;
  uint32_t tmr_out;

  char Buf[50];

    Serial2.read();
    SendDataToBMS(cmd);
    delay(15);
     
    req_sta=0;
    Init_Var();
   
    while(Serial2.available()==0 && tmr_out++ <100000UL);

    //if( Serial2.available() > 0 )
    if( 1 )
    {
     
        len = Serial2.readBytes(Buf, 50);      
        // prints the received data
        chksum = calChecksum(Buf , len-1);
    
      //  /*
        if(chksum == Buf[len-1]) // if header don't match, return zero value
        {
          if(Buf[2] == 0x90)
          {
            
           // /*
            BattVolt =  ((unsigned int)Buf[4]<<8) | Buf[5];
            fBattVolt = BattVolt * 0.1;
           // Serial.println(Buf[4], HEX);
            //Serial.println(Buf[5], HEX);

            BattCurrent =  ((int)Buf[8]<<8) | Buf[9];
            fBattCurrent = (BattCurrent - 30000) * 0.1;
            //Serial.println(Buf[8], HEX);
            //Serial.println(Buf[9], HEX);

            BattSOC =  ((unsigned int)Buf[10]<<8) | Buf[11];
            fBattSOC = BattSOC * 0.1;
           // Serial.println(Buf[10], HEX);
           // Serial.println(Buf[11], HEX);
           req_sta =1;
        
          }
          else if(Buf[2] == 0x93)
          {
            ChargeStatus = Buf[4];
            BmsLife =  Buf[7];
            BattCap =  Buf[8]<<24 | Buf[9]<<16 | Buf[10]<<8 | Buf[11];
            req_sta =1;
          
          }               
        }
      //  */
           
    }
    //Serial.println(req_sta);    
    return req_sta;
       
}


void DoSomeThing100()
{
	digitalWrite(2,HIGH);
	delay(100);
	digitalWrite(2,LOW);
	delay(100);
}


void DoSomeThing300()
{
	digitalWrite(2,HIGH);
	delay(300);
	digitalWrite(2,LOW);
	delay(300);
}
