#include "common.h"


int main() {
	sync();
	printf("Hello client\n");
	set_timer_handle();
	set_core();
	set_threshold();
	
	send_frame(73);
	
}