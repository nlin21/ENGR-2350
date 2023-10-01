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
void Port3_ISR();
void Timer_ISR();

// Add global variables here, as needed.
Timer_A_UpModeConfig timer_config;
uint8_t time[4];
uint8_t flag;

int main (void) /** Main Function **/
{  
    // Add local variables here, as needed.

    // We always call the "SysInit()" first to set up the microcontroller
    // for how we are going to use it.
    SysInit();

    // Place initialization code (or run-once) code here
    Timer_Init();
    GPIO_Init();

    while(1)
    {  
        // Place code that runs continuously in here
        if (flag == 1) {
            // Timer has completed a full period
            GPIO_toggleOutputOnPin(GPIO_PORT_P2, GPIO_PIN4);
            flag = 0;
            UpdateTime();
        }
    }   
}   

// Add function declarations here as needed
void Timer_Init() {
    timer_config.clockSource = TIMER_A_CLOCKSOURCE_SMCLK;
    timer_config.clockSourceDivider = TIMER_A_CLOCKSOURCE_DIVIDER_32;
    timer_config.timerPeriod = 0.01 / (1.0 / (3.0/4.0 * pow(10,6)));

    timer_config.timerInterruptEnable_TAIE = TIMER_A_TAIE_INTERRUPT_ENABLE;
    Timer_A_registerInterrupt(TIMER_A1_BASE, TIMER_A_CCRX_AND_OVERFLOW_INTERRUPT, Timer_ISR);

    Timer_A_configureUpMode(TIMER_A1_BASE, &timer_config);
    Timer_A_startCounter(TIMER_A1_BASE, TIMER_A_UP_MODE);
}

void GPIO_Init() {
    GPIO_setAsOutputPin(GPIO_PORT_P1, GPIO_PIN0);   // LED1
    GPIO_setAsOutputPin(GPIO_PORT_P2, GPIO_PIN4);

    GPIO_setAsInputPinWithPullUpResistor(GPIO_PORT_P4, GPIO_PIN0); // BMP0
    GPIO_setAsInputPinWithPullUpResistor(GPIO_PORT_P4, GPIO_PIN2); // BMP1

    GPIO_registerInterrupt(GPIO_PORT_P4, Port3_ISR);
    GPIO_interruptEdgeSelect(GPIO_PORT_P4, GPIO_PIN0 | GPIO_PIN2, GPIO_HIGH_TO_LOW_TRANSITION);
    GPIO_enableInterrupt(GPIO_PORT_P4, GPIO_PIN0 | GPIO_PIN2);
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
void Port3_ISR() {
    __delay_cycles(240e3);

    uint8_t active_pins = GPIO_getEnabledInterruptStatus(GPIO_PORT_P4);

    if (active_pins & GPIO_PIN0) {
        GPIO_clearInterruptFlag(GPIO_PORT_P4, GPIO_PIN0);
        time[3] += 1;
        if (time[3] == 24) {
            time[3] = 0;
        }
        if (!GPIO_getInputPinValue(GPIO_PORT_P4, GPIO_PIN0)) {}
    }

    if (active_pins & GPIO_PIN2) {
        GPIO_clearInterruptFlag(GPIO_PORT_P4, GPIO_PIN2);
        time[2] += 1;
        if (time[2] == 60) {
            time[2] = 0;
            time[3] += 1;
            if (time[3] == 24) {
                time[3] = 0;
            }
        }
        if (!GPIO_getInputPinValue(GPIO_PORT_P4, GPIO_PIN0)) {}
    }
}

void Timer_ISR() {
    Timer_A_clearInterruptFlag(TIMER_A1_BASE);
    GPIO_toggleOutputOnPin(GPIO_PORT_P1, GPIO_PIN0);
    flag = 1;
}
