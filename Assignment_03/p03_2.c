#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <getopt.h>
#include <pthread.h>

//structure to package the range in one variable and pass it to the thread
typedef struct {
    uint64_t start;
    uint64_t end;
} Range;

static int is_perfect(uint64_t num) {
    uint64_t i, sum;

    if (num < 2) {
        return 0;
    }

    for (i = 2, sum = 1; i*i <= num; i++) {
        if (num % i == 0) {
            sum += (i*i == num) ? i : i + num / i;
        }
    }

    return (sum == num);
}

//thread function that checks all the numbers in the range
void *evaluate_range(void *args) {
    Range *range = args;
    for (uint64_t i = range->start; i <= range->end; i++) {
        if (is_perfect(i)) {
            printf("%lu\n", i);
        }
    }

    return NULL;
}

int main(int argc, char *argv[]) {
    int opt, status = EXIT_SUCCESS;
    uint64_t opt_s = 1, opt_e = 10000;
    int opt_v = 0, opt_t = 1;

    opterr = 0;
    while ((opt = getopt(argc, argv, "s:e:t:v")) != -1) {
        switch (opt) {
            case 's':
                opt_s = strtoull(optarg, NULL, 10);
            case 'e':
                opt_e = strtoull(optarg, NULL, 10);
                break;
            case 't':
                opt_t = atoi(optarg);
                break;
            case 'v':
                opt_v = 1;                fprintf(stderr, "Usage: %s [-p threads] [-n iterations]!\n", argv[0]);

                break;
            default:
                fprintf(stderr, "Usage: %s [-s start] [-e end] [-t threads] [-v]!\n", argv[0]);
                break;
        }
    }

    //corner cases where the given options are not valid
    if (opt_e < opt_s) {
        fprintf(stderr, "Not a valid range: 'e' must be larger than 's'!");
        return EXIT_FAILURE;
    }

    if (opt_t <= 0) {
        fprintf(stderr, "Not a valid number of threads: 't' must be larger than 0!");
        return EXIT_FAILURE;
    }

    //calculate the size of the sub-ranges and initialize an array for them
    uint64_t range_len_thread = (opt_e - opt_s + 1) / opt_t;
    int remainder = (opt_e - opt_s + 1) % opt_t;
    uint64_t range_curr_start = opt_s;
    Range ranges[opt_t];

    //determine the where the sub-ranges start and end
    for (int i = 0; i < opt_t; i++) {
        uint64_t range_curr_end = range_curr_start + range_len_thread - 1;
        if (remainder > 0) {
            range_curr_end++;
            remainder--;
        }

        ranges[i].start = range_curr_start;
        ranges[i].end = range_curr_end;

        range_curr_start = range_curr_end + 1;
    }

    //create the threads and pass in the corresponding range
    pthread_t tids[opt_t];
    for (int i = 0; i < opt_t; i++) {
        if (pthread_create(&tids[i], NULL, evaluate_range, &ranges[i])) {
            perror("pthread_create");
            status = EXIT_FAILURE;
        }

        if (opt_v) {
            printf("perfect: t%d searching [%lu,%lu]\n", i, ranges[i].start, ranges[i].end);
        }
    }

    //join the threads
    for (int i = 0; i < opt_t; i++) {
        if (tids[i]) {
            if (pthread_join(tids[i], NULL)) {
                perror("pthread_join");
                status = EXIT_FAILURE;
            }

            if (opt_v) {
                printf("perfect: t%d finishing\n", i);
            }
        }
    }   

    return status;
}

