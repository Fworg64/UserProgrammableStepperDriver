/*
* Authors: Marc Olberding, Austin Oltmans
* Company: CTIPP @ NDSU
* Date: 12/5/16
*
* Rev.History
* 12/7/2016 - Added keypad code (Austin)
* 12/12/2016 - added menu code
* 12/14/2016 -refactored
 code for new driver
*
*/

#include <avr/io.h>
#include <avr/interrupt.h>
#include "LCD.h"
#include "usci.h"
#include "menu.h"
#include "timer.h"
#include "eeprom.h"
#include "keypad.h"
#include "stepper.h"

#define MS_MAX	60000
#define MS_SUB_MAX	9

//#define DEFINE define
#define MAINMENU 0
#define RUNNINGMENU 1
#define SETTINGMENU 2
#define FASTFORWARD 3
#define FASTBACKWARD 4
#define FRAMEUPDATEMS 30 //30MS faster than 30 fps

unsigned int ms = 0;
unsigned char framems=0;

unsigned char screen =MAINMENU;
char mainmenustring[33] =    "1.GO XX.XX 3.FFW2.SET RPM  4.FBW";
char runningmenustring[33] = "2.STOP          Going XX.XX RPM ";
char settingsmenustring[33]= "Curr: XX.XX RPM New :   .   RPM ";
char fastforwardstring[33]=  "Fast Forward... Release to stop.";
char fastbackwardstring[33]= "Fast Backward...Release to stop.";
volatile char runframe =1;

char inputcar;
char getanotherkey=1;

//eeprom and rpm stuff
struct eeprom_struct eeprom_rpm_high = {.startaddress = 25, .data = 10, .number_of_redundancy = 5};
struct eeprom_struct eeprom_rpm_low  = {.startaddress = 31, .data = 10, .number_of_redundancy = 5};
int eeprpm_setup();
void eeprpm_write(int rpm2write);
char EEPROM_ERROR=0;
int rpm; //initialized from eeprpm_setup
char rpmdisplaychars[5];
char rpminputbuff[4];
char rpminputindex=0;

T_STEPPER stepper1 = {.stepperport = &PORTC, .stepperreadport = &PINC, .dirpinmask = 1<<PC1, .faultpinmask = 1<<PC0, .steppinmask = 1<<PC2, .togglecomparetime = 240, .dir =1, .internaltimer=0, .enable=0};

