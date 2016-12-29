//stepper.c
#include "stepper.h"

void start_stepper (struct stepper_driver_struct *step);
void stop_stepper (struct stepper_driver_struct *step);
void binary_to_string (unsigned char bin, char *str);
void int_to_string (unsigned int number, char *str);
void release_stepper (struct stepper_driver_struct *step);



int RPMtofromCompareTime(int RPMorCompareTime)
{
    return 7500000/RPMorCompareTime; //rpm is rpm*100
}
