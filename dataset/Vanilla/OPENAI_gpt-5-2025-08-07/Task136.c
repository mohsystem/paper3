#include <stdio.h>
#include <string.h>

int authenticate(const char* usernames[], const char* passwords[], int n, const char* username, const char* password) {
    if (usernames == NULL || passwords == NULL || username == NULL || password == NULL) return 0;
    for (int i = 0; i < n; ++i) {
        if (strcmp(usernames[i], username) == 0) {
            return strcmp(passwords[i], password) == 0 ? 1 : 0;
        }
    }
    return 0;
}

int main() {
    const char* usernames[] = {"alice", "bob", "charlie"};
    const char* passwords[] = {"pass123", "qwerty", "letmein"};
    int n = 3;

    printf("Test 1 (alice, correct): %d\n", authenticate(usernames, passwords, n, "alice", "pass123"));
    printf("Test 2 (bob, wrong): %d\n", authenticate(usernames, passwords, n, "bob", "wrong"));
    printf("Test 3 (unknown user): %d\n", authenticate(usernames, passwords, n, "eve", "anything"));
    printf("Test 4 (charlie, empty password): %d\n", authenticate(usernames, passwords, n, "charlie", ""));
    printf("Test 5 (case sensitivity): %d\n", authenticate(usernames, passwords, n, "Alice", "pass123"));

    return 0;
}