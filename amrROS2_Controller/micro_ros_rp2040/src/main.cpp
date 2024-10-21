#include <Arduino.h>
#include <micro_ros_platformio.h>

#include <rcl/rcl.h>
#include <rclc/rclc.h>
#include <rclc/executor.h>

#include <micro_ros_utilities/string_utilities.h>
#include <rmw_microros/rmw_microros.h>

#include <std_msgs/msg/bool.h>
#include <std_msgs/msg/u_int64.h>
#include <sensor_msgs/msg/range.h>
#include <Ultrasonic.h>

#include <Wire.h>
#include <ACROBOTIC_SSD1306.h>
#include <Adafruit_VL6180X.h>
#include <Int64String.h>


#if !defined(MICRO_ROS_TRANSPORT_ARDUINO_SERIAL)
#error This example is only avaliable for Arduino framework with serial transport.
#endif

#define CURRENT_SENSOR_AI 28
#define CURRENT_SENSOR_DI 7
// #define BtnGPIO 20 //Topic Add

const char * robot_namespace = "miniRobot_1";

rcl_publisher_t publisher_range_left, publisher_range_center, publisher_range_right;
rcl_publisher_t publisher_charge_state, publisher_cliff_state;//Topic add
// rcl_publisher_t publisher_charge_state, publisher_cliff_state, publisher_btn_state;//Topic add

rcl_subscription_t ip_subscriber ;
sensor_msgs__msg__Range msg_range_left, msg_range_center, msg_range_right;
std_msgs__msg__UInt64 msg_ip;
std_msgs__msg__Bool msg_charge_state, msg_cliff_state;
// std_msgs__msg__Bool msg_charge_state, msg_cliff_state, msg_btn_state;//Topic add

rclc_executor_t executor;
rclc_support_t support;
rcl_allocator_t allocator;
rcl_node_t node;
rcl_timer_t timer;

#define RCCHECK(fn) { rcl_ret_t temp_rc = fn; if((temp_rc != RCL_RET_OK)){error_loop();}}
#define RCSOFTCHECK(fn) { rcl_ret_t temp_rc = fn; if((temp_rc != RCL_RET_OK)){}}

Ultrasonic ranger[3] = {
  Ultrasonic(5),
  Ultrasonic(3),
  Ultrasonic(1)
};

Adafruit_VL6180X vl = Adafruit_VL6180X();

String ip_address = "";
bool chargeState;

unsigned long long time_offset = 0;
// Timeout for each ping attempt
const int timeout_ms = 100;
// Number of ping attempts
const uint8_t attempts = 1;
// Spin period
const unsigned int spin_timeout = RCL_MS_TO_NS(100);
// Enum with connection status
enum states {
    WAITING_AGENT,
    AGENT_AVAILABLE,
    AGENT_CONNECTED,
    AGENT_DISCONNECTED
} state;

// Error handle loop
void error_loop() {
  while(1) {
    delay(100);
  }
}

// Calculate the time difference between the microROS agent and the MCU
static void sync_time(void)
{
    unsigned long now = millis();
    RCSOFTCHECK(rmw_uros_sync_session(10));
    unsigned long long ros_time_ms = rmw_uros_epoch_millis();
    time_offset = ros_time_ms - now;
}

// Get timestamp
struct timespec get_timespec(void)
{
    struct timespec tp = {0};
    // deviation of synchronous time
    unsigned long long now = millis() + time_offset;
    tp.tv_sec = now / 1000;
    tp.tv_nsec = (now % 1000) * 1000000;
    return tp;
}

void timer_callback(rcl_timer_t * timer, int64_t last_call_time) {
  
  RCLC_UNUSED(last_call_time);
  float range[3];
  String warn_msg;

  for (int i = 0; i < 3; i++){
    range[i] = ranger[i].MeasureInCentimeters() * 0.01;
  }

  struct timespec time_stamp = get_timespec();

  msg_range_left.header.stamp.sec = time_stamp.tv_sec;
  msg_range_center.header.stamp.sec = time_stamp.tv_sec;
  msg_range_right.header.stamp.sec = time_stamp.tv_sec;
  
  msg_range_left.header.stamp.nanosec = time_stamp.tv_nsec;
  msg_range_center.header.stamp.nanosec = time_stamp.tv_nsec;
  msg_range_right.header.stamp.nanosec = time_stamp.tv_nsec;

  msg_range_left.range =range[0];
  msg_range_center.range = range[1];
  msg_range_right.range = range[2];

  chargeState = analogRead(CURRENT_SENSOR_AI) > 525;
  msg_charge_state.data = chargeState;
  msg_cliff_state.data = vl.readRange() > 50; 
  // msg_btn_state.data = digitalRead(BtnGPIO);
  if(msg_cliff_state.data){ warn_msg = "FOUND THE CLIFF!";}
  else if(chargeState){ warn_msg = " -- CHARGING -- ";}
  else{ warn_msg = "                ";}

  oled.setTextXY(1,0);
  oled.putString(warn_msg);

  if (timer != NULL) {
    RCSOFTCHECK(rcl_publish(&publisher_range_left, &msg_range_left, NULL));
    RCSOFTCHECK(rcl_publish(&publisher_range_center, &msg_range_center, NULL));
    RCSOFTCHECK(rcl_publish(&publisher_range_right, &msg_range_right, NULL));
    RCSOFTCHECK(rcl_publish(&publisher_charge_state, &msg_charge_state, NULL));
    RCSOFTCHECK(rcl_publish(&publisher_cliff_state, &msg_cliff_state, NULL));
    // RCSOFTCHECK(rcl_publish(&publisher_btn_state, &msg_btn_state, NULL)); //Topic
  }
}

