/*
 * =====================================================================================
 *
 *       Filename:  IcMDEncoder.h
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  10/02/2023 05:56:32 AM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Pished Bunnun (pished.bunnun@nectec.or.th), 
 *   Organization:  NECTEC
 *
 * =====================================================================================
 */
#ifndef IcMdEncoder_h_
#define IcMdEncoder_h_

#define USE_SMR_CONFIG

#include <SPI.h>
#include <DriverIcMd.h>

inline int64_t check_signed(uint64_t raw) __attribute__((always_inline));
int64_t check_signed(uint64_t raw) { return (raw & 0x800000000000ULL) ? -((~raw & 0x0000FFFFFFFFFFFFLL)+1) : raw & 0x0000FFFFFFFFFFFFLL; }

typedef struct {
    int64_t prv_position;
    uint64_t prv_update_time;
    int counts_per_rev;
    bool inverse;
} Encoder;

float getRPM( Encoder & encoder, DriverIcMd & counter )
{
    DriverIcMd::UnionCounterValues icmd_pose = counter.GetCounter();
    int64_t position = encoder.inverse ? check_signed(icmd_pose.Counter0_48Bit.Counter0) :
                        -check_signed(icmd_pose.Counter0_48Bit.Counter0); 
    uint64_t current_time = micros();
    uint64_t dt = current_time - encoder.prv_update_time;

    double dtm = (double)dt/60000000;
    double delta_ticks = position - encoder.prv_position;

    encoder.prv_update_time = current_time;
    encoder.prv_position = position;

    return ( (delta_ticks / encoder.counts_per_rev) / dtm ); 
}

void setup_encoders(DriverIcMd & counter_left, DriverIcMd & counter_right)
{
    uint8_t ret;
    //Set pins for Encoders
    pinMode(ENCODER_CS_LEFT, OUTPUT);
    pinMode(ENCODER_CS_RIGHT, OUTPUT);
    digitalWrite(ENCODER_CS_LEFT, HIGH);
    digitalWrite(ENCODER_CS_RIGHT, HIGH);
    SPI.setMOSI(ENCODER_SPI_SDO);
    SPI.setMISO(ENCODER_SPI_SDI);
    SPI.setSCK(ENCODER_SPI_SCK);
    SPI.begin();
    
    delay(100);
    ret = counter_left.CheckDevice();
    if( ret == 0 )
    {
        counter_left.SetCounterMode( DriverIcMd::CounterMode::Counter0_48Bit );
        counter_left.SetIndexSignalZMode( DriverIcMd::IndexSignalZMode::A1B1 );
        counter_left.SetInputMode( DriverIcMd::Differential );
        counter_left.SetIndexClearedCounter( 0, DriverIcMd::NotCleanedByZ );
        counter_left.SetCountingDirection( 0, DriverIcMd::CWpositive );
        counter_left.SetIndexInvertedMode( 0, DriverIcMd::NonInverted );

        DriverIcMd::UnionInstructionByte instruction = {0};
        instruction.bits.ZCen_EnableZeroCodification = 1;
        instruction.bits.ABres0_ResetCounter0 = 1;
        instruction.bits.ABres1_ResetCounter1 = 1;
        instruction.bits.ABres2_ResetCounter2 = 1;
        counter_left.SendInstruction( instruction );
    }
    // else error
    delay(100);
    ret = counter_left.CheckDevice();
    if( ret == 0 )
    {
        counter_right.SetCounterMode( DriverIcMd::CounterMode::Counter0_48Bit );
        counter_right.SetIndexSignalZMode( DriverIcMd::IndexSignalZMode::A1B1 );
        counter_right.SetInputMode( DriverIcMd::Differential );
        counter_right.SetIndexClearedCounter( 0, DriverIcMd::NotCleanedByZ );
        counter_right.SetCountingDirection( 0, DriverIcMd::CWpositive );
        counter_right.SetIndexInvertedMode( 0, DriverIcMd::NonInverted );

        DriverIcMd::UnionInstructionByte instruction = {0};
        instruction.bits.ZCen_EnableZeroCodification = 1;
        instruction.bits.ABres0_ResetCounter0 = 1;
        instruction.bits.ABres1_ResetCounter1 = 1;
        instruction.bits.ABres2_ResetCounter2 = 1;
        counter_right.SendInstruction( instruction );
    }
    // else error
}

#endif
