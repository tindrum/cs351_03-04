Assignment 3 for CS 351 Section 3:
-------

Implement a multi-threaded quicksort that uses a thread pool similar to the hash table program.


#### Your name and email address.
Project members:
Christopher Tarazi - christarazi@csu.fullerton.edu
Daniel Henderson   - tindrum@csu.fullerton.edu


#### The programming language you used (i.e., C or C++).
This program is written in C++11.


#### How to execute your program.
Create two terminal windows. Since this is a client-server architecture, 
the server will need to be run first in one terminal, 
then the client in the other.

The server first loads a simple 'database', with each line in the format:
<id=integer> <firstName=string, no spaces> <last=string, no spaces><newline>

To execute the server:
`./server <path_to_database_file> <threads=integer>`
e.g. 
`./server namesDB.txt 4`

The server begins and loads the database file into an internal hash
for quick record lookup.

Now the client can be started:
`./client`
The client does not require parameters.


#### Whether you implemented the extra credit.
This is our implementation for the basic program.
A separate README.md describes our implementation of Bonus 1.

##### General Information



#### Anything special about your submission that we should take note of.

