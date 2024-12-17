#ifndef PID_H
#define PID_H

#include "Arduino.h"

class PID
{
    public:
        PID(float min_val, float max_val, float kp, float ki, float kd);
        void set_deadband_comp(float c_stick_pos, float c_stick_neg);
        double compute(float setpoint, float measured_value);
        void updateConstants(float kp, float ki, float kd);
        void reset();

    private:
        double deadband( double c_wanted );
        float min_val_;
        float max_val_;
        float c_stick_pos_;
        float c_stick_neg_;
        float pos_scale_;
        float neg_scale_;
        
        float kp_;
        float ki_;
        float kd_;
        double integral_;
        double derivative_;
        double prev_error_;
};

#endif
