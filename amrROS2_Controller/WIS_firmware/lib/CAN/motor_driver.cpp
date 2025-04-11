/*
  CANOpen source code (V1.0) for comunication with L12 dual BLDC driver eMR motor(TeensyMicromod).
  Created: 2024-03-01
  Modified: 2024-03-01
  By: Udom K.(NECTEC)
*/
//#include <ModbusMaster.h>
//#include <ModbusSlave.h>
// #include <JY901.h>
#include <Arduino.h>
#include <TimerOne.h>
//#include <ODriveArduino.h>
#include <stdio.h>
#include <FlexCAN_T4.h>           // https://github.com/tonton81/FlexCAN_T4
#include <TimerOne.h>

#define DLC                       8   


/* eMR COB-ID */
#define NMT_COBID                 0x000
#define EMERG_COBID               0x080
#define BOOTUP_COBID              0x700
#define TSDO_COBID                0x600   
#define RSDO_COBID                0x580            
#define TPDO1_COBID               0x200     
#define TPDO2_COBID               0x300 
#define TPDO3_COBID               0x400 
#define TPDO4_COBID               0x500 
#define RPDO1_COBID               0x180       
#define RPDO2_COBID               0x280     
#define RPDO3_COBID               0x380     
#define RPDO4_COBID               0x480     

#define SDO_Expedited_1           0x2F
#define SDO_Expedited_2           0x2B
#define SDO_Expedited_3           0x27
#define SDO_Expedited_4           0x23
#define RSDO_Expedited_4          0x40
#define SDO_Error_Msg             0x80

/* eMR Network Management Command */
#define NMT_Operation             0x01            // Switch to the "Operational" state
#define NMT_Stop                  0x02            // Switch to the "Stop" state
#define NMT_Query                 0x03            // Query NMT state command
#define NMT_PreOperation          0x80            // Switch to the "Pre-Operational" state
#define NMT_Reset_Node            0x81            // Reset Node
#define NMT_Reset_communiction    0x82            // Reset Communication



#define ReceiveTimeOut            1000            // Maximum for waiting answer from eMR in milli second
#define Acceleration              5000            // rpm/s^2 
#define Deceleration              5000            // rpm/s^2 
#define MaxVelocity               160             // rpm/s      // maxspeed eMR motor hub
#define ProfileQuickDecel         10000           // rpm/s^2  




// CiA402 CANOpen eMR Object Dictionary 
#define Controlword_Obj           0x6040
#define Statusword_Obj            0x6041
#define OperationMode_Obj         0x6060
#define MotionProfileType_Obj     0x6086
#define TargetPosition_Obj        0x607A
#define DemandPosition_Obj        0x6062
#define TargetVelocity_Obj        0x60FF
#define DemandVelocity_Obj        0x606B
#define ProfileVelocity_Obj       0x6081
#define MaxProfileVelocity_Obj    0x607F
#define MaxMotorSpeed_Obj         0x6080
#define MaxGearInputSpeed_Obj     0x3003
#define MaxAcceleration_Obj       0x60C5
#define ProfileAcceleration_Obj   0x6083
#define ProfileDeceleration_Obj   0x6084
#define ActualPosition_Obj        0x6062
//#define ActualVelocity_Obj        0x606B                      // Maxon
#define ActualVelocity_Obj        0x6069                        // eMR Motor hub
#define PositionWindow_Obj        0x6067
#define SoftwarePositionLimit_Obj 0x607D
#define HomingMethod_Obj          0x6098
#define HomingSpeed_Obj           0x6099
#define HomingAcceleration_Obj    0x609A
#define HomeOffsetMove_Obj        0x30B1
#define HomePosition              0x30B0
#define HomingCurrentThreshold    0x30B2

