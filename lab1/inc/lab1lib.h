/*
 * lab1lib.h
 *
 *  Created on: Aug 17, 2023
 *      Author: kyle
 */

#ifndef INC_LAB1LIB_H_
#define INC_LAB1LIB_H_


/**
 * Initialize the Segment driver
 */
void init_Sequence();
/**
 * @brief Adds a segment to the programmed list.
 *
 * @param mv A number representing the segment to add:
 *        -2 - Drive and turn -90 degrees
 *        -1 - Drive and turn -45 degrees
 *         0 - Drive forward
 *         1 - Drive and turn 45 degrees
 *         2 - Drive and turn 90 degrees
 *       127 - Stop for a fixed time
 *
 * @output 0 if recorded properly, 1 if buffer is full; segment omitted, -1 if invalid segment #
 */
int8_t record_Segment(int8_t mv);

/**
 * @brief Erase most recently added segment
 *
 * @output 0 if erased properly, 1 if no segments to erase
 */
uint8_t pop_Segment(void);

/**
 * @brief Erase all stored segments
 */
void clear_Sequence(void);

/**
 * @brief Runs the programmed segments
 *
 * @output 0 if started properly, 1 if no segments to run
 */
uint8_t run_Sequence(void);

/**
 * @brief Gets the current segment being run
 *
 * @output returns the segment #. If not running, returns 100
 */
int8_t status_Segment(void);

/**
 * @brief Returns what # out of the total # of segments the current
 * one is (e.g., [4] out of 15).
 *
 * @output returns the segment sequence #. If not running, returns 100
 */
uint8_t status_Sequence(void);

/**
 * @brief Gets the filled length of the segment sequence
 *
 * @output returns the sequence length stored
 */
uint8_t get_SequenceLength(void);

#endif /* INC_LAB1LIB_H_ */
