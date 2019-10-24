#include "common.h"

FILE *stream;

void parse(int argc, char **argv) {
	if (argc <= 1) {
		printf("Usage: %s <filename>", argv[0]);
		exit(0);
	}
	stream = fopen(argv[1], "r");
}



int main(int argc, char **argv) {
	unsigned char data;
	int res;
	parse(argc, argv);
	
	set_timer_handle();
	set_core();
	set_threshold();
	printf("Hello client\n");
	sync();
	
	data = (char)fgetc(stream);
	while (!feof(stream)) {
		printf("Send = %c\n", data);
		res = 1;
		while (res)
			res = send_frame(data);
		
		data = (char)fgetc(stream);
	}
	send_end_sequence();
}