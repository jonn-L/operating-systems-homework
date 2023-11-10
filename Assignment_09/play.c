#include "quiz.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

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
            return -1;
    }
}

void swap(char **a, char **b) 
{
    char *temp = *a;
    *a = *b;
    *b = temp;
}

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

    char *fetch_result = fetch(url);
    if (parse(quiz, fetch_result) == -1) {
        fprintf(stderr, "parse: failed!\n");
        return EXIT_FAILURE;
    }
    free(fetch_result);

    char ch = 'a';
    printf("%s\n\n", quiz->question);
    char *correct_choice = quiz->choices[0];

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
        scanf(" %c", &choice);

        int index = convert_to_index(choice);
        if (index == -1) {
            printf("[%c] is not an option\n", choice);
            continue;
        }

        if (strcmp(correct_choice, quiz->choices[index]) == 0) {
            quiz->score += points;
            printf("Congratulation, answer [%c] is correct. Your current score is %d/%d points.\n\n", choice, quiz->score, quiz->max);
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
