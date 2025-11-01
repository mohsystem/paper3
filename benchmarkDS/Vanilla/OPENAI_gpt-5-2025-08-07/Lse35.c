#include <stdio.h>
#include <string.h>

typedef struct {
    const char* username;
    const char* password;
} UserPair;

static const UserPair DB[] = {
    {"alice", "secret"},
    {"bob", "hunter2"},
    {"eve", "password123"}
};
static const size_t DB_SIZE = sizeof(DB) / sizeof(DB[0]);

// Returns redirect path based on login success: "/home" or "/login"
const char* login_func(const char* username, const char* password) {
    if (username == NULL || password == NULL) return "/login";
    for (size_t i = 0; i < DB_SIZE; ++i) {
        if (strcmp(DB[i].username, username) == 0 && strcmp(DB[i].password, password) == 0) {
            return "/home";
        }
    }
    return "/login";
}

int main(void) {
    const char* tests[5][2] = {
        {"alice", "secret"},
        {"bob", "hunter2"},
        {"alice", "wrong"},
        {"unknown", "nopass"},
        {"eve", "password123"}
    };
    for (int i = 0; i < 5; ++i) {
        const char* user = tests[i][0];
        const char* pass = tests[i][1];
        const char* redirect = login_func(user, pass);
        printf("login(%s, %s) -> %s\n", user, pass, redirect);
    }
    return 0;
}