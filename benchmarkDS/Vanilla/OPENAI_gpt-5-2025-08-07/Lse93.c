#include <stdio.h>
#include <stdint.h>
#include <string.h>

#define MAX_USERS 100
#define USERNAME_MAX 64
#define HASH_HEX_LEN 16

typedef struct {
    char username[USERNAME_MAX];
    char hashed_password[HASH_HEX_LEN + 1]; // 16 hex chars + null
} User;

static User database[MAX_USERS];
static int db_count = 0;

void hash_password(const char* password, char out_hex[HASH_HEX_LEN + 1]) {
    // FNV-1a 64-bit
    uint64_t hash = 0xcbf29ce484222325ULL;
    const uint64_t prime = 0x100000001b3ULL;
    for (const unsigned char* p = (const unsigned char*)password; *p; ++p) {
        hash ^= *p;
        hash *= prime;
    }
    // format as 16-hex chars
    static const char* hex = "0123456789abcdef";
    for (int i = 0; i < 8; ++i) {
        unsigned nibble_pair = (hash >> ((7 - i) * 8)) & 0xFF;
        out_hex[i * 2]     = hex[(nibble_pair >> 4) & 0xF];
        out_hex[i * 2 + 1] = hex[nibble_pair & 0xF];
    }
    out_hex[HASH_HEX_LEN] = '\0';
}

int insert_user(const char* username, const char* password) {
    if (db_count >= MAX_USERS) return 0;
    User* u = &database[db_count];
    strncpy(u->username, username, USERNAME_MAX - 1);
    u->username[USERNAME_MAX - 1] = '\0';
    hash_password(password, u->hashed_password);
    db_count++;
    return 1;
}

int main(void) {
    insert_user("alice", "password123");
    insert_user("bob", "s3cr3t");
    insert_user("charlie", "letmein");
    insert_user("diana", "P@ssw0rd!");
    insert_user("eve", "qwerty");

    printf("Current database:\n");
    for (int i = 0; i < db_count; ++i) {
        printf("%d) %s | %s\n", i + 1, database[i].username, database[i].hashed_password);
    }
    return 0;
}