// Jerry Gama
// CPSC 351

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>

#define MAX_LINE 80 //max length command

void the_fork(char *s[], bool waiting, char* fname, bool output, bool input) {
    printf("Entering the_fork()\n");
    bool open_file = false;
    int fd = 0;
    int stdin_backup = dup(STDIN_FILENO);
    pid_t pid;
    pid = fork();
    printf("running the fork()\n");
    if(open_file == true) {
          fflush(stdout); // trying to remove whatever is left in the buffer 
          fflush(stdin);
    }

    printf("filename: %s\n", fname);    
    
    if(output == true || input == true) {
        if((open_file == false) &&((fd = open(fname, O_RDWR | O_CREAT)) == -1)){ 
            printf("didnt open the file\n");
        }
        if((output == true) && (dup2(fd, STDOUT_FILENO) < 0)) {
            printf("didnt duplicate fd\n");
        }
        else if((input == true) && (dup2(fd, STDIN_FILENO) < 0)) {
            printf("didn't duplicate fd\n");
        }
        open_file = true;
    }

    if (pid < 0) { 
		fprintf(stderr, "Fork Failed");
	}
	else if (pid == 0) { /* child process */
		execvp(s[0], s);
		printf("child process\n");
	}
	else { /* parent process */
		printf("pid: %d\n", pid);
        if(waiting == true) {
            wait(NULL);
        }
	}

    // attempting to close the output file
    if(open_file == true) { 
       if(output == true) {
          close(STDOUT_FILENO);
       }
       else if(input == true) {
          close(STDIN_FILENO);
       }
       close(fd);
       open_file = false;
    }
}

void parse(char* s) {
    const char break_chars[] = " \t";
    char* p;
    char* fname = "no file";
    char *tokens[BUFSIZ];
    int counter = 0;
    int redir_offset;
    bool waiting = true;
    bool redir_output = false;
    bool redir_input = false;

    p = strtok(s, break_chars);
    tokens[counter] = p;

    while(p != NULL){
        ++counter;
        printf("token was: %s\n", p);
        p = strtok(NULL, break_chars);
        if(p != NULL && strcmp(p, "&") == 0) {
            waiting = false;
            break;
        }
        else if ((p != NULL) && ((strcmp(p, ">") == 0) || (strcmp(p, "<") == 0))) {
            //check for redirection output and dont copy "> or filename to tokens
            if(strcmp(p, "<") == 0) {
                redir_input = true;
            }
            else {
                redir_output = true;
            }
            p = strtok(NULL, break_chars);
            fname = p; //copy the file name
            break;
        }
        tokens[counter] = p;
    }
    for(int i = 0; i < counter; ++i) {
        printf("%d:%s\n", i, tokens[i]);
    }
    the_fork(tokens, waiting, fname, redir_output, redir_input);
}

int main(void) {

    char input[BUFSIZ];
    char last_command[BUFSIZ];

    bool should_run = true;  // flag to determine when to exit

    memset(input, 0, BUFSIZ * sizeof(char));

    while(should_run) {
        printf("osh> ");
        fflush(stdout);
        fflush(stdin);
        
        bool no_command = false;
        
        if(((fgets(input, BUFSIZ, stdin)) == NULL) ||
            (strcmp(input, "\n") == 0)){
                no_command = true;
                fprintf(stderr, "No commands in history.\n");
                exit(1);
        }
        input[strlen(input) - 1] = '\0'; //delete newline

        if(strncmp(input, "!!", 2) == 0) {
            if(strlen(last_command) == 0) {
                fprintf(stderr, "There is no last command\n");
            }
            printf("Last command was: %s\n", last_command);
            parse(last_command);
        } else if (strncmp(input, "exit()", 6) == 0) { //compare first 6 chars
            should_run = false;
        } else {
            if(strncmp(input, "!!", 2) != 0) {
                strcpy(last_command,input);
            }
            strcpy(last_command,input);
            if(no_command == false) {
                printf("You entered: %s\n", input);
             parse(input);
            }
        }
    }
    printf("exiting osh\n");

    return 0;
}