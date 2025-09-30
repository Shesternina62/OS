#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <errno.h>

int main(int argc, char *argv[]) {
    bool flag_n = false;  // нумерация всех строк
    bool flag_b = false;  // нумерация только непустых строк
    bool flag_E = false;  // символ $ в конце строки

    int file_start = 1;

    
    for (int i = 1; i < argc; i++) {
        if (argv[i][0] == '-') {
            for (int j = 1; argv[i][j] != '\0'; j++) {
                if (argv[i][j] == 'n') flag_n = true;
                else if (argv[i][j] == 'b') flag_b = true;
                else if (argv[i][j] == 'E') flag_E = true;
                else {
                    fprintf(stderr, "mycat: неизвестный флаг -%c\n", argv[i][j]);
                    return 1;
                }
            }
            file_start++;
        } else break;
    }

    
    if (flag_b) flag_n = false;

    long long line_num_all = 1;     
    long long line_num_nonempty = 1; 

    
    if (file_start >= argc) {
        argv[file_start] = "-";
        argc++;
    }

    for (int i = file_start; i < argc; i++) {
        FILE *f;
        if (strcmp(argv[i], "-") == 0) f = stdin;
        else {
            f = fopen(argv[i], "r");
            if (!f) {
                fprintf(stderr, "mycat: не могу открыть %s: %s\n",
                        argv[i], strerror(errno));
                continue;
            }
        }

        char *line = NULL;
        size_t len = 0;
        ssize_t nread;

        while ((nread = getline(&line, &len, f)) != -1) {
            bool has_newline = (nread > 0 && line[nread - 1] == '\n');
            if (has_newline) line[nread - 1] = '\0';

            if (flag_n) {
                printf("%6lld\t", line_num_all++);
            } else if (flag_b && line[0] != '\0') {
                printf("%6lld\t", line_num_nonempty++);
            }

            printf("%s", line);
            if (flag_E) printf("$");
            if (has_newline) putchar('\n');
        }

        free(line);
        if (f != stdin) fclose(f);
    }

    return 0;
}
