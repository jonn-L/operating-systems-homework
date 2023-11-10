#include "quiz.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

#define INTIAL_BUFF_SIZE 256

char *fetch(char *url) 
{
    int pipefd[2];
    pid_t pid;
    char *result = NULL;

    if (pipe(pipefd) == -1) {
        fprintf(stderr, "pipe: failed!\n");
        exit(EXIT_FAILURE);
    }

    pid = fork();
    if (pid == -1) {
        fprintf(stderr, "fork: failed!\n");
        exit(EXIT_FAILURE);
    }

    if (pid == 0) { // Child process
        close(pipefd[0]);
        dup2(pipefd[1], STDOUT_FILENO); // duplicate the write end to stdout
        close(pipefd[1]); // close the duplicated write end

        char *args[] = {"curl", "-s", url, NULL};
        execvp("curl", args);
        
        fprintf(stderr, "execvp: failed!\n");
        exit(EXIT_FAILURE);

    } else { // Parent process
        close(pipefd[1]);

        size_t buffer_size = INTIAL_BUFF_SIZE;
        // allocate memory for the result buffer
        result = malloc((sizeof *result) * buffer_size);
        if (result == NULL) {
            fprintf(stderr, "malloc: failed!\n");
            close(pipefd[0]);
            
            // wait for the child process to finish
            if (waitpid(pid, NULL, 0) == -1) {
                fprintf(stderr, "waitpid: failed!\n");
            }
            exit(EXIT_FAILURE);
        }

        size_t bytes_read;
        size_t bytes_read_total = 0;

        // read data from the pipe into the result buffer
        while ((bytes_read = read(pipefd[0], result + bytes_read_total, buffer_size - bytes_read_total)) > 0) {
            bytes_read_total += bytes_read;

            // check if the buffer is full, and if so, resize it
            if (bytes_read_total == buffer_size) {
                buffer_size += INTIAL_BUFF_SIZE; // increase size of buffer
                result = realloc(result, buffer_size);
                if (result == NULL) {
                    fprintf(stderr, "realloc: failed!\n");
                    close(pipefd[0]);
                    waitpid(pid, NULL, 0);
                    exit(EXIT_FAILURE);
                }
            }
        }

        result[bytes_read_total] = '\0';

        close(pipefd[0]);
        
        // Wait for the child process to finish
        if (waitpid(pid, NULL, 0) == -1) {
            fprintf(stderr, "waitpid: failed!\n");
            exit(EXIT_FAILURE);
        }
    }

    return result;
}
