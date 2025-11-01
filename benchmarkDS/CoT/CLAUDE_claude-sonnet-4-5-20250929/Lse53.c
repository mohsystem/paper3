
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

// Enum to represent user roles
typedef enum {
    GUEST,
    USER,
    ADMIN
} UserRole;

// Struct to represent a user session
typedef struct {
    bool isLoggedIn;
    UserRole role;
    char username[100];
} UserSession;

/**
 * Determines the redirect page based on user session
 * @param session The user session pointer
 * @return The page to redirect to (caller should not free this string)
 */
const char* determineRedirectPage(const UserSession* session) {
    // Input validation
    if (session == NULL) {
        return "/login";
    }
    
    // Check if user is not logged in
    if (!session->isLoggedIn) {
        return "/login";
    }
    
    // Check if user is admin
    if (session->role == ADMIN) {
        return "/admin";
    }
    
    // Check if user is a regular user
    if (session->role == USER) {
        return "/user";
    }
    
    // Default fallback to login
    return "/login";
}

int main() {
    // Test case 1: Not logged in
    UserSession session1 = {false, GUEST, ""};
    printf("Test 1 - Not logged in: %s\\n", determineRedirectPage(&session1));
    
    // Test case 2: Logged in as regular user
    UserSession session2 = {true, USER, "john_doe"};
    printf("Test 2 - Regular user: %s\\n", determineRedirectPage(&session2));
    
    // Test case 3: Logged in as admin
    UserSession session3 = {true, ADMIN, "admin_user"};
    printf("Test 3 - Admin user: %s\\n", determineRedirectPage(&session3));
    
    // Test case 4: Null session
    printf("Test 4 - Null session: %s\\n", determineRedirectPage(NULL));
    
    // Test case 5: Logged in but with GUEST role
    UserSession session5 = {true, GUEST, "guest_user"};
    printf("Test 5 - Logged in guest: %s\\n", determineRedirectPage(&session5));
    
    return 0;
}
