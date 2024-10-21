#include "motor.h"

Motor::Motor(uint8_t pwm_pin, uint8_t dir_pin, uint8_t channal, uint16_t frequency, uint8_t resolution)
            : pwm_pin_(pwm_pin), dir_pin_(dir_pin), channal_(channal), frequency_(frequency), resolution_(resolution){

            pinMode(pwm_pin_, OUTPUT);
            pinMode(dir_pin_, OUTPUT);
            ledcSetup(channal_, frequency_, resolution_);
            ledcAttachPin(pwm_pin_, channal_);

}

void Motor::drive(int cmd){
            
            uint16_t pwm_limit = pow(2, resolution_) - 1;  
            uint16_t pwm;  
            bool dir;        

            if (cmd > 0){ 
                dir = HIGH;
                pwm = cmd;}
            else {
                dir = LOW;
                pwm = cmd * -1;}

            if(pwm > pwm_limit){
                pwm = pwm_limit;}

            // Serial1.printf("[Motor Channel %d] PWM: %d | Direction: %d\n", channal_, pwm, dir);

            ledcWrite(channal_, pwm);
            digitalWrite(dir_pin_, dir);
}
