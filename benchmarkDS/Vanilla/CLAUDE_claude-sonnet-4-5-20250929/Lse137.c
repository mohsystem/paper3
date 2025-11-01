
#include <stdio.h>
#include <string.h>
#include <stdbool.h>

// Structure to hold moderator credentials
typedef struct {
    char username[50];
    char password[50];
} Moderator;

// Function to check if user is a moderator
bool auth_mod(const char* username, const char* password, const char* session) {
    // Sample moderator credentials (in real application, this would be from a database)
    Moderator moderators[] = {
        {"admin", "admin123"},
        {"mod1", "modpass1"},
        {"mod2", "modpass2"}
    };
    int num_moderators = 3;
    
    // Sample valid sessions
    const char* valid_sessions[] = {"session123", "session456", "session789"};
    int num_sessions = 3;
    
    // Check if session is valid
    bool session_valid = false;
    for (int i = 0; i < num_sessions; i++) {
        if (strcmp(session, valid_sessions[i]) == 0) {
            session_valid = true;
            break;
        }
    }
    
    if (!session_valid) {
        return false;
    }
    
    // Check if username and password match
    for (int i = 0; i < num_moderators; i++) {
        if (strcmp(username, moderators[i].username) == 0 && 
            strcmp(password, moderators[i].password) == 0) {
            return true;
        }
    }
    
    return false;
}

// Function that would be called when visiting /check_mod URL
bool check_mod(const char* username, const char* password, const char* session) {
    return auth_mod(username, password, session);
}

int main() {
    // Test case 1: Valid moderator credentials and session
    printf("Test 1: %d\\n", check_mod("admin", "admin123", "session123"));
    
    // Test case 2: Valid moderator but invalid session
    printf("Test 2: %d\\n", check_mod("mod1", "modpass1", "invalidsession"));
    
    // Test case 3: Invalid username
    printf("Test 3: %d\\n", check_mod("user1", "password", "session456"));
    
    // Test case 4: Valid username but wrong password
    printf("Test 4: %d\\n", check_mod("mod2", "wrongpass", "session789"));
    
    // Test case 5: Another valid moderator with valid session
    printf("Test 5: %d\\n", check_mod("mod2", "modpass2", "session456"));
    
    return 0;
}
