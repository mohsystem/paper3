#include <iostream>
#include <string>

/**
 * @brief Determines the redirection page based on user's login and admin status.
 * 
 * @param isLoggedIn A boolean indicating if the user is logged in.
 * @param isAdmin A boolean indicating if the logged-in user is an admin.
 * @return A string message indicating the redirection destination.
 */
std::string redirectUser(bool isLoggedIn, bool isAdmin) {
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
    std::cout << "Test Case 1 (isLoggedIn: false, isAdmin: false): " << redirectUser(false, false) << std::endl;

    // Test Case 2: Not logged in, but admin flag is true (should still go to login)
    std::cout << "Test Case 2 (isLoggedIn: false, isAdmin: true): " << redirectUser(false, true) << std::endl;

    // Test Case 3: Logged in, but not an admin
    std::cout << "Test Case 3 (isLoggedIn: true, isAdmin: false): " << redirectUser(true, false) << std::endl;

    // Test Case 4: Logged in and is an admin
    std::cout << "Test Case 4 (isLoggedIn: true, isAdmin: true): " << redirectUser(true, true) << std::endl;
    
    // Test Case 5: Another test for a regular logged-in user
    std::cout << "Test Case 5 (isLoggedIn: true, isAdmin: false): " << redirectUser(true, false) << std::endl;

    return 0;
}