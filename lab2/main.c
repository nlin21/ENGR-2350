/**********************************************************************/
//** ENGR-2350 F23 Lab 2
//** Names: Nicky Lin, Tyler O'Brien
//** Section: 1
//** Side: B
//** Seat#: 21,22
/**********************************************************************/

#include "engr2350_msp432.h"
#include <math.h>
#include <stdlib.h>

// Add function prototypes here, as needed.
void GPIO_Init();
void Timer_Init();
int8_t readBumpers();
void setRGB(int8_t color);
uint8_t checkGuess(int8_t *sol,int8_t *gss,int8_t *res);
void printResult(int8_t *gss,int8_t *res);
void Timer_ISR();
void blinkSequence(int8_t *seq);
int8_t runGame();

// Add global variables here, as needed.
uint8_t bmp0, bmp1, bmp2, bmp3, bmp4, bmp5, pb1;
Timer_A_UpModeConfig timer_config;
uint8_t flag;
uint32_t timer_resets;
uint32_t total_time;
uint32_t timeout_time;

int8_t guess[4], solution[4], result[4];

int main (void) /** Main Function **/
{  
    SysInit();
    GPIO_Init();
    Timer_Init();

    // Place initialization code (or run-once) code here
    int8_t bumper, status;
    int8_t start_flag = 0;
    int8_t restart_flag = 0;

    printf("Welcome to Colordle!\r\n\n");
    printf("Press pushbutton to start\r\n\n\n");

    while(1)
    {  
        // Place code that runs continuously in here
        pb1 = GPIO_getInputPinValue(GPIO_PORT_P5, GPIO_PIN4);
        if (pb1) {
            start_flag = 1;
            setRGB(6);
            timer_resets = 0;
            while (timer_resets < 200);
            setRGB(-1);
            printf("New Game:\r\n");
            status = runGame();
        }

        if (start_flag == 1) {
            restart_flag = 0;
            if (status == 0) {
                setRGB(0);
                timer_resets = 0;
                while (timer_resets < 50) {
                    pb1 = GPIO_getInputPinValue(GPIO_PORT_P5, GPIO_PIN4);
                    if (pb1) {
                        restart_flag = 1;
                        break;
                    }
                }
                if (restart_flag) continue;
                setRGB(-1);
                timer_resets = 0;
                while (timer_resets < 50) {
                    pb1 = GPIO_getInputPinValue(GPIO_PORT_P5, GPIO_PIN4);
                    if (pb1) {
                        restart_flag = 1;
                        break;
                    }
                }
            } else if (status == 1) {
                setRGB(1);
                timer_resets = 0;
                while (timer_resets < 50) {
                    pb1 = GPIO_getInputPinValue(GPIO_PORT_P5, GPIO_PIN4);
                    if (pb1) {
                        restart_flag = 1;
                        break;
                    }
                }
                if (restart_flag) continue;
                setRGB(-1);
                timer_resets = 0;
                while (timer_resets < 50) {
                    pb1 = GPIO_getInputPinValue(GPIO_PORT_P5, GPIO_PIN4);
                    if (pb1) {
                        restart_flag = 1;
                        break;
                    }
                }
            }
        } else {
            bumper = readBumpers();
            setRGB(bumper);

            while (bumper != -1) {
                bumper = readBumpers();
            }
        }
    }
}   

void GPIO_Init(){
    // Complete for Part B
    GPIO_setAsInputPinWithPullUpResistor(GPIO_PORT_P4, GPIO_PIN0 | GPIO_PIN2 | GPIO_PIN3 | GPIO_PIN5 | GPIO_PIN6 | GPIO_PIN7);  // BMPx
    GPIO_setAsOutputPin(GPIO_PORT_P2, GPIO_PIN0 | GPIO_PIN1 | GPIO_PIN2);   // LED2
    GPIO_setAsInputPin(GPIO_PORT_P5, GPIO_PIN4);    // PB1
}

void Timer_Init(){
    // Complete for Part B. Also add interrupt function
    timer_config.clockSource = TIMER_A_CLOCKSOURCE_SMCLK;
    timer_config.clockSourceDivider = TIMER_A_CLOCKSOURCE_DIVIDER_32;
    timer_config.timerPeriod = 0.01 / (1.0 / (3.0/4.0 * pow(10,6)));

    timer_config.timerInterruptEnable_TAIE = TIMER_A_TAIE_INTERRUPT_ENABLE;
    Timer_A_registerInterrupt(TIMER_A1_BASE, TIMER_A_CCRX_AND_OVERFLOW_INTERRUPT, Timer_ISR);

    Timer_A_configureUpMode(TIMER_A1_BASE, &timer_config);
    Timer_A_startCounter(TIMER_A1_BASE, TIMER_A_UP_MODE);
}

