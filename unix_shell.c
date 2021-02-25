#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>

#define MAX_LINE 80 //max length command

/*
void redirection(char *s[], int offset) {
    int fd = open(s[offset], O_RDWR);
    dup2(fd, STDOUT_FILENO);
    close(fd);
}
*/

void the_fork(char *s[], bool waiting, int offset, char* fname) {
    pid_t pid;
    pid = fork();
    int fd = 0;

    printf("%s\n", fname);    
    
    if(offset != 0) {
        fd = open(fname, O_RDWR | O_CREAT);
        if(fd == -1){ printf("didnt open the file\n");}
        if(dup2(fd, STDOUT_FILENO) < 0) { printf("didnt duplicate fd\n");}
    }

    if (pid < 0) { 
		fprintf(stderr, "Fork Failed");
	}
	else if (pid == 0) { /* child process */
		execvp(s[0], s);
        //if(offset != 0) {close(fd);}
		printf("child process\n");
	}
	else { /* parent process */
		/* parent will wait for the child to complete */
		printf("pid: %d\n", pid);
        if(waiting == true) {
            printf("were waiting\n");
            wait(NULL);
        }
        //wait(NULL);
		printf("Child Complete\n");
	}
    if(offset != 0) { close(fd);}
}

void parse(char* s) {
    const char break_chars[] = " \t";
    char* p;
    char* fname = "no file";
    char *tokens[BUFSIZ];
    int counter = 0;
    int redirection_offset = 0;
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
            printf("& at the end\n");
            waiting = false;
            break;
        }
        else if (p != NULL && strcmp(p, ">") == 0) {
            //check for redirection output and dont copy "">"" or filename to tokens
            redir_output = true;
            redirection_offset = counter + 1;
            p = strtok(NULL, break_chars);
            fname = p; //copy the file name
            break;
        }
        tokens[counter] = p;
    }
    
    for(int i = 0; i < counter; ++i) {
        printf("%d:%s\n", i, tokens[i]);
    }
/*
    if(strcmp(tokens[counter-1], "&") == 0) {
        printf("& at the end\n");
    }
    if (redir_output == true) {
        redirection(tokens, redirection_offset);
    }
*/
    the_fork(tokens, waiting, redirection_offset, fname);
}

int main(void) {

    char input[BUFSIZ];
    char last_command[BUFSIZ];

    //char *args[MAX_LINE/2 + 1]; //command like arguments
    bool should_run = true;  // flag to determine when to exit

    memset(input, 0, BUFSIZ * sizeof(char));
    memset(input, 0, BUFSIZ * sizeof(char));

    while(should_run) {
        printf("osh> ");
        fflush(stdout);
        bool no_command = false;
        

        if(((fgets(input, BUFSIZ, stdin)) == NULL) ||
            (strcmp(input, "\n") == 0)){
                no_command = true;
                fprintf(stderr, "No commands in history.\n");
                //exit(1);
        }
        input[strlen(input) - 1] = '\0'; //delete newline

        //check history !!
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

        /* After reading user input
            1. fork a child process using fork()
            2. the child process will invoke execvp()
            3. parent will invoke wait() unless command included &
        */
    }

    printf("exiting osh\n");

    return 0;
}