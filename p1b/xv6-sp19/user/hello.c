#include "types.h"
#include "stat.h"
#include "user.h"

int
main(int argc, char *argv[])
{
    int loop = atoi(argv[1]);
    printf(1, "Starting to issue %d number of open system calls\n", loop);
    if(argc != 2) {
        printf(1, "wrong argument, exiting\n");
        exit();
    }
    for(int i = 0;i < loop;i++) {
        printf(1, "doing the %d opening\n", i);
        open("test", 0);
        //close(fd);
    }
    exit();
}
