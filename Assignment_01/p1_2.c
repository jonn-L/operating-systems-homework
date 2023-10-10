#include <stdlib.h>
#include <string.h>
#include <stdio.h>

char *strndup(const char *s, size_t n) //*s: data segment; n: stack segment
{
    char *p = NULL; //*p: stack segment

    if (s) {
        size_t len = strlen(s);

        if (n < len) {
            len = n;
        }

        //allocated memory (len+1 bytes): heap segment
        p = (char *) malloc(len+1);

        if (p) {
            strncpy(p, s, len);
        }
    }

    return p;
}

int main(void)
{
    static char m[] = "Hello World!"; //data segment
    size_t len = strlen(m); //stack segment

    for (size_t n = 1; n <= len; n++) { //n: stack segment

        char *p = strndup(m, n); //*p: stack segment

        if (!p) {
            perror("strndup"); //"strndup": data segment
            return EXIT_FAILURE;
        }

        if (puts(p) == EOF) {
            perror("puts"); //"puts": data segment
            return EXIT_FAILURE;
        }
        
        free(p);
    }

    if (fflush(stdout) == EOF) {
        perror("fflush"); //"fflush": data segment
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}