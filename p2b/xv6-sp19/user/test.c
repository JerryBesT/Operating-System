#include "types.h"
#include "stat.h"
#include "user.h"

int
main(int argc, char *argv[])
{
  if(argc <= 1){
    exit();
  }
    
  int x;
  for(int i = 0; i < atoi(argv[1]); i++){
      x = x + i;
  }

  exit();
}
