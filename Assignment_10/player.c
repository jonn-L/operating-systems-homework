#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "player.h"

player_t* player_new(void)
{
    player_t *new_player = malloc(sizeof(*new_player));
    if (new_player == NULL) {
        fprintf(stderr, "malloc: failed!\n");
        exit(EXIT_FAILURE);
    }

    new_player->finished = false;
    new_player->solved = 0;
    new_player->total = 0;

    chlng_t* new_chlng = chlng_new();
    new_player->chlng = new_chlng;

    return new_player;
}

void player_reset(player_t *p) 
{
    p->finished = false;
    p->solved = 0;
    p->total = 0;
    chlng_reset(p->chlng);
}

void player_del(player_t *p)
{
    player_reset(p);
    chlng_del(p->chlng);
    free(p);
} 

int player_fetch_chlng(player_t *p)
{
    if (chlng_fetch_text(p->chlng) == -1) {
        fprintf(stderr, "chlng_fetch_text: failed!\n");
        return EXIT_FAILURE;
    }

    if (chlng_hide_word(p->chlng) == -1) {
        fprintf(stderr, "chlng_hide_word: failed!\n");
        return EXIT_FAILURE; 
    }

    return EXIT_SUCCESS;
}

int player_get_greeting(char **msg) 
{
    char *greeting = "M: Guess the missing ____!\n"
                     "M: Send your guess in the form 'R: word\\r\\n'.\n";
    *msg = strdup(greeting);
    if (msg == NULL) {
        fprintf(stderr, "strdup: failed!\n");
        return EXIT_FAILURE;
    }

    return strlen(greeting);
}

int player_get_challenge(player_t *p, char **msg) 
{
    if (player_fetch_chlng(p) == -1) {
        fprintf(stderr, "player_fetch_chlng: failed!\n");
        return EXIT_FAILURE;
    }

    char *format = "C: %s";
    size_t length = snprintf(NULL, 0, format, p->chlng->text) + 1;
    char *new_msg = malloc(length);
    if (new_msg == NULL) {
        fprintf(stderr, "malloc: failed!\n");
        return EXIT_FAILURE; 
    }
    snprintf(new_msg, length, format, p->chlng->text);
    *msg = new_msg;

    if (msg == NULL) {
        fprintf(stderr, "strdup: failed!\n");
        return EXIT_FAILURE;
    }

    return strlen(*msg);
}

int player_post_challenge(player_t *p, char *guess, char **msg)
{
    
    p->total++;
    if (strcmp(guess, "Q: ") == 0) {
        char *format = "M: You mastered %d/%d challenges. Good bye!";
        size_t length = snprintf(NULL, 0, format, p->solved, p->total) + 1;
        char *new_msg = malloc(length);

        if (new_msg == NULL) {
            fprintf(stderr, "malloc: failed!\n");
            return EXIT_FAILURE; 
        }
        snprintf(new_msg, length, format, p->solved, p->total);
        *msg = new_msg;

        player_del(p);
    } else {
        if (strcmp(guess, p->chlng->word) == 0) {
            p->solved++;
            char *new_msg = "O: Congratulation - challenge passed!\n";
            *msg = strdup(new_msg);
        } else {
            const char *format = "F: Wrong guess '%s' - expected '%s'\n";
            
            size_t length = snprintf(NULL, 0, format, guess, p->chlng->word) + 1;
            char *new_msg = malloc(length);

            if (new_msg == NULL) {
                fprintf(stderr, "malloc: failed!\n");
                return EXIT_FAILURE; 
            }
            snprintf(new_msg, length, format, guess, p->chlng->word);
            *msg = new_msg;
        }
    }


    return EXIT_SUCCESS;
}
