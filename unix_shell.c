#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

#define MAX_LINE 80 //max length command

void the_fork(char *s[], bool waiting) {
    pid_t pid;
    pid = fork();

    if (pid < 0) { 
		fprintf(stderr, "Fork Failed");
	}
	else if (pid == 0) { /* child process */
		execvp(s[0], s);
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
}

void parse(char* s) {
    const char break_chars[] = " \t";
    char* p;
    char *tokens[BUFSIZ];
    int counter = 0;
    bool waiting = true;

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
        tokens[counter] = p;
    }
    
    for(int i = 0; i < counter; ++i) {
        printf("%d:%s\n", i, tokens[i]);
    }

  //  if(strcmp(tokens[counter-1], "&") == 0) {
   //     printf("& at the end\n");
   // }

    the_fork(tokens, waiting);
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