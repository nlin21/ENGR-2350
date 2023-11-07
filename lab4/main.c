// ENGR-2350 Lab 4 Template S23

// README!!!!
// README!!!!
// README!!!!

// This template project has all initializations required to both control the motors
// via PWM and measure the speed of the motors. The PWM is configured using a 30 kHz
// period (800 counts). The motors are initialized to be DISABLED and in FORWARD mode.
// The encoders measurements are stored within the variables Tach_R and Tach_L for the
// right and left motors, respectively. A maximum value for Tach_R and Tach_L is
// enforced to be 1e6 such that when the wheel stops, a reasonable value for the
// encoders exists: a very large number that can be assumed to be stopped.
// Finally, a third timer is added to measure a 100 ms period for control system
// timing. The variable run_control is set to 1 each period and then reset in the main.

#include "engr2350_msp432.h"
#include <math.h>

void GPIOInit();
void TimerInit();
void ADCInit();
void Encoder_ISR();
void T1_100ms_ISR();

Timer_A_UpModeConfig TA0cfg; // PWM timer
Timer_A_UpModeConfig TA1cfg; // 100 ms timer
Timer_A_ContinuousModeConfig TA3cfg; // Encoder timer
Timer_A_CompareModeConfig TA0_ccr3; // PWM Right
Timer_A_CompareModeConfig TA0_ccr4; // PWM Left
Timer_A_CaptureModeConfig TA3_ccr0; // Encoder Right
Timer_A_CaptureModeConfig TA3_ccr1; // Encoder Left


// Encoder total events
uint32_t enc_total_L,enc_total_R;
// Speed measurement variables
int32_t Tach_L_count,Tach_L,Tach_L_sum,Tach_L_sum_count,Tach_L_avg; // Left wheel
int32_t Tach_R_count,Tach_R,Tach_R_sum,Tach_R_sum_count,Tach_R_avg; // Right wheel
    // Tach_L_avg is the averaged value after every 6 encoder measurements
    // The rest are the intermediate variables used to assemble Tach_L_avg

uint8_t run_control = 0; // Flag to denote that 100ms has passed and control should be run.

uint16_t pot_val;
uint16_t desired_speed, current_speed_L, current_speed_R;
int32_t error_sum_L = 0;
int32_t error_sum_R = 0;
int16_t pwm_max = 720; // Maximum limit on PWM output
int16_t pwm_min = 80; // Minimum limit on PWM output
int16_t pwm_set_L = 100;
int16_t pwm_set_R = 100;
float ki = 0.1;

