/** This program illustrates the server end of the message queue **/
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

#include "msg.h"
using namespace std;

/* The record in the hash table */
struct record
{
	/* The record id */
	int id;

	/* The first name */	
	string firstName;

	/* The first name */
	string lastName;	
};



/**
 * The structure of a hashtable cell
 */
class hashTableCell
{
	/* Public members */
	public:	

		/**
		 * Initialize the mutex
		 */
		hashTableCell()
		{
			/* TODO: Initialize the mutex (which you will declare at the end of this
			 * class; please see the bottom of the class) using pthread_mutex_init()
			 */
		}

		/**
		 * TODO: Deallocate the mutex declared at the buttom. The function to use 
		 * is pthread_mutex_destroy. E.g., pthread_mutex_destroy(&myMutex) where
		 * myMutex is the name of the mutex.       
		 */
		~hashTableCell()
		{
			/* Deallocate the mutex using pthread_mutex_destroy() */
		}

		/**
		 * Locks the cell mutex
		 */
		void lockCell()
		{
			/* TODO: Add code for locking the cell mutex e.g., 
			 * declared at the bottom of this class
			 * e.g., pthread_mutex_lock(& <mutex name>)          
			 */
		}

		/**
		 * Unlocks the cell mutex
		 */
		void unlockCell()
		{
			/* TODO: Add code for unlocking the cell mutex
			 * declared at the bottom of this class.
			 * e.g., pthread_mutex_unlock(&<mutex name>) 
			 */
		}



		/* The linked list of records */
		list<record> recordList;


		/**
		 * TODO: declare a mutex. E.g., pthread_mututex_t myMutex.
		 * The assignment description contains references to more detailed
		 * documentation.               
		 */

};

/* The number of cells in the hash table */
#define NUMBER_OF_HASH_CELLS 100

/* The number of inserter threads */
#define NUM_INSERTERS 5

/* The hash table */
vector<hashTableCell> hashTable(NUMBER_OF_HASH_CELLS);

/* The number of threads */
int numThreads = 0;

/* The message queue id */
int msqid;

/* The client will send messages to the server to request certain ids (please
 * see the code for the client). When the server receives a request (which is
 * just an integer representing an ID of the requested record), the server will
 * save it on this queue, which represents a queue of requests that the server
 * must service. 
 */
list<int> idsToLookUpList;

/**
 * TODO: Declare and initialize a mutex for protecting the idsToLookUpList.
 *
 *  Rationale:  
 * As you will see below, the idsToLookUpList will be accessed by multiple threads
 * that will be servicing client's requests. Specifically, one thread 
 * (the main thread) will be receiving the ids of the requested records from the 
 * client, adding the received ids to the idsToLookUpList list (by calling idsToLookUpList.push_back(<the received id>)),
 * and multiple threads will be removing the ids from this list in order to look them
 * up and reply to the client. Hence, we need a mutex that will protect idsToLookUpList
 * against race conditions.   
 */

/**
 * TODO: declare and initialize the condition variable, threadPoolCondVar, 
 * for implementing a thread pool.  E.g., pthread_cond_t <name of the condition variable> = 
 * PTHREAD_COND_INITIALIZER. This variable will serve the purpose of the thread pool.
 * 
 * Rationale:
 * We will have a group of threads (sometimes referred to as worker threads),
 * whose job it will be to sleep until woken up by the main thread. When woken up, 
 * the worker thread will remove a record id it from the idsToLookUpList list, look up 
 * the id in the hash table, and send the record back to the server. The main
 * thread will wake up the worker thread when it receives an id of the requested 
 * record.  The purpose of the condition variable is to provide a place for a 
 * worker thread to sleep until it is woken up by the main thread; another words,
 * to serve as basis for implementing the thread pool.          
 */

/* TODO: Declare the mutex, threadPoolMutex, for protecting the condition variable
 * condition variable. 
 */


/**
 * Prototype for createInserterThreads
 */
void createInserterThreads();


/**
 * A prototype for adding new records.
 */