int8_t readBumpers(){
    // Complete for Part B
    bmp0 = !(GPIO_getInputPinValue(GPIO_PORT_P4, GPIO_PIN0));
    bmp1 = !(GPIO_getInputPinValue(GPIO_PORT_P4, GPIO_PIN2));
    bmp2 = !(GPIO_getInputPinValue(GPIO_PORT_P4, GPIO_PIN3));
    bmp3 = !(GPIO_getInputPinValue(GPIO_PORT_P4, GPIO_PIN5));
    bmp4 = !(GPIO_getInputPinValue(GPIO_PORT_P4, GPIO_PIN6));
    bmp5 = !(GPIO_getInputPinValue(GPIO_PORT_P4, GPIO_PIN7));
    if (bmp0) {
        return 0;
    } else if (bmp1) {
        return 1;
    } else if (bmp2) {
        return 2;
    } else if (bmp3) {
        return 3;
    } else if (bmp4) {
        return 4;
    } else if (bmp5) {
        return 5;
    } else {
        return -1;
    }
}

void setRGB(int8_t color){
    // Complete for Part B
    GPIO_setOutputLowOnPin(GPIO_PORT_P2, GPIO_PIN0 | GPIO_PIN1 | GPIO_PIN2);
    if (color == -1) {
        // Off
    } else if (color == 0) {
        GPIO_setOutputHighOnPin(GPIO_PORT_P2, GPIO_PIN0);   // Red
    } else if (color == 1) {
        GPIO_setOutputHighOnPin(GPIO_PORT_P2, GPIO_PIN1);   // Green
    } else if (color == 2) {
        GPIO_setOutputHighOnPin(GPIO_PORT_P2, GPIO_PIN2);   // Blue
    } else if (color == 3) {
        GPIO_setOutputHighOnPin(GPIO_PORT_P2, GPIO_PIN0);   // Yellow
        GPIO_setOutputHighOnPin(GPIO_PORT_P2, GPIO_PIN1);
    } else if (color == 4) {
        GPIO_setOutputHighOnPin(GPIO_PORT_P2, GPIO_PIN0);   // Magenta
        GPIO_setOutputHighOnPin(GPIO_PORT_P2, GPIO_PIN2);
    } else if (color == 5) {
        GPIO_setOutputHighOnPin(GPIO_PORT_P2, GPIO_PIN1);   // Cyan
        GPIO_setOutputHighOnPin(GPIO_PORT_P2, GPIO_PIN2);
    } else if (color == 6) {
        GPIO_setOutputHighOnPin(GPIO_PORT_P2, GPIO_PIN0);   // White
        GPIO_setOutputHighOnPin(GPIO_PORT_P2, GPIO_PIN1);
        GPIO_setOutputHighOnPin(GPIO_PORT_P2, GPIO_PIN2);
    }
}

/**
 * checkGuess is used to check the player's guess against the solution
 * and produce the associated correct positions, incorrect positions, and
 * incorrect colors.
 *
 *  !!! WARNING !!! All of these inputs are expected to be pointers. Arrays are
 *                  technically pointers already! They should not have an & in
 *                  front of them when passed into the function.
 * Input Parameters:
 *      int8_t * sol: A 4-element array that stores the game solution (input)
 *      int8_t * gss: A 4-element array that stores the player's guess (input)
 *      int8_t * res: A 4-element array that stores the guess correctness result
 *                    This array is really an output of this function. It is
 *                    modified within the function, with the changes persistent
 *                    after the function is complete.
 *                    This array will only have the values of:
 *                      0: Red - Incorrect color
 *                      1: Green - Correct color and position
 *                      3: Yellow - Correct color, incorrect position
 * Outputs:
 *      uint8_t - the number of correct positions. This may be used to determine
 *                is the guess was correct.
 */
uint8_t checkGuess(int8_t *sol,int8_t *gss,int8_t *res){
    uint8_t PAT_LEN = 4; // The pattern length
    uint8_t _i,_j; // Loop variables. underscores added to avoid conflict with possible globals.
    uint8_t matched[4]; // Array to store if a color in the answer has been matched yet or not
    for(_i=0;_i<PAT_LEN;_i++){ // set default values of arrays
        res[_i] = 0; // Answer is incorrect (RED)
        matched[_i] = 0; // Guess position is not used yet
    }
    uint8_t Ncorrect = 0; // Number of positions correct.
    // Fist loop through and find corrects
    for(_i=0;_i<PAT_LEN;_i++){
        if(sol[_i] == gss[_i]){ // If the guess and answer match...
            Ncorrect++; // Increment number of correct guesses
            res[_i] = 1; // 1 for green
            matched[_i] = 1; // 1 for used (can't compare this position again)
        }
    }
    // Now check for correct color, incorrect position
    for(_i=0;_i<PAT_LEN;_i++){ // Loop through guess positions
        if(res[_i] == 1) continue; // If this position is marked correct, skip it
        for(_j=0;_j<PAT_LEN;_j++){ // Loop through answer positions, looking for the same color
         // if(i==j) continue; // If checking the same position, skip. This isn't necessary as it would correspond
                               // the correct case and would be skipped by the "checked" array anyway
            if(matched[_j]) continue; // If this answer color is already taken by a correct or close, skip it
            if(gss[_i] == sol[_j]){ // If the colors are the same (correct color, incorrect position)
                res[_i] = 3; // 3 for yellow
                matched[_j] = 1; // 1 for used (can't compare this position again)
            }
        }
    }
    return Ncorrect; // return number of correct positions
}

