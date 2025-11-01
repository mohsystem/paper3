#include <stdio.h>
#include <string.h>
#include <stdbool.h>

/**
 * @brief Checks if the username and password are correct.
 * The correct credentials are "admin" and "password123".
 * 
 * @param username The username string to check.
 * @param password The password string to check.
 * @return true if the credentials are correct, false otherwise.
 */
bool check_mod(const char* username, const char* password) {
    if (username == NULL || password == NULL) {
        return false;
    }
    
    // strcmp returns 0 if the strings are equal
    int user_match = strcmp(username, "admin");
    int pass_match = strcmp(password, "password123");
    
    return user_match == 0 && pass_match == 0;
}

int main() {
    // 5 Test Cases
    printf("Test Case 1 (Correct): %s\n", check_mod("admin", "password123") ? "true" : "false");
    printf("Test Case 2 (Wrong Password): %s\n", check_mod("admin", "wrongpass") ? "true" : "false");
    printf("Test Case 3 (Wrong Username): %s\n", check_mod("user", "password123") ? "true" : "false");
    printf("Test Case 4 (Both Wrong): %s\n", check_mod("user", "wrongpass") ? "true" : "false");
    printf("Test Case 5 (Empty Strings): %s\n", check_mod("", "") ? "true" : "false");

    return 0;
}