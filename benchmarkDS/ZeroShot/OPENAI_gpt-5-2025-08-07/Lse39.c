#include <stdio.h>
#include <string.h>
#include <stdbool.h>

typedef struct {
    const char* username;
    const char* password; // Expected password (demo only)
} Cred;

static const Cred USER_STORE[] = {
    {"alice", "Str0ngP@ss!"},
    {"admin", "S3cure#Admin"}
};

static const char* DUMMY_SECRET = "Dummy#Secret1";

// Constant-time comparison
bool secure_equals(const unsigned char* a, size_t alen, const unsigned char* b, size_t blen) {
    size_t maxlen = (alen > blen) ? alen : blen;
    unsigned int diff = (unsigned int)(alen ^ blen);
    for (size_t i = 0; i < maxlen; ++i) {
        unsigned char x = (i < alen) ? a[i] : 0;
        unsigned char y = (i < blen) ? b[i] : 0;
        diff |= (unsigned int)(x ^ y);
    }
    return diff == 0u;
}

// Login function: accepts parameters and returns result message
const char* login(const char* username, const char* password) {
    static char buf[128];
    if (username == NULL || username[0] == '\0') {
        // Perform dummy compare to keep timing uniform
        secure_equals((const unsigned char*)password, strlen(password),
                      (const unsigned char*)DUMMY_SECRET, strlen(DUMMY_SECRET));
        snprintf(buf, sizeof(buf), "Invalid username or password.");
        return buf;
    }

    const char* expected = NULL;
    bool userExists = false;
    for (size_t i = 0; i < sizeof(USER_STORE)/sizeof(USER_STORE[0]); ++i) {
        if (strcmp(username, USER_STORE[i].username) == 0) {
            expected = USER_STORE[i].password;
            userExists = true;
            break;
        }
    }
    if (!userExists) {
        expected = DUMMY_SECRET;
    }

    bool ok = secure_equals((const unsigned char*)password, strlen(password),
                            (const unsigned char*)expected, strlen(expected));

    if (ok && userExists) {
        snprintf(buf, sizeof(buf), "SESSION: %s", username);
    } else {
        snprintf(buf, sizeof(buf), "Invalid username or password.");
    }
    return buf;
}

int main(void) {
    // 5 test cases
    printf("%s\n", login("alice", "Str0ngP@ss!"));
    printf("%s\n", login("alice", "wrong"));
    printf("%s\n", login("admin", "S3cure#Admin"));
    printf("%s\n", login("eve", "anything"));
    printf("%s\n", login("", "S3cure#Admin"));
    return 0;
}