int main(void)
{
    SysInit();
    GPIOInit();
    ADCInit();
    TimerInit();

    __delay_cycles(24e6);
    GPIO_setOutputHighOnPin(GPIO_PORT_P3,GPIO_PIN6|GPIO_PIN7);

    while(1){
        if(run_control){    // If 100 ms has passed
            run_control = 0;    // Reset the 100 ms flag
            // Control routine ... Explicity follow pseudocode from Lab document

            ADC14_toggleConversionTrigger();
            while (ADC14_isBusy());
            pot_val = ADC14_getResult(ADC_MEM0);

            // For Part A, do:
//                Convert the speed potentiometer to a desired speed
//                Ensure the desired speed follows the control specifications below.
//                <-- Start Wheel Speed Control -->
//                For Each Wheel:
//                    calculate desired wheel speed as desired speed +/- differential speed
//                                        For Part A: differential speed == 0
//                    if desired wheel speed is less than minimum value,
//                        set the desired wheel speed and compare value to 0
//                    otherwise:
//                        calculate the current speed error from the encoder and desired wheel speed
//                                     (use trendline equation from figure for current speed)
//                        add current speed error to an "error sum" variable (int32_t is a good type for this!)
//                        calculate the corrected speed using the discrete control equation given
//                        convert the corrected speed (duty cycle) into a compare value
//                        enforce minimum and maximum limits on the compare value
//                    apply the compare value
//                <-- End Wheel Speed Control -->

            desired_speed = (320/pow(2,14)) * pot_val + 80;
            if (desired_speed < 80) {
                desired_speed = 0;
                pwm_set_L = 0;
                pwm_set_R = 0;
            } else {
                current_speed_L = 1500000 / Tach_L * 8;
                error_sum_L += (desired_speed - current_speed_L);
                pwm_set_L = desired_speed + ki * error_sum_L;
                if(pwm_set_L > pwm_max) pwm_set_L = pwm_max;
                if(pwm_set_L < pwm_min) pwm_set_L = pwm_min;

                current_speed_R = 1500000 / Tach_R * 8;
                error_sum_R += (desired_speed - current_speed_R);
                pwm_set_R = desired_speed + ki * error_sum_R;
                if(pwm_set_R > pwm_max) pwm_set_R = pwm_max;
                if(pwm_set_R < pwm_min) pwm_set_R = pwm_min;

                printf("Current_L: %u, Current_R: %u\r\n", current_speed_L, current_speed_R);

            }
            Timer_A_setCompareValue(TIMER_A0_BASE,TIMER_A_CAPTURECOMPARE_REGISTER_4,pwm_set_L);
            Timer_A_setCompareValue(TIMER_A0_BASE,TIMER_A_CAPTURECOMPARE_REGISTER_3,pwm_set_R);

            // For Part B - add in missing pieces!
        }
    }
}

void ADCInit(){
    // Add your ADC initialization code here.
    //  Don't forget the GPIO, either here or in GPIOInit()!!
    ADC14_enableModule();
    ADC14_initModule(ADC_CLOCKSOURCE_SMCLK, ADC_PREDIVIDER_4, ADC_DIVIDER_1, ADC_NOROUTE);
    ADC14_setResolution(ADC_14BIT);
    ADC14_configureConversionMemory(ADC_MEM0, ADC_VREFPOS_AVCC_VREFNEG_VSS, ADC_INPUT_A12, false);
    ADC14_configureConversionMemory(ADC_MEM1, ADC_VREFPOS_AVCC_VREFNEG_VSS, ADC_INPUT_A9, false);
    ADC14_configureMultiSequenceMode(ADC_MEM0, ADC_MEM1, false);
    ADC14_enableSampleTimer(ADC_MANUAL_ITERATION);
    ADC14_enableConversion();

}

void GPIOInit(){
    GPIO_setAsOutputPin(GPIO_PORT_P5,GPIO_PIN4|GPIO_PIN5);   // Motor direction pins
    GPIO_setAsOutputPin(GPIO_PORT_P3,GPIO_PIN6|GPIO_PIN7);   // Motor enable pins
        // Motor PWM pins
    GPIO_setAsPeripheralModuleFunctionOutputPin(GPIO_PORT_P2,GPIO_PIN6|GPIO_PIN7,GPIO_PRIMARY_MODULE_FUNCTION);
        // Motor Encoder pins
    GPIO_setAsPeripheralModuleFunctionInputPin(GPIO_PORT_P10,GPIO_PIN4|GPIO_PIN5,GPIO_PRIMARY_MODULE_FUNCTION);

    GPIO_setOutputLowOnPin(GPIO_PORT_P5,GPIO_PIN4|GPIO_PIN5);   // Motors set to forward
    GPIO_setOutputLowOnPin(GPIO_PORT_P3,GPIO_PIN6|GPIO_PIN7);   // Motors are OFF

    GPIO_setAsPeripheralModuleFunctionInputPin(GPIO_PORT_P4, GPIO_PIN1, GPIO_TERTIARY_MODULE_FUNCTION); // A12
    GPIO_setAsPeripheralModuleFunctionInputPin(GPIO_PORT_P4, GPIO_PIN4, GPIO_TERTIARY_MODULE_FUNCTION); // A9
}

