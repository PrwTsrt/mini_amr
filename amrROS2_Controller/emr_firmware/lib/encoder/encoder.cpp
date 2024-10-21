#include <encoder.h>

Encoder::Encoder(uint8_t which_enc, uint8_t pinA, uint8_t pinB, uint16_t count_per_rev)
            : which_enc_(which_enc), pinA_(pinA), pinB_(pinB), count_per_rev_(count_per_rev) {}

void Encoder::begin()
{
    pinMode(pinA_, INPUT_PULLUP);
    pinMode(pinB_, INPUT_PULLUP);

    switch (which_enc_)
    {
    case 0 :
        attachInterrupt(digitalPinToInterrupt(pinA_), Encoder::ISR0, CHANGE);
        attachInterrupt(digitalPinToInterrupt(pinB_), Encoder::ISR0, CHANGE);
        instance0_ = this;
        break;
    
    case 1 :
        attachInterrupt(digitalPinToInterrupt(pinA_), Encoder::ISR1, CHANGE);
        attachInterrupt(digitalPinToInterrupt(pinB_), Encoder::ISR1, CHANGE);
        instance1_ = this;
        break;
    
    case 2 :
        attachInterrupt(digitalPinToInterrupt(pinA_), Encoder::ISR2, CHANGE);
        attachInterrupt(digitalPinToInterrupt(pinB_), Encoder::ISR2, CHANGE);
        instance2_ = this;
        break;

    case 3 :
        attachInterrupt(digitalPinToInterrupt(pinA_), Encoder::ISR3, CHANGE);
        attachInterrupt(digitalPinToInterrupt(pinB_), Encoder::ISR3, CHANGE);
        instance3_ = this;
        break;
    
    default:
        Serial1.printf("This class is provided for encoder number 0 - 3.");
        break;
    }
}

int64_t Encoder::get_count() const
{
    return count_;
}

void Encoder::ISR0() 
{
    instance0_->handleInterrupt();
}

void Encoder::ISR1() 
{
    instance1_->handleInterrupt();
}

void Encoder::ISR2() 
{
    instance2_->handleInterrupt();
}

void Encoder::ISR3() 
{
    instance3_->handleInterrupt();
}

void Encoder::handleInterrupt()
{
    bool MSB = digitalRead(pinA_); 
    bool LSB = digitalRead(pinB_); 

    uint8_t encoded = (MSB << 1) | LSB; 
    uint8_t sum  = (lastEncoded_ << 2) | encoded; 

    if(sum == 0b1101 || sum == 0b0100 || sum == 0b0010 || sum == 0b1011) count_++;
    if(sum == 0b1110 || sum == 0b0111 || sum == 0b0001 || sum == 0b1000) count_--;

    lastEncoded_ = encoded; 
}

float Encoder::getRPM( )
{
    uint64_t current_time = micros();
    uint64_t dt = current_time - prv_update_time_;
    int64_t d_count = count_ - prv_count_;

    // Serial1.printf(">dt :%d\n", dt);
    // Serial1.printf(">d_count :%d\n", d_count);
    // Serial1.printf(">count_per_rev_ :%d\n", count_per_rev_);

    float rpm = (float)(d_count * 60000000.00) / (dt * count_per_rev_);

    prv_update_time_ = current_time;
    prv_count_ = count_;

    return rpm;
}