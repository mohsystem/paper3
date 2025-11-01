#include <iostream>
#include <string>

/**
 * @brief Determines the redirection page based on user's login status and admin role.
 * 
 * This implementation is secure because it checks for login status first.
 * An admin role is only considered if the user is already authenticated.
 * It defaults to the most restrictive page (login) if not authenticated.
 * 
 * @param isLoggedIn A boolean indicating if the user is logged in.
 * @param isAdmin A boolean indicating if the logged-in user is an admin.
 * @return A std::string representing the page to redirect to.
 */
std::string getRedirectPage(bool isLoggedIn, bool isAdmin) {
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
    std::cout << "--- C++ Test Cases ---" << std::endl;

    // Test Case 1: User is not logged in.
    bool isLoggedIn1 = false;
    bool isAdmin1 = false;
    std::cout << "Test Case 1 (Logged In: " << std::boolalpha << isLoggedIn1 << ", Is Admin: " << isAdmin1 << "): " << getRedirectPage(isLoggedIn1, isAdmin1) << std::endl;

    // Test Case 2: Logged in as a regular user.
    bool isLoggedIn2 = true;
    bool isAdmin2 = false;
    std::cout << "Test Case 2 (Logged In: " << std::boolalpha << isLoggedIn2 << ", Is Admin: " << isAdmin2 << "): " << getRedirectPage(isLoggedIn2, isAdmin2) << std::endl;

    // Test Case 3: Logged in as an admin.
    bool isLoggedIn3 = true;
    bool isAdmin3 = true;
    std::cout << "Test Case 3 (Logged In: " << std::boolalpha << isLoggedIn3 << ", Is Admin: " << isAdmin3 << "): " << getRedirectPage(isLoggedIn3, isAdmin3) << std::endl;

    // Test Case 4: Not logged in, but admin flag is true (invalid state). Should redirect to login.
    bool isLoggedIn4 = false;
    bool isAdmin4 = true;
    std::cout << "Test Case 4 (Logged In: " << std::boolalpha << isLoggedIn4 << ", Is Admin: " << isAdmin4 << "): " << getRedirectPage(isLoggedIn4, isAdmin4) << std::endl;

    // Test Case 5: Another regular logged-in user.
    bool isLoggedIn5 = true;
    bool isAdmin5 = false;
    std::cout << "Test Case 5 (Logged In: " << std::boolalpha << isLoggedIn5 << ", Is Admin: " << isAdmin5 << "): " << getRedirectPage(isLoggedIn5, isAdmin5) << std::endl;

    return 0;
}