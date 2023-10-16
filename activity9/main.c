/**********************************************************************/
//** ENGR-2350 Template Project
//** Name: Nicky Lin
//** RIN: 662041711
//** This is the base project for several activities and labs throughout
//** the course.  The outline provided below isn't necessarily *required*
//** by a C program; however, this format is required within ENGR-2350
//** to ease debugging/grading by the staff.
/**********************************************************************/

// We'll always add this include statement. This basically takes the
// code contained within the "engr_2350_msp432.h" file and adds it here.
#include "engr2350_msp432.h"

// Add function prototypes here, as needed.
void GPIO_Init();
void setAllPinsLow();

// Add global variables here, as needed.


int main (void) /** Main Function **/
{
    // Add local variables here, as needed.

    // We always call the "SysInit()" first to set up the microcontroller
    // for how we are going to use it.
    SysInit();

    // Place initialization code (or run-once) code here
    GPIO_Init();

    while(1)
    {  
        // Place code that runs continuously in here
        uint8_t input = getchar();
        if (input == 'f') {
            setAllPinsLow();
            __delay_cycles(240);
            GPIO_setOutputHighOnPin(GPIO_PORT_P6, GPIO_PIN7);
            GPIO_setOutputHighOnPin(GPIO_PORT_P3, GPIO_PIN2);
        }
        if (input == 'r') {
            setAllPinsLow();
            __delay_cycles(240);
            GPIO_setOutputHighOnPin(GPIO_PORT_P3, GPIO_PIN3);
            GPIO_setOutputHighOnPin(GPIO_PORT_P6, GPIO_PIN6);
        }
        if (input == 's') {
            setAllPinsLow();
        }

    }   
}   

// Add function declarations here as needed
void GPIO_Init() {
    GPIO_setAsOutputPin(GPIO_PORT_P3, GPIO_PIN2 | GPIO_PIN3);
    GPIO_setAsOutputPin(GPIO_PORT_P6, GPIO_PIN6 | GPIO_PIN7);
    GPIO_setOutputLowOnPin(GPIO_PORT_P3, GPIO_PIN2 | GPIO_PIN3);
    GPIO_setOutputLowOnPin(GPIO_PORT_P3, GPIO_PIN6 | GPIO_PIN7);
}

void setAllPinsLow() {
    GPIO_setOutputLowOnPin(GPIO_PORT_P3, GPIO_PIN2 | GPIO_PIN3);
    GPIO_setOutputLowOnPin(GPIO_PORT_P6, GPIO_PIN6 | GPIO_PIN7);
}

// Add interrupt functions last so they are easy to find
