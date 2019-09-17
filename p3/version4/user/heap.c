/* heap can grow to 5 pages below stack, but not more */
#include "types.h"
#include "user.h"

#undef NULL
#define NULL ((void*)0)

#define assert(x) if (x) {} else { \
  printf(1, "%s: %d ", __FILE__, __LINE__); \
  printf(1, "assert failed (%s)\n", # x); \
  printf(1, "TEST FAILED\n"); \
  exit(); \
}

int
main(int argc, char *argv[])
{
  uint sz = (uint) sbrk(0);
  uint stackpage = (160 - 1) * 4096;
  uint guardpage = stackpage - 5 * 4096;

  // ensure they actually placed stack high...
  *(char*)stackpage = 'a';

  printf(1, "1\n");
  // full use of heap possible
  assert((int) sbrk(guardpage - sz) != -1);
  // but not into guardpage
  assert((int) sbrk(1) == -1);
  printf(1, "1\n");
  assert((int) sbrk(1*4096) == -1);
  printf(1, "2\n");
  assert((int) sbrk(5*4096-1) == -1);
  printf(1, "3\n");
  assert((int) sbrk(5*4096) == -1);
  printf(1, "4\n");
  assert((int) sbrk(5*4096+1) == -1);
  printf(1, "5\n");
  assert((int) sbrk(6*4096) == -1);
  printf(1, "6\n");
  assert((int) sbrk(-1*(guardpage - sz)) != -1);
  printf(1, "7\n");
  assert((int) sbrk(guardpage - sz + 1) == -1);
  printf(1, "8\n");
  assert((int) sbrk(guardpage - sz + 1*4096) == -1);
  printf(1, "9\n");
  assert((int) sbrk(guardpage - sz + 5*4096-1) == -1);
  printf(1, "10\n");
  assert((int) sbrk(guardpage - sz + 5*4096) == -1);
  printf(1, "shit\n");
  assert((int) sbrk(guardpage - sz + 5*4096+1) == -1);
  printf(1, "shit\n");
  assert((int) sbrk(guardpage - sz + 6*4096) == -1);
  printf(1, "shit\n");
  assert((int) sbrk(guardpage - sz) != -1);
  printf(1, "TEST PASSED\n");
  exit();
}

