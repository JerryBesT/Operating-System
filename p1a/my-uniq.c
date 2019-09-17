#include <stdio.h>
#include <string.h>
#include <stdlib.h>

int main(int argc, char *argv[]) {
    if(argc == 1) {
        char *buffer = NULL;
        char *compare = NULL;
        size_t len = 0;

        while(getline(&buffer, &len, stdin) != -1) {
            if(compare != NULL && strcmp(compare, buffer) == 0){
                    continue; 
            } 
            compare = strdup(buffer);
            printf("%s", buffer);
        }
    }
    else {
        for(int i = 0;i < argc - 1;i++) {
            FILE *fp = fopen(argv[i + 1], "r");
            if(fp == NULL) {
                printf("my-uniq: cannot open file\n");
                exit(1);
            }

            char *buffer = NULL;
            char *compare = NULL;
            size_t len = 0;
            while(getline(&buffer, &len, fp) != -1) {
                if(compare != NULL){
                    int x = strcmp(compare, buffer);
                    if(x == 0)
                        continue; 
                } 
                compare = strdup(buffer);
                printf("%s", buffer);
            }
            fclose(fp);
        }
    }
    return 0;
}
