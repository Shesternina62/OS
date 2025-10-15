#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <errno.h>

void print_usg() {
    printf("использование: ./mychmod <mode> <file>\n");
    printf("примеры:\n");
    printf("  ./mychmod 755 file.txt\n");
    printf("  ./mychmod u+r file.txt\n");
    printf("  ./mychmod g-w file.txt\n");
    printf("  ./mychmod a+rw file.txt\n");
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        print_usg();
        return 1;
    }

    char *mode_str = argv[1];
    char *filename = argv[2];
    struct stat st;

    //текущие права файла
    if (stat(filename, &st) == -1) {
        perror("ошибка stat");
        return 1;
    }

    mode_t mode = st.st_mode;

    if (mode_str[0] >= '0' && mode_str[0] <= '9') {
        char *endptr;
        long new_mode = strtol(mode_str, &endptr, 8);
        if (*endptr != '\0' || new_mode < 0 || new_mode > 0777) {
            fprintf(stderr, "неверный числовой режим: %s\n", mode_str);
            return 1;
        }
        if (chmod(filename, (mode_t)new_mode) == -1) {
            perror("ошибка chmod");
            return 1;
        }
        printf("права файла %s изменены на %s (восьмерично)\n", filename, mode_str);
        return 0;
    }

    int who = 0;
    int op = 0;
    int perm = 0;

    for (int i = 0; mode_str[i]; i++) {
        char c = mode_str[i];
        if (c == 'u') who |= S_IRWXU;
        else if (c == 'g') who |= S_IRWXG;
        else if (c == 'o') who |= S_IRWXO;
        else if (c == 'a') who |= (S_IRWXU | S_IRWXG | S_IRWXO);
        else if (c == '+' || c == '-') op = c;
        else if (c == 'r') perm |= (S_IRUSR | S_IRGRP | S_IROTH);
        else if (c == 'w') perm |= (S_IWUSR | S_IWGRP | S_IWOTH);
        else if (c == 'x') perm |= (S_IXUSR | S_IXGRP | S_IXOTH);
    }

    if (who == 0) who = (S_IRWXU | S_IRWXG | S_IRWXO);

    //применяем нашу операцию
    if (op == '+') {
        if (who & S_IRWXU) mode |= (perm & S_IRWXU);
        if (who & S_IRWXG) mode |= (perm & S_IRWXG);
        if (who & S_IRWXO) mode |= (perm & S_IRWXO);
    } else if (op == '-') {
        if (who & S_IRWXU) mode &= ~(perm & S_IRWXU);
        if (who & S_IRWXG) mode &= ~(perm & S_IRWXG);
        if (who & S_IRWXO) mode &= ~(perm & S_IRWXO);
    } else {
        fprintf(stderr, "ошибка: не указана нужная операция\n");
        return 1;
    }

    // задействуем chmod
    if (chmod(filename, mode) == -1) {
        perror("ошибка chmod");
        return 1;
    }

    printf("права файла %s успешно изменены (%s)\n", filename, mode_str);
    return 0;
}
