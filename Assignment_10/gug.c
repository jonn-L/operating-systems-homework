#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "player.h"
#include "lib/tcp.h"

int gug(int cfd)
{
    player_t *p;
    char *msg;
    int rc;

    p = player_new();
    if (!p) {
        return EXIT_FAILURE;
    }

    // Get the greeting message from the player
    rc = player_get_greeting(&msg);
    if (rc > 0) {
        // Send the greeting message to the client
        if (tcp_write(cfd, msg, strlen(msg)) == -1) {
            // Clean up resources and return failure if writing to the client fails
            player_del(p);
            (void) free(msg);
            return EXIT_FAILURE;
        }
        (void) free(msg);
    }

    while (!(p->finished)) {
        char *line = NULL;
        size_t linecap = 0;

        // Get the challenge message from the player
        rc = player_get_challenge(p, &msg);
        if (rc > 0) {
            // Send the challenge message to the client
            if (tcp_write(cfd, msg, strlen(msg)) == -1) {
                // Clean up resources and return failure if writing to the client fails
                player_del(p);
                (void) free(msg);
                return EXIT_FAILURE;
            }            
            (void) free(msg);
        }

        // Read the client's response
        ssize_t bytes_read = getline(&line, &linecap, fdopen(cfd, "r"));
        if (bytes_read <= 0) {
            // If no bytes are read, post a default challenge message and break out of the loop
            rc = player_post_challenge(p, "Q: ", &msg);
            if (tcp_write(cfd, msg, strlen(msg)) == -1) {
                // Clean up resources and return failure if writing to the client fails
                player_del(p);
                (void) free(msg);
                return EXIT_FAILURE;
            }            
            break;
        }

        // Post the client's response as the player's answer
        rc = player_post_challenge(p, line, &msg);
        if (rc > 0) {
            // Send the response to the client
            if (tcp_write(cfd, msg, strlen(msg)) == -1) {
                // Clean up resources and return failure if writing to the client fails
                player_del(p);
                (void) free(msg);
                return EXIT_FAILURE;
            }                        
            (void) free(msg);
        }
        (void) free(line);
    }

    // Clean up resources and return success
    player_del(p);
    return EXIT_SUCCESS;
}
