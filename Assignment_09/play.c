#include "quiz.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

// convert a choice ('a' to 'd') to an array index (0 to 3)
int convert_to_index(char choice) {
    switch (choice)
    {
        case 'a':
            return 0;
        case 'b':
            return 1;
        case 'c':
            return 2;
        case 'd':
            return 3;
        default:
            return -1;  // invalid choice
    }
}

// swap two char pointers
void swap(char **a, char **b) 
{
    char *temp = *a;
    *a = *b;
    *b = temp;
}

// randomize the order of answers in the array
void randomize_answers(char *arr[4]) 
{
    srand(time(NULL));

    for (int i = 3; i > 0; i--) {
        int j = rand() % (i + 1);
        swap(&arr[i], &arr[j]);
    }
}

int play(quiz_t *quiz) 
{
    char *url = "https://opentdb.com/api.php?amount=1&category=18&type=multiple";

    // fetch the quiz question and anwers from the URL
    char *fetch_result = fetch(url);
    if (parse(quiz, fetch_result) == -1) {
        fprintf(stderr, "parse: failed!\n");
        return EXIT_FAILURE;
    }
    free(fetch_result);

    char ch = 'a';
    // print the quiz question and randomized answer choices
    printf("%s\n\n", quiz->question);
    randomize_answers(quiz->choices);
    for (size_t i = 0; i < 4; i++) {
        printf("[%c] %s\n", ch, quiz->choices[i]);
        ch++;
    }
    printf("\n");

    size_t points = 8;
    char choice;
    while (points != 1) {

        printf("(%ldpt) > ", points);
        // get user input for the answer choice
        if (scanf(" %c", &choice) == EOF) {
            // handle CTRL+D
            free(quiz->answer);
            free(quiz->question);
            for (size_t i = 0; i < 4; i++) {
                free(quiz->choices[i]);
            }

            // cover case where max is overcounted
            if (quiz->n != 1) {
                quiz->max -= 8;
            }
            printf("\n\nThanks for playing today.\n"
                "Your final score is %d/%d.\n", quiz->score, quiz->max);
            exit(EXIT_SUCCESS);
        }

        int index = convert_to_index(choice);
        if (index == -1) {
            printf("[%c] is not an option\n", choice);
            continue;
        }

        if (strcmp(quiz->answer, quiz->choices[index]) == 0) {
            quiz->score += points;
            printf("Congratulations, answer [%c] is correct. Your current score is %d/%d points.\n\n", choice, quiz->score, quiz->max);
            break;
        } else {
            if (points == 2) {
                printf("Answer [%c] is wrong. Your current score is %d/%d points.\n\n", choice, quiz->score, quiz->max);
            } else {
                printf("Answer [%c] is wrong, try again.\n", choice);
            }
        }

        points /= 2;
    }

    quiz->n++;
    quiz->max += 8;

    return EXIT_SUCCESS;
}
