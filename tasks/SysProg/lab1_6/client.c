#include "common.h"

int main() {
    key_t key = ftok(".", 'S');
    int msgid = msgget(key, 0666);
    if (msgid == -1) {
        printf("Message Error\n");
        return MSG_ERROR;
    }

    message msg;
    msg.mtype = 1;

    char input[MAX_TEXT_LEN];
    while(1) {
        printf("Enter path or 'stop': \n");
        if (fgets(input, MAX_TEXT_LEN, stdin) == NULL) {
            break;
        }
        input[strcspn(input, "\n")] = '\0';

        if (strcmp(input, "stop") == 0) {
            strcpy(msg.mtext, input);
            if (msgsnd(msgid, &msg, sizeof(msg.mtext), 0) == -1) {
                printf("Message Error\n");
                return MSG_ERROR;
            }
            break;
        }

        if (input[0] != '/') {
            printf("Error: Path must be absolute.\n");
            continue;
        }

        strncpy(msg.mtext, input, MAX_TEXT_LEN);
        msg.mtext[MAX_TEXT_LEN - 1] = '\0';

        if (msgsnd(msgid, &msg, sizeof(msg.mtext), 0) == -1) {
            printf("Message Error\n");
            return MSG_ERROR;
        }
    }

    printf("Client stopped\n");
    return 0;
}
