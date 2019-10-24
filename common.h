#ifndef _COMMON_H_
#define _COMMON_H_

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sys/time.h>
#include <signal.h>
#include <sched.h>
#include <math.h>
#include <unistd.h>
#include <pthread.h>

#define SAMPLING_PERIOD 999

//extern long THRESHOLD;


void sync();
void set_core();
void set_timer_handle();
void set_threshold();

void send_frame(unsigned char data);
unsigned char recv_frame();





#endif		// _COMMON_H_