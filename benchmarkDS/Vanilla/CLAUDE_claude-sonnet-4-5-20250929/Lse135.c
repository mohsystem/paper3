
#include <stdio.h>
#include <string.h>
#include <stdbool.h>

#define MAX_USERS 10
#define MAX_STRING_LEN 50

typedef struct {
    char username[MAX_STRING_LEN];
    char password[MAX_STRING_LEN];
} User;

// Simulated user database
User userDatabase[MAX_USERS];
int userCount = 0;

void initializeDatabase() {
    strcpy(userDatabase[0].username, "admin");
    strcpy(userDatabase[0].password, "admin123");
    
    strcpy(userDatabase[1].username, "user1");
    strcpy(userDatabase[1].password, "password1");
    
    strcpy(userDatabase[2].username, "moderator");
    strcpy(userDatabase[2].password, "mod123");
    
    strcpy(userDatabase[3].username, "john");
    strcpy(userDatabase[3].password, "doe456");
    
    strcpy(userDatabase[4].username, "alice");
    strcpy(userDatabase[4].password, "wonderland");
    
    userCount = 5;
}

/**
 * Checks if the provided username and password are correct
 * @param username The username from the form
 * @param password The password from the form
 * @return true if credentials are correct, false otherwise
 */
bool checkMod(const char* username, const char* password) {
    if (username == NULL || password == NULL) {
        return false;
    }
    
    if (strlen(username) == 0 || strlen(password) == 0) {
        return false;
    }
    
    for (int i = 0; i < userCount; i++) {
        if (strcmp(userDatabase[i].username, username) == 0) {
            return strcmp(userDatabase[i].password, password) == 0;
        }
    }
    
    return false;
}

int main() {
    initializeDatabase();
    
    // Test case 1: Valid credentials
    printf("Test 1 - Valid user (admin/admin123): %s\\n", 
           checkMod("admin", "admin123") ? "true" : "false");
    
    // Test case 2: Valid credentials
    printf("Test 2 - Valid user (user1/password1): %s\\n", 
           checkMod("user1", "password1") ? "true" : "false");
    
    // Test case 3: Invalid password
    printf("Test 3 - Invalid password (admin/wrong): %s\\n", 
           checkMod("admin", "wrong") ? "true" : "false");
    
    // Test case 4: Non-existent user
    printf("Test 4 - Non-existent user (hacker/hack): %s\\n", 
           checkMod("hacker", "hack") ? "true" : "false");
    
    // Test case 5: NULL values
    printf("Test 5 - NULL values: %s\\n", 
           checkMod(NULL, NULL) ? "true" : "false");
    
    return 0;
}
