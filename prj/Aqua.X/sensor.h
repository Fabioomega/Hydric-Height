/* 
 * File:   sensor.h
 * Author: Dell
 *
 * Created on July 4, 2024, 8:48 PM
 */

#ifndef SENSOR_H
#define	SENSOR_H

#ifdef	__cplusplus
extern "C" {
#endif

#ifdef _XTAL_FREQ
    #define FREQ _XTAL_FREQ
#else
    #define _XTAL_FREQ 4300800
#endif

#define FREQ ((float)_XTAL_FREQ)
#define PERIOD 4.0f / FREQ
#define SPEED_OF_SOUND_CM_SECS 34000.0f
#define CONVERSION_FACTOR (PERIOD*SPEED_OF_SOUND_CM_SECS) / 2
    
__bit echo_cycle = 0;
uint8_t extra_timer = 0;
const float conversion_factor = CONVERSION_FACTOR;

inline void start_timer(void)
{
    TMR2 = 0;
    extra_timer = 0;
    T2CONbits.TMR2ON = 1;
}

inline uint16_t stop_timer(void) 
{
    T2CONbits.TMR2ON = 0;
    return ((uint16_t)extra_timer << 8) + (uint16_t) TMR2;
}

inline void start_sensor_reading(void) 
{
    PORTBbits.RB0 = 1;
    __delay_ms(10);
    PORTBbits.RB0 = 0;
}

inline void handle_sensor(void)
{
    echo_cycle = PORTBbits.RB4;
}

inline void handle_tmr2_interrupt(void)
{
    extra_timer++;
}

inline void disable_unecessary_interruptions()
{
    // No T1 interrupts <0>
    // Setup T2 interrupts <1>
    // No RX interrupts <5>
    PIE1 = 0b00000010;
}

inline void reenable_unecessary_interruptions()
{
    // Setup T1 interrupts <0>
    // Setup T2 interrupts <1>
    // Setup RX interrupts <5>
    PIE1 = 0b00100011;
}

inline uint16_t read_sensor(void)
{
    disable_unecessary_interruptions();
    // Enable RB Port Change Interrupt
    INTCONbits.RBIE = 1;
    RD7 = 1;
    start_sensor_reading();
    while (echo_cycle != 1) continue;
    start_timer();
    while (echo_cycle != 0) continue;
    uint16_t tm2 = stop_timer();
    RD7 = 0;
    // Disable RB Port Change Interrupt
    INTCONbits.RBIE = 0;
    reenable_unecessary_interruptions();
    return ((uint16_t) ((float)tm2*conversion_factor));
}

#ifdef	__cplusplus
}
#endif

#endif	/* SENSOR_H */

