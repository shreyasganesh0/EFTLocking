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
        pthread_mutex_init(&workers[i].acc2_mtx, NULL);
    }
    account_setup(fd);

    tid_t distribute_tid;
    pthread_init(&distribute_tid, distributor, &fd);

    
    return 0;
}

void *distributor(void *arg) {

    int fd = *(int *)arg;
    ssize_t bytes_read;  
    char buf[READ_BYTES];

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
            
            idx++;

            ptr = newline;
            ptr++;//jmp to next line start chr
        }

        off_t curr_offset = lseek(fd, 0, SEEK_CUR);
        lseek(fd, curr_offset - (end_ptr - ptr), SEEK_SET); //reset the cursor on exit to avoid skipping bytes
    }
}


void accounts_setup(int fd) {
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
}




        
             
             
