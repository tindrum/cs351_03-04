all: server client condvar signal pthread

server: server.cpp msg.o
	g++ server.cpp msg.o -o server -lpthread

client:	client.cpp msg.o
	g++ client.cpp msg.o -o client -lpthread

msg.o:	msg.h msg.cpp
	g++ -c msg.cpp 

signal:	signal.cpp
	g++ signal.cpp -o signal

condvar:	condvar.cpp
	g++ condvar.cpp -o condvar -lpthread

pthread:	pthread.cpp
	g++ pthread.cpp -o pthread -lpthread

clean:
	rm -rf client server condvar signal pthread msg.o
