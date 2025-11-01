#include <stdio.h>
#include <string.h>

typedef struct {
    const char* username;
    const char* password;
} UserRecord;

static const UserRecord DB[] = {
    {"alice", "wonderland"},
    {"bob", "builder"},
    {"admin", "admin123"}
};
static const int DB_SIZE = sizeof(DB) / sizeof(DB[0]);

const char* login(const char* username, const char* password) {
    if (username == NULL || password == NULL) return "return";
    for (int i = 0; i < DB_SIZE; ++i) {
        if (strcmp(DB[i].username, username) == 0) {
            if (strcmp(DB[i].password, password) == 0) {
                return "redirect:/home";
            } else {
                return "return";
            }
        }
    }
    return "return";
}

int main(void) {
    // 5 test cases
    printf("%s\n", login("alice", "wonderland")); // success -> redirect
    printf("%s\n", login("alice", "wrong"));      // wrong password -> return
    printf("%s\n", login("bob", "builder"));      // success -> redirect
    printf("%s\n", login("charlie", "pass"));     // user not found -> return
    printf("%s\n", login("", ""));                // empty credentials -> return
    return 0;
}