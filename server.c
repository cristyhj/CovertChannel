#include "common.h"



int main() {
	sync();
	printf("Hello server\n");
	unsigned char data;
	set_timer_handle();
	set_core();
	set_threshold();
	
	data = recv_frame();
	printf("Recv = %d\n", data);
	
}