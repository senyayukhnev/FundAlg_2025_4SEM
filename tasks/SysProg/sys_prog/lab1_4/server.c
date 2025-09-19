#include "common.h"

void cleanup(ServerState* state) {
    if (state->msqid != -1) {
        if (msgctl(state->msqid, IPC_RMID, NULL) == -1) {
            print_status(ERR_QUEUE_DELETE, strerror(errno));
        }
    }
}

void send_response(const ServerState* state, int client_id, const char* message, ErrorCode error) {
    Message msg;
    memset(&msg, 0, sizeof(Message));
    msg.mtype = client_id;
    msg.client_id = getpid();
    msg.cmd_type = CMD_RESPONSE;
    msg.error = error;
    strncpy(msg.message, message, MSG_SIZE - 1);
    msg.message[MSG_SIZE - 1] = '\0';

    if (msgsnd(state->msqid, &msg, sizeof(Message) - sizeof(long), 0) == -1) {
        print_status(ERR_QUEUE_SEND, strerror(errno));
        exit(EXIT_FAILURE);
    }
}

void handle_init(ServerState* state, Message* msg) {
    if (state->num_clients >= MAX_CLIENTS) {
        send_response(state, msg->client_id, "Server is full", ERR_SERVER_FULL);
        return;
    }

    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (state->client_ids[i] == 0) {
            state->client_ids[i] = msg->client_id;
            state->num_clients++;
            send_response(state, msg->client_id, "Connected to server", OK);
            printf("Client %d connected\n", msg->client_id);
            return;
        }
    }

    send_response(state, msg->client_id, "Failed to connect", ERR_UNKNOWN);
}

void handle_take(ServerState* state, Object object, int client_id) {
    if (state->game_state.game_over) {
        send_response(state, client_id, "Game is already over", ERR_GAME_OVER);
        return;
    }

    printf("[TAKE] Client %d trying to take %d. Current boat: %d\n",
           client_id, object, state->game_state.boat_object);

    if (state->game_state.boat_object != NOTHING) {
        send_response(state, client_id, "Boat is already full", ERR_BOAT_FULL);
        return;
    }

    Shore object_shore;
    switch (object) {
        case WOLF: object_shore = state->game_state.wolf_shore; break;
        case GOAT: object_shore = state->game_state.goat_shore; break;
        case CABBAGE: object_shore = state->game_state.cabbage_shore; break;
        default:
            send_response(state, client_id, "Invalid object", ERR_INVALID_OBJECT);
            return;
    }

    if (object_shore != state->game_state.farmer_shore) {
        send_response(state, client_id, "Object is not on the same shore", ERR_WRONG_SHORE);
        return;
    }

    state->game_state.boat_object = object;
    printf("[TAKE] Client %d took %d. Boat now has: %d\n",
           client_id, object, state->game_state.boat_object);

    send_response(state, client_id, "Object taken successfully", OK);
}

void handle_put(ServerState* state, int client_id) {
    if (state->game_state.game_over) {
        send_response(state, client_id, "Game is already over", ERR_GAME_OVER);
        return;
    }

    printf("[PUT] Client %d trying to put. Current boat: %d\n",
           client_id, state->game_state.boat_object);

    if (state->game_state.boat_object == NOTHING) {
        send_response(state, client_id, "Boat is empty", ERR_BOAT_EMPTY);
        return;
    }

    switch (state->game_state.boat_object) {
        case WOLF:
            state->game_state.wolf_shore = state->game_state.farmer_shore;
            break;
        case GOAT:
            state->game_state.goat_shore = state->game_state.farmer_shore;
            break;
        case CABBAGE:
            state->game_state.cabbage_shore = state->game_state.farmer_shore;
            break;
        default:
            break;
    }

    printf("[PUT] Client %d put %d on shore %d\n",
           client_id, state->game_state.boat_object, state->game_state.farmer_shore);

    state->game_state.boat_object = NOTHING;
    send_response(state, client_id, "Object put successfully", OK);

    if (check_game_state(&state->game_state)) {
        state->game_state.game_over = true;
        state->game_state.success = false;
        send_response(state, client_id, "Game over! Rules violated.", ERR_GAME_OVER);
    }
    else if (state->game_state.wolf_shore == RIGHT &&
             state->game_state.goat_shore == RIGHT &&
             state->game_state.cabbage_shore == RIGHT) {
        state->game_state.game_over = true;
        state->game_state.success = true;
        send_response(state, client_id, "Congratulations! All objects are safely on the right shore.", OK);
    }
}