/* eMR Statusword */
#define eMR_BIT15               0x8000          // bit code: position referenced to home position
#define eMR_BIT14               0x4000          // bit code: refresh cycle of power stage
#define eMR_BIT13               0x2000          // bit code: OpMode specific, some error
#define eMR_BIT12               0x1000          // bit code: OpMode specific
#define eMR_BIT11               0x0800          // bit code: NOT USED
#define eMR_BIT10               0x0400          // bit code: Target reached
#define eMR_BIT09               0x0200          // bit code: Remote (?)
#define eMR_BIT08               0x0100          // bit code: offset current measured (?)
#define eMR_BIT07               0x0080          // bit code: WARNING
#define eMR_BIT06               0x0040          // bit code: switch on disable
#define eMR_BIT05               0x0020          // bit code: quick stop
#define eMR_BIT04               0x0010          // bit code: voltage enabled
#define eMR_BIT03               0x0008          // bit code: FAULT
#define eMR_BIT02               0x0004          // bit code: operation enable
#define eMR_BIT01               0x0002          // bit code: switched on
#define eMR_BIT00               0x0001          // bit code: ready to switch on


#define PPM_MODE                  0x01             // Profile Position Mode
#define PVM_MODE                  0x03             // Profile Velocity Mode
#define PVT_MODE                  0x07             // Interpolated Position Mode
#define PM_MODE                   0xFF             // Position Mode (Electronic Gear)
#define VM_MODE                   0xFE             // Velocity Mode (Electronic Gear)
#define CM_MODE                   0xFD             // Current Mode
#define HM_MODE                   0x06             // Homing Mode

#define DIR_POS                   0
#define DIR_NEG                   1
#define STEP                      10


typedef enum axis_s{
  axis1 = 1,
  axis2 = 2
}eMR_axis;


typedef struct eMR_s {
  uint32_t cobid;
  uint32_t bootup_ID1;
  uint32_t bootup_ID2;
  uint16_t cmd;
  uint8_t mode;
  bool Err_Flag;
  int32_t ProfileAcceleration;
  int32_t ProfileDeceleration;
  int32_t MaxProfileVelocity;
  int32_t TargetVelocity;
  int32_t TargetPosition;
  int32_t TargetTorque;
  int32_t ActualVelocity;
  int32_t ActualPosition;
  int32_t ActualTorque;
  int32_t StatusWord;
  int32_t ControlWord;


} eMR_t;


FlexCAN_T4<CAN1, RX_SIZE_256, TX_SIZE_16> can1;
FlexCAN_T4<CAN2, RX_SIZE_256, TX_SIZE_16> can2;

CAN_message_t msg;
CAN_message_t revPACKET;
eMR_t eMR; 
uint16_t count = 0;

// int LED = 13;                 // LED status TeensyMicromod
// int LED_RUN = 32;             // G9 - Teensy pin 32, MicroMod pad 65
// int LED_STATUS = 26;          // G8 - Teensy pin 26, MicroMod pad 67
const int PWM0 = 3;
const int PWM1 = 2;
float motor_pwm1;
float motor_pwm2;


