#include "common.h"

 long THRESHOLD = 0;
 
 
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

void send_end_sequence() {
	send_not_ack();
}

unsigned char xor(unsigned char x) {
	unsigned char a = 0;
	int i = 0;
	for (i = 0; i < 8; i++) {
		a ^= (x >> i) & 1;
	}
	return a;
}

unsigned char recv_bit() {
	long sample = 0;
	while (!sampling_flag) {
		sample++;
	}
	//printf("%ld\n", i);
	sampling_flag = 0;
	printf("Sample = %ld  ", sample);
	if (sample < THRESHOLD) {
		return 1;
	} else {
		return 0;
	}
}

int send_frame(unsigned char data) {
	int i;
	send_1();
	send_1();
	send_0();
	send_1();
	for (i = 0; i < 8; i++) {
		if (data & (1 << i)) send_1();
		else send_0();
	}
	if (xor(data & 0x49)) send_1(); else send_0();		// 0100 1001‬
	if (xor(data & 0x92)) send_1(); else send_0();		// 1001 0010‬
	if (xor(data & 0x24)) send_1(); else send_0();		// 0010 0100‬
	
	set_timer_interval();
	unsigned char a = recv_bit();
	printf("%d\n", a);
	unsigned char b = recv_bit();
	printf("%d\n", b);
	unsigned char c = recv_bit();
	printf("%d\n", c);
	unsigned char d = recv_bit();
	printf("%d\n", d);
	clear_timer();
	
	if (a && b && c == 0 && d) {
		printf("Sent! %d\n", data);
		return 0;
	} else if (a && b == 0 && c && d) {
		printf("Received NOT ack!\n");
		return 1;
	}
	return 0;	// no warnings
}

int recv_ack() {
	unsigned char frame[4];
	set_timer_interval();
	
	frame[0] = recv_bit();
	frame[1] = recv_bit();
	frame[2] = recv_bit();
	frame[3] = recv_bit();
	
	clear_timer();
	
	return frame[0] == 1 && frame[1] == 1 && frame[2] == 0 && frame[3] == 1;
}

unsigned char get_data(unsigned char *frame) {
	int i;
	unsigned char data = 0;
	for (i = 0; i < 8; i++) {
		if (frame[4 + i]) data |= 1 << i;
	}
	return data;
}

int verify_crc(unsigned char* frame) {
	unsigned char a = frame[4] ^ frame[7] ^ frame[10];
	unsigned char b = frame[5] ^ frame[8] ^ frame[11];
	unsigned char c = frame[6] ^ frame[9];
	if (a == frame[12] && b == frame[13] && c == frame[14])
		return 1;
	return 0;
}

int detect_start(unsigned char *bits) {
	if (bits[0] == 1 && bits[1] == 0 && bits[2] == 1 && bits[3] == 1)
		return 1;
	return 0;
}
int detect_end(unsigned char *bits) {
	if (bits[0] == 1 && bits[1] == 1 && bits[2] == 0 && bits[3] == 1)
		return 1;
	return 0;
}

int recv_frame(unsigned char *res) {
	unsigned char bit;
	unsigned char frame[15];
	int receiving = 0;
	int i = 4;
	printf("Listening...\n");
	frame[0] = 0;
	frame[1] = 0;
	frame[2] = 0;
	frame[3] = 0;
	set_timer_interval();
	while (1) {
		bit = recv_bit();
		frame[3] = frame[2];
		frame[2] = frame[1];
		frame[1] = frame[0];
		frame[0] = bit;
		printf("%d\n", bit);
		if (!receiving && detect_end(frame)) {
			return 0;
		}
		if (!receiving && detect_start(frame)) {
			printf("_start\n");
			receiving = 1;
			continue;
		}
		if (receiving) {
			frame[i++] = bit;
		}
		if (i == 15) {
			printf("_stop\n");
			clear_timer();
			if (verify_crc(frame)) {
				send_ack();
				*res = get_data(frame);
				return 1;
			} else {
				printf("CRC error!\n");
				send_not_ack();
				return -1;
			}
		}
	}
}

void set_threshold() {
	FILE *fd = fopen("settings", "r");
	if (fd) {
		fscanf(fd, "%ld", &THRESHOLD);
		fclose(fd);
		return;
	}
	
	long sample = 0;
	long sample0 = 0;
	long sample1 = 0;
	set_timer_interval();
	while (!sampling_flag) {
		sample++;
	}
	sample0 = sample;
	sample = 0;
	sampling_flag = 0;
	
	pid_t pid = fork();
	if (!pid) {	// lunch another process to stimulate the processor
		long a = 1, X = 123000321;
		while (1) {
			a *= X;
		}
	}
	
	while (!sampling_flag) {
		sample++;
	}
	sample1 = sample;
	sampling_flag = 0;
	
	printf("Sample0= %ld\n", sample0);
	printf("Sample1= %ld\n", sample1);
	
	THRESHOLD = (sample1 + sample0) / 2;
	kill(pid, SIGKILL);
	fd = fopen("settings", "w");
	if (fd) {
		fprintf(fd, "%ld", THRESHOLD);
		fclose(fd);
	}
}








