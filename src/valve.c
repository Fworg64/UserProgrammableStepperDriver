#include "valve.h"

void valve_init (struct valve *valve, volatile unsigned char *port, unsigned char pin, unsigned int starttime, unsigned int ontime){
	if (pin <= 7){
		valve->port = port;
		valve->mask = 0x01 << pin;
		*valve->port &= ~valve->mask;
		valve->starttime = starttime;
		valve->ontime = ontime;
		valve->state = VALVE_STATE_UNSCHEDULED;
	}
}


void schedule_ms (unsigned int *time, unsigned int ms_max, unsigned int current_ms, unsigned int diff)
{
	if ((ms_max - current_ms) <= diff)
	{
		*time = diff;
		*time -= (ms_max - current_ms);
	} else {
		*time = diff + current_ms;
	}
}

void valve_schedule_in_ms (struct valve *valve, unsigned int ms_max, unsigned int current_ms){
		if (valve->state == VALVE_STATE_UNSCHEDULED){
			schedule_ms (&(valve->triggerontime), ms_max, current_ms, valve->starttime);
			schedule_ms (&(valve->triggerofftime), ms_max, current_ms, valve->starttime+valve->ontime);
			valve->state = VALVE_STATE_WAITING_FOR_ON;
		}
}


void valve_eval (struct valve *valve, unsigned int ms){
		if (valve->state == VALVE_STATE_WAITING_FOR_ON){		// if its off, check if the time has pased
			if (ms >= valve->triggerontime)
			{
				*valve->port |= valve->mask;	 // if it has, turn it on and reflect it in the state
				valve->state = VALVE_STATE_WAITING_FOR_OFF;				
			}
		} else  if (valve->state == VALVE_STATE_WAITING_FOR_OFF)
		{
			if (ms >= valve->triggerofftime)
			{
				*valve->port &= ~valve->mask;
				valve->state = VALVE_STATE_UNSCHEDULED;
			}
		}
}