bool CAN1_ReceiveFrame()
{
  unsigned long timer_out = millis();

  for ( uint8_t i = 0; i <revPACKET.len ; i++ ) revPACKET.buf[i] = 0;

  revPACKET.id = 0x000000;

  while ((revPACKET.id == 0x000000) and (millis()-timer_out < ReceiveTimeOut))
  {
    if(can1.read(revPACKET))
    { 

      if (revPACKET.id == 0x000701) eMR.bootup_ID1 = 0x000701;
      if (revPACKET.id == 0x000702) eMR.bootup_ID2 = 0x000702;
      
    }
  }

  // Check CANOpen eMR Time out:
  if (millis()-timer_out >= ReceiveTimeOut)
  {
    // Serial.println("CANOpen eMR respond time out.. ");
    return false;
  }

  return true;
}
bool CAN1_SendFrame(uint32_t node_id, uint32_t data_size, uint8_t* data)
{
  msg.id = node_id;
  msg.len = data_size;

  memcpy(&msg.buf[0], &data[0], data_size);
  can1.write(msg);  

  return CAN1_ReceiveFrame();
}
//////////////////  NMT command ///////////////////////
uint8_t NMT_SetOperational()
{
  uint8_t data[2];
  uint32_t node_id;
  
  node_id = NMT_COBID;
  data[0] = NMT_Operation ;                                             
  data[1] = 0x00 ;                                   //  NMT Command all network                             
  
  return CAN1_SendFrame(node_id, 0x02, data);
}
uint8_t NMT_SetPreoperational()
{
  uint8_t data[2];
  uint32_t node_id;
  
  node_id = NMT_COBID;
  data[0] = NMT_PreOperation;                                             
  data[1] = 0x00 ;                                   //  NMT Command all network                             
  
  return CAN1_SendFrame(node_id, 0x02, data);
}
//////////////////  NMT bootup check command ///////////////////////
uint8_t NMT_Bootup()
{
  uint8_t data[2];

//   Serial.println("eMR CANopen check bootup message.. "); 
                                    
  for ( uint8_t i = 0; i <5 ; i++ ) {

        data[0] = NMT_Query;                                               //  NMT Command                 
        data[1] = axis1 ;    
        CAN1_SendFrame(NMT_COBID, 0x02, data);      
        data[0] = NMT_Query;                                               //  NMT Command                 
        data[1] = axis2 ;    
        CAN1_SendFrame(NMT_COBID, 0x02, data);        
        
        if ((eMR.bootup_ID1==0x701) || (eMR.bootup_ID1==0x702))
        {
            // Serial.println("eMR CANopen Initial OK... "); 
            // Serial.print(" Found! eMR ID1: 0x"); Serial.println(eMR.bootup_ID1, HEX );
            // Serial.print(" Found! eMR ID2: 0x"); Serial.println(eMR.bootup_ID2, HEX );
            return 1;
        }
  }

    // Serial.println("Can't find eMR L12 dual driver motor in CANopen bus. "); 
  return 0;  
}
//////////////  SYNCHRONIZATION OBJECT (SYNC) /////////
////////// The Synchronization object is used to simultaneously validate the time of PDO data //////////
uint8_t Sync_message(eMR_t *eMR)
{
  uint8_t data[1];          
  uint32_t node_id;
  
  node_id = eMR->cobid;
  data[0] = 0x00;                                                                

  return CAN1_SendFrame(node_id, 0x01, data);
}
////////////// Producer Heartbeat messsage /////////
uint8_t Heartbeat_CheckMessage(eMR_t *eMR)
{
  CAN_message_t revPACKET;

  if(can1.read(revPACKET))
  {   
    // Serial.print("  ID: 0x"); Serial.print(revPACKET.id, HEX );
    // Serial.print(" DATA: ");
    // Serial.print(revPACKET.buf[0],HEX); Serial.print(" ");
    // Serial.print("  TS: "); Serial.println(revPACKET.timestamp);
  }
  return revPACKET.buf[0];
}

uint8_t CANOpen_ResetFaults(eMR_t *eMR)
{
  uint8_t data[8];
  uint32_t node_id;

  node_id = eMR->cobid;
  data[0] = SDO_Expedited_2;                                  // SDO <CMD>            Byte0
  data[1] = (uint8_t)(Controlword_Obj & 0xFF);                // SDO <Lowbyte_Index>  Byte1
  data[2] = (uint8_t)((Controlword_Obj>>8) & 0xFF);           // SDO <Highbyte_Index> Byte2
  data[3] = 0x00;                                             // SDO <SubIndex>       Byte3
  data[4] = 0x80;                                             // SDO <DATA_Byte0>     Byte4
  data[5] = 0x00;                                             // SDO <DATA_Byte1>     Byte5
  data[6] = 0x00;                                             // SDO <DATA_Byte2>     Byte6
  data[7] = 0x00;                                             // SDO <DATA_Byte3>     Byte7

  return CAN1_SendFrame(node_id,DLC, data);                          // Test DLC = 6?
}
uint8_t CANOpen_Shutdown(eMR_t *eMR)
{
  uint8_t data[8];
  uint32_t node_id;

  node_id = eMR->cobid;
  data[0] = SDO_Expedited_2;                                  // SDO <CMD>            Byte0
  data[1] = (uint8_t)(Controlword_Obj & 0xFF);                // SDO <Lowbyte_Index>  Byte1
  data[2] = (uint8_t)((Controlword_Obj>>8) & 0xFF);           // SDO <Highbyte_Index> Byte2
  data[3] = 0x00;                                             // SDO <SubIndex>       Byte3
  data[4] = 0x06;                                             // SDO <DATA_Byte0>     Byte4
  data[5] = 0x00;                                             // SDO <DATA_Byte1>     Byte5
  data[6] = 0x00;                                             // SDO <DATA_Byte2>     Byte6
  data[7] = 0x00;                                             // SDO <DATA_Byte3>     Byte7

  return CAN1_SendFrame(node_id,DLC, data);                               // Test DLC = 6?
}  

