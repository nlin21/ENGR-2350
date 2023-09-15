// **ACTIVITY**: Simple GPIO
// ENGR-2350
// Name: Nicky Lin
// RIN: 662041711

#include "engr2350_msp432.h"

// Add function prototypes here, as needed.
void GPIOInit();
void GetInputs();
void RegisterLogic();
void DriverLibLogic();

// Add global variables here, as needed.
uint8_t pb1;
// **ACTIVITY**: Add pb2 and ss1 variables
uint8_t pb2;
uint8_t ss1;

// Main Function
int main(void)
{
    // We always call the "SysInit()" first to set up the microcontroller
    // for how we are going to use it.
    SysInit();

    // Place initialization code (or run-once) code here
    GPIOInit();

    while(1){
        // Place code that runs continuously in here
        GetInputs();

        // Functions to calculate outputs
        RegisterLogic();
        // DriverLibLogic();


        //// TEST CODE BELOW ////

        // Test print of inputs
        printf("PB1: %u, PB2: %u, SS1: %u\r\n",pb1,pb2,ss1);

        // Code to test the outputs
        /*
        uint32_t count;
        // &= (AND=) sets bits low, |= (OR=) sets bits high,
        // ^= (Exclusive OR=) toggles the value of a bit
        P6OUT ^= 0xXX;  // Replace number to toggle one leg of BiLED1
        P5OUT ^= 0xXX;  // Replace number to toggle LED1
        for(count=100000; count>0; count--); // This Creates a crude delay
        P6OUT ^= 0xXX;  // Replace number to toggle the other leg of BiLED1
        for(count=100000; count>0; count--); // This Creates a crude delay
        */

        //// END OF TEST CODE ////
    }
}

// Add function declarations here as needed
void GPIOInit(){
    // Configure inputs and outputs
    // INPUT                OUTPUT
    // SS1: P3.5            BiLED1: P6.0, P6.1
    // PB1: P5.6            LED1: P5.1
    // PB2: P5.7

    P6DIR |= 0x03; // Set P6.0 and P6.1 to output (BiLED1)
    P3DIR &= ~0x20; // Set P3.5 to input (SS1)
    // **ACTIVITY**: Add initializations for missing inputs and outputs
    P5DIR &= ~0xC0; // 00xxxxxx
    P5DIR |= 0x02;  // 00xxxx1x


    /* DRIVERLIB
    GPIO_setAsOutputPin(GPIO_PORT_P6,GPIO_PIN0|GPIO_PIN1);
    GPIO_setAsInputPin(GPIO_PORT_P3,GPIO_PIN5);

    GPIO_setAsInputPin(GPIO_PORT_P5,GPIO_PIN6|GPIO_PIN7);
    GPIO_setAsOutputPin(GPIO_PORT_P5,GPIO_PIN1);
    */

}

void GetInputs(){
    // Read the input values
    ss1 = ((P3IN & 0x20) != 0);    // Determine ss1 value.
                       // Notice the repeated use of 0x20 from GPIOInit().
    // **ACTIVITY**: Get input state for pb1 and pb2
    pb1 = ((P5IN & 0x40) != 0);   // Get 6th bit
    pb2 = ((P5IN & 0x80) != 0);   // Get 7th bit

    // INPUT                OUTPUT
    // SS1: P3.5            BiLED1: P6.0, P6.1
    // PB1: P5.6            LED1: P5.1
    // PB2: P5.7

    /* DRIVERLIB
    ss1 = GPIO_getInputPinValue(GPIO_PORT_P3,GPIO_PIN5);
    pb1 = GPIO_getInputPinValue(GPIO_PORT_P5,GPIO_PIN6);
    pb2 = GPIO_getInputPinValue(GPIO_PORT_P5,GPIO_PIN7);
    */
}

