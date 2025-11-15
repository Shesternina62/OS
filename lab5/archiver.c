#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <utime.h>
#include <time.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <errno.h>

#define BUF_SIZE 1024

struct FileHeader {
    unsigned short name_len;  // длина имени файла
    off_t file_size;          // размер файла
    mode_t perm;              // права доступа
    time_t mod_time;          // время модификации
};

void perm_to_string(mode_t perm, char *str) {
    str[0] = (perm & S_IRUSR) ? 'r' : '-';
    str[1] = (perm & S_IWUSR) ? 'w' : '-';
    str[2] = (perm & S_IXUSR) ? 'x' : '-';
    str[3] = (perm & S_IRGRP) ? 'r' : '-';
    str[4] = (perm & S_IWGRP) ? 'w' : '-';
    str[5] = (perm & S_IXGRP) ? 'x' : '-';
    str[6] = (perm & S_IROTH) ? 'r' : '-';
    str[7] = (perm & S_IWOTH) ? 'w' : '-';
    str[8] = (perm & S_IXOTH) ? 'x' : '-';
    str[9] = '\0';
}

void format_size(off_t size, char *buf) {
    if (size < 1024)
        sprintf(buf, "%lld B", (long long)size);
    else if (size < 1024*1024)
        sprintf(buf, "%.2f KB", size / 1024.0);
    else
        sprintf(buf, "%.2f MB", size / (1024.0*1024.0));
}

