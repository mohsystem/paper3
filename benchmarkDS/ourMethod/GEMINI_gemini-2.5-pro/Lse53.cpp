#include <iostream>
#include <string>

/**
 * @brief Determines the redirection page based on user's login and admin status.
 *
 * @param isLoggedIn A boolean indicating if the user is logged in.
 * @param isAdmin    A boolean indicating if the logged-in user is an admin.
 * @return A std::string representing the page to redirect to.
 */
std::string getRedirectPage(bool isLoggedIn, bool isAdmin) {
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
    std::cout << "Test Case 1: (Logged In: false, Is Admin: false) -> " << getRedirectPage(false, false) << std::endl;
    std::cout << "Test Case 2: (Logged In: false, Is Admin: true) -> " << getRedirectPage(false, true) << std::endl;
    std::cout << "Test Case 3: (Logged In: true, Is Admin: false) -> " << getRedirectPage(true, false) << std::endl;
    std::cout << "Test Case 4: (Logged In: true, Is Admin: true) -> " << getRedirectPage(true, true) << std::endl;
    std::cout << "Test Case 5: (Another not logged in) -> " << getRedirectPage(false, false) << std::endl;
    
    return 0;
}