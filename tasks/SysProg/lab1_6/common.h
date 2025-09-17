#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <libgen.h>

#define MAX_TEXT_LEN 256

typedef enum Errors {
    MSG_ERROR = 2,
    MEMORY_LEAK
} Errors;

typedef struct message {
    long mtype;
    char mtext[MAX_TEXT_LEN];
} message;

typedef struct FileNode {
    char *name;
    struct FileNode *next;
} FileNode;

typedef struct DirNode {
    char *path;
    FileNode *files;
    struct DirNode *next;
} DirNode;

DirNode *find_dir(DirNode *head, const char *path);

DirNode *add_dir(DirNode **head, const char *path);

int add_file(DirNode *dir, const char *filename);

void free_dirs(DirNode *dir_list);