void RegisterLogic(){
    // **ACTIVITY**: Finish the code in this function (look at comments)
    if( ss1 ){    // Check if slide switch is ON
        if( pb1 && pb2 ){ // **ACTIVITY**: Check if Both pushbuttons are pressed (replace 0)
            // Turn BiLED1 OFF by setting both pins to the same value
            P6OUT |= 0x03;  // This sets both 6.0&6.1 to 1 (xxxxxx11)
            // **ACTIVITY**: Turn LED1 ON (add command below)

            P5OUT |= 0x02;  // xxxxxx1x

        }else if( pb1 ){ // **ACTIVITY**: Check if pushbutton 1 is pressed (replace 0)
            // Turn BiLED1 to 1 color by setting both pins to the opposite value
           P6OUT |= 0x02;  // Set P6.1 to 1 (xxxxxx1x)
           P6OUT &= ~0x01; // Then set P6.0 to 0 (xxxxxx10)
           // **ACTIVITY**: Turn LED1 OFF (add command below)

           P5OUT &= ~0x02;  // xxxxxx0x

        }else if( pb2 ){ // Check if pushbutton 2 is pressed
           // **ACTIVITY**: Turn BiLED1 to the other color by... and also turn LED1 ON

           P6OUT |= 0x01;   // xxxxxxx1
           P6OUT &= ~0x02;  // xxxxxx01

           P5OUT |= 0x02;   // xxxxxx1x

        }else{
           // **ACTIVITY**: Turn everything off

           P6OUT &= ~0x03; // This sets both 6.0&6.1 to 0 (xxxxxx00)
           P5OUT &= ~0x02; // set pin 5.1 to 0 (xxxxxx0x)

        }
    }else{
        // Turn everything off
        P6OUT &= ~0x03; // This sets both 6.0&6.1 to 0 (xxxxxx00)
        P5OUT &= ~0x02; // set pin 5.1 to 0 (xxxxxx0x)
    }
}

void DriverLibLogic(){
    // **ACTIVITY**: Finish the code in this function (look at comments)
    if( ss1 ){    // Check if slide switch is ON
        if( pb1 && pb2 ){ // **ACTIVITY**: Check if Both pushbuttons are pressed (replace 0)
            // Turn BiLED1 OFF by setting both pins to the same value
            GPIO_setOutputLowOnPin(GPIO_PORT_P6,GPIO_PIN0|GPIO_PIN1);
            // **ACTIVITY**: Turn LED1 ON (add command below)

            GPIO_setOutputHighOnPin(GPIO_PORT_P5,GPIO_PIN1);

        }else if( pb1 ){ // **ACTIVITY**: Check if pushbutton 1 is pressed (replace 0)
            // Turn BiLED1 to 1 color by setting both pins to the opposite value
            GPIO_setOutputHighOnPin(GPIO_PORT_P6,GPIO_PIN1);
            GPIO_setOutputLowOnPin(GPIO_PORT_P6,GPIO_PIN0);
            // **ACTIVITY**: Turn LED1 OFF (add command below)

            GPIO_setOutputLowOnPin(GPIO_PORT_P5,GPIO_PIN1);

        }else if( pb2 ){ // Check if pushbutton 2 is pressed
            // **ACTIVITY**: Turn BiLED1 to the other color by... and also turn LED1 ON

            GPIO_setOutputHighOnPin(GPIO_PORT_P6,GPIO_PIN0);
            GPIO_setOutputLowOnPin(GPIO_PORT_P6,GPIO_PIN1);

            GPIO_setOutputHighOnPin(GPIO_PORT_P5,GPIO_PIN1);

        }else{
            // **ACTIVITY**: Turn everything off
            GPIO_setOutputLowOnPin(GPIO_PORT_P6,GPIO_PIN0);
            GPIO_setOutputLowOnPin(GPIO_PORT_P6,GPIO_PIN1);
            GPIO_setOutputLowOnPin(GPIO_PORT_P5,GPIO_PIN1);
        }
    }else{
        // Turn everything off
        GPIO_setOutputLowOnPin(GPIO_PORT_P6,GPIO_PIN0);
        GPIO_setOutputLowOnPin(GPIO_PORT_P6,GPIO_PIN1);
        GPIO_setOutputLowOnPin(GPIO_PORT_P5,GPIO_PIN1);
    }
}

// Add interrupt functions last so they are easy to find
