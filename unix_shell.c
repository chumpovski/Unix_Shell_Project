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

void the_fork(char *s[], bool waiting, char* fname, bool output, bool input, bool pipe_b, char *pname[]) {
    bool open_file = false;
    int fd = 0;
    int fdp[2];
    int pid_pipe = 0;
    int stdin_backup = dup(STDIN_FILENO);
    pid_t pid;
    pid = fork(); 
    
    if(output == true || input == true) {
        if((open_file == false) &&((fd = open(fname, O_RDWR | O_CREAT)) == -1)){ 
            printf("Didnt open the file\n");
        }
        if((output == true) && (dup2(fd, STDOUT_FILENO) < 0)) { // > used
            printf("Didnt duplicate fd\n");
        }
        else if((input == true) && (dup2(fd, STDIN_FILENO) < 0)) { // < used
            printf("Didn't duplicate fd\n");
        }
        open_file = true;
    }
    // if(pipe_b == true) {
    //     if(pipe(fdp) < 0) {
    //         printf("Pipe() failed\n");
    //     }
    // }

    if (pid < 0) { 
		fprintf(stderr, "Fork Failed");
	}
	else if (pid == 0) { /* child process */
        if(pipe_b == true) {
            if(pipe(fdp) < 0) {
                printf("Pipe() failed\n");
            }
            pid_pipe = fork();
            if(pid_pipe == 0) { // pipe child process
                close(fdp[0]);
                dup2(fdp[1], STDOUT_FILENO);
                close(fdp[1]);
                execvp(s[0], s); // ls -l
            }
            else {
                wait(NULL); // pipe parent process
                close(fdp[1]);
                dup2(fdp[0], STDIN_FILENO);
                close(fdp[0]);
                execvp(pname[0], pname); // less
            }
        }
        else { //child process without piping
            execvp(s[0], s);
        }
	}
	else { /* parent process */
        if(waiting == true) {
            wait(NULL);
        }
	}

    // closing files from redirection
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
    char* pipe_through[BUFSIZ];
    char *tokens[BUFSIZ];
    int counter = 0;
    int redir_offset;
    bool waiting = true;
    bool redir_output = false;
    bool redir_input = false;
    bool pipe = false;

    p = strtok(s, break_chars);
    tokens[counter] = p;

    while(p != NULL){
        ++counter;
      //printf("token was: %s\n", p);
        p = strtok(NULL, break_chars);
        if(p != NULL && strcmp(p, "&") == 0) {
            waiting = false;
            break;
        }
        else if ((p != NULL) && ((strcmp(p, ">") == 0) || (strcmp(p, "<") == 0))) {
            //check for redirection output and dont copy ">" or "<" or filename to tokens
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
        else if ((p != NULL) && (strcmp(p, "|") == 0)) {
            pipe = true;
            p = strtok(NULL, break_chars);
            pipe_through[0] = p; //copy command after "|"
            break;
        }
        tokens[counter] = p;
    }
    the_fork(tokens, waiting, fname, redir_output, redir_input, pipe, pipe_through);
}

int main(void) {

    char input[BUFSIZ];
    char last_command[BUFSIZ];

    bool should_run = true;  // flag to determine when to exit

    memset(input, 0, BUFSIZ * sizeof(char));
    while(should_run) {
        printf("osh> ");
        fflush(stdout);
        
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
               // printf("You entered: %s\n", input);
             parse(input);
            }
        }
    }
    printf("exiting osh\n");

    return 0;
}

// compiled using: 
//      gcc -pthread unix_shell.c -o unix_shell
//
///////////////////////////////////////////////////////////
//  EXAMPLES OF OUTPUT
///////////////////////////////////////////////////////////
// input: ls -l
// output:
// total 32
// -rw-rw-r-- 1 osc osc     7 Mar 25 22:19 in.txt
// -rw-rw-r-- 1 osc osc   208 Mar 25 22:55 out.txt
// -rwxrwxr-x 1 osc osc 13880 Mar 25 22:57 unix_shell
// -rw-rw-r-- 1 osc osc  4497 Mar 25 22:57 unix_shell.c
//
// input: ls > out.txt
// out.txt:
// in.txt
// out.txt
// unix_shell
// unix_shell.c
//
// input: sort < in.txt
// a
// b
// c
// d
// osh> No commands in history.
//
// input: cat in.txt &
// osh> d
// a
// c
// b
