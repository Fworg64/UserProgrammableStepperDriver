//stepper.c
#include "stepper.h"

int RPMtofromCompareTime(int RPMorCompareTime)
{
    return 7500000/RPMorCompareTime; //rpm is rpm*100
}
