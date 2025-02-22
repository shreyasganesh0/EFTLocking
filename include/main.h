#ifndef MAIN_H
#define MAIN_H


#define READ_BYTES 4096
#define TXN_BUF_SIZE 10 //should not be needed for this case 

char *file_buf;
int num_users;

struct {
    pthread_mutex_t acc_mtx;
    int acc_no;
    int balance;
} account_t;
account_t* accounts;

struct {
    account_t *sender;
    account_t *reciver;
    int amt;
} txn_t;

struct {
    txn_t txn_buf[TXN_BUF_SIZE];
    sem_t full;
    sem_t empty;
    int in;
    int out;
} worker_t;
worker_t *workers;

#endif

