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
void GPIO_Init();
void Timer_Init();
void CCR_ISR();

// Add global variables here, as needed.
Timer_A_UpModeConfig timer_config_PWM;          // Timer to generate PWM at 30 kHz
Timer_A_CompareModeConfig CCR_three, CCR_four;  // CCR_three: right motor, CCR_four: left motor
uint16_t CCR_three_compareValue = 200;
uint16_t CCR_four_compareValue = 200;           // Give initial speed to allow a capture interrupt

Timer_A_ContinuousModeConfig timer_config_ENC;  // Timer for encoders to detect speed
Timer_A_CaptureModeConfig CCR_zero;             // CCR_zero: right encoder

uint32_t enc_total;
int32_t enc_counts_track;
int32_t enc_counts;
uint8_t enc_flag;
uint16_t capture_value;

int32_t setpoint = 65000;
uint8_t WHEEL_RADIUS_IN_MM = 35;
uint16_t distance;

uint8_t bmp0;
uint32_t encoder_events[250];
uint8_t encoder_events_counter = 0;

int main (void) /** Main Function **/
{  
    // Add local variables here, as needed.

    // We always call the "SysInit()" first to set up the microcontroller
    // for how we are going to use it.
    SysInit();

    // Place initialization code (or run-once) code here
    GPIO_Init();
    Timer_Init();
    uint8_t i;

    while(1)
    {  
        // Place code that runs continuously in here
        bmp0 = !GPIO_getInputPinValue(GPIO_PORT_P4, GPIO_PIN0);
        while (!bmp0) {
            bmp0 = !GPIO_getInputPinValue(GPIO_PORT_P4, GPIO_PIN0);
        }
        __delay_cycles(24e6);
        encoder_events_counter = 0;
        GPIO_setOutputHighOnPin(GPIO_PORT_P3, GPIO_PIN7 | GPIO_PIN6);
        while (encoder_events_counter < 250);
        GPIO_setOutputLowOnPin(GPIO_PORT_P3, GPIO_PIN7 | GPIO_PIN6);
        __delay_cycles(12e6);
        printf("Sample\tValue\r\n");
        for (i = 0; i < 250; ++i) {
            printf("%u\t%u\r\n",i,encoder_events[i]);
        }
    }   
}   

// Add function declarations here as needed
void GPIO_Init() {
    GPIO_setAsOutputPin(GPIO_PORT_P5, GPIO_PIN4 | GPIO_PIN5);                   // Direction. P5.4: left motor, P5.5: right motor
    GPIO_setOutputLowOnPin(GPIO_PORT_P5, GPIO_PIN4 | GPIO_PIN5);                // Wheels will rotate forward if Direction is low

    GPIO_setAsOutputPin(GPIO_PORT_P3, GPIO_PIN7 | GPIO_PIN6);                   // Enable. P3.7: left motor, P3.6, right motor
    GPIO_setOutputLowOnPin(GPIO_PORT_P3, GPIO_PIN7 | GPIO_PIN6);

    GPIO_setAsPeripheralModuleFunctionOutputPin(GPIO_PORT_P2,                   // Speed
                                                GPIO_PIN7 | GPIO_PIN6,          // P2.7: left motor, P2.6: right motor
                                                GPIO_PRIMARY_MODULE_FUNCTION);
    GPIO_setAsPeripheralModuleFunctionInputPin(GPIO_PORT_P10,                   // Encoders
                                               GPIO_PIN4 | GPIO_PIN5,           // P10.4: left encoder, P10.5: right encoder
                                               GPIO_PRIMARY_MODULE_FUNCTION);

    GPIO_setAsInputPinWithPullUpResistor(GPIO_PORT_P4, GPIO_PIN0);              // BMP0
}