void* addNewRecords(void* arg);

/**
 * This function defines a signal handler for SIGINT i.e., the
 * signal generated when the user pressed Ctrl-c. Here we want to simply
 * set the globally declared flags meant to tell threads that is is it time
 * to exit. That is, when the user presses Ctrl-c, the main thread will 
 * automatically jump to this function, set all flags indicating that is time 
 * to exit to true, and then resume in processIncomingMessages() where it will
 * wait for the threads to exit.       
 * @param sig - the signal
 */
void tellAllThreadsToExit(int sig)
{
	/* TODO: Set the global variable flags described in threadPoolFunc(), 
	 * addNewRecords(), and processIncomingMessages().
	 */   

	signaled = 1;

}

/**
 * Sends the message over the message queue
 * @param msqid - the message queue id
 * @param rec - the record to send
 */
void sendRecord(const int& msqid, const record& rec)
{
	/**
	 * Convert the record to message
	 */

	/* The message to send */
	message msg; 

	/* Copy fields from the record into the message queue */	
	msg.messageType = SERVER_TO_CLIENT_MSG;
	msg.id = rec.id;
	strncpy(msg.firstName, rec.firstName.c_str(), MAX_NAME_LEN);	
	strncpy(msg.lastName, rec.lastName.c_str(), MAX_NAME_LEN);

	/* Send the message */
	sendMessage(msqid, msg);		
}

/**
 * Prints the hash table
 */
void printHashTable()
{
	/* Go through the hash table */
	for(vector<hashTableCell>::const_iterator hashIt = hashTable.begin();
			hashIt != hashTable.end(); ++hashIt)
	{
		/* Go through the list at each hash location */
		for(list<record>::const_iterator lIt = hashIt->recordList.begin();
				lIt != hashIt->recordList.end(); ++lIt)
		{
			fprintf(stderr, "%d-%s-%s-%d\n", lIt->id, 
					lIt->firstName.c_str(), 
					lIt->lastName.c_str(),
					lIt->id % NUMBER_OF_HASH_CELLS
			       );
		}
	}
}

/**
 * Adds a record to hashtable
 * @param rec - the record to add
 */
void addToHashTable(const record& rec)
{
	/**
	 * TODO: grab mutex of the hash table cell
	 */

	/* Hash, and save the record */
	hashTable.at(rec.id % NUMBER_OF_HASH_CELLS).recordList.push_back(rec);

	/**
	 * TODO: release mutex of the hashtable cell
	 */

}


/**
 * Retrieve a record from hashtable  (edited by DgH, I think it was wrong before)
 * @param id the id of the record to retrieve
 * @return - the record from hashtable if exists;
 * otherwise returns a record with id field set to -1
 */
record getHashTableRecord(const int& id)
{
	/* Get pointer to the hash table record */
	hashTableCell* hashTableCellPtr = &hashTable.at(id % NUMBER_OF_HASH_CELLS); 

	/**
	 * The record to return
	 */
	record rec = { -1, "", ""};

	/**
	 * TODO: lock the mutex protecting the hashtable cell (by calling lockCell()). 
	 */

	/* Get the iterator to the list of records hashing to this location */
	list<record>::iterator recIt = hashTableCellPtr->recordList.begin();

	do
	{
		/* Save the record */
		if(recIt->id == id) 
		{
			rec = *recIt;

		}

		/* Advance the record it */
		++recIt;
	}
	/* Go through all the records */
	while((recIt != hashTableCellPtr->recordList.end()) && (rec.id != id));



	/**
	 * TODO: release mutex of the cell. Hint: call unlockCell() to release
	 * mutex protecting the cell.
	 */

	return rec;
}


/**
 * Loads the database into the hashtable
 * @param fileName - the file name
 * @return - the number of records left.
 */
