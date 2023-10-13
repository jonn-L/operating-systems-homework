#include <pthread.h>
#include <getopt.h>
#include "linked_list.c"
#include <time.h>

#define _POSIX_C_SOURCE 200809L

node *taxi_stand = NULL;

typedef struct {
    unsigned int counter;   // keep count of the number of threads
    unsigned int waiting_travelers;
    unsigned int waiting_drivers;
    pthread_mutex_t mutex;
    pthread_cond_t cond;
} counter_t;

// Function to shuffle the threads 
void shuffle(int *array, int size) {
    srand(time(NULL));
    for (int i = size - 1; i > 0; i--) {
        int j = rand() % (i + 1);
        int temp = array[i];
        array[i] = array[j];
        array[j] = temp;
    }
}

static void *stand_visit_traveler(void *arg) {
    counter_t *c = (counter_t *) arg;

    (void) pthread_mutex_lock(&c->mutex);

    //setup name for person
    char *person_name;
    int length = snprintf(NULL, 0, "t%d", c->counter);
    person_name = (char *) malloc(length + 1);
    snprintf(person_name, length + 1, "t%d", c->counter);

    print_all(taxi_stand);
    printf("%s entering\n", person_name);

    add_person(taxi_stand, person_name);

    c->counter++;
    c->waiting_travelers++;

    // wait for drivers
    while ((c->waiting_drivers == 0) && (c->waiting_travelers != 0)) {
        print_all(taxi_stand);
        printf("%s waiting...\n", person_name);

        (void) pthread_cond_wait(&c->cond, &c->mutex);

        print_all(taxi_stand);
        printf("...%s waking up\n", person_name);
    }

    // case where the traveler needs to choose the longest waiting driver
    if (c->waiting_travelers == 1) {
        c->waiting_travelers = 0;
        c->waiting_drivers--;

        print_all(taxi_stand);
        printf("%s entering driver ", person_name);
        print_longest_waiting_driver(taxi_stand);
    }

    print_all(taxi_stand);
    printf("%s leaving\n", person_name);
    delete_person(taxi_stand, person_name);

    (void) pthread_mutex_unlock(&c->mutex);
    (void) pthread_cond_signal(&c->cond);

    return NULL;
}  

static void *stand_visit_driver(void *arg) {
    counter_t *c = (counter_t *) arg;

    (void) pthread_mutex_lock(&c->mutex);

    // setup name for person
    char *person_name;
    int length = snprintf(NULL, 0, "d%d", c->counter);
    person_name = (char *) malloc(length + 1);
    snprintf(person_name, length + 1, "d%d", c->counter);

    print_all(taxi_stand);
    printf("%s entering\n", person_name);

    add_person(taxi_stand, person_name);

    c->counter++;
    c->waiting_drivers++;

    // wait for travelers
    while ((c->waiting_travelers == 0) && (c->waiting_drivers != 0)) {
        print_all(taxi_stand);
        printf("%s waiting...\n", person_name);

        (void) pthread_cond_wait(&c->cond, &c->mutex);

        print_all(taxi_stand);
        printf("...%s waking up\n", person_name);
    }

    // case where driver picks up all travelers at the taxi stand
    if (c->waiting_drivers == 1) {
        c->waiting_drivers--;
        c->waiting_travelers = 0;    
        
        print_all(taxi_stand);
        printf("%s picking traveler ", person_name);
        print_travelers(taxi_stand);
    }

    print_all(taxi_stand);
    printf("%s leaving\n", person_name);
    delete_person(taxi_stand, person_name);

    (void) pthread_mutex_unlock(&c->mutex);
    (void) pthread_cond_signal(&c->cond);

    return NULL;
}

int main(int argc, char *argv[]) {
    unsigned int opt_t = 1, opt_d = 1;
    int opt, status = EXIT_SUCCESS;

    opterr = 0;
    while ((opt = getopt(argc, argv, "t:d:")) != -1) {
        switch (opt) {
            case 't':
                opt_t = atoi(optarg);
                break;
            case 'd':
                opt_d = atoi(optarg);
                break;
            default:
                fprintf(stderr, "Usage: %s [-t travelers] [-d drivers]!\n", argv[0]);
                break;
        }
    }


    // edge cases for invalid input
    if (opt_t < 1) {
        fprintf(stderr, "Not a valid number of travelers: 't' must be larger than or equal to 1!");
    }
    if (opt_d < 1) {
        fprintf(stderr, "Not a valid number of drivers: 'd' must be larger than or equal to 1!");
    }


    //intialize the threads array, the cntr, and the taxi_stand linked list
    pthread_t tids[opt_t + opt_d];
    counter_t cnter = { .counter = 0, .waiting_travelers = 0, .waiting_drivers = 0, 
                        .mutex = PTHREAD_MUTEX_INITIALIZER, .cond = PTHREAD_COND_INITIALIZER };
    taxi_stand = (node *) malloc(sizeof(node));
    taxi_stand->name = (char *) malloc(1);
    taxi_stand->name[0] = '\0';
    taxi_stand->next = NULL;

    /*
        shuffle the order of the threads so that cases where a driver or a 
        traveler is stuck at the end are less likely
    */
    int total_threads = opt_t + opt_d;
    int *thread_indices = malloc(total_threads * sizeof(int));
    for (int i = 0; i < total_threads; i++) {
        thread_indices[i] = i;
    }
    shuffle(thread_indices, total_threads);

    for (int i = 0; i < total_threads; i++) {
        int idx = thread_indices[i];
        if (idx < opt_t) {
            // create traveler thread
            if (pthread_create(&tids[idx], NULL, stand_visit_traveler, &cnter)) {
                perror("pthread_create");
                status = EXIT_FAILURE;
            }
        } else {
            // create driver thread
            if (pthread_create(&tids[idx], NULL, stand_visit_driver, &cnter)) {
                perror("pthread_create");
                status = EXIT_FAILURE;
            }
        }
    }

    for (int i = 0; i < (opt_t + opt_d); i++) {
        if (pthread_join(tids[i], NULL)) {
            perror("pthread_join");
            status = EXIT_FAILURE;
        }
    }

    free(thread_indices);
    return status;
}

