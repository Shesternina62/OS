#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>


int main(int argc, char *argv[]) {
    if (argc < 2 || argc > 3) {
        fprintf(stderr, "Usage: %s pattern [file]\n", argv[0]);
        return 1;
    }

    const char *pattern = argv[1];
    FILE *f = NULL;

    if (argc == 3) {
        f = fopen(argv[2], "r");
        if (!f) {
            fprintf(stderr, "mygrep: cannot open '%s': %s\n", argv[2], strerror(errno));
            return 1;
        }
    } else {
        f = stdin;
    }

    char *line = NULL;
    size_t len = 0;
    ssize_t nread;

   
    while ((nread = getline(&line, &len, f)) != -1) {
        if (strstr(line, pattern) != NULL) {
            fputs(line, stdout);
        }
    }

    free(line);
    if (argc == 3 && f != NULL) fclose(f);

    return 0;
}


