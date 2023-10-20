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
void Timer_Init();

// Add global variables here, as needed.
Timer_A_UpModeConfig timer_config;
Timer_A_CompareModeConfig CCR_three, CCR_four;
uint16_t CCR_three_compareValue = 0;
uint16_t CCR_four_compareValue = 0;

int main (void) /** Main Function **/
{  
    // Add local variables here, as needed.

    // We always call the "SysInit()" first to set up the microcontroller
    // for how we are going to use it.
    SysInit();

    // Place initialization code (or run-once) code here
    GPIO_Init();
    Timer_Init();
    int32_t key;

    while(1)
    {  
        // Place code that runs continuously in here
        key = getchar();
        if (key == 'a') {
            if (CCR_four_compareValue + 50 <= 0.9 * 800) {
                CCR_four_compareValue += 50;
            }
            Timer_A_setCompareValue(TIMER_A0_BASE,
                                    TIMER_A_CAPTURECOMPARE_REGISTER_4,
                                    CCR_four_compareValue);
        }
        if (key == 'z') {
            if (CCR_four_compareValue - 50 >= 0) {
                CCR_four_compareValue -= 50;
            }
            Timer_A_setCompareValue(TIMER_A0_BASE,
                                    TIMER_A_CAPTURECOMPARE_REGISTER_4,
                                    CCR_four_compareValue);
        }
        if (key == 's') {
            if (CCR_three_compareValue + 50 <= 0.9 * 800) {
                CCR_three_compareValue += 50;
            }
            Timer_A_setCompareValue(TIMER_A0_BASE,
                                    TIMER_A_CAPTURECOMPARE_REGISTER_3,
                                    CCR_three_compareValue);
        }
        if (key == 'x') {
            if (CCR_three_compareValue - 50 >= 0) {
                CCR_three_compareValue -= 50;
            }
            Timer_A_setCompareValue(TIMER_A0_BASE,
                                    TIMER_A_CAPTURECOMPARE_REGISTER_3,
                                    CCR_three_compareValue);
        }
    }   
}   

// Add function declarations here as needed
void GPIO_Init() {
    GPIO_setAsOutputPin(GPIO_PORT_P3, GPIO_PIN7 | GPIO_PIN3);   // Enable
    GPIO_setAsOutputPin(GPIO_PORT_P5, GPIO_PIN4 | GPIO_PIN5);   // Direction
    GPIO_setOutputHighOnPin(GPIO_PORT_P3, GPIO_PIN7 | GPIO_PIN3);
    GPIO_setOutputLowOnPin(GPIO_PORT_P5, GPIO_PIN4 | GPIO_PIN5);
    GPIO_setAsPeripheralModuleFunctionOutputPin(GPIO_PORT_P2,
                                                GPIO_PIN7 | GPIO_PIN6,
                                                GPIO_PRIMARY_MODULE_FUNCTION);
}

void Timer_Init() {
    timer_config.clockSource = TIMER_A_CLOCKSOURCE_SMCLK;
    timer_config.clockSourceDivider = TIMER_A_CLOCKSOURCE_DIVIDER_1;
    timer_config.timerPeriod = 800;
    timer_config.timerClear = TIMER_A_DO_CLEAR;
    Timer_A_configureUpMode(TIMER_A0_BASE, &timer_config);

    // CCR3, right motor
    CCR_three.compareRegister = TIMER_A_CAPTURECOMPARE_REGISTER_3;
    CCR_three.compareInterruptEnable = TIMER_A_CAPTURECOMPARE_INTERRUPT_DISABLE;
    CCR_three.compareOutputMode = TIMER_A_OUTPUTMODE_RESET_SET;
    CCR_three.compareValue = CCR_three_compareValue;
    Timer_A_initCompare(TIMER_A0_BASE, &CCR_three);

    // CCR4, left motor
    CCR_four.compareRegister = TIMER_A_CAPTURECOMPARE_REGISTER_4;
    CCR_four.compareInterruptEnable = TIMER_A_CAPTURECOMPARE_INTERRUPT_DISABLE;
    CCR_four.compareOutputMode = TIMER_A_OUTPUTMODE_RESET_SET;
    CCR_four.compareValue = CCR_four_compareValue;
    Timer_A_initCompare(TIMER_A0_BASE, &CCR_four);

    Timer_A_startCounter(TIMER_A0_BASE, TIMER_A_UP_MODE);
}

// Add interrupt functions last so they are easy to find
