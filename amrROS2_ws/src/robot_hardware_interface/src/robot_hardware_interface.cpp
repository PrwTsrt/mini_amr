#include "hardware_interface/robot_hardware_interface.h"

HardwareInterface::HardwareInterface(const std::string& port) : port_name(port), run_receive_thread(true){

    try {
        // Open the serial port
        serial_port.Open(port_name);
        serial_port.SetBaudRate(LibSerial::BaudRate::BAUD_115200);
        std::cout << "Serial port opened at port "<< port_name << " with baudrate 115200" << std::endl;
    }
    catch (const std::exception& e) {
        std::cerr << "Open the serial port error: " << e.what() << std::endl;
    }

    try {
        // Create the thread for receiving data
        std::thread receive_thread(&HardwareInterface::ReceiveData, this);
        receive_thread.detach();
        std::cout << "Created receive data thread" << std::endl;
    }
    catch (const std::exception& e) {
        std::cerr << "Create receive data thread error: " << e.what() << std::endl;
    }        
}

HardwareInterface::~HardwareInterface(){

    run_receive_thread = false; 

    if (serial_port.IsOpen()) {
        serial_port.Close();
    }
}

void HardwareInterface::SetMotion(float v_x, float v_y, float v_z) {

    try {
        int16_t v_x_int = static_cast<int16_t>(v_x * 1000);
        int16_t v_y_int = static_cast<int16_t>(v_y * 1000);
        int16_t v_z_int = static_cast<int16_t>(v_z * 1000);

        std::vector<uint8_t> cmd = {
            static_cast<uint8_t>(v_x_int & 0xFF), static_cast<uint8_t>((v_x_int >> 8) & 0xFF),
            static_cast<uint8_t>(v_y_int & 0xFF), static_cast<uint8_t>((v_y_int >> 8) & 0xFF),
            static_cast<uint8_t>(v_z_int & 0xFF), static_cast<uint8_t>((v_z_int >> 8) & 0xFF)
        };

        SendData(FUNC_MOTION, cmd);

    } catch (const std::exception& e) {
        std::cerr << "Set motion error: " << e.what() << std::endl;
    }
}

void HardwareInterface::UpdateIP(char* addressBuffer) {
    uint8_t ipPart1, ipPart2, ipPart3, ipPart4;
    try {
        sscanf(addressBuffer, "%hhu.%hhu.%hhu.%hhu", &ipPart1, &ipPart2, &ipPart3, &ipPart4);

        std::vector<uint8_t> ipBytes = {
                    static_cast<uint8_t>(ipPart1), 
                    static_cast<uint8_t>(ipPart2), 
                    static_cast<uint8_t>(ipPart3), 
                    static_cast<uint8_t>(ipPart4)  
                };

        SendData(FUNC_IP, ipBytes);

    } catch (const std::exception& e) {
        std::cerr << "Update ip address error: " << e.what() << std::endl;
    }
}

void HardwareInterface::UpdateStatus(int status) {
    try {
        std::vector<uint8_t> statusBytes = {static_cast<uint8_t>(status)};

        SendData(FUNC_STATUS, statusBytes);

    } catch (const std::exception& e) {
        std::cerr << "Update status error: " << e.what() << std::endl;
    }
}

void HardwareInterface::ParseData(uint8_t FUNC_TYPE, const std::vector<uint8_t>& data) {
    if (FUNC_TYPE == FUNC_IMU) {
        int16_t roll  = static_cast<int16_t>(data[0] | (data[1] << 8));
        int16_t pitch = static_cast<int16_t>(data[2] | (data[3] << 8));
        int16_t yaw   = static_cast<int16_t>(data[4] | (data[5] << 8));
        int16_t acc_x = static_cast<int16_t>(data[6] | (data[7] << 8));
        int16_t acc_y = static_cast<int16_t>(data[8] | (data[9] << 8));
        int16_t acc_z = static_cast<int16_t>(data[10] | (data[11] << 8));

        angular_velocity.x = roll  / 1000.0;
        angular_velocity.y = pitch / 1000.0;
        angular_velocity.z = yaw   / 1000.0;

        linear_acceleration.x = acc_x / 1000.0;
        linear_acceleration.y = acc_y / 1000.0;
        linear_acceleration.z = acc_z / 1000.0;

        update_imu_ = true;

        if(DEBUG_IMU){
            std::cout << "IMU -";
            std::cout << " " << angular_velocity.x;
            std::cout << " " << angular_velocity.y;
            std::cout << " " << angular_velocity.z;
            std::cout << " " << linear_acceleration.x;
            std::cout << " " << linear_acceleration.y;
            std::cout << " " << linear_acceleration.z << std::endl;
        }
    }
    else if (FUNC_TYPE == FUNC_ODOM) {
        int16_t Vx = static_cast<int16_t>(data[0] | (data[1] << 8));
        int16_t Vy = static_cast<int16_t>(data[2] | (data[3] << 8));
        int16_t Wz = static_cast<int16_t>(data[4] | (data[5] << 8));

        odom_velocity.x = Vx / 1000.0;
        odom_velocity.y = Vy / 1000.0;
        odom_velocity.z = Wz / 1000.0;

        update_odom_ = true;

        if(DEBUG_ODOM){
            std::cout << "Odom -";
            std::cout << " " << odom_velocity.x / 1000.0;
            std::cout << " " << odom_velocity.y / 1000.0;
            std::cout << " " << odom_velocity.z / 1000.0 << std::endl;
        }
    }
    else if (FUNC_TYPE == FUNC_RANGE) {
        int16_t range_1 = static_cast<int16_t>(data[0] | (data[1] << 8));
        int16_t range_2 = static_cast<int16_t>(data[2] | (data[3] << 8));
        int16_t range_3 = static_cast<int16_t>(data[4] | (data[5] << 8));

        range_left   = range_1 / 1000.0;
        range_center = range_2 / 1000.0;
        range_right  = range_3 / 1000.0;

        update_range_ = true;

        if(DEBUG_RANGE){
            std::cout << "Range -";
            std::cout << " " << range_left;
            std::cout << " " << range_center;
            std::cout << " " << range_right << std::endl;
        }
    }
    else if (FUNC_TYPE == FUNC_BATT) {
        int16_t voltage = static_cast<int16_t>(data[0] | (data[1] << 8));
        int16_t current = static_cast<int16_t>(data[2] | (data[3] << 8));

        voltage_ = voltage / 1000.0;
        current_ = current;

        update_batt_ = true;

        if(DEBUG_BATT){
            std::cout << "Battery -";
            std::cout << " " << voltage;
            std::cout << " " << current << std::endl;
        }
    }

    // update_imu_ = false;
    // update_odom_ = false;
    // update_range_ = false;
    // update_batt_ = false;
}

