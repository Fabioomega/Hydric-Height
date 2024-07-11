/* 
 * File:   file_handling.h
 * Author: Dell
 *
 * Created on June 22, 2024, 10:50 PM
 */

#ifndef FILE_HANDLING_H
#define	FILE_HANDLING_H

#ifdef	__cplusplus
extern "C" {
#endif
    
#define BUFFER_SIZE 4

uint8_t buffer[BUFFER_SIZE];
uint8_t memory_locations[BUFFER_SIZE];
uint8_t size = 0;
uint8_t idx = 0;

uint8_t last_filled_memory_pos = 0;

inline void _write_byte(uint8_t data, uint8_t addr) {
    // Sets up the address
    EEADR = addr;
    // Sets up the data
    EEDATA = data;
    // Set the write to be at EEPROM
    EEPGD = 0;
    // Enables writes cicles
    EECON1bits.WREN = 1;
    // Disables interrupts
    INTCONbits.GIE = 0;
    // Magical constants
    EECON2 = 0x55;
    EECON2 = 0xAA;
    // Start the write
    EECON1bits.WR = 1;
    // Restore Global Interrupt
    INTCONbits.GIE = 1;
}

inline void start_write(void) {
    _write_byte(buffer[idx], memory_locations[idx]);
    idx++;
}

inline void reset_buffer(void) {
    idx = 0;
    size = 0;
}

inline __bit is_write_finished(void) {
    return idx == size;
}

void write_uint8(uint8_t data) {
    // Is the EEPROM full?
    if (last_filled_memory_pos == 255) return;
    // Save the first value to the buffer
    buffer[size] = data;
    // Save the location to be saved at
    memory_locations[size] = last_filled_memory_pos;
    size++;
    last_filled_memory_pos++;
}

void write_uint16(uint16_t data) {
    // Is the EEPROM full?
    if (last_filled_memory_pos == 255) return;
    // Treat the data as an array of uint8_t
    uint8_t* black_magic = (uint8_t*) & data;
    // Save the first value to the buffer
    buffer[size] = black_magic[0];
    // Save the location to be saved at
    memory_locations[size] = last_filled_memory_pos;
    size++;
    last_filled_memory_pos++;
    // Save the second value to the buffer
    buffer[size] = black_magic[1];
    // Save the location to be saved at
    memory_locations[size] = last_filled_memory_pos;
    size++;
    last_filled_memory_pos++;
}

uint8_t read_byte(uint8_t addr) {
    //Set address
    EEADR = addr;
    // Get data from EEPROM
    EEPGD = 0;
    // Start read
    EECON1bits.RD = 1;
    return EEDATA;
}

uint16_t read_uint16(uint8_t addr) {
    // Bound condition
    if (addr == 255) return 0;
    // Read the low byte
    uint16_t result = read_byte(addr + 1);
    // Prepare result for merge
    result = (result << 8);
    // Merge low byte with high byte
    result = result | (uint16_t) read_byte(addr);
    return result;
}

#ifdef	__cplusplus
}
#endif

#endif	/* FILE_HANDLING_H */

