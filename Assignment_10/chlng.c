#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <time.h>

#include "chlng.h"
#include "lib/tcp.h"

chlng_t* chlng_new(void) 
{
    chlng_t *new_chlng = malloc(sizeof(*new_chlng));
    if (new_chlng == NULL) {
        fprintf(stderr, "malloc: failed!\n");
        exit(EXIT_FAILURE);
    }

    new_chlng->text = NULL;
    new_chlng->word = NULL;

    return new_chlng;
}

void chlng_reset(chlng_t* c) 
{
    free(c->text);
    free(c->word);
    c->text = NULL;
    c->word = NULL;
}

void chlng_del(chlng_t* c) 
{
    chlng_reset(c);
    free(c);
}

int chlng_fetch_text(chlng_t *c)
{
    int pipefd[2];
    pid_t pid;
    
    // Create a pipe to capture the output of the 'fortune' command
    if (pipe(pipefd) == -1) {
        (void) fprintf(stderr, "pipe: failed!\n");
        return EXIT_FAILURE;
    }

    // Fork a child process to execute the 'fortune' command
    pid = fork();
    if (pid == -1) {
        (void) fprintf(stderr, "fork: failed!\n");
        return EXIT_FAILURE;
    }

    if (pid == 0) {
        // In the child process, redirect standard output to the pipe
        close(pipefd[0]);
        dup2(pipefd[1], STDOUT_FILENO);
        close(pipefd[1]);

        // Execute the 'fortune' command with the '-s' flag
        char *args[] = {"fortune", "-s", NULL};
        execvp(args[0], args);

        // If execvp fails, print an error message
        (void) fprintf(stderr, "execvp: failed!\n");
        return EXIT_FAILURE;
    } else {
        // In the parent process, close the write end of the pipe
        close(pipefd[1]);

        // Read the output of the 'fortune' command from the pipe
        char buffer[1024];
        ssize_t bytes_read = read(pipefd[0], buffer, sizeof(buffer) - 1);        
        if (bytes_read == -1) {
            fprintf(stderr, "read: failed!\n");
            return EXIT_FAILURE;
        }

        buffer[bytes_read] = '\0';

        // Store the fetched text in the challenge structure
        c->text = strdup(buffer);
        if (c->text == NULL) {
            fprintf(stderr, "strdup: failed!\n");
            return EXIT_FAILURE;
        }

        // Close the read end of the pipe
        close(pipefd[0]);
    }

    return EXIT_SUCCESS;
}

int chlng_hide_word(chlng_t *c) {
    srand((unsigned int)time(NULL));

    // Create a copy of the challenge text to avoid modifying the original
    char *text_copy = strdup(c->text);
    if (text_copy == NULL) {
        fprintf(stderr, "strdup: failed!\n");
        return EXIT_FAILURE;
    }

    // Count the number of words in the challenge text
    char *token = strtok(text_copy, " ");
    int word_count = 0;
    while (token) {
        word_count++;
        token = strtok(NULL, " ");
    }
    
    // Generate a random index to choose a word to hide
    int rnd_ind = rand() % word_count;

    // Reset the text copy and iterate to find the chosen word
    strcpy(text_copy, c->text);
    token = strtok(text_copy, " ");
    int i = 0;
    while (token) {
        if (i == rnd_ind) {
            // Store the chosen word in the challenge structure
            c->word = strdup(token);
            break;
        }
        token = strtok(NULL, " ");
        i++;
    }
    free(text_copy);

    // Find the position of the word in the original text and replace it with underscores
    char *word_to_replace = strstr(c->text, c->word);
    memset(word_to_replace, '_', strlen(c->word));

    return EXIT_SUCCESS;
}
