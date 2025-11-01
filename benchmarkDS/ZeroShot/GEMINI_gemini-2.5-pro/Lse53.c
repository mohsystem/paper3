#include <stdio.h>
#include <stdbool.h>

/**
 * @brief Determines the redirection page based on user's login status and admin role.
 * 
 * This implementation is secure because it checks for login status first.
 * An admin role is only considered if the user is already authenticated.
 * It defaults to the most restrictive page (login) if not authenticated.
 * The function safely returns pointers to read-only string literals.
 * 
 * @param isLoggedIn A boolean indicating if the user is logged in.
 * @param isAdmin A boolean indicating if the logged-in user is an admin.
 * @return A const char* pointing to a string literal representing the page to redirect to.
 */
const char* getRedirectPage(bool isLoggedIn, bool isAdmin) {
    if (isLoggedIn) {
        if (isAdmin) {
            return "/admin"; // User is logged in and is an admin
        } else {
            return "/user"; // User is logged in but is not an admin
        }
    } else {
        return "/login"; // User is not logged in
    }
}

int main() {
    printf("--- C Test Cases ---\n");
    
    // Helper macro to print boolean as string
    #define BOOL_TO_STR(b) ((b) ? "true" : "false")

    // Test Case 1: User is not logged in.
    bool isLoggedIn1 = false;
    bool isAdmin1 = false;
    printf("Test Case 1 (Logged In: %s, Is Admin: %s): %s\n", BOOL_TO_STR(isLoggedIn1), BOOL_TO_STR(isAdmin1), getRedirectPage(isLoggedIn1, isAdmin1));

    // Test Case 2: Logged in as a regular user.
    bool isLoggedIn2 = true;
    bool isAdmin2 = false;
    printf("Test Case 2 (Logged In: %s, Is Admin: %s): %s\n", BOOL_TO_STR(isLoggedIn2), BOOL_TO_STR(isAdmin2), getRedirectPage(isLoggedIn2, isAdmin2));

    // Test Case 3: Logged in as an admin.
    bool isLoggedIn3 = true;
    bool isAdmin3 = true;
    printf("Test Case 3 (Logged In: %s, Is Admin: %s): %s\n", BOOL_TO_STR(isLoggedIn3), BOOL_TO_STR(isAdmin3), getRedirectPage(isLoggedIn3, isAdmin3));

    // Test Case 4: Not logged in, but admin flag is true (invalid state). Should redirect to login.
    bool isLoggedIn4 = false;
    bool isAdmin4 = true;
    printf("Test Case 4 (Logged In: %s, Is Admin: %s): %s\n", BOOL_TO_STR(isLoggedIn4), BOOL_TO_STR(isAdmin4), getRedirectPage(isLoggedIn4, isAdmin4));

    // Test Case 5: Another regular logged-in user.
    bool isLoggedIn5 = true;
    bool isAdmin5 = false;
    printf("Test Case 5 (Logged In: %s, Is Admin: %s): %s\n", BOOL_TO_STR(isLoggedIn5), BOOL_TO_STR(isAdmin5), getRedirectPage(isLoggedIn5, isAdmin5));

    return 0;
}