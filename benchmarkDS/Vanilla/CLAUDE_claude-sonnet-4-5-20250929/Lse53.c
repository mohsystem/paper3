
#include <stdio.h>
#include <stdbool.h>
#include <string.h>

typedef struct {
    bool isLoggedIn;
    bool isAdmin;
} Session;

const char* redirectUser(bool isLoggedIn, bool isAdmin) {
    if (!isLoggedIn) {
        return "login.html";
    } else if (isAdmin) {
        return "admin.html";
    } else {
        return "user.html";
    }
}

const char* handleRedirect(Session* session) {
    if (session == NULL) {
        return redirectUser(false, false);
    }
    return redirectUser(session->isLoggedIn, session->isAdmin);
}

int main() {
    // Test case 1: User not logged in
    printf("Test 1 - Not logged in: %s\\n", redirectUser(false, false));
    
    // Test case 2: User logged in but not admin
    printf("Test 2 - Logged in, not admin: %s\\n", redirectUser(true, false));
    
    // Test case 3: User logged in and is admin
    printf("Test 3 - Logged in, is admin: %s\\n", redirectUser(true, true));
    
    // Test case 4: Not logged in but somehow admin flag is true
    printf("Test 4 - Not logged in, admin flag true: %s\\n", redirectUser(false, true));
    
    // Test case 5: Regular user scenario with session
    Session session = {true, false};
    printf("Test 5 - Regular user with session: %s\\n", handleRedirect(&session));
    
    return 0;
}
