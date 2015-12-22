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
#include <cstdio>
#include <cerrno>

using namespace std;

// Our data structure to hold the left, right index of the array we're sorting
struct Data
{
	int left;
	int right;
};

// threaded_arr: array for the the threaded version of quicksort
// serial_arr: array for the serial version of quicksort
vector<int> threaded_arr;
vector<int> serial_arr;

// workerQueue holds the "work" (array partitions that need to be sorted) each thread has to do.
list<Data> workerQueue;

bool threadPoolExit = false;

pthread_mutex_t vectorMutex;


// using more robust mutex creation method
// pthread_mutex_t vectorMutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t threadPoolMutex;

// pthread_mutex_t threadPoolMutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t workerQueueMutex;

// pthread_mutex_t workerQueueMutex = PTHREAD_MUTEX_INITIALIZER;

pthread_cond_t condVar;
// pthread_cond_t condVar = PTHREAD_COND_INITIALIZER;

void* threadPoolQuickSort(void* args)
{
	// Initializing arguments to run quicksort
	Data data = *((Data*) args);
	int left = data.left, right = data.right;
	int i = left, j = right;

	pthread_mutex_lock(&vectorMutex);
	int tmp;
	int pivot = threaded_arr[(left + right) / 2];

	// Partitioning the array
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
	pthread_mutex_unlock(&vectorMutex);

	// Recursive call to sorting the left partition
	Data leftPart = {left, j};
	if (left < j)
	{
		pthread_mutex_lock(&workerQueueMutex);
		workerQueue.push_back(leftPart);
		//printf("sent signal\n");
		pthread_cond_signal(&condVar);
		pthread_mutex_unlock(&workerQueueMutex);
		threadPoolQuickSort(&leftPart);
	}

	// Recursive call to sorting the right partition
	Data rightPart = {i, right};
	if (i < right)
	{
		pthread_mutex_lock(&workerQueueMutex);
		workerQueue.push_back(rightPart);
		//printf("sent signal\n");
		pthread_cond_signal(&condVar);
		pthread_mutex_unlock(&workerQueueMutex);
		threadPoolQuickSort(&rightPart);
	}
	// return 0;
}


// void* threadedQuickSort(void* args)
// {
// 	Data data = *((Data*) args);
// 	int left = data.left, right = data.right;
// 	int i = left, j = right;

// 	int tmp;
// 	int pivot = threaded_arr[(left + right) / 2];

// 	while (i <= j)
// 	{
// 		while (threaded_arr[i] < pivot) i++;
// 		while (threaded_arr[j] > pivot) j--;

// 		if (i <= j)
// 		{
// 			tmp = threaded_arr[i];
// 			threaded_arr[i] = threaded_arr[j];
// 			threaded_arr[j] = tmp;
// 			i++;
// 			j--;
// 		}
// 	}

// 	pthread_t tid1, tid2;

// 	Data leftPart = {left, j};
// 	if (threadCount < 4)
// 	{
// 		pthread_mutex_lock(&vectorMutex);
// 		threadCount++;
// 		pthread_mutex_unlock(&vectorMutex);
// 		pthread_create(&tid1, NULL, threadedQuickSort, &leftPart);
// 		pthread_join(tid1, NULL);
// 	}
// 	else
// 		threadedQuickSort(&leftPart);

// 	Data rightPart = {i, right};
// 	if (threadCount < 4)
// 	{
// 		pthread_mutex_lock(&vectorMutex);
// 		threadCount++;
// 		pthread_mutex_unlock(&vectorMutex);
// 		pthread_create(&tid2, NULL, threadedQuickSort, &rightPart);
// 		pthread_join(tid2, NULL);
// 	}
// 	else
// 		threadedQuickSort(&rightPart);
// }

// void* serialQuickSort(void* args)
// {
// 	Data data = *((Data*) args);
// 	int left = data.left, right = data.right;
// 	int i = left, j = right;

// 	if(left >= right) return NULL;

// 	int tmp;
// 	int pivot = serial_arr[(left + right) / 2];

// 	while (i <= j)
// 	{
// 		while (serial_arr[i] < pivot) i++;
// 		while (serial_arr[j] > pivot) j--;

// 		if (i <= j)
// 		{
// 			tmp = serial_arr[i];
// 			serial_arr[i] = serial_arr[j];
// 			serial_arr[j] = tmp;
// 			i++;
// 			j--;
// 		}
// 	}

// 	Data leftPart = {left, j};
// 	serialQuickSort(&leftPart);

// 	Data rightPart = {i, right};
// 	serialQuickSort(&rightPart);
// }

/*
 * Each thread is born into this function. They will sleep the the condition variable <condVar>
 * until the threadPoolQuickSort signals to wake one of the threads. WHen a thread is waken,
 * it will look at the workerQueue for work. If it finds some, it will do it. If not, it will go
 * back to sleep on <condVar>.
 */
