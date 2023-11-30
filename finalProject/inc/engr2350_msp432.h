/*
 * engr2350_msp432.h
 *
 *  Created on: Feb 18, 2020
 *      Author: Kyle Wilt
 */

#ifndef ENGR2350_MSP432_H_
#define ENGR2350_MSP432_H_

#include <stdio.h>
#include <file.h>
#include <ti/devices/msp432p4xx/driverlib/driverlib.h>

void SysInit();

// stdio.h support
int dopen(const char *path, unsigned flags, int llv_fd);
int dclose(int dev_fd);
int dread(int dev_fd, char *buf, unsigned count);
int dwrite(int dev_fd, const char *buf, unsigned count);
long dlseek(int dev_fd, off_t offset, int origin);
int dunlink(const char *path);
int drename(const char *old_name, const char *new_name);

uint8_t getchar_nw();

#endif /* ENGR2350_MSP432_H_ */
