/* 
 * File:   state_machine.h
 * Author: Fabio Siqueira
 *
 * Created on June 18, 2024, 4:12 PM
 */

#ifndef STATE_MACHINE_H
#define	STATE_MACHINE_H

#ifdef	__cplusplus
extern "C" {
#endif

    enum state
    {
        SLEEP=0,
        SERIAL_COMMUNICATION,
        READING_SENSOR,
        WRITING_TO_EEPROM,
        WRITING_IN_PROGRESS,
    };

#ifdef	__cplusplus
}
#endif

#endif	/* STATE_MACHINE_H */

