
#include <Arduino.h>
#include <Wire.h>
#include <vector>
#include "math.h"

#include "kinematics.h"
#include "motor.h"
#include <encoder.h>
#include "pid.h"

#include "JY61P.h"
#include <Ultrasonic.h>
#include <ACROBOTIC_SSD1306.h>
#include <Adafruit_VL6180X.h>
#include <Adafruit_INA219.h>

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

#define EMER_PIN    37
#define BUMPER_PIN  8

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

#define PWM_M1      15
#define DIR_M1      16
#define PWM_M2      4
#define DIR_M2      5

#define H1A         48
#define H1B         47
#define H2A         6
#define H2B         7

#define BASE DIFFERENTIAL_DRIVE  
#define MOTOR_MAX_RPM           67 
#define MAX_RPM_RATIO           0.85
#define WHEEL_DIAMETER          0.127            
#define LR_WHEELS_DISTANCE      0.121
#define MOTOR_OPERATING_VOLTAGE 12
#define MOTOR_POWER_MAX_VOLTAGE 12 

#define PWM_NEG_M1 -120
#define PWM_POS_M1 120
#define PWM_NEG_M2 -120
#define PWM_POS_M2 120

const int freq = 21000;
const int PWM_CH_MOTOR_LEFT = 0;
const int PWM_CH_MOTOR_RIGHT = 0;
const int resolution = 8;
int dutyCycle = 130;

#define PWM_MAX pow(2, resolution)-1
#define PWM_MIN -PWM_MAX
#define K_P 4.0
#define K_I 2.0
#define K_D 0

QueueHandle_t xSendDataMutex;

unsigned long prev_cmd_time = 0;
unsigned long prev_ip_time  = 0;

Motor motor1(PWM_M1, DIR_M1, 0, freq, resolution);
Motor motor2(PWM_M2, DIR_M2, 1, freq, resolution);

Encoder* Encoder::instance0_ ;
Encoder* Encoder::instance1_ ;
Encoder* Encoder::instance2_ ;
Encoder* Encoder::instance3_ ;

Encoder encoder1(0, H1A, H1B, 6114);
Encoder encoder2(1, H2A, H2B, 6114);

Kinematics kinematics(
    Kinematics::BASE,
    MOTOR_MAX_RPM,
    MAX_RPM_RATIO,
    MOTOR_OPERATING_VOLTAGE,
    MOTOR_POWER_MAX_VOLTAGE,
    WHEEL_DIAMETER,
    LR_WHEELS_DISTANCE
);

Kinematics::velocities cmd_vel;

PID pid_motor1(PWM_MIN, PWM_MAX, K_P, K_I, K_D);
PID pid_motor2(PWM_MIN, PWM_MAX, K_P, K_I, K_D);

Ultrasonic ultrasonic[3] = {
  Ultrasonic(41),
  Ultrasonic(42),
  Ultrasonic(45)
};

Adafruit_VL6180X vl = Adafruit_VL6180X();
Adafruit_INA219 ina219;

String ip_address, prev_ip; 
uint8_t status, prev_status;

bool cliff_state, bumper_state, emer_state, stop, ack;

void parse_data(uint8_t func, std::vector<uint8_t> data){

    if (func == FUNC_MOTION){

        int16_t packed_v_x = (data[1] << 8) | data[0];
        int16_t packed_v_y = (data[3] << 8) | data[2];
        int16_t packed_w_z = (data[5] << 8) | data[4];

        cmd_vel.linear_x  = packed_v_x/1000.0;
        cmd_vel.linear_y  = packed_v_y/1000.0;
        cmd_vel.angular_z = packed_w_z/1000.0;

        prev_cmd_time = millis();

        if(DEBUG_MOTION){
            Serial1.printf("Vx: %f\n", cmd_vel.linear_x);
            Serial1.printf("Vy: %f\n", cmd_vel.linear_y);
            Serial1.printf("Wz: %f\n", cmd_vel.angular_z);            
        }
    }
    else if(func == FUNC_IP){

        String ipPart1 = String(data[0]);
        String ipPart2 = String(data[1]);
        String ipPart3 = String(data[2]);
        String ipPart4 = String(data[3]);

        ip_address = ipPart1 + ":" + ipPart2 + ":" + ipPart3 + ":" + ipPart4;
        prev_ip_time = millis();
     
        if(DEBUG_OLED){
            Serial1.printf("IP Address: %s\n", ip_address);                
        }
    }
    else if(func == FUNC_STATUS){

        status = data[0];
     
        if(DEBUG_OLED){
            Serial1.printf("Status: %d\n", status);                
        }
    }
    else{
        if(DEBUG_RECEIVE){
            Serial1.println("Out of provided function");
        }
    }
}