uint8_t CANOpen_SetOperationMode(eMR_t *eMR)
{
  uint8_t data[8];
  uint32_t node_id;

  node_id = eMR->cobid;
  data[0] = SDO_Expedited_1;                                  // SDO <CMD>            Byte0
  data[1] = (uint8_t)(OperationMode_Obj & 0xFF);              // SDO <Lowbyte_Index>  Byte1
  data[2] = (uint8_t)((OperationMode_Obj>>8) & 0xFF);         // SDO <Highbyte_Index> Byte2
  data[3] = 0x00;                                             // SDO <SubIndex>       Byte3
  data[4] = eMR->mode;                                      // SDO <DATA_Byte0>     Byte4
  data[5] = 0x00;                                             // SDO <DATA_Byte1>     Byte5
  data[6] = 0x00;                                             // SDO <DATA_Byte2>     Byte6
  data[7] = 0x00;                                             // SDO <DATA_Byte3>     Byte7

  return CAN1_SendFrame(node_id,DLC, data);                               // Test DLC = 6?
}
uint8_t CANOpen_ReadStatusObj(eMR_t *eMR)
{
  uint8_t data[8];
  uint32_t node_id;
  int32_t Statusword = 0;

  node_id = eMR->cobid;
  data[0] = RSDO_Expedited_4;                                 // SDO <CMD>            Byte0
  data[1] = (uint8_t)(Statusword_Obj & 0xFF);                 // SDO <Lowbyte_Index>  Byte1
  data[2] = (uint8_t)((Statusword_Obj>>8) & 0xFF);            // SDO <Highbyte_Index> Byte2
  data[3] = 0x00;                                             // SDO <SubIndex>       Byte3
  data[4] = 0x00;                                             // SDO <DATA_Byte0>     Byte4
  data[5] = 0x00;                                             // SDO <DATA_Byte1>     Byte5
  data[6] = 0x00;                                             // SDO <DATA_Byte2>     Byte6
  data[7] = 0x00;                                             // SDO <DATA_Byte3>     Byte7

  CAN1_SendFrame(node_id,DLC, data);      

  eMR->StatusWord = ((Statusword | revPACKET.buf[7])<<24) + ((Statusword | revPACKET.buf[6])<<16)
             + ((Statusword | revPACKET.buf[5])<<8) + (Statusword | revPACKET.buf[4]);                         

  return 0;
}

