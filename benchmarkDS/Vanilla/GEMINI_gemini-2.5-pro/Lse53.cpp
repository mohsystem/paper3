#include <iostream>
#include <string>

/**
 * Determines the redirection page based on user's login and admin status.
 *
 * @param isLoggedIn A boolean indicating if the user is logged in.
 * @param isAdmin    A boolean indicating if the logged-in user is an admin.
 * @return A std::string representing the page to redirect to.
 */
std::string redirectUser(bool isLoggedIn, bool isAdmin) {
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
    std::cout << "Test Case 1 (Not logged in, not admin): " << redirectUser(false, false) << std::endl;
    std::cout << "Test Case 2 (Not logged in, is admin): " << redirectUser(false, true) << std::endl;
    std::cout << "Test Case 3 (Logged in, not admin): " << redirectUser(true, false) << std::endl;
    std::cout << "Test Case 4 (Logged in, is admin): " << redirectUser(true, true) << std::endl;
    std::cout << "Test Case 5 (Another logged in user): " << redirectUser(true, false) << std::endl;

    return 0;
}