int populateHashTable(const string& fileName)
{	
	/* The record */
	record rec;

	/* Open the file */
	ifstream dbFile(fileName.c_str());

	/* Is the file open */
	if(!dbFile.is_open())
	{
		fprintf(stderr, "Could not open file %s\n", fileName.c_str());
		exit(-1);
	}


	/* Read the entire file */
	while(!dbFile.eof())
	{
		/* Read the id */
		dbFile >> rec.id;

		/* Make sure we did not hit the EOF */
		if(!dbFile.eof())
		{
			/* Read the first name and last name */
			dbFile >> rec.firstName >> rec.lastName;

			/* Add to hash table */
			addToHashTable(rec);	
		}
	}

	/* Close the file */
	dbFile.close();
}

/**
 * Gets ids to process from work list
 * @return - the id of record to look up, or
 * -1 if there is no work
 */
int getIdsToLookUp()
{
	/* The id */
	int id = -1;

	/* TODO: Lock the mutex protecting the idsToLookUpList */

	/* Remove id from the list if exists */
	if(!idsToLookUpList.empty()) 
	{ 
		id = idsToLookUpList.front(); 
		idsToLookUpList.pop_front(); 
	}

	/* TODO: Release idsToLookUpListMutex  */

	return id;
}

/**
 * Add id of record to look up
 * @param id - the id to process
 */
void addIdsToLookUp(const int& id)
{
	/* TODO: Lock the mutex meant to protect idsToLookUpListMutex the list mutex */


	/* Add the element to look up */
	idsToLookUpList.push_back(id);

	/* TODO: Release the idsToLookUpList mutex */
}

/**
 * This is a function in which the worker threads (i.e., the threads
 * responsible for looking up the records requested by the client),
 * spend the entirety of their existance. That is, they sleep on the 
 * condition variable until woken up, and upon awakening start removing
 * the ids of requested records from the idsToLookUp list, looking them up,
 * and replying to the client with the looked up records.      
 * @param thread argument
 */
void* threadPoolFunc(void* arg)
{
	/* Just a variable to store the id of the requested record. */
	int id = -1; 

	/* Continuously loop: sleep until woken up, remove ids from idsToLookUp,
	 * look up the records associated with the ids (in the hashtable), and go back
	 * to sleep.  
	 * 
	 * ADVICE: globally declare a variable which is a flag representing whether it is
	 * time for the thread to exit. Replace the condition for the loop with !flag 
	 * (assuming the name of the variable is flag). When it is time to exit, the 
	 * main thread will set flag to "true", which will cause all the worker threads
	 * to break out of the loop.                          
	 */
	while(true)
	{
		/* TODO: Lock the mutex protecting the condition variable on which threads
		 * sleep. The name of the mutex depends on what you declared it to be above.
		 */

		/* Remove the requested record id from the idsToLookUp list. */
		id = getIdsToLookUp();	

		/* If getIdsToLookUp() has returned a -1, it means there are no records to 
		 * look up. In this case, the thread should sleep on the condition variable.
		 * 
		 * ADVICE: above you were advised to create a flag indicating whether it is
		 * time to exit yet. Here it is good to change this condition to:
		 * while(id==-1 && !flag) where flag is assumed to be the name of the flag
		 * indicating that it is time to exit.                                     
		 */
		while(id == -1)
		{


			/* TODO: Sleep on the condition variable by calling pthread_cond_wait on the
			 * condition variable declared above, the mutex that is protecting it. Recall:
			 * this will two two things: 1) Put the thread to sleep on the condition
			 * variable; and 2) unlock the mutex passed as argument to pthread_cond_wait()
			 * that was locked above. When the thread will wake up, the first thing it
			 * will try to do is lock the mutex (please note: the last part is implicit
			 * and is handled by the pthreads library for you).                                          
			 */

			/* Get the id to look up */
			id = getIdsToLookUp();	

		}


		/* TODO: Release the mutex protecting the condition variable */


		/* Record requests have arrived! Let's look up the record with the requested
		 * id in the hash table. If the record does not exist, then the "id" field
		 * of the rec will be set to -1 (indicating to the client the record does 
		 * not exist).     
		 */
		record rec = getHashTableRecord(id);

		/* Send the looked up record to the client (note: even if the requested record
		 * did not exist, we still send a reply. The "id" field of the record is set 
		 * to -1. When the client reads this field, he will know that the server is
		 * telling him that the specified record does not exist.           
		 */
		sendRecord(msqid, rec);
	}

}

