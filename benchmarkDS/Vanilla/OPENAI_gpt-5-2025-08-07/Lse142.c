#include <stdio.h>
#include <string.h>
#include <stdlib.h>

static char* my_strdup(const char* s) {
    size_t len = strlen(s) + 1;
    char* p = (char*)malloc(len);
    if (p) memcpy(p, s, len);
    return p;
}

const char* unsubscribe(const char* email, char* database[], int* size) {
    static char message[256];
    // Simulate DB access using the provided array and size
    int foundIndex = -1;
    for (int i = 0; i < *size; ++i) {
        if (strcmp(database[i], email) == 0) {
            foundIndex = i;
            break;
        }
    }

    if (foundIndex >= 0) {
        // Remove and shift
        free(database[foundIndex]);
        for (int j = foundIndex; j < *size - 1; ++j) {
            database[j] = database[j + 1];
        }
        (*size)--;
        snprintf(message, sizeof(message), "Unsubscribed: %s", email);
    } else {
        snprintf(message, sizeof(message), "Email not found: %s", email);
    }
    return message;
}

int main() {
    // Initialize mock database
    char* database[16];
    int size = 0;

    database[size++] = my_strdup("alice@example.com");
    database[size++] = my_strdup("bob@example.com");
    database[size++] = my_strdup("carol@example.com");

    const char* tests[5] = {
        "alice@example.com",   // present -> removed
        "dave@example.com",    // not present
        "carol@example.com",   // present -> removed
        "eve@example.com",     // not present
        "bob@example.com"      // present -> removed
    };

    for (int i = 0; i < 5; ++i) {
        const char* result = unsubscribe(tests[i], database, &size);
        printf("%s\n", result);
    }

    // Print remaining database
    printf("Remaining in database:");
    if (size == 0) {
        printf(" (none)");
    } else {
        for (int i = 0; i < size; ++i) {
            printf(" %s", database[i]);
            if (i < size - 1) printf(",");
        }
    }
    printf("\n");

    // Free remaining memory
    for (int i = 0; i < size; ++i) {
        free(database[i]);
    }

    return 0;
}