void recive_data_task(void * parameter){

    uint8_t header;
    uint8_t device_id;
    uint8_t len;
    uint8_t func;
    uint8_t data_len;
    uint8_t value;
    uint8_t rx_check_num;
    uint8_t check_sum;  
    std::vector<uint8_t> data;

    while(true){

        if (Serial0.available() > 0) {
            header = Serial0.read();

            if (header == HEAD) {
                if(DEBUG_RECEIVE){
                    Serial1.printf("--------------New Data--------------\n");
                    Serial1.printf("Correct header %d\n", header);
                }
                device_id = Serial0.read();

                if (device_id == DEVICE_ID) {

                    len = Serial0.read();
                    func = Serial0.read();

                    check_sum = header + device_id + len + func;
                    data_len = len - 4;
                    data = {}; 

                    while (data.size() < data_len) {  

                        value = Serial0.read();
                        data.push_back(value);
                        check_sum += value;

                        if (data.size() == data_len) {
                            rx_check_num = Serial0.read();                

                            if ((check_sum & 0xFF) == rx_check_num){
                                if(DEBUG_RECEIVE){
                                    Serial1.println("Data Recived");
                                }
                                delay(1);
                                parse_data(func, data);
                            }
                            else {
                                if(DEBUG_RECEIVE){
                                    Serial1.println("Check sum error");
                                }
                            } 
                        }                         
                    }
                    if(DEBUG_RECEIVE){
                        Serial1.printf("Device_id:  %d\n", device_id);
                        Serial1.printf("Data range: %d\n", len);
                        Serial1.printf("Function:   %d\n", func);
                        for (int i=0; i < data.size(); i++){
                            Serial1.printf("Data %d:    %d\n", i, data[i]);
                        }
                        Serial1.printf("Rx check :  %d\n", rx_check_num);
                        Serial1.printf("Check sum:  %d\n", check_sum & 0xFF);                     
                    }
                }
            }
        }     
        vTaskDelay(pdMS_TO_TICKS(10));
    }
    vTaskDelete(NULL);
}

void send_data(uint8_t FUNC_TYPE, std::vector<uint8_t> param) {

    if (xSemaphoreTake(xSendDataMutex, pdMS_TO_TICKS(10)) == pdTRUE){

        std::vector<uint8_t> cmd = {HEAD, HOST_ID, 0x00, FUNC_TYPE};
        cmd.insert(cmd.end(), param.begin(), param.end());
        cmd[2] = cmd.size(); 

        uint8_t checksum = 0;

        for (uint8_t byte : cmd) {
            checksum += byte;
        }

        checksum &= 0xFF;  
        cmd.push_back(checksum);

        for (size_t i = 0; i < cmd.size(); i++) {
            Serial0.write(cmd[i]);
        }

        if(DEBUG_SEND){
            Serial1.println("Sent command:");
            for (size_t i = 0; i < cmd.size(); i++) {
                Serial1.printf(" %d", cmd[i]);
            }
            Serial1.println();
        }
    xSemaphoreGive(xSendDataMutex);
    }
}

