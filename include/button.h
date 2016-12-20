#ifndef _BUTTON_H_
#define _BUTTON_H_


#define BUTTON_NEG_EDGE	-1
#define BUTTON_POS_EDGE	0
#define BUTTON_NO_EDGE	1

struct button_struct {
	unsigned char state;
	unsigned char laststate;
};



char button_struct_check_state (struct button_struct *butt);



#endif
