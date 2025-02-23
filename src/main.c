#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <string.h>
#include <semaphore.h>
#include <pthread.h>
#include <stdio.h>

#include "main.h"
#include "syscalls.c"

int main(int argc, char *argv[]) {

    if (argc < 3) {
        my_write(1, "Invalid arguments\n");
        return -1;
    }
    num_workers = atoi(argv[2]);
    
    int fd = open(argv[1], O_RDWR, 666);
    if (fd == -1) {
        my_write(1, "Failed to read file\n");
        return  -1;
    }

    
    workers = malloc(num_workers * sizeof(worker_t));
    for (int i = 0; i < num_workers; i++) {
       sem_init(&workers[i].empty, 0, TXN_BUF_SIZE);
       sem_init(&workers[i].full, 0, 0);
       sem_init(&workers[i].mtx, 0, 1);
       workers[i].in = 0;
       workers[i].out = 0;
    }
    num_accounts = account_setup(fd);

    pthread_t distribute_tid;
    pthread_create(&distribute_tid, NULL, distributor, &fd);

    pthread_t *worker_tids = malloc(num_workers * sizeof(pthread_t));
    
    int *idxs = malloc(num_workers * sizeof(int));
    for (int i = 0; i < num_workers; i++) {
        idxs[i] = i;
        pthread_create(&worker_tids[i], NULL, etf_handler, &idxs[i]);
    }

    pthread_join(distribute_tid, NULL);

    for (int i = 0; i < num_workers; i++) {
        pthread_join(worker_tids[i], NULL);
        printf("Joined the thread %d\n", i);
    }
    
    for (int i = 0; i < num_accounts; i++) {
        char retval[100];
        sprintf(retval, "%d %d\n",accounts[i].acc_no, accounts[i].balance); 
        my_write(1, retval);
    }
    return 0;
}

void *distributor(void *arg) {

    int fd = *(int *)arg;
    ssize_t bytes_read;  
    char buf[READ_BYTES];
    int curr_worker_idx = 0;
    char *newline;
    int loop_count = 1;
    while ((bytes_read = read(fd, buf, READ_BYTES)) > 0) {
        
        char *ptr = buf;
        char *end_ptr = ptr + bytes_read;
        
        printf("Enter loop count: %d\n", loop_count);
        while ((newline = memchr(ptr, '\n', end_ptr - ptr))) {
            // this is the account setup loop

            *newline = '\0';
            printf("Reading line %s\n", ptr);
            char *space = memchr(ptr, ' ', newline - ptr);
            *space = '\0';
            if (strcmp(ptr, "Transfer")) {
                my_write(1, "Invalid transaction detected\n");
                ptr = newline;
                ptr++;
                continue;
            }
            ptr = space;
            ptr++; //jmp from id

            space = memchr(ptr, ' ', newline - ptr);
            *space = '\0';
            int from_idx = atoi(ptr);
            ptr = space;
            ptr++; //jmp to_id

            space = memchr(ptr, ' ', newline - ptr);
            *space = '\0';
            int to_idx = atoi(ptr);
            ptr = space;
            ptr++; //jmp to transfer amount

            int amt = atoi(ptr);

            worker_t *curr_worker = &workers[curr_worker_idx];
            printf("Waiting for signal %d\n", curr_worker_idx);
            sem_wait(&curr_worker->empty);
            printf("Waiting for buf mtx\n");
            sem_wait(&curr_worker->mtx);
            printf("Got buf mtx\n");
             
            curr_worker->txn_buf[curr_worker->in].sender = &accounts[from_idx - 1]; 
            curr_worker->txn_buf[curr_worker->in].reciever = &accounts[to_idx - 1];
            curr_worker->txn_buf[curr_worker->in].amt = amt;
            
            sem_post(&curr_worker->mtx);
            sem_post(&curr_worker->full);
            

            curr_worker->in++;
            curr_worker->in = curr_worker->in - (curr_worker->in >= TXN_BUF_SIZE) * TXN_BUF_SIZE; 
            curr_worker_idx++;
            curr_worker_idx = curr_worker_idx - (curr_worker_idx >= num_workers) * num_workers;

            ptr = newline;
            ptr++;//jmp to next line start chr
            printf("Ran inner loop\n");
        }

        loop_count++;
        off_t curr_offset = lseek(fd, 0, SEEK_CUR);
        lseek(fd, curr_offset - (end_ptr - ptr), SEEK_SET); //xit to avoid skipping bytes
    }
    printf("No more input lines found\n");
	for (int i = 0; i < num_workers; i++) {
            printf("Sending exit code to worker %d\n", i);

	    worker_t *curr_worker = &workers[i];
	    sem_wait(&curr_worker->empty);
            sem_wait(&curr_worker->mtx);
	     
	    curr_worker->txn_buf[curr_worker->in].sender = &accounts[0]; 
	    curr_worker->txn_buf[curr_worker->in].reciever = &accounts[1];
	    curr_worker->txn_buf[curr_worker->in].amt = -1;
	    
            sem_post(&curr_worker->mtx);
	    sem_post(&curr_worker->full);

	    curr_worker->in++;
	    curr_worker->in = curr_worker->in - (curr_worker->in >= TXN_BUF_SIZE) * TXN_BUF_SIZE; 
	}
    return NULL;

}

