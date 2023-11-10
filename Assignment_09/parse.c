#include "quiz.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <jansson.h>

// Function to parse quiz data from a JSON string
int parse(quiz_t *quiz, char *msg) 
{
    json_t *root;
    json_error_t error;

    root = json_loads(msg, 0, &error);

    if (root == NULL) {
        fprintf(stderr, "json_loads: failed!\n");
        return EXIT_FAILURE;
    }

    json_t *results_array = json_object_get(root, "results");
    if (!json_array_size(results_array)) {
        fprintf(stderr, "json_object_get: no 'results' found!\n");
        return EXIT_FAILURE;
    }

    json_t *results = json_array_get(results_array, 0);

    // parse the question
    const char *question = json_string_value(json_object_get(results, "question"));
    if (question == NULL) {
        fprintf(stderr, "json_object_get: no 'question' found!\n");
        return EXIT_FAILURE;
    }
    quiz->question = strdup(question);
    if (quiz->question == NULL) {
        fprintf(stderr, "strdup: failed!\n");
        return EXIT_FAILURE;
    }

    // parse the correct answer
    const char *correct_answer = json_string_value(json_object_get(results, "correct_answer"));
    if (correct_answer == NULL) {
        fprintf(stderr, "json_object_get: no 'correct answer' found!\n");
        return EXIT_FAILURE;
    }
    quiz->answer = strdup(correct_answer);
    if (quiz->answer == NULL) {
        fprintf(stderr, "strdup: failed!\n");
        return EXIT_FAILURE;
    }
    quiz->choices[0] = strdup(correct_answer);
    if (quiz->choices[0] == NULL) {
        fprintf(stderr, "strdup: failed!\n");
        return EXIT_FAILURE;
    }

    // parse incorrect answers
    json_t *incorrect_answers = json_object_get(results, "incorrect_answers");
    if (!json_array_size(incorrect_answers)) {
        fprintf(stderr, "json_object_get: no 'incorrect_answers' found\n");
        return EXIT_FAILURE;
    }
    // loop through the incorrect answers
    for (size_t i = 0; i < 3; i++) {
        const char *incorrect_answer = json_string_value(json_array_get(incorrect_answers, i));
        if (incorrect_answer == NULL) {
            fprintf(stderr, "json_array_get: no 'incorrect[%ld] answer' found!\n", i);
            return EXIT_FAILURE;
        }
        // store incorrect answers
        quiz->choices[i+1] = strdup(incorrect_answer);
        if (quiz->choices[i+1] == NULL) {
            fprintf(stderr, "strdup: failed\n");
            return EXIT_FAILURE;
        }
    }

    // release the allocated JSON object
    json_decref(root);

    return EXIT_SUCCESS;
}
