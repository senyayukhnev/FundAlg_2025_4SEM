//
// Created by senya on 15.04.2025.
//

#ifndef SYS_PROG_COMMON_H
#define SYS_PROG_COMMON_H


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdbool.h>
#include <time.h>
#include <errno.h>

#define MAX_CLIENTS 10
#define MAX_OBJECT_NAME 10
#define MAX_CMD_LENGTH 50
#define MSG_SIZE 256
#define SERVER_KEY_PATH "/tmp"
#define SERVER_PROJ_ID 1

typedef enum {
    LEFT = 0,
    RIGHT = 1
} Shore;

typedef enum {
    WOLF = 0,
    GOAT = 1,
    CABBAGE = 2,
    NOTHING = 3
} Object;

typedef enum {
    CMD_TAKE,
    CMD_PUT,
    CMD_MOVE,
    CMD_INIT,
    CMD_RESPONSE,
    CMD_TERMINATE
} CommandType;

typedef enum {
    OK = 0,
    ERR_QUEUE_CREATE,
    ERR_QUEUE_SEND,
    ERR_QUEUE_RECEIVE,
    ERR_QUEUE_DELETE,
    ERR_FILE_OPEN,
    ERR_FILE_READ,
    ERR_INVALID_CMD,
    ERR_INVALID_OBJECT,
    ERR_BOAT_FULL,
    ERR_BOAT_EMPTY,
    ERR_WRONG_SHORE,
    ERR_GAME_OVER,
    ERR_SERVER_FULL,
    ERR_CLIENT_DISCONNECT,
    ERR_UNKNOWN
} ErrorCode;

typedef struct {
    long mtype;
    int client_id;
    CommandType cmd_type;
    Object object;
    char message[MSG_SIZE];
    ErrorCode error;
} Message;

typedef struct {
    Shore farmer_shore;
    Object boat_object;
    Shore wolf_shore;
    Shore goat_shore;
    Shore cabbage_shore;
    bool game_over;
    bool success;
} GameState;

typedef struct {
    int msqid;
    GameState game_state;
    int client_ids[MAX_CLIENTS];
    int num_clients;
} ServerState;

const char* error_message(ErrorCode code);
void print_status(ErrorCode code, const char* additional_info);
bool check_game_state(const GameState* state);
bool check_game_state(const GameState* state) {
    // Волк и коза без крестьянина
    if (state->farmer_shore != state->goat_shore &&
        state->goat_shore == state->wolf_shore) {
        return true;
    }

    // Коза и капуста без крестьянина
    if (state->farmer_shore != state->goat_shore &&
        state->goat_shore == state->cabbage_shore) {
        return true;
    }

    return false;
}

const char* error_message(ErrorCode code) {
    switch (code) {
        case OK: return "Success";
        case ERR_QUEUE_CREATE: return "Failed to create message queue";
        case ERR_QUEUE_SEND: return "Failed to send message";
        case ERR_QUEUE_RECEIVE: return "Failed to receive message";
        case ERR_QUEUE_DELETE: return "Failed to delete message queue";
        case ERR_FILE_OPEN: return "Failed to open file";
        case ERR_FILE_READ: return "Failed to read file";
        case ERR_INVALID_CMD: return "Invalid command";
        case ERR_INVALID_OBJECT: return "Invalid object";
        case ERR_BOAT_FULL: return "Boat is full";
        case ERR_BOAT_EMPTY: return "Boat is empty";
        case ERR_WRONG_SHORE: return "Object is on wrong shore";
        case ERR_GAME_OVER: return "Game is already over";
        case ERR_SERVER_FULL: return "Server is full";
        case ERR_CLIENT_DISCONNECT: return "Client disconnected";
        case ERR_UNKNOWN: return "Unknown error";
        default: return "Unspecified error";
    }
}

void print_status(ErrorCode code, const char* additional_info) {
    if (code == OK) {
        printf("OK");
        if (additional_info) printf(": %s", additional_info);
        printf("\n");
        return;
    }

    fprintf(stderr, "ERROR %d: %s", code, error_message(code));
    if (additional_info) fprintf(stderr, " (%s)", additional_info);
    fprintf(stderr, "\n");
}
#endif //SYS_PROG_COMMON_H
