Assignment 3 Bonus 1 for CS 351 Section 3:
-------

Implement a multi-threaded quicksort that uses a thread pool similar to the hash table program.


#### Your name and email address.
Project members:
Christopher Tarazi - christarazi@csu.fullerton.edu
Daniel Henderson   - tindrum@csu.fullerton.edu


#### The programming language you used (i.e., C or C++).
This program is written in C++11.


#### How to execute your program.
To execute this program:
`./quicksort <integer>`
e.g., to sort one thousand integers, use 
`./quicksort 1000`


#### Whether you implemented the extra credit.
This is our implementation for BONUS 1

##### General Information
Christopher Tarazi implemented the logic for the quicksort algorithm for both the threaded and brute-force functions.
Daniel Henderson wrote `Makefile` and `README.md`. 

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


#### Anything special about your submission that we should take note of.
##### SPEED BENEFIT

In our runs, no vector of integers below one billion elements showed improved performance for the threaded version over the serial version.
This may be because we are not properly taking our threads from a thread pool. 

_TODO: see if thread pool is used, implement if not, update timing paragraph if speed improves (or worsens)_

_TODO: implement as a template class that can work with any class/object that can be ordered, i.e. implements less-than, equal, and greater-than methods for comparison.



