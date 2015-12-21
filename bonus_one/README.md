BONUS 1:
-------

Implement a multi-threaded quicksort that uses a thread pool similar to the hash table program.

Christopher Tarazi implemented the logic for the quicksort algorithm for both the threaded and brute-force functions.
Daniel Henderson wrote `Makefile` and `README.md`. 

Our program requires an integer number of elements as an argument on the command line:

`./quicksort 1000`

The passed-in integer generates that many random integers, and each integer is pushed to two vectors:

* threaded_arr
* serial_arr

These vectors of ints are identical after generation.

Next, a time-hack is gotten from the OS, and the threaded version of quicksort is called.
The first copy of the duplicate vectors is used for the threaded quicksort.

The function prototype is
`void* threadedQuickSort(void* args)`

_TODO: pass number of threads to use as a CL argument, or make it a global_

When threaded quicksort function completes, another time-hack is gotten, and total execution time is computed.

_TODO: get a more detailed time-hack that can also find actual processor time, rather than raw elapsed time._

Next, another time-hack is gotten, and the serial quicksort function is called on the duplicate vector.
The function prototype for the serial method is
`void* serialQuickSort(void* args)`

When it completes, the total time is computed by getting the final time-hack, as was done for the threaded method.

Before exit of `main()`, both total times are printed to the terminal.

#### SPEED BENEFIT

In our runs, no vector of integers below one billion elements showed improved performance for the threaded version over the serial version.
This may be because we are not properly taking our threads from a thread pool. 

_TODO: see if thread pool is used, implement if not, update timing paragraph if speed improves (or worsens)_