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

pthread_mutex_t threadPoolMutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t workerQueueMutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t condVar = PTHREAD_COND_INITIALIZER;

void* threadPoolQuickSort(void* args)
{
	// Initializing arguments to run quicksort
	Data data = *((Data*) args);
	int left = data.left, right = data.right;
	int i = left, j = right;

	int tmp;
	int pivot_point = left + ((right - left) / 2);
	int pivot = threaded_arr[pivot_point];

	// printf("*** sorting elements {%i-%i} around pivot at index %i **\n", left, right, pivot_point);
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

	// Add left partition to queue and signal a thread to process it
	Data leftPart = {left, j};
	if (left < j)
	{
		// Check to see if the array range is large enough to warrant a new thread.
		// Else, do recursive call instead of pushing to work queue
		if ((j - left) >= elementThreadRatio)
		{
			// printf("pushing {%i-%i} to workerQueue\n", left, j);

			workerQueue.push_back(leftPart);

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
			// printf("pushing {%i-%i} to workerQueue\n", i, right);

			workerQueue.push_back(rightPart);

			pthread_cond_signal(&condVar);
		}
		else
		{
			threadPoolQuickSort(&rightPart);
		}
	}

	// This is key; our termination condition
	if (i >= (j - 1))
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
		// Threads sleep here on <condVar>
		pthread_mutex_lock(&threadPoolMutex);
		while (!threadPoolExit && workerQueue.empty())
			pthread_cond_wait(&condVar, &threadPoolMutex);

		// Begin working as soon as thread is signaled or queue is not empty
		pthread_mutex_lock(&workerQueueMutex);
		while (!workerQueue.empty())
		{
			work = workerQueue.front();
			workerQueue.pop_front();

			threadPoolQuickSort(&work);
		}
  		pthread_mutex_unlock(&workerQueueMutex);

		pthread_mutex_unlock(&threadPoolMutex);
	}
	if (workerQueue.empty())
		pthread_exit(0);
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


int main(int argc, char *argv[])
{
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
	printf("inserting items into array...\n");
	for (int i = 0; i < numberOfElements; ++i)
	{
		int item = rand() % 1000000;
		threaded_arr.push_back(item);
		serial_arr.push_back(item);
	}

	int len = threaded_arr.size();
	printf("length = %i\n", len);

	pthread_t tid[numberOfThreads];

	// Create <numberOfThreads> threads and assign them to the threadPool
	for (int i = 0; i < numberOfThreads; ++i)
		pthread_create(&tid[i], NULL, threadPool, NULL);

	// Push first indices of the vector bounds
	Data data = {0, threaded_arr.size() - 1};
	printf("\nFirst push to workerQueue:\npushing {%i-%i} to workerQueue\n", data.left, data.right);

	workerQueue.push_back(data);

	// Begin timing
	printf("running threaded quick sort...\n");
	// chrono::time_point<chrono::high_resolution_clock> start = chrono::high_resolution_clock::now();
	pthread_cond_signal(&condVar);

	// Join all threads together
	for (int i = 0; i < numberOfThreads; ++i)
		pthread_join(tid[i], NULL);

	// End timing
	// chrono::time_point<chrono::high_resolution_clock> end = chrono::high_resolution_clock::now();
	// chrono::duration<double> elapsed_seconds_threaded = end-start;
	printf("done\n\n");

	// Timing for serial quick sort
	printf("running serial quick sort...\n");
	// start = chrono::high_resolution_clock::now();
	serialQuickSort(&data);
	// end = chrono::high_resolution_clock::now();
	// chrono::duration<double> elapsed_seconds_serial = end-start;
	printf("done\n\n");

	//for (int p = 0; p < threaded_arr.size(); p++){
	//	printf("index[%i] = %i\n", p, threaded_arr[p]);
	//}

	printf("\nsorted: %i\n", isSorted());

	// printf("threadpool time taken %f\n", elapsed_seconds_threaded.count());
	// printf("serial time taken %f\n", elapsed_seconds_serial.count());



	return 0;
}