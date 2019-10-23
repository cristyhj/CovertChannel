all:
	gcc -o server -D_GNU_SOURCE -Wall server.c -lm
	gcc -o client -D_GNU_SOURCE -Wall client.c -lm

server:
	./server
client:
	./client

clean:
	rm server
	rm client
