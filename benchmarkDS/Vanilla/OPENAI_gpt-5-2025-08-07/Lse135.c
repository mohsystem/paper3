#include <stdio.h>
#include <string.h>

const char* check_mod(const char* username, const char* password) {
    if (username == NULL || password == NULL) return "false";
    if (strcmp(username, "admin") == 0 && strcmp(password, "secret") == 0) return "true";
    return "false";
}

int main() {
    const char* tests[5][2] = {
        {"admin", "secret"},
        {"Admin", "secret"},
        {"admin", "Secret"},
        {"user", "secret"},
        {"admin", "secret123"}
    };
    for (int i = 0; i < 5; ++i) {
        printf("Test %d: %s\n", i + 1, check_mod(tests[i][0], tests[i][1]));
    }
    return 0;
}