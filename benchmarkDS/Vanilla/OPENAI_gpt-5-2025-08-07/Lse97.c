#include <stdio.h>
#include <string.h>
#include <stdint.h>

#define MAX_USERS 100
#define MAX_LEN_USERNAME 64
#define MAX_LEN_NAME 64
#define MAX_LEN_EMAIL 128
#define HASH_HEX_LEN 17  // 16 hex digits + null

typedef struct {
    char username[MAX_LEN_USERNAME];
    char firstName[MAX_LEN_NAME];
    char lastName[MAX_LEN_NAME];
    char email[MAX_LEN_EMAIL];
    char passwordHash[HASH_HEX_LEN];
} User;

static User db[MAX_USERS];
static int db_count = 0;

static void simple_hash(const char* s, char out_hex[HASH_HEX_LEN]) {
    uint64_t h = 5381ULL;
    while (*s) {
        h = ((h << 5) + h) + (unsigned char)(*s); // h * 33 + c
        s++;
    }
    // write 16 hex digits (lowercase), ensure null-termination
    // If leading zeros desired for fixed width: "%016llx"
    // Using fixed width for consistency
    snprintf(out_hex, HASH_HEX_LEN, "%016llx", (unsigned long long)h);
}

const char* registerUser(const char* username,
                         const char* password,
                         const char* firstName,
                         const char* lastName,
                         const char* email) {
    // Check if username exists
    for (int i = 0; i < db_count; i++) {
        if (strcmp(db[i].username, username) == 0) {
            return "Username already exists";
        }
    }

    if (db_count >= MAX_USERS) {
        return "Database full";
    }

    // Insert new user
    User* u = &db[db_count++];
    strncpy(u->username, username, MAX_LEN_USERNAME - 1);
    u->username[MAX_LEN_USERNAME - 1] = '\0';

    strncpy(u->firstName, firstName, MAX_LEN_NAME - 1);
    u->firstName[MAX_LEN_NAME - 1] = '\0';

    strncpy(u->lastName, lastName, MAX_LEN_NAME - 1);
    u->lastName[MAX_LEN_NAME - 1] = '\0';

    strncpy(u->email, email, MAX_LEN_EMAIL - 1);
    u->email[MAX_LEN_EMAIL - 1] = '\0';

    simple_hash(password, u->passwordHash);

    return "Registration succeeded";
}

int main() {
    printf("%s\n", registerUser("alice", "p@ss1", "Alice", "Wonder", "alice@example.com"));
    printf("%s\n", registerUser("bob", "12345", "Bob", "Builder", "bob@example.com"));
    printf("%s\n", registerUser("alice", "newpass", "Alice", "W", "alice2@example.com"));
    printf("%s\n", registerUser("charlie", "pw", "Char", "Lie", "charlie@example.com"));
    printf("%s\n", registerUser("bob", "other", "Bobby", "B", "bob@other.com"));
    return 0;
}