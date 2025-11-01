
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

typedef struct {
    char username[50];
    char password[50];
} User;

User database[5] = {
    {"admin", "admin123"},
    {"user1", "password1"},
    {"user2", "password2"},
    {"testuser", "testpass"},
    {"john", "doe123"}
};

const char* login(const char* username, const char* password) {
    if (username == NULL || password == NULL || strlen(username) == 0 || strlen(password) == 0) {
        return "login";
    }
    
    for (int i = 0; i < 5; i++) {
        if (strcmp(database[i].username, username) == 0 && 
            strcmp(database[i].password, password) == 0) {
            return "dashboard";
        }
    }
    
    return "login";
}

int main() {
    // Test case 1: Valid credentials
    printf("Test 1 - Valid user (admin/admin123): %s\\n", login("admin", "admin123"));
    
    // Test case 2: Invalid password
    printf("Test 2 - Invalid password (admin/wrongpass): %s\\n", login("admin", "wrongpass"));
    
    // Test case 3: Non-existent user
    printf("Test 3 - Non-existent user (unknown/pass): %s\\n", login("unknown", "pass"));
    
    // Test case 4: Valid credentials (user1)
    printf("Test 4 - Valid user (user1/password1): %s\\n", login("user1", "password1"));
    
    // Test case 5: NULL credentials
    printf("Test 5 - NULL credentials: %s\\n", login(NULL, NULL));
    
    return 0;
}
