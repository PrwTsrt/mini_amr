#include <Arduino.h>
#include <micro_ros_platformio.h>

#include <rcl/rcl.h>
#include <rclc/rclc.h>
#include <rclc/executor.h>

#include <micro_ros_utilities/string_utilities.h>
#include <rmw_microros/rmw_microros.h>

#include <geometry_msgs/msg/twist.h>
#include <sensor_msgs/msg/imu.h>
#include <sensor_msgs/msg/range.h>
#include <nav_msgs/msg/odometry.h>

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

#define RCCHECK(fn) { rcl_ret_t temp_rc = fn; if((temp_rc != RCL_RET_OK)){Serial1.printf("Failed status on line %d: %d. Aborting.\n",__LINE__,(int)temp_rc);vTaskDelete(NULL);}}
#define RCSOFTCHECK(fn) { rcl_ret_t temp_rc = fn; if((temp_rc != RCL_RET_OK)){Serial1.printf("Failed status on line %d: %d. Continuing.\n",__LINE__,(int)temp_rc);}}

const char * robot_namespace = "miniRobot";

rcl_subscription_t twist_subscriber;
rcl_publisher_t publisher_imu;
rcl_publisher_t publisher_odom;
rcl_publisher_t publisher_range_left, publisher_range_center, publisher_range_right;

sensor_msgs__msg__Imu msg_imu;
geometry_msgs__msg__Twist twist_msg;
nav_msgs__msg__Odometry msg_odom;
sensor_msgs__msg__Range msg_range_left, msg_range_center, msg_range_right;

rclc_executor_t executor;
rclc_support_t support;
rcl_allocator_t allocator;
rcl_node_t node;
rcl_timer_t timer_update;

rcl_init_options_t init_options;

enum states {
    WAITING_AGENT,
    AGENT_AVAILABLE,
    AGENT_CONNECTED,
    AGENT_DISCONNECTED
}state ; 

const int timeout_ms = 100;
const uint8_t attempts = 1;
const unsigned int spin_timeout = RCL_MS_TO_NS(100);

float imu_gyro_dps[3] = {0};
float imu_accel_g[3] = {0};
float range[3];

unsigned long long time_offset = 0;

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
#define ENC_PULSE_PER_REV       6114

#define PWM_NEG_M1 -120
#define PWM_POS_M1  120
#define PWM_NEG_M2 -120
#define PWM_POS_M2  120

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

Encoder encoder1(0, H1A, H1B, ENC_PULSE_PER_REV);
Encoder encoder2(1, H2A, H2B, ENC_PULSE_PER_REV);

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

Kinematics::velocities current_vel;

bool update_imu, update_odom, update_range, update_batt;

void imu_update_task(void *arg)
{
    // float imu_gyro_dps[3] = {0};
    // float imu_accel_g[3] = {0};

    while (1)
    {   
        imu_gyro_dps[0] = (JY61P.getGyroX() / 180) * 3.14;
        imu_gyro_dps[1] = (JY61P.getGyroY() / 180) * 3.14;
        imu_gyro_dps[2] = (JY61P.getGyroZ() / 180) * 3.14;

        imu_accel_g[0] = JY61P.getAccX();
        imu_accel_g[1] = JY61P.getAccY();
        imu_accel_g[2] = JY61P.getAccZ();

        msg_imu.angular_velocity.x = imu_gyro_dps[0];
        msg_imu.angular_velocity.y = imu_gyro_dps[1];
        msg_imu.angular_velocity.z = imu_gyro_dps[2];

        msg_imu.linear_acceleration.x = imu_accel_g[0];
        msg_imu.linear_acceleration.y = imu_accel_g[1];
        msg_imu.linear_acceleration.z = imu_accel_g[2];

        // RCSOFTCHECK(rcl_publish(&publisher_imu, &msg_imu, NULL));

        update_imu = true;
                
        if (DEBUG_IMU){
            Serial1.printf(">Roll  :%f\n", imu_gyro_dps[0]);
            Serial1.printf(">Pitch :%f\n", imu_gyro_dps[1]);
            Serial1.printf(">Yaw   :%f\n", imu_gyro_dps[2]);
            Serial1.printf(">AccX  :%f\n", imu_accel_g[0]);
            Serial1.printf(">AccY  :%f\n", imu_accel_g[1]);
            Serial1.printf(">AccZ  :%f\n", imu_accel_g[2]);       
        }
        vTaskDelay(pdMS_TO_TICKS(10));
    }

    vTaskDelete(NULL);
}

