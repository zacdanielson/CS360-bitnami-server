#include <stdlib.h>
#include <pthread.h>
#include <queue>
#include <iostream>
#include <stdio.h>
#include <semaphore.h>
sem_t empty, full, mutex;

class myqueue{
	std::queue <int> stlqueue;
	public:
	void push(int sock){
		sem_wait(&empty);
		sem_wait(&mutex);
		stlqueue.push(sock);
		sem_post(&mutex);
		sem_post(&full);
	}
	int pop(){
		sem_wait(&full);
		sem_wait(&mutex);
		int rval = stlqueue.front();
		stlqueue.pop();
		sem_post(&mutex);
		sem_post(&empty);
		return(rval);
	}
} sockqueue;

void *PrintHello(void *threadid){

	for(;;){
		std::cout << "GOT " <<sockqueue.pop()<<std::endl;
	}
}



int main (int argc, char *argv[])
{
#define NUM_THREADS 10
#define NQUEUE 20

	pthread_t threads[NUM_THREADS];
	int rc;
	long t;
	sem_init(&mutex, PTHREAD_PROCESS_PRIVATE,1);
	sem_init(&full, PTHREAD_PROCESS_PRIVATE,0);
	sem_init(&empty, PTHREAD_PROCESS_PRIVATE,NQUEUE);


	for (t =0; t<NUM_THREADS; t++){
		printf("In main: creating thread %ld\n", t);
		rc = pthread_create(&threads[t], NULL, PrintHello, (void *)t);
		if(rc){
			printf("ERROR; return code from pthread_create() is %d\n", rc);
			exit(-1);
		}
	}
	for (int i = 0; i < NQUEUE; i++){
		sockqueue.push(i);
	}
//Set up socket, bind, listen
//for(;;){
//	fd = accept
//	put fd socket in queue
//	}

	pthread_exit(NULL);
}
