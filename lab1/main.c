// Lab 1
// ENGR-2350
// Name: Nicky Lin
// RIN: 662041711

#include "engr2350_msp432.h"
#include "lab1lib.h"

void GPIOInit();
void TestIO();
void ControlSystem();

uint8_t LEDFL = 0; // Two variables to store the state of
uint8_t LEDFR = 0; // the front left/right LEDs (on-car)

uint8_t ss1, pb1;
uint8_t bmp0, bmp1, bmp2, bmp3, bmp4, bmp5;

int main(void) {

    SysInit(); // Basic car initialization
    init_Sequence(); // Initializes the Lab1Lib Driver
    GPIOInit();

    printf("\r\n\n"
           "***********\r\n"
           "Lab 1 Start\r\n"
           "***********\r\n");

    while(1){
        // TestIO(); // Used in Part A to test the IO
        ControlSystem(); // Used in Part B to implement the desired functionality
    }
}

void GPIOInit(){
    // Add initializations of inputs and outputs
    GPIO_setAsInputPin(GPIO_PORT_P5, GPIO_PIN6); // PB1
    GPIO_setAsInputPin(GPIO_PORT_P3, GPIO_PIN5); // SS1
    GPIO_setAsOutputPin(GPIO_PORT_P6, GPIO_PIN0 | GPIO_PIN1); // BiLED1

    GPIO_setAsInputPinWithPullUpResistor(GPIO_PORT_P4, GPIO_PIN0 | GPIO_PIN2 | GPIO_PIN3 | GPIO_PIN5 | GPIO_PIN6 | GPIO_PIN7);

    GPIO_setAsOutputPin(GPIO_PORT_P8, GPIO_PIN0 | GPIO_PIN5); // LEDFL, LEDFR
    GPIO_setAsOutputPin(GPIO_PORT_P3, GPIO_PIN7 | GPIO_PIN6); // Left On/Off, Right On/Off
    GPIO_setAsOutputPin(GPIO_PORT_P5, GPIO_PIN4 | GPIO_PIN5); // Left Direction, Right Direction
}

void TestIO(){
    // Add printf statement(s) for testing inputs

    // Example code for testing outputs
    while(1) {
        uint8_t cmd = getchar();
        if (cmd == 'a') {
            // Turn LEDFL On
            GPIO_setOutputHighOnPin(GPIO_PORT_P8, GPIO_PIN0);
        } else if (cmd == 'z') {
            // Turn LEDFL Off
            GPIO_setOutputLowOnPin(GPIO_PORT_P8, GPIO_PIN0);
        } else if (cmd == 's') {
            // Turn LEDFR On
            GPIO_setOutputHighOnPin(GPIO_PORT_P8, GPIO_PIN5);
        } else if (cmd == 'x') {
            // Turn LEDFR Off
            GPIO_setOutputLowOnPin(GPIO_PORT_P8, GPIO_PIN5);
        } else if (cmd == 'q') {
            // Turn BiLED1 Red
            GPIO_setOutputHighOnPin(GPIO_PORT_P6, GPIO_PIN0);
            GPIO_setOutputLowOnPin(GPIO_PORT_P6, GPIO_PIN1);
        } else if (cmd == 'w') {
            // Turn BiLED1 Off {
            GPIO_setOutputHighOnPin(GPIO_PORT_P6, GPIO_PIN0);
            GPIO_setOutputHighOnPin(GPIO_PORT_P6, GPIO_PIN1);
        } else if (cmd == 'e') {
            // Turn BiLED1 Green
            GPIO_setOutputHighOnPin(GPIO_PORT_P6, GPIO_PIN1);
            GPIO_setOutputLowOnPin(GPIO_PORT_P6, GPIO_PIN0);
        } else if (cmd == 'd') {
            // Test Left Motor
            // Test_Motor(0);
        } else if (cmd == 'c') {
            // Test Right Motor
            // Test_Motor(1);
        }
    }
}