void TimerInit(){
    // Configure PWM timer for 30 kHz
    TA0cfg.clockSource = TIMER_A_CLOCKSOURCE_SMCLK;
    TA0cfg.clockSourceDivider = TIMER_A_CLOCKSOURCE_DIVIDER_1;
    TA0cfg.timerPeriod = 800;
    Timer_A_configureUpMode(TIMER_A0_BASE,&TA0cfg);
    // Configure TA0.CCR3 for PWM output, Right Motor
    TA0_ccr3.compareRegister = TIMER_A_CAPTURECOMPARE_REGISTER_3;
    TA0_ccr3.compareOutputMode = TIMER_A_OUTPUTMODE_RESET_SET;
    TA0_ccr3.compareValue = 0;
    Timer_A_initCompare(TIMER_A0_BASE,&TA0_ccr3);
    // Configure TA0.CCR4 for PWM output, Left Motor
    TA0_ccr4.compareRegister = TIMER_A_CAPTURECOMPARE_REGISTER_4;
    TA0_ccr4.compareOutputMode = TIMER_A_OUTPUTMODE_RESET_SET;
    TA0_ccr4.compareValue = 0;
    Timer_A_initCompare(TIMER_A0_BASE,&TA0_ccr4);
    // Configure Encoder timer in continuous mode
    TA3cfg.clockSource = TIMER_A_CLOCKSOURCE_SMCLK;
    TA3cfg.clockSourceDivider = TIMER_A_CLOCKSOURCE_DIVIDER_1;
    TA3cfg.timerInterruptEnable_TAIE = TIMER_A_TAIE_INTERRUPT_ENABLE;
    Timer_A_configureContinuousMode(TIMER_A3_BASE,&TA3cfg);
    // Configure TA3.CCR0 for Encoder measurement, Right Encoder
    TA3_ccr0.captureRegister = TIMER_A_CAPTURECOMPARE_REGISTER_0;
    TA3_ccr0.captureMode = TIMER_A_CAPTUREMODE_RISING_EDGE;
    TA3_ccr0.captureInputSelect = TIMER_A_CAPTURE_INPUTSELECT_CCIxA;
    TA3_ccr0.synchronizeCaptureSource = TIMER_A_CAPTURE_SYNCHRONOUS;
    TA3_ccr0.captureInterruptEnable = TIMER_A_CAPTURECOMPARE_INTERRUPT_ENABLE;
    Timer_A_initCapture(TIMER_A3_BASE,&TA3_ccr0);
    // Configure TA3.CCR1 for Encoder measurement, Left Encoder
    TA3_ccr1.captureRegister = TIMER_A_CAPTURECOMPARE_REGISTER_1;
    TA3_ccr1.captureMode = TIMER_A_CAPTUREMODE_RISING_EDGE;
    TA3_ccr1.captureInputSelect = TIMER_A_CAPTURE_INPUTSELECT_CCIxA;
    TA3_ccr1.synchronizeCaptureSource = TIMER_A_CAPTURE_SYNCHRONOUS;
    TA3_ccr1.captureInterruptEnable = TIMER_A_CAPTURECOMPARE_INTERRUPT_ENABLE;
    Timer_A_initCapture(TIMER_A3_BASE,&TA3_ccr1);
    // Register the Encoder interrupt
    Timer_A_registerInterrupt(TIMER_A3_BASE,TIMER_A_CCR0_INTERRUPT,Encoder_ISR);
    Timer_A_registerInterrupt(TIMER_A3_BASE,TIMER_A_CCRX_AND_OVERFLOW_INTERRUPT,Encoder_ISR);
    // Configure 10 Hz timer
    TA1cfg.clockSource = TIMER_A_CLOCKSOURCE_SMCLK;
    TA1cfg.clockSourceDivider = TIMER_A_CLOCKSOURCE_DIVIDER_64;
    TA1cfg.timerInterruptEnable_TAIE = TIMER_A_TAIE_INTERRUPT_ENABLE;
    TA1cfg.timerPeriod = 37500;
    Timer_A_configureUpMode(TIMER_A1_BASE,&TA1cfg);
    Timer_A_registerInterrupt(TIMER_A1_BASE,TIMER_A_CCRX_AND_OVERFLOW_INTERRUPT,T1_100ms_ISR);
    // Start all the timers
    Timer_A_startCounter(TIMER_A0_BASE,TIMER_A_UP_MODE);
    Timer_A_startCounter(TIMER_A1_BASE,TIMER_A_UP_MODE);
    Timer_A_startCounter(TIMER_A3_BASE,TIMER_A_CONTINUOUS_MODE);
}


