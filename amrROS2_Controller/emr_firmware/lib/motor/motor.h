#ifndef MOTOR_H
#define MOTOR_H

#include <Arduino.h>

class Motor
{
    public:
        void drive(int cmd);
        Motor(uint8_t pwm_pin, uint8_t dir_pin, uint8_t channal, uint16_t frequency, uint8_t resolution);

    private:
        uint8_t pwm_pin_;
        uint8_t dir_pin_;
        uint8_t channal_;
        uint8_t resolution_;
        uint16_t frequency_;
};

#endif