void ip_callback(const void * msgin){

  String new_msg = int64String(msg_ip.data);

  if (ip_address != new_msg){
    ip_address = new_msg;
    int length = ip_address.length();
    int status = ip_address.substring(length-1).toInt();
    int i = 13 -length;
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

    String ip2display = ip_address.substring(0, 3-i) + ":" + ip_address.substring(3-i, 6-i) + ":" + ip_address.substring(6-i, 9-i) + ":" + ip_address.substring(9-i, 12-i);

    oled.clearDisplay();              
    oled.setTextXY(3,1);   
    oled.putString(ip2display);

    oled.setTextXY(5,0);
    oled.putString("STATUS:" + str_status); 
  }
}

void range_msgs_init(sensor_msgs__msg__Range &range_msg, const char *frame_id_name){
  range_msg.header.frame_id = micro_ros_string_utilities_set(range_msg.header.frame_id, frame_id_name);
  range_msg.radiation_type = sensor_msgs__msg__Range__ULTRASOUND;
  range_msg.field_of_view = 20 * (3.14/180); // rad
  range_msg.min_range = 0.03; // m
  range_msg.max_range = 0.5; // m  
}

bool create_entities(void) {
  
  allocator = rcl_get_default_allocator();
  RCCHECK(rclc_support_init(&support, 0, NULL, &allocator));

  // create node
  RCCHECK(rclc_node_init_default(&node, "micro_ros_raspico_node", robot_namespace, &support));

  // create publisher
  RCCHECK(rclc_publisher_init_default(
    &publisher_range_left,
    &node,
    ROSIDL_GET_MSG_TYPE_SUPPORT(sensor_msgs, msg, Range),
    "range_left"));
  
  //Topic add
  // RCCHECK(rclc_publisher_init_default(
  //   &publisher_btn_state,
  //   &node,
  //   ROSIDL_GET_MSG_TYPE_SUPPORT(std_msgs, msg, Bool),
  //   "Btn_Click"));
  // //Topic add


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
  RCCHECK(rclc_publisher_init_default(
    &publisher_charge_state,
    &node,
    ROSIDL_GET_MSG_TYPE_SUPPORT(std_msgs, msg, Bool),
    "charge_state"));
  RCCHECK(rclc_publisher_init_default(
    &publisher_cliff_state,
    &node,
    ROSIDL_GET_MSG_TYPE_SUPPORT(std_msgs, msg, Bool),
    "cliff_state"));

  RCCHECK(rclc_subscription_init_default(
      &ip_subscriber,
      &node,
      ROSIDL_GET_MSG_TYPE_SUPPORT(std_msgs, msg, UInt64),
      "ip_address"));  

  // create timer,
  const unsigned int timer_timeout = 2;
  RCCHECK(rclc_timer_init_default(
    &timer,
    &support,
    RCL_MS_TO_NS(timer_timeout),
    timer_callback));

  // create executor
  executor = rclc_executor_get_zero_initialized_executor();
  RCCHECK(rclc_executor_init(&executor, &support.context, 5, &allocator));
  RCCHECK(rclc_executor_add_timer(&executor, &timer));

  // Add a subscriber to the executor
  RCCHECK(rclc_executor_add_subscription(
      &executor,
      &ip_subscriber,
      &msg_ip,
      &ip_callback,
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
  RCCHECK(rcl_publisher_fini(&publisher_charge_state, &node));
  RCCHECK(rcl_publisher_fini(&publisher_cliff_state, &node));
  // RCCHECK(rcl_publisher_fini(&publisher_btn_state, &node)); //Topic add
  RCCHECK(rcl_subscription_fini(&ip_subscriber, &node));
  RCCHECK(rcl_node_fini(&node)); 
  RCCHECK(rcl_timer_fini(&timer));
  RCCHECK(rclc_executor_fini(&executor));
  RCCHECK(rclc_support_fini(&support));

}

void setup() {

  Wire.begin();	
  vl.begin(&Wire1);
  oled.init();                      // Initialze SSD1306 OLED display
  oled.clearDisplay();              // Clear screen
  oled.setTextXY(3,3);   
  oled.putString("NECTEC SMR");

  // Configure serial transport
  Serial.begin(921600);
  set_microros_serial_transports(Serial);
  delay(2000);

  pinMode(CURRENT_SENSOR_DI, INPUT);
  // pinMode(BtnGPIO, INPUT_PULLUP); // Add topic
  range_msgs_init(msg_range_left, "left_ranger_link");
  range_msgs_init(msg_range_center, "center_ranger_link");
  range_msgs_init(msg_range_right, "right_ranger_link");
  
}

void loop() {

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
                RCSOFTCHECK(rclc_executor_spin_some(&executor, RCL_MS_TO_NS(100)));}
            break;

        case AGENT_DISCONNECTED:
            // Connection is lost, destroy entities and go back to first step
            destroy_entities();
            state = WAITING_AGENT;
            break;

        default:
            break;
    }

}
