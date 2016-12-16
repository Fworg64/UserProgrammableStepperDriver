#ifndef _STEPPER_H_
#define _STEPPER_H_

typedef struct mystepper
{
    volatile char* stepperport;
    volatile char *stepperreadport;
    char dirpinmask, faultpinmask, enablepinmask;
    unsigned int togglecomparetime, internaltimer;
    char enable, dir;
} T_STEPPER;

//toggle compare time is in clks
int RPMtofromCompareTime(int RPMorCompareTime); //rpm in 100 x rpm
#endif
