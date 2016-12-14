//stepper.c
#include "stepper.h"

int RPMtotoggletime(int rpm)
{
    return (4.8*100000)/rpm; //toggletime in ms rpm in 100xrpm
}

int toggletimetoRPM(int toggletime)
{
    return RPMtotoggletime(toggletime); //they undo each other
}