/*
 * printResult will take the players guess and the checked result and print them
 * in the necessary format on the terminal. The colors in the player's guess will be
 * printed first, using the first letter of each color. Afterwards the result of
 * the guess is printed using the characters:
 *              $ - correct color and position (Green result)
 *              O - correct color, incorrect position (Yellow result)
 *              X - incorrect color (Red result)
 *
 *  !!! WARNING !!! Both of these inputs are expected to be pointers. Arrays are
 *                  technically pointers already! They should not have an & in
 *                  front of them when passed into the function.
 * Input Parameters:
 *      int8_t * gss: A 4-element array that stores the player's guess (input)
 *      int8_t * res: A 4-element array that stores the guess correctness result
 */
void printResult(int8_t *gss,int8_t *res){
    uint8_t PAT_LEN=4;
    uint8_t i = 0; // loop variable
    for(i=0;i<PAT_LEN;i++){
        switch(gss[i]){
        case 0: putchar('R'); break;
        case 1: putchar('G'); break;
        case 2: putchar('B'); break;
        case 3: putchar('Y'); break;
        case 4: putchar('M'); break;
        case 5: putchar('C'); break;
        }
    }
    putchar(' '); // put a space in
    for(i=0;i<PAT_LEN;i++){
        switch(res[i]){
        case 0: putchar('X'); break;
        case 3: putchar('O'); break;
        case 1: putchar('$'); break;
        }
    }
    putchar('\r');putchar('\n');
}

void blinkSequence(int8_t *result) {
    uint8_t i;
    for (i = 0; i < 4; ++i) {
        if (result[i] == 0) {
            setRGB(0);
            timer_resets = 0;
            while (timer_resets < 50);
            setRGB(-1);
            timer_resets = 0;
            while (timer_resets < 50);
            setRGB(-1);
        } else if (result[i] == 1) {
            setRGB(1);
            timer_resets = 0;
            while (timer_resets < 50);
            setRGB(-1);
            timer_resets = 0;
            while (timer_resets < 50);
            setRGB(-1);
        } else if (result[i] == 3) {
            setRGB(3);
            timer_resets = 0;
            while (timer_resets < 50);
            setRGB(-1);
            timer_resets = 0;
            while (timer_resets < 50);
            setRGB(-1);
        }
    }
}

int8_t runGame() {
    int8_t guess_pos;
    int8_t turn;
    int8_t max_turns = 5;
    int8_t num_correct;
    int8_t bumper;
    int8_t i;
    int8_t timeout_flag = 0;
    turn = 1;
    srand(total_time);
    for (i = 0; i < 4; ++i) {
        solution[i] = rand() % 6;
        printf("%u ", solution[i]);
    }
    printf("\r\n");

    total_time = 0;
    while (turn <= max_turns) {
        guess_pos = 0;
        num_correct = 0;
        timer_resets = 0;
        timeout_time = 0;
        while (guess_pos < 4) {
            pb1 = GPIO_getInputPinValue(GPIO_PORT_P5, GPIO_PIN4);
            if (pb1) {
                guess_pos = 0;
                timer_resets = 0;
                setRGB(6);
                while (timer_resets < 50);
                setRGB(-1);
            }
            timeout_flag = 0;
            if (timeout_time >= 3000) {
                printf("Timeout!\r\n");
                printf("---- XXXX\r\n");
                timeout_flag = 1;
                turn += 1;
                break;
            }
            __delay_cycles(480e3);
            bumper = readBumpers();
            __delay_cycles(480e3);

            if (bumper != -1) {
                setRGB(bumper);
                guess[guess_pos] = bumper;
                guess_pos += 1;
                while (bumper != -1) {
                    bumper = readBumpers();
                }
                setRGB(-1);
            }
        }
        if (timeout_flag) continue;
        timer_resets = 0;
        while (timer_resets < 100);
        num_correct = checkGuess(solution, guess, result);
        blinkSequence(result);
        printResult(guess, result);
        if (num_correct == 4) {
            break;
        }
        turn += 1;
    }

    if (turn > max_turns) {
        printf("Failure :(\r\n\n");
        return 0;
    } else {
        printf("Win! Total Time: %0.1f, Total Turns: %u\r\n\n", total_time / 100.0, turn);
        return 1;
    }
}

// Add interrupt functions last so they are easy to find
void Timer_ISR() {
    Timer_A_clearInterruptFlag(TIMER_A1_BASE);
    timer_resets += 1;
    total_time += 1;
    timeout_time += 1;
    flag = 1;
}
