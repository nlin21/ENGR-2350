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
#include <math.h>


// Add function prototypes here, as needed.
void Timer_Init();
void GPIO_Init();
void UpdateTime();

// Add global variables here, as needed.
Timer_A_UpModeConfig timer_config;
uint8_t time[4];

int main (void) /** Main Function **/
{  
    // Add local variables here, as needed.
    uint8_t counter = 0;

    // We always call the "SysInit()" first to set up the microcontroller
    // for how we are going to use it.
    SysInit();

    // Place initialization code (or run-once) code here
    Timer_Init();
    GPIO_Init();

    while(1)
    {  
        // Place code that runs continuously in here
        if (Timer_A_getInterruptStatus(TIMER_A1_BASE) == TIMER_A_INTERRUPT_PENDING) {
            // Timer has completed a full period
            GPIO_toggleOutputOnPin(GPIO_PORT_P2, GPIO_PIN4);
            counter++;
            Timer_A_clearInterruptFlag(TIMER_A1_BASE);
            UpdateTime();
        }
        if (counter == 10) {
            GPIO_toggleOutputOnPin(GPIO_PORT_P1, GPIO_PIN0);
            counter = 0;
        }
    }   
}   

// Add function declarations here as needed
void Timer_Init() {
    timer_config.clockSource = TIMER_A_CLOCKSOURCE_SMCLK;
    timer_config.clockSourceDivider = TIMER_A_CLOCKSOURCE_DIVIDER_32;
    timer_config.timerPeriod = 0.01 / (1.0 / (3.0/4.0 * pow(10,6)));

    // timer_config.clockSourceDivider = TIMER_A_CLOCKSOURCE_DIVIDER_64;
    // timer_config.timerPeriod = 0.01 / (1.0 / (1.0/2.0 * pow(10,6)));

    Timer_A_configureUpMode(TIMER_A1_BASE, &timer_config);
    Timer_A_startCounter(TIMER_A1_BASE, TIMER_A_UP_MODE);
}

void GPIO_Init() {
    GPIO_setAsOutputPin(GPIO_PORT_P1, GPIO_PIN0);   // LED1
    GPIO_setAsOutputPin(GPIO_PORT_P2, GPIO_PIN4);
}

void UpdateTime() {
    time[0]++;                      // Increment hundredths of seconds
    if (time[0] == 100) {           // If a whole second has passed...
        time[0] = 0;                // Reset tenths of seconds
        time[1]++;                  // And increment seconds
        if (time[1] == 60) {        // If a minute has passed...
            time[1] = 0;            // Reset seconds
            time[2]++;              // Increment minutes
            if (time[2] == 60) {    // and so on...
                time[2] = 0;
                time[3]++;
                if (time[3] == 24) {
                    time[3] = 0;
                }
            }
        }
    }
    if ((time[0] % 10) == 0) {   // Print time every 100ms
        printf("%2u:%02u:%02u.%u\r",time[3],time[2],time[1],time[0]);
    }
}

// Add interrupt functions last so they are easy to find
