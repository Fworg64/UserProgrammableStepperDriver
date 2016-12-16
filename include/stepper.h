#ifndef _STEPPER_H_
#define _STEPPER_H_

typedef struct mystepper
{
    volatile char* stepperport;
    volatile char *stepperreadport;
    char dirpinmask, faultpinmask, steppinmask;
    unsigned int togglecomparetime, internaltimer;
    char enable, dir;
} T_STEPPER;

//T_STEPPER stepper_maker(volatile char* stepperport, volatile char* dirpinmask, volatile char* faultpinmask, volatile char* steppinmask, int togglecomparetime, int dir, int internaltimer);

int RPMtotoggletime(int rpm); //toggle time in ms
int toggletimetoRPM(int toggletime); //toggletime in ms

#endif