void* threadPool(void* args)
{
	Data work;
	while (!workerQueue.empty())
	{
		pthread_mutex_lock(&threadPoolMutex);
		if (!workerQueue.empty())
		{
			pthread_mutex_lock(&workerQueueMutex);
			work = workerQueue.front();
			workerQueue.pop_front();
			pthread_mutex_unlock(&workerQueueMutex);
			threadPoolQuickSort(&work);
		}
		else
		{
			while (!threadPoolExit && workerQueue.empty())
			{
				// Threads sleep here on <condVar>
				pthread_cond_wait(&condVar, &threadPoolMutex);

				// Do work as soon as a thread is signaled to wake (or if there's a spurious wakeup).
				if (!workerQueue.empty())
				{
					pthread_mutex_lock(&workerQueueMutex);
					work = workerQueue.front();
					workerQueue.pop_front();
					pthread_mutex_unlock(&workerQueueMutex);
					threadPoolQuickSort(&work);
				}
			}

		}
		pthread_mutex_unlock(&threadPoolMutex);
	}

	// if (!workerQueue.empty())
	// {
	// 	printf("why?\n");
	// 	pthread_mutex_lock(&workerQueueMutex);
	// 	work = workerQueue.front();
	// 	workerQueue.pop_front();
	// 	pthread_mutex_unlock(&workerQueueMutex);
	// 	threadPoolQuickSort(&work);
	// }
	// else
	// {
	// 	pthread_exit(0);
	// }

	// Exit if we have reached the end of the loop.
	pthread_exit(0);
}


int main(int argc, char *argv[])
{
	// Sanity check for args
	if (argc != 2)
	{
		printf("usage %s <number of elements>\n", argv[0]);
		return -1;
	}

	srand(time(0));

	int num = atoi(argv[1]);

	// Insert random ints into our array
	for (int i = 0; i < num; ++i)
	{
		int item = rand()%1000000;
		threaded_arr.push_back(item);
		//serial_arr.push_back(item);
	}

	int len = threaded_arr.size();
	printf("length = %i\n", len);


	// Initialize mutexes and condition variable
	if (pthread_mutex_init(&vectorMutex, NULL) != 0)
	{
		perror("mutex not created");
		exit(-1);

	}


	if (pthread_mutex_init(&threadPoolMutex, NULL) != 0)
	{
		perror("threadPoolMutex not created");
		exit(-1);

	}


	if (pthread_mutex_init(&workerQueueMutex, NULL) != 0)
	{
		perror("workerQueueMutex not created");
		exit(-1);
	}


	if (pthread_cond_init(&condVar, NULL) != 0)
	{
		perror("condVar not created");
		exit(-1);
	}
	// Timing the runtimes of threaded vs serial quick sort

	// Data data = {0, len-1};
	// chrono::time_point<chrono::high_resolution_clock> start = chrono::high_resolution_clock::now();
	// threadedQuickSort(&data);
	// chrono::time_point<chrono::high_resolution_clock> end = chrono::high_resolution_clock::now();
	// chrono::duration<double> elapsed_seconds_threaded = end-start;

	// start = chrono::high_resolution_clock::now();
	// serialQuickSort(&data);
	// end = chrono::high_resolution_clock::now();
	// chrono::duration<double> elapsed_seconds_serial = end-start;


	// for (int i = 0; i < len; ++i)
	// {
	// 	printf("%i ", threaded_arr[i]);
	// }
	// printf("threaded quick sort time taken: %f\n", elapsed_seconds_threaded.count());
	// printf("serial quick sort time taken: %f\n", elapsed_seconds_serial.count());

	// for (int i = 0; i < threaded_arr.size(); ++i)
	// {
	// 	printf("%i ", threaded_arr[i]);
	// }
	// printf("\n");


	Data data = {0, static_cast<int>(threaded_arr.size()-1)};
	pthread_t tid[4];

	// Create four threads (arbitrary number for now)
	for (int i = 0; i < 4; ++i)
	{
		pthread_create(&tid[i], NULL, threadPool, NULL);
		//printf("created thread %i\n", i);
	}

	// Begin quicksort on main thread to do the partition and adding work to queue
	threadPoolQuickSort(&data);

	pthread_cond_broadcast(&condVar);
	for (int i = 0; i < 4; ++i)
	{
		printf("thread % i\n", i);
		printf("%i\n", workerQueue.empty());
		pthread_join(tid[i], NULL);
	}

	// Check last five elements of array to make sure it's sorted
	for (int i = static_cast<int>(threaded_arr.size())-5; i < static_cast<int>(threaded_arr.size()); ++i)
	{
		printf("%i ", threaded_arr[i]);
	}
	printf("\n");

	len = threaded_arr.size();
	printf("length = %i\n", len);

	return 0;
}