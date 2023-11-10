#include "quiz.h"
#include <stdlib.h>
#include <stdio.h>
#include <signal.h>
#include <unistd.h>
#include <string.h>

quiz_t quiz = { .score = 0, .n = 1, .max = 8 };

static void sig_action(int signum, siginfo_t *siginfo, void *unused);

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

static void sig_action(int signum, siginfo_t *siginfo, void *unused)
{
    (void) unused;
    if (siginfo && siginfo->si_code <= 0) {
        fprintf(stderr, "catp: sig_action: %s (from pid %d, uid %d)\n",
                strsignal(signum), siginfo->si_pid, siginfo->si_uid);
    } else {
        fprintf(stderr, "catp: sig_action: %s\n", strsignal(signum));
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
        play(&quiz);
    }

    return EXIT_SUCCESS;
}