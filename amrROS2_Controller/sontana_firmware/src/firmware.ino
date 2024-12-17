#define USE_SMR_CONFIG

#include <Arduino.h>
#include <TeensyThreads.h>

#include <stdio.h>
#include <vector>
#include "math.h"

#include "config.h"
#include "motor.h"
#include "kinematics.h"
#include "pid.h"
#include "IcMdEncoder.h" 

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

DriverIcMd counter_left( ENCODER_CS_LEFT );
DriverIcMd counter_right( ENCODER_CS_RIGHT );
Encoder motor_encoder_left = {0, 0, COUNTS_PER_REV_LEFT, MTR_ENCODER_INV_LEFT};
Encoder motor_encoder_right = {0, 0, COUNTS_PER_REV_RIGHT, MTR_ENCODER_INV_RIGHT};

Motor motor_controller_left(PWM_FREQUENCY, PWM_BITS, MTR_INV_LEFT, MTR_PWM_LEFT, MTR_FWD_LEFT, MTR_BKD_LEFT);
Motor motor_controller_right(PWM_FREQUENCY, PWM_BITS, MTR_INV_RIGHT, MTR_PWM_RIGHT, MTR_FWD_RIGHT, MTR_BKD_RIGHT);

PID motor_pid_left(PWM_MIN, PWM_MAX, K_P, K_I, K_D);
PID motor_pid_right(PWM_MIN, PWM_MAX, K_P, K_I, K_D);

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
            Serial5.printf("Vx: %f\n", cmd_vel.linear_x);
            Serial5.printf("Vy: %f\n", cmd_vel.linear_y);
            Serial5.printf("Wz: %f\n", cmd_vel.angular_z);            
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

void setup() 
{
    motor_pid_left.reset();
    motor_pid_right.reset();
    pinMode(LED_PIN_RUN, OUTPUT); // LED_PIN 33 is used for MicroMod Running
    pinMode(LED_PIN_STATUS, OUTPUT); 
    motor_pid_left.set_deadband_comp( PWM_POS_LEFT, PWM_NEG_LEFT );
    motor_pid_right.set_deadband_comp( PWM_POS_RIGHT, PWM_NEG_RIGHT );

    setup_encoders(counter_left, counter_right);

    Serial.begin(115200);
    Serial5.begin(115200);

    if(DEBUG){
        Serial.println("Hello from Serial");
        Serial5.println("Hello from Serial5");
    }

    Serial.flush();

    threads.addThread(control_task);
    threads.addThread(recive_data_task);
    
}

void loop() {}

void control_task(void *arg)
{
    while (1)
    {
        uint64_t start_time = millis();  
         
        if (millis() - prev_cmd_time > 100){
            cmd_vel.linear_x  = 0.0;
            cmd_vel.linear_y  = 0.0;
            cmd_vel.angular_z = 0.0;
        }
        
        Kinematics::rpm req_rpm = kinematics.getRPM(
        cmd_vel.linear_x, 
        cmd_vel.linear_y, 
        cmd_vel.angular_z);    
       
        float current_rpm_left = getRPM(motor_encoder_left, counter_left);
        float current_rpm_right = getRPM(motor_encoder_right, counter_right);

        // the required rpm is capped at -/+ MAX_RPM to prevent the PID from having too much error
        // the PWM value sent to the motor driver is the calculated PID based on required RPM vs measured RPM
        /*
        double left = motor_pid_left.compute(req_rpm.motor1, current_rpm_left);
        float64_msg.data = left;
        RCSOFTCHECK(rcl_publish(&float64_publisher, &float64_msg, NULL));

        motor_controller_left.spin(left);
        */
        motor_controller_left.spin(motor_pid_left.compute(req_rpm.motor1, current_rpm_left));
        motor_controller_right.spin(motor_pid_right.compute(req_rpm.motor2, current_rpm_right));

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
        
        // Serial5.println(dt);
        if (dt >= 20){dt = 0;}
        threads.delay(20-dt);
    }    
}

void flashLED(int n_times)
{
    for(int i=0; i<n_times; i++)
    {
        digitalWrite(LED_PIN_RUN, HIGH);
        delay(150);
        digitalWrite(LED_PIN_RUN, LOW);
        delay(150);
    }
    delay(1000);
}
