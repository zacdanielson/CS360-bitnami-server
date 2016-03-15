all: server thread
thread: threadPractice.c
	g++ -o threadPractice threadPractice.c -pthread
server: server_lab3.c
	g++ -o server server_lab3.c -pthread
clean:
	rm server threadPractice
