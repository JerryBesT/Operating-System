#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char *argv[]) {
		if(argc == 2) {
			char *find = argv[1];
			FILE *words = fopen("/usr/share/dict/words", "r");
			char buffer[1000];
			while(fgets(buffer, 1000, words) != NULL) {
				char* comp = buffer;
				int sz = strlen(find);
				if(strncmp(find, comp, sz) == 0)
					printf("%s", buffer);
			}
			fclose(words);
		}
		else if(argc == 3){
			FILE *fp = fopen(argv[2], "r");
			char *find = argv[1];
			char buffer[1000];
			while(fgets(buffer, 1000, fp) != NULL) {
				char* comp = buffer;
				int sz = strlen(find);
				if(strncmp(find, comp, sz) == 0)
					printf("%s", buffer);
			}	
			fclose(fp);
		}
		else if(argc == 4) {
			char *find = argv[1];
			int low = atoi(argv[2]);
			int high = atoi(argv[3]); //high = len
			FILE *words = fopen("/usr/share/dict/words", "r");
			char buffer[1000];
			while(fgets(buffer, 1000, words) != NULL) {
				char* comp = buffer;
				int sz = high + 1;
				char* dest = strstr(comp, find);
				int pos = dest - comp;
				if(strlen(comp) == sz && dest != NULL && (int)pos == low)
					printf("%s", buffer);
			}
			fclose(words);
		}
		else
			exit(0);

    return 0;
}
