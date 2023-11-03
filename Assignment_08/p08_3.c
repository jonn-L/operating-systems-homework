#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>

// function to count the number of lines
int count_lines(char *content, int size) {
    int lines = 0;
    for (int i = 0; i < size; i++) {
        if (content[i] == '\n') {
            lines++;
        }
    }

    // count the last line if the last character is not a '\n'
    if (content[size - 1] != '\n') {
        lines++;
    }

    return lines;
}

// function to count the number of words
int count_words(char *content, int size) {
    int words = 0;
    int wasSpace = 0;     // boolean to check whether the previous character was a space

    for (int i = 0; i < size; i++) {
        if (isspace(content[i])) {
            wasSpace = 1;
        }
        else {
            if ((wasSpace == 1) || (i == 0)) {
                words++;
            }
            wasSpace = 0;
        }
    }

    return words;
}

// function to read using mmap
int read_mmap(int fd, char *file, int size) {
    char *file_content = mmap(NULL, size, PROT_READ, MAP_PRIVATE, fd, 0);
    if (file_content == MAP_FAILED) {
        perror("mmap");
        return -1;
    }

    int lines = count_lines(file_content, size);
    int words = count_words(file_content, size);

    printf("%d\t%d\t%d\t%s\n", lines, words, size, file);

    if (munmap(file_content, size) == -1) {
        perror("munmap");
        return -1;
    }

    return 0;
}

// function to read using standard I/O
int read_io(int fd, char *file, int size) {
    char buffer[size];

    ssize_t bytes = read(fd, buffer, size);
    if (bytes == -1) {
        perror("read");
        return -1;
    }

    int lines = count_lines(buffer, bytes);
    int words = count_words(buffer, bytes);

    printf("%d\t%d\t%ld\t%s\n", lines, words, bytes, file);

    return 0;
}

int main(int argc, char *argv[]) {
    // if no files are provided, read from STDIN
    if (argc == 1) {
        // get stdin stats
        struct stat stdin_stats;
        if (fstat(STDIN_FILENO, &stdin_stats) == -1) {
            perror("fstat");
            fprintf(stderr, "Error getting stdin size!\n");
            return EXIT_FAILURE;
        }

        // read from stdin using read_io function
        if (read_io(STDIN_FILENO, "STDIN", stdin_stats.st_size) == -1) {
            fprintf(stderr, "Error reading STDIN!\n");
            return EXIT_FAILURE;
        }
    }
    else {
        // loop through the files
        for (int i = 1; i < argc; i++) {
            int fd = open(argv[i], O_RDONLY);
            if (fd == -1) {
                fprintf(stderr, "Could not find %s!\n", argv[i]);
                return EXIT_FAILURE;
            }

            // get file stats
            struct stat file_stats;
            if (fstat(fd, &file_stats) == -1) {
                perror("fstat");
                return EXIT_FAILURE;
            }

            // handle empty files
            if (file_stats.st_size == 0) {
                printf("0\t0\t0\t%s\n", argv[i]);
                continue;
            }

            int read_result = 0;
            // check if the file is regular and use mmap, otherwise use standard I/O
            if (S_ISREG(file_stats.st_mode)) {
                read_result = read_mmap(fd, argv[i], file_stats.st_size);
            }
            else {
                read_result = read_io(fd, argv[i], file_stats.st_size);
            }

            // Handle errors while reading files    
            if (read_result == -1) {
                fprintf(stderr, "Error reading %s!\n", argv[i]);
                return EXIT_FAILURE;
            }
        }
    }

    return EXIT_SUCCESS;
}
