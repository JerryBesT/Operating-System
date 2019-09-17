#include "types.h"
#include "stat.h"
#include "user.h"

int
main(int argc, char *argv[])
{
    int count = getopenedcount();
    printf(1, "There has been %d open system calls issued\n", count);
    exit();
}