void twist_Callback(const void *msgin)
{
    prev_cmd_time = millis();
}

void control_task(void *arg)
{
    while (1)
    {
        uint64_t start_time = millis();  
         
        if ((millis() - prev_cmd_time > 100) || stop || emer_state){
            twist_msg.linear.x = 0.0;
            twist_msg.linear.y = 0.0;
            twist_msg.angular.z = 0.0;
        }
        
        Kinematics::rpm req_rpm = kinematics.getRPM(
        twist_msg.linear.x, 
        twist_msg.linear.y, 
        twist_msg.angular.z);    
       
        float rpm_motor1 = encoder1.getRPM();
        float rpm_motor2 = encoder2.getRPM();

        motor1.drive((int)pid_motor1.compute(req_rpm.motor1, rpm_motor1));
        motor2.drive((int)pid_motor2.compute(req_rpm.motor2, rpm_motor2));

        current_vel = kinematics.getVelocities(
            rpm_motor1, 
            rpm_motor2, 
            0, 0 
        );
        uint64_t end_time = millis();
        uint32_t dt = end_time - start_time;

        msg_odom.twist.twist.linear.x = current_vel.linear_x;
        msg_odom.twist.twist.angular.z = current_vel.angular_z;

        // RCSOFTCHECK(rcl_publish(&publisher_odom, &msg_odom, NULL));

        update_odom = true;

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
    // float range[3];
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

            msg_range_left.range =   range[0];
            msg_range_center.range = range[1];
            msg_range_right.range =  range[2];

            // RCSOFTCHECK(rcl_publish(&publisher_range_left, &msg_range_left, NULL));
            // RCSOFTCHECK(rcl_publish(&publisher_range_center, &msg_range_center, NULL));
            // RCSOFTCHECK(rcl_publish(&publisher_range_right, &msg_range_right, NULL));

            update_range = true;

            if (DEBUG_RANGE){
                uint64_t end_time = millis();
                uint32_t dt = end_time - start_time;
                Serial1.printf(">dt %d:%d\n", i, dt);
                Serial1.printf(">Range %d:%f\n", i, range[i]);
                Serial1.printf(">Filtered Range %d:%f\n", i, filtered_range[i]);
            }
        }  
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

        if(DEBUG_OLED){Serial1.println(connected);}

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

        // float cliff_range = vl.readRange();   
        float cliff_range = 15.0;   

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

        update_batt = true;

        if (DEBUG_BATT){
            Serial1.printf(">Load Voltage    :  %f V\n"     , loadvoltage); 
            Serial1.printf(">Filtered Voltage:  %f V\n"     , filtered_voltage); 
            Serial1.printf(">Current         :  %f mA\n"    , current_mA); 
            Serial1.printf(">Filtered Current:  %f mA\n"    , filtered_current); 
        }
        vTaskDelay(pdMS_TO_TICKS(200));
    }
}

