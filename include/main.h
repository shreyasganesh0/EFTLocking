#ifndef MAIN_H
#define MAIN_H


struct {
    int acc_no; 
    int amount;
} account_t;

struct {
    pthread_mutex_t acc1_mtx;
    pthread_mutex_t acc2_mtx;
} worker_t;

struct {
    account_t sender;
    account_t reciver;
} txn_t;

#endif

