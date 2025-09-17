#include "common.h"
#include <unistd.h> // для usleep

typedef struct {
    int msqid;
    int client_id;
} ClientState;

void cleanup(ClientState* state) {
    if (state->msqid != -1) {
        Message msg = {
                .mtype = 1,
                .client_id = state->client_id,
                .cmd_type = CMD_TERMINATE
        };
        msgsnd(state->msqid, &msg, sizeof(Message) - sizeof(long), 0);
    }
}

ErrorCode send_command(ClientState* state, CommandType cmd_type, Object object, const char* cmd_str) {
    Message msg = {
            .mtype = 1,
            .client_id = state->client_id,
            .cmd_type = cmd_type,
            .object = object
    };
    strncpy(msg.message, cmd_str, MSG_SIZE - 1);
    msg.message[MSG_SIZE - 1] = '\0';

    if (msgsnd(state->msqid, &msg, sizeof(Message) - sizeof(long), 0) == -1) {
        print_status(ERR_QUEUE_SEND, strerror(errno));
        return ERR_QUEUE_SEND;
    }

    Message response;
    if (msgrcv(state->msqid, &response, sizeof(Message) - sizeof(long), state->client_id, 0) == -1) {
        print_status(ERR_QUEUE_RECEIVE, strerror(errno));
        return ERR_QUEUE_RECEIVE;
    }

    print_status(response.error, response.message);
    usleep(200000); // 200ms задержка между командами
    return response.error;
}

ErrorCode process_file(ClientState* state, const char* filename) {
    FILE* file = fopen(filename, "r");
    if (!file) return ERR_FILE_OPEN;

    char line[MAX_CMD_LENGTH];
    while (fgets(line, sizeof(line), file)) {
        line[strcspn(line, "\r\n")] = '\0';
        if (!line[0]) continue;

        printf("Executing: %s\n", line);

        char* cmd = strtok(line, " ");
        if (!cmd) continue;

        ErrorCode err = OK;
        Object obj = NOTHING;

        if (strcmp(cmd, "take") == 0) {
            char* arg = strtok(NULL, " ");
            if (!arg) {
                printf("Missing object for 'take'\n");
                continue;
            }

            if (strcmp(arg, "wolf") == 0) obj = WOLF;
            else if (strcmp(arg, "goat") == 0) obj = GOAT;
            else if (strcmp(arg, "cabbage") == 0) obj = CABBAGE;
            else {
                printf("Invalid object: %s\n", arg);
                continue;
            }

            err = send_command(state, CMD_TAKE, obj, line);
        }
        else if (strcmp(cmd, "put") == 0) {
            err = send_command(state, CMD_PUT, NOTHING, line);
        }
        else if (strcmp(cmd, "move") == 0) {
            err = send_command(state, CMD_MOVE, NOTHING, line);
        }

        if (err != OK) {
            fclose(file);
            return err;
        }

        usleep(300000); // 300ms задержка
    }

    fclose(file);
    return OK;
}
int main(int argc, char* argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <command_file>\n", argv[0]);
        return EXIT_FAILURE;
    }

    ClientState state = {
            .client_id = getpid(),
            .msqid = -1
    };

    key_t key = ftok(SERVER_KEY_PATH, SERVER_PROJ_ID);
    if (key == -1) {
        print_status(ERR_QUEUE_CREATE, strerror(errno));
        return EXIT_FAILURE;
    }

    state.msqid = msgget(key, 0666);
    if (state.msqid == -1) {
        print_status(ERR_QUEUE_CREATE, strerror(errno));
        return EXIT_FAILURE;
    }

    // Регистрация клиента
    Message init_msg = {
            .mtype = 1,
            .client_id = state.client_id,
            .cmd_type = CMD_INIT
    };

    if (msgsnd(state.msqid, &init_msg, sizeof(Message) - sizeof(long), 0) == -1) {
        print_status(ERR_QUEUE_SEND, strerror(errno));
        return EXIT_FAILURE;
    }

    Message response;
    if (msgrcv(state.msqid, &response, sizeof(Message) - sizeof(long), state.client_id, 0) == -1) {
        print_status(ERR_QUEUE_RECEIVE, strerror(errno));
        return EXIT_FAILURE;
    }

    if (response.error != OK) {
        return EXIT_FAILURE;
    }

    ErrorCode err = process_file(&state, argv[1]);
    cleanup(&state);

    return (err == OK) ? EXIT_SUCCESS : EXIT_FAILURE;
}