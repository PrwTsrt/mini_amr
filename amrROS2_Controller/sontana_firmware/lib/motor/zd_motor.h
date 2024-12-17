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

#ifndef ZD_MOTOR
#define ZD_MOTOR

#include <Arduino.h>
#include <Servo.h> 

#include "motor_interface.h"

class ZD_Motor: public MotorInterface
{
    private:
        int fwd_pin_;
        int bkd_pin_;
        int pwm_pin_;

    protected:
        void forward(int pwm) override
        {
            digitalWrite(fwd_pin_, HIGH);
            digitalWrite(bkd_pin_, LOW);
            analogWrite(pwm_pin_, abs(pwm));
        }

        void reverse(int pwm) override
        {
            digitalWrite(fwd_pin_, LOW);
            digitalWrite(bkd_pin_, HIGH);
            analogWrite(pwm_pin_, abs(pwm));
        }

    public:
        ZD_Motor(float pwm_frequency, int pwm_bits, bool invert, int pwm_pin, int fwd_pin, int bkd_pin): 
            MotorInterface(invert),
            fwd_pin_(fwd_pin),
            bkd_pin_(bkd_pin),
            pwm_pin_(pwm_pin)
        {
            delay(10);

            pinMode(fwd_pin_, OUTPUT);
            pinMode(bkd_pin_, OUTPUT);
            pinMode(pwm_pin_, OUTPUT);
            if(pwm_frequency > 0)
            {
                analogWriteFrequency(pwm_pin_, pwm_frequency);
            }
            analogWriteResolution(pwm_bits);

            //ensure that the motor is in neutral state during bootup
            analogWrite(pwm_pin_, abs(0));
        }

        void brake() override
        {
            analogWrite(pwm_pin_, 0);
        }
};

#endif
