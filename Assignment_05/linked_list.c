#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*
    just some helper structures and functions to handle the taxi stand
*/

typedef struct node {
    struct node *next;
    char *name;
} node;

void print_all(node *head) {
    node *curr = head;

    printf("taxi:\t"); 
    printf("[");
    int totalCharacters = 0;

    while (curr != NULL) {
        printf("%s ", curr->name);
        totalCharacters += strlen(curr->name) + 1;
        curr = curr->next;
    }

    printf("]");

    // Calculate the remaining characters to fill
    int remainingCharacters = 30 - totalCharacters;

    // Add padding spaces
    for (int i = 0; i < remainingCharacters; i++) {
        printf(" ");
    }
}

void print_travelers(node *head) {
    node *curr = head;
    while (curr != NULL) {
        if (curr->name[0] == 't') {
            printf("%s ", curr->name);
        }
        curr = curr->next;
    }
    printf("\n");
}

void print_longest_waiting_driver(node *head) {
    node *curr = head;
    while (curr != NULL) {
        if (curr->name[0] == 'd') {
            printf("%s\n", curr->name);
            break;
        }
        curr = curr->next;
    }
}

void add_person(node *head, char *name) {
    node *new_node = (node *) malloc(sizeof(node)); 
    new_node->name = malloc(strlen(name) + 1);
    strcpy(new_node->name, name);
    new_node->next = NULL;

    node *curr = head;
    while (curr->next != NULL) {
        curr = curr->next;
    }

    curr->next = new_node;
}

void delete_person(node *head, char *name) {
    if (head->next == NULL) {
        free(head->name);
        free(head);
        return;
    } 

    node *prev = head;
    node *curr = head->next;

    while (curr != NULL) {
        if (strcmp(curr->name, name) == 0) {
            prev->next = curr->next;
            free(curr->name);
            free(curr);
            return;
        }

        prev = curr;
        curr = curr->next;
    }
}