void Timer_Init() {
    // Timer to generate PWM at 30 kHz
    timer_config_PWM.clockSource = TIMER_A_CLOCKSOURCE_SMCLK;
    timer_config_PWM.clockSourceDivider = TIMER_A_CLOCKSOURCE_DIVIDER_1;
    timer_config_PWM.timerPeriod = 800;
    timer_config_PWM.timerClear = TIMER_A_DO_CLEAR;
    Timer_A_configureUpMode(TIMER_A0_BASE, &timer_config_PWM);

    // CCR3: right motor
    CCR_three.compareRegister = TIMER_A_CAPTURECOMPARE_REGISTER_3;
    CCR_three.compareInterruptEnable = TIMER_A_CAPTURECOMPARE_INTERRUPT_DISABLE;
    CCR_three.compareOutputMode = TIMER_A_OUTPUTMODE_RESET_SET;
    CCR_three.compareValue = CCR_three_compareValue;
    Timer_A_initCompare(TIMER_A0_BASE, &CCR_three);

    // CCR4: left motor
    CCR_four.compareRegister = TIMER_A_CAPTURECOMPARE_REGISTER_4;
    CCR_four.compareInterruptEnable = TIMER_A_CAPTURECOMPARE_INTERRUPT_DISABLE;
    CCR_four.compareOutputMode = TIMER_A_OUTPUTMODE_RESET_SET;
    CCR_four.compareValue = CCR_four_compareValue;
    Timer_A_initCompare(TIMER_A0_BASE, &CCR_four);

    // Timer for encoders to detect speed
    timer_config_ENC.clockSource = TIMER_A_CLOCKSOURCE_SMCLK;
    timer_config_ENC.clockSourceDivider = TIMER_A_CLOCKSOURCE_DIVIDER_1;
    timer_config_ENC.timerInterruptEnable_TAIE = TIMER_A_TAIE_INTERRUPT_ENABLE;
    timer_config_ENC.timerClear = TIMER_A_DO_CLEAR;
    Timer_A_configureContinuousMode(TIMER_A3_BASE, &timer_config_ENC);

    // CCR0: right encoder
    CCR_zero.captureRegister = TIMER_A_CAPTURECOMPARE_REGISTER_0;
    CCR_zero.captureMode = TIMER_A_CAPTUREMODE_RISING_EDGE;
    CCR_zero.captureInputSelect = TIMER_A_CAPTURE_INPUTSELECT_CCIxA;
    CCR_zero.synchronizeCaptureSource = TIMER_A_CAPTURE_SYNCHRONOUS;
    CCR_zero.captureInterruptEnable = TIMER_A_CAPTURECOMPARE_INTERRUPT_ENABLE;
    Timer_A_initCapture(TIMER_A3_BASE, &CCR_zero);

    Timer_A_registerInterrupt(TIMER_A3_BASE, TIMER_A_CCRX_AND_OVERFLOW_INTERRUPT, CCR_ISR);
    Timer_A_registerInterrupt(TIMER_A3_BASE, TIMER_A_CCR0_INTERRUPT, CCR_ISR);
    Timer_A_startCounter(TIMER_A3_BASE, TIMER_A_CONTINUOUS_MODE);
    Timer_A_startCounter(TIMER_A0_BASE, TIMER_A_UP_MODE);
}

// Add interrupt functions last so they are easy to find
void CCR_ISR() {
    if (Timer_A_getEnabledInterruptStatus(TIMER_A3_BASE) == TIMER_A_INTERRUPT_PENDING) {
        Timer_A_clearInterruptFlag(TIMER_A3_BASE);
        enc_counts_track += 65536;
    }
    if (Timer_A_getCaptureCompareEnabledInterruptStatus(TIMER_A3_BASE, TIMER_A_CAPTURECOMPARE_REGISTER_0) & TIMER_A_CAPTURECOMPARE_INTERRUPT_FLAG) {
        Timer_A_clearCaptureCompareInterrupt(TIMER_A3_BASE, TIMER_A_CAPTURECOMPARE_REGISTER_0);
        enc_total++;
        capture_value = Timer_A_getCaptureCompareCount(TIMER_A3_BASE, TIMER_A_CAPTURECOMPARE_REGISTER_0);
        enc_counts = enc_counts_track + capture_value;
        enc_counts_track = -capture_value;
        enc_flag = 1;

        if (encoder_events_counter < 250) {
            encoder_events[encoder_events_counter] = enc_counts;
            encoder_events_counter++;
        }
    }
}