uint8_t CANOpen_ReadActualVelocityObj(eMR_t *eMR)
{
  
  uint8_t data[8];
  uint32_t node_id;
  int32_t velocity = 0;

  node_id = eMR->cobid;
  data[0] = RSDO_Expedited_4;                                 // SDO <CMD>            Byte0
  data[1] = (uint8_t)(ActualVelocity_Obj & 0xFF);             // SDO <Lowbyte_Index>  Byte1
  data[2] = (uint8_t)((ActualVelocity_Obj>>8) & 0xFF);        // SDO <Highbyte_Index> Byte2
  data[3] = 0x00;                                             // SDO <SubIndex>       Byte3
  data[4] = 0x00;                                             // SDO <DATA_Byte0>     Byte4
  data[5] = 0x00;                                             // SDO <DATA_Byte1>     Byte5
  data[6] = 0x00;                                             // SDO <DATA_Byte2>     Byte6
  data[7] = 0x00;                                             // SDO <DATA_Byte3>     Byte7

  
  CAN1_SendFrame(node_id,DLC, data);                          

  eMR->ActualVelocity = ((velocity | revPACKET.buf[7])<<24) + ((velocity | revPACKET.buf[6])<<16)
             + ((velocity | revPACKET.buf[5])<<8) + (velocity | revPACKET.buf[4]);

  return 0;
}


