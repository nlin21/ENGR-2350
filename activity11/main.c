// ENGR-2350 Template Project
#include "engr2350_msp432.h"
#include <math.h>

void PWMInit();
void ADCInit();
void GPIOInit();
void PWM_ISR();


Timer_A_UpModeConfig TA2cfg; // Using P5.7, TA2.2
Timer_A_CompareModeConfig TA2_ccr;

uint8_t timer_flag = 0;
int16_t pwm_max = 2300; // Maximum limit on PWM output
int16_t pwm_min = 100; // Minimum limit on PWM output
int16_t pwm_set = 1200; // Calculated PWM output (control output)

float kp = 0.1; // proportional control gain
float error_sum = 0; // Integral control error summation
float ki = 0.5; // integral control gain

uint16_t pot_val; // ADC value from potetiometer
float desired; // Current "setpoint" voltage, from POT
uint16_t rc_val; // ADC value from RC circuit
float actual = 0; // Current output voltage from RC circuit

// Main Function
int main(void)
{
    SysInit();

    GPIOInit();
    PWMInit();
    ADCInit();

    printf("\r\n\nDes. ADC\tDes. V\tAct. ADC\tAct. V\tPWM Set\r\n");

    while(1){
        // If the PWM has cycled, request an ADC sample
        if(timer_flag){
            // Add ADC conversion code here

            // Part 1
            // ADC14_toggleConversionTrigger();
            // while (ADC14_isBusy());
            // pot_val = ADC14_getResult(ADC_MEM0);
            // desired = pot_val / pow(2,14) * 3.3;

            // Part 2
            __delay_cycles(10e3);
            ADC14_toggleConversionTrigger();
            while (ADC14_isBusy());
            pot_val = ADC14_getResult(ADC_MEM0);
            desired = (pot_val * 3.3) / pow(2,14);
            rc_val = ADC14_getResult(ADC_MEM1);
            actual = (rc_val * 3.3) / pow(2,14);

            // *********** CONTROL ROUTINE *********** //
            error_sum += desired-actual; // perform "integration"
            pwm_set = kp*(pwm_max-pwm_min)/desired-ki*error_sum; // PI control equation
            if(pwm_set > pwm_max) pwm_set = pwm_max;  // Set limits on the pwm control output
            if(pwm_set < pwm_min) pwm_set = pwm_min;
            Timer_A_setCompareValue(TIMER_A2_BASE,TIMER_A_CAPTURECOMPARE_REGISTER_2,pwm_set); // enforce pwm control output
            // ********* END CONTROL ROUTINE ********* //



            printf("\r%5u\t   %1.3f\t  %5u\t   %1.3f\t%5u",pot_val,desired,rc_val,actual,pwm_set); // report
            __delay_cycles(240e3); // crude delay to prevent this from running too quickly
            timer_flag = 0; // Mark that we've performed the control loop

        }
    }
}

void GPIOInit(){
    GPIO_setAsPeripheralModuleFunctionOutputPin(GPIO_PORT_P5, GPIO_PIN7, GPIO_PRIMARY_MODULE_FUNCTION); // PWM output

    // Add ADC pins

    // Part 1
    // GPIO_setAsPeripheralModuleFunctionInputPin(GPIO_PORT_P6, GPIO_PIN1, GPIO_TERTIARY_MODULE_FUNCTION);

    // Part 2
    GPIO_setAsPeripheralModuleFunctionInputPin(GPIO_PORT_P6, GPIO_PIN1, GPIO_TERTIARY_MODULE_FUNCTION); // A14
    GPIO_setAsPeripheralModuleFunctionInputPin(GPIO_PORT_P6, GPIO_PIN0, GPIO_TERTIARY_MODULE_FUNCTION); // A15
}


void ADCInit(){
    // Activity Stuff...

    // Part 1
    // ADC14_enableModule();
    // ADC14_initModule(ADC_CLOCKSOURCE_SMCLK, ADC_PREDIVIDER_4, ADC_DIVIDER_1, ADC_NOROUTE);
    // ADC14_configureConversionMemory(ADC_MEM0, ADC_VREFPOS_AVCC_VREFNEG_VSS, ADC_INPUT_A14, false); // P6.1
    // ADC14_configureSingleSampleMode(ADC_MEM0, false);
    // ADC14_enableSampleTimer(ADC_MANUAL_ITERATION);
    // ADC14_enableConversion();

    // Part 2
    ADC14_enableModule();
    ADC14_initModule(ADC_CLOCKSOURCE_SMCLK, ADC_PREDIVIDER_4, ADC_DIVIDER_1, ADC_NOROUTE);
    ADC14_configureConversionMemory(ADC_MEM0, ADC_VREFPOS_AVCC_VREFNEG_VSS, ADC_INPUT_A15, false);
    ADC14_configureConversionMemory(ADC_MEM1, ADC_VREFPOS_AVCC_VREFNEG_VSS, ADC_INPUT_A14, false);
    ADC14_configureMultiSequenceMode(ADC_MEM0, ADC_MEM1, false);
    ADC14_enableSampleTimer(ADC_MANUAL_ITERATION);
    ADC14_enableConversion();
}

void PWMInit(){
    // Set up Timer_A2 to run at 10 kHz
    TA2cfg.clockSource = TIMER_A_CLOCKSOURCE_SMCLK;
    TA2cfg.clockSourceDivider = TIMER_A_CLOCKSOURCE_DIVIDER_10;
    TA2cfg.timerPeriod = 2400;
    TA2cfg.timerInterruptEnable_TAIE = TIMER_A_TAIE_INTERRUPT_ENABLE;
    Timer_A_configureUpMode(TIMER_A2_BASE,&TA2cfg);

    // Configure TA2.CCR2 for PWM generation
    TA2_ccr.compareOutputMode = TIMER_A_OUTPUTMODE_SET_RESET;
    TA2_ccr.compareValue = pwm_set;
    TA2_ccr.compareRegister = TIMER_A_CAPTURECOMPARE_REGISTER_2;
    Timer_A_initCompare(TIMER_A2_BASE,&TA2_ccr);

    Timer_A_registerInterrupt(TIMER_A2_BASE,TIMER_A_CCRX_AND_OVERFLOW_INTERRUPT,PWM_ISR);
    Timer_A_startCounter(TIMER_A2_BASE,TIMER_A_UP_MODE);
}

void PWM_ISR(){
    Timer_A_clearInterruptFlag(TIMER_A2_BASE);
    timer_flag = 1;
}

