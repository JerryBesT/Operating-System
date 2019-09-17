#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main (int argc, char *argv[]) {
    if (argc == 1 || argc >= 6) {
        fprintf(stderr, "across: invalid number of arguments\n");
        exit(1);
    }
    FILE *file;
    if (argc == 5) {
        file = fopen(argv[4], "r");
        if (file == NULL) {
            fprintf(stderr, "across: cannot open file\n");
            exit(1);
        }
    } else {
        file = fopen("/usr/share/dict/words", "r");
        if (file == NULL) {
            fprintf(stderr, "across: cannot open file\n");
            exit(1);
        }
    }
  

    const int bufsize = 257;
    char buffer[bufsize];
    char *key = argv[1]; // too
    int start = atoi(argv[2]); // 1
    int len = atoi(argv[3]); // 6
		// 1 + 6 
    if ((start+strlen(key) > len)) {
        fprintf(stderr, "across: invalid position\n");
        exit(1);
    }
   
    while (fgets(buffer, bufsize, file) != NULL) {
    		char buffKey[strlen(key) + 1];
        int valid = 1;    // if the word is valid
        for (int i = 0; i < strlen(buffer); i++) {
            if (buffer[i] < 'a' || buffer[i] > 'z') {
                //printf("str: %s\n", buffer);
                valid = 0;
            }
        }
       
        if (valid == 1) {
            strncpy(buffKey, buffer + start, strlen(key));
                printf("str: %s\n", buffer);
            if (strcmp(buffKey, key) == 0 && strlen(buffer) == len) {
                printf("%s", buffer);
            }
        }
    }
    return 0;
}
