#ifndef ENCODER_H
#define ENCODER_H

#include <Arduino.h>

class Encoder
{
    public:
        Encoder(uint8_t which_enc, uint8_t pinA, uint8_t pinB, uint16_t count_per_rev);
        void begin();
        int64_t get_count() const;
        float getRPM( );

    private:
        static Encoder *instance0_ ;
        static Encoder *instance1_ ;
        static Encoder *instance2_ ;
        static Encoder *instance3_ ;

        uint8_t which_enc_;
        uint8_t pinA_;
        uint8_t pinB_;
        uint16_t count_per_rev_;
        volatile int64_t count_;
        uint8_t lastEncoded_;
        uint64_t prv_update_time_;
        int64_t prv_count_;

        static void ISR0();
        static void ISR1();
        static void ISR2();
        static void ISR3();

        void handleInterrupt();
};

#endif