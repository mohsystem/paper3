#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <ctype.h>

static void trim(char* s) {
    // Trim leading
    char* p = s;
    while (*p && isspace((unsigned char)*p)) p++;
    if (p != s) memmove(s, p, strlen(p) + 1);
    // Trim trailing
    size_t len = strlen(s);
    while (len > 0 && isspace((unsigned char)s[len - 1])) {
        s[len - 1] = '\0';
        len--;
    }
}

static void generate_salt_hex(char* out, size_t bytes) {
    // out must be at least 2*bytes + 1
    for (size_t i = 0; i < bytes; ++i) {
        unsigned char b = (unsigned char)(rand() & 0xFF);
        sprintf(out + 2 * i, "%02x", b);
    }
    out[2 * bytes] = '\0';
}

static unsigned long djb2_hash(const char* str) {
    unsigned long hash = 5381UL;
    int c;
    while ((c = (unsigned char)*str++)) {
        hash = ((hash << 5) + hash) + (unsigned long)c; // hash * 33 + c
    }
    return hash;
}

static void ulong_to_hex(unsigned long v, char* out) {
    // out should be large enough (e.g., 17 bytes)
    sprintf(out, "%lx", v);
}

static int ensure_db_exists(const char* db_path) {
    FILE* f = fopen(db_path, "ab+");
    if (!f) return -1;
    fclose(f);
    return 0;
}

static int username_exists(const char* db_path, const char* username) {
    FILE* f = fopen(db_path, "rb");
    if (!f) return 0; // treat as not exists
    char line[4096];
    int found = 0;
    while (fgets(line, sizeof(line), f)) {
        char* comma = strchr(line, ',');
        if (comma) {
            *comma = '\0';
        } else {
            line[strcspn(line, "\r\n")] = 0;
        }
        // line now holds username
        if (strcmp(line, username) == 0) {
            found = 1;
            break;
        }
    }
    fclose(f);
    return found;
}

// Returns malloc'd message string that caller must free.
char* register_user(const char* db_path, const char* username_in, const char* password) {
    if (!username_in || !*username_in) {
        char* msg = (char*)malloc(28);
        strcpy(msg, "ERROR: Username cannot be empty");
        return msg;
    }
    if (!password || !*password) {
        char* msg = (char*)malloc(28);
        strcpy(msg, "ERROR: Password cannot be empty");
        return msg;
    }

    if (ensure_db_exists(db_path) != 0) {
        char* msg = (char*)malloc(23);
        strcpy(msg, "ERROR: Cannot open DB");
        return msg;
    }

    char username[1024];
    strncpy(username, username_in, sizeof(username) - 1);
    username[sizeof(username) - 1] = '\0';
    trim(username);
    if (username[0] == '\0') {
        char* msg = (char*)malloc(28);
        strcpy(msg, "ERROR: Username cannot be empty");
        return msg;
    }

    if (username_exists(db_path, username)) {
        char* msg = (char*)malloc(29);
        strcpy(msg, "ERROR: Username already exists");
        return msg;
    }

    char salt_hex[33]; // 16 bytes -> 32 hex chars + null
    generate_salt_hex(salt_hex, 16);

    // Build salted string
    char combined[2048];
    snprintf(combined, sizeof(combined), "%s%s", salt_hex, password);

    unsigned long hv = djb2_hash(combined);
    char hash_hex[32];
    ulong_to_hex(hv, hash_hex);

    FILE* f = fopen(db_path, "ab");
    if (!f) {
        char* msg = (char*)malloc(23);
        strcpy(msg, "ERROR: Cannot open DB");
        return msg;
    }
    fprintf(f, "%s,%s,%s\n", username, salt_hex, hash_hex);
    fclose(f);

    char* msg = (char*)malloc(21);
    strcpy(msg, "OK: User registered");
    return msg;
}

int main(void) {
    srand((unsigned int)time(NULL));
    const char* db = "users_c.csv";
    char* r1 = register_user(db, "alice", "password123");
    printf("%s\n", r1); free(r1);
    char* r2 = register_user(db, "bob", "Secr3t!");
    printf("%s\n", r2); free(r2);
    char* r3 = register_user(db, "charlie", "hunter2");
    printf("%s\n", r3); free(r3);
    char* r4 = register_user(db, "alice", "anotherpass"); // duplicate
    printf("%s\n", r4); free(r4);
    char* r5 = register_user(db, "dave", ""); // empty password
    printf("%s\n", r5); free(r5);
    return 0;
}