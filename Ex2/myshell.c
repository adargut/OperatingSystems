#include "string.h"
#include "stdio.h"
#include "stdlib.h"
#include "stdbool.h"
#include "errno.h"
#include "wait.h"
#include "signal.h"
#include <unistd.h> // for close
#include <fcntl.h> // for open

// Don't return a value and ignore compiler warning
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wreturn-type"

pid_t curr_pid;

void handle_sigint(sig) // Handle SIGINTS
        int sig;
{
    if (curr_pid!=getpid()) exit(1);
}


int prepare(void) { // Prepare signal handler

    curr_pid = getpid();

    signal(SIGINT,SIG_IGN);
    return 0;
}


int finalize(void) {

    return 0;
}


int is_pipe(int count, char **arglist) { // Check if input command is pipe

    int i = 0;

    while (i++ < count-1) {
        if (strcmp(arglist[i], "|") == 0) return i;
    }
    return -1;
}


int process_arglist(int count, char **arglist) {

    int valid;

    if (strcmp(arglist[count-1], "&") == 0) { // & case, run child process in background
        signal(SIGINT,SIG_IGN);
        pid_t child_pid = fork();

        if (child_pid < 0) {
            perror("Forking failed...\n");
            exit(1);
        }
        else if (child_pid == 0) { // Child process
            arglist[count-1] = NULL;
            valid = execvp(arglist[0], arglist);

            if (-1 == valid) {
                perror("Execvp failed\n");
                exit(1);
            }
        }
        else { // Parent process
            signal(SIGINT, SIG_IGN);
            // Minimize zombie children by waiting for them
            for (pid_t pid = waitpid(-1,NULL,WNOHANG);
                 pid != 0 && pid != -1;
                 pid = waitpid(-1,NULL,WNOHANG));
        }
    }
    else if (-1 != is_pipe(count, arglist)) { // Pipe case
        int pipefd[2];
        int idx = is_pipe(count, arglist);
        arglist[idx] = NULL;
        valid = pipe(pipefd);

        if (-1 == valid) perror("Error occured while piping..\n");

        pid_t child_a, child_b;
        child_a = fork();

        if (child_a < 0) {
            perror("Child process failed\n");
            exit(1);
        }
        if (child_a == 0) {
            /* Child A code */
            dup2(pipefd[0], 0);
            close(pipefd[1]);
            arglist += (idx + 1);
            valid = execvp(arglist[0], arglist);

            if (-1 == valid) {
                perror("Execvp failed\n");
                exit(1);
            }
        } else {
            child_b = fork();

            if (child_b < 0) {
                perror("Child process failed\n");
                exit(1);
            }
            if (child_b == 0) {
                /* Child B code */
                dup2(pipefd[1], 1);
                close(pipefd[0]);
                valid = execvp(arglist[0], arglist);
                if (-1 == valid) {
                    perror("Execvp failed\n");
                    exit(1);
                }
            } else {
                /* Parent Code */
                close(pipefd[0]);
                close(pipefd[1]);
                int status;
                while (-1 != wait(&status)); // wait for both children to read/write
            }
        }
    }
    else { // Not pipe or & case
        int pid = fork();
        signal(SIGINT,SIG_IGN);

        if (pid < 0) {
            perror("Error occured while forking..\n");
            exit(1);
        }
        else if (pid == 0) { // Child process
            signal(SIGINT,SIG_DFL);
            valid = execvp(arglist[0], arglist);
            if (-1 == valid) {
                perror("Execvp failed\n");
                exit(1);
            }
        } else { // Parent process
            signal(SIGINT, handle_sigint);
            int status;
            while (-1 != wait(&status));
        }
    }
}

#pragma GCC diagnostic pop