void *etf_handler(void *arg) {

    int worker_idx = *(int *)arg;
    worker_t *curr_worker = &workers[worker_idx];

    while (1) {
        sem_wait(&curr_worker->full);
        sem_wait(&curr_worker->mtx);
            
        txn_t *curr_txn = &curr_worker->txn_buf[curr_worker->out];
        if (curr_txn->sender->acc_no > curr_txn->reciever->acc_no) {

	    pthread_mutex_lock(&curr_txn->reciever->acc_mtx); 
	    pthread_mutex_lock(&curr_txn->sender->acc_mtx);

        } else {

	    pthread_mutex_lock(&curr_txn->sender->acc_mtx);
	    pthread_mutex_lock(&curr_txn->reciever->acc_mtx); 

        }

        if (curr_txn->amt == -1) {
	    printf("\n ------ Exiting thread %d\n\n", worker_idx);
	    pthread_mutex_unlock(&curr_txn->sender->acc_mtx);
	    pthread_mutex_unlock(&curr_txn->reciever->acc_mtx); 
	    sem_post(&curr_worker->mtx);
	    sem_post(&curr_worker->empty);
    	    return NULL;
        }
        curr_txn->sender->balance -= curr_txn->amt;
        curr_txn->reciever->balance += curr_txn->amt;

        curr_worker->out++;
        curr_worker->out = curr_worker->out - (curr_worker->out >= TXN_BUF_SIZE) * TXN_BUF_SIZE;

        pthread_mutex_unlock(&curr_txn->sender->acc_mtx);
        pthread_mutex_unlock(&curr_txn->reciever->acc_mtx); 

        sem_post(&curr_worker->mtx);
        sem_post(&curr_worker->empty);
    }
    return NULL;

}

int account_setup(int fd) {
    ssize_t bytes_read;  
    char buf[READ_BYTES];
    accounts = malloc(acc_capacity * sizeof(account_t));
    int idx = 0;
    char *newline;

    while((bytes_read = read(fd, buf, READ_BYTES)) > 0) {
        
        char *ptr = buf;
        char *end_ptr = ptr + bytes_read;
        char transfer_flag = '\0';
        
        
        while((newline = memchr(ptr, '\n', end_ptr - ptr)) != NULL) {
            // this is the account setup loop

            *newline = '\0';
            char *space = memchr(ptr, ' ', newline - ptr);

            *space = '\0';
            if (!strcmp(ptr, "Transfer")) {
                transfer_flag = 'x';
                break;
            }
            
            accounts[idx].acc_no = idx + 1;
            ptr = space;
            ptr++; //jmp to amount
                   
            accounts[idx].balance = atoi(ptr);
             
            pthread_mutex_init(&accounts[idx].acc_mtx, NULL);
            idx++;

            ptr = newline;
            ptr++;//jmp to next line start chr
            my_write(1, "init one account\n");
        }

        off_t curr_offset = lseek(fd, 0, SEEK_CUR);
        lseek(fd, curr_offset - (end_ptr - ptr), SEEK_SET); //reset the cursor on exit to avoid skipping bytes

        if (transfer_flag) {
            my_write(1, "init complete\n");
            break;
        }
    }

    return idx;
}

