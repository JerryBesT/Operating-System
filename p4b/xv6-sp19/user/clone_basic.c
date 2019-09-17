/*
 * test for basic functionailty of clone(), with no cleanup
 * Authors:
 * - Varun Naik, Spring 2018
 */
#include "types.h"
#include "stat.h"
#include "user.h"

#define PGSIZE 0x1000
#define check(exp, msg) if(exp) {} else {\
  printf(1, "%s:%d check (" #exp ") failed: %s\n", __FILE__, __LINE__, msg);\
  printf(1, "TEST FAILED\n");\
  kill(ppid);\
  exit();}

int ppid = 0;
volatile int global = 0;
char *stack = 0;

void
func(void *arg1, void *arg2)
{
  int pid;

  pid = getpid();
  check(pid > ppid, "getpid() returned the wrong pid");
	printf(1, "stack = %d\n", stack);
	printf(1, "pid = %d\n", (char *)&pid);
  check(
    stack < (char *)&pid && (char *)&pid < stack + PGSIZE,
    "&pid is not in range (stack, stack + PGSIZE)"
  );

  while (global == 0) {
    // Spin
  }

  check(global == 1, "global is incorrect");

  ++global;

  // The test case passes as soon as this message is printed
  printf(1, "PASSED TEST!\n");

  // Should only reach here after test passes
  exit();

  check(0, "Continued after exit");
}

int
main(int argc, char *argv[])
{
  int pid;

  ppid = getpid();
  check(ppid > 2, "getpid() failed");

  // Expand address space for stack
	//printf(1, "stack1 = %x\n", stack);
  stack = sbrk(2*PGSIZE);
	//printf(1, "stack2 = %x\n", stack);
  check(stack != (char *)-1, "sbrk() failed");
  check((uint)stack % PGSIZE == 0, "sbrk() return value is not page aligned");
  stack += PGSIZE;
	//printf(1, "stack3 = %x\n", stack);

  pid = clone(&func, NULL, NULL, stack);
  check(pid > ppid, "clone() failed");

  ++global;

  // Sleep, so that the test passes before the parent terminates
  sleep(10);

  // Should only reach here after test passes
  check(global == 2, "global is incorrect");

  exit();
}
