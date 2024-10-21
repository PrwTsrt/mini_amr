#ifndef ROBOT_HARDWARE_INTERFACE_H
#define ROBOT_HARDWARE_INTERFACE_H

#include <iostream>
#include <libserial/SerialPort.h>
#include <libserial/SerialStream.h>
#include <vector>
#include <thread>
#include <iomanip>

class HardwareInterface 
{
public:
    const uint8_t DEBUG         = false;
    const uint8_t DEBUG_IMU     = false;
    const uint8_t DEBUG_SEND    = false;
    const uint8_t DEBUG_RECEIVE = false;
    const uint8_t DEBUG_ODOM    = false;
    const uint8_t DEBUG_RANGE   = false;
    const uint8_t DEBUG_BATT    = false;

    const uint8_t SERIALPORT_TIMEOUT_MS = 50;

    struct three_dimension
    {
        float x = 0.0;
        float y = 0.0;
        float z = 0.0;
    };

    three_dimension angular_velocity;
    three_dimension linear_acceleration;
    three_dimension odom_velocity;

    float range_left;
    float range_center;
    float range_right;

    float voltage_;
    float current_;

    HardwareInterface(const std::string& port);

    ~HardwareInterface();

    void SetMotion(float v_x, float v_y, float v_z);
    void UpdateIP(char* addressBuffer);
    void UpdateStatus(int status);

private:
    const uint8_t HEAD          = 0xFF;
    const uint8_t HOST_ID       = 0x00;
    const uint8_t DEVICE_ID     = 0x01;

    const uint8_t FUNC_MOTION   = 0x01;
    const uint8_t FUNC_IMU      = 0x02;
    const uint8_t FUNC_ODOM     = 0x03;
    const uint8_t FUNC_RANGE    = 0x04;
    const uint8_t FUNC_IP       = 0x05;
    const uint8_t FUNC_STATUS   = 0x06;
    const uint8_t FUNC_BATT     = 0x07;

    std::string port_name;
    LibSerial::SerialPort serial_port;
    bool run_receive_thread;

    void ParseData(uint8_t FUNC_TYPE, const std::vector<uint8_t>& data); 
    void SendData(uint8_t FUNC_TYPE, const std::vector<uint8_t>& param);
    void ReceiveData();

};


#endif