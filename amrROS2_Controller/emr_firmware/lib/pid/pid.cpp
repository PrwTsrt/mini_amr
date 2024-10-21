#include "Arduino.h"
#include "pid.h"

PID::PID(float min_val, float max_val, float kp, float ki, float kd):
    min_val_(min_val),
    max_val_(max_val),
    kp_(kp),
    ki_(ki),
    kd_(kd)
{
    c_stick_pos_ = 0.;
    c_stick_neg_ = 0.;
    pos_scale_ = 1.;
    neg_scale_ = 1.;
}

void PID::set_deadband_comp(float c_stick_pos, float c_stick_neg)
{
    c_stick_pos_ = c_stick_pos;
    c_stick_neg_ = c_stick_neg;
    pos_scale_ = ( max_val_ - c_stick_pos_ ) / max_val_;
    neg_scale_ = ( min_val_ - c_stick_neg_ ) / min_val_; 
}

void PID::reset()
{
//    c_stick_pos_ = 0.;
//    c_stick_neg_ = 0.;
//    pos_scale_ = 1.;
//    neg_scale_ = 1.;
    integral_   = 0.;
    derivative_ = 0.;
    prev_error_ = 0.;
}

double PID::compute(float setpoint, float measured_value)
{
    double error;
    double pid;
    //setpoint is constrained between min and max to prevent pid from having too much error
    error = setpoint - measured_value;
    derivative_ = error - prev_error_;

    pid = (kp_ * error) + (ki_ * integral_) + (kd_ * derivative_);
    prev_error_ = error;

    // Serial1.printf(">pid:%f\n", pid);
    pid = deadband( pid );
    // Serial1.printf(">pid after deadband:%f\n", pid);

    if(setpoint == 0 && error == 0)
        integral_ = 0;
    else if( pid < max_val_ && pid > min_val_ ) 
        integral_ += error;
    
    return pid;
}

double PID::deadband(double c_wanted )
{
    double c_adjusted;
    if( c_wanted == 0 )
	    c_adjusted = 0;
    else if( c_wanted > 0 )
    {
    	c_adjusted = c_stick_pos_ + pos_scale_ * c_wanted;
	    if( c_adjusted > max_val_ )
	        c_adjusted = max_val_;
    }
    else
    {
    	c_adjusted = c_stick_neg_ + neg_scale_ * c_wanted;
        if( c_adjusted  < min_val_ )
	        c_adjusted = min_val_;	
    }
    return c_adjusted;
}

void PID::updateConstants(float kp, float ki, float kd)
{
    kp_ = kp;
    ki_ = ki;
    kd_ = kd;
}
