#define USE_SMR_CONFIG

#include <Arduino.h>
#include <TeensyThreads.h>
#include <ModbusMaster.h>

#include <stdio.h>
#include <vector>
#include "math.h"

#include "config.h"
#include "kinematics.h"
#include "JY61P.h"

#define DEBUG           false
#define DEBUG_IMU       false
#define DEBUG_MOTOR     false
#define DEBUG_MOTION    false
#define DEBUG_RANGE     false

#define DEBUG_ODOM      false
#define DEBUG_SEND      false
#define DEBUG_RECEIVE   false
#define DEBUG_OLED      false
#define DEBUG_SAFTY     false
#define DEBUG_BATT      false

#define HEAD        0xFF
#define HOST_ID     0x00
#define DEVICE_ID   0x01

#define FUNC_MOTION 0x01
#define FUNC_IMU    0x02
#define FUNC_ODOM   0x03
#define FUNC_RANGE  0x04
#define FUNC_IP     0x05
#define FUNC_STATUS 0x06
#define FUNC_BATT   0x07

/////////////////////////////////////////////////////////////////////////////////////
#define motor_axis0  0 
#define motor_axis1  1 
#define MAX485_DE    4
#define MAX485_RE    5

#define TEENSY_RS485_DIR_PIN 22
#define MODBUS_SERIAL Serial1

#define REG_DUALAXIS_CMD_SPEED    0x0B1A
#define REG_AXIS1_COMMAND_SPEED   0x0A02
#define REG_AXIS1_FEEDBACK_SPEED  0x0A03
#define REG_AXIS2_COMMAND_SPEED   0x0C02
#define REG_AXIS2_FEEDBACK_SPEED  0x0C03

ModbusMaster Motor_dualdrive;

int LED = 13;                 // LED status TeensyMicromod
int LED_RUN = 32;             // G9 - Teensy pin 32, MicroMod pad 65
int LED_STATUS = 26;          // G8 - Teensy pin 26, MicroMod pad 67
int RS485_DE = 4;
int RS485_RE = 5;
String data;
/////////////////////////////////////////////////////////////////////////////////////

Kinematics kinematics(
    Kinematics::SMR_BASE, 
    MOTOR_MAX_RPM, 
    MAX_RPM_RATIO, 
    MOTOR_OPERATING_VOLTAGE, 
    MOTOR_POWER_MAX_VOLTAGE, 
    WHEEL_DIAMETER, 
    LR_WHEELS_DISTANCE
);

Kinematics::velocities cmd_vel;

Threads::Mutex xSendDataMutex;

static const int RX_BUF_SIZE = 1024;

unsigned long prev_cmd_time = 0;

void parse_data(uint8_t func, uint8_t *data, uint8_t data_len){

    if (func == FUNC_MOTION){

        Serial.println("In Motion function");

        int16_t packed_v_x = (data[1] << 8) | data[0];
        int16_t packed_v_y = (data[3] << 8) | data[2];
        int16_t packed_w_z = (data[5] << 8) | data[4];

        cmd_vel.linear_x  = packed_v_x/1000.0;
        cmd_vel.linear_y  = packed_v_y/1000.0;
        cmd_vel.angular_z = packed_w_z/1000.0;

        prev_cmd_time = millis();

        if(DEBUG_MOTION){
            Serial.printf("Vx: %f\n", cmd_vel.linear_x);
            Serial.printf("Vy: %f\n", cmd_vel.linear_y);
            Serial.printf("Wz: %f\n", cmd_vel.angular_z);            
        }
    }
    else{
        if(DEBUG_RECEIVE){
            Serial5.println("Out of provided function");
        }
    }
}

