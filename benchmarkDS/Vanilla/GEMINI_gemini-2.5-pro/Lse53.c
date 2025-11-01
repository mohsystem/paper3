#include <stdio.h>
#include <stdbool.h>

/**
 * Determines the redirection page based on user's login and admin status.
 *
 * @param isLoggedIn A boolean indicating if the user is logged in.
 * @param isAdmin    A boolean indicating if the logged-in user is an admin.
 * @return A constant character pointer (string) representing the page to redirect to.
 */
const char* redirectUser(bool isLoggedIn, bool isAdmin) {
    if (isLoggedIn) {
        if (isAdmin) {
            return "Admin Page";
        } else {
            return "User Page";
        }
    } else {
        return "Login Page";
    }
}

int main() {
    // Test Cases
    printf("Test Case 1 (Not logged in, not admin): %s\n", redirectUser(false, false));
    printf("Test Case 2 (Not logged in, is admin): %s\n", redirectUser(false, true));
    printf("Test Case 3 (Logged in, not admin): %s\n", redirectUser(true, false));
    printf("Test Case 4 (Logged in, is admin): %s\n", redirectUser(true, true));
    printf("Test Case 5 (Another logged in user): %s\n", redirectUser(true, false));

    return 0;
}