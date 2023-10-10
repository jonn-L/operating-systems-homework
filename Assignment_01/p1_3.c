#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <getopt.h>
#include <string.h>

extern char **environ;

//function to print environment
void print_env() {
    char **env = environ;
    while (*env) {
        printf("%s\n", *env);
        env++;
    }
}

int main(int argc, char *argv[]) {
    int opt;
    int opt_u = 0, opt_v = 0;
    
    opterr = 0;
    while ((opt = getopt(argc, argv, "vu:")) != -1) {
        switch (opt) {
        case 'v':
            opt_v = 1;
            break;
        case 'u':
            if (optarg[0] == '-') {
                fprintf(stderr, "Invalid -u option argument: '%s'!\n", optarg);
                return EXIT_FAILURE;
            }
            opt_u = 1;
            if (unsetenv(optarg)) {
                perror("unsetenv");
                return EXIT_FAILURE;
            }
            break;
        default:
            fprintf(stderr, "Usage: %s [-v] [-u name] [name=value]... [command [arg]...]!\n", argv[0]);
            exit(EXIT_FAILURE);
            break;
        }
    }

    //called without any arguments
    if (argc == 1) {
        print_env();
    }

    //-v option is selected, print to "stderr" the removed environment variables
    if ((opt_v == 1) && (opt_u == 1)) {
        for (int i = 1; i < optind; i++) {
            if (!strcmp(argv[i], "-u")) {
                fprintf(stderr, "Removed %s!\n", argv[i+1]);
            }
        } 
    }

    //cycle through the non-option arguments
    for (int i = optind; i < argc; i++) {
        if (strchr(argv[i], '=')) {
            char *arg_copy = argv[i];
            char *name = strtok(arg_copy, "=");
            char *value = strtok(NULL, "=");

            if ((name == NULL) || (value == NULL)) {
                fprintf(stderr, "Invalid name=value pair!\n");
                return EXIT_FAILURE;
            }

            if (setenv(name, value, 1)) {
                perror("setenv");
                return EXIT_FAILURE;
            }

            if (opt_v == 1) {
                fprintf(stderr, "Added %s=%s pair!\n", name, value);
            }
        }
        else {
            //-v selected, print to "stderr" the name of the program to be executed
            if (opt_v == 1) {
                fprintf(stderr, "Executing %s!\n", argv[i]);
            }
                //run the program
                execvp(argv[i], &argv[i]);
                perror("execvp");
                return EXIT_FAILURE;
        }
    }

    //called with only a sequence of "name=value" pairs
    if ((opt_u == 0) && (opt_v == 0)) {
        print_env();
    }

    return EXIT_SUCCESS;
}