void recive_data_task(void * parameter){

    uint8_t header;
    uint8_t device_id;
    uint8_t len;
    uint8_t func;
    uint8_t data_len;
    uint8_t data_to_mem = 0;
    uint8_t value;
    uint8_t rx_check_num;
    uint8_t check_sum;  

    uint8_t *data = (uint8_t*) malloc(RX_BUF_SIZE);
    uint8_t* buffer = (uint8_t*) malloc(RX_BUF_SIZE + 1);

    while(true){

        if (Serial.available() > 0) {
            header = Serial.read();

            if (header == HEAD) {
                if(DEBUG_RECEIVE){
                    Serial5.println("--------------New Data--------------");
                    Serial5.println("Correct header");
                }
                device_id = Serial.read();

                if (device_id == DEVICE_ID) {

                    len = Serial.read();
                    func = Serial.read();

                    check_sum = header + device_id + len + func;
                    data_len = len - 4;
                    data_to_mem = data_len;
                    memset(data, 0, RX_BUF_SIZE);

                    while (data_to_mem > 0){
                        uint8_t index = data_len - data_to_mem;
                        data[index] = Serial.read();
                        check_sum += data[index];

                        data_to_mem--;
                    }

                    rx_check_num = Serial.read();  

                    if ((check_sum & 0xFF) == rx_check_num){
                                if(DEBUG_RECEIVE){
                                    Serial5.println("Data Recived");
                                }
                                threads.delay(1);
                                parse_data(func, data, data_len);
                            }
                    else {
                                if(DEBUG_RECEIVE){
                                    Serial5.println("Check sum error");
                                }                      
                    }
                    if(DEBUG_RECEIVE){
                        Serial5.print("Device_id:  ");
                        Serial5.println(device_id);
                        Serial5.print("Data_range:  ");
                        Serial5.println(len);
                        Serial5.print("Function:  ");
                        Serial5.println(func);
                        for (uint8_t i=0; i < data_len; i++){
                            Serial5.print("Data ");
                            Serial5.print(i);
                            Serial5.print(": ");
                            Serial5.println(data[i]);
                        }
                        Serial5.print("Rx_check:  ");
                        Serial5.println(rx_check_num);
                        Serial5.print("Check sum:  ");
                        Serial5.println(check_sum & 0xFF);                   
                    }
                }
            }
        }     
        threads.delay(10);
    }
    free(data);
    free(buffer);
}

void send_data(uint8_t FUNC_TYPE, uint8_t *param, size_t param_len) {

    if (xSendDataMutex.getState() == 0){

        xSendDataMutex.lock();

        size_t cmd_len = param_len + 4 + 1;
        uint8_t *cmd = (uint8_t*) malloc(cmd_len); 

        cmd[0] = HEAD;
        cmd[1] = HOST_ID;
        cmd[2] = cmd_len - 1;
        cmd[3] = FUNC_TYPE;
        memcpy(&cmd[4], param, param_len);
        
        uint8_t checksum = 0;
        for (size_t i = 0; i < cmd_len - 1; i++) {
            checksum += cmd[i];
        }
        checksum &= 0xFF;
        cmd[cmd_len - 1] = checksum;

        for (size_t i = 0; i < cmd_len; i++) {
            Serial.write(cmd[i]);
        }

        if(DEBUG_SEND){
            Serial5.println("Sent command:");
            for (size_t i = 0; i < cmd_len; i++) {
                Serial5.print(cmd[i]);
                Serial5.print(" ");
            }
            Serial5.println();
        }
    free(cmd);
    xSendDataMutex.unlock();
    }
}

void preTransmission(){
  delay(10);
  digitalWrite(MAX485_RE, 1);
  digitalWrite(MAX485_DE, 1);
  delay(10);
}

void postTransmission(){
  delay(10);
  digitalWrite(MAX485_DE, 0);
  digitalWrite(MAX485_RE, 0);
  delay(10);
}

void setup() 
{
    pinMode(LED, OUTPUT);  
    pinMode(LED_STATUS, OUTPUT);
    pinMode(LED_RUN, OUTPUT);
    pinMode(MAX485_RE, OUTPUT);
    pinMode(MAX485_DE, OUTPUT);

    Serial.begin(115200);    
    MODBUS_SERIAL.begin(115200);
    
    Motor_dualdrive.begin(1, MODBUS_SERIAL );
    JY61P.startIIC();
    JY61P.caliIMU();

    threads.addThread(control_task);
    threads.addThread(recive_data_task);
    threads.addThread(imu_update_task);
    delay(10);
}

void loop() {}

void imu_update_task(void *arg)
{
    float imu_gyro_dps[3] = {0};
    float imu_accel_g[3] = {0};

    while (1)
    {   
        imu_gyro_dps[0] = (JY61P.getGyroX() / 180) * 3.14;
        imu_gyro_dps[1] = (JY61P.getGyroY() / 180) * 3.14;
        imu_gyro_dps[2] = (JY61P.getGyroZ() / 180) * 3.14;

        imu_accel_g[0] = JY61P.getAccX();
        imu_accel_g[1] = JY61P.getAccY();
        imu_accel_g[2] = JY61P.getAccZ();
	    
        if (DEBUG_IMU){
            Serial.println(imu_gyro_dps[0]);
            Serial.println(imu_gyro_dps[1]);
            Serial.println(imu_gyro_dps[2]);
            Serial.println(imu_accel_g[0]);
            Serial.println(imu_accel_g[1]);
            Serial.println(imu_accel_g[2]);     
            Serial.println("-------------------------------------------");       
        }

        int16_t roll   = static_cast<int16_t>(imu_gyro_dps[0]*1000);
        int16_t pitch  = static_cast<int16_t>(imu_gyro_dps[1]*1000);
        int16_t yaw    = static_cast<int16_t>(imu_gyro_dps[2]*1000);

        int16_t acc_x  = static_cast<int16_t>(imu_accel_g[0]*1000);
        int16_t acc_y  = static_cast<int16_t>(imu_accel_g[1]*1000);
        int16_t acc_z  = static_cast<int16_t>(imu_accel_g[2]*1000);

        uint8_t cmd[12] = {
            static_cast<uint8_t>(roll  & 0xFF), static_cast<uint8_t>((roll  >> 8) & 0xFF),
            static_cast<uint8_t>(pitch & 0xFF), static_cast<uint8_t>((pitch >> 8) & 0xFF),
            static_cast<uint8_t>(yaw   & 0xFF), static_cast<uint8_t>((yaw   >> 8) & 0xFF),
            static_cast<uint8_t>(acc_x & 0xFF), static_cast<uint8_t>((acc_x >> 8) & 0xFF),
            static_cast<uint8_t>(acc_y & 0xFF), static_cast<uint8_t>((acc_y >> 8) & 0xFF),
            static_cast<uint8_t>(acc_z & 0xFF), static_cast<uint8_t>((acc_z >> 8) & 0xFF)
        };

        send_data(FUNC_IMU, cmd, sizeof(cmd));
        threads.delay(10);
    }
}

