#ifndef MAIN_H
#define MAIN_H

#define READ_BYTES 4096
#define TXN_BUF_SIZE 10 //should not be needed for this case 

char *file_buf;
int num_workers;
int num_accounts;
int acc_capacity = 200;

typedef struct {
    pthread_mutex_t acc_mtx;
    int acc_no;
    int balance;
} account_t;
account_t* accounts;

typedef struct {
    account_t *sender;
    account_t *reciever;
    int amt;
} txn_t;

typedef struct {
    txn_t txn_buf[TXN_BUF_SIZE];
    sem_t full;
    sem_t empty;
    int in;
    int out;
} worker_t;
worker_t *workers;

void *distributor(void *arg);
void *etf_handler(void *arg);
int account_setup(int fd);

#endif