void HardwareInterface::SendData(uint8_t FUNC_TYPE, const std::vector<uint8_t>& param) {
    try {
        std::vector<uint8_t> cmd = {HEAD, DEVICE_ID, 0x00, FUNC_TYPE};
        cmd.insert(cmd.end(), param.begin(), param.end());
        cmd[2] = cmd.size();

        uint8_t checksum = 0;

        for (uint8_t byte : cmd) {
            checksum += byte;
        }

        checksum &= 0xFF;
        cmd.push_back(checksum);
        
        if (serial_port.IsOpen()) {
            serial_port.Write(cmd);
        }

        if (DEBUG_SEND) {
            std::cout << "Sending command: ";
            for (const auto& byte : cmd) {
                // std::cout << "0x" << std::hex << std::setw(2) << std::setfill('0') << (int)byte << " ";
                std::cout << std::dec << std::setw(3) << std::setfill(' ') << (int)byte << " ";
            }
            std::cout << std::endl;
        }
    } catch (const std::exception& e) {
        std::cerr << "Send Data Error: " << e.what() << std::endl;
    }
}

void HardwareInterface::ReceiveData() {
    try {

        uint8_t header;
        uint8_t device_id;
        uint8_t len;
        uint8_t func_type;
        uint8_t data_len;
        uint8_t value;
        uint8_t rx_check_num;
        uint8_t check_sum;
        std::vector<uint8_t> data;

        while (run_receive_thread) {
            if (serial_port.IsDataAvailable()) {

                serial_port.ReadByte(header, SERIALPORT_TIMEOUT_MS);
                
                if (header == HEAD) {
                    serial_port.ReadByte(device_id, SERIALPORT_TIMEOUT_MS);

                    if (device_id == HOST_ID) {

                        serial_port.ReadByte(len, SERIALPORT_TIMEOUT_MS);
                        serial_port.ReadByte(func_type, SERIALPORT_TIMEOUT_MS);

                        uint8_t check_sum = header + device_id + len + func_type;
                        uint8_t data_len = len - 4;
                        data = {};

                        while (data.size() < data_len) {                            
                            serial_port.ReadByte(value, SERIALPORT_TIMEOUT_MS);
                            data.push_back(value);
                            check_sum += value;
                        }

                        serial_port.ReadByte(rx_check_num, SERIALPORT_TIMEOUT_MS);                        

                        if ((check_sum & 0xFF) == rx_check_num) {
                            if (DEBUG_RECEIVE) {
                                std::cout << "Data received" << std::endl;
                            }
                            ParseData(func_type, data);
                        } else {
                            if (DEBUG_RECEIVE) {
                                std::cout << "Checksum error" << std::endl;
                            }
                        }

                        if (DEBUG_RECEIVE) {
                            std::cout << "Device_id: " << (int)device_id << std::endl;
                            std::cout << "Data range: " << (int)len << std::endl;
                            std::cout << "Function type: " << (int)func_type << std::endl;
                            for (size_t i = 0; i < data.size(); ++i) {
                                std::cout << "Data" << i << ": " << (int)data[i] << std::endl;
                            }
                            std::cout << "Ground truth: " << (int)rx_check_num << std::endl;
                            std::cout << "Checksum: " << (check_sum & 0xFF) << std::endl;
                        }
                    }
                }
            }
        }
    } catch (const std::exception& e) {
        std::cerr << "Received Data Error: " << e.what() << std::endl;
    }
}