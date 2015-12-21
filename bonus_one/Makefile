all: quicksort

quicksort:	bonus_one.o
	g++ bonus_one.o -o -lpthread -std=c++11 bonus_one

bonus_one.o:	bonus_one.cpp
	g++ -c bonus_one.cpp

clean:
	rm -rf quicksort bonus_one.o


# Originally compiled with 	
# g++ bonus_one.cpp -o bonus_one -lpthread -std=c++11	