void timer_update_callback(rcl_timer_t * timer, int64_t last_call_time) {
  
  RCLC_UNUSED(last_call_time);

//   msg_range_left.range =   range[0];
//   msg_range_center.range = range[1];
//   msg_range_right.range =  range[2];

//   msg_imu.angular_velocity.x  = imu_gyro_dps[0];
//   msg_imu.angular_velocity.y   = imu_gyro_dps[1];
//   msg_imu.angular_velocity.z = imu_gyro_dps[2];

//   msg_imu.linear_acceleration.x = imu_accel_g[0];
//   msg_imu.linear_acceleration.y = imu_accel_g[1];
//   msg_imu.linear_acceleration.z = imu_accel_g[2];

//   msg_odom.twist.twist.linear.x = current_vel.linear_x;
//   msg_odom.twist.twist.angular.z = current_vel.angular_z;

  if (timer != NULL) {
    if(update_imu){
        RCSOFTCHECK(rcl_publish(&publisher_imu, &msg_imu, NULL));
        update_imu = false;
    }
    if (update_odom){
        RCSOFTCHECK(rcl_publish(&publisher_odom, &msg_odom, NULL));
        update_odom = false;
    }
    if (update_range){
        RCSOFTCHECK(rcl_publish(&publisher_range_left, &msg_range_left, NULL));
        RCSOFTCHECK(rcl_publish(&publisher_range_center, &msg_range_center, NULL));
        RCSOFTCHECK(rcl_publish(&publisher_range_right, &msg_range_right, NULL));
        update_range = false;
    }

    // RCSOFTCHECK(rcl_publish(&publisher_range_left, &msg_range_left, NULL));
    // RCSOFTCHECK(rcl_publish(&publisher_range_center, &msg_range_center, NULL));
    // RCSOFTCHECK(rcl_publish(&publisher_range_right, &msg_range_right, NULL));
    // RCSOFTCHECK(rcl_publish(&publisher_imu, &msg_imu, NULL));
    // RCSOFTCHECK(rcl_publish(&publisher_odom, &msg_odom, NULL));
  }
}

static void sync_time(void)
{
    unsigned long now = millis();
    RCSOFTCHECK(rmw_uros_sync_session(10));
    unsigned long long ros_time_ms = rmw_uros_epoch_millis();
    time_offset = ros_time_ms - now;
}

bool create_entities(void) {

    allocator = rcl_get_default_allocator();   
    init_options = rcl_get_zero_initialized_init_options();

    RCCHECK(rcl_init_options_init(&init_options, allocator));

    size_t domain_id = 49;
    RCCHECK(rcl_init_options_set_domain_id(&init_options, domain_id));

    // create init_options
    RCCHECK(rclc_support_init_with_options(&support, 0, NULL, &init_options, &allocator));

    // create node
    RCCHECK(rclc_node_init_default(&node, "emr_micro_ros_node", robot_namespace, &support));

    // create publisher_odom
    RCCHECK(rclc_publisher_init_default(
        &publisher_odom,
        &node,
        ROSIDL_GET_MSG_TYPE_SUPPORT(nav_msgs, msg, Odometry),
        "odom_raw"));

    // create publisher_imu
    RCCHECK(rclc_publisher_init_default(
        &publisher_imu,
        &node,
        ROSIDL_GET_MSG_TYPE_SUPPORT(sensor_msgs, msg, Imu),
        "imu/data_raw"));

    // Create subscriber cmd_vel
    RCCHECK(rclc_subscription_init_default(
        &twist_subscriber,
        &node,
        ROSIDL_GET_MSG_TYPE_SUPPORT(geometry_msgs, msg, Twist),
        "cmd_vel"));

    RCCHECK(rclc_publisher_init_default(
        &publisher_range_left,
        &node,
        ROSIDL_GET_MSG_TYPE_SUPPORT(sensor_msgs, msg, Range),
        "range_left"));

    RCCHECK(rclc_publisher_init_default(
        &publisher_range_center,
        &node,
        ROSIDL_GET_MSG_TYPE_SUPPORT(sensor_msgs, msg, Range),
        "range_center"));

    RCCHECK(rclc_publisher_init_default(
        &publisher_range_right,
        &node,
        ROSIDL_GET_MSG_TYPE_SUPPORT(sensor_msgs, msg, Range),
        "range_right"));
    

    // create timer. Set the publish frequency to 20HZ
    const unsigned int timer_timeout = 5;
    RCCHECK(rclc_timer_init_default(
        &timer_update,
        &support,
        RCL_MS_TO_NS(timer_timeout),
        timer_update_callback));

    // create executor. Three of the parameters are the number of actuators controlled that is greater than or equal to the number of subscribers and publishers added to the executor.
    int handle_num = 6;
    executor = rclc_executor_get_zero_initialized_executor();
    RCCHECK(rclc_executor_init(&executor, &support.context, handle_num, &allocator));
        
    // Adds the publishers's timer to the executor
    RCCHECK(rclc_executor_add_timer(&executor, &timer_update));

    // Add a subscriber twist to the executor
    RCCHECK(rclc_executor_add_subscription(
        &executor,
        &twist_subscriber,
        &twist_msg,
        &twist_Callback,
        ON_NEW_DATA));
        
    sync_time();

    return true;
}

