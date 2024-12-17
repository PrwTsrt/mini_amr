// Copyright (c) 2021 Juan Miguel Jimeno
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#ifndef SMR_BASE_CONFIG_H
#define SMR_BASE_CONFIG_H

#define LED_PIN_RUN 32
#define LED_PIN_STATUS 26
//uncomment the base you're building
#define SMR_BASE DIFFERENTIAL_DRIVE       // 2WD and Tracked robot w/ 2 motors
// #define SMR_BASE SKID_STEER            // 4WD robot
// #define SMR_BASE MECANUM               // Mecanum drive robot

//uncomment the IMU you're using
#define USE_GY85_IMU
// #define USE_MPU6050_IMU
// #define USE_MPU9150_IMU
// #define USE_MPU9250_IMU
#define USE_ZD_MOTOR_DRIVER

#define K_P 14.0                             // P constant
#define K_I 0.5                             // I constant
#define K_D 0.0                             // D constant

/*
echo "source /usr/share/colcon_cd/function/colcon_cd.sh" >> ~/.bashrc
echo "export _colcon_cd_root=/opt/ros/humble/" >> ~/.bashrcecho "source /usr/share/colcon_cd/function/colcon_cd.sh" >> ~/.bashrc
echo "export _colcon_cd_root=/opt/ros/humble/" >> ~/.bashrcecho "source /usr/share/colcon_cd/function/colcon_cd.sh" >> ~/.bashrc
echo "export _colcon_cd_root=/opt/ros/humble/" >> ~/.bashrcROBOT ORIENTATION
         FRONT
    MTR_LEFT  MTR_RIGHT  (2WD/ACKERMANN)
    MOTOR3  MOTOR4  (4WD/MECANUM)  
         BACK
*/

//define your robot' specs here
#define MOTOR_MAX_RPM 80                   // motor's max RPM          
#define MAX_RPM_RATIO 0.85                  // max RPM allowed for each MAX_RPM_ALLOWED = MOTOR_MAX_RPM * MAX_RPM_RATIO          

#define COUNTS_PER_REV_RIGHT 200000              // wheel1 encoder's no of ticks per rev
#define COUNTS_PER_REV_LEFT 200000              // wheel2 encoder's no of ticks per rev
#define WHEEL_DIAMETER 0.152                // wheel's diameter in meters
#define LR_WHEELS_DISTANCE 0.333            // distance between left and right wheels
#define PWM_BITS 12                          // PWM Resolution of the microcontroller
#define PWM_FREQUENCY 10000                 // PWM Frequency
#define PWM_MAX (pow(2, PWM_BITS) - 1)
#define PWM_MIN -PWM_MAX
#define MOTOR_OPERATING_VOLTAGE 24
#define MOTOR_POWER_MAX_VOLTAGE 24 
#define MOTOR_POWER_MEASURED_VOLTAGE 24
// Right 319
#define PWM_NEG_LEFT -267
#define PWM_POS_LEFT 267 
#define PWM_NEG_RIGHT -270
#define PWM_POS_RIGHT 269

// INVERT ENCODER COUNTS
#define MTR_ENCODER_INV_LEFT true 
#define MTR_ENCODER_INV_RIGHT false 

// INVERT MOTOR DIRECTIONS
#define MTR_INV_LEFT true 
#define MTR_INV_RIGHT false 

// MOTOR PINS
#ifdef USE_ZD_MOTOR_DRIVER
  #define MTR_PWM_LEFT 3
  #define MTR_PWM_RIGHT 2

  #define ENCODER_SPI_SCK 13
  #define ENCODER_SPI_SDO 11
  #define ENCODER_SPI_SDI 12

  #define ENCODER_CS_LEFT 10
  #define ENCODER_CS_RIGHT 39 

  #define MTR_FWD_LEFT 43
  #define MTR_BKD_LEFT 42
  #define MTR_FWD_RIGHT 9
  #define MTR_BKD_RIGHT 6
#endif 


#endif