void ControlSystem() {
    GPIO_setOutputHighOnPin(GPIO_PORT_P8, GPIO_PIN0);
    GPIO_setOutputLowOnPin(GPIO_PORT_P8, GPIO_PIN5);

    // Infinite Loop
    while(1) {
        // Check SS1 state
        ss1 = GPIO_getInputPinValue(GPIO_PORT_P3, GPIO_PIN5);

        if (ss1 == 1) { // SS1 is on
            // Has Pattern Started?
            if (status_Sequence() == 100) {
                run_Sequence();

                GPIO_setOutputLowOnPin(GPIO_PORT_P6, GPIO_PIN0);   // Turn BiLED1 Green
                GPIO_setOutputHighOnPin(GPIO_PORT_P6, GPIO_PIN1);

                while (status_Sequence() != 100) {}

                GPIO_setOutputLowOnPin(GPIO_PORT_P6, GPIO_PIN1);
                GPIO_setOutputHighOnPin(GPIO_PORT_P6, GPIO_PIN0);    // Turn BiLED1 Red

                clear_Sequence();
            }
        } else { // SS2 is Off
            // Turn BiLED OFF
            GPIO_setOutputHighOnPin(GPIO_PORT_P6, GPIO_PIN0);
            GPIO_setOutputHighOnPin(GPIO_PORT_P6, GPIO_PIN1);

            // Any BMPx Pressed?
            bmp0 = !(GPIO_getInputPinValue(GPIO_PORT_P4, GPIO_PIN0));
            bmp1 = !(GPIO_getInputPinValue(GPIO_PORT_P4, GPIO_PIN2));
            bmp2 = !(GPIO_getInputPinValue(GPIO_PORT_P4, GPIO_PIN3));
            bmp3 = !(GPIO_getInputPinValue(GPIO_PORT_P4, GPIO_PIN5));
            bmp4 = !(GPIO_getInputPinValue(GPIO_PORT_P4, GPIO_PIN6));
            bmp5 = !(GPIO_getInputPinValue(GPIO_PORT_P4, GPIO_PIN7));

            if (bmp0 || bmp1 || bmp2 || bmp3 || bmp4 || bmp5) {
                __delay_cycles(480e3);
                if (bmp0) {
                    __delay_cycles(480e3);
                    printf("bmp0: %u\r\n", bmp0);
                    record_Segment(-2);
                    __delay_cycles(480e3);
                } else if (bmp1) {
                    __delay_cycles(480e3);
                    printf("bmp1: %u\r\n", bmp1);
                    record_Segment(-1);
                    __delay_cycles(480e3);
                } else if (bmp2) {
                    __delay_cycles(480e3);
                    printf("bmp2: %u\r\n", bmp2);
                    record_Segment(0);
                    __delay_cycles(480e3);
                } else if (bmp3) {
                    __delay_cycles(480e3);
                    printf("bmp3: %u\r\n", bmp3);
                    record_Segment(127);
                    __delay_cycles(480e3);
                } else if (bmp4) {
                    __delay_cycles(480e3);
                    printf("bmp4: %u\r\n", bmp4);
                    record_Segment(1);
                    __delay_cycles(480e3);
                } else if (bmp5) {
                    __delay_cycles(480e3);
                    printf("bmp5: %u\r\n", bmp5);
                    record_Segment(2);
                    __delay_cycles(480e3);
                }
                GPIO_toggleOutputOnPin(GPIO_PORT_P8, GPIO_PIN0 | GPIO_PIN5);
                __delay_cycles(480e3);
            } else { // No
                // PB1 Pressed?
                pb1 = GPIO_getInputPinValue(GPIO_PORT_P5, GPIO_PIN6);
                if (pb1) { // Yes
                    __delay_cycles(480e3);
                    pop_Segment();  // Remove Last Segment
                    __delay_cycles(480e3);
                }
            }
        }
    }
}
