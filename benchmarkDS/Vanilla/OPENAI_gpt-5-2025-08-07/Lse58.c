#include <stdio.h>
#include <string.h>
#include <stdbool.h>

#define USERNAME_LEN 64
#define EMAIL_LEN 128
#define PASSWORD_LEN 64

typedef struct {
    char username[USERNAME_LEN];
    char email[EMAIL_LEN];
    char password[PASSWORD_LEN];
    bool logged_in;
} User;

void init_user(User* u, const char* username, const char* email, const char* password) {
    if (!u) return;
    strncpy(u->username, username, USERNAME_LEN - 1);
    u->username[USERNAME_LEN - 1] = '\0';
    strncpy(u->email, email, EMAIL_LEN - 1);
    u->email[EMAIL_LEN - 1] = '\0';
    strncpy(u->password, password, PASSWORD_LEN - 1);
    u->password[PASSWORD_LEN - 1] = '\0';
    u->logged_in = false;
}

bool login(User* u, const char* password) {
    if (!u || !password) return false;
    if (strcmp(u->password, password) == 0) {
        u->logged_in = true;
        return true;
    }
    return false;
}

bool change_email(User* u, const char* old_email, const char* password, const char* new_email) {
    if (!u || !old_email || !password || !new_email) return false;
    if (!u->logged_in) return false;
    if (strcmp(u->email, old_email) != 0) return false;
    if (strcmp(u->password, password) != 0) return false;
    strncpy(u->email, new_email, EMAIL_LEN - 1);
    u->email[EMAIL_LEN - 1] = '\0';
    return true;
}

int main() {
    // Test 1: Not logged in
    User u1; init_user(&u1, "bob", "bob@example.com", "bobpass");
    bool t1 = change_email(&u1, "bob@example.com", "bobpass", "newbob@example.com");
    printf("Test1 (not logged in, expect false): %s | email=%s\n", t1 ? "true" : "false", u1.email);

    // Test 2: Correct login + change
    User u2; init_user(&u2, "alice", "alice@example.com", "pass123");
    bool login2 = login(&u2, "pass123");
    bool t2 = change_email(&u2, "alice@example.com", "pass123", "alice_new@example.com");
    printf("Test2 (logged in, correct creds, expect true): %s | email=%s\n", (login2 && t2) ? "true" : "false", u2.email);

    // Test 3: Wrong old email
    User u3; init_user(&u3, "carol", "carol@example.com", "cpass");
    login(&u3, "cpass");
    bool t3 = change_email(&u3, "wrong@example.com", "cpass", "carol_new@example.com");
    printf("Test3 (wrong old email, expect false): %s | email=%s\n", t3 ? "true" : "false", u3.email);

    // Test 4: Wrong password
    User u4; init_user(&u4, "dan", "dan@example.com", "dpass");
    login(&u4, "dpass");
    bool t4 = change_email(&u4, "dan@example.com", "wrongpass", "dan_new@example.com");
    printf("Test4 (wrong password, expect false): %s | email=%s\n", t4 ? "true" : "false", u4.email);

    // Test 5: After change, using original old email should fail
    User u5; init_user(&u5, "eve", "eve@example.com", "epass");
    login(&u5, "epass");
    bool firstChange = change_email(&u5, "eve@example.com", "epass", "eve_new@example.com");
    bool secondChange = change_email(&u5, "eve@example.com", "epass", "eve_new2@example.com");
    printf("Test5 (first true then second false): first=%s, second=%s | email=%s\n",
           firstChange ? "true" : "false",
           secondChange ? "true" : "false",
           u5.email);

    return 0;
}