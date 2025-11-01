#include <stdio.h>
#include <string.h>

const char* login(const char* username, const char* password) {
    typedef struct { const char* user; const char* pass; } User;
    static const User db[] = {
        {"alice", "password123"},
        {"bob", "qwerty"},
        {"charlie", "letmein"}
    };
    size_t db_len = sizeof(db) / sizeof(db[0]);

    if (username == NULL || password == NULL) {
        return "internal_server_error";
    }
    if (strcmp(username, "error") == 0 || strcmp(password, "error") == 0) {
        return "internal_server_error";
    }

    for (size_t i = 0; i < db_len; ++i) {
        if (strcmp(db[i].user, username) == 0) {
            if (strcmp(db[i].pass, password) == 0) {
                return "success";
            } else {
                return "cant_login";
            }
        }
    }
    return "cant_login";
}

int main() {
    printf("%s\n", login("alice", "password123")); // success
    printf("%s\n", login("alice", "wrong"));       // cant_login
    printf("%s\n", login("unknown", "pass"));      // cant_login
    printf("%s\n", login(NULL, "pass"));           // internal_server_error
    printf("%s\n", login("error", "pass"));        // internal_server_error
    return 0;
}