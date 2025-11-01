#include <stdio.h>
#include <string.h>

#define MAX_USERS 20
#define EMAIL_LEN 128
#define PASS_LEN 64

typedef struct {
    char emails[MAX_USERS][EMAIL_LEN];
    char passwords[MAX_USERS][PASS_LEN];
    int count;
    int currentIndex; // -1 means not logged in
} Auth;

void init(Auth* a) {
    a->count = 0;
    a->currentIndex = -1;
}

int findIndex(Auth* a, const char* email) {
    for (int i = 0; i < a->count; ++i) {
        if (strcmp(a->emails[i], email) == 0) return i;
    }
    return -1;
}

int addUser(Auth* a, const char* email, const char* password) {
    if (!email || !password) return 0;
    if (a->count >= MAX_USERS) return 0;
    if (findIndex(a, email) >= 0) return 0;
    strncpy(a->emails[a->count], email, EMAIL_LEN - 1);
    a->emails[a->count][EMAIL_LEN - 1] = '\0';
    strncpy(a->passwords[a->count], password, PASS_LEN - 1);
    a->passwords[a->count][PASS_LEN - 1] = '\0';
    a->count++;
    return 1;
}

int login(Auth* a, const char* email, const char* password) {
    if (!email || !password) return 0;
    int idx = findIndex(a, email);
    if (idx >= 0 && strcmp(a->passwords[idx], password) == 0) {
        a->currentIndex = idx;
        return 1;
    }
    return 0;
}

void logout(Auth* a) {
    a->currentIndex = -1;
}

int isLoggedIn(Auth* a) {
    return a->currentIndex >= 0;
}

const char* getCurrentEmail(Auth* a) {
    if (!isLoggedIn(a)) return NULL;
    return a->emails[a->currentIndex];
}

int changeEmail(Auth* a, const char* oldEmail, const char* newEmail, const char* confirmPassword) {
    if (!isLoggedIn(a)) return 0;                         // must be logged in
    if (!oldEmail || !newEmail || !confirmPassword) return 0;
    int oldIdx = findIndex(a, oldEmail);
    if (oldIdx < 0) return 0;                             // old email must exist
    if (a->currentIndex != oldIdx) return 0;              // must match logged-in user
    if (strcmp(a->passwords[oldIdx], confirmPassword) != 0) return 0; // confirm password must match
    if (findIndex(a, newEmail) >= 0) return 0;            // prevent duplicate emails
    strncpy(a->emails[oldIdx], newEmail, EMAIL_LEN - 1);
    a->emails[oldIdx][EMAIL_LEN - 1] = '\0';
    return 1;
}

// Seeder to pre-populate users
void seed(Auth* a) {
    init(a);
    addUser(a, "alice@example.com", "alice123");
    addUser(a, "bob@example.com", "bobpwd");
    addUser(a, "charlie@example.com", "charlie!");
}

int main() {
    // Test 1: Successful login and change
    Auth t1;
    seed(&t1);
    int t1Login = login(&t1, "alice@example.com", "alice123");
    int t1Change = changeEmail(&t1, "alice@example.com", "alice.new@example.com", "alice123");
    printf("Test1 login=%s change=%s current=%s\n",
           t1Login ? "true" : "false",
           t1Change ? "true" : "false",
           getCurrentEmail(&t1) ? getCurrentEmail(&t1) : "NULL");

    // Test 2: Attempt change without login
    Auth t2;
    seed(&t2);
    int t2Change = changeEmail(&t2, "bob@example.com", "bob.new@example.com", "bobpwd");
    printf("Test2 changeWithoutLogin=%s\n", t2Change ? "true" : "false");

    // Test 3: Wrong confirm password
    Auth t3;
    seed(&t3);
    int t3Login = login(&t3, "bob@example.com", "bobpwd");
    int t3Change = changeEmail(&t3, "bob@example.com", "bob.new@example.com", "wrong");
    printf("Test3 login=%s changeWithWrongPassword=%s\n",
           t3Login ? "true" : "false",
           t3Change ? "true" : "false");

    // Test 4: Non-existing old email while logged in
    Auth t4;
    seed(&t4);
    int t4Login = login(&t4, "charlie@example.com", "charlie!");
    int t4Change = changeEmail(&t4, "ghost@example.com", "ghost2@example.com", "charlie!");
    printf("Test4 login=%s changeWithNonExistingOldEmail=%s\n",
           t4Login ? "true" : "false",
           t4Change ? "true" : "false");

    // Test 5: New email already in use
    Auth t5;
    seed(&t5);
    int t5Login = login(&t5, "alice@example.com", "alice123");
    int t5Change = changeEmail(&t5, "alice@example.com", "bob@example.com", "alice123");
    printf("Test5 login=%s changeToExistingEmail=%s\n",
           t5Login ? "true" : "false",
           t5Change ? "true" : "false");

    return 0;
}