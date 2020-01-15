#include <iostream>
#include <pthread.h>
#include <stdio.h>
#include <unistd.h>
#include <string>
#include <stdlib.h> 
#include <queue> 
#include <semaphore.h>
using namespace std;

#define NUM_THREADS 5
#define MEMORY_SIZE 1000

struct node
{
	int id;
	int size;
};


queue<node> myqueue; // shared que
pthread_mutex_t sharedLock = PTHREAD_MUTEX_INITIALIZER; // mutex
pthread_t server; // server thread handle
sem_t semlist[NUM_THREADS]; // thread semaphores

int thread_message[NUM_THREADS]={0}; // thread memory information
char  memory[MEMORY_SIZE]; // memory size



void release_function()
{
	//This function will be called
	//whenever the memory is no longer needed.
	//It will kill all the threads and deallocate all the data structures.
}

void my_malloc(int thread_id, int size)
{
	pthread_mutex_lock(&sharedLock);
	node partOfMemory;	//This function will add the struct to the queue
	partOfMemory.id = thread_id;
	partOfMemory.size = size;
	myqueue.push(partOfMemory);
	pthread_mutex_unlock(&sharedLock);
}

void * server_function(void *)
{
	//This function should grant or decline a thread depending on memory size.
	int i=0;
	int index=0;
	bool check=true;

	while(check)
	{
		if(!myqueue.empty())
		{
			pthread_mutex_lock(&sharedLock);
			node inQueue = myqueue.front();
			myqueue.pop();

			if(inQueue.size < MEMORY_SIZE-index)
			{
				thread_message[inQueue.id] = index;
				index = index + inQueue.size + 1;
			}
			else
			{
				thread_message[inQueue.id]=-1;
			}
			i++;
			sem_post(&(semlist[inQueue.id]));
			pthread_mutex_unlock(&sharedLock);
			
			if(i==NUM_THREADS)
				check=false;
		}
	}
}

void* thread_function(void *id) 
{
	
	int randomNum = rand() %(300+1);		//This function will create a random size, and call my_malloc
	int* threadID = (int*) id;
	my_malloc(*threadID,randomNum);	
	
	sem_wait(&semlist[*threadID]);		//Block
	pthread_mutex_lock(&sharedLock);
	
	if(thread_message[*threadID]!=-1)	//Then fill the memory with 1's or give an error prompt
	{
		for(int j=thread_message[*threadID]; j <= (randomNum+thread_message[*threadID]) ;j++)
		{
			char fuck = '1';
			memory[j]=fuck;
		}
	}
	else
		cout << "Thread " << *threadID << ": Not enough memory \n";

	pthread_mutex_unlock(&sharedLock);
  		
}

void init()	 
{
	pthread_mutex_lock(&sharedLock);	//lock
	for(int i = 0; i < NUM_THREADS; i++) //initialize semaphores
	{sem_init(&semlist[i],0,0);}
	for (int i = 0; i < MEMORY_SIZE; i++)	//initialize memory 
  	{char zero = '0'; memory[i] = zero;}
   	pthread_create(&server,NULL,server_function,NULL); //start server 
	pthread_mutex_unlock(&sharedLock); //unlock
}



void dump_memory() 
{
	cout << "Memory Dump: " << endl;	// You need to print the whole memory array here.
	for(int i=0;i<MEMORY_SIZE;i++)	
	{
		cout << memory[i];
	}		
}

int main (int argc, char *argv[])
 {
	pthread_t thread[NUM_THREADS];//You need to create a thread ID array here

 	init();	// call init

 	for(int i=0;i<NUM_THREADS;i++)	//You need to create threads with using thread ID array, using pthread_create()
		pthread_create(&thread[i],NULL,thread_function,(void*) &i);
	

 	for(int j=0;j<NUM_THREADS;j++)	//You need to join the threads
		pthread_join(thread[j],NULL);
						
	pthread_join(server,NULL);

 	dump_memory(); // this will print out the memory
 	printf("\nMemory Indexes:\n" );
 	
	for (int i = 0; i < NUM_THREADS; i++)
 		printf("[%d]" ,thread_message[i]); // this will print out the memory indexes
 	
 	printf("\nTerminating...\n");
 }