void handle_move(ServerState* state, int client_id) {
    if (state->game_state.game_over) {
        send_response(state, client_id, "Game is already over", ERR_GAME_OVER);
        return;
    }

    printf("[MOVE] Client %d moving from %d\n",
           client_id, state->game_state.farmer_shore);

    state->game_state.farmer_shore = (state->game_state.farmer_shore == LEFT) ? RIGHT : LEFT;

    if (state->game_state.boat_object != NOTHING) {
        switch (state->game_state.boat_object) {
            case WOLF:
                state->game_state.wolf_shore = state->game_state.farmer_shore;
                break;
            case GOAT:
                state->game_state.goat_shore = state->game_state.farmer_shore;
                break;
            case CABBAGE:
                state->game_state.cabbage_shore = state->game_state.farmer_shore;
                break;
            default:
                break;
        }
    }

    printf("[MOVE] Client %d moved to %d. Boat: %d\n",
           client_id, state->game_state.farmer_shore, state->game_state.boat_object);

    send_response(state, client_id, "Moved to the other shore", OK);
}

void handle_terminate(ServerState* state, int client_id) {
    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (state->client_ids[i] == client_id) {
            state->client_ids[i] = 0;
            state->num_clients--;
            printf("Client %d disconnected\n", client_id);
            break;
        }
    }
    send_response(state, client_id, "Client disconnected", OK);
}

void process_message(ServerState* state, Message* msg) {
    printf("Processing command %d from client %d\n", msg->cmd_type, msg->client_id);

    switch (msg->cmd_type) {
        case CMD_INIT:
            handle_init(state, msg);
            break;
        case CMD_TAKE:
            handle_take(state, msg->object, msg->client_id);
            break;
        case CMD_PUT:
            handle_put(state, msg->client_id);
            break;
        case CMD_MOVE:
            handle_move(state, msg->client_id);
            break;
        case CMD_TERMINATE:
            handle_terminate(state, msg->client_id);
            break;
        default:
            send_response(state, msg->client_id, "Unknown command", ERR_INVALID_CMD);
            break;
    }
}

int main() {
    ServerState state = {
            .msqid = -1,
            .game_state = {
                    .farmer_shore = LEFT,
                    .boat_object = NOTHING,
                    .wolf_shore = LEFT,
                    .goat_shore = LEFT,
                    .cabbage_shore = LEFT,
                    .game_over = false,
                    .success = false
            },
            .num_clients = 0
    };
    memset(state.client_ids, 0, sizeof(state.client_ids));

    key_t key = ftok(SERVER_KEY_PATH, SERVER_PROJ_ID);
    if (key == -1) {
        print_status(ERR_QUEUE_CREATE, strerror(errno));
        exit(EXIT_FAILURE);
    }

    state.msqid = msgget(key, IPC_CREAT | 0666);
    if (state.msqid == -1) {
        print_status(ERR_QUEUE_CREATE, strerror(errno));
        exit(EXIT_FAILURE);
    }

    printf("Server started. Waiting for messages...\n");

    Message msg;
    while (1) {
        ssize_t msg_size = msgrcv(state.msqid, &msg, sizeof(Message) - sizeof(long), 1, 0);
        if (msg_size == -1) {
            print_status(ERR_QUEUE_RECEIVE, strerror(errno));
            continue;
        }

        printf("Received message of size %zd from client %d\n", msg_size, msg.client_id);
        process_message(&state, &msg);
    }

    cleanup(&state);
    return 0;
}