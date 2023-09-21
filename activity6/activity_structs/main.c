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

typedef struct _homework_t {
    float average;
    float stdev;
    uint8_t min;
    uint8_t max;
    uint16_t subs;
} homework_t;

// Add function prototypes here, as needed.
float remove_min_from_avg(homework_t *homework);

// Add global variables here, as needed.
homework_t hws[5];

int main (void) /** Main Function **/
{  
    // Add local variables here, as needed.
    homework_t hw1;
    uint8_t i;

    // We always call the "SysInit()" first to set up the microcontroller
    // for how we are going to use it.
    SysInit();

    // Place initialization code (or run-once) code here
    hw1.average = 94.2;
    hw1.stdev = 10.8;
    hw1.min = 55;
    hw1.max = 100;
    hw1.subs = 109;
    printf("Homework 1 Stats\r\n"
           "    Average: %.2f\r\n"
           "  Std. Dev.: %.2f\r\n"
           "    Minimum: %u\r\n"
           "    Maximum: %u\r\n"
           "Submissions: %u\r\n",
           hw1.average,hw1.stdev,hw1.min,
           hw1.max,hw1.subs);

    printf("\n\n\n");

    hws[0] = hw1;

    hws[1].average = 76.7;
    hws[1].stdev = 12.6;
    hws[1].min = 40;
    hws[1].max = 100;
    hws[1].subs = 106;

    hws[2].average = 84.5;
    hws[2].stdev = 15.9;
    hws[2].min = 25;
    hws[2].max = 100;
    hws[2].subs = 101;

    hws[3].average = 92.1;
    hws[3].stdev = 12.6;
    hws[3].min = 45;
    hws[3].max = 100;
    hws[3].subs = 99;

    hws[4].average = 66.3;
    hws[4].stdev = 27.3;
    hws[4].min = 10;
    hws[4].max = 100;
    hws[4].subs = 99;

    for (i = 0; i < 5; ++i) {
        printf("Homework %u Stats\r\n"
               "    Average: %.2f\r\n"
               "  Std. Dev.: %.2f\r\n"
               "    Minimum: %u\r\n"
               "    Maximum: %u\r\n"
               "Submissions: %u\r\n",
               i+1, hws[i].average,hws[i].stdev,hws[i].min,
               hws[i].max,hws[i].subs);
        printf("\n");
    }

    printf("\n\n\n");

    float hw1_newAvg = remove_min_from_avg(&hw1);
    printf("HW1 New Avg: %.2f\r\n", hw1_newAvg);

    float hw2_newAvg = remove_min_from_avg(&(hws[1]));
    printf("HW2 New Avg: %.2f\r\n", hw2_newAvg);

    while(1)
    {  
        // Place code that runs continuously in here

    }   
}   

// Add function declarations here as needed
float remove_min_from_avg(homework_t *homework) {
    float homework_sum = homework->average*homework->subs;
    homework_sum -= homework->min;
    return homework_sum/(homework->subs-1);
}

// Add interrupt functions last so they are easy to find
