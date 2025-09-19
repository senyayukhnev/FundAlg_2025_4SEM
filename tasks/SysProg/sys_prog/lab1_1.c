#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <ctype.h>

#define MAX_USERS 100
#define MAX_LOGIN_LEN 6
#define PIN_CODE 12345
#define MAX_PIN 100000

typedef enum {
    LOGIN_NOT_EXIST,
    LOGIN_SUCCESS,
    INCORRECT_PIN,
    USER_REGISTERED,
    USER_LIMIT_REACHED,
    INVALID_COMMAND,
    INVALID_DATA,
    SANCTIONS_APPLIED,
    USER_ALREADY_EXISTS,
    INVALID_PIN
} StatusCode;

typedef struct {
    char login[MAX_LOGIN_LEN + 1];
    int pin;
    int restrictions;
    int command_count;
} User;

User users[MAX_USERS];
int user_count = 0;
User *current_user = NULL;

void print_status(StatusCode code) {
    const char *messages[] = {
            "Login does not exist.",
            "Login successful.",
            "Incorrect PIN.",
            "User registered successfully.",
            "User limit reached.",
            "Invalid command.",
            "Incorrect data.",
            "Sanctions applied successfully.",
            "User already exists.",
            "Invalid PIN. Must be between 0 and 100000."
    };
    printf("%s\n", messages[code]);
}

User* find_user(char *login) {
    for (int i = 0; i < user_count; i++) {
        if (strcmp(users[i].login, login) == 0) {
            return &users[i];
        }
    }
    return NULL;
}

void register_user() {
    if (user_count >= MAX_USERS) {
        print_status(USER_LIMIT_REACHED);
        return;
    }
    User new_user;
    printf("Enter login and PIN: ");
    if (scanf("%6s %d", new_user.login, &new_user.pin) != 2) {
        print_status(INVALID_DATA);
        while (getchar() != '\n');
        return;
    }
    if (find_user(new_user.login)) {
        print_status(USER_ALREADY_EXISTS);
        return;
    }
    if (new_user.pin < 0 || new_user.pin > MAX_PIN) {
        print_status(INVALID_PIN);
        return;
    }
    new_user.restrictions = -1;
    new_user.command_count = 0;
    users[user_count++] = new_user;
    print_status(USER_REGISTERED);
}

void login_user() {
    char login[MAX_LOGIN_LEN + 1];
    int pin;
    printf("Enter login and PIN: ");
    if (scanf("%6s %d", login, &pin) != 2) {
        print_status(INVALID_DATA);
        while (getchar() != '\n');
        return;
    }
    User *user = find_user(login);
    if (!user) {
        print_status(LOGIN_NOT_EXIST);
        return;
    }
    if (user->pin != pin) {
        print_status(INCORRECT_PIN);
        return;
    }
    current_user = user;
    print_status(LOGIN_SUCCESS);
}

void get_time() {
    time_t t = time(NULL);
    struct tm *tm_info = localtime(&t);
    printf("%02d:%02d:%02d\n", tm_info->tm_hour, tm_info->tm_min, tm_info->tm_sec);
}

void get_date() {
    time_t t = time(NULL);
    struct tm *tm_info = localtime(&t);
    printf("%02d:%02d:%04d\n", tm_info->tm_mday, tm_info->tm_mon + 1, tm_info->tm_year + 1900);
}

void howmuch(char *date, char *flag) {
    int day, month, year;
    if (sscanf(date, "%d:%d:%d", &day, &month, &year) != 3) {
        print_status(INVALID_DATA);
        return;
    }
    struct tm input_time = {0};
    input_time.tm_mday = day;
    input_time.tm_mon = month - 1;
    input_time.tm_year = year - 1900;
    time_t input_epoch = mktime(&input_time);
    if (input_epoch == -1) {
        print_status(INVALID_DATA);
        return;
    }
    time_t current_time = time(NULL);
    double diff = difftime(current_time, input_epoch);
    if (strcmp(flag, "-s") == 0) {
        printf("%.0f seconds\n", diff);
    } else if (strcmp(flag, "-m") == 0) {
        printf("%.0f minutes\n", diff / 60);
    } else if (strcmp(flag, "-h") == 0) {
        printf("%.0f hours\n", diff / 3600);
    } else if (strcmp(flag, "-y") == 0) {
        printf("%.2f years\n", diff / (3600 * 24 * 365.25));
    } else {
        print_status(INVALID_DATA);
    }
}

void apply_sanctions(char *username, int limit) {
    printf("Enter confirmation code: ");
    int confirmation;
    if (scanf("%d", &confirmation) != 1 || confirmation != PIN_CODE) {
        print_status(INVALID_DATA);
        return;
    }
    User *user = find_user(username);
    if (!user) {
        print_status(LOGIN_NOT_EXIST);
        return;
    }
    user->restrictions = limit;
    print_status(SANCTIONS_APPLIED);
}

void logout() {
    current_user = NULL;
}

void shell() {
    char command[50], arg1[20], arg2[10];
    while (1) {
        if (!current_user) {
            printf("1. Register\n2. Login\n> ");
            int choice;
            if (scanf("%d", &choice) != 1) {
                print_status(INVALID_DATA);
                while (getchar() != '\n');
                continue;
            }
            if (choice == 1) register_user();
            else if (choice == 2) login_user();
            continue;
        }
        if (current_user->restrictions != -1 && current_user->command_count >= current_user->restrictions) {
            printf("Command limit reached. Logging out.\n");
            logout();
            continue;
        }
        printf("shell> ");
        if (scanf("%s", command) != 1) {
            print_status(INVALID_DATA);
            while (getchar() != '\n');
            continue;
        }
        current_user->command_count++;
        if (strcmp(command, "Time") == 0) get_time();
        else if (strcmp(command, "Date") == 0) get_date();
        else if (strcmp(command, "Howmuch") == 0 && scanf("%s %s", arg1, arg2) == 2) howmuch(arg1, arg2);
        else if (strcmp(command, "Logout") == 0) logout();
        else if (strcmp(command, "Sanctions") == 0 && scanf("%s %d", arg1, &arg2) == 2) apply_sanctions(arg1, atoi(arg2));
        else print_status(INVALID_COMMAND);
    }
}

int main() {
    shell();
    return 0;
}