// запись заголовка файла в архив
int write_header(int fd_archive, struct FileHeader *header, const char *filename) {
    if (write(fd_archive, header, sizeof(*header)) != sizeof(*header)) {
        perror("write header");
        return -1;
    }
    if (write(fd_archive, filename, header->name_len) != header->name_len) {
        perror("write filename");
        return -1;
    }
    return 0;
}
// копирование данных файла в архив
int write_file_data(int fd_archive, int fd_file, off_t size) {
    char buf[BUF_SIZE];
    off_t remaining = size;
    while (remaining > 0) {
        ssize_t to_read = remaining > BUF_SIZE ? BUF_SIZE : remaining;
        ssize_t r = read(fd_file, buf, to_read);
        if (r <= 0) break;
        if (write(fd_archive, buf, r) != r) perror("write data");
        remaining -= r;
    }
    return 0;
}
// добавление одного файла в архив
int add_file(const char *archive, const char *filename) {
    int fd_archive = open(archive, O_WRONLY | O_CREAT | O_APPEND, 0644);
    if (fd_archive < 0) { perror("open archive"); return -1; }

    int fd_file = open(filename, O_RDONLY);
    if (fd_file < 0) { perror("open file"); close(fd_archive); return -1; }

    struct stat st;
    if (fstat(fd_file, &st) < 0) { perror("fstat"); close(fd_file); close(fd_archive); return -1; }

    struct FileHeader header;
    header.name_len = strlen(filename);
    header.file_size = st.st_size;
    header.perm = st.st_mode;
    header.mod_time = st.st_mtime;

    write_header(fd_archive, &header, filename);
    write_file_data(fd_archive, fd_file, header.file_size);

    close(fd_file);
    close(fd_archive);

    printf("[INFO] Added file: %s (%lld bytes)\n", filename, (long long)header.file_size);
    return 0;
}
// добавление нескольких файлов
void add_files(const char *archive, int count, char *files[]) {
    for (int i = 0; i < count; i++) {
        add_file(archive, files[i]);
    }
}
// просмотр содержимого архива
void list_archive(const char *archive) {
    int fd = open(archive, O_RDONLY);
    if (fd < 0) { perror("open archive"); return; }

    struct FileHeader header;
    char perm_str[10], size_str[20];
    while (read(fd, &header, sizeof(header)) == sizeof(header)) {
        char name[header.name_len + 1];
        if (read(fd, name, header.name_len) != header.name_len) break;
        name[header.name_len] = '\0';

        perm_to_string(header.perm, perm_str);
        format_size(header.file_size, size_str);

        char timebuf[64];
        struct tm *tm_info = localtime(&header.mod_time);
        strftime(timebuf, sizeof(timebuf), "%Y-%m-%d %H:%M:%S", tm_info);

        printf("File: %s | Size: %s | Perm: %s | Modified: %s\n",
               name, size_str, perm_str, timebuf);

        lseek(fd, header.file_size, SEEK_CUR);
    }

    close(fd);
}
// восстановление атрибутов файла
void restore_attributes(const char *filename, struct FileHeader *header) {
    chmod(filename, header->perm);
    struct utimbuf t;
    t.actime = header->mod_time;
    t.modtime = header->mod_time;
    utime(filename, &t);
}
// извлечение файла из архива
void extract_file(const char *archive, const char *filename) {
    int fd = open(archive, O_RDONLY);
    if (fd < 0) { perror("open archive"); return; }

    int fd_tmp = open("tmp_archive", O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (fd_tmp < 0) { perror("open tmp archive"); close(fd); return; }

    struct FileHeader header;
    int found = 0;
    char buf[BUF_SIZE];

    while (read(fd, &header, sizeof(header)) == sizeof(header)) {
        char *name = malloc(header.name_len + 1);
        if (read(fd, name, header.name_len) != header.name_len) { free(name); break; }
        name[header.name_len] = '\0';

        if (strcmp(name, filename) == 0 && !found) {
            found = 1;
            int out = open(filename, O_WRONLY | O_CREAT | O_TRUNC, header.perm);
            if (out < 0) { perror("create file"); free(name); continue; }

            off_t remaining = header.file_size;
            while (remaining > 0) {
                ssize_t to_read = remaining > BUF_SIZE ? BUF_SIZE : remaining;
                ssize_t r = read(fd, buf, to_read);
                if (r <= 0) break;
                if (write(out, buf, r) != r) perror("write file");
                remaining -= r;
            }
            close(out);

            restore_attributes(filename, &header);
            printf("[INFO] Extracted file: %s\n", filename);
        } else {
            write(fd_tmp, &header, sizeof(header));
            write(fd_tmp, name, header.name_len);

            off_t remaining = header.file_size;
            while (remaining > 0) {
                ssize_t to_read = remaining > BUF_SIZE ? BUF_SIZE : remaining;
                ssize_t r = read(fd, buf, to_read);
                if (r <= 0) break;
                if (write(fd_tmp, buf, r) != r) perror("write tmp data");
                remaining -= r;
            }
        }

        free(name);
    }

    close(fd);
    close(fd_tmp);
    rename("tmp_archive", archive);

    if (!found) printf("[WARN] File '%s' not found in archive.\n", filename);
}

// удаление файла из архива
void remove_file(const char *archive, const char *filename) {
    int fd = open(archive, O_RDONLY);
    if (fd < 0) { perror("open archive"); return; }

    int fd_tmp = open("tmp_archive", O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (fd_tmp < 0) { perror("open tmp archive"); close(fd); return; }

    struct FileHeader header;
    int found = 0;
    char buf[BUF_SIZE];

    while (read(fd, &header, sizeof(header)) == sizeof(header)) {
        char *name = malloc(header.name_len + 1);
        if (read(fd, name, header.name_len) != header.name_len) { free(name); break; }
        name[header.name_len] = '\0';

        if (strcmp(name, filename) == 0 && !found) {
            found = 1;
            lseek(fd, header.file_size, SEEK_CUR);
            printf("[INFO] Removed file: %s\n", filename);
        } else {
            write(fd_tmp, &header, sizeof(header));
            write(fd_tmp, name, header.name_len);

            off_t remaining = header.file_size;
            while (remaining > 0) {
                ssize_t to_read = remaining > BUF_SIZE ? BUF_SIZE : remaining;
                ssize_t r = read(fd, buf, to_read);
                if (r <= 0) break;
                if (write(fd_tmp, buf, r) != r) perror("write tmp data");
                remaining -= r;
            }
        }

        free(name);
    }

    close(fd);
    close(fd_tmp);
    rename("tmp_archive", archive);

    if (!found) printf("[WARN] File '%s' not found in archive.\n", filename);
}

void print_help(const char *prog) {
    printf("Usage:\n");
    printf("%s archive -i file1 [file2 ...]   Add files to archive\n", prog);
    printf("%s archive -e file1 [file2 ...]   Extract files from archive\n", prog);
    printf("%s archive -r file1 [file2 ...]   Remove files from archive\n", prog);
    printf("%s archive -s                     Show archive contents\n", prog);
    printf("%s -h                             Show this help\n", prog);
}

int main(int argc, char *argv[]) {
    if (argc < 2) { print_help(argv[0]); return 1; }

    if (strcmp(argv[1], "-h") == 0) { print_help(argv[0]); return 0; }

    if (argc < 3) { print_help(argv[0]); return 1; }

    const char *archive = argv[1];
    const char *flag = argv[2];

    if ((strcmp(flag, "-i") == 0 || strcmp(flag, "--input") == 0) && argc >= 4)
        add_files(archive, argc - 3, &argv[3]);
    else if ((strcmp(flag, "-e") == 0 || strcmp(flag, "--extract") == 0) && argc >= 4)
        for (int i = 3; i < argc; i++) extract_file(archive, argv[i]);
    else if ((strcmp(flag, "-r") == 0 || strcmp(flag, "--remove") == 0) && argc >= 4)
        for (int i = 3; i < argc; i++) remove_file(archive, argv[i]);
    else if (strcmp(flag, "-s") == 0 || strcmp(flag, "--stat") == 0)
        list_archive(archive);
    else
        print_help(argv[0]);

    return 0;
}
