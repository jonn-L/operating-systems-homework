#define _DEFAULT_SOURCE

#include <string.h>
#include <stdio.h>
#include <stdlib.h>

/*
* Count the number of characters in string that are an element of the
* character set delim. Returns 0 if none of the characters in string
* is in the character set delim or the string is empty.
*/

size_t strcnt(const char *string, const char *delim)
{
    size_t cnt = 0;

    for (const char *s = string; *s; s++) {
        for (const char *d = delim; *d; d++) {
            if (*s == *d) {
                cnt++;
                break;
            }
        }
    }
    return cnt;
}

/*
* Split the string whenever a character appears that is in the
* character set delim. Return a NULL terminated vector of pointers to
* the sub-strings.
*/

char ** strsplit(char **string, const char *delim)
{
    char *token;
    size_t cnt = strcnt(*string, delim);

    char **splitv = calloc((cnt + 2), sizeof(char *));

    if (splitv) {
        for (int i = 0; (token = strsep(string, delim)); i++) {
            splitv[i] = token;
        }
    }
    splitv[cnt+1] = NULL;
    return splitv;
}

int main(int argc, char *argv[])
{
    for (int i = 1; i < argc; i++) {
        char **splitv = strsplit(&argv[i], " ");
        if (splitv) {
            for (int j = 0; splitv[j]; j++) {
                (void) puts(splitv[j]);
            }
            (void) free(splitv);
        }
    }

    return EXIT_SUCCESS;
}

/*
* a)
* 1. When allocating the memory for "splitv" in the "strsplit" function, it 
* is reserving only "char" bytes, which is not enough in the case that one of 
* the token is a string.
* 2. When trying to print the split string in the main function, the program
* runs into a segmentation fault due to it trying access and print an element
* that does not exist in splitv.
* 
* b)
* 1. All is needed is to allocate "char *" instead of "char".
* 2. A NULL sentinel should be added to "splitv" such that when the program 
* attemps to print it in the "main" function it stops before trying to access 
* nonexistent a nonexistent element, similiar to how "\0' works in strings.
*
* CHANGES:
* line 39: char **splitv = calloc(cnt +2, sizeof(char *)); 
* line 46: splitv[cnt+1] = NULL;
*/