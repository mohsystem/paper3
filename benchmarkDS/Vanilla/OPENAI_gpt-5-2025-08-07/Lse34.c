#include <stdio.h>
#include <string.h>

const char* login_redirect(const char* username, const char* password,
                           const char** usernames, const char** passwords, int n) {
    for (int i = 0; i < n; i++) {
        if (strcmp(username, usernames[i]) == 0) {
            if (strcmp(password, passwords[i]) == 0) {
                return "/dashboard";
            } else {
                return "/login";
            }
        }
    }
    return "/login";
}

int main() {
    const char* usernames[] = {"alice", "bob", "carol"};
    const char* passwords[] = {"pass123", "secret", "hunter2"};
    int n = 3;

    const char* test_users[] = {"alice", "alice", "unknown", "bob", "carol"};
    const char* test_passes[] = {"pass123", "wrong", "pass123", "secret", "HUNTER2"};
    int tests = 5;

    for (int i = 0; i < tests; i++) {
        const char* result = login_redirect(test_users[i], test_passes[i], usernames, passwords, n);
        printf("Test %d (%s): %s\n", i + 1, test_users[i], result);
    }

    return 0;
}