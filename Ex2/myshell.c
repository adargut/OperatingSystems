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

// Custom handler for sigints
void handle_sigint(sig) // Handle SIGINTS
        int sig;
{
    if (curr_pid!=getpid()) exit(1); // Exit for processes != shell process
    signal(SIGINT, SIG_IGN); // Set to SIG_IGN for shell
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
        signal(SIGINT,SIG_IGN); // Allow shell to ignore sigints
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
            signal(SIGINT, SIG_IGN); // Parent process needs to ignore sigints
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
            signal(SIGINT, SIG_DFL);
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
                signal(SIGINT, SIG_DFL);
                dup2(pipefd[1], 1);
                close(pipefd[0]);
                valid = execvp(arglist[0], arglist);
                if (-1 == valid) {
                    perror("Execvp failed\n");
                    exit(1);
                }
            } else {
                /* Parent Code */
                signal(SIGINT, handle_sigint); // Parent process need not terminate upon sigint
                close(pipefd[0]);
                close(pipefd[1]);
                int status = 0;
                // Wait for both children to read/write
                waitpid(child_a, &status, 0);
                waitpid(child_b, &status, 0);
                signal(SIGINT, SIG_IGN); // Parent process need not terminate upon sigint
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
            signal(SIGINT,SIG_DFL); // Foreground children must terminate upon sigint
            valid = execvp(arglist[0], arglist);
            if (-1 == valid) {
                perror("Execvp failed\n");
                exit(1);
            }
        } else { // Parent process
            signal(SIGINT, handle_sigint); // Parent process need not terminate upon sigint
            int status;
            while (-1 != wait(&status));
        }
    }
}

#pragma GCC diagnostic pop