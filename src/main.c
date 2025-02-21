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

#define READ_BYTES 4096

char *file_buf;
int num_users;

int main(int argc, char *argv[]) {

    if (argc < 3) {
        my_write(1, "Invalid arguments\n");
        return -1;
    }
    num_users = atoi(argv[2]);
    
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
    
    worker_t *workers = malloc(num_users * sizeof(worker_t)); 
    for (int i = 0; i < num_users; i++) {
        pthread_mutex_init(&workers[i].acc1_mtx, NULL);
        pthread_mutex_init(&workers[i].acc2_mtx, NULL);
    }
    account_setup(fd);
    
    return 0;
}

void accounts_setup(int fd) {
    ssize_t bytes_read;  
    char buf[READ_BYTES];
    temp_acc_arr[200];
    int idx = 0;

    while((bytes_read = read(fd, buf, READ_BYTES) > 0) {
        
        char *ptr = buf;
        
        
        while((char *newline = memchr(ptr, '\n', bytes_read) != NULL) {

            *newline = '\0';
            ehar *space = memchr(ptr, ' ', newline - ptr);
            *space = '\0';
            account_t account.acc_no = atoi(ptr);
            ptr = space;
            ptr++;
            account.amount = atoi(ptr);
            
            temp_acc_arr[idx] = account;
            idx++;
        }
        
             
             
