#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/uio.h>

#include "write_syscall.c"

int main(int argc, char *argv[]) {

    if (argc < 3) {

        int x = my_write(1, "Invalid arguments", 18);
        return -1;
    }
    return 0;
}

