#include <stdio.h>
#include <string.h>

const char* login(const char* username, const char* password) {
    if (username == NULL || password == NULL) {
        return "Incorrect username or password";
    }
    if (strcmp(username, "admin") == 0 && strcmp(password, "password123") == 0) {
        return "Redirecting to session page";
    }
    return "Incorrect username or password";
}

int main() {
    const char* tests[5][2] = {
        {"admin", "password123"},
        {"admin", "wrong"},
        {"user", "password123"},
        {"", "password123"},
        {"Admin", "password123"}
    };

    for (int i = 0; i < 5; i++) {
        printf("Test %d: %s\n", i + 1, login(tests[i][0], tests[i][1]));
    }

    return 0;
}