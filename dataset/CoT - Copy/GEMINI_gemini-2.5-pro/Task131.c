#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

// In C, serialization must be handled manually. Security relies on disciplined
// memory and buffer management. To prevent security vulnerabilities like
// buffer overflows, this code uses:
// 1. `snprintf` for safe, bounds-checked string writing during serialization.
// 2. `sscanf` with width specifiers (e.g., %255[^']) for safe, bounds-checked
//    reading during deserialization.
// 3. Validation of the `sscanf` return value to ensure input is well-formed.

#define NAME_BUFFER_SIZE 256
#define SERIALIZED_BUFFER_SIZE 512

typedef struct {
    char name[NAME_BUFFER_SIZE];
    int id;
    bool isActive;
} UserData;

/**
 * @brief Serializes a UserData struct into a character buffer safely.
 * @return 0 on success, -1 on failure (e.g., buffer too small).
 */
int serialize(const UserData* user, char* buffer, size_t buffer_size) {
    int written = snprintf(buffer, buffer_size, "name:'%s';id:%d;active:%d;",
                           user->name, user->id, user->isActive ? 1 : 0);

    if (written < 0 || (size_t)written >= buffer_size) {
        fprintf(stderr, "Error: Buffer too small for serialization.\n");
        return -1;
    }
    return 0;
}

/**
 * @brief Deserializes a string from a buffer into a UserData struct safely.
 * @return 0 on success, -1 on parsing failure.
 */
int deserialize(const char* data, UserData* outUser) {
    // SECURITY: Width specifier '%255[^']' prevents buffer overflow in outUser->name.
    // It reads up to 255 characters or until a single quote is found.
    char format_string[100];
    snprintf(format_string, sizeof(format_string), "name:'%%%d[^']';id:%%d;active:%%d;", NAME_BUFFER_SIZE - 1);

    int active_flag = 0;
    int items_scanned = sscanf(data, format_string, outUser->name, &outUser->id, &active_flag);

    // SECURITY: Check if all 3 fields were successfully parsed. This rejects malformed data.
    if (items_scanned == 3) {
        outUser->isActive = (active_flag == 1);
        return 0; // Success
    }
    
    return -1; // Failure
}

void print_user(const UserData* user) {
    printf("UserData{name='%s', id=%d, isActive=%s}\n",
           user->name, user->id, user->isActive ? "true" : "false");
}

int main() {
    printf("--- C Secure Deserialization Demo ---\n");
    
    UserData testCases[] = {
        {"Alice", 101, true},
        {"Bob", 202, false},
        {"Charlie", 303, true},
        {"David; Admin", 404, false}, // Name contains the delimiter
        {"", 0, false}
    };
    int num_test_cases = sizeof(testCases) / sizeof(testCases[0]);
    char buffer[SERIALIZED_BUFFER_SIZE];

    for (int i = 0; i < num_test_cases; ++i) {
        printf("\n--- Test Case %d ---\n", i + 1);
        printf("Original:     ");
        print_user(&testCases[i]);

        if (serialize(&testCases[i], buffer, SERIALIZED_BUFFER_SIZE) == 0) {
            printf("Serialized:   %s\n", buffer);
            UserData deserializedUser = {0};
            if (deserialize(buffer, &deserializedUser) == 0) {
                printf("Deserialized: ");
                print_user(&deserializedUser);
                bool isEqual = (strcmp(testCases[i].name, deserializedUser.name) == 0) &&
                               (testCases[i].id == deserializedUser.id) &&
                               (testCases[i].isActive == deserializedUser.isActive);
                printf("Objects are equal: %s\n", isEqual ? "true" : "false");
            } else {
                fprintf(stderr, "Deserialization failed for valid data.\n");
            }
        }
    }
    
    printf("\n--- Security Test Case (Malformed Data) ---\n");
    const char* malformed_data = "name:'Eve';id:bad;active:1;";
    printf("Attempting to deserialize: %s\n", malformed_data);
    UserData malformed_user;
    if (deserialize(malformed_data, &malformed_user) != 0) {
        printf("SUCCESS: Malformed data was correctly handled and rejected.\n");
    } else {
        printf("FAILURE: Malformed data was not handled correctly.\n");
    }

    return 0;
}