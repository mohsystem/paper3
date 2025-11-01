#include <stdio.h>
#include <stdbool.h>

/**
 * @brief Determines the redirection page based on user's login and admin status.
 * 
 * @param isLoggedIn A boolean indicating if the user is logged in.
 * @param isAdmin A boolean indicating if the logged-in user is an admin.
 * @return A constant character pointer to a string message indicating the redirection destination.
 */
const char* redirectUser(bool isLoggedIn, bool isAdmin) {
    if (!isLoggedIn) {
        return "Redirecting to Login Page";
    } else {
        if (isAdmin) {
            return "Redirecting to Admin Page";
        } else {
            return "Redirecting to User Page";
        }
    }
}

int main() {
    // Test Case 1: Not logged in
    printf("Test Case 1 (isLoggedIn: false, isAdmin: false): %s\n", redirectUser(false, false));

    // Test Case 2: Not logged in, but admin flag is true (should still go to login)
    printf("Test Case 2 (isLoggedIn: false, isAdmin: true): %s\n", redirectUser(false, true));

    // Test Case 3: Logged in, but not an admin
    printf("Test Case 3 (isLoggedIn: true, isAdmin: false): %s\n", redirectUser(true, false));

    // Test Case 4: Logged in and is an admin
    printf("Test Case 4 (isLoggedIn: true, isAdmin: true): %s\n", redirectUser(true, true));
    
    // Test Case 5: Another test for a regular logged-in user
    printf("Test Case 5 (isLoggedIn: true, isAdmin: false): %s\n", redirectUser(true, false));

    return 0;
}