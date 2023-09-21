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


// Add global variables here, as needed.
uint32_t array1[] = {1,2,3,4,5,6,7,8,9,10};


int main (void) /* Main Function */
{
    // Add local variables here, as needed.
    uint32_t array2[20] = {0};
    uint32_t m[10][10];
    uint8_t i, j;

    // We always call the "SysInit()" first to set up the microcontroller
    // for how we are going to use it.
    SysInit();

    // Place initialization code (or run-once) code here
    for (i = 0; i < 10; ++i) {
        array2[i] = rand() % 21;
    }
    for (i = 0; i < 10; ++i) {
        printf("%u\r\n", array2[i]);
    }

    for (i = 0; i < 10; ++i) {
        for (j = 0; j < 10; ++j) {
            m[i][j] = (i+1) * (j+1);
        }
    }

    printf("\n\n\t");
    for(i = 0; i < 10; i++) {
        printf("\t%u", i+1);
    }
    printf("\r\n");

    for(i = 0; i < 10; i++) {
        printf("\t%u", i+1);
        for(j=0; j<10; j++) {
            printf("\t%u", m[i][j]);
        }
        printf("\r\n");
    }

    while(1){
        // Place code that runs continuously in here

    }
}

// Add function declarations here as needed

// Add interrupt functions last so they are easy to find
