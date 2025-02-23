#ifndef MAIN_H
#define MAIN_H

#define READ_BYTES 4096
#define TXN_BUF_SIZE 2 //should not be needed for this case 
#define NO_ACC 100

char *file_buf;
int num_workers;
int num_accounts;

typedef struct {
    char *ptr;
    char *start;
    int capacity;
    int size;
} buf_t;

typedef struct {
    pthread_mutex_t acc_mtx;
    int acc_no;
    int balance;
} account_t;
account_t accounts[NO_ACC];

typedef struct {
    account_t *sender;
    account_t *reciever;
    int amt;
} txn_t;

typedef struct {
    txn_t txn_buf[TXN_BUF_SIZE];
    sem_t full;
    sem_t empty;
    sem_t mtx;
    int in;
    int out;
} worker_t;
worker_t *workers;

void *distributor(void *arg);
void *etf_handler(void *arg);
void my_stream_buf(int num, int bal, buf_t *buf);
int account_setup(int fd);

#endif

