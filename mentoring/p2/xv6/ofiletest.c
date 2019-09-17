#include "types.h"
#include "stat.h"
#include "user.h"

int
main(int argc, char *argv[])
{
	if(argc < 2) {
		printf(2, "invalid argument");
		exit();
	}

	int open_num = atoi(argv[1]);
	for(int i = 0;i < open_num;i++)
		open(argv[1], 0 || 512);
	
	for(int i = 2;i < argc;i++) {
		int fd = atoi(argv[i]);
		close(fd);
	}

	int fd_opened = getofilecnt(getpid());
	int fd_next = getofilenext(getpid());

	printf("%d %d\n", fd_opened, fd_next);

  exit();
}