void imu_update_task(void *arg)
{
    float imu_gyro_dps[3] = {0};
    float imu_accel_g[3] = {0};

    while (1)
    {   
        // imu_gyro_dps[0] = JY61P.getRoll();
        // imu_gyro_dps[1] = JY61P.getPitch();
        // imu_gyro_dps[2] = JY61P.getYaw();

        imu_gyro_dps[0] = (JY61P.getGyroX() / 180) * 3.14;
        imu_gyro_dps[1] = (JY61P.getGyroY() / 180) * 3.14;
        imu_gyro_dps[2] = (JY61P.getGyroZ() / 180) * 3.14;

        imu_accel_g[0] = JY61P.getAccX();
        imu_accel_g[1] = JY61P.getAccY();
        imu_accel_g[2] = JY61P.getAccZ();
	    
        if (DEBUG_IMU){
            Serial1.printf(">Roll  :%f\n", imu_gyro_dps[0]);
            Serial1.printf(">Pitch :%f\n", imu_gyro_dps[1]);
            Serial1.printf(">Yaw   :%f\n", imu_gyro_dps[2]);
            Serial1.printf(">AccX  :%f\n", imu_accel_g[0]);
            Serial1.printf(">AccY  :%f\n", imu_accel_g[1]);
            Serial1.printf(">AccZ  :%f\n", imu_accel_g[2]);       
        }

        int16_t roll   = static_cast<int16_t>(imu_gyro_dps[0]*1000);
        int16_t pitch  = static_cast<int16_t>(imu_gyro_dps[1]*1000);
        int16_t yaw    = static_cast<int16_t>(imu_gyro_dps[2]*1000);

        int16_t acc_x  = static_cast<int16_t>(imu_accel_g[0]*1000);
        int16_t acc_y  = static_cast<int16_t>(imu_accel_g[1]*1000);
        int16_t acc_z  = static_cast<int16_t>(imu_accel_g[2]*1000);

        std::vector<uint8_t> cmd = {
            static_cast<uint8_t>(roll  & 0xFF), static_cast<uint8_t>((roll  >> 8) & 0xFF),
            static_cast<uint8_t>(pitch & 0xFF), static_cast<uint8_t>((pitch >> 8) & 0xFF),
            static_cast<uint8_t>(yaw   & 0xFF), static_cast<uint8_t>((yaw   >> 8) & 0xFF),
            static_cast<uint8_t>(acc_x & 0xFF), static_cast<uint8_t>((acc_x >> 8) & 0xFF),
            static_cast<uint8_t>(acc_y & 0xFF), static_cast<uint8_t>((acc_y >> 8) & 0xFF),
            static_cast<uint8_t>(acc_z & 0xFF), static_cast<uint8_t>((acc_z >> 8) & 0xFF)
        };

        send_data(FUNC_IMU, cmd);
        vTaskDelay(pdMS_TO_TICKS(10));
    }

    vTaskDelete(NULL);
}

void control_task(void *arg)
{
    while (1)
    {
        uint64_t start_time = millis();  
         

        if ((millis() - prev_cmd_time > 100) || stop || emer_state){
            cmd_vel.linear_x  = 0.0;
            cmd_vel.linear_y  = 0.0;
            cmd_vel.angular_z = 0.0;
        }
        
        Kinematics::rpm req_rpm = kinematics.getRPM(
        cmd_vel.linear_x, 
        cmd_vel.linear_y, 
        cmd_vel.angular_z);    
       
        float rpm_motor1 = encoder1.getRPM();
        float rpm_motor2 = encoder2.getRPM();

        motor1.drive((int)pid_motor1.compute(req_rpm.motor1, rpm_motor1));
        motor2.drive((int)pid_motor2.compute(req_rpm.motor2, rpm_motor2));

        Kinematics::velocities current_vel = kinematics.getVelocities(
            rpm_motor1, 
            rpm_motor2, 
            0, 0 
        );

        int16_t Vx  = static_cast<int16_t>(current_vel.linear_x *1000);
        int16_t Vy  = static_cast<int16_t>(current_vel.linear_y *1000);
        int16_t Wz  = static_cast<int16_t>(current_vel.angular_z*1000);

        std::vector<uint8_t> cmd = {
            static_cast<uint8_t>(Vx & 0xFF), static_cast<uint8_t>((Vx >> 8) & 0xFF),
            static_cast<uint8_t>(Vy & 0xFF), static_cast<uint8_t>((Vy >> 8) & 0xFF),
            static_cast<uint8_t>(Wz & 0xFF), static_cast<uint8_t>((Wz >> 8) & 0xFF)
        };

        send_data(FUNC_ODOM, cmd);

        uint64_t end_time = millis();
        uint32_t dt = end_time - start_time;

        if (DEBUG_ODOM){
            Serial1.printf(">Command Vx :%f\n", cmd_vel.linear_x);
            Serial1.printf(">Command Wz :%f\n", cmd_vel.angular_z);
            Serial1.printf(">Odom Vx :%f\n", current_vel.linear_x);
            Serial1.printf(">Odom Wz :%f\n", current_vel.angular_z);
        }

        if (DEBUG_MOTOR){
            Serial1.printf(">Command Motor 1:%f\n", req_rpm.motor1);
            Serial1.printf(">Command Motor 2:%f\n", req_rpm.motor2);
            Serial1.printf(">Current Motor 1:%f\n", rpm_motor1);
            Serial1.printf(">Current Motor 2:%f\n", rpm_motor2);
            Serial1.printf(">dt:%d\n", dt);
        }

        if (dt >= 20){dt = 0;}

        vTaskDelay(pdMS_TO_TICKS(20 - dt));
    }
    
}

