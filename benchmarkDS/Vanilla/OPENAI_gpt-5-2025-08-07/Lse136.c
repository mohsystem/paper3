#include <stdio.h>
#include <string.h>

const char* check_mod(const char* username, const char* password) {
    if (username == NULL || password == NULL) return "false";
    if (strcmp(username, "moderator") == 0 && strcmp(password, "mod123") == 0) return "true";
    return "false";
}

int main() {
    printf("%s\n", check_mod("moderator", "mod123"));  // true
    printf("%s\n", check_mod("moderator", "wrong"));   // false
    printf("%s\n", check_mod("user", "mod123"));       // false
    printf("%s\n", check_mod("moderator", ""));        // false
    printf("%s\n", check_mod("MODERATOR", "mod123"));  // false
    return 0;
}