uint8_t CANOpen_SwitchON(eMR_t *eMR)
{
  uint8_t data[8];
  uint32_t node_id;

  node_id = eMR->cobid;
  data[0] = SDO_Expedited_2;                                  // SDO <CMD>            Byte0
  data[1] = (uint8_t)(Controlword_Obj & 0xFF);                // SDO <Lowbyte_Index>  Byte1
  data[2] = (uint8_t)((Controlword_Obj>>8) & 0xFF);           // SDO <Highbyte_Index> Byte2
  data[3] = 0x00;                                             // SDO <SubIndex>       Byte3
  data[4] = 0x0F;                                             // SDO <DATA_Byte0>     Byte4
  data[5] = 0x00;                                             // SDO <DATA_Byte1>     Byte5
  data[6] = 0x00;                                             // SDO <DATA_Byte2>     Byte6
  data[7] = 0x00;                                             // SDO <DATA_Byte3>     Byte7

  return CAN1_SendFrame(node_id,DLC, data);                               // Test DLC = 6?
}
uint8_t CANOpen_SetProfileAcceleration(eMR_t *eMR)
{
  uint8_t data[8];
  uint32_t node_id;
  int32_t ProfileAccel;

  node_id = eMR->cobid;
  ProfileAccel = eMR->ProfileAcceleration;
  data[0] = SDO_Expedited_4;                                  // SDO <CMD>            Byte0
  data[1] = (uint8_t)(ProfileAcceleration_Obj & 0xFF);             // SDO <Lowbyte_Index>  Byte1
  data[2] = (uint8_t)((ProfileAcceleration_Obj>>8) & 0xFF);        // SDO <Highbyte_Index> Byte2
  data[3] = 0x00;                                             // SDO <SubIndex>       Byte3

  data[4] = (uint8_t)(ProfileAccel & 0xFF);                       // SDO <DATA_Byte0>     Byte4
  data[5] = (uint8_t)((ProfileAccel>>8) & 0xFF);                  // SDO <DATA_Byte1>     Byte5
  data[6] = (uint8_t)((ProfileAccel>>16) & 0xFF);                 // SDO <DATA_Byte2>     Byte6
  data[7] = (uint8_t)((ProfileAccel>>24) & 0xFF);                 // SDO <DATA_Byte3>     Byte7

  return CAN1_SendFrame(node_id,DLC, data); 
}
uint8_t CANOpen_SetProfileDeceleration(eMR_t *eMR)
{
  uint8_t data[8];
  uint32_t node_id;
  int32_t ProfileDecel;

  node_id = eMR->cobid;
  ProfileDecel = eMR->ProfileAcceleration;
  data[0] = SDO_Expedited_4;                                  // SDO <CMD>            Byte0
  data[1] = (uint8_t)(ProfileDeceleration_Obj & 0xFF);             // SDO <Lowbyte_Index>  Byte1
  data[2] = (uint8_t)((ProfileDeceleration_Obj>>8) & 0xFF);        // SDO <Highbyte_Index> Byte2
  data[3] = 0x00;                                             // SDO <SubIndex>       Byte3

  data[4] = (uint8_t)(ProfileDecel & 0xFF);                       // SDO <DATA_Byte0>     Byte4
  data[5] = (uint8_t)((ProfileDecel>>8) & 0xFF);                  // SDO <DATA_Byte1>     Byte5
  data[6] = (uint8_t)((ProfileDecel>>16) & 0xFF);                 // SDO <DATA_Byte2>     Byte6
  data[7] = (uint8_t)((ProfileDecel>>24) & 0xFF);                 // SDO <DATA_Byte3>     Byte7

  return CAN1_SendFrame(node_id,DLC, data); 
}
uint8_t CANOpen_SetTargetVelocity(eMR_t *eMR)
{
  uint8_t data[8];
  uint32_t node_id;
  int32_t velocity;

  node_id = eMR->cobid;
  velocity = eMR->TargetVelocity;
  data[0] = SDO_Expedited_4;                                  // SDO <CMD>            Byte0
  data[1] = (uint8_t)(TargetVelocity_Obj & 0xFF);             // SDO <Lowbyte_Index>  Byte1
  data[2] = (uint8_t)((TargetVelocity_Obj>>8) & 0xFF);        // SDO <Highbyte_Index> Byte2
  data[3] = 0x00;                                             // SDO <SubIndex>       Byte3

  data[4] = (uint8_t)(velocity & 0xFF);                       // SDO <DATA_Byte0>     Byte4
  data[5] = (uint8_t)((velocity>>8) & 0xFF);                  // SDO <DATA_Byte1>     Byte5
  data[6] = (uint8_t)((velocity>>16) & 0xFF);                 // SDO <DATA_Byte2>     Byte6
  data[7] = (uint8_t)((velocity>>24) & 0xFF);                 // SDO <DATA_Byte3>     Byte7

  // 5 rpm 

  //data[4] = 0x6A;                       
  //data[5] = 0xAA;                  
  //data[6] = 0x00;                 
  //data[7] = 0x00;                 

  return CAN1_SendFrame(node_id,DLC, data); 
}
uint8_t CANOpen_StopMotor(eMR_t *eMR)
{
  uint8_t data[8];
  uint32_t node_id;

  node_id = eMR->cobid;
  data[0] = SDO_Expedited_4;                                  // SDO <CMD>            Byte0
  data[1] = (uint8_t)(TargetVelocity_Obj & 0xFF);             // SDO <Lowbyte_Index>  Byte1
  data[2] = (uint8_t)((TargetVelocity_Obj>>8) & 0xFF);        // SDO <Highbyte_Index> Byte2
  data[3] = 0x00;                                             // SDO <SubIndex>       Byte3
  data[4] = 0x00;                       
  data[5] = 0x00;                  
  data[6] = 0x00;                 
  data[7] = 0x00;                 

  return CAN1_SendFrame(node_id,DLC, data); 
}
uint8_t CANOpen_SetTargetPosition(eMR_t *eMR)
{
  uint8_t data[8];
  uint32_t node_id;
  int32_t position;

  node_id = eMR->cobid;
  position = eMR->TargetPosition;
  data[0] = SDO_Expedited_4;                                  // SDO <CMD>            Byte0
  data[1] = (uint8_t)(TargetPosition_Obj & 0xFF);             // SDO <Lowbyte_Index>  Byte1
  data[2] = (uint8_t)((TargetPosition_Obj>>8) & 0xFF);        // SDO <Highbyte_Index> Byte2
  data[3] = 0x00;                                             // SDO <SubIndex>       Byte3

  data[4] = (uint8_t)(position & 0xFF);                       // SDO <DATA_Byte0>     Byte4
  data[5] = (uint8_t)((position>>8) & 0xFF);                  // SDO <DATA_Byte1>     Byte5
  data[6] = (uint8_t)((position>>16) & 0xFF);                 // SDO <DATA_Byte2>     Byte6
  data[7] = (uint8_t)((position>>24) & 0xFF);                 // SDO <DATA_Byte3>     Byte7

  return CAN1_SendFrame(node_id,DLC, data); 
}
void CANOpen_eMR_Init(void)
{

  NMT_SetOperational();
  delay(100);
  eMR.cobid = TSDO_COBID + axis1;

  CANOpen_ResetFaults(&eMR);
                                                 
  eMR.mode = PVM_MODE;                                      //  Target velocity mode     
  CANOpen_SetOperationMode(&eMR);                           
  //CANOpen_SetProfileAcceleration(&eMR);  
  //CANOpen_SetProfileDeceleration(&eMR); 
  //CANOpen_Shutdown(&eMR);
  delay(100);
  CANOpen_SwitchON(&eMR);    
  delay(100);

  eMR.cobid = TSDO_COBID + axis2;
  CANOpen_ResetFaults(&eMR);

  eMR.mode = PVM_MODE; 
  CANOpen_SetOperationMode(&eMR);                   
  //CANOpen_SetProfileAcceleration(&eMR);  
  //CANOpen_SetProfileDeceleration(&eMR); 
  //CANOpen_Shutdown(&eMR);
  delay(100);
  CANOpen_SwitchON(&eMR);
  delay(100);
  
}

