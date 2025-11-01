#include <stdio.h>
#include <stdbool.h>

/**
 * @brief Determines the redirection page based on user's login and admin status.
 *
 * @param isLoggedIn A boolean indicating if the user is logged in.
 * @param isAdmin    A boolean indicating if the logged-in user is an admin.
 * @return A const char pointer to a string literal representing the page to redirect to.
 */
const char* getRedirectPage(bool isLoggedIn, bool isAdmin) {
    if (isLoggedIn) {
        if (isAdmin) {
            return "admin page";
        } else {
            return "user page";
        }
    } else {
        return "login page";
    }
}

int main() {
    // Test cases
    printf("Test Case 1: (Logged In: false, Is Admin: false) -> %s\n", getRedirectPage(false, false));
    printf("Test Case 2: (Logged In: false, Is Admin: true) -> %s\n", getRedirectPage(false, true));
    printf("Test Case 3: (Logged In: true, Is Admin: false) -> %s\n", getRedirectPage(true, false));
    printf("Test Case 4: (Logged In: true, Is Admin: true) -> %s\n", getRedirectPage(true, true));
    printf("Test Case 5: (Another not logged in) -> %s\n", getRedirectPage(false, false));

    return 0;
}