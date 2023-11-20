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
void I2CInit();
void I2C_writeData(uint32_t moduleInstance, uint8_t PeriphAddress, uint8_t StartReg, uint8_t * data , uint8_t len);
void I2C_readData(uint32_t moduleInstance, uint8_t PeriphAddress, uint8_t StartReg, uint8_t * data, uint8_t len);
uint16_t readCompass();
uint16_t readRanger();
void TimerInit();
void Encoder_ISR();
void T1_100ms_ISR();

eUSCI_I2C_MasterConfig eUSCI_config;
uint16_t heading;
uint16_t range;

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
uint8_t tracker = 0; // Tracker for 10s.

uint16_t desired_distance = 60; // 60 cm
int16_t desired_speed, desired_speed_L, desired_speed_R;
int16_t current_speed_L, current_speed_R;

int32_t error_sum_L = 0;
int32_t error_sum_R = 0;
int16_t pwm_max = 400; // Maximum limit on PWM output
int16_t pwm_min = 100; // Minimum limit on PWM output
int16_t pwm_set_L = 0;
int16_t pwm_set_R = 0;
uint8_t DISTANCE_BETWEEN_WHEELS_IN_MM = 149;
uint8_t WHEEL_RADIUS_IN_MM = 35;
float ki = 0.2;
float kp = -7;

int main(void)
{
    SysInit();
    GPIOInit();
    I2CInit();
    TimerInit();

    __delay_cycles(24e6);
    GPIO_setOutputHighOnPin(GPIO_PORT_P3,GPIO_PIN6|GPIO_PIN7);

    while(1){
        if (run_control) {
            // Start distance control
            run_control = 0;
            readRanger();
            __delay_cycles(2.4e6); // Wait 1/10 of a second
            desired_speed = kp * (desired_distance - range);

            printf("desired_speed: %d\r\n", desired_speed);

            // Start wheel speed control
            desired_speed_L = desired_speed;
            desired_speed_R = desired_speed;
            if (abs(desired_speed_L) < pwm_min) {
                desired_speed_L = 0;
                pwm_set_L = 0;
                error_sum_L = 0;
            } else {
                current_speed_L = 1500000.0 / Tach_L_avg * 8;
                error_sum_L += (desired_speed_L - current_speed_L);
                pwm_set_L = desired_speed_L + ki * error_sum_L;
                if (pwm_set_L > 0) {
                    GPIO_setOutputLowOnPin(GPIO_PORT_P5,GPIO_PIN4);   // Forward
                } else if (pwm_set_L < 0) {
                    GPIO_setOutputHighOnPin(GPIO_PORT_P5,GPIO_PIN4);  // Reverse
                }

                pwm_set_L = abs(pwm_set_L);
                if(pwm_set_L > pwm_max) pwm_set_L = pwm_max;
                if(pwm_set_L < pwm_min) pwm_set_L = pwm_min;

            }
            if (abs(desired_speed_R) < pwm_min) {
                desired_speed_R = 0;
                pwm_set_R = 0;
                error_sum_R = 0;
            } else {
                current_speed_R = 1500000.0 / Tach_R_avg * 8;
                error_sum_R += (desired_speed_R - current_speed_R);
                pwm_set_R = desired_speed_R + ki * error_sum_R;
                if (pwm_set_R > 0) {
                    GPIO_setOutputLowOnPin(GPIO_PORT_P5,GPIO_PIN5);   // Forward
                } else if (pwm_set_R < 0) {
                    GPIO_setOutputHighOnPin(GPIO_PORT_P5,GPIO_PIN5);  // Reverse
                }

                pwm_set_R = abs(pwm_set_R);
                if(pwm_set_R > pwm_max) pwm_set_R = pwm_max;
                if(pwm_set_R < pwm_min) pwm_set_R = pwm_min;

            }
            Timer_A_setCompareValue(TIMER_A0_BASE,TIMER_A_CAPTURECOMPARE_REGISTER_4,pwm_set_L);
            Timer_A_setCompareValue(TIMER_A0_BASE,TIMER_A_CAPTURECOMPARE_REGISTER_3,pwm_set_R);
        }
    }
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

    GPIO_setAsPeripheralModuleFunctionOutputPin(GPIO_PORT_P1, GPIO_PIN6, GPIO_PRIMARY_MODULE_FUNCTION); // eUSCI_B0 SDA
    GPIO_setAsPeripheralModuleFunctionOutputPin(GPIO_PORT_P1, GPIO_PIN7, GPIO_PRIMARY_MODULE_FUNCTION); // SCL
}

