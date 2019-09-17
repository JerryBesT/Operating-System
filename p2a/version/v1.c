#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <ctype.h>
#include <sys/stat.h>
#include <fcntl.h>

int total_command = 0;
struct node *exe_path;

struct node {
    char *data;
    struct node *next;
};

void freeList(struct node *head) {
    struct node *temp;
    while(head != NULL) {
        temp = head;
        head = head->next;
        free(temp);
    }
}

void addLast(struct node *head, char *info) {
    struct node *ptr = head;
    while(ptr->next != NULL) { 
        ptr = ptr->next;
    }
    struct node *temp = (struct node*)malloc(sizeof(struct node));
    ptr->next = temp;
    temp->data = strdup(info);
    temp->next = NULL;
}

void errorMsg() {
    char error_message[30] = "An error has occurred\n";
    write(STDERR_FILENO, error_message, strlen(error_message)); 
}

int path_try(char *command, struct node *arg_list) {
    struct node *ptr = exe_path;
    char *command_path;
    while(ptr->next != NULL) {
        char *temp = malloc(1000);
        strcat(temp, strdup(ptr->next->data));
        strcat(temp, command);   
        if(access(temp, X_OK) == -1 && ptr->next->next == NULL) {
            errorMsg();
            return -1;
        }
        else if(access(temp, X_OK) == -1) {
            ptr = ptr->next;
        }
        else {
            command_path = temp;
            break;
        }
    }

    int rc = fork();
    if(rc < 0) {
        fprintf(stderr, "fork fail\n");
        exit(1);
    }
    else if(rc == 0) {
        // child
        char* ls_argv[100];
        ls_argv[0] = command;
        int i = 1;
        struct node *ptr2 = arg_list;
        while(ptr2->next != NULL) {
            ls_argv[i] = ptr2->next->data;
            ptr2 = ptr2->next;
            i++;
        }    
        ls_argv[i] = NULL;
        int exec_rc = execv(command_path, ls_argv);     
        if(exec_rc == -1) {
            errorMsg();
            return -1;
        }
    }
    else {
        wait(NULL);
    }
    return 0;
}

void history_command(struct node *head) {
    struct node *temp = head;
    while(temp->next != NULL) {
        printf("%s", temp->next->data);
        temp = temp->next;
    }
}

void history_command_numbered(struct node *head, int itr, int total) {
    struct node *temp = head;
    int first = total - itr;
    for(int i = 0;i < first;i++)
        temp = temp->next;
    for(int i = 0;i < itr;i++) {
        printf("%s", temp->next->data);
        temp = temp->next;
    }
}

void check_history(char *arg, struct node *head) {
    int err = 0, isDecimal = 0, isNeg = 0;
    char *arg_dup = strdup(arg);

    while(*arg) {
        int ascii = *arg - '0';
        if(ascii == -3)
            isNeg = 1; 
        if(ascii == -2)
            isDecimal = 1;
        if(isdigit(*arg) == 0)
            err++;
        arg++;
    }

    // did not handle upward keyboard
    if(isNeg == 1)
        return;
    else if(err > 1)
        errorMsg();
    else if(err == 1 && isDecimal == 0)
        errorMsg(); 
    else if(err == 1 && isDecimal == 1) {
        // can not handle 1.000000000000000000000000000000000001
        double num = atof(arg_dup);
        int integral = (int) num;
        double decimal = integral;
        int itr = integral;
        if(num - decimal != 0)
            itr = integral + 1; 
        history_command_numbered(head, itr, total_command);
    }
    else if(err == 0 && isDecimal == 0) {
        int itr = atoi(arg_dup);
        if(itr >= total_command)
            history_command(head); 
        else if(itr <= 0)
            return;
        else
            history_command_numbered(head, itr, total_command);
    }
}

void path_command(struct node* arg_list) {
    freeList(exe_path);
    struct node *temp = arg_list;
    exe_path = (struct node*)malloc(sizeof(struct node));
    exe_path->next = NULL;
    while(temp->next != NULL){
        char *dir = strdup(temp->next->data); // if I don't do strdup, it is segfault
        if(*(dir + (strlen(dir) - 1)) != '/') {
            strcat(dir, "/");
        }
        addLast(exe_path, dir);
        temp = temp->next;
    }
}

void redirect_command(char *command, char* buffer) {
    // check the format, here assume that command is valid
    // split the string according to '>'
    struct node *left_arg = (struct node*)malloc(sizeof(struct node));
    char *str = strdup(buffer);
    char *left = strtok(str, ">");
    char *right;
    if(left == NULL)
        right = strtok(str, ">");
    else
        right = strtok(NULL, ">");
        

    // formatting left
    const char delim[4] = " \t\n";
    char *token;
    token = strtok(left, delim);
    command = token;
    while(token != NULL) {
        token = strtok(NULL, delim);
        if(token != NULL)
            addLast(left_arg, token);
    }

 
    // formatting right
    int total = 1;
    char *token2 = strtok(right, delim);
    if(token2 == NULL) {
        errorMsg();
        return;
    }
    char *fileName = strdup(token2);
    while(token2 != NULL) {
        if(total == 2) {
            errorMsg();
            return;
        }
        token2 = strtok(NULL, delim);
        total++;
    }

    int rc = open(fileName, O_WRONLY | O_TRUNC | O_CREAT,
                     S_IRUSR | S_IRGRP | S_IWGRP | S_IWUSR);
    if(rc < 0) {
        printf("something is wrong with opening file and I do not know what happened");
        return;
    }

    // could be concise by useing fork()
    int fd = dup(1);
    int err = dup(1);
    
    dup2(rc,1);
    dup2(rc,2);
    path_try(command, left_arg);
    dup2(fd, 1);
    dup2(err, 2);
    close(rc);
}

