#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

void list_directory(const char *path) {
    struct dirent *entry;
    struct stat file_stat;
    char full_path[PATH_MAX];

    DIR *dir = opendir(path);
    if (dir == NULL) {
        perror("opendir");
        return;
    }

    printf("Contents of directory: %s\n", path);
    while ((entry = readdir(dir)) != NULL) {
        if (entry->d_name[0] == '.')  // Skip hidden files
            continue;

        snprintf(full_path, sizeof(full_path), "%s/%s", path, entry->d_name);
        if (stat(full_path, &file_stat) == -1) {
            perror("stat");
            continue;
        }

        printf("%-30s Block Address: %lu\n", entry->d_name, file_stat.st_blocks);
    }

    closedir(dir);
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <directory> [directory ...]\n", argv[0]);
        return EXIT_FAILURE;
    }

    for (int i = 1; i < argc; i++) {
        list_directory(argv[i]);
        printf("\n");
    }

    return EXIT_SUCCESS;
}
