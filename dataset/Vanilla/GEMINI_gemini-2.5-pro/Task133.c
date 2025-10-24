#include <stdio.h>
#include <string.h>
#include <stdbool.h>

#define MAX_USERS 10
#define MAX_NAME_LEN 50
#define MAX_PASS_LEN 50

// Structure to represent a user
typedef struct {
    char username[MAX_NAME_LEN];
    char password[MAX_PASS_LEN];
} User;

/**
 * @brief Resets the password for a given user if the user exists in the array.
 * 
 * @param users Array of User structs.
 * @param user_count The number of users currently in the array.
 * @param username The username to search for.
 * @param newPassword The new password to set.
 * @return true if the password was reset successfully, false otherwise.
 */
bool resetPassword(User users[], int user_count, const char* username, const char* newPassword) {
    if (username == NULL || strlen(username) == 0 || newPassword == NULL) {
        return false;
    }
    
    for (int i = 0; i < user_count; i++) {
        if (strcmp(users[i].username, username) == 0) {
            // Found the user, update password
            strncpy(users[i].password, newPassword, MAX_PASS_LEN - 1);
            users[i].password[MAX_PASS_LEN - 1] = '\0'; // Ensure null-termination
            return true;
        }
    }
    return false; // User not found
}

void printDatabase(User users[], int user_count) {
    printf("{");
    for (int i = 0; i < user_count; i++) {
        printf("%s: %s", users[i].username, users[i].password);
        if (i < user_count - 1) {
            printf(", ");
        }
    }
    printf("}\n");
}

int main() {
    // Initialize the database with some users
    User userDatabase[MAX_USERS];
    int userCount = 3;

    strcpy(userDatabase[0].username, "alice");
    strcpy(userDatabase[0].password, "password123");
    strcpy(userDatabase[1].username, "bob");
    strcpy(userDatabase[1].password, "bobspass");
    strcpy(userDatabase[2].username, "charlie");
    strcpy(userDatabase[2].password, "securepass");
    
    printf("Initial Database: ");
    printDatabase(userDatabase, userCount);
    printf("--- Running Test Cases ---\n");
    
    // Test Case 1: Successful password reset for an existing user
    bool result1 = resetPassword(userDatabase, userCount, "alice", "newAlicePass!@#");
    printf("1. Resetting 'alice' password. Success: %s\n", result1 ? "true" : "false");
    printf("   Database after: ");
    printDatabase(userDatabase, userCount);

    // Test Case 2: Attempt to reset password for a non-existent user
    bool result2 = resetPassword(userDatabase, userCount, "dave", "davespass");
    printf("\n2. Resetting 'dave' (non-existent) password. Success: %s\n", result2 ? "true" : "false");
    printf("   Database after: ");
    printDatabase(userDatabase, userCount);

    // Test Case 3: Successful reset for another existing user
    bool result3 = resetPassword(userDatabase, userCount, "bob", "betterP@ssw0rd");
    printf("\n3. Resetting 'bob' password. Success: %s\n", result3 ? "true" : "false");
    printf("   Database after: ");
    printDatabase(userDatabase, userCount);

    // Test Case 4: Resetting the same user's password again
    bool result4 = resetPassword(userDatabase, userCount, "alice", "evenNewerPass456");
    printf("\n4. Resetting 'alice' password again. Success: %s\n", result4 ? "true" : "false");
    printf("   Database after: ");
    printDatabase(userDatabase, userCount);

    // Test Case 5: Attempt to reset password with an empty username
    bool result5 = resetPassword(userDatabase, userCount, "", "somepassword");
    printf("\n5. Resetting password for empty username. Success: %s\n", result5 ? "true" : "false");
    printf("   Database after: ");
    printDatabase(userDatabase, userCount);

    return 0;
}