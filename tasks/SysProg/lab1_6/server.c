#include "common.h"

DirNode *find_dir(DirNode *head, const char *path) {
    for (DirNode *current = head; current != NULL; current = current->next) {
        if (strcmp(current->path, path) == 0) {
            return current;
        }
    }
    return NULL;
}

DirNode *add_dir(DirNode **head, const char *path) {
    DirNode *new_dir = malloc(sizeof(DirNode));
    if (!new_dir) {
        return NULL;
    }

    new_dir->path = strdup(path);
    if (!new_dir->path) {
        free(new_dir);
        return NULL;
    }

    new_dir->files = NULL;
    new_dir->next = NULL;

    if (*head == NULL) {
        *head = new_dir;
    } else {
        DirNode *current = *head;
        while (current->next) current = current->next;
        current->next = new_dir;
    }
    return new_dir;
}

int add_file(DirNode *dir, const char *filename) {
    FileNode *new_file = malloc(sizeof(FileNode));
    if (!new_file) {
        return MEMORY_LEAK;
    }

    new_file->name = strdup(filename);
    if (!new_file->name) {
        return MEMORY_LEAK;
    }

    new_file->next = NULL;

    if (!dir->files) {
        dir->files = new_file;
    } else {
        FileNode *current = dir->files;
        while (current->next) current = current->next;
        current->next = new_file;
    }
    return 0;
}

void free_dirs(DirNode *dir_list) {
    while (dir_list) {
        DirNode *next_dir = dir_list->next;
        FileNode *current_file = dir_list->files;
        while (current_file) {
            FileNode *next_file = current_file->next;
            free(current_file->name);
            free(current_file);
            current_file = next_file;
        }
        free(dir_list->path);
        free(dir_list);
        dir_list = next_dir;
    }
}

int main() {
    DirNode *dir_list = NULL;
    int msgid = -1;

    key_t key = ftok(".", 'S');
    if ((msgid = msgget(key, 0666 | IPC_CREAT | IPC_EXCL)) == -1) {
        printf("Message Error\n");
        return MSG_ERROR;
    }

    printf("Server started\n");

    message msg;
    while(1) {
        if (msgrcv(msgid, &msg, sizeof(msg.mtext), 1, 0) == -1) {
            printf("Message Error\n");
            return MSG_ERROR;
        }

        if (strcmp(msg.mtext, "stop") == 0) break;

        char path_copy[MAX_TEXT_LEN];
        strncpy(path_copy, msg.mtext, MAX_TEXT_LEN);
        path_copy[MAX_TEXT_LEN-1] = '\0';

        char *dir_path = dirname(path_copy);
        char *file_name = basename(msg.mtext);

        if (dir_path[0] != '/') {
            fprintf(stderr, "Invalid path: %s\n", msg.mtext);
            continue;
        }

        DirNode *dir = find_dir(dir_list, dir_path);
        if (!dir) {
            if (!(dir = add_dir(&dir_list, dir_path))) {
                printf("Memory leak\n");
                free_dirs(dir_list);
                return MEMORY_LEAK;
            }
        }

        if (add_file(dir, file_name) != 0) {
            printf("Memory leak\n");
            free_dirs(dir_list);
            return MEMORY_LEAK;
        }
    }

    for (DirNode *d = dir_list; d; d = d->next) {
        printf("Directory: %s\nFiles: ", d->path);
        for (FileNode *f = d->files; f; f = f->next) {
            printf("%s%s", f->name, f->next ? ", " : "");
        }
        printf("\n");
    }


    free_dirs(dir_list);
    if (msgid != -1 && msgctl(msgid, IPC_RMID, NULL) == -1) {
        printf("Message Error\n");
        return MSG_ERROR;
    }

    printf("Server stopped \n");
    return 0;
}