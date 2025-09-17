#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>


#define MAX_PATH_LEN 1024
#define BUFFER_SIZE 4096

typedef enum {
    OK = 0,
    ERR_INVALID_ARGS,
    ERR_FILE_OPEN,
    ERR_MEMORY,
    ERR_FORK,
    ERR_FILE_WRITE,
    ERR_INVALID_N,
    ERR_INVALID_HEX,
    ERR_INVALID_NUM,
    ERR_STRING_NOT_FOUND
} ErrorCode;


void print_status(ErrorCode code, const char *additional_info) {
    const char *messages[] = {
            "Success",
            "Invalid arguments",
            "Failed to open file",
            "Memory allocation failed",
            "Fork failed",
            "Failed to write to file",
            "Invalid N for operation",
            "Invalid hexadecimal value",
            "Invalid number",
            "Search string not found in any files"
    };

    if (code != OK) {
        fprintf(stderr, "Error: %s", messages[code]);
        if (additional_info) {
            fprintf(stderr, " (%s)", additional_info);
        }
        fprintf(stderr, "\n");
    }
}

int is_hex(const char *str) {
    for (size_t i = 0; i < strlen(str); i++) {
        if (!((str[i] >= '0' && str[i] <= '9') ||
              (str[i] >= 'A' && str[i] <= 'F') ||
              (str[i] >= 'a' && str[i] <= 'f'))) {
            return 0;
        }
    }
    return 1;
}

int is_number(const char *str) {
    for (size_t i = 0; i < strlen(str); i++) {
        if (str[i] < '0' || str[i] > '9') {
            return 0;
        }
    }
    return 1;
}


ErrorCode xor_operation(const char *filename, int n) {
    FILE *file = fopen(filename, "rb");
    if (!file) return ERR_FILE_OPEN;

    const int block_bits = 1 << n;
    const int nibbles_in_block = block_bits / 4;

    uint8_t *block = (uint8_t *)calloc(nibbles_in_block, sizeof(uint8_t));
    if (!block) {
        fclose(file);
        return ERR_MEMORY;
    }

    uint8_t byte;
    int bit_counter = 0;

    while (fread(&byte, 1, 1, file) == 1) {
        for (int bit_pos = 7; bit_pos >= 0; bit_pos--) {
            uint8_t bit = (byte >> bit_pos) & 1;
            int nibble_idx = (bit_counter % block_bits) / 4;
            int bit_in_nibble = 3 - ((bit_counter % block_bits) % 4);

            if (bit) {
                block[nibble_idx] ^= (1 << bit_in_nibble);
            }

            bit_counter++;
        }
    }

    printf("XOR result for %s (N=%d): ", filename, n);
    for (int i = 0; i < nibbles_in_block; i++) {
        printf("%01x ", block[i]);
    }
    printf("\n");

    free(block);
    fclose(file);
    return OK;
}
ErrorCode mask_operation(const char *filename, uint32_t mask) {
    FILE *file = fopen(filename, "rb");
    if (!file) return ERR_FILE_OPEN;

    uint32_t buffer;
    size_t count = 0;
    size_t total = 0;
    size_t file_size;

    // Получаем размер файла
    fseek(file, 0, SEEK_END);
    file_size = ftell(file);
    fseek(file, 0, SEEK_SET);

    // Вычисляем сколько полных 4-байтных слов в файле
    size_t full_words = file_size / 4;
    size_t remaining_bytes = file_size % 4;

    // Читаем полные 4-байтные слова
    for (size_t i = 0; i < full_words; i++) {
        if (fread(&buffer, 4, 1, file) != 1) {
            fclose(file);
            return ERR_FILE_OPEN;
        }
        if ((buffer & mask) == mask) {
            count++;
        }
        total++;
    }

    // Читаем оставшиеся байты (если есть)
    if (remaining_bytes > 0) {
        buffer = 0;
        if (fread(&buffer, 1, remaining_bytes, file) != remaining_bytes) {
            fclose(file);
            return ERR_FILE_OPEN;
        }
        if ((buffer & mask) == mask) {
            count++;
        }
        total++;
    }

    printf("File %s: %zu matching values out of %zu (mask: 0x%08x)\n",
           filename, count, total, mask);

    fclose(file);
    return OK;
}
ErrorCode copy_operation(const char *filename, int n) {
    for (int i = 1; i <= n; i++) {
        pid_t pid = fork();
        if (pid == -1) {
            return ERR_FORK;
        } else if (pid == 0) {
            char new_filename[MAX_PATH_LEN];
            snprintf(new_filename, MAX_PATH_LEN, "%s_%d", filename, i);

            FILE *src = fopen(filename, "rb");
            if (!src) {
                exit(ERR_FILE_OPEN);
            }

            FILE *dst = fopen(new_filename, "wb");
            if (!dst) {
                fclose(src);
                exit(ERR_FILE_OPEN);
            }

            uint8_t buffer[BUFFER_SIZE];
            size_t bytes_read;

            while ((bytes_read = fread(buffer, 1, BUFFER_SIZE, src)) > 0) {
                if (fwrite(buffer, 1, bytes_read, dst) != bytes_read) {
                    fclose(src);
                    fclose(dst);
                    exit(ERR_FILE_WRITE);
                }
            }

            fclose(src);
            fclose(dst);
            exit(OK);
        }
    }


    ErrorCode overall_status = OK;
    for (int i = 1; i <= n; i++) {
        int status;
        wait(&status);
        if (WIFEXITED(status)) {
            ErrorCode child_status = WEXITSTATUS(status);
            if (child_status == OK) {
                printf("Created copy %d of %s\n", i, filename);
            } else {
                print_status(child_status, filename);
                overall_status = child_status;
            }
        } else {
            print_status(ERR_FORK, filename);
            overall_status = ERR_FORK;
        }
    }

    return overall_status;
}


