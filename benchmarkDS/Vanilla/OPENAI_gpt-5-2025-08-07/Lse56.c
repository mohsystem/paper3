#include <stdio.h>
#include <string.h>

typedef struct {
    char email[256];
    char password[256];
} User;

const char* change_email(User* user, int logged_in, const char* old_email, const char* new_email, const char* confirm_password) {
    static char msg[512];
    if (user == NULL || old_email == NULL || new_email == NULL || confirm_password == NULL) {
        snprintf(msg, sizeof(msg), "ERROR: Invalid input.");
        return msg;
    }
    if (!logged_in) {
        snprintf(msg, sizeof(msg), "ERROR: User must be logged in.");
        return msg;
    }
    if (strcmp(old_email, user->email) != 0) {
        snprintf(msg, sizeof(msg), "ERROR: Old email does not match.");
        return msg;
    }
    if (strcmp(new_email, user->email) == 0) {
        snprintf(msg, sizeof(msg), "ERROR: New email must be different from old email.");
        return msg;
    }
    if (strcmp(confirm_password, user->password) != 0) {
        snprintf(msg, sizeof(msg), "ERROR: Incorrect password.");
        return msg;
    }
    /* Update email (truncate if necessary) */
    strncpy(user->email, new_email, sizeof(user->email) - 1);
    user->email[sizeof(user->email) - 1] = '\0';
    snprintf(msg, sizeof(msg), "SUCCESS: Email changed to %s", user->email);
    return msg;
}

int main(void) {
    // Test case 1: Success
    User u1; strcpy(u1.email, "user@example.com"); strcpy(u1.password, "secret123");
    printf("%s\n", change_email(&u1, 1, "user@example.com", "new@example.com", "secret123"));

    // Test case 2: Not logged in
    User u2; strcpy(u2.email, "user@example.com"); strcpy(u2.password, "secret123");
    printf("%s\n", change_email(&u2, 0, "user@example.com", "newer@example.com", "secret123"));

    // Test case 3: Wrong old email
    User u3; strcpy(u3.email, "user@example.com"); strcpy(u3.password, "secret123");
    printf("%s\n", change_email(&u3, 1, "wrong@example.com", "newer@example.com", "secret123"));

    // Test case 4: Incorrect password
    User u4; strcpy(u4.email, "user@example.com"); strcpy(u4.password, "secret123");
    printf("%s\n", change_email(&u4, 1, "user@example.com", "newer@example.com", "badpass"));

    // Test case 5: New email same as old
    User u5; strcpy(u5.email, "user@example.com"); strcpy(u5.password, "secret123");
    printf("%s\n", change_email(&u5, 1, "user@example.com", "user@example.com", "secret123"));

    return 0;
}