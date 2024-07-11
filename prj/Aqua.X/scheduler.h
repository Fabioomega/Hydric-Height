/* 
 * File:   scheduler.h
 * Author: Dell
 *
 * Created on July 10, 2024, 7:31 PM
 */

#ifndef SCHEDULER_H
#define	SCHEDULER_H

#ifdef	__cplusplus
extern "C" {
#endif

#include "state_machine.h"

    enum state st = SLEEP;
    enum state saved_state = SLEEP;
    __bit high_priority_task = 0;

    inline void high_priority_change_state(enum state s) {
        if (st != SLEEP && high_priority_task == 0)
            saved_state = st;
        st = s;
        high_priority_task = 1;
    }

    inline void low_priority_change_state_to(enum state s) {
        if (s == SLEEP)
            high_priority_task = 0;

        if (high_priority_task == 1)
            saved_state = s;
        else if (saved_state != SLEEP && s != saved_state) {
            st = saved_state;
            saved_state = s;
        }
        else
            st = s;
    }

#ifdef	__cplusplus
}
#endif

#endif	/* SCHEDULER_H */