void destroy_entities(void){

  rmw_context_t * rmw_context = rcl_context_get_rmw_context(&support.context);
  (void) rmw_uros_set_context_entity_destroy_session_timeout(rmw_context, 0);

  RCCHECK(rcl_publisher_fini(&publisher_range_left, &node));
  RCCHECK(rcl_publisher_fini(&publisher_range_center, &node));
  RCCHECK(rcl_publisher_fini(&publisher_range_right, &node));
  RCCHECK(rcl_subscription_fini(&twist_subscriber, &node));
  RCCHECK(rcl_publisher_fini(&publisher_odom, &node));
  RCCHECK(rcl_publisher_fini(&publisher_imu, &node));
  RCCHECK(rcl_timer_fini(&timer_update));
  RCCHECK(rcl_node_fini(&node)); 
  RCCHECK(rclc_executor_fini(&executor));
  RCCHECK(rclc_support_fini(&support));
}

void micro_ros_task(void *arg)
{
    while (true)
    {
        switch (state)
        {
            case WAITING_AGENT:
                // Check for agent connection
                state = (RMW_RET_OK == rmw_uros_ping_agent(timeout_ms, attempts)) ? AGENT_AVAILABLE : WAITING_AGENT;
                break;

            case AGENT_AVAILABLE:
                // Create micro-ROS entities
                state = (true == create_entities()) ? AGENT_CONNECTED : WAITING_AGENT;

                if (state == WAITING_AGENT)
                {
                    // Creation failed, release allocated resources
                    destroy_entities();
                };
                break;

            case AGENT_CONNECTED:
                // Check connection and spin on success
                state = (RMW_RET_OK == rmw_uros_ping_agent(timeout_ms, attempts)) ? AGENT_CONNECTED : AGENT_DISCONNECTED;
                if (state == AGENT_CONNECTED)
                {
                    RCSOFTCHECK(rclc_executor_spin_some(&executor, spin_timeout));
                }
                break;

            case AGENT_DISCONNECTED:
                // Connection is lost, destroy entities and go back to first step
                destroy_entities();
                state = WAITING_AGENT;
                break;

            default:
                break;
        }
        vTaskDelay(pdMS_TO_TICKS(10));
    }

}

void setup() {

    Wire.begin(40,39);	
    vl.begin();
    ina219.begin();

    encoder1.begin();
    encoder2.begin();

    Serial0.begin(921600);                      // Serial port for communication
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

    set_microros_serial_transports(Serial0);

    delay(100);

    if(DEBUG){
        Serial0.println("Hello from Serial0");
        Serial1.println("Hello from Serial1");
    }

    // Serial0.flush();
    // xSendDataMutex = xSemaphoreCreateMutex();

    xTaskCreatePinnedToCore(imu_update_task , "imu_update_task" , 4096, NULL, 1, NULL, 0);  
    // xTaskCreatePinnedToCore(recive_data_task, "recive_data_task", 4096, NULL, 1, NULL, 0);  
    xTaskCreatePinnedToCore(micro_ros_task  , "micro_ros_task"  , 4096, NULL, 1, NULL, 0);  
    xTaskCreatePinnedToCore(control_task    , "control_task"    , 4096, NULL, 1, NULL, 1);  
    xTaskCreatePinnedToCore(ranger_task     , "ranger_task"     , 4096, NULL, 1, NULL, 0);  
    // xTaskCreatePinnedToCore(oled_task       , "oled_task"       , 4096, NULL, 1, NULL, 1); 
    // xTaskCreatePinnedToCore(battery_task    , "battery_task"    , 4096, NULL, 1, NULL, 1);   
    // xTaskCreatePinnedToCore(safty_task      , "safty_task"      , 4096, NULL, 1, NULL, 0);                 

}

void loop() {}
