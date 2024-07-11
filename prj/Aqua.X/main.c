
// PIC16F877A Configuration Bit Settings

// 'C' source line config statements

// A
// CONFIG
#pragma config FOSC = XT        // Oscillator Selection bits (XT oscillator)
#pragma config WDTE = OFF       // Watchdog Timer Enable bit (WDT disabled)
#pragma config PWRTE = OFF      // Power-up Timer Enable bit (PWRT disabled)
#pragma config BOREN = OFF      // Brown-out Reset Enable bit (BOR disabled)
#pragma config LVP = OFF        // Low-Voltage (Single-Supply) In-Circuit Serial Programming Enable bit (RB3 is digital I/O, HV on MCLR must be used for programming)
#pragma config CPD = OFF        // Data EEPROM Memory Code Protection bit (Data EEPROM code protection off)
#pragma config WRT = OFF        // Flash Program Memory Write Enable bits (Write protection off; all program memory may be written to by EECON control)
#pragma config CP = ON          // Flash Program Memory Code Protection bit (All program memory code-protected)

// #pragma config statements should precede project file includes.
// Use project enums instead of #define for ON and OFF.

#include <xc.h>
#include "state_machine.h"
#include "file_handling.h"
#include "sensor.h"
#include "serial.h"
#include "scheduler.h"
#define _XTAL_FREQ 4300800
#define EXT_CLOCK_FREQ 32000
#define TIME_INTERVAL_SECS 2
#define HOUR_IN_SECS 4

#define START_MEMORY_ADDRESS 0x00

const uint16_t update_data_secs_req = 4;

uint8_t last_hour = 0;
uint8_t passed_hours = 0;
uint16_t passed_seconds = 0;
uint16_t distance = 0;

inline uint8_t downcast_and_compress_uint16(uint16_t data) {
    return (uint8_t) (data >> 1);
}

inline void handle_passage_of_time(void) {
    passed_seconds += TIME_INTERVAL_SECS;
    if (passed_seconds >= HOUR_IN_SECS) {
        last_hour = passed_hours;
        passed_hours++;
        passed_seconds = 0;
    }
}

void __interrupt(high_priority) handleInterruption(void) {
    // Disable Global Interrupt
    INTCONbits.GIE = 0;
    // EEPROM finished writing
    if (PIR2bits.EEIF == 1) {
        high_priority_change_state(WRITING_TO_EEPROM);
        PIR2bits.EEIF = 0;
    }

    // Timer1 Interrupt happened
    if (PIR1bits.TMR1IF == 1) {
        handle_passage_of_time();
        PIR1bits.TMR1IF = 0;
    }

    // RB Port Change Interrupt Flag bit
    if (INTCONbits.RBIF == 1) {
        handle_sensor();
        INTCONbits.RBIF = 0;
    }

    // Timer2 Interrupt happened
    if (PIR1bits.TMR2IF == 1) {
        handle_tmr2_interrupt();
        PIR1bits.TMR2IF = 0;
    }

    // RX Interrupt happened
    if (PIR1bits.RCIF == 1) {
        if (data_locked == 0) {
            lock_data();
            handle_serial_read();
            low_priority_change_state_to(SERIAL_COMMUNICATION);
        } else {
            clean_received_buff();
            send_serial_packet(PROCESSING_BITCH_PLEASE_STOP);
        }
    }

    INTCONbits.GIE = 1;
}

void setup(void) {
    // Setup ports
    TRISB = 0;
    TRISBbits.TRISB4 = 1;
    TRISCbits.TRISC1 = 1;
    TRISCbits.TRISC7 = 1;
    PORTB = 0;
    PORTC = 0;
    //Setup OPTION_REG
    OPTION_REG = 0b10000000;
    // Setup Interrupts
    // General Interrupts and Peripherals
    // RB Port Change Interrupt Enabled bit <3> is disabled by default
    // to prevent issues with instabilties
    INTCON = 0b11000000;
    // Setup T1 interrupts <0>
    // Setup T2 interrupts <1>
    // Setup RX interrupts <5>
    PIE1 = 0b00100011;
    // Setup EEPROM interrupts
    PIE2 = 0b00010000;

    // Sets the TMR2 interrupt value
    PR2 = 0xFF;

    // Disables writes cicles for safety    
    EECON1bits.WREN = 0;

    // Enable Timer1
    // For longer durations, use the preescaler (It's the first 4 bits)
    T1CON = 0b00000111;

    // Safe and Sound
    PIR1bits.TMR2IF = 0;
    TXIF = 0;
    RCIF = 0;

    // Set Baudrate to 9600
    SPBRG = 6;
    //BRGH=0, TXEN = 1, Asynchronous Mode, 8-bit mode
    TXSTA = 0x20;
    //Serial Port enabled,8-bit reception
    RCSTA = 0b10010000;
}

void main(void) {
    setup();
    while (1) {
        switch (st) {
            case SLEEP:
                SLEEP();
                // Did and hour passed?
                if (passed_hours != last_hour)
                {
                    high_priority_change_state(READING_SENSOR);
                    last_hour = passed_hours;
                }
                if (passed_hours == 255) {
                    last_filled_memory_pos = START_MEMORY_ADDRESS;
                    passed_hours = 0;
                }
                break;
            case READING_SENSOR:
            {
                distance = read_sensor();
                uint8_t compressed = downcast_and_compress_uint16(distance);
                write_uint8(compressed);
                high_priority_change_state(WRITING_TO_EEPROM);
            }
                break;
            case SERIAL_COMMUNICATION:
                if (last_received_data == TESTING_PLEASE_ANSWER) {
                    send_serial_packet(LOOKS_DANDY_BABE);
                } else if (last_received_data == SHOULD_READ) {
                    if (last_filled_memory_pos == 0) {
                        // Header
                        send_serial_packet(EMPTY_BRUH);
                        // Body
                        send_serial_packet(EMPTY_BRUH);
                    } else {
                        // Header
                        send_serial_packet(last_filled_memory_pos);
                        for (uint8_t i = 0; i < last_filled_memory_pos; i++) {
                            send_serial_packet(read_byte(i));
                        }
                        last_filled_memory_pos = 0;
                        passed_hours = 0;
                        passed_seconds 0;
                    }
                } else {
                    send_serial_packet(DONT_UNDERSTAND_MONKEY);
                }
                low_priority_change_state_to(SLEEP);
                unlock_data();
                break;
            case WRITING_TO_EEPROM:
                if (is_write_finished()) {
                    reset_buffer();
                    low_priority_change_state_to(SLEEP);
                    continue;
                }
                start_write();
                high_priority_change_state(WRITING_IN_PROGRESS);
                break;
            case WRITING_IN_PROGRESS:
                break;
        }
    }

    return;
};