/**
 * Wakes up a thread from the thread pool
 */
void wakeUpThread()
{
	/* TODO: Lock the mutex protecting the condition variable from race conditions.
	 * The name of the mutex and the condition variable depend on what you named
	 * them above. First, you will need to lock the mutex protecting the condition
	 * variable, call pthread_cond_signal(<condition variable name>) to wake up
	 * a thread, and then release the mutex.
	 * PLEASE NOTE: the mutex you manipulate here must be the same mutex that is
	 * passed as parameter to pthread_cond_wait() in the threadPoolFunc(). Recall,
	 * this ensures that the sleeping thread does not miss the wake up call.           
	 * The TODO's below break this down furthetr:                             
	 */

	/* TODO: Lock the mutex proctecting the condition variable against race conditions */

	/* TODO: Call pthread_cond_signal(<condition variable name>) to wake up
	 * a sleeping thread.
	 */      

	/* TODO: Release the mutex protecting the condition variable from race 
	 * conditions.
	 */
}

/**
 * Creates the threads for looking up ids
 * @param numThreads - the number of threads to create
 */
void createThreads(const int& numThreads)
{
	/** TODO: create numThreads threads that call threadPoolFunc() **/

}

/**
 * Creates threads that update the database
 * with randomly generated records
 */
void createInserterThreads()
{
	/* TODO: create NUM_INSERTERS threads that spin in a loop and add new elements
	 * to the hashtable by continually calling addNewRecords(). 
	 * 
	 * ADVICE: it is highly recommended that you globally declare a vector of type 
	 * pthread_t (e.g., vector<pthread_t> inserterTids) and in the vector store the
	 * id of each created thread. It will come in handy when the server is terminating
	 * and needs to call pthread_join() for all threads.          
	 */
}


/**
 * Called by the main thread. In this function, the main thread waits to receive
 * a record request from the client. It then adds the received id request to the
 * idsToLookUp list, and wakes up a worker thread sleeping in the thread pool 
 * (i.e., using pthread_cond_signal()).  
 */
void processIncomingMessages()
{
	/* The arriving message */
	message msg;

	/* The id of the record */
	int id = -1;

	/* Wait for messages forever
	 * ADVICE: replace the loop condition below with a boolean variable, which will
	 * be a flag telling all threads, including the main thread, that it is time
	 * to exit. When the user presses Ctrl-C and the cleanUp() function is called,
	 * it will set the flag to true. After the main thread resumes in the loop below,
	 * it will see that it is time to exit, will wake up all the sleeping threads
	 * and will call pthread_join() to wait for all threads to exit.                       
	 */

	while(true)
	{

		/* Receive the record request from the client. */
		recvMessage(msqid, msg, CLIENT_TO_SERVER_MSG);

		/* TODO: Add id to the list of ids that should be processed
		 * (i.e., idsToLookUpList).
		 */
		addIdsToLookUp(msg.id);

		/* TODO: Wake up a thread to process the newly received id. Recall: we have
		 * a condition variable on which all worker threads sleep. To wake up a worker
		 * thread, we need to signal that condition variable. To achieve all of this,
		 * you need to call wakeUpThread() function, where the appropriate thread wake
		 * up logic is to be implemented.                      
		 */
	}

	/* TODO: If we are out of this loop, it means it is time to exit. Call
	 * pthread_cond_broadcast() in order to wake up all threads sleeping on the
	 * condition variable, followed by a loop which calls pthread_join() for all
	 * threads. Where do I get the thread ids to pass to pthread_join()? Well, 
	 * in createThreads() and createInserterThreads() functions you were advised
	 * to globally declare vectors, and in them store the thread ids of the 
	 * corresponding threads. You can use them here.             
	 */      

	/* TODO: Last but not least: deallocate the message queue */
}

