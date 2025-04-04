#define USE_SMR_CONFIG

#include <Arduino.h>
#include <TeensyThreads.h>
#include <ModbusMaster.h>
#include <Adafruit_MCP23X17.h>

#include <stdio.h>
#include <vector>
#include "math.h"

#include "config.h"
#include "kinematics.h"
#include "JY61P.h"
#include "bms.h"

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

const int ledPins[4] = {40, 41, 44, 73};
// const int ledPins[4] = {15, 25, 16, 32};

/////////////////////////////////////////////////////////////////////////////////////
#define motor_axis0  0 
#define motor_axis1  1 
#define MAX485_DE    4
#define MAX485_RE    5

#define TEENSY_RS485_DIR_PIN 22
#define MOTOR_DRIVER_SERIAL Serial1
#define SENSORS_SERIAL      Serial2
#define BMS_SERIAL          Serial2
#define ULTARSONICS_SERIAL  Serial2

#define REG_DUALAXIS_CMD_SPEED    0x0B1A
#define REG_AXIS1_COMMAND_SPEED   0x0A02
#define REG_AXIS1_FEEDBACK_SPEED  0x0A03
#define REG_AXIS2_COMMAND_SPEED   0x0C02
#define REG_AXIS2_FEEDBACK_SPEED  0x0C03

ModbusMaster Motor_dualdrive;
ModbusMaster Sensor_module;
ModbusMaster Ultrasonics_1; //Left
ModbusMaster Ultrasonics_2; //Center
ModbusMaster Ultrasonics_3; //Right

int LED = 13;                 // LED status TeensyMicromod
int LED_RUN = 32;             // G9 - Teensy pin 32, MicroMod pad 65
int LED_STATUS = 26;          // G8 - Teensy pin 26, MicroMod pad 67
int RS485_DE = 4;
int RS485_RE = 5;
String data;
/////////////////////////////////////////////////////////////////////////////////////

int16_t range_left;
int16_t range_center;
int16_t range_right;
uint16_t cliff;

uint8_t range_limit = 200;
uint8_t led_mode_prev;
uint8_t alarm_mode_prev;

uint8_t imu2send[12];
uint8_t odom2send[6];
uint8_t batt2send[7];
uint8_t range2send[6];
bool imu_ready;
bool odom_ready;
bool batt_ready;
bool range_ready;

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

Adafruit_MCP23X17 mcp;
bool mcp_input[8];
bool mcp_out_put[8];

bool cliff_state, bumper_state, emer_state, stop, ack;
bool connection_failed;

volatile unsigned long msStart;
volatile unsigned int state_prev=0, state_batt=0;