int main (void)
{
    DDRC |=  0b00000110;
    PORTC |= 0b00000001;

	USART_init (207); // 9600 baud
	timer_init ();
	lcd_init (USART_transmit_array);
	lcd_reset ();
	lcd_set_backlight (2);
	lcd_set_contrast(50);
	keypad_init();
	sei ();	// enable interrupts
	timer_start ();
	lcd_send_string("Phase 1");
	rpm = eeprpm_setup(); //setup rpm in memory from value potentially stored in eemprom. If no valid value is found a default one is written and loaded.
	lcd_send_string("Phase 2");

	char updatescreen =0;

	rpmdisplaychars[0] = rpm/1000 + '0';
	rpmdisplaychars[1] = rpm/100 -(rpmdisplaychars[0]-'0')*10 +'0';
	rpmdisplaychars[2] = '.';
	rpmdisplaychars[3] = rpm/10 - (rpmdisplaychars[0]-'0')*100 - (rpmdisplaychars[1]-'0')*10 + '0';
	rpmdisplaychars[4] = rpm - (rpmdisplaychars[0]-'0')*1000 - (rpmdisplaychars[1]-'0')*100 - (rpmdisplaychars[3]-'0')*10 + '0';

	for (char looper=0; looper <5; looper++) mainmenustring[5+looper] = rpmdisplaychars[looper];

	lcd_send_string(mainmenustring);
	//wait for lcd to chill out
	for (int awful=0; awful<30000;awful++) for (int awfuler =0; awfuler<10;awfuler++);
	lcd_reset();
	for (int awful=0; awful<30000;awful++) for (int awfuler =0; awfuler<100;awfuler++);
	lcd_send_string(mainmenustring);

	while (9)
	{ if (runframe) { //only if runframe is true;
		runframe = 0; //wait for isr to reset to 1 after 30ms
		//USART_transmit('b');
		//USART_transmit(getanotherkey ? 'a' : 'f');
		pollKeys(); //call this guy everyframe

		//get input if getanotherkey
		if (isKeyPressed() && getanotherkey) // a key was pressed, but not yet released
		{
		    inputcar = getKey(); //get input from input poller
		    getanotherkey=0; //disable next key fetch
		    //USART_transmit(inputcar);
		    //USART_transmit('c');

			switch (screen) //respond to keypress based on current screen
			{
				case MAINMENU:
					if (inputcar == '1')
					{
                        //stepper1.enable =1;
						screen = RUNNINGMENU;
						runningmenustring[22] = rpmdisplaychars[0];
						runningmenustring[23] = rpmdisplaychars[1];
						runningmenustring[25] = rpmdisplaychars[3];
						runningmenustring[26] = rpmdisplaychars[4];
						updatescreen =1; //be sure to call this guy if you want to see anything
					}
					if (inputcar == '2')
					{
						settingsmenustring[6] = rpmdisplaychars[0];
						settingsmenustring[7] = rpmdisplaychars[1];
						settingsmenustring[9] = rpmdisplaychars[3];
						settingsmenustring[10] = rpmdisplaychars[4];
						settingsmenustring[22]= '_';
						settingsmenustring[23]= '_';
						settingsmenustring[25]= '_';
						settingsmenustring[26]= '_';
						screen = SETTINGMENU;
						updatescreen =1;
					}
					if (inputcar =='3')
					{
						screen = FASTFORWARD;
						updatescreen=1;
						//startmotor fastforward
					}
					if (inputcar =='4')
					{
						screen = FASTBACKWARD;
						updatescreen=1;
						//startmotor fastbackward
					}
					break;
				case RUNNINGMENU:
					if (inputcar == '2')
					{
                       // stepper1.enable =0;
                        screen = MAINMENU;
						updatescreen=1;  //be sure to call this guy if you want to see anything
						//stop mot0r
					}
					//spin motor at rpm here
					break;
				case SETTINGMENU:
					if (inputcar != '\0' && inputcar != '*' && inputcar != '#')
					{
						rpminputbuff[rpminputindex] = inputcar;
						rpmdisplaychars[(rpminputindex>1 ? rpminputindex+1 : rpminputindex)] = inputcar;
						mainmenustring[5+(rpminputindex>1 ? rpminputindex+1 : rpminputindex)]= inputcar;
						settingsmenustring[22 +(rpminputindex>1 ? rpminputindex+1 : rpminputindex)] = inputcar;
						if (++rpminputindex ==4)
						{
							screen = MAINMENU;
							rpminputindex=0;
							rpm = (rpminputbuff[0]-'0')*1000 + (rpminputbuff[1] - '0')*100 + (rpminputbuff[2]- '0')*10 + (rpminputbuff[3] - '0');
							eeprpm_write(rpm);
							//stepper1.togglecomparetime = RPMtotoggletime(rpm); //100xRPM to ms
						}
						updatescreen=1;
					}
					break;
				case FASTFORWARD:
					//spin motor quickly here
					break;
				case FASTBACKWARD:
					//spin motor quickly here
					break;
			}


			//(inputcar != '\0') ? USART_transmit('n') : USART_transmit(inputcar);
			//(dont) put code here to be run for any input on any screen
		}

		if (!isKeyPressed() && wasKeyReleased() && !getanotherkey) // key pressed before, need to check if it was released.
		{
		    clearKey();
		    getanotherkey =1;
			//shouldnt do much more here, unless you need to do something on a key release
		    if (screen == FASTFORWARD||screen==FASTBACKWARD)
		    {
			screen = MAINMENU;
			updatescreen=1;
			//stop motor here
		    }
		}

		//display output based on screen and anything else
		if (updatescreen)
		{
			updatescreen =0; //screen update handled, wait for next time,, next time
			switch (screen)
				{
					case MAINMENU:
						lcd_send_string(mainmenustring);
						break;
					case RUNNINGMENU:
						lcd_send_string(runningmenustring);
						break;
					case SETTINGMENU:
						lcd_send_string(settingsmenustring);
						break;
					case FASTFORWARD:
						lcd_send_string(fastforwardstring);
						break;
					case FASTBACKWARD:
						lcd_send_string(fastbackwardstring);
						break;
				}
		}
	}
	//code to run every while

	}
	return 0; //this shouldnt execute.
}


