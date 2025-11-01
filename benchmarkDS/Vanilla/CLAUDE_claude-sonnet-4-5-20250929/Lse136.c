
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

typedef struct {
    char username[50];
    char password[50];
} Credential;

const char* check_mod(const char* username, const char* password) {
    static Credential validCredentials[] = {
        {"admin", "admin123"},
        {"user1", "password1"},
        {"moderator", "mod123"},
        {"testuser", "test456"},
        {"john", "john789"}
    };
    
    int numCredentials = sizeof(validCredentials) / sizeof(Credential);
    
    if (username == NULL || password == NULL) {
        return "false";
    }
    
    for (int i = 0; i < numCredentials; i++) {
        if (strcmp(validCredentials[i].username, username) == 0 &&
            strcmp(validCredentials[i].password, password) == 0) {
            return "true";
        }
    }
    
    return "false";
}

int main() {
    // Test case 1: Valid credentials - admin
    printf("Test 1: %s\\n", check_mod("admin", "admin123"));
    
    // Test case 2: Valid credentials - user1
    printf("Test 2: %s\\n", check_mod("user1", "password1"));
    
    // Test case 3: Invalid password
    printf("Test 3: %s\\n", check_mod("admin", "wrongpassword"));
    
    // Test case 4: Invalid username
    printf("Test 4: %s\\n", check_mod("nonexistent", "password"));
    
    // Test case 5: NULL values
    printf("Test 5: %s\\n", check_mod(NULL, NULL));
    
    return 0;
}