void Encoder_ISR(){
    // If encoder timer has overflowed...
    if(Timer_A_getEnabledInterruptStatus(TIMER_A3_BASE) == TIMER_A_INTERRUPT_PENDING){
        Timer_A_clearInterruptFlag(TIMER_A3_BASE);
        Tach_R_count += 65536;
        if(Tach_R_count >= 1e6){ // Enforce a maximum count to Tach_R so stopped can be detected
            Tach_R_count = 1e6;
            Tach_R = 1e6;
        }
        Tach_L_count += 65536;
        if(Tach_L_count >= 1e6){ // Enforce a maximum count to Tach_L so stopped can be detected
            Tach_L_count = 1e6;
            Tach_L = 1e6;
        }
    // Otherwise if the Left Encoder triggered...
    }else if(Timer_A_getCaptureCompareEnabledInterruptStatus(TIMER_A3_BASE,TIMER_A_CAPTURECOMPARE_REGISTER_0)&TIMER_A_CAPTURECOMPARE_INTERRUPT_FLAG){
        Timer_A_clearCaptureCompareInterrupt(TIMER_A3_BASE,TIMER_A_CAPTURECOMPARE_REGISTER_0);
        enc_total_R++;   // Increment the total number of encoder events for the left encoder
        // Calculate and track the encoder count values
        Tach_R = Tach_R_count + Timer_A_getCaptureCompareCount(TIMER_A3_BASE,TIMER_A_CAPTURECOMPARE_REGISTER_0);
        Tach_R_count = -Timer_A_getCaptureCompareCount(TIMER_A3_BASE,TIMER_A_CAPTURECOMPARE_REGISTER_0);
        // Sum values for averaging
        Tach_R_sum_count++;
        Tach_R_sum += Tach_R;
        // If 6 values have been received, average them.
        if(Tach_R_sum_count == 6){
            Tach_R_avg = Tach_R_sum/6;
            Tach_R_sum_count = 0;
            Tach_R_sum = 0;
        }
    // Otherwise if the Right Encoder triggered...
    }else if(Timer_A_getCaptureCompareEnabledInterruptStatus(TIMER_A3_BASE,TIMER_A_CAPTURECOMPARE_REGISTER_1)&TIMER_A_CAPTURECOMPARE_INTERRUPT_FLAG){
        Timer_A_clearCaptureCompareInterrupt(TIMER_A3_BASE,TIMER_A_CAPTURECOMPARE_REGISTER_1);
        enc_total_L++;
        Tach_L = Tach_L_count + Timer_A_getCaptureCompareCount(TIMER_A3_BASE,TIMER_A_CAPTURECOMPARE_REGISTER_1);
        Tach_L_count = -Timer_A_getCaptureCompareCount(TIMER_A3_BASE,TIMER_A_CAPTURECOMPARE_REGISTER_1);
        Tach_L_sum_count++;
        Tach_L_sum += Tach_L;
        if(Tach_L_sum_count == 6){
            Tach_L_avg = Tach_L_sum/6;
            Tach_L_sum_count = 0;
            Tach_L_sum = 0;
        }
    }
}

void T1_100ms_ISR(){
    Timer_A_clearInterruptFlag(TIMER_A1_BASE);
    run_control = 1;
}