void control_task(void *arg)
{
    while (1)
    {
        uint64_t start_time = millis();  
        uint8_t result;
        int16_t buff;
        float current_rpm_left;
        float current_rpm_right;
         
        if (millis() - prev_cmd_time > 100){
            cmd_vel.linear_x  = 0.0;
            cmd_vel.linear_y  = 0.0;
            cmd_vel.angular_z = 0.0;
        }
        
        Kinematics::rpm req_rpm = kinematics.getRPM(
        cmd_vel.linear_x, 
        cmd_vel.linear_y, 
        cmd_vel.angular_z);  

        Motor_dualdrive.setTransmitBuffer(0, 0x0003);
        Motor_dualdrive.setTransmitBuffer(1, int16_t(req_rpm.motor2 * 30));
        Motor_dualdrive.setTransmitBuffer(2, int16_t(req_rpm.motor1 * 30));
        Motor_dualdrive.writeMultipleRegisters(REG_DUALAXIS_CMD_SPEED, 3);
        delay(1);

        if( Motor_dualdrive.readHoldingRegisters(REG_AXIS2_FEEDBACK_SPEED,1) == Motor_dualdrive.ku8MBSuccess ){
          buff = Motor_dualdrive.getResponseBuffer(0);
          current_rpm_left = (buff / 30.0) * (-1);
          delay(1);
        }

        
        if( Motor_dualdrive.readHoldingRegisters(REG_AXIS1_FEEDBACK_SPEED,1) == Motor_dualdrive.ku8MBSuccess ){
          buff = Motor_dualdrive.getResponseBuffer(0);
          current_rpm_right = (buff / 30.0) ;
          delay(1);
        }
        
        Kinematics::velocities current_vel = kinematics.getVelocities(
            current_rpm_left, 
            current_rpm_right, 
            0, 
            0
        );

        int16_t Vx  = static_cast<int16_t>(current_vel.linear_x *1000);
        int16_t Vy  = static_cast<int16_t>(current_vel.linear_y *1000);
        int16_t Wz  = static_cast<int16_t>(current_vel.angular_z*1000);

        uint8_t cmd[6] = {
            static_cast<uint8_t>(Vx & 0xFF), static_cast<uint8_t>((Vx >> 8) & 0xFF),
            static_cast<uint8_t>(Vy & 0xFF), static_cast<uint8_t>((Vy >> 8) & 0xFF),
            static_cast<uint8_t>(Wz & 0xFF), static_cast<uint8_t>((Wz >> 8) & 0xFF)
        };

        send_data(FUNC_ODOM, cmd, sizeof(cmd));

        uint64_t end_time = millis();
        uint32_t dt = end_time - start_time;

        if (DEBUG_ODOM){
            Serial5.printf(">Command Vx :%f\n", cmd_vel.linear_x);
            Serial5.printf(">Command Wz :%f\n", cmd_vel.angular_z);
            Serial5.printf(">Odom Vx :%f\n", current_vel.linear_x);
            Serial5.printf(">Odom Wz :%f\n", current_vel.angular_z);
        }

        if (DEBUG_MOTOR){
            Serial5.printf(">Command Motor 1:%f\n", req_rpm.motor1);
            Serial5.printf(">Command Motor 2:%f\n", req_rpm.motor2);
            Serial5.printf(">Current Motor 1:%f\n", current_rpm_left);
            Serial5.printf(">Current Motor 2:%f\n", current_rpm_right);
            Serial5.printf(">dt:%d\n", dt);
        }
        
        // Serial.println(dt);
        if (dt >= 35){dt = 0;}
        threads.delay(35-dt);
    }    
}



