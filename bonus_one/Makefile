CC = g++
CCFLAGS = -Wall
CCVERSION = -std=c++11

all: quicksort

quicksort:	bonus_one.o
	$(CC)  $(CCFLAGS) $(CCVERSION) -o quicksort bonus_one.o -lpthread 

bonus_one.o:	bonus_one.cpp
	$(CC) $(CCFLAGS) $(CCVERSION) -c  bonus_one.cpp  -lpthread

clean:
	- rm -f quicksort
	- rm -f bonus_one.o


# Originally compiled with 	
# g++ bonus_one.cpp -o bonus_one -lpthread -std=c++11	
