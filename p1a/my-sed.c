#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void modify(char *buffer, char *find, char *replace) {
    if(strlen(buffer) < strlen(find)) {
        printf("%s", buffer);
        return;
    }

    char *newSentence = malloc(strlen(buffer) 
                        + strlen(find) + strlen(replace) + 1000);
    char *ptr = newSentence;

    while(*buffer) {
        int x = strncmp(buffer, find, strlen(find));
        if(x == 0) {
            strcpy(ptr, replace);
            buffer += strlen(find);
            ptr += strlen(replace);
        }
        else {
            *ptr = *buffer;
            ptr++;
            buffer++;    
        } 
    }
    *ptr = 0;
    printf("%s", newSentence);
    free(newSentence);
    newSentence = NULL;
}

int main(int argc, char *argv[]) {
    if(argc <= 2) {
        printf("my-sed: find_term replace_term [file ...]\n");
        exit(1);
    }
    else if(argc == 3) {
        char *buffer = NULL;
        size_t len = 0;

        while(getline(&buffer, &len, stdin) != -1) { //multiple?
            char *find = argv[1];
            char *replace = argv[2];
            modify(buffer, find, replace);
        }
    }
    else {
        for(int i = 0;i < argc - 3;i++) {
            FILE *fp = fopen(argv[i + 3], "r");
            if(fp == NULL) {
                printf("my-sed: cannot open file\n");
                exit(1);
            }

            char *find = argv[1];
            char *replace = argv[2];
            char *buffer = NULL;
            size_t len = 0;
            while(getline(&buffer, &len, fp) != -1) {
                modify(buffer, find, replace);
            }
            fclose(fp);
        }
    }
    return 0;
}
