
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

typedef struct {
    char username[50];
    char password[50];
} Credential;

const char* checkMod(const char* username, const char* password) {
    // Static array of valid credentials
    static Credential validCredentials[] = {
        {"admin", "admin123"},
        {"user1", "pass1"},
        {"user2", "pass2"},
        {"testuser", "testpass"},
        {"moderator", "modpass"}
    };
    
    int numCredentials = sizeof(validCredentials) / sizeof(validCredentials[0]);
    
    if (username == NULL || password == NULL) {
        return "false";
    }
    
    // Check if username exists and password matches
    for (int i = 0; i < numCredentials; i++) {
        if (strcmp(validCredentials[i].username, username) == 0 && 
            strcmp(validCredentials[i].password, password) == 0) {
            return "true";
        }
    }
    
    return "false";
}

int main() {
    // Test case 1: Valid credentials
    printf("Test 1: %s\\n", checkMod("admin", "admin123"));
    
    // Test case 2: Valid credentials
    printf("Test 2: %s\\n", checkMod("user1", "pass1"));
    
    // Test case 3: Invalid password
    printf("Test 3: %s\\n", checkMod("admin", "wrongpass"));
    
    // Test case 4: Invalid username
    printf("Test 4: %s\\n", checkMod("invaliduser", "pass1"));
    
    // Test case 5: NULL inputs
    printf("Test 5: %s\\n", checkMod(NULL, NULL));
    
    return 0;
}