void parse_data(uint8_t func, uint8_t *data, uint8_t data_len){

    if (func == FUNC_MOTION){
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

        while (Serial.available() > 0) {
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
        threads.delay(25);
    }
    free(data);
    free(buffer);
}

void send_data_task(void * parameter){

    while(true){
        if (odom_ready && imu_ready){
            if(!DEBUG){
                send_data(FUNC_IMU, imu2send, sizeof(imu2send));
                send_data(FUNC_ODOM, odom2send, sizeof(odom2send));
                send_data(FUNC_RANGE, range2send, sizeof(range2send));
                if(batt_ready){
                    send_data(FUNC_BATT, batt2send, sizeof(batt2send));
                    batt_ready = false;
                }
                odom_ready = false;
                imu_ready = false;
            }
        }
        threads.delay(10);
    }
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

    mcp.begin_I2C(0x21);

    for(uint8_t i=0; i<16; i++){
        if (i>7) mcp.pinMode(i, INPUT_PULLUP);
        else     mcp.pinMode(i, OUTPUT);
    }

    // for (uint8_t i = 0; i < sizeof(ledPins); i++) {
    //     pinMode(ledPins[i], OUTPUT);
    // }

    mcp.digitalWrite(7, HIGH);
    delay(5000);

    Serial.begin(115200);    
    MOTOR_DRIVER_SERIAL.begin(115200);
    SENSORS_SERIAL.begin(9600, SERIAL_8N1);
    init_ultrasonics();

    Motor_dualdrive.begin(1, MOTOR_DRIVER_SERIAL);
    Sensor_module.begin(5, SENSORS_SERIAL);
    JY61P.startIIC();
    JY61P.caliIMU();

    threads.addThread(control_task);
    threads.addThread(recive_data_task);
    threads.addThread(imu_update_task);
    threads.addThread(send_data_task);
    threads.addThread(safty_task);
    threads.addThread(LED_task);
    threads.addThread(sensor_module_task);
}

void loop() {}

void imu_update_task(void *arg)
{
    float imu_gyro_dps[3] = {0};
    float imu_accel_g[3] = {0};

    while (1)
    {   
        uint64_t start_time = millis(); 

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

        memcpy(imu2send, cmd, sizeof(cmd));
        imu_ready = true;

        uint64_t end_time = millis();
        uint32_t dt = end_time - start_time;
        // Serial.println(dt);

        // send_data(FUNC_IMU, cmd, sizeof(cmd));
        threads.delay(45 - dt);
    }
}

void control_task(void *arg)
{
    bool emer_flag;

    while (1)
    {
        uint64_t start_time = millis();  
        uint8_t result;
        int16_t buff;
        float current_rpm_left;
        float current_rpm_right;
         
        if ((millis() - prev_cmd_time > 100) || stop || emer_state){
            cmd_vel.linear_x  = 0.0;
            cmd_vel.linear_y  = 0.0;
            cmd_vel.angular_z = 0.0;            
        }
        
        Kinematics::rpm req_rpm = kinematics.getRPM(
        cmd_vel.linear_x, 
        cmd_vel.linear_y, 
        cmd_vel.angular_z);  

        if(!emer_state){
            if (emer_flag) {
                if (connection_failed) {
                    connection_failed = false;
                    mcp.digitalWrite(7, HIGH); // Enable motor driver after reset with emer switch
                }
                delay(8000); 
                emer_flag = false;
            }
            if (!connection_failed) {
                if( Motor_dualdrive.readHoldingRegisters(REG_AXIS2_FEEDBACK_SPEED,1) == Motor_dualdrive.ku8MBSuccess ){
                    buff = Motor_dualdrive.getResponseBuffer(0);
                    current_rpm_left = (buff / 30.0) * (-1);
                    delay(1);
                } else {
                    mcp.digitalWrite(7, LOW); // Disable motor driver when connection is failed.
                    connection_failed = true;
                    Serial.println("Connection is failed");
                    break;
                }
                if( Motor_dualdrive.readHoldingRegisters(REG_AXIS1_FEEDBACK_SPEED,1) == Motor_dualdrive.ku8MBSuccess ){
                    buff = Motor_dualdrive.getResponseBuffer(0);
                    current_rpm_right = (buff / 30.0) ;
                    delay(1);
                } else {
                    mcp.digitalWrite(7, LOW); // Disable motor driver when connection is failed.
                    connection_failed = true;
                    Serial.println("Connection is failed");
                    break;
                }
                Motor_dualdrive.setTransmitBuffer(0, 0x0003);
                Motor_dualdrive.setTransmitBuffer(1, int16_t(req_rpm.motor2 * 30));
                Motor_dualdrive.setTransmitBuffer(2, int16_t(req_rpm.motor1 * 30));
                Motor_dualdrive.writeMultipleRegisters(REG_DUALAXIS_CMD_SPEED, 3);
                delay(1);
            }
        } else emer_flag = true;

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

        // send_data(FUNC_ODOM, cmd, sizeof(cmd));
        
        memcpy(odom2send, cmd, sizeof(cmd));
        odom_ready = true;

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
        if (dt >= 45){dt = 0;}
        threads.delay(45-dt);
    }    
}

void init_ultrasonics(){
    // ULTARSONICS_SERIAL.begin(9600, SERIAL_8N1, U0_RXD, U0_TXD);

    Ultrasonics_1.begin(1, ULTARSONICS_SERIAL);
    Ultrasonics_2.begin(2, ULTARSONICS_SERIAL);
    Ultrasonics_3.begin(3, ULTARSONICS_SERIAL);

    // Ultrasonics_1.writeSingleRegister(0x0201,3);
    // delay(10);
    // Ultrasonics_2.writeSingleRegister(0x0201,3);
    // delay(10);
    // Ultrasonics_3.writeSingleRegister(0x0201,3);
}

void sensor_module_task()
{
    int16_t buff;
    uint8_t alarm_mode, led_mode;
    uint64_t prev_batt;
    int16_t voltage, current;
    int16_t percentage;
    uint8_t batt_status;

    while(true){

        uint64_t start_time = millis();

        if( Ultrasonics_1.readHoldingRegisters(0x0101,1) == Ultrasonics_1.ku8MBSuccess ){
            buff = Ultrasonics_1.getResponseBuffer(0);
            if (buff > 500 || buff < 0)  buff = 495;
            range_left = buff;
            if(DEBUG_RANGE){
                Serial.print(buff);
                Serial.print("  ");
            }
        }
        else{ if(DEBUG_RANGE) Serial.println("Read range_1 error"); }
        delay(10);

        if( Ultrasonics_2.readHoldingRegisters(0x0101,1) == Ultrasonics_2.ku8MBSuccess ){
            buff = Ultrasonics_2.getResponseBuffer(0);
            if (buff > 500 || buff < 0)  buff = 495;
            range_center = buff;
            if(DEBUG_RANGE){
                Serial.print(buff);
                Serial.print("  ");
            }
        }
        else{ if(DEBUG_RANGE) Serial.println("Read range_2 error"); }
        delay(10);

        if( Ultrasonics_3.readHoldingRegisters(0x0101,1) == Ultrasonics_3.ku8MBSuccess ){
            buff = Ultrasonics_3.getResponseBuffer(0);
            if (buff > 500 || buff < 0)  buff = 495;
            range_right = buff;
            if(DEBUG_RANGE){
                Serial.print(buff);
                Serial.print("  ");
            }
        }
        else{ if(DEBUG_RANGE) Serial.println("Read range_3 error"); }
        delay(50);

        if( Sensor_module.readHoldingRegisters(0x00,1) == Sensor_module.ku8MBSuccess ){
            buff = Sensor_module.getResponseBuffer(0);
            cliff = buff;
            if(DEBUG_RANGE){
                Serial.print(buff);
                Serial.println("  ");
            }
        }
        else{ if(DEBUG_RANGE) Serial.println("Read cliff error"); }
        delay(10); 

        bool A = (range_left   < range_limit) && (range_left   > 0);
        bool B = (range_center < range_limit) && (range_center > 0);
        bool C = (range_right  < range_limit) && (range_right  > 0);

        if (stop || emer_state || connection_failed) A = B = C = true;

        uint8_t alarm_mode = (static_cast<uint8_t>(A) << 2 ) | 
                             (static_cast<uint8_t>(B) << 1 ) |
                             (static_cast<uint8_t>(C));

        if (cmd_vel.linear_x == 0 && cmd_vel.angular_z == 0){
            led_mode = 0;
        }else if (cmd_vel.linear_x != 0 && cmd_vel.angular_z == 0){
            led_mode = 1;
        }else if (((cmd_vel.angular_z < -0.2) && (cmd_vel.linear_x >= 0)) || ((cmd_vel.angular_z > 0.2) && (cmd_vel.linear_x < 0))){
            led_mode = 2;
        }else if (((cmd_vel.angular_z > 0.2) && (cmd_vel.linear_x >= 0)) || ((cmd_vel.angular_z < -0.2) && (cmd_vel.linear_x < 0))){
            led_mode = 3;
        }

        if (led_mode != led_mode_prev){
            if(DEBUG_OLED) Serial.println(led_mode);
            Sensor_module.writeSingleRegister(2, led_mode);
            delay(10);
        }        
        Sensor_module.writeSingleRegister(1, alarm_mode);
        delay(10);

        led_mode_prev = led_mode;
        alarm_mode_prev = alarm_mode;
        A = B = C = false;

        if (start_time - prev_batt > 1000){
            if(RequestDataFromBMD(VOLT_AMP_CMD)==1)
            {
                voltage = static_cast<int16_t>(fBattVolt * 100);
                current = static_cast<int16_t>(fBattCurrent * 100);
                percentage = static_cast<int16_t>(fBattSOC * 100);

                if (fBattSOC >= 100)        batt_status = 4; // FULL
                else if (fBattCurrent > 0)  batt_status = 1; // CHARGIGN
                else if (fBattCurrent < 0)  batt_status = 2; // DISCHARGIGN  
                else batt_status = 0;                        // UNKNOWN

            }
            uint8_t cmd_batt[7] = {
                static_cast<uint8_t>(voltage & 0xFF), static_cast<uint8_t>((voltage >> 8) & 0xFF),
                static_cast<uint8_t>(current & 0xFF), static_cast<uint8_t>((current >> 8) & 0xFF),
                static_cast<uint8_t>(percentage & 0xFF), static_cast<uint8_t>((percentage >> 8) & 0xFF),
                static_cast<uint8_t>(batt_status & 0xFF)             
            };

            memcpy(batt2send, cmd_batt, sizeof(cmd_batt));
            batt_ready = true;

            prev_batt = start_time;
        }

        uint8_t cmd_range[6] = {
                static_cast<uint8_t>(range_left   & 0xFF), static_cast<uint8_t>((range_left   >> 8) & 0xFF),
                static_cast<uint8_t>(range_center & 0xFF), static_cast<uint8_t>((range_center >> 8) & 0xFF),
                static_cast<uint8_t>(range_right  & 0xFF), static_cast<uint8_t>((range_right  >> 8) & 0xFF)
            }; 

        uint64_t dt = millis() - start_time;

        if(DEBUG_RANGE){
            Serial.print("dt : ");
            Serial.println(dt);
        }        
        memcpy(range2send, cmd_range, sizeof(cmd_range));
        range_ready = true;

        threads.delay(50);
    }
}

void LED_task(){

    while(1){
        // for (int i = 0; i < sizeof(ledPins); i++) {
        //     digitalWrite(ledPins[i], HIGH);
        //     delay(1000);
        //     digitalWrite(ledPins[i], LOW);
        // }

        digitalWrite(LED_PIN_STATUS, HIGH);
        delay(1000);
        digitalWrite(LED_PIN_STATUS, LOW);

        threads.delay(500);
    }
}

void safty_task()
{
    while(1)
    {   
        emer_state   = !mcp.digitalRead(8);
        bumper_state = !mcp.digitalRead(9) || !mcp.digitalRead(10);
    
        if (cliff > 70.0){cliff_state = true;} // 50 mm. for flat surface
        else {cliff_state = false;}

        if(bumper_state || cliff_state){stop = true;}
        else stop = false;

        if (DEBUG_SAFTY){
            Serial.print(bumper_state);
            Serial.print(" ");
            Serial.print(cliff);
            Serial.print(" ");
            Serial.println(emer_state);
        }
        threads.delay(50);
    }
}


