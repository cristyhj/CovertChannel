all:
	gcc -o s -D_GNU_SOURCE -Wall server.c common.c -lm -lpthread
	gcc -o c -D_GNU_SOURCE -Wall client.c common.c -lm -lpthread

start_server:
	./s
start_client:
	./c

remote:
	sshpass -p 'andrei' scp ./* 192.168.126.136:/home/andrei/tema/
	sshpass -p 'andrei' scp ./* 192.168.126.137:/home/andrei/tema/

clean:
	rm s
	rm c
	rm settings
