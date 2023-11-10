#include "quiz.h"
#include <stdlib.h>
#include <stdio.h>
#include <signal.h>
#include <unistd.h>
#include <string.h>

quiz_t quiz = { .score = 0, .n = 1, .max = 8 };

static void sig_action(int signum, siginfo_t *siginfo, void *unused);

// signal installer
static void sig_install(void)
{
    struct sigaction sa;

    sa.sa_sigaction = sig_action;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_SIGINFO | SA_RESTART;

    if (sigaction(SIGINT, &sa, NULL) == -1) {
        perror("sigaction");
        exit(EXIT_FAILURE);
    }
}
// signal handler
static void sig_action(int signum, siginfo_t *siginfo, void *unused)
{
    (void) unused;

    if (siginfo && siginfo->si_code <= 0) {
        fprintf(stderr, "sig_action: failed!\n");
        exit(EXIT_FAILURE);
    } else {
        free(quiz.question);
        for (size_t i = 0; i < 4; i++) {
            free(quiz.choices[i]);
        }

        // cover case where max is overcounted
        if (quiz.n != 1) {
            quiz.max -= 8;
        }
        printf("\n\nThanks for playing today.\n"
               "Your final score is %d/%d.\n", quiz.score, quiz.max);
        exit(EXIT_SUCCESS);
    }
}

int main(void) 
{
    sig_install();

    printf("Answer multiple choice questions about computer science."
           " You score points for each correctly answered question."
           " If you need multiple attempts to answer a question, the"
           "points you score for a correct answer go down.\n\n");

    while (1) {
        // call the play function to start a quiz round
        if (play(&quiz) == -1) {
            fprintf(stderr, "play: failed!\n");
            return EXIT_FAILURE;
        }
        
        // free memory allocated for the quiz question, answer, and choices
        free(quiz.question);
        free(quiz.answer);
        for (size_t i = 0; i < 4; i++) {
            free(quiz.choices[i]);
        }
    }
}
