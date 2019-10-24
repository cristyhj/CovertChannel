# CPU Load Covert Channel
Cover channel using CPU Load as data carrier.
The system consists of a client and a server, both running on the same processor.

## How it works. Layer 0
1. The clocks are synchronised. To achieve this, both processes waits untill a new second begins.
2. Uses signals to measure time. A SIGALRM is scheduled once per second.
3. The client do hard computations to send an 1 or waits to send a 0.
4. The server samples every second. The threshold is computed one time and saved in settings file. This may not work at first try, so other settings needs to be created. To do this, remove settings file and run again.

## FCS. Layer 1
1. Every frame begins with 4 bits = 1101. This solution was chosen to avoid noise.
2. Then 8 bits of data is send.
3. Checksum, 3 bits.

4. Receive 4 bits for ack:
     - 1101 ACK
     - 1011 NOT ACK
5. If session is over, instead of start sequence (1101), client sends end sequence 1011 then session is closed

## Build
Run the make command in the main folder.
'''>> make'''
This creates 2 executables, one for server called '''s''' and one for client called '''c'''. The server should be run first.

## Run
1. After the build is done, run the command '''>> ./s <filename>''' where the filename parameter is the name of the file to write the data.
2. When the server is running, run the command '''>> ./c <filename''' where the filename parameter is the name of the file containing ascii characters to be sent.
3. Wait for the program to finish. Please, use 1 or 2 characters because the speed is 1 bit per second and the process may take too long.

## Further implementation
THRESHOLD situation samples just one time for 0 and one time for 1. Sometimes it isn't enough. Due to the deadline I couldn't implement a better algorithm.

The client does not detect when the server is not listening.
