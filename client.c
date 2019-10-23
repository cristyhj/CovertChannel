#include "common.h"



int main() {
	sync();
	printf("Hello client\n");
	set_core();
	set_timer_handle();
	
	while (send_frame(45));
	
}