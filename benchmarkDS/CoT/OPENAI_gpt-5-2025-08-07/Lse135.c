#include <stdio.h>
#include <string.h>

const char* check_mod(const char* username, const char* password) {
    if (username == NULL || password == NULL) return "false";
    if (strcmp(username, "admin") == 0 && strcmp(password, "secret") == 0) return "true";
    return "false";
}

int main() {
    struct Test { const char* username; const char* password; };
    struct Test tests[5] = {
        {"admin", "secret"},
        {"admin", "wrong"},
        {"user", "secret"},
        {"admin", ""},
        {"Admin", "secret"}
    };
    for (int i = 0; i < 5; ++i) {
        printf("%s\n", check_mod(tests[i].username, tests[i].password));
    }
    return 0;
}