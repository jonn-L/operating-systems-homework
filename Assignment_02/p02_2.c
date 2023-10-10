#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <getopt.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <time.h>

enum { NS_PER_SECOND = 1000000000 };

//function to calculate the time past betweem t1 and t2 (t2 - t1)
void sub_timespec(struct timespec t1, struct timespec t2, struct timespec *td) {
    td->tv_nsec = t2.tv_nsec - t1.tv_nsec;
    td->tv_sec  = t2.tv_sec - t1.tv_sec;
    if (td->tv_sec > 0 && td->tv_nsec < 0)
    {
        td->tv_nsec += NS_PER_SECOND;
        td->tv_sec--;
    }
    else if (td->tv_sec < 0 && td->tv_nsec > 0)
    {
        td->tv_nsec -= NS_PER_SECOND;
        td->tv_sec++;
    }
}

int main(int argc, char *argv[]) {
    int opt, status = EXIT_SUCCESS;
    int opt_d = 5, opt_w = 0; //intialize with default values

    opterr = 0;
    while ((opt = getopt(argc, argv, "d:w:")) != -1) {
        switch (opt) {
        case 'd':
            opt_d = atoi(optarg);
            break;
        case 'w':
            opt_w = atoi(optarg);
            break;
        default:
            fprintf(stderr, "Usage: %s [-d seconds] [-w warmups] [command [arg]...]!\n", argv[0]);
            return EXIT_FAILURE;
            break;
        }
    }
    
    //run warmpus
    int stat;
    for (int i = 0; i < opt_w; i++) {
        pid_t pid_w = fork();
        
        if (pid_w == -1) {
            perror("fork");
            status = EXIT_FAILURE;
            continue;
        }

        if (pid_w == 0) {
            execvp(argv[optind], &argv[optind]);
            perror("execvp");
        }

        if (pid_w > 0) {
            if (waitpid(pid_w, &stat, 0) == -1) {
                perror("waitpid");
                status = EXIT_FAILURE;
            }
        }
    }

    int fails = 0, runs = 0;
    double secs_min = opt_d + 1, secs_max = -1, secs_avg, secs_total;

    struct timespec time_start, time_curr, time_past;
    //check when the runs start and save it into "time_start"
    if (clock_gettime(CLOCK_MONOTONIC, &time_start) != 0) {
        perror("clock_gettime");
        return EXIT_FAILURE;
    }

    while (1) {
        //check the current time and save it into "time_curr"
        if (clock_gettime(CLOCK_MONOTONIC, &time_curr) != 0) {
            perror("clock_gettitme");
            return EXIT_FAILURE;
        }
        //check how much time has passed
        sub_timespec(time_start, time_curr, &time_past);

        //if "opt_d" seconds have passed, break out of loop
        if ((int) time_past.tv_sec >= opt_d) {
            secs_total = time_past.tv_sec + (double) time_past.tv_nsec / NS_PER_SECOND;
            break;
        }

        //fork current process
        pid_t pid = fork();

        if (pid == -1) {
            perror("fork");
            fails++;
            status = EXIT_FAILURE;
            continue;
        }

        struct timespec time_start_execvp, time_end_execvp, time_past_execvp;

        //check start time of the current process
        if (clock_gettime(CLOCK_MONOTONIC, &time_start_execvp) != 0) {
            perror("clock_gettime");
            return EXIT_FAILURE;
        }

        //run it
        runs++;
        if (pid == 0) {
            execvp(argv[optind], &argv[optind]);
            perror("execvp");

            //if the run fails, adjust the number of fails accordingly
            fails++;
            status = EXIT_FAILURE;
        }  

        if (pid > 0) {
            if (waitpid(pid, &stat, 0) == -1) {
                perror("waitpid");
                status = EXIT_FAILURE;
                break;
            }
        }              

        //check the time after the process is finished
        if (clock_gettime(CLOCK_MONOTONIC, &time_end_execvp) != 0) {
            perror("clock_gettitme");
            return EXIT_FAILURE;
        }

        //calculate the time it took
        sub_timespec(time_start_execvp, time_end_execvp, &time_past_execvp);
        double secs_total_execvp = time_past_execvp.tv_sec + (double) time_past_execvp.tv_nsec / NS_PER_SECOND;

        //check if the shortest or longest run has changed
        if (secs_total_execvp > secs_max) {
            secs_max = secs_total_execvp;
        }
        if (secs_total_execvp < secs_min) {
            secs_min = secs_total_execvp;
        }
    }

    //print out results
    secs_avg = secs_total / runs;
    printf("Min:   %10.6lf seconds      Warmups: %d\n", secs_min, opt_w);
    printf("Avg:   %10.6lf seconds      Runs: %d\n", secs_avg, runs);
    printf("Max:   %10.6lf seconds      Fails: %d\n", secs_max, fails);
    printf("Total: %10.6lf seconds\n", secs_total);

    return status;
}