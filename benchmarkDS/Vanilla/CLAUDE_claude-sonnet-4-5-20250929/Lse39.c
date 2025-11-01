
#include <stdio.h>
#include <string.h>

typedef struct {
    char username[50];
    char password[50];
} User;

User userDatabase[5] = {
    {"admin", "admin123"},
    {"user1", "password1"},
    {"user2", "password2"},
    {"testuser", "test123"},
    {"john", "doe456"}
};

const char* login(const char* username, const char* password) {
    if (username == NULL || password == NULL) {
        return "Username or password is incorrect";
    }
    
    for (int i = 0; i < 5; i++) {
        if (strcmp(userDatabase[i].username, username) == 0 && 
            strcmp(userDatabase[i].password, password) == 0) {
            return "Login successful! Redirecting to session page...";
        }
    }
    
    return "Username or password is incorrect";
}

int main() {
    // Test cases
    printf("Test Case 1:\\n");
    printf("%s\\n\\n", login("admin", "admin123"));
    
    printf("Test Case 2:\\n");
    printf("%s\\n\\n", login("user1", "wrongpassword"));
    
    printf("Test Case 3:\\n");
    printf("%s\\n\\n", login("nonexistent", "password"));
    
    printf("Test Case 4:\\n");
    printf("%s\\n\\n", login("testuser", "test123"));
    
    printf("Test Case 5:\\n");
    printf("%s\\n", login("john", "doe456"));
    
    return 0;
}
