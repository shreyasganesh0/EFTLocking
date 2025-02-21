#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <string.h>

#include "syscalls.c"

#define READ_BYTES 4096

char *file_buf;

int main(int argc, char *argv[]) {

    if (argc < 3) {
        my_write(1, "Invalid arguments\n");
        return -1;
    }
    
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
    
    account_t *workers = malloc(num_users * sizeof(account_t)); 
    for (int i = 0; i < num_users; i++) {
        accounts[i].user_id = i + 1;
        accounts[i].mutex = pthread_mutex_init(accounts[i].mutex,workers
    }
    account_setup(fd);
    
    return 0;
}

void accounts_setup(int fd) {
    ssize_t bytes_read;  
    char buf[READ_BYTES];

    while((bytes_read = read(fd, buf, READ_BYTES) > 0) {
        
        char *ptr = buf;
        
        while((char *newline = memchr(ptr, '\n', bytes_read) != NULL) {
          
            ehar *space = memchr(ptr, ' ', newline - ptr);
            strncpy(
          
             
