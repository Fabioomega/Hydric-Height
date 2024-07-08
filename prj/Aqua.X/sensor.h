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
    #define _XTAL_FREQ 1000000
#endif

#define FREQ _XTAL_FREQ
#define PERIOD 1.0 /( (float) FREQ / 4.0)
#define SPEED_OF_SOUND_CM_SECS 34000.0
#define CONVERSION_FACTOR (float) (PERIOD*SPEED_OF_SOUND_CM_SECS)
    
__bit echo_cycle = 0;
const float conversion_factor = CONVERSION_FACTOR;

inline void start_timer(void)
{
    TMR2 = 0;
    T2CONbits.TMR2ON = 1;
}

inline uint8_t stop_timer(void) 
{
    T2CONbits.TMR2ON = 0;
    return TMR2;
}

inline void start_sensor_reading(void) 
{
    RB0 = 1;
    __delay_ms(10);
    RB0 = 0;
}

inline void handle_sensor(void)
{
    echo_cycle = RB4;
}

inline uint8_t read_sensor(void)
{
    start_sensor_reading();
    while (echo_cycle != 1) continue;
    start_timer();
    while (echo_cycle != 0) continue;
    uint8_t time_took = stop_timer();
    return (uint8_t)((float)time_took*conversion_factor);
}

#ifdef	__cplusplus
}
#endif

#endif	/* SENSOR_H */

