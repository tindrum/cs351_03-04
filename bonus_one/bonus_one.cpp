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

int elementThreadRatio;

pthread_mutex_t vectorMutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t threadPoolMutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t workerQueueMutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t condVar = PTHREAD_COND_INITIALIZER;

void* threadPoolQuickSort(void* args)
{
	// Initializing arguments to run quicksort
	Data data = *((Data*) args);
	int left = data.left, right = data.right;
	int i = left, j = right;
	
	pthread_mutex_lock(&vectorMutex);
	int tmp;
	int pivot = threaded_arr[left + (right - left) / 2];

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

	// Add left partition to queue and signal a thread to process it
	Data leftPart = {left, j};
	if (left < j)
	{
		// Check to see if the array range is large enough to warrant a new thread.
		// Else, do recursive call instead of pushing to work queue
		if ((j - left) >= elementThreadRatio)
		{
			printf("sending {%i-%i}\n", left, j);
			pthread_mutex_lock(&workerQueueMutex);
			workerQueue.push_back(leftPart);
			pthread_mutex_unlock(&workerQueueMutex);
			pthread_cond_signal(&condVar);
		}
		else
		{
			threadPoolQuickSort(&leftPart);
		}
	}

	// Add right partition to queue and signal a thread to process it
	Data rightPart = {i, right};
	if (i < right)
	{
		// Check to see if the array range is large enough to warrant a new thread.
		// Else, do recursive call instead of pushing to work queue
		if ((right - i) >= elementThreadRatio)
		{
			printf("sending {%i-%i}\n", i, right);
			pthread_mutex_lock(&workerQueueMutex);
			workerQueue.push_back(rightPart);
			pthread_mutex_unlock(&workerQueueMutex);
			pthread_cond_signal(&condVar);
		}
		else
		{
			threadPoolQuickSort(&rightPart);
		}
	}

	// This is key; our termination condition
	if (i >= j)
	{
	    if (workerQueue.empty()) 
	    {
	        threadPoolExit = true;
	        pthread_cond_broadcast(&condVar);
	        //printf("all threads must exit now\n");
	    }
	    else
	    {
	        pthread_cond_signal(&condVar);
	        //printf("threads continuing\n");
	    }
	    
	    return NULL;
	}
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
	while (!threadPoolExit)
	{
		pthread_mutex_lock(&threadPoolMutex);
		while (!threadPoolExit && workerQueue.empty())
		{
			// Threads sleep here on <condVar>
			pthread_cond_wait(&condVar, &threadPoolMutex);

			// Do work as soon as a thread is signaled to wake (or if there's a spurious wakeup).
			while (!workerQueue.empty())
			{
				pthread_mutex_lock(&workerQueueMutex);
				work = workerQueue.front();
				workerQueue.pop_front();
				pthread_mutex_unlock(&workerQueueMutex);
				threadPoolQuickSort(&work);
				//printf("thread did work after wake\n");
			}
		}
		while (!workerQueue.empty())
		{
			pthread_mutex_lock(&workerQueueMutex);
			work = workerQueue.front();
			workerQueue.pop_front();
			pthread_mutex_unlock(&workerQueueMutex);
			threadPoolQuickSort(&work);
			//printf("thread did work when queue was not empty\n");
		}
		pthread_mutex_unlock(&threadPoolMutex);	
	}
	if (workerQueue.empty())
	{
		pthread_exit(0);
	}
	else
		printf("maybe this is the problem\n");
}

bool isSorted()
{
	for (int i = 0; i < threaded_arr.size()-1; ++i)
	{
		if (threaded_arr[i] > threaded_arr[i+1])
		{
			printf("%i > %i\n", threaded_arr[i], threaded_arr[i+1]);
			return false;
		}
	}
	return true;
}


int main(int argc, char *argv[]) {
	// Sanity check for args
	if (argc != 3) {
		printf("usage %s <number of elements> <number of threads>\n", argv[0]);
		return -1;
	}

	srand(time(0));

	int numberOfElements = atoi(argv[1]);
	int numberOfThreads = atoi(argv[2]);
	elementThreadRatio = numberOfElements / numberOfThreads;

	// Insert random ints into our array
	for (int i = 0; i < numberOfElements; ++i) {
		int item = rand() % 1000000;
		threaded_arr.push_back(item);
		//serial_arr.push_back(item);
	}

	int len = threaded_arr.size();
	printf("length = %i\n", len);

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


	pthread_t tid[numberOfThreads];

	// Create <numberOfThreads> threads
	for (int i = 0; i < numberOfThreads; ++i) {
		pthread_create(&tid[i], NULL, threadPool, NULL);
		//printf("created thread %i\n", i);
	}

	// Push first indices of the vector bounds
	Data data = {0, threaded_arr.size() - 1};
	workerQueue.push_back(data);

	// Begin timing
	chrono::time_point<chrono::high_resolution_clock> start = chrono::high_resolution_clock::now();
	pthread_cond_signal(&condVar);

	// Join all threads together
	for (int i = 0; i < numberOfThreads; ++i) {
		printf("thread: %i empty: %i\n", i, workerQueue.empty());
		for (auto &&elem : workerQueue) {
			printf("{%i, %i}", elem.left, elem.right);
		}
		printf("\n");
		pthread_join(tid[i], NULL);
	}

	// End timing
	chrono::time_point<chrono::high_resolution_clock> end = chrono::high_resolution_clock::now();
	chrono::duration<double> elapsed_seconds_threaded = end-start;

	len = threaded_arr.size();
	printf("length = %i\n", len);

	printf("\nsorted: %i\n", isSorted());

	printf("time taken %f\n", elapsed_seconds_threaded.count());
	return 0;
}