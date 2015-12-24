/***
 * This program illustrates the usage of the pipe()
 * function in order to enable parent child communications
 * compilation: g++ pipe.cpp -o pipe
 * running: ./pipe
 */

#include <unistd.h>
#include <stdio.h>
#include <stdio.h>
#include <sys/wait.h>
#include <string.h>
#include <stdlib.h>
#include <linux/limits.h> // for PATH_MAX, longest path allowed

/* The array to contain the file descriptors 
 * the read and write ends of the pipe.
 */
int parent_to_child[2];
int child_to_parent[2];

/* The size of filename send message buffer */
#define FILENAME_BUFFER_SIZE PATH_MAX // 4096 or so
// http://stackoverflow.com/questions/9449241/where-is-path-max-defined-in-linux
	
/* The size of checksum receive message buffer */
#define CHECKSUM_BUFFER_SIZE PATH_MAX // 4096 or so
	// http://stackoverflow.com/questions/9449241/where-is-path-max-defined-in-linux
	
/* The read end of the pipe */
#define READ_END 0

/* The write end of the pipe */
#define WRITE_END 1

/**
 * The function to be called by the child
 */
void childFunc()
{
		
	/* The buffer to store the message received from the parent */
	char recvBuffer[FILENAME_BUFFER_SIZE];
	
	/* The child closes his write end of the pipe --
	 * he does not need it
	 */
	if(close(parent_to_child[WRITE_END]) < 0)
	{
		perror("couldn't close write end of parent_to_child filename pipe");
		exit(-1);	
	}

	if(close(child_to_parent[READ_END]) < 0)
	{
		perror("couldn't close read end of child_tp_parent checksum pipe");
		exit(-1);
	}
	
	/* Try to receive the message from the parent */
	if(read(parent_to_child[READ_END], recvBuffer, FILENAME_BUFFER_SIZE) < 0)
	{
		perror("read");
		exit(-1);
	}
	
	/* Print the message */
	fprintf(stdout, "Got message from the parent: %s\n", recvBuffer);
	fflush(stdout);
	
	/* The child closes his read end of the pipe --
	 * he does not need it anymore
	 */
	if(close(fd[READ_END]) < 0)
	{
		perror("close");
		exit(-1);
	}	
	
	exit(0);
}

/**
 * The function called by the parent process
 */
void parentFunc()
{
	/* The message to send to the child over the pipe */
	char message[] = "Hello child!";
	
	/* The parent closes his read end of the pipe -- 
	 * he does not need it
	 */
	if(close(fd[READ_END]) < 0)
	{
		perror("close");
	}
	
	/* Write the string to the child over the pipe */
	if(write(fd[WRITE_END], message, strlen(message)) < 0)
	{
		perror("write");
		exit(-1);
	}		
	
	/* The parent closes his write end of the pipe -- 
	 * he does not need it anymore. 
	 */
	if(close(fd[WRITE_END]) < 0)
	{
		perror("close");
	}
}


int main()
{

	
	/* Create the pipe */
	if(pipe(fd) < 0)
	{
		perror("pipe");
		exit(-1);
	}
		
	/* Create a child process */
	pid_t pid = fork();
	
	/* Make sure the fork succeeded */
	if(pid < 0)
	{
		perror("fork");
		exit(-1);
	}
	
	/* If this is a child process, then call
	 * the child function
	 */
	if(pid == 0) { childFunc(); }
	
	/* If this is the parent process, then call the parent function */
	else { parentFunc(); }
	
	/* Wait for the child process to terminate */
	if(wait(NULL) < 0)
	{
		perror("wait");
		exit(-1);
	}
	
	
	return 0;
}

