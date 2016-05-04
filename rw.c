/*
 * Written By: Utsav Dhungel
 * CS 306
 * Lab 2
 */

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include "rw.h"

#define SLOWNESS 30000
#define INVALID_ACCNO -99999

// sleep function
void rest() {
    //usleep(SLOWNESS * (rand() % (READ_THREADS + WRITE_THREADS)));
    usleep(100);
}


/* Global shared data structure */
account account_list[SIZE];				/* this is the data structure that the readers and writers will be accessing concurrently.*/


/* Define your global CS access variables for the Reader-writer problem.
   YOUR CODE GOES HERE.
*/
pthread_mutex_t r_lock=PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t rw_lock=PTHREAD_MUTEX_INITIALIZER;
int read_count=0;




/* Writer thread - will update the account_list data structure. 
   Takes as argument the seed for the srand() function.	
*/
void * writer_thr(void * arg) {
	printf("Writer thread ID %ld\n", pthread_self());	
	srand(*((unsigned int *) arg));		/* set random number seed for this writer */
	
	int i, j;
	int r_idx;
	unsigned char found;            /* For every update_acc[j], set to TRUE if found in account_list, else set to FALSE */
	account update_acc[WRITE_ITR];

	/* first create a random data set of account updates */
	for (i = 0; i < WRITE_ITR;i++) {
		r_idx = rand() % SIZE;		/* a random number in the range [0, SIZE) */ 
		update_acc[i].accno = account_list[r_idx].accno;
		update_acc[i].balance = 1000.0 + (float) (rand() % MAX_BALANCE);
	}
	/* open a writer thread log file */
	char thr_fname[64];
	snprintf(thr_fname, 64, "writer_%ld_thr.log", pthread_self());
	FILE* fd = fopen(thr_fname, "w");
	if (!fd) {
		fprintf(stderr,"Failed to open writer log file %s\n", thr_fname);
		pthread_exit(&errno);
	}

	/* The writer thread will now to update the shared account_list data structure. 
	   For each entry 'j' in the update_acc[] array, it will find the corresponding 
       account number in the account_list array and update the balance of that account
	   number with the value stored in update_acc[j]. 	
	*/

	int temp_accno;
    /* The writer thread will now to update the shared account_list data structure */
    for (j = 0; j < WRITE_ITR;j++) {
        found = FALSE;
        /* Now update */
        rest();     /* makes the write long duration - PLACE THIS IN THE CORRECT PLACE SO AS TO INTRODUCE LATENCY IN WRITE before going for next 'j' */
        for (i = 0; i < SIZE;i++) {
            if (account_list[i].accno == update_acc[j].accno) {
                /* lock and update location */
                /* You MUST FIRST TEMPORARILY INVALIDATE the accno by setting account_list[i] = INVALID_ACCNO; before making any updates to the account_list[i].balance.
				   Once the account balance is updated, you MUST put the rest() call in the appropriate place before going for update_acc[j+1]. 
				   This is to enable detecting race condition with reader threads violating CS entry criteria.
 				   For every successful update to the account_list, you must write a log entry using the following format string:
						 fprintf(fd, "Account number = %d [%d]: old balance = %6.2f, new balance = %6.2f\n",
                        				account_list[i].accno, update_acc[j].accno, account_list[i].balance, update_acc[j].balance);
				   
				   Additionally, your code must also introduce checks/test to detect possible corruption due to race condition from CS violations.	
				*/
				/* YOUR CODE FOR THE WRITER GOES IN HERE */
				pthread_mutex_lock(&rw_lock);
				temp_accno = account_list[i].accno;
				account_list[i].accno=INVALID_ACCNO;
				account_list[i].balance = update_acc[j].balance;
				account_list[i].accno = temp_accno;
				fprintf(fd, "Account number = %d [%d]: old balance = %6.2f, new balance = %6.2f\n",
                account_list[i].accno, update_acc[j].accno, account_list[i].balance, update_acc[j].balance);
				found=TRUE;
				 
			
				pthread_mutex_unlock(&rw_lock);
			}
		}
       
        if (!found)
            fprintf(fd, "Failed to find account number %d!\n", update_acc[j].accno);

    }   // end test-set for-loop

	fclose(fd);
	return NULL;
}

