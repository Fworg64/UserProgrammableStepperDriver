//stepper.c
#include "stepper.h"

int RPMtotoggletime(int rpm)
{
    return (4.8*1000)/rpm; //toggletime in ms
}

int toggletimetoRPM(int toggletime)
{
    return RPMtotoggletime(toggletime); //they undo each other
}
