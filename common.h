#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sys/time.h>
#include <signal.h>
#include <sched.h>
#include <math.h>
#include <unistd.h>

#define SAMPLING_PERIOD 999
#define THRESHOLD	420000000	/* aflata empiric strans corelata cu SAMPLING_PERIOD */

int get_ack();

/*
*	System and clock sync functions
*/

volatile sig_atomic_t sampling_flag = 0;

void handle_alarm(int sig) {
	fflush(stdout);
    sampling_flag = 1;
}

void sync() {
	struct timeval  tv1;
	gettimeofday(&tv1, NULL);
	double now = (double)(tv1.tv_usec) / 1000000 + (double)(tv1.tv_sec);
	double next = floor(now) + 1;
	long time = (long)((next - now) * 1000000);
	usleep(time);
}

void set_core() {
	cpu_set_t  mask;
	CPU_ZERO(&mask);
	CPU_SET(1, &mask);
	sched_setaffinity(0, sizeof(mask), &mask);
}

void set_timer_handle() {
	signal(SIGALRM, handle_alarm);
}

void set_timer_interval() {
	struct itimerval it_val;
	it_val.it_value.tv_sec = SAMPLING_PERIOD / 1000;
	it_val.it_value.tv_usec = (SAMPLING_PERIOD * 1000) % 1000000;   
	it_val.it_interval = it_val.it_value;
	setitimer(ITIMER_REAL, &it_val, NULL);
}

void clear_timer() {
	struct itimerval it_val;
	it_val.it_value.tv_sec = 0;
	it_val.it_value.tv_usec = 0;   
	it_val.it_interval = it_val.it_value;
	setitimer(ITIMER_REAL, &it_val, NULL);
}

void set_timer_once() {
	ualarm(SAMPLING_PERIOD * 1000, 0);
}

/*
*	Layer 0: send bits across
*/

void send_0() {
	printf("send 0");
	usleep(SAMPLING_PERIOD * 1000);
	printf(" sent\n");
}

void send_1() {
	printf("send 1");
	long a = 1, X = 123000321;
	ualarm(SAMPLING_PERIOD * 1000, 0);
	while (!sampling_flag) {
		a *= X;
	}
	sampling_flag = 0;
	printf(" sent\n");
}

void send_ack() {
	send_1();
	send_1();
	send_0();
	send_1();
}
void send_not_ack() {
	send_1();
	send_0();
	send_1();
	send_1();
}

/*
*	Layer 1: frames
*/

void put_crc(unsigned char* frame) {
	frame[12] = frame[4] ^ frame[7] ^ frame[10];
	frame[13] = frame[5] ^ frame[8] ^ frame[11];
	frame[14] = frame[6] ^ frame[9];
}
void put_data(unsigned char* frame, unsigned char data) {
	int i;
	for (i = 0; i < 8; i++) {
		if (data & (1 << i)) {
			frame[4 + i] = 1;
		} else {
			frame[4 + i] = 0;
		}
	}
}
void put_frame(unsigned char* frame, unsigned char data) {
	frame[0] = 1;
	frame[1] = 1;
	frame[2] = 0;
	frame[3] = 1;
	
	put_data(frame, data);
	put_crc(frame);
}

int send_frame(unsigned char data) {
	unsigned char frame[15];
	int i;
	put_frame(frame, data);
	for (i = 0; i < 15; i++) {
		if (frame[i]) send_1();
		else send_0();
	}
	if (!get_ack()) {
		printf("error at ack, retransmitting...\n");
		return 1;
	}
	return 0;
}

/*
*	Layer 0: recv bits
*/

int verify_crc(unsigned char* frame) {
	unsigned char a = frame[4] ^ frame[7] ^ frame[10];
	unsigned char b = frame[5] ^ frame[8] ^ frame[11];
	unsigned char c = frame[6] ^ frame[9];
	if (a == frame[12] && b == frame[13] && c == frame[14])
		return 1;
	return 0;
}

unsigned char recv_bit() {
	long sample = 0;
	while (!sampling_flag) {
		sample++;
	}
	//printf("%ld\n", i);
	sampling_flag = 0;
	//printf("%ld  ", sample);
	if (sample < THRESHOLD) {
		return 1;
	} else {
		return 0;
	}
}

unsigned char get_data(unsigned char *frame) {
	int i;
	unsigned char data = 0;
	for (i = 0; i < 8; i++) {
		if (frame[4 + i]) data |= 1 << i;
	}
	return data;
}

int detect_start(unsigned char *bits) {
	if (bits[0] == 1 && bits[1] == 0 && bits[2] == 1 && bits[3] == 1)
		return 1;
	return 0;
}

unsigned char listen_and_recv() {
	unsigned char bit;
	unsigned char frame[15];
	int receiving = 0;
	int i = 4;
	printf("Listening...\n");
	set_timer_interval();
	while (1) {
		bit = recv_bit();
		frame[3] = frame[2];
		frame[2] = frame[1];
		frame[1] = frame[0];
		frame[0] = bit;
		printf("%d", bit);
		if (!receiving && detect_start(frame)) {
			printf("_");
			receiving = 1;
			continue;
		}
		if (receiving) {
			frame[i++] = bit;
		}
		if (i == 15) {
			printf("_");
			clear_timer();
			if (verify_crc(frame)) {
				send_ack();
				return get_data(frame);
			} else {
				printf("CRC error!\n");
				send_not_ack();
				return listen_and_recv();
			}
			return 0;
		}
	}
}

int get_ack() {
	unsigned char frame[4];
	printf("Listening...\n");
	set_timer_interval();
	
	frame[0] = recv_bit();
	frame[1] = recv_bit();
	frame[2] = recv_bit();
	frame[3] = recv_bit();
	
	clear_timer();
	
	return frame[0] == 1 && frame[1] == 1 && frame[2] == 0 && frame[3] == 1;
}