ErrorCode find_operation(const char *filename, const char *search_string) {
    FILE *file = fopen(filename, "r");
    if (!file) {
        return ERR_FILE_OPEN;
    }

    char line[BUFFER_SIZE];
    int found = 0;

    while (fgets(line, BUFFER_SIZE, file)) {
        if (strstr(line, search_string)) {
            found = 1;
            break;
        }
    }

    fclose(file);

    if (found) {
        printf("Found in: %s\n", filename);
        return OK;
    } else {
        return ERR_STRING_NOT_FOUND;
    }
}

int main(int argc, char *argv[]) {
    if (argc < 3) {
        print_status(ERR_INVALID_ARGS, "Usage: file1 [file2 ...] command [args]");
        return ERR_INVALID_ARGS;
    }

    int num_files = argc - 2;
    char *command = argv[argc - 1];
    char *command_arg = NULL;


    if (strcmp(command, "xor2") != 0 && strcmp(command, "xor3") != 0 &&
        strcmp(command, "xor4") != 0 && strcmp(command, "xor5") != 0 &&
        strcmp(command, "xor6") != 0 && strcmp(command, "copyN") != 0) {
        if (argc < 4) {
            print_status(ERR_INVALID_ARGS, command);
            return ERR_INVALID_ARGS;
        }
        num_files = argc - 3;
        command_arg = argv[argc - 1];
        command = argv[argc - 2];
    }

    ErrorCode status = OK;


    if (strncmp(command, "xor", 3) == 0) {
        int n = command[3] - '0';
        if (n < 2 || n > 6) {
            print_status(ERR_INVALID_N, command);
            return ERR_INVALID_N;
        }

        for (int i = 1; i <= num_files; i++) {
            status = xor_operation(argv[i], n);
            if (status != OK) {
                print_status(status, argv[i]);
            }
        }
    }
    else if (strcmp(command, "mask") == 0) {
        if (!command_arg || !is_hex(command_arg)) {
            print_status(ERR_INVALID_HEX, command_arg);
            return ERR_INVALID_HEX;
        }

        uint32_t mask = strtoul(command_arg, NULL, 16);
        for (int i = 1; i <= num_files; i++) {
            status = mask_operation(argv[i], mask);
            if (status != OK) {
                print_status(status, argv[i]);
            }
        }
    }
    else if (strcmp(command, "copyN") == 0) {
        if (!command_arg || !is_number(command_arg)) {
            print_status(ERR_INVALID_NUM, command_arg);
            return ERR_INVALID_NUM;
        }

        int n = atoi(command_arg);
        if (n <= 0) {
            print_status(ERR_INVALID_NUM, command_arg);
            return ERR_INVALID_NUM;
        }

        for (int i = 1; i <= num_files; i++) {
            status = copy_operation(argv[i], n);
            if (status != OK) {
                print_status(status, argv[i]);
            }
        }
    }
    else if (strcmp(command, "find") == 0) {
        if (!command_arg) {
            print_status(ERR_INVALID_ARGS, "Missing search string");
            return ERR_INVALID_ARGS;
        }

        int any_found = 0;
        for (int i = 1; i <= num_files; i++) {
            pid_t pid = fork();
            if (pid == -1) {
                print_status(ERR_FORK, argv[i]);
                status = ERR_FORK;
                continue;
            } else if (pid == 0) {
                ErrorCode child_status = find_operation(argv[i], command_arg);
                exit(child_status);
            } else {
                int child_status;
                waitpid(pid, &child_status, 0);
                if (WIFEXITED(child_status) && WEXITSTATUS(child_status) == OK) {
                    any_found = 1;
                }
            }
        }

        if (!any_found) {
            print_status(ERR_STRING_NOT_FOUND, command_arg);
            status = ERR_STRING_NOT_FOUND;
        }
    }
    else {
        print_status(ERR_INVALID_ARGS, command);
        return ERR_INVALID_ARGS;
    }

    return status;
}