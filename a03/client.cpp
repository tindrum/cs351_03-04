
/** This program illustrates the client end of the message queue **/
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <string.h>
#include <vector>
#include <list>
#include <string>

/* Defines message queue related functions */
#include "msg.h"

using namespace std;

int main() 
{

	/* Generate a random key based on the file keyfile.txt (must exist)*/
	key_t key = ftok("keyfile", 'O');

	/* Was the key allocation successful ? */
	if(key < 0) { perror("ftok"); exit(-1); }	

	/* Connect to the message queue; fail if the	there is no message
	 * queue associated with this key. This function returns the id of the queue.
	 * It basically calls msgget() behind the scenes (see msg.h)    
	 */	
	int msqid = connectToMessageQueue(key);	

	/* The message */
	message msg; 

	while(true) 
	{

		/* Generate a random id. The id field of the message struct
		 * represents the id of the record requested from the server.
		 */
		msg.id = rand() % 1000;

		/* Set the type of the message to represent a message sent from the client
		 * to the server.
		 */
		msg.messageType = CLIENT_TO_SERVER_MSG;	

		/* Send the message to the server! */
		sendMessage(msqid, msg);

		/* Receive a reply from the server. The requested record will be stored in the
		 * msg struct as well. If the id field of the record is -1, it means the server
		 * did not have the record we requested. Otherwise, the it contains the record
		 * with which the server has responded.           
		 */	
		recvMessage(msqid, msg, SERVER_TO_CLIENT_MSG);

		/* Print only records that actually exist */
		if(msg.id != -1) 
		{	
			/* Print the message */
			msg.print(stderr); 
		}

	}

	return 0; 
}