void pipe_command(char *command, char* buffer) {
    // ***************** wish> > a undon!!!*************

    // check the format, here assume that command is valid
    // split the string according to '>'
    struct node *left_arg = (struct node*)malloc(sizeof(struct node));
    struct node *right_arg = (struct node*)malloc(sizeof(struct node));
    char *str = strdup(buffer);
    char *left = strtok(str, "|");
    char *right = strtok(NULL, "|");
    if(right == NULL) {
        errorMsg();
        return;
    }

    // formatting left
    const char delim[4] = " \t\n";
    char *token;
    token = strtok(left, delim);
    command = token;
    while(token != NULL) {
        token = strtok(NULL, delim);
        if(token != NULL)
            addLast(left_arg, token);
    }

    // formatting right
    char *token2 = strtok(right, delim);
    char *command2 = token2;
    //char *fileName = strdup(token2);
    while(token2 != NULL) {
        token2 = strtok(NULL, delim);
        if(token2 != NULL)
            addLast(right_arg, token2);
    }

    int fds[2];
    pipe(fds);
    int rc = fork();
    if(rc == 0) { // child process
        dup2(fds[0], STDIN_FILENO);
        close(fds[0]);
        close(fds[1]);
        path_try(command, left_arg);
    }
    else { // parent process
        close(fds[0]);
        path_try(command2, right_arg);
        close(fds[1]);
        wait(NULL);
    }
}

void exe(char* buffer) {

}

int main(int argc, char* argv[]) { 
        
    exe_path = (struct node*)malloc(sizeof(struct node));
    addLast(exe_path, "/bin/");

    // for all the command typed
    struct node *head = (struct node*)malloc(sizeof(struct node));

    while(1) {
        printf("wish> ");
        fflush(stdout);
        char *buffer = NULL;
        size_t len = 0;
        total_command++;

        // for storing all the arguments each command
        struct node *arg_list = (struct node*)malloc(sizeof(struct node));
        arg_list->next = NULL; 

        // getting stdin for a line
        if(getline(&buffer, &len, stdin) != -1) {
            fflush(stdout);

            // *************************** batch check?  **************************
            if(1 == 0) {
                FILE *fp = fopen(buffer, "r");
                if(fp != NULL) {
                    char *fp_buffer = NULL;
                    while(getline(&fp_buffer, 0, fp) != -1) {
                    }
                    fclose(fp);
                }
            }

            char *d_buffer = strdup(buffer);
            // if nothing is typed to the shell, continue
            if(strlen(buffer) == 1)
                continue;

            // adding one more history to the history command
            addLast(head, buffer);

            // check redirection first lol, and pipe
            int redirect = 0, pipe = 0;
            for(int i = 0;i < strlen(buffer);i++) {
                if(*(buffer + i) == '>') {
                    redirect++;
                }
                if(*(buffer + i) == '|') {
                    pipe++;
                }
            }

            // splitting the command 
            const char delim[4] = " \t\n";
            char *token;
            int arg_num = 1;
            char *command;
            char *arg;

            token = strtok(buffer, delim);
            command = token;
            if(command == NULL)
                continue;

            // iterating the args
            while(token != NULL) {
                if(arg_num >= 2)
                    addLast(arg_list, token);
                if(arg_num == 2)
                    arg = token;
                token = strtok(NULL, delim);
                arg_num++;
            }

            // *************************** built-in **************************
            // command is "exit"
            char *exit_command = "exit";
            if(strlen(command) == 4 && strncmp(command, exit_command, 4) == 0) {
                if(arg_num >= 3) {
                    errorMsg();
                    continue;
                }
                else
                    exit(0);
            }
            
            // command is "cd"
            char *cd = "cd";
            if(strlen(command) == 2 && strncmp(command, cd, 2) == 0) {
                if(arg_num == 2 || arg_num >= 4) {
                    errorMsg();
                    continue;
                }
                else {
                    int cd_rc = chdir(arg);
                    if(cd_rc != 0) {
                        errorMsg();
                    }     
                    continue;
                }
            }            
            
            // command is "history"
            char *history = "history";
            if(strlen(command) == 7 && strncmp(command, history, 7) == 0) {
                if(arg_num == 2) {
                    history_command(head); 
                    continue;
                }
                else if(arg_num == 3) {
                    check_history(arg, head);
                    continue;
                }
                else {
                    errorMsg();
                    continue;
                }
            }
            
            // command is "path"
            char *path = "path";
            if(strlen(command) == 4 && strncmp(command, path, 4) == 0) {
                path_command(arg_list); 
                continue;
            }

            // *************************** built-in **************************

            // ******************* check redirect and pipe command ************************
            if(redirect == 1 && pipe == 1) {
                errorMsg();
                continue;
            }

            if(pipe > 1) {
                errorMsg();
                continue;
            }

            if(pipe == 1) {
                pipe_command(command, d_buffer); 
                continue;
            } 

            if(redirect > 1) {
                errorMsg();
                continue;
            }

            if(redirect == 1) {
                redirect_command(command, d_buffer); 
                continue;
            } 

            if(redirect == 0 && pipe == 0)
                if(path_try(command, arg_list) == -1)
                    continue;
        }
        freeList(arg_list);
        fflush(stdout);
    }

    // free up the entire linked list -> head, arg_list
    freeList(head);
    exit(0);
}
