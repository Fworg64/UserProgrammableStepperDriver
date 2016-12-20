#include "button.h"




char button_struct_check_state (struct button_struct *button){
	char retval = BUTTON_NO_EDGE;
	if (button->state){
		if (button->laststate == 0){
			retval = BUTTON_POS_EDGE;
		}
		button->laststate = 1;
	} else {
		if (button->laststate){
			retval = BUTTON_NEG_EDGE;
		}
		button->laststate = 0;
	}
	return (retval);
}
