#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <pthread.h>
#include <getopt.h>

const int C = 20;
char coins[] = "0000000000XXXXXXXXXX";

void flip_coin(char coins[], int coin_ind) {
    if ((rand() % 2) == 0) {
        coins[coin_ind] = '0';
    }
    else {
        coins[coin_ind] = 'X';
    }
}

//struct to pass in mutex and number of iterations
typedef struct {
    unsigned int n;
    pthread_mutex_t mutex;
} args;

//thread obtains lock and completes all coin flips
static void* strategy_one(void *arg) {
    args *global_lock = (args *) arg;

    (void) pthread_mutex_lock(&global_lock->mutex);
    for (int i = 0; i < global_lock->n; i++) {
        for (int j = 0; j < C; j++) {
            flip_coin(coins, j);
        }
    }
    (void) pthread_mutex_unlock(&global_lock->mutex);

    return NULL;
}

//thread obtains lock and completes one iteration of coin flips (flips all 20 coins once)
static void* strategy_two(void *arg) {
    args *iteration_lock = (args *) arg;

    for (int i = 0; i < iteration_lock->n; i++) {
        (void) pthread_mutex_lock(&iteration_lock->mutex);
        for (int j = 0; j < C; j++) {
            flip_coin(coins, j);
        }
        (void) pthread_mutex_unlock(&iteration_lock->mutex);
    }

    return NULL;
}

//thread obtains lock and completes one coin flip
static void* strategy_three(void *arg) {
    args *coin_lock = (args *) arg;

    for (int i = 0; i < coin_lock->n; i++) {
        for (int j = 0; j < C; j++) {
            (void) pthread_mutex_lock(&coin_lock->mutex);
            flip_coin(coins, j);
            (void) pthread_mutex_unlock(&coin_lock->mutex);
        }
    }

    return NULL;
}

int main(int argc, char *argv[]) {
    srand(time(NULL));
    int opt, status = EXIT_SUCCESS;
    int opt_p = 100, opt_n = 10000;

    opterr = 0;
    while ((opt = getopt(argc, argv, "p:n:")) != -1) {
        switch (opt) {
            case 'n':
                opt_n = atoi(optarg);
                break;
            case 'p':
                opt_p = atoi(optarg);
                break;
            default:
                fprintf(stderr, "Usage: %s [-p threads] [-n iterations]!\n", argv[0]);
                break;
        }
    }

    if (opt_p <= 0) {
        fprintf(stderr, "Not a valid number of threads: 'p' must be greater than 0!");
        return EXIT_FAILURE;
    }

    if (opt_n <= 0) {
        fprintf(stderr, "Not a valid number of iterations: 'n' must be greater than 0!");
        return EXIT_FAILURE;  
    }

    //variables to calculate time
    clock_t time_start, time_end;
    double time_taken;

    args args = { .n = opt_n, .mutex = PTHREAD_MUTEX_INITIALIZER };  //initialize args
    pthread_t tids[opt_p];  //array for thread ids


    printf("coins: ");
    for (int i = 0; i < C; i++) {
        printf("%c", coins[i]);
    }
    printf(" (start - global lock)\n");
    

    //start clock and create the threads
    time_start = clock();
    for (int i = 0; i < opt_p; i++) {
        if (pthread_create(&tids[i], NULL, strategy_one, &args)) {
            perror("pthread_create");
            status = EXIT_FAILURE;
        }
    }

    //join the threads and stop clock
    for (int i = 0; i < opt_p; i++) {
        if (pthread_join(tids[i], NULL)) {
            perror("pthread_join");
            status = EXIT_FAILURE;
        }
    }
    time_end = clock();
    time_taken = ((double) time_end - (double) time_start) / CLOCKS_PER_SEC * 1000;

    printf("coins: ");
    for (int i = 0; i < C; i++) {
        printf("%c", coins[i]);
    }
    printf(" (end - global lock)\n");
    printf("%d threads x %d flips: %lf ms\n\n", opt_p, opt_n, time_taken);

///////////////////////////////////////////////////////////////////////////////

    printf("coins: ");
    for (int i = 0; i < C; i++) {
        printf("%c", coins[i]);
    }
    printf(" (start - iteration lock)\n");

    //start clock and create threads
    time_start = clock();
    for (int i = 0; i < opt_p; i++) {
        if (pthread_create(&tids[i], NULL, strategy_two, &args)) {
            perror("pthread_create");
            status = EXIT_FAILURE;
        }
    }

    //join threads and stop clock
    for (int i = 0; i < opt_p; i++) {
        if (pthread_join(tids[i], NULL)) {
            perror("pthread_join");
            status = EXIT_FAILURE;
        }
    }
    time_end = clock();
    time_taken = ((double) time_end - (double) time_start) / CLOCKS_PER_SEC * 1000;

    printf("coins: ");
    for (int i = 0; i < C; i++) {
        printf("%c", coins[i]);
    }
    printf(" (end - iteration lock)\n");
    printf("%d threads x %d flips: %lf ms\n\n", opt_p, opt_n, time_taken);

///////////////////////////////////////////////////////////////////////////////

    printf("coins: ");
    for (int i = 0; i < C; i++) {
        printf("%c", coins[i]);
    }
    printf(" (start - coin lock)\n");

    //start time and create threads
    time_start = clock();
    for (int i = 0; i < opt_p; i++) {
        if (pthread_create(&tids[i], NULL, strategy_three, &args)) {
            perror("pthread_create");
            status = EXIT_FAILURE;
        }
    }

    //join threads and stop clock
    for (int i = 0; i < opt_p; i++) {
        if (pthread_join(tids[i], NULL)) {
            perror("pthread_join");
            status = EXIT_FAILURE;
        }
    }
    time_end = clock();
    time_taken = ((double) time_end - (double) time_start) / CLOCKS_PER_SEC * 1000;

    printf("coins: ");
    for (int i = 0; i < C; i++) {
        printf("%c", coins[i]);
    }
    printf(" (end - coin lock)\n");
    printf("%d threads x %d flips: %lf ms\n\n", opt_p, opt_n, time_taken);


    return status;
}