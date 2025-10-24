#include <sodium.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>

#define MAX_USERS 64
#define USERNAME_MAX 64

typedef struct {
    char username[USERNAME_MAX + 1];
    char hash[crypto_pwhash_STRBYTES]; // includes salt and params
    int in_use;
} User;

typedef struct {
    User users[MAX_USERS];
    size_t count;
} UserDb;

static int is_valid_username(const char* username) {
    if (username == NULL) return 0;
    size_t len = strnlen(username, USERNAME_MAX + 1);
    if (len == 0 || len > USERNAME_MAX) return 0;
    for (size_t i = 0; i < len; i++) {
        unsigned char c = (unsigned char)username[i];
        if (!(isalnum(c) || c=='_' || c=='-' || c=='.' || c=='@')) {
            return 0;
        }
    }
    return 1;
}

static int password_policy_valid(const char* password, const char* username, char* errbuf, size_t errlen) {
    if (!password) { snprintf(errbuf, errlen, "Null password"); return 0; }
    size_t plen = strnlen(password, 4097);
    if (plen < 12 || plen > 4096) { snprintf(errbuf, errlen, "Password length invalid"); return 0; }

    int hasLower=0, hasUpper=0, hasDigit=0, hasSymbol=0;
    for (size_t i = 0; i < plen; i++) {
        unsigned char c = (unsigned char)password[i];
        if (islower(c)) hasLower = 1;
        else if (isupper(c)) hasUpper = 1;
        else if (isdigit(c)) hasDigit = 1;
        else hasSymbol = 1;
    }
    if (!(hasLower && hasUpper && hasDigit && hasSymbol)) {
        snprintf(errbuf, errlen, "Password must include upper, lower, digit, and symbol");
        return 0;
    }

    // Must not contain username (case-insensitive)
    if (username && username[0] != '\0') {
        size_t ulen = strnlen(username, USERNAME_MAX + 1);
        char *plower = (char*)malloc(plen + 1);
        char *ulower = (char*)malloc(ulen + 1);
        if (!plower || !ulower) {
            if (plower) free(plower);
            if (ulower) free(ulower);
            snprintf(errbuf, errlen, "Memory error");
            return 0;
        }
        for (size_t i = 0; i < plen; i++) plower[i] = (char)tolower((unsigned char)password[i]);
        plower[plen] = '\0';
        for (size_t i = 0; i < ulen; i++) ulower[i] = (char)tolower((unsigned char)username[i]);
        ulower[ulen] = '\0';

        int bad = 0;
        if (ulen > 0) {
            // naive substring search
            if (strstr(plower, ulower) != NULL) bad = 1;
        }
        free(plower);
        free(ulower);
        if (bad) {
            snprintf(errbuf, errlen, "Password must not contain username");
            return 0;
        }
    }

    // No >=4 repeated characters in a row
    int run = 1;
    for (size_t i = 1; i < plen; i++) {
        if (password[i] == password[i-1]) {
            run++;
            if (run >= 4) { snprintf(errbuf, errlen, "Too many repeated characters"); return 0; }
        } else {
            run = 1;
        }
    }

    if (errbuf && errlen > 0) errbuf[0] = '\0';
    return 1;
}

static void userdb_init(UserDb* db) {
    if (!db) return;
    memset(db, 0, sizeof(*db));
}

static int userdb_find_index(const UserDb* db, const char* username) {
    if (!db || !username) return -1;
    for (size_t i = 0; i < db->count; i++) {
        if (db->users[i].in_use && strncmp(db->users[i].username, username, USERNAME_MAX) == 0) {
            return (int)i;
        }
    }
    return -1;
}

static int userdb_add(UserDb* db, const char* username, const char* password, char* errbuf, size_t errlen) {
    if (!db || !username || !password) { snprintf(errbuf, errlen, "Invalid input"); return 0; }
    if (!is_valid_username(username)) { snprintf(errbuf, errlen, "Invalid username"); return 0; }
    if (userdb_find_index(db, username) >= 0) { snprintf(errbuf, errlen, "User exists"); return 0; }
    if (db->count >= MAX_USERS) { snprintf(errbuf, errlen, "User DB full"); return 0; }

    char pwderr[128];
    if (!password_policy_valid(password, username, pwderr, sizeof(pwderr))) {
        snprintf(errbuf, errlen, "%s", pwderr);
        return 0;
    }

    User u;
    memset(&u, 0, sizeof(u));
    snprintf(u.username, sizeof(u.username), "%s", username);

    if (crypto_pwhash_str(u.hash,
                          password,
                          strnlen(password, 4097),
                          crypto_pwhash_OPSLIMIT_MODERATE,
                          crypto_pwhash_MEMLIMIT_MODERATE) != 0) {
        snprintf(errbuf, errlen, "Hashing error");
        return 0;
    }

    u.in_use = 1;
    db->users[db->count++] = u;
    if (errbuf && errlen > 0) errbuf[0] = '\0';
    return 1;
}

static int userdb_auth(const UserDb* db, const char* username, const char* password) {
    if (!db || !username || !password) return 0;
    int idx = userdb_find_index(db, username);
    if (idx < 0) return 0;
    const User* u = &db->users[(size_t)idx];
    if (!u->in_use) return 0;
    if (crypto_pwhash_str_verify(u->hash, password, strnlen(password, 4097)) == 0) {
        return 1;
    }
    return 0;
}

int main(void) {
    if (sodium_init() < 0) {
        fprintf(stderr, "Failed to initialize sodium\n");
        return 1;
    }

    UserDb db;
    userdb_init(&db);

    // Test case 1: Create user and authenticate successfully
    {
        char err[128];
        int created = userdb_add(&db, "alice", "GoodPass!234", err, sizeof(err));
        printf("TC1 Create alice success: %s\n", created ? "true" : "false");
        int authOK = userdb_auth(&db, "alice", "GoodPass!234");
        printf("TC1 Auth alice correct pwd: %s\n", authOK ? "true" : "false");
    }

    // Test case 2: Wrong password
    {
        int authBad = userdb_auth(&db, "alice", "WrongPass!234");
        printf("TC2 Auth alice wrong pwd: %s\n", authBad ? "true" : "false");
    }

    // Test case 3: Duplicate user
    {
        char err[128];
        int created = userdb_add(&db, "alice", "AnotherGood!999", err, sizeof(err));
        printf("TC3 Create duplicate alice: %s\n", created ? "true" : "false");
    }

    // Test case 4: Weak password policy
    {
        char err[128];
        int created = userdb_add(&db, "bob", "weakpass", err, sizeof(err));
        printf("TC4 Create bob weak password: %s\n", created ? "true" : "false");
    }

    // Test case 5: Another user success
    {
        char err[128];
        int created = userdb_add(&db, "charlie", "Stronger#Pass123", err, sizeof(err));
        printf("TC5 Create charlie success: %s\n", created ? "true" : "false");
        int authOK = userdb_auth(&db, "charlie", "Stronger#Pass123");
        printf("TC5 Auth charlie correct pwd: %s\n", authOK ? "true" : "false");
    }

    return 0;
}