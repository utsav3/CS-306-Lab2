#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#define BUFF_SZ 35

int ITERS = 6;              // num of reads & writes
int SLOWNESS = 30000;       // program speed factor 
int NUM_R = 7;              // reader threads
int NUM_W = 3;              // writer threads
char  BUFFER[BUFF_SZ + 1];  // shared memory


/* Critical Section (CS) access variables for the reader-writer problem */
pthread_mutex_t r_lock = PTHREAD_MUTEX_INITIALIZER;			/* read lock ,shared only between readers */
pthread_mutex_t rw_lock = PTHREAD_MUTEX_INITIALIZER;		/* read-write lock ,shared between readers and writers */
int read_count = 0;											/* keeps track of number of readers inside the CS */


// sleep function
void rest() {
  usleep(SLOWNESS * (rand() % (NUM_R + NUM_W)));
}

void* reader(void *arg) {
  // set up private buffer
  char buff[BUFF_SZ + 1];
  buff[BUFF_SZ] = '\0';
  
  // thread-local random seed
  srand( (unsigned int) (intptr_t) &arg);

  int i, j;
  for (i = 0; i < ITERS; i++) {
    rest();
	pthread_mutex_lock(&r_lock);
	read_count++;
	if (read_count == 1) pthread_mutex_lock(&rw_lock);		/* prevent any writer's from entering the CS */
	pthread_mutex_unlock(&r_lock);							/* free to read now - any potential writer has been locked out */
    for (j = 0; j < BUFF_SZ; j++) {
      buff[j] = BUFFER[j];
      usleep(SLOWNESS / BUFF_SZ);
    }
    printf("[R%ld] got: %s\n", (intptr_t) arg, buff);
	/* done reading, now bookkeeping */
	pthread_mutex_lock(&r_lock);
	read_count--;
	if (read_count == 0) pthread_mutex_unlock(&rw_lock);		/* allow writers to enter - last reader has left the CS */
	pthread_mutex_unlock(&r_lock);
  }

  return 0;
}

void *writer(void *arg) {
  // thread-local random seed
  srand( (unsigned int)(intptr_t) &arg);
  
  int i, j;
  for (i = 0; i < ITERS; i++) {
    rest();
    pthread_mutex_lock(&rw_lock);			/* a writer must hold the lock in a mutually exclusive manner */
	for (j = 0; j < BUFF_SZ; j++) {
      BUFFER[j] = 'a' + i;
      // yield the thread
      usleep(1);
    }
    printf("[W%ld] put: %s\n", (intptr_t) arg, BUFFER); 
    pthread_mutex_unlock(&rw_lock);			/* allow other writers and readers to access the CS */ 
  }
  return 0;
 }

int main(int argc, char **argv) {
  void *result;
  pthread_t readers[NUM_R], writers[NUM_W];
 

  // initialize & stringify buffer
  memset(BUFFER, 'X', sizeof BUFFER);
  BUFFER[BUFF_SZ] = '\0';
  
  // create readers
  int i = 0;
  while (i < NUM_R) {
    if (pthread_create(readers + i, NULL, reader, (void *) (intptr_t) i) != 0) {
      perror("pthread create");
      exit(-1);
    }
    i++;
  }
  
  // create writers
  i = 0;
  while (i < NUM_W) {
    if (pthread_create(writers + i, NULL, writer, (void *) (intptr_t) i) != 0) {
      perror("pthread create");
      exit(-1);
    }
    i++;
  }

  // join all
  i = 0;
  while (i < NUM_W) {
    pthread_join(writers[i], &result);
    printf("Joined %d with status: %ld\n", i, (intptr_t) result);
    i++;
  }
  i = 0;
  while (i < NUM_R) {
    pthread_join(readers[i], &result);
    printf("Joined %d with status: %ld\n", i, (intptr_t) result);
    i++;
  }
  
  return 0;
}