/* Reader thread - will read the account_list data structure. 
   Takes as argument the seed for the srand() function. 
*/
void * reader_thr(void *arg) {
	printf("Reader thread ID %ld\n", pthread_self());
    srand(*((unsigned int *) arg));   /* set random number seed for this reader */

    int i, j;
	int r_idx;
	unsigned char found;			/* For every read_acc[j], set to TRUE if found in account_list, else set to FALSE */
    account read_acc[READ_ITR];

    /* first create a random data set of account updates */
    for (i = 0; i < READ_ITR;i++) {
        r_idx = rand() % SIZE;      /* a random number in the range [0, SIZE) */
        read_acc[i].accno = account_list[r_idx].accno;
        read_acc[i].balance = 0.0;		/* we are going to read in the value */
    }

    /* open a reader thread log file */
	char thr_fname[64];
	snprintf(thr_fname, 64, "reader_%ld_thr.log", pthread_self());
	FILE *fd = fopen(thr_fname, "w");
	if (!fd) {
		fprintf(stderr,"Failed to reader log file %s\n", thr_fname);
		pthread_exit(&errno);
	}
 
   	 /* The reader thread will now to read the shared account_list data structure.
	   For each entry 'j' in the read_acc[] array, the reader will fetch the 
     	   corresponding balance from the account_list[] array and store in
     	   read_acc[j].balance. The reader will then make a log entry in the log file
	   for every successful read from the account_list using the format: 
			fprintf(fd, "Account number = %d [%d], balance read = %6.2f\n",
                        	account_list[i].accno, read_acc[j].accno, read_acc[j].balance);  
	   
	   If it is unable to find a account number, then the following log entry must be
	   made: fprintf(fd, "Failed to find account number %d!\n", read_acc[j].accno);

	   Additionally, your code must also introduce checks/test to detect possible corruption due to race condition from CS violations.	

 	*/

    /* The reader thread will now to read the shared account_list data structure */
    


	for (j = 0; j < READ_ITR;j++) {
        /* Now read the shared data structure */
        found = FALSE;
        rest();
        for (i = 0; i < SIZE;i++) {
            
            if (account_list[i].accno == read_acc[j].accno) {
                /* Now lock and update */
				/* YOUR CODE FOR THE READER GOES IN HERE */
				pthread_mutex_lock(&r_lock);
				read_count++;
				
        		
        		if(read_count == 1){
					pthread_mutex_lock(&rw_lock);
				}	
				
				pthread_mutex_unlock(&r_lock);
				
				read_acc[j].balance = account_list[i].balance;
				
        		fprintf(fd, "Account number = %d [%d], balance read = %6.2f\n",
                        	account_list[i].accno, read_acc[j].accno, read_acc[j].balance);
            
				found=TRUE;
				
				pthread_mutex_lock(&r_lock);
				
            
				read_count--;
				
				if(read_count == 0){
					pthread_mutex_unlock(&rw_lock);	
				}
				
				pthread_mutex_unlock(&r_lock);
            
            }
            
         }   

        if (!found)
            fprintf(fd, "Failed to find account number %d!\n", read_acc[j].accno);
    }   // end test-set for-loop

    fclose(fd);
    return NULL;
}

/* populate the shared account_list data structure */
void create_testset() {
	time_t t;
	srand(time(&t));
	int i;	
	for (i = 0;i < SIZE;i++) {
		account_list[i].accno = 1000 + rand() % RAND_MAX;
		account_list[i].balance = 100 + rand() % MAX_BALANCE;
	}	
	return;
}


void usage(char *str) {
	printf("Usage: %s -r <NUM_READERS> -w <NUM_WRITERS>\n", str);
	return;
}


int main(int argc, char *argv[]) {
	time_t t;
	unsigned int seed;
	int i;
	void *result;
	int READ_THREADS;			/* number of readers to create */
	int WRITE_THREADS;			/* number of writers to create */
	
	/* Generate a list of account informations. This will be used as the input to the Reader/Writer threads. */
	create_testset();

	/* USE getopt() to read as command line arguments the number of readers (READ_THREADS) 
	   and number of writers (WRITE_THREADS). If any of the arguments are missing, then print
       the usage message using the usage() function defined above and exit. 
	   For reference on getopt(), see "man getopt(3)" 
	*/
	/* YOUR CODE GOES HERE */
	int argument;
	
	while(((argument = getopt(argc , argv,"r:w:")) != -1)) {
			switch(argument){
			case 'r':
			  READ_THREADS = atoi(optarg);
			  break;
			case 'w':
			  WRITE_THREADS = atoi(optarg);
			  break;
			default:
			  usage(argv[0]);
			  exit(EXIT_FAILURE);
			  break;
			}
	}
	

	

	pthread_t* reader_idx = (pthread_t *) malloc(sizeof(pthread_t) * READ_THREADS);		/* holds thread IDs of readers */
	pthread_t* writer_idx  = (pthread_t *) malloc(sizeof(pthread_t) * WRITE_THREADS);		/* holds thread IDs of writers */
	
	/* create readers */
  	for (i = 0;i < READ_THREADS;i++) {
		seed = (unsigned int) time(&t);
		/* YOUR CODE GOES HERE */
		  if (pthread_create((reader_idx+i), NULL, reader_thr, (void *) (intptr_t) (&seed)) != 0) {
				perror("pthread create");
				exit(-1);
		  }
	}
  	
  	printf("Done creating reader threads!\n");

	/* create writers */ 
  	for (i = 0;i < WRITE_THREADS;i++) {
		seed = (unsigned int) time(&t);
		 if (pthread_create((writer_idx+i), NULL, writer_thr, (void *) (intptr_t) (&seed)) != 0) {
			perror("pthread create");
			exit(-1);
		}	
	}
	
	
	printf("Done creating writer threads!\n");

  	/* Join all reader and writer threads.
       YOUR CODE GOES HERE. 
    */
	 for (i = 0;i < READ_THREADS;i++) {
		pthread_join(reader_idx[i], &result);
		printf("Joined %d with status: %d\n", i, (intptr_t) result);
	 }

	printf("Reader threads joined.\n");

	for (i = 0;i < WRITE_THREADS;i++) {
		pthread_join(writer_idx[i], &result);
		printf("Joined %d with status: %d\n", i, (intptr_t) result);
	}

	printf("Writer threads joined.\n");





	/* clean-up - always a good thing to do! */
 	pthread_mutex_destroy(&r_lock);
	pthread_mutex_destroy(&rw_lock);
	
	return 0;
}


