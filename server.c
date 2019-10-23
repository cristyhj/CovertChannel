#include "common.h"



int main() {
	sync();
	printf("Hello server\n");
	unsigned char data;
	set_core();
	set_timer_handle();
	
	data = listen_and_recv();
	printf("Recv = %d\n", data);
	
}

















