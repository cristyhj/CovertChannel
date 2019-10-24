#include "common.h"

FILE *stream;

void parse(int argc, char **argv) {
	if (argc <= 1) {
		printf("Usage: %s <filename>", argv[0]);
		exit(0);
	}
	stream = fopen(argv[1], "w");
}



int main(int argc, char **argv) {
	unsigned char data;
	int res;
	parse(argc, argv);
	
	set_timer_handle();
	set_core();
	set_threshold();
	printf("Hello server\n");
	sync();
	
	while (1) {
		res = recv_frame(&data);
		if (res == 0) break;
		if (res == -1) continue;
		
		printf("Recv = %c\n", data);
		fprintf(stream, "%c", data);
	}
	printf("Received all!\n");
	
}