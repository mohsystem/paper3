#include <stdio.h>
#include <string.h>
#include <stddef.h>

typedef struct {
    const char* user;
    const char* pass;
} Credential;

static const Credential DB[] = {
    {"alice", "Wonderland!42"},
    {"bob", "Secure*Pass9"},
    {"charlie", "S3cret#Key"}
};
static const size_t DB_COUNT = sizeof(DB) / sizeof(DB[0]);
static const char* DUMMY_PASSWORD = "dummy_secret_password_123";

static int is_printable_ascii(const char* s) {
    for (size_t i = 0; s[i] != '\0'; ++i) {
        unsigned char ch = (unsigned char)s[i];
        if (ch < 32 || ch > 126) return 0;
    }
    return 1;
}

static int constant_time_equals(const char* a, size_t la, const char* b, size_t lb) {
    size_t max = la > lb ? la : lb;
    unsigned int diff = (unsigned int)(la ^ lb);
    for (size_t i = 0; i < max; ++i) {
        unsigned char ba = i < la ? (unsigned char)a[i] : 0;
        unsigned char bb = i < lb ? (unsigned char)b[i] : 0;
        diff |= (unsigned int)(ba ^ bb);
    }
    return diff == 0;
}

const char* login(const char* username, const char* password) {
    if (username == NULL || password == NULL) {
        return "internal_server_error";
    }
    size_t ulen = strlen(username);
    size_t plen = strlen(password);
    if (ulen < 1 || ulen > 128 || plen < 1 || plen > 128) {
        return "internal_server_error";
    }
    if (!is_printable_ascii(username) || !is_printable_ascii(password)) {
        return "internal_server_error";
    }

    const char* stored = NULL;
    for (size_t i = 0; i < DB_COUNT; ++i) {
        if (strcmp(DB[i].user, username) == 0) {
            stored = DB[i].pass;
            break;
        }
    }

    const char* compare_against = stored != NULL ? stored : DUMMY_PASSWORD;
    int match = constant_time_equals(password, plen, compare_against, strlen(compare_against));

    if (stored != NULL && match) {
        return "success";
    } else {
        return "cant_login";
    }
}

int main(void) {
    // 5 test cases
    printf("%s\n", login("alice", "Wonderland!42")); // success
    printf("%s\n", login("bob", "Secure*Pass9"));    // success
    printf("%s\n", login("alice", "NotIt"));         // cant_login
    printf("%s\n", login("dave", "anything"));       // cant_login
    printf("%s\n", login(NULL, "x"));                // internal_server_error
    return 0;
}