void I2CInit() {
    eUSCI_config.selectClockSource = EUSCI_B_I2C_CLOCKSOURCE_SMCLK;
    eUSCI_config.i2cClk = 24000000;
    eUSCI_config.dataRate = EUSCI_B_I2C_SET_DATA_RATE_100KBPS;
    eUSCI_config.byteCounterThreshold = 0;
    I2C_initMaster(EUSCI_B0_BASE, &eUSCI_config);
    I2C_enableModule(EUSCI_B0_BASE);
}

void I2C_writeData(uint32_t moduleInstance
                  ,uint8_t PeriphAddress
                  ,uint8_t StartReg
                  ,uint8_t *data
                  ,uint8_t len)
{
    I2C_setSlaveAddress(moduleInstance,PeriphAddress); // Set the peripheral address
    I2C_setMode(moduleInstance,EUSCI_B_I2C_TRANSMIT_MODE); // Indicate a write operation

    I2C_masterSendMultiByteStart(moduleInstance,StartReg); // Start the communication.
                // This function does three things. It sends the START signal,
                // sends the address, and then sends the start register.

    // This code loops through all of the bytes to send.
    uint8_t ctr;
    for(ctr = 0;ctr<len;ctr++){
        I2C_masterSendMultiByteNext(moduleInstance,data[ctr]);
    }
    // Once all bytes are sent, the I2C transaction is stopped by sending the STOP signal
    I2C_masterSendMultiByteStop(moduleInstance);

    __delay_cycles(200); // A short delay to avoid starting another I2C transaction too quickly
}

void I2C_readData(uint32_t moduleInstance
                 ,uint8_t PeriphAddress
                 ,uint8_t StartReg
                 ,uint8_t *data
                 ,uint8_t len)
{
    // First write the start register to the peripheral device. This can be
    // done by using the I2C_writeData function with a length of 0.
    I2C_writeData(moduleInstance,PeriphAddress,StartReg,0,0);

    Interrupt_disableMaster(); //  Disable all interrupts to prevent timing issues

    // Then do read transaction...
    I2C_setSlaveAddress(moduleInstance,PeriphAddress); // Set the peripheral address
    I2C_setMode(moduleInstance,EUSCI_B_I2C_RECEIVE_MODE); // Indicate a read operation
    I2C_masterReceiveStart(moduleInstance); // Start the communication. This function
                // doe two things: It first sends the START signal and
                // then sends the peripheral address. Once started, the eUSCI
                // will automatically fetch bytes from the peripheral until
                // a STOP signal is requested to be sent.

    // This code loops through 1 less than all bytes to receive
    uint8_t ctr;
    for(ctr = 0;ctr<(len-1);ctr++){
        uint32_t tout_tmp = 10000;
        while(!(EUSCI_B_CMSIS(moduleInstance)->IFG & EUSCI_B_IFG_RXIFG0) && --tout_tmp); // Wait for a data byte to become available
        if(tout_tmp){
            data[ctr] = I2C_masterReceiveMultiByteNext(moduleInstance); // read and store received byte
        }else{
            data[ctr] = 0xFF;
        }
    }
    // Prior to receiving the final byte, request the STOP signal such that the
    // communication will halt after the byte is received.
    data[ctr] = I2C_masterReceiveMultiByteFinish(moduleInstance); // send STOP, read and store received byte

    Interrupt_enableMaster(); // Re-enable interrupts

    __delay_cycles(200); // A short delay to avoid starting another I2C transaction too quickly
}

uint16_t readCompass() {
    uint8_t arry[2];
    uint16_t result;
    I2C_readData(EUSCI_B0_BASE, 0x60, 2, arry, 2);
    result = arry[0] * 256 + arry[1];
    heading = result;
    return result;
}

uint16_t readRanger() {
    uint8_t arry[2];
    uint16_t result;
    I2C_readData(EUSCI_B0_BASE, 0x70, 2, arry, 2);
    result = arry[0] * 256 + arry[1];
    range = result;

    arry[0] = 0x51;
    I2C_writeData(EUSCI_B0_BASE, 0x70, 0, arry, 1);

    return result;
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
