#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <signal.h>
#include <vector>
#include <list>
#include <string>
#include <fstream>
#include <chrono>

using namespace std;

struct Data
{
	int left;
	int right;
};

vector<int> threaded_arr;
vector<int> serial_arr;
int threadCount = 0;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

void* threadedQuickSort(void* args)
{
	Data data = *((Data*) args);
	int left = data.left, right = data.right;
	int i = left, j = right;

	if(left >= right) return NULL;

	int tmp;
	int pivot = threaded_arr[(left + right) / 2];

	while (i <= j) 
	{
		while (threaded_arr[i] < pivot) i++;
		while (threaded_arr[j] > pivot) j--;

		if (i <= j) 
		{
			tmp = threaded_arr[i];
			threaded_arr[i] = threaded_arr[j];
			threaded_arr[j] = tmp;
			i++;
			j--;
		}
	}
	
	pthread_t tid1, tid2;

	Data leftPart = {left, j};
	if (threadCount < 4)
	{
		pthread_mutex_lock(&mutex);
		threadCount++;
		pthread_mutex_unlock(&mutex);
		pthread_create(&tid1, NULL, threadedQuickSort, &leftPart);
		pthread_join(tid1, NULL);
	}
	else 
		threadedQuickSort(&leftPart);

	Data rightPart = {i, right};
	if (threadCount < 4)
	{
		pthread_mutex_lock(&mutex);
		threadCount++;
		pthread_mutex_unlock(&mutex);
		pthread_create(&tid2, NULL, threadedQuickSort, &rightPart);
		pthread_join(tid2, NULL);
	}
	else
		threadedQuickSort(&rightPart);
}

void* serialQuickSort(void* args)
{
	Data data = *((Data*) args);
	int left = data.left, right = data.right;
	int i = left, j = right;

	if(left >= right) return NULL;

	int tmp;
	int pivot = serial_arr[(left + right) / 2];

	while (i <= j) 
	{
		while (serial_arr[i] < pivot) i++;
		while (serial_arr[j] > pivot) j--;

		if (i <= j) 
		{
			tmp = serial_arr[i];
			serial_arr[i] = serial_arr[j];
			serial_arr[j] = tmp;
			i++;
			j--;
		}
	}

	Data leftPart = {left, j};
	serialQuickSort(&leftPart);

	Data rightPart = {i, right};
	serialQuickSort(&rightPart);
}


int main(int argc, char *argv[])
{
	if (argc != 2)
	{
		printf("usage %s <number of elements>\n", argv[0]);
		return -1;
	}

	srand(time(0));

	int num = atoi(argv[1]);

	for (int i = 0; i < num; ++i)
	{
		int item = rand()%100000;
		threaded_arr.push_back(item);
		serial_arr.push_back(item);
	}

	int len = threaded_arr.size();
	printf("length = %i\n", len);
	
	Data data = {0, len-1};
	chrono::time_point<chrono::high_resolution_clock> start = chrono::high_resolution_clock::now();
	threadedQuickSort(&data);
	chrono::time_point<chrono::high_resolution_clock> end = chrono::high_resolution_clock::now();
	chrono::duration<double> elapsed_seconds_threaded = end-start;

	start = chrono::high_resolution_clock::now();
	serialQuickSort(&data);
	end = chrono::high_resolution_clock::now();
	chrono::duration<double> elapsed_seconds_serial = end-start;


	for (int i = 0; i < len; ++i)
	{
		printf("%i ", threaded_arr[i]);
	}
	printf("threaded quick sort time taken: %f\n", elapsed_seconds_threaded.count());
	printf("serial quick sort time taken: %f\n", elapsed_seconds_serial.count());

	return 0;
}