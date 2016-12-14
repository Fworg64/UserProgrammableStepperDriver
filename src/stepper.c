//stepper.c

int RPMtotoggletime(int rpm)
{
    return (4.8*1000)/rpm;
}

int toggletimetoRPM(int toggletime)
{
    return RPMtotoggletime(toggletime); //they undo each other
}