float getMovingAverage(float* samples, uint8_t sample_size) {
  float sum = 0;
  for (int i = 0; i < sample_size; i++) {
    sum += samples[i];
  }
  return sum / sample_size;
}

void ranger_task(void *arg)
{
    float range[3];
    float filtered_range[3];
    uint8_t ut_sample_size = 10 ;
    float range_samples[3][ut_sample_size];
    uint16_t currentSample = 0;

    while (1)
    {
        for (int i = 0; i < 3; i++){

            uint64_t start_time = millis();

            range[i] = ultrasonic[i].MeasureInCentimeters();            
            if (range[i] > 50) {range[i] = 50;}

            range_samples[i][currentSample] = range[i];
            filtered_range[i] = getMovingAverage(range_samples[i], ut_sample_size);

            if (DEBUG_RANGE){
                uint64_t end_time = millis();
                uint32_t dt = end_time - start_time;
                Serial1.printf(">dt %d:%d\n", i, dt);
                Serial1.printf(">Range %d:%f\n", i, range[i]);
                Serial1.printf(">Filtered Range %d:%f\n", i, filtered_range[i]);
            }
        }  

        currentSample = (currentSample + 1) % ut_sample_size;

        int16_t range_right   = static_cast<int16_t>(filtered_range[0] *10);
        int16_t range_center = static_cast<int16_t>(filtered_range[1] *10);
        int16_t range_left  = static_cast<int16_t>(filtered_range[2] *10);

        std::vector<uint8_t> cmd = {
            static_cast<uint8_t>(range_left   & 0xFF), static_cast<uint8_t>((range_left   >> 8) & 0xFF),
            static_cast<uint8_t>(range_center & 0xFF), static_cast<uint8_t>((range_center >> 8) & 0xFF),
            static_cast<uint8_t>(range_right  & 0xFF), static_cast<uint8_t>((range_right  >> 8) & 0xFF)
        };

        send_data(FUNC_RANGE, cmd);
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}

void centerText(String text, uint8_t row) 
{   
    uint8_t rowWidth = 16;  
    uint8_t textLength = text.length();  
    uint8_t spaces = (rowWidth - textLength) / 2;  

    String paddedText = "";  
    for (uint8_t i = 0; i < spaces; i++) {
        paddedText += " "; 
    }
    paddedText += text + paddedText;

    oled.setTextXY(row,0);
    oled.putString(paddedText);
}

void oled_task(void *arg)
{     
    String warn_msg;
    bool connected = false; 
    bool flag = false;

    while(1)
    {
        Serial1.println(connected);
        if (connected){
            if( millis() - prev_ip_time > 15000 ){connected = false;}
            centerText(ip_address, 3);
            centerText(" ", 4);
            centerText(" ", 6);
        }
        else{
            if( millis() - prev_ip_time < 1000){
                if(flag){
                    connected = true;
                }
                flag = true;
            }
            centerText(" ", 3);
            centerText("WAITING  FOR", 4);
            centerText("CONNECTION", 6);
        }
        
        if(status != prev_status){
            String str_status;
            switch (status)
            {
                case 0 :
                str_status = " UNKNOWN";
                break;
                case 1 :
                str_status = " ACCEPTED";
                break;
                case 2 :
                str_status = "EXECUTING";
                break;
                case 3 :
                str_status = "CANCELING";
                break;
                case 4 :
                str_status = "SUCCEEDED";
                break;
                case 5 :
                str_status = " CANCELED";
                break;
                case 6 :
                str_status = " ABORTED";
                break;
                default:
                break;
            }
            if(status != 0){
                centerText("STATUS:" + str_status, 5);
            }
            else{
                centerText(" ", 5);
            }
            prev_status = status;
        }

        if (cliff_state){warn_msg = "CLIFF DETECTED";}
        else if(bumper_state){warn_msg = "BUMP DETECTED";}   
        else if(stop||emer_state){warn_msg = "EMERGENCY STOP";}
        else {warn_msg = "NECTEC SMR";}

        centerText(warn_msg, 1);

        vTaskDelay(pdMS_TO_TICKS(500));
    }
}

void safty_task(void *arg)
{
    while(1)
    {
        uint64_t cliff_start_time = millis(); 

        float cliff_range = vl.readRange();   
        // float cliff_range = 15.0;   

        bumper_state = !digitalRead(BUMPER_PIN);
        emer_state   = !digitalRead(EMER_PIN);

        if (cliff_range > 40.0){cliff_state = true;}
        else {cliff_state = false;}

        if(bumper_state || cliff_state){stop = true;}
        if(stop && emer_state){ack = true;}
        if(ack && !bumper_state && !cliff_state){ack = false; stop = false;}

        if (DEBUG_SAFTY){
            uint64_t cliff_end_time = millis();
            uint32_t cliff_dt = cliff_end_time - cliff_start_time;
            Serial1.printf(">Cliff range: %f\n", cliff_range);
            Serial1.printf(">Cliff dt: %d\n", cliff_dt);
            Serial1.printf("Bumper status:%d\n", bumper_state);
            Serial1.printf("Emergency status:%d\n", emer_state);
        }

        vTaskDelay(pdMS_TO_TICKS(20));
    }
}

void battery_task(void *arg)
{
    float previous_filtered_voltage;
    float previous_filtered_current;
    float alpha = 0.1;
    float offset = -0.09;

    float shuntvoltage = 0;
    float busvoltage = 0;
    float current_mA = 0;
    float loadvoltage = 0;
    float power_mW = 0;

    while(1)
    {
        shuntvoltage = ina219.getShuntVoltage_mV();
        busvoltage = ina219.getBusVoltage_V();
        current_mA = ina219.getCurrent_mA();
        loadvoltage = busvoltage + (shuntvoltage / 1000) + offset;

        float filtered_voltage = alpha * loadvoltage + (1.0 - alpha) * previous_filtered_voltage;
        float filtered_current = alpha * current_mA  + (1.0 - alpha) * previous_filtered_current;

        previous_filtered_voltage = filtered_voltage;
        previous_filtered_current = filtered_current;

        uint16_t voltage   = static_cast<int16_t>(filtered_voltage *1000);
        uint16_t current   = static_cast<int16_t>(filtered_current);

        std::vector<uint8_t> cmd = {
            static_cast<uint8_t>(voltage & 0xFF), static_cast<uint8_t>((voltage >> 8) & 0xFF),
            static_cast<uint8_t>(current & 0xFF), static_cast<uint8_t>((current >> 8) & 0xFF)            
        };

        send_data(FUNC_BATT, cmd);

        if (DEBUG_BATT){
            Serial1.printf(">Load Voltage    :  %f V\n"     , loadvoltage); 
            Serial1.printf(">Filtered Voltage:  %f V\n"     , filtered_voltage); 
            Serial1.printf(">Current         :  %f mA\n"    , current_mA); 
            Serial1.printf(">Filtered Current:  %f mA\n"    , filtered_current); 
        }
        vTaskDelay(pdMS_TO_TICKS(200));
    }
}

void setup() {

    Wire.begin(40,39);	
    vl.begin();
    ina219.begin();

    encoder1.begin();
    encoder2.begin();

    Serial0.begin(115200);                      // Serial port for communication
    Serial1.begin(115200, SERIAL_8N1, 35, 36);  // Serial port for monitoring

    pid_motor1.reset();
    pid_motor2.reset();

    pid_motor1.set_deadband_comp( PWM_POS_M1, PWM_NEG_M1 );
    pid_motor2.set_deadband_comp( PWM_POS_M2, PWM_NEG_M2 );

    JY61P.startIIC();
    JY61P.caliIMU();

    oled.init();                    
    oled.clearDisplay();            

    pinMode(BUMPER_PIN, INPUT_PULLUP);
    pinMode(EMER_PIN  , INPUT_PULLUP);

    delay(100);

    if(DEBUG){
        Serial0.println("Hello from Serial0");
        Serial1.println("Hello from Serial1");
    }

    Serial0.flush();
    xSendDataMutex = xSemaphoreCreateMutex();

    xTaskCreatePinnedToCore(imu_update_task , "imu_update_task" , 4096, NULL, 1, NULL, 0);  
    xTaskCreatePinnedToCore(recive_data_task, "recive_data_task", 4096, NULL, 1, NULL, 0);  
    xTaskCreatePinnedToCore(control_task    , "control_task"    , 4096, NULL, 1, NULL, 1);  
    xTaskCreatePinnedToCore(ranger_task     , "ranger_task"     , 4096, NULL, 1, NULL, 0);  
    xTaskCreatePinnedToCore(oled_task       , "oled_task"       , 4096, NULL, 1, NULL, 1); 
    // xTaskCreatePinnedToCore(battery_task    , "battery_task"    , 4096, NULL, 1, NULL, 1);   
    // xTaskCreatePinnedToCore(safty_task      , "safty_task"      , 4096, NULL, 1, NULL, 0);                 

}

void loop() {}
