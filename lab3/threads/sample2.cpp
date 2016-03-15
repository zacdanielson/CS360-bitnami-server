#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <semaphore.h>

void *functionC(void *ptr);
int  counter = 0;
sem_t sem;

main()
{
   int rc1, rc2;
   pthread_t thread1, thread2;

   sem_init(&sem, PTHREAD_PROCESS_PRIVATE, 1);
   // Now it is set to one, one person will be able to access at a time

   printf("Got semaphore %d\n",sem);

   /* Create independent threads each of which will execute functionC */

   if( (rc1=pthread_create( &thread1, NULL, &functionC, NULL)) )
   {
      printf("Thread creation failed: %d\n", rc1);
   }

   if( (rc2=pthread_create( &thread2, NULL, &functionC, NULL)) )
   {
      printf("Thread creation failed: %d\n", rc2);
   }

   /* Wait till threads are complete before main continues. Unless we  */
   /* wait we run the risk of executing an exit which will terminate   */
   /* the process and all threads before the threads have completed.   */

   pthread_join( thread1, NULL);
   pthread_join( thread2, NULL); 

   exit(0);
}


void *functionC(void *ptr)
{
   int tmp;

   sem_wait(&sem);
   tmp = counter;
   sleep(1);
   tmp++;
   counter = tmp;
   printf("Counter value: %d\n",counter);
   sem_post(&sem);
}
