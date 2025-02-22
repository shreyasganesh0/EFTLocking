#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <string.h>

#include "main.h"
#include "syscalls.c"

int acc_capcity = 200;
int main(int argc, char *argv[]) {

    if (argc < 3) {
        my_write(1, "Invalid arguments\n");
        return -1;
    }
    num_workers = atoi(argv[2]);
    
    int fd = open(argv, O_RDWR, 666);
    if (fd == -1) {
        my_write(1, "Failed to read file\n");
        return  -1;
    }

    struct stat *file_stat;
    if (fstat(fd, file_stat) == -1) {
        
        my_write(1, "Failed to get file stat\n");
        return -1;
    }
    
    workers = malloc(num_workers * sizeof(worker_t));
    for (int i = 0; i < num_workers; i++) {
       sem_init(&workers[i].empty, 0, TXN_BUF_SIZE);
       sem_init(&workers[i].full, 0, 0);
       workers[i].in = 0;
       workers[i].out = 0;
    }
    int num_accounts = account_setup(fd);

    tid_t distribute_tid;
    pthread_create(&distribute_tid, NULL, distributor, &fd);

    pthread_t *worker_tids = malloc(num_workers * sizeof(pthread_t));
    for (int i = 0; i < num_workers; i++) {
        pthread_create(&worker_tids[i], NULL, etf_handler, i);
    }

    pthread_join(distribute_tid, NULL);

    for (int i = 0; i < num_workers; i++) {
        pthread_join(worker_tids[i], NULL);
    }
    
    for (int i = 0; i <= num_accounts; i++) {
        char retval[100];
        char *ptr = retval;
        strcpy(ptr, atoi(accounts[i].acc_no));
        ptr += strlen(ptr) + 1;

        *ptr = ' ';
        ptr++;
        
        strcpy(ptr, atoi(accounts[i].balance));
        ptr += strlen(ptr) + 1;
        *ptr = '\n';
        ptr++;
        *ptr = '\0';
        
        my_write(1, retval);
    }
    return 0;
}

void distributor(void *arg) {

    int fd = *(int *)arg;
    ssize_t bytes_read;  
    char buf[READ_BYTES];
    int curr_worker_idx = 0;

    while((bytes_read = read(fd, buf, READ_BYTES) > 0) {
        
        char *ptr = buf;
        char *end_ptr = ptr + bytes_read;
        
        while((char *newline = memchr(ptr, '\n', end_ptr - ptr) != NULL) {
            // this is the account setup loop

            *newline = '\0';
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
            sem_wait(&curr_worker->empty);
             
            curr_worker->txn_buf[curr_worker->in].sender = &accounts[from_idx - 1]; 
            curr_worker->txn_buf[curr_worker->in].reciever = &accounts[to_idx - 1];
            curr_worker->txn_buf[curr_worker->in].amt = amt;
            
            sem_post(&curr_worker->full);

            curr_worker->in++;
            curr_worker->in = curr_worker->in - (curr_worker->in >= TXN_BUF_SIZE) * TXN_BUF_SIZE; 
            curr_worker_idx = curr_worker_idx - (curr_worker_idx >= num_workers) * num_workers;

            ptr = newline;
            ptr++;//jmp to next line start chr
        }

        off_t curr_offset = lseek(fd, 0, SEEK_CUR);
        lseek(fd, curr_offset - (end_ptr - ptr), SEEK_SET); //reset the cursor on exit to avoid skipping bytes
    }
        for (int i = 0; i < num_workers; i++) {
            worker_t *curr_worker = &workers[i];
            sem_wait(&curr_worker->empty);
             
            curr_worker->txn_buf[curr_worker->in].sender = &accounts[from_idx - 1]; 
            curr_worker->txn_buf[curr_worker->in].reciever = &accounts[to_idx - 1];
            curr_worker->txn_buf[curr_worker->in].amt = -1;
            
            sem_post(&curr_worker->full);

            curr_worker->in++;
            curr_worker->in = curr_worker->in - (curr_worker->in >= TXN_BUF_SIZE) * TXN_BUF_SIZE; 
            curr_worker_idx = curr_worker_idx - (curr_worker_idx >= num_workers) * num_workers;
        }

}

void etf_handler(void *arg) {

    int worker_idx = *(int *)arg;
    worker_t *curr_worker = &workers[worker_idx];

    while (1) {
        sem_wait(curr_worker->full);

        int sig_count = 0;

        while (curr_worker->out != curr_worker->in) {
            
            txn_t *curr_txn = &curr_worker->txn_buf[curr_worker->out];
            if (curr_txn->sender->acc_no < curr_txn->reciever->acc_no) {

                pthread_mutex_lock(&curr_txn->reciever->acc_mtx); 
                pthread_mutex_lock(&curr_txn->sender->acc_mtx);

            } else {

                pthread_mutex_lock(&curr_txn->sender->acc_mtx);
                pthread_mutex_lock(&curr_txn->reciever->acc_mtx); 

            }

            if (curr_txn->amt == -1) {
                return;
            }

            curr_txn->sender->balance -= curr_txn->amt;
            curr_txn->reciver->balance += curr_txn->amt;

            curr_worker->out++;
            curr_worker->out = curr_worker->out - (curr_worker->out >= TXN_BUF_SIZE) * TXN_BUF_SIZE;

            pthread_mutex_unlock(&curr_txn->sender->acc_mtx);
            pthread_mutex_unlock(&curr_txn->reciever->acc_mtx); 

            sig_count++;
        }

        for (int i = 0; i < sig_count; i++) {
            sem_post(&curr_worker->full);
        }
    }

}

int accounts_setup(int fd) {
    ssize_t bytes_read;  
    char buf[READ_BYTES];
    accounts = malloc(capacity * sizeof(account_t));
    int idx = 0;

    while((bytes_read = read(fd, buf, READ_BYTES) > 0) {
        
        char *ptr = buf;
        char *end_ptr = ptr + bytes_read;
        char transfer_flag = '\0';
        
        
        while((char *newline = memchr(ptr, '\n', end_ptr - ptr) != NULL) {
            // this is the account setup loop

            *newline = '\0';
            ehar *space = memchr(ptr, ' ', newline - ptr);

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
        }

        off_t curr_offset = lseek(fd, 0, SEEK_CUR);
        lseek(fd, curr_offset - (end_ptr - ptr), SEEK_SET); //reset the cursor on exit to avoid skipping bytes

        if (transfer_flag) {
            break;
        }
    }

    return idx;
}




        
             
             