void CANOpen_eMR_QuickStop(void)
{
    eMR.cobid = TSDO_COBID + axis1;
    //CANOpen_Shutdown(&eMR);
    CANOpen_StopMotor(&eMR);
    eMR.cobid = TSDO_COBID + axis2;
    //CANOpen_Shutdown(&eMR);
    CANOpen_StopMotor(&eMR);
}


uint8_t eMRCanSpeedCntrl(float percentPwm, bool direction, uint8_t motorNo)
{

    eMR.MaxProfileVelocity = MaxVelocity;                    // Max RPM

    if (motorNo==1)
    {
      eMR.cobid = TSDO_COBID + axis1;
      if (direction)                             // Positive direction
      {
          // eMR.TargetVelocity = (int32_t)(eMR.MaxProfileVelocity*percentPwm*16384/600);
          eMR.TargetVelocity = (int32_t)(eMR.MaxProfileVelocity*percentPwm*16384/947);
        //   Serial.print("Target Speed motor1: "); Serial.println(eMR.TargetVelocity, DEC );
      }
      else                                       // Negative direction
      {
          // eMR.TargetVelocity = (int32_t)(-eMR.MaxProfileVelocity*percentPwm*16384/600);    
          eMR.TargetVelocity = (int32_t)(-eMR.MaxProfileVelocity*percentPwm*16384/947);
        //   Serial.print("Target Speed motor1: "); Serial.println(eMR.TargetVelocity, DEC );
      }      
      CANOpen_SetTargetVelocity(&eMR);
    }
    else if (motorNo==2){
      eMR.cobid = TSDO_COBID + axis2;
      if (direction)
      {
          // eMR.TargetVelocity = (int32_t)(eMR.MaxProfileVelocity*percentPwm*16384/600);
          eMR.TargetVelocity = (int32_t)(eMR.MaxProfileVelocity*percentPwm*16384/947);
        //   Serial.print("Target Speed motor2: "); Serial.println(eMR.TargetVelocity, DEC );
      }
      else
      {
          // eMR.TargetVelocity = (int32_t)(-eMR.MaxProfileVelocity*percentPwm*16384/600);  
          eMR.TargetVelocity = (int32_t)(-eMR.MaxProfileVelocity*percentPwm*16384/947);
        //   Serial.print("Target Speed motor2: "); Serial.println(eMR.TargetVelocity, DEC );   
      }
      CANOpen_SetTargetVelocity(&eMR);
    }
    else
    {
      return -1;
    }
  return 0;   
}

void Update_eMRMotorSpeed(void)
{
  eMR.cobid = TSDO_COBID + axis1;
  CANOpen_SwitchON(&eMR); 
  eMR.cobid = TSDO_COBID + axis2;
  CANOpen_SwitchON(&eMR); 
}



