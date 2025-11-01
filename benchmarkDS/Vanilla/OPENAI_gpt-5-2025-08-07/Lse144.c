#include <stdio.h>
#include <string.h>

static const char* DATABASE[] = {
    "alice@example.com",
    "bob_fail@example.com",
    "charlie@example.com"
};
static const int DB_SIZE = 3;

static int emailExists(const char* email) {
    if (email == NULL) return 0;
    for (int i = 0; i < DB_SIZE; ++i) {
        if (strcmp(DATABASE[i], email) == 0) return 1;
    }
    return 0;
}

// Simulated external script: returns 0 on success, non-zero on failure
static int callUnsubscribeScript(const char* email) {
    if (email == NULL || email[0] == '\0') return 1; // failure
    if (strstr(email, "fail") != NULL) return 2;     // simulate failure
    return 0; // success
}

// Returns 1 if email exists and unsubscribe script succeeds (returns 0), else 0
int unsubscribeIfExists(const char* email) {
    if (!emailExists(email)) return 0;
    int scriptResult = callUnsubscribeScript(email);
    return scriptResult == 0 ? 1 : 0;
}

int main(void) {
    const char* tests[5] = {
        "alice@example.com",     // 1
        "missing@example.com",   // 0
        "bob_fail@example.com",  // 0
        "",                      // 0
        "charlie@example.com"    // 1
    };
    for (int i = 0; i < 5; ++i) {
        printf("%d\n", unsubscribeIfExists(tests[i]));
    }
    return 0;
}