all: skel

skel:	skel.o
	g++ skel.o -o skel

skel.o:	skel.cpp
	g++ -c skel.cpp

clean:
	rm -rf skel skel.o
