#include <stdio.h>
#include <string.h>

typedef struct {
    const char* user;
    const char* pass;
} Entry;

static const Entry DB[] = {
    {"alice", "password123"},
    {"bob", "qwerty"},
    {"charlie", "charlie!"},
    {"dana", "D@n@2025"},
    {"eve", "12345"}
};
static const int DB_SIZE = sizeof(DB) / sizeof(DB[0]);

// Returns "dashboard" if credentials are correct, otherwise "login"
const char* login(const char* username, const char* password) {
    if (username == NULL || password == NULL) return "login";
    for (int i = 0; i < DB_SIZE; ++i) {
        if (strcmp(DB[i].user, username) == 0 && strcmp(DB[i].pass, password) == 0) {
            return "dashboard";
        }
    }
    return "login";
}

int main() {
    // 5 test cases
    const char* tests[5][2] = {
        {"alice", "password123"}, // correct
        {"alice", "wrong"},       // wrong password
        {"unknown", "whatever"},  // unknown user
        {"Bob", "qwerty"},        // case sensitivity
        {"", ""}                  // empty credentials
    };

    for (int i = 0; i < 5; ++i) {
        const char* user = tests[i][0];
        const char* pass = tests[i][1];
        const char* result = login(user, pass);
        printf("login(\"%s\", \"%s\") => %s\n", user, pass, result);
    }
    return 0;
}