ISR (TIMER1_COMPA_vect)
{
	static unsigned char ms_sub_timer = 0;
	if (ms_sub_timer++ >= MS_SUB_MAX){ //a ms has passed
        	ms_sub_timer = 0;
		if (ms++ >= MS_MAX){ //reset after toomany ms
			ms = 0;
		}

		if (++framems>=FRAMEUPDATEMS) {runframe=1; framems =0;}

		/*if (stepper1.enable)
		{
            (stepper1.stepperport) |= dir & dirpinmask;
            if (stepper1.dir){
                *(stepper1.stepperport) |= (stepper1.dirpinmask);
            }else {
                *(stepper1.stepperport) &= ~(stepper1.dirpinmask);
            }
            if (++stepper1.internaltimer >= stepper1.togglecomparetime)
            {
                stepper1.internaltimer =0;
                *(stepper1.stepperport) ^= stepper1.steppinmask; //flip step bit
            }
            if ((*(stepper1.stepperreadport) & (stepper1.faultpinmask)) == 0) //bad, overheat or something when pin is low
            {
                stepper1.enable = 0;
                runningmenustring[6] = 'F';
                runningmenustring[7] = 'A';
                runningmenustring[8] = 'U';
                runningmenustring[9] = 'L';
                runningmenustring[10] = 'T';
                updatescreen =1;
                //trigger error state
            }
		}
		else
		{
            *(stepper1.stepperport) &= ~(stepper1.steppinmask); //turn off power
		}*/

	}
}

int eeprpm_setup()
{
	//initial dummy struct
	EEPROM_ERROR = eeprom_redundant_read(&eeprom_rpm_high); //check here for eeprom errors
	EEPROM_ERROR += eeprom_redundant_read(&eeprom_rpm_low); //also here
	//check for sanity of values
	if ((eeprom_rpm_high.data >= 100) || (eeprom_rpm_low.data >=100)) EEPROM_ERROR = 1;//ERROR, try to write better values
	if (EEPROM_ERROR >= EEPROM_IS_CORRUPT)
	{
		//set error flag
		eeprom_rpm_high.data = 10;
		eeprom_rpm_low.data = 10;
		eeprom_redundant_write(eeprom_rpm_low);
		eeprom_redundant_write(eeprom_rpm_high);
		mainmenustring[20] = 'E';
		mainmenustring[21] = 'R';
		mainmenustring[22] = 'R';
		mainmenustring[23] = '0';
		mainmenustring[24] = 'R';
	}

	//convert high low chars to int
	//lcd_send_string("Phase 1");
	return eeprom_rpm_high.data * 100 + eeprom_rpm_low.data;

}

void eeprpm_write(int rpm2write)
{
	//split rpm2write into high and low
	eeprom_rpm_high.data = (char) (rpm2write/100);
	eeprom_rpm_low.data = (char) (rpm2write - eeprom_rpm_high.data*100);
	eeprom_redundant_write(eeprom_rpm_high);
	eeprom_redundant_write(eeprom_rpm_low);

}



/*
These are definitions to be placed into TCCR1B
typedef enum timer_prescale_enum {
	STOPPED=0, PRESCALE_1 = 1, PRESCALE_8 = 2,
	PRESCALE_64 =3, PRESCALE_256=4,
	PRESCALE_1024 = 5, EXTERNAL_FALLING=6, EXTERNAL_RISING=7
}t_timer_prescale;

*/


