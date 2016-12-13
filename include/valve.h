#ifndef _VALVE_H_
#define _VALVE_H_


#define VALVE_STATE_UNSCHEDULED		0
#define VALVE_STATE_WAITING_FOR_ON	1
#define VALVE_STATE_WAITING_FOR_OFF	2

struct valve {
	unsigned int triggerontime;
	unsigned int triggerofftime;
	volatile unsigned char *port;
	unsigned char mask;	
	unsigned char state;
	unsigned int ontime;
	unsigned int starttime;
};


void valve_eval (struct valve *valve, unsigned int ms);
void valve_schedule_in_ms (struct valve *valve, unsigned int ms_max, unsigned int current_ms);
void valve_init (struct valve *valve, volatile unsigned char *port, unsigned char pin, unsigned int ontime, unsigned int starttime);

#endif
