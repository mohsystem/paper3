#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>
#include <openssl/evp.h>
#include <openssl/rand.h>

#define ITERATIONS 100000
#define KEY_LEN 32
#define SALT_LEN 16
#define MAX_USERS 32

typedef struct {
    char* username;
    unsigned char salt[SALT_LEN];
    unsigned char hash[KEY_LEN];
} UserRecord;

typedef struct {
    UserRecord users[MAX_USERS];
    size_t count;
} Database;

static void init_db(Database* db) {
    db->count = 0;
}

static void free_db(Database* db) {
    for (size_t i = 0; i < db->count; ++i) {
        free(db->users[i].username);
        db->users[i].username = NULL;
    }
    db->count = 0;
}

static void trim_inplace(char* s) {
    if (!s) return;
    size_t len = strlen(s);
    size_t start = 0;
    while (start < len && isspace((unsigned char)s[start])) start++;
    size_t end = len;
    while (end > start && isspace((unsigned char)s[end - 1])) end--;
    size_t newlen = end - start;
    if (start > 0 && newlen > 0) memmove(s, s + start, newlen);
    s[newlen] = '\0';
}

static bool constant_time_eq(const unsigned char* a, const unsigned char* b, size_t len) {
    unsigned char res = 0;
    for (size_t i = 0; i < len; ++i) {
        res |= (a[i] ^ b[i]);
    }
    return res == 0;
}

static bool pbkdf2_sha256(const char* password, const unsigned char* salt, int salt_len, unsigned char* out, int out_len) {
    if (PKCS5_PBKDF2_HMAC(password, (int)strlen(password), salt, salt_len, ITERATIONS, EVP_sha256(), out_len, out) != 1) {
        return false;
    }
    return true;
}

void add_user(Database* db, const char* username, const char* password) {
    if (!db || !username || !password) return;
    if (db->count >= MAX_USERS) return;
    char* uname = strdup(username);
    if (!uname) return;
    trim_inplace(uname);
    if (uname[0] == '\0') { free(uname); return; }

    unsigned char salt[SALT_LEN];
    if (RAND_bytes(salt, SALT_LEN) != 1) { free(uname); return; }

    unsigned char hash[KEY_LEN];
    if (!pbkdf2_sha256(password, salt, SALT_LEN, hash, KEY_LEN)) { free(uname); return; }

    UserRecord* rec = &db->users[db->count++];
    rec->username = uname;
    memcpy(rec->salt, salt, SALT_LEN);
    memcpy(rec->hash, hash, KEY_LEN);
}

const char* login(Database* db, const char* username, const char* password) {
    if (!db || !username || !password) return "redirect:/login";
    char* uname = strdup(username);
    if (!uname) return "redirect:/login";
    trim_inplace(uname);
    if (uname[0] == '\0') {
        free(uname);
        return "redirect:/login";
    }
    for (size_t i = 0; i < db->count; ++i) {
        if (strcmp(db->users[i].username, uname) == 0) {
            unsigned char cand[KEY_LEN];
            bool ok = pbkdf2_sha256(password, db->users[i].salt, SALT_LEN, cand, KEY_LEN);
            free(uname);
            if (!ok) return "redirect:/login";
            if (constant_time_eq(db->users[i].hash, cand, KEY_LEN)) {
                return "redirect:/dashboard";
            } else {
                return "redirect:/login";
            }
        }
    }
    free(uname);
    return "redirect:/login";
}

int main() {
    Database db;
    init_db(&db);
    add_user(&db, "alice", "CorrectHorseBatteryStaple!");
    add_user(&db, "bob", "b0b$ecret");
    add_user(&db, "eve", "n0t_used");

    // 5 test cases
    printf("Test1 (alice correct): %s\n", login(&db, "alice", "CorrectHorseBatteryStaple!")); // dashboard
    printf("Test2 (alice wrong pw): %s\n", login(&db, "alice", "wrong")); // login
    printf("Test3 (unknown user): %s\n", login(&db, "charlie", "anything")); // login
    printf("Test4 (empty username): %s\n", login(&db, "   ", "whatever")); // login
    printf("Test5 (bob correct with trim): %s\n", login(&db, "  bob ", "b0b$ecret")); // dashboard

    free_db(&db);
    return 0;
}