/**
 * Generates a random record
 * @return - a random record
 */
record generateRandomRecord()
{
	/* A record */
	record rec;

	/* Generate a random id */
	rec.id = rand() % NUMBER_OF_HASH_CELLS;	

	/* Add the fake first name and last name */
	rec.firstName = "Random";
	rec.lastName = "Record";

	return rec;
}

/**
 * This is the function executed by the threads that will
 * be inserting new, randomly generate records into the database. 
 * @param arg - some argument (unused)
 */
void* addNewRecords(void* arg)
{	
	/* An instance of the record struct representing
	 * a randomly generated record.
	 */
	record rec;

	/* 
	 * ADVICE: You may also want to declare a boolean variable e.g., 
	 * inserterTimeToExit. The variable will be a flag representing the moment
	 * when threads should exit. The flag can initially be set to false (meaning
	 * it is not time to exit yet) and be used as a condition for the inserter thread
	 * loop e.g., while (! time to exit) { keep adding records}. When it is time to
	 * exit, the main thread will set the variable to true, which will cause all threads
	 * to break out of the loop.                                   
	 * 
	 * ADVICE: you may want to, at least initially, add "usleep(0.5)" function 
	 * before adding a new record which will somewhat slow down the rate at
	 * which the inserter threads are adding records. Previous experience with this
	 * assignment shows that the inserter threads often add records so fast, they
	 * exhaust the process memory.
	 */                              	

	/* Keep generating random records */	
	while(true)
	{
		/* Generate a random record. */
		rec = generateRandomRecord();

		/* Add the record to hashtable */
		addToHashTable(rec);
	}

}

int main(int argc, char** argv)
{
	/**
	 * Check the command line
	 */
	if(argc < 3)
	{
		fprintf(stderr, "USAGE: %s <DATABASE FILE NAME> <NUMBER OF THREADS>\n", argv[0]);
		exit(-1);
	}

	/* TODO: install a signal handler for the SIGINT signal i.e., the signal
	 * generated when the user presses Ctrl-c to call tellAllThreadsToExit()
	 * function.
	 */	
	void (*sigint_handler)(int);
	sigint_handler = signal (SIGINT, tellAllThreadsToExit);

	/* Populate the hash table */
	populateHashTable(argv[1]);


	printHashTable();

	/* Get the number of lookup ID threads */
	numThreads = atoi(argv[2]);

	/* Use a random file and a random character to generate
	 * a unique key. Same parameters to this function will 
	 * always generate the same value. This is how multiple
	 * processes can connect to the same queue.
	 */
	key_t key = ftok("/bin/ls", 'O');

	/* Error checks */
	if(key < 0)
	{
		perror("ftok");
		exit(-1);
	}

	/* Connect to the message queue which will be using to receive record
	 * requests from client and to send replies containing the reuqested records. 
	 */
	msqid = createMessageQueue(key);	

	/* Instantiate a message buffer for sending/receiving msgs 
	 * from msg queue 
	 */
	message msg;

	/* Create the specified number of worker threads. I.e., the threads
	 * that will sleep until the main thread wakes them up (when it recives a record
	 * request)
	 */
	createThreads(numThreads);				

	/* In addition to worker threads, whose job it is to look up the requested 
	 * records, we will also have a group of inserter threads. These threads will
	 * be continuously generating new random records and will be inserting them into
	 * the database.
	 * 
	 * Rationale:
	 * A group of threads inserting records is meant to simulate updates to the
	 * database that involve addition of new records. Many real-world databases
	 * are updated with new data while simultaneously being queried for existing
	 * data. Hence, the setup will give us a real-world glance at the world of 
	 * parallel databases.                             
	 */
	createInserterThreads();

	/* This function will be invoked by the main thread. In it, it will will 
	 * continuously wait to receive a record request from the client (i.e., an id
	 * of the requested record). When it receives the request, it will add the 
	 * received id to the idsToLookUp list, will wake up one of the worker
	 * threads, and will then wait to receive more requests.
	 */
	processIncomingMessages();


	return 0;
}
