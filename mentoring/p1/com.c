#include <stdio.h>
#include <stdlib.h>

int main(int argc, char* argv[]) {
	FILE *fp1 = fopen(argv[1], "r");
	FILE *fp2 = fopen(argv[2], "r");
	
	char buffer[1000];
	char buffer2[1000];
	fgets(buffer, 1000, fp1);
	fgets(buffer2, 1000, fp2);
	char* s1 = buffer;
	char* s2 = buffer2;
	s1 = 
	printf("s1 = %s, s2 = %s\n", s1, s2);
  fclose(fp1); 
 fclose(fp2);
}
