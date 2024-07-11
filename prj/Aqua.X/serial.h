/* 
 * File:   serial.h
 * Author: Dell
 *
 * Created on July 9, 2024, 8:49 PM
 */

#ifndef SERIAL_H
#define	SERIAL_H

#ifdef	__cplusplus
extern "C" {
#endif
    
// Received
#define SHOULD_READ 114
#define TESTING_PLEASE_ANSWER 116

// Sent
#define PROCESSING_BITCH_PLEASE_STOP 69
#define DONT_UNDERSTAND_MONKEY 63
#define LOOKS_DANDY_BABE 64
#define EMPTY_BRUH 48

    
unsigned char last_received_data;
__bit data_locked = 0;
    
void send_serial_packet(uint8_t data)
{
    TXREG = data;
    while(!TXIF);
    TXIF = 0;
}

inline unsigned char read_serial_packet()
{
    return RCREG;
}

inline void handle_serial_read()
{
    last_received_data = read_serial_packet();
}

inline void clean_received_buff()
{
    read_serial_packet();
}

inline void lock_data()
{
    data_locked = 1;
}

inline void unlock_data()
{
    data_locked = 0;
}

#ifdef	__cplusplus
}
#endif

#endif	/* SERIAL_H */

