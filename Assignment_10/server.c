#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <getopt.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <string.h>
#include <pthread.h>
#include <signal.h>

#include "lib/tcp.h"
#include "gug.c"

// Structure to hold information about listening sockets
typedef struct {
    char *address;
    int fd;
} listen_t;

// Function to handle default mode (no threading or forking)
void mode_default(int cfd) {
    // Call the main application logic
    gug(cfd);
    (void) close(cfd);
}

void *mode_thread_go(void *arg) {
    int cfd = (intptr_t) arg;

    // Call default mode within the thread
    mode_default(cfd);
    return NULL;
}

// Function to create a thread and execute the default mode
void mode_thread(int cfd) {
    int rc;
    pthread_t tid;

    // Create a thread and pass the client socket to it
    rc = pthread_create(&tid, NULL, mode_thread_go, (void *) (intptr_t) cfd);
    if (rc != 0) {
        fprintf(stderr, "pthread_create: failed\n");
    }
}

// Function to fork and execute the default mode
void mode_fork(int cfd) {
    pid_t pid;
    pid = fork();

    if (pid == -1) {
        fprintf(stderr, "fork: failed!\n");
        exit(EXIT_FAILURE);
    }

    // In the child process, execute the default mode and exit
    if (pid == 0) {
        mode_default(cfd);
        exit(EXIT_SUCCESS);
    }
    (void) close(cfd);
}

static void mainloop(listen_t *listeners, void (*mode)(int)) {
    while (1) {
        fd_set fdset;
        FD_ZERO(&fdset);
        int maxfd = 0;

        // Iterate through the listening sockets and add them to the fdset
        for (listen_t *l = listeners; l->address; l++) {
            if (l->fd > 0) {
                FD_SET(l->fd, &fdset);
                maxfd = (l->fd > maxfd) ? l->fd : maxfd;
            }
        }

        // If there are no active sockets, break out of the loop
        if (maxfd == 0) {
            break;
        }

        // Use select to wait for activity on the listening sockets
        if (select(maxfd + 1, &fdset, NULL, NULL, NULL) == -1) {
            (void) fprintf(stderr, "select: failed!\n");
            exit(EXIT_FAILURE);
        }

        // Iterate through the listening sockets and handle incoming connections
        for (listen_t *l = listeners; l->address; l++) {
            if ((l->fd > 0) && (FD_ISSET(l->fd, &fdset))) {
                // Accept incoming connection
                int cfd = tcp_accept(l->fd);
                if (cfd == -1) {
                    (void) fprintf(stderr, "accept: failed!\n");
                }

                // Call the specified mode for handling the connection
                mode(cfd);
            }
        }
    }
}

int main(int argc, char *argv[]) {
    // Define listening interfaces
    listen_t *iface, interfaces[] = {
        { .address = "0.0.0.0" },
        { .address = "::" },
        { .address = NULL }
    };

    int opt, port = 12345, ignore[] = { SIGPIPE, SIGCHLD, 0 };;
    void (*mode)(int) = mode_default;
    opterr = 0;

    // Parse command line options
    while ((opt = getopt(argc, argv, "tfp:")) != -1) {
        switch (opt) {
        case 't':
            mode = mode_thread;
            break;
        case 'f':
            mode = mode_fork;
            break;
        case 'p':
            port = atoi(optarg);
            break;
        default:
            fprintf(stderr, "Usage: %s [-t] [-f] [-p port_number]!\n", argv[0]);
            break;
        }
    }

    for (int i = 0; ignore[i]; i++) {
        if (signal(ignore[i], SIG_IGN) == SIG_ERR) {
            fprintf(stderr, "signal: failed!\n");
            return EXIT_FAILURE;
        }
    }

    // Initialize listening sockets
    for (iface = interfaces; iface->address; iface++) {
        char port_char[5];
        sprintf(port_char, "%d", port);

        // Attempt to bind and listen on the specified interface and port
        iface->fd = tcp_listen(iface->address, port_char);
        if (iface->fd == -1) {
            fprintf(stderr, "listening on %s port %d failed!\n", iface->address, port);
        }
    }

    mainloop(interfaces, mode);

    return EXIT_